//========================================================================
// rsort-xloops-opt.cc
//========================================================================
// shreesha: XLOOPS implementation where an alternative algorithm is used
// for better performance. Illustrates app tuning for XLOOPS architectures

#include "rsort-xloops-opt.h"

#include <limits>
#include <algorithm>

extern void unordered_for();

//------------------------------------------------------------------------
// local amo functions
//------------------------------------------------------------------------

namespace {

  inline int fetch_add( volatile int* ptr, int value )
  {
    #ifdef _MIPS_ARCH_MAVEN
      int res;
      __asm__ volatile
        ( "amo.add %0, %1, %2"
          : "=r"(res) : "r"(ptr), "r"(value) : "memory" );
      return res;
    #else
      int temp = *ptr;
      *ptr += value;
      return temp;
    #endif
  }

}

namespace rsort {

  void rsort_xloops_opt( int dest[], int src[], int size, int radix_lg,
                         int nthreads )
  {
    // Some variables based on radix_lg

    int radix = 1 << radix_lg;
    unsigned int radix_mask = ~(~0u << radix_lg);

    // Determine how many threads we will be using to do the sort. Maybe
    // we should do this a different way? If we have a ton of threads or
    // a very short size then that means each thread only gets a very
    // little bit of work to do. For now we just use all the threads no
    // matter what. We partition based on both the size (when working
    // across the source input array) and the radix (when working across
    // digits in lookup tables).

    //int nthreads = 16;

    int range_size = (size + nthreads - 1) / nthreads;

    // We will use a digit lookup table during the radix sort.
    // Throughout the sort, this will contain information about each
    // possible value for the digit. We use gcc's extension which enable
    // dynamically allocating arrays on the stack.

    int global_digit_lut[radix];

    int local_digit_luts[radix*nthreads];

    // We need a temporary buffer so we can ping-pong back and forth
    // between the dest and temp buffers during each stage of the sort.
    // We use gcc's extension which enable dynamically allocating
    // arrays on the stack.

    int temp_buf[size];

    // Determne the number of stages in this sort. One stage for each
    // digit which is radix_lg bits wide. We need to find the number of
    // bits in an unsigned int, because otherwise it will not include
    // the sign bit. We do the extra bit of math to essentially round up
    // in case radix_lg does not evenly divide into bits_per_int.

    int bits_per_int = std::numeric_limits<unsigned int>::digits;
    int stages = ( bits_per_int + radix_lg - 1 ) / radix_lg;

    // Check if the number of stages is even or odd. If it is even then
    // the first stage should write the temporary buffer, but if it is
    // odd then the first stage should write the destination buffer.
    // This way when we are done all the stages the final result will
    // end up in the destination buffer.

    bool stages_even = ( (stages % 2) == 0 );

    int* stage_in   = src;
    int* stage_out  = ( stages_even ) ? temp_buf : dest;
    int* stage1_out = ( stages_even ) ? dest     : temp_buf;

    // Create vector of ptrs to each thread's local digit lookup table.

    int* local_digit_lut[nthreads];

    for ( int i = 0; i < nthreads; i++ )
      local_digit_lut[i] = &(local_digit_luts[i*radix]);

    // create the ranges for each thread
    int range_begin[ nthreads ];
    int range_end[ nthreads ];
    for ( int i = 0; i < nthreads; i++ ) {
      range_begin[i] = i * range_size;
      range_end[i] = std::min( range_begin[i] + range_size, size );
    }

    // Sort the input values looking at a single digit at a time.

    for ( int stage_idx = 0; stage_idx < stages; stage_idx++ ) {

      //------------------------------------------------------------------
      // Per stage setup
      //------------------------------------------------------------------

      // Zero out global and local digit lookup tables

      for ( int i = 0; i < radix; i++ ) {
        global_digit_lut[i] = 0;

        for ( int j = 0; j < nthreads; j++ )
          local_digit_lut[j][i] = 0;
      }

      // Put stage index * radix_lg into a variable for use in threads

      int radix_shift = stage_idx * radix_lg;

      //------------------------------------------------------------------
      // Phase 1 : Build local and global histograms
      //------------------------------------------------------------------
      // Each thread will work on a consecutive subset of the input
      // array. Consecutive subsets simplifies making this a stable
      // sort. Each thread calculates the digit value and update both
      // that thread's local histogram as well as the global histogram.
      // Note we need the local sync to make sure the updates to the
      // lookup tables are done before the next phase.

      // CANNOT parallelize xloop across array size, each hardware ut
      // must work on a consecutive subset of the array...

      for ( int loop_idx = 0; loop_idx < nthreads;
                                  loop_idx++, unordered_for() ) {

        for ( int i = range_begin[ loop_idx ];
                                i < range_end[ loop_idx ]; i++ ) {

          int digit = (stage_in[i] >> radix_shift) & radix_mask;

          // Each thread has its own local digit lookup table to update.

          local_digit_lut[loop_idx][digit]++;

          // We need to use an atomic op because all threads are
          // updating the same global digit lookup table.

          // can we also implement this as for.om?
          fetch_add( &(global_digit_lut[digit]), 1 );
        }

      }


      //------------------------------------------------------------------
      // Phase 2 : Calculate global offsets for each thread and digit
      //------------------------------------------------------------------
      // We first do a quick scalar sum of the global lookup table. The
      // global lookup table will then hold the starting index into the
      // output buffer for an element with that digit value. Then use
      // the threads to do a prefix sum of the local histograms - we are
      // vectorizing across digits this time. Each thread works on
      // calculating the prefix sum for a specific digit. When we are
      // done each thread will have a unique index into the output
      // buffer for each digit value which indicates where to start
      // writing its elements with that digit value. Again we need a
      // sync to make sure the writes to the lookup tables are done
      // before the next phase.

      int sum = 0;
      for ( int i = 0; i < radix; i++ ) {
        int temp = global_digit_lut[i];
        global_digit_lut[i] = sum;
        sum += temp;
      }

      // Each thread does a chunk of digits.

      for ( int i = 0; i < radix; i++, unordered_for() ) {

        int accum = global_digit_lut[i];

        for ( int j = 0; j < nthreads; j++ ) {
          int temp = local_digit_lut[j][i];
          local_digit_lut[j][i] = accum;
          accum += temp;
        }
      }

      //------------------------------------------------------------------
      // Phase 3 : Permute input into output
      //------------------------------------------------------------------
      // Iterate over keys again and write them to output buffer. We
      // update the local digit lookup tables as we go so they always
      // points to where the next value with that digit for that thread
      // should go in the output buffer. This loop looks very similar to
      // the Phase 1 loop, except we don't need to do anything with the
      // global digit lookup table and we use the local lookup tables to
      // figure out where to write into the destination array. Again we
      // need a sync to make sure the writes to the stage output buffer
      // are done before we start the next stage.

      for ( int loop_idx = 0; loop_idx < nthreads;
                                  loop_idx++, unordered_for() ) {

        for ( int i = range_begin[ loop_idx ];
                                    i < range_end[ loop_idx ]; i++ ) {

          int digit = (stage_in[i] >> radix_shift) & radix_mask;

          stage_out[ local_digit_lut[loop_idx][digit]++ ] = stage_in[i];

        }
      }


      //------------------------------------------------------------------
      // Finish up by swapping the input and output buffers
      //------------------------------------------------------------------
      // On the first stage we need to be careful because the input
      // buffer points to the source array which we don't want to
      // overwrite.

      if ( stage_idx == 0 ) {
        int* temp_ptr = stage_out;
        stage_out = stage1_out;
        stage_in  = temp_ptr;
      }
      else
        std::swap( stage_out, stage_in );
    }
  }

}

