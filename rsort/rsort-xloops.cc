//========================================================================
// rsort-xloops.cc
//========================================================================

#include "rsort-xloops.h"

#include <limits>
#include <algorithm>

extern void unordered_for();
extern void ordered_for();
extern void mem_for();
extern void parallel_for();

namespace rsort {

  void rsort_xloops( int dest[], int src[], int size, int radix_lg )
  {
    // Some variables based on radix_lg

    int radix = 1 << radix_lg;
    unsigned int radix_mask = ~(~0u << radix_lg);

    // We will use a digit lookup table during the radix sort.
    // Throughout the sort, this will contain information about each
    // possible value for the digit. We use gcc's extension which enable
    // dynamically allocating arrays on the stack.

    int digit_lut[radix];

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

    // Sort the input values looking at a single digit at a time.

    for ( int stage_idx = 0; stage_idx < stages; stage_idx++ ) {

      // Zero out digit lookup table

      for ( int i = 0; i < radix; i++ )
        digit_lut[i] = 0;

      // Iterate over keys and update the digit lookup table. The digit
      // lookup table is essentially a histogram at this point which
      // will contain how many values in the input buffer contain the
      // corresponding digit value.

      for ( int i = 0; i < size; i++, mem_for() ) {
        int digit = (stage_in[i] >> (stage_idx * radix_lg)) & radix_mask;
        digit_lut[digit]++;
      }

      // Parallel prefix sum over the digit lookup table. Digit lookup
      // table will now hold the starting index into the output buffer
      // for an element with that digit value.

      int accum = 0;
      for ( int i = 0; i < radix; i++, ordered_for() ) {
        // shreesha: need the asm hacks to prevent LSR pass in llvm from
        // choosing a solution which transforms the IV starting from value
        // 1 and hence not resulting in a "bne" check condition. With these
        // changes, xfor.r instruction would be inferred
        __asm__ volatile ( "" );
        int temp = digit_lut[i];
        digit_lut[i] = accum;
        accum += temp;
        __asm__ volatile ( "" );
      }

      // Iterate over keys again and write them to output buffer. We
      // update the digit lookup table as we go so it always points to
      // where the next value with that digit should go in the output
      // buffer.

      for ( int i = 0; i < size; i++, mem_for() ) {
        int digit = (stage_in[i] >> (stage_idx * radix_lg)) & radix_mask;
        stage_out[ digit_lut[digit]++ ] = stage_in[i];
      }

      // Swap the stage input and output buffers. On the first stage we
      // need to be careful because the input buffer points to the
      // source array which we don't want to overwrite.

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

