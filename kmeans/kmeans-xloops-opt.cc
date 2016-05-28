//========================================================================
// kmeans-xloops-opt.cc
//========================================================================
// shreesha: XLOOPS implementation where an alternative algorithm is used
// for better performance. Illustrates app tuning for XLOOPS architectures

#include <limits>
#include <algorithm>
#include <iostream>

#include "kmeans-xloops-opt.h"

extern void unordered_for();

// hacky empty asm blocks to make sure the code looks as expected
#ifdef _MIPS_ARCH_MAVEN
#define HACK_ASM_BLOCK __asm__( "" );
#else
#define HACK_ASM_BLOCK
#endif

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

namespace kmeans {

  //----------------------------------------------------------------------
  // Phase 0 : Initialize centroids
  //----------------------------------------------------------------------

  void stage0_xloops_opt( float c[], float ex[], int ex_2_cl[],
                  int ex_len, int c_len, int f_len )
  {
    // initialize the centroids to be the first c_len examples
    // TODO: randomize?
    for ( int i = 0; i < c_len*f_len; i++, unordered_for() ) {
      c[i] = ex[i];
    }
    // initialize the example to cluster mappings to -1
    for ( int i = 0; i < ex_len; i++, unordered_for() ) {
      // hack to prevent llvm to turn this into a memset
      HACK_ASM_BLOCK
      ex_2_cl[i] = -1;
    }
  }

  //----------------------------------------------------------------------
  // Phase 1: Assign examples to clusters
  //----------------------------------------------------------------------

  float stage1_xloops_opt( float c[], float ex[], int ex_2_cl[],
                   int ex_len, int c_len, int f_len )
  {
    int swap_count = 0;

    for ( int i = 0; i < ex_len; i++, unordered_for() ) {
      // compute its distance to each centroid, j
      float min_dist = std::numeric_limits<float>::max();
      int min_idx = -1;
      int eidx = i * f_len;
      for ( int j = 0; j < c_len; j++ ) {
        // hack: we need each an every one of these asm blocks to ensure
        // llvm doesn't optimize this loop away. i tried as many of these
        // asm blocks, this is the absolute minimum number...
        HACK_ASM_BLOCK

        // by computing euclidian distance along each dimension
        float dist = 0;
        int cidx = j * f_len;
        for ( int k = 0; k < f_len; k++ ) {
          HACK_ASM_BLOCK
          dist += (ex[eidx + k] - c[cidx + k]) *
                  (ex[eidx + k] - c[cidx + k]);
          HACK_ASM_BLOCK
        }

        // determine which centroid is closest, and assign
        // this example to that cluster
        if (dist < min_dist) {
          HACK_ASM_BLOCK
          min_idx = j;
          min_dist = dist;
          HACK_ASM_BLOCK
        }
      }

      // atomically increment swap count if this example changed clusters
      if (min_idx != ex_2_cl[i]) {
        fetch_add( &swap_count, 1 );
        ex_2_cl[i] = min_idx;
      }
    }

    // return the delta
    return ((float) swap_count) / ex_len;
  }

  //----------------------------------------------------------------------
  // Phase 2: Recompute centroids
  //----------------------------------------------------------------------

