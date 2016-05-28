//========================================================================
// quicksort-common.h
//========================================================================
// Common utilities for quicksort implementations

#ifndef QUICKSORT_COMMON_H
#define QUICKSORT_COMMON_H

namespace quicksort {

  //------------------------------------------------------------------------
  // Basic Helper Functions
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

  inline void swap( int *arr, int idx0, int idx1 ) {
    int tmp = arr[idx0];
    arr[idx0] = arr[idx1];
    arr[idx1] = tmp;
  }

  int partition( int *arr, int left_idx, int right_idx, int pivot_idx )
  {

    int pivot = arr[pivot_idx];

    // Move the pivot to the right

    swap( arr, pivot_idx, right_idx );

    int store_idx = left_idx;

    // Swap elements that are less than pivot

    for ( int i = left_idx; i < right_idx; i++ )
      if ( arr[i] <= pivot )
        swap( arr, i, store_idx++ );

    // Swap back the pivot

    swap( arr, store_idx, right_idx );
    return store_idx;
  }

  //------------------------------------------------------------------------
  // WorkItem Class
  //------------------------------------------------------------------------
  // Work items in the work list basically consist of a pair of indices
  // into the array that indicate the beginning and end of that partition.
  // Note that we converted this to a POD array so that we can compile
  // this with LLVM which doesn't support stack-allocated non-POD arrays.

  class WorkItem {
   public:

    int left_idx;
    int right_idx;

  };


  }

}

#endif /* QUICKSORT_COMMON_H */
