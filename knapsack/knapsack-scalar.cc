//========================================================================
// knapsack-scalar.cc
//========================================================================
// scalar implementation of unbounded knapsack problem solved using
// dynamic programming

#include <algorithm>

namespace knapsack {

  void knapsack_scalar( int *m, int *weights, int *values,
                        int num_items, int max_weight ) {

    // clear the m array first
    for ( int w = 0; w < max_weight; w++ )
      m[w] = 0;

    // solve the knapsack, first loop over all potential weights until max
    // weight, and see if we can add a new item, and if its weight gives
    // us the best value
    for ( int w = 0; w < max_weight; w++ ) {
      int max_val = 0;
      for ( int i = 0; i < num_items; i++ ) {
        if ( weights[i] <= w )
          max_val = std::max( max_val, values[i] + m[ w - weights[i] ] );
      }
      if ( max_val > 0 )
        m[w] = max_val;
    }
  }

}