  void stage2_xloops_opt( float c[], float ex[], int ex_2_cl[],
                  int ex_len, int c_len, int f_len, int nthreads )
  {

    //--------------------------------------------------------------------
    // Initialization
    //--------------------------------------------------------------------
    // In order to avoid the need for locks or atomic ops, each
    // thread performs computations in a local scratchpad.
    // The localc[] and the count[] arrays represent these scratchpads.
    // Once all the examples have been processed, a reduce phase is
    // performed to merge the results and normalize the centroids.
    // reset centroid & count arrays (memset wont work on maven-build)

    // the count array keeps track of how many examples are assigned to
    // each cluster, each thread is assigned its own local array space
    int count_[c_len*nthreads];
    int * count = &(count_[0]);

    // the localc array is a scratchpad for iterative centroid
    // computation, each thread is assigned its own dedicated (local)
    // array space
    float localc_[c_len * f_len * nthreads];
    float * localc = &(localc_[0]);

    // TODO: the following will turn into memset's. we need to implement
    // memset_xloops, an optimized version of memset that uses for.u
    // instructions and use them here
    for ( int i = 0; i < c_len * f_len * nthreads; i++ ) {
      localc[i] = 0;
    }

    for ( int i = 0; i < c_len * nthreads; i++ ) {
      count[i] = 0;
    }

    //--------------------------------------------------------------------
    // Recompute centroids (local)
    //--------------------------------------------------------------------

    // statically distribute work among the lanes
    int range_size = (ex_len + nthreads - 1) / nthreads;

    // create the ranges for each thread
    int range_begin[ nthreads ];
    int range_end[ nthreads ];

    // create the ranges for each thread
    for ( int i = 0; i < nthreads; i++ ) {
      range_begin[i] = i * range_size;
      range_end[i] = std::min( range_begin[i] + range_size, ex_len );
    }

    // for each example, j
    for ( int loop_idx = 0; loop_idx < nthreads; loop_idx++, unordered_for() )
    {
      int c_offset     = loop_idx * c_len * f_len;
      int count_offset =  loop_idx * c_len;

      //for ( int j = 0; j < ex_len; j++ )
      for ( int j = range_begin[loop_idx]; j < range_end[loop_idx]; j++ ) {

        // get its cluster assignment
        int cidx = ex_2_cl[j];
        float * c_ptr = &(localc[cidx * f_len + c_offset]);

        // add the sample to the centroid computation
        int eidx = j * f_len;
        for ( int k = 0; k < f_len; k++ )
          c_ptr[k] += ex[eidx + k];

        // increment the cluster's count (used for normalization)
        count[cidx + count_offset]++;
      }
    }

    //--------------------------------------------------------------------
    // Reduction and normalization
    //--------------------------------------------------------------------

    for ( int i = 0; i < c_len; i++, unordered_for() ) {
        // for each thread, aggregate the local cluster counts
        // into count[] belonging to thread 0 (so start at thread 1)
        for ( int h = 1; h < nthreads; h++ ) {
          count[i] += count[h * c_len + i];
        }

        // for each thread, aggregate the local centroid computations
        // into localc[] belonging to thread 0 (so start at thread 1)
        int cidx = i * f_len;
        for ( int h = 1; h < nthreads; h++ ) {
          int c_offset = h * c_len * f_len;
          for ( int j = 0; j < f_len; j++ ) {
            localc[cidx + j] += localc[c_offset + cidx + j];
          }
        }

        // normalize each feature of the aggregated centroid result
        // store the normalized feature to the c array
        for ( int k = 0; k < f_len; k++ ) {
          c[cidx + k] = localc[cidx + k] / count[i];
        }
    }
  }

  //----------------------------------------------------------------------
  // Run KMeans
  //----------------------------------------------------------------------

  void kmeans_xloops_opt( float c[], float ex[], int ex_len,
                  int c_len, int f_len, float threshold, int nthreads )
  {
    int ex_2_cl[ex_len];
    float delta;

    // initialize centroids and cluster mapping
    stage0_xloops_opt( c, ex, ex_2_cl, ex_len, c_len, f_len );

    // repeat clustering and recompute centroids until convergence
    do {
      delta = stage1_xloops_opt( c, ex, ex_2_cl, ex_len, c_len, f_len );
      // TODO: use the following once we can reliably pass nthreads from
      // kmeans.cc
      //stage2_xloops_opt( c, ex, ex_2_cl, ex_len, c_len, f_len, nthreads );
      stage2_xloops_opt( c, ex, ex_2_cl, ex_len, c_len, f_len, 4 );


    } while (delta > threshold);
  }

}

