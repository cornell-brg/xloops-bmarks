//========================================================================
// knapsack-xloops.cc
//========================================================================
// xloops implementation of unbounded knapsack problem solved using
// dynamic programming

#include <algorithm>

extern void mem_for();
extern void unordered_for();

namespace knapsack {

  void knapsack_xloops( int *m, int *weights, int *values,
                        int num_items, int max_weight ) {

    // clear the m array first
    for ( int w = 0; w < max_weight; w++ )
      m[w] = 0;

    // solve the knapsack, first loop over all potential weights until max
    // weight, and see if we can add a new item, and if its weight gives
    // us the best value
    // NOTE: flipped the weights and items loops in the xloops
    // implementation to reduce the number of speculative loads and stores
    // in the tagged for.m loop
    for ( int i = 0; i < num_items; i++ ) {

      for ( int w = 0; w < max_weight; w++, mem_for() ) {
        if ( weights[i] <= w ) {
          int curr_val = m[w];
          int val = values[i] + m[ w - weights[i] ];
          if ( val > curr_val )
            m[w] = val;
        }

      }
    }
  }

}
