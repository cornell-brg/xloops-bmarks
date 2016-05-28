//========================================================================
// quicksort-scalar.cc
//========================================================================
// This is the basic recursive implementation of quicksort. It is not
// clear how it is possibly to directly map this to xloops since there is
// essentially no "loop".

#include "quicksort-scalar.h"

namespace quicksort {

  void quicksort_scalar_h( int *arr, int left_idx, int right_idx )
  {
    if ( left_idx < right_idx ) {

      // Pick a pivot index

      int pivot_idx = left_idx + ( right_idx - left_idx ) / 2;

      // Partition the array using the pivot

      pivot_idx = partition( arr, left_idx, right_idx, pivot_idx );

      // Recurse for left and right of the array

      quicksort_scalar_h( arr, left_idx, pivot_idx - 1 );
      quicksort_scalar_h( arr, pivot_idx + 1, right_idx );

    }
  }

  __attribute__ ((noinline))
  void quicksort_scalar( int* dest, int* src, int size )
  {
    // In-place quicksort, so we copy src to dest

    for ( int i = 0; i < size; i++ )
      dest[i] = src[i];

    quicksort_scalar_h( dest, 0, size-1 );
  }
}
