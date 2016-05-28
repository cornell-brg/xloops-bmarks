//========================================================================
// kmeans-xloops.cc
//========================================================================

#include "xloops-common-memset.h"
#include "kmeans-xloops.h"

#include <limits>

extern void unordered_for();
extern void ordered_for();
extern void mem_for();
extern void parallel_for();

namespace kmeans {

  //----------------------------------------------------------------------
  // Phase 0 : Initialize centroids
  //----------------------------------------------------------------------

  void stage0_xloops( float c[], float ex[], int ex_2_cl[],
                      int ex_len, int c_len, int f_len )
  {
    // initialize the centroids to be the first c_len examples
    // TODO: randomize?
    for ( int i = 0; i < c_len*f_len; i++, unordered_for() )
      c[i] = ex[i];

    memset_xloops( ex_2_cl, -1,  ex_len * sizeof( int ) );

  }

  //----------------------------------------------------------------------
  // Phase 1: Assign examples to clusters
  //----------------------------------------------------------------------

  __attribute__ ((noinline))
  float stage1_xloops( float c[], float ex[], int ex_2_cl[],
                       int ex_len, int c_len, int f_len )
  {
    int swap_count = 0;

    // for each example, i
    for ( int i = 0; i < ex_len; i++, ordered_for() ) {

      // compute its distance to each centroid, j
      float min_dist = std::numeric_limits<float>::max();
      int min_idx = -1;
      int eidx = i * f_len;
      for ( int j = 0; j < c_len; j++ ) {

        // by computing euclidian distance along each dimension
        float dist = 0;
        int cidx = j * f_len;
        for ( int k = 0; k < f_len; k++ ) {
          dist += (ex[eidx + k] - c[cidx + k]) *
                  (ex[eidx + k] - c[cidx + k]);
        }

        // determine which centroid is closest, and assign
        // this example to that cluster
        if (dist < min_dist) {
          min_idx = j;
          min_dist = dist;
        }
      }

      // increment the swap count if this example changed clusters
      if (min_idx != ex_2_cl[i]) {
        swap_count++;
        ex_2_cl[i] = min_idx;
      }
    }

    // return the delta
    return ((float) swap_count) / ex_len;
  }

  //----------------------------------------------------------------------
  // Phase 2: Recompute centroids
  //----------------------------------------------------------------------

  void stage2_xloops( float c[], float ex[], int ex_2_cl[],
                      int ex_len, int c_len, int f_len )
  {
    // reset centroid & count arrays (memset wont work on maven-build)
    int count[c_len];
    memset_xloops( count, 0, c_len * sizeof( int ) );
    memset_xloops( c, 0, c_len * f_len * sizeof( int ) );

    // for each example, j
    for ( int j = 0; j < ex_len; j++, mem_for() ) {

      // get its cluster assignment
      int cidx = ex_2_cl[j];
      float * c_ptr = &(c[cidx * f_len]);

      // add the sample to the centroid computation
      int eidx = j * f_len;
      for ( int k = 0; k < f_len; k++ )
        c_ptr[k] += ex[eidx + k];

      // increment the cluster's count (used for normalization)
      count[cidx]++;
    }

    // normalize the centroid
    for ( int i = 0; i < c_len; i++, unordered_for() ) {
      int cidx = i * f_len;
      for ( int k = 0; k < f_len; k++ ) {
        c[cidx + k] /= count[i];
      }
    }
  }

  //----------------------------------------------------------------------
  // Run KMeans
  //----------------------------------------------------------------------

  void kmeans_xloops( float c[], float ex[], int ex_len,
                      int c_len, int f_len, float threshold )
  {
    int ex_2_cl[ex_len];
    float delta;

    // initialize centroids and cluster mapping
    stage0_xloops( c, ex, ex_2_cl, ex_len, c_len, f_len );

    // repeat clustering and recompute centroids until convergence
    do {
      delta = stage1_xloops( c, ex, ex_2_cl, ex_len, c_len, f_len );
      stage2_xloops( c, ex, ex_2_cl, ex_len, c_len, f_len );
    } while (delta > threshold);
  }
}
