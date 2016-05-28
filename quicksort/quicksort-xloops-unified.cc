//========================================================================
// quicksort-xloops-unified.cc
//========================================================================
// This version supports a more dynamic form of parallelism using a
// single work list. A single for loop iterates over all items in the
// work list, but the key point is that we can add items to the end of
// this work list even as we are processing work items earlier in the
// work list. If we are carefuly, we can potentially avoid any
// synchronization when popping work items off of the work list. We do
// need synchronization when pushing to the output work list since
// multiple iteartions can be doing this as the same time. We can use
// "work chunking" so we only need a single atomic memory operation. We
// must carefully update the variable used as the bound for the for loop;
// we do not want to update it until we have finished pushing the items
// onto the work list. This scheme can potentially enable much better
// load balancing, but it requires support for dynamically increasing the
// for loop bound.

#include "quicksort-xloops-unified.h"
#include <vector>

extern void unordered_for();

namespace quicksort {


  __attribute__ ((noinline))
  void quicksort_xloops_unified( int* dest, int* src, int size )
  {
    // In-place quicksort, so we copy src to dest. This is an example of an
    // xfor.u where the bound (i.e., size) does not change.

    for ( int i = 0; i < size; i++ /* xfor.u */ )
      dest[i] = src[i];

    // Create a worklist

    WorkItem unified_work_list[size];
    //std::vector<WorkItem> unified_work_list(size);

    int unified_work_list_size = 0;
    int unified_work_list_push_idx = 0;

    // Add first work item to the work list

    unified_work_list[0].left_idx = 0;
    unified_work_list[0].right_idx = size-1;
    unified_work_list_size = 1;
    unified_work_list_push_idx = 1;

    // Iterate until there is no work left. This is an example of an xfor.u
    // where the bound (i.e., unified_work_list_size) monotonically
    // increases.
    //register int i asm ("$7");
    int j = 0;

    for ( int i = 0; i < unified_work_list_size; i++, unordered_for() ) {

      // hack: the compiler currently uses two different induction
      // variables, so it creates an illegal register dependency over the
      // reads for i. We use another variable j and use inline assembly to
      // copy i's value
      #ifdef _MIPS_ARCH_MAVEN
      __asm__ volatile ( "addiu %0, %1, 0" : "=r"(j) : "r"(i) );
      #else
      j = i;
      #endif

      // Get work item from the work list. We could use some kind of pop to
      // get the next item off of the work list, but then this would
      // require an atomic memory operation. We can instead simply use our
      // iteration index, since we know that no other iteration will be
      // working on this specific work item. If we implemented the work
      // list with a dynamically resizing vector or a linked list then we
      // would need to use some kind of synchronization.
      WorkItem work_item = unified_work_list[j];
      int left_idx  = work_item.left_idx;
      int right_idx = work_item.right_idx;

      // Pick a pivot index

      int pivot_idx = left_idx + ( right_idx - left_idx ) / 2;

      // Partition the array using the pivot

      pivot_idx = partition( dest, left_idx, right_idx, pivot_idx );

      // Determine how many entries we want to add to the work list.

      int num_new_work_items = 0;

      if ( left_idx < pivot_idx - 1 )
        num_new_work_items++;

      if ( pivot_idx + 1 < right_idx )
        num_new_work_items++;

      // If we do not need to add any new work items, then we are done

      if ( num_new_work_items > 0 ) {

        // Use an atomic memory operation to reserve entries in work list.
        // Note that by calculating how many we need we can use a single
        // atomic memory operation instead of two atomic memory operations
        // which might be slightly more efficient.

        int push_idx = fetch_add( &unified_work_list_push_idx,
                                  num_new_work_items );

        // Add the left and right partitions to work list if they are
        // valid.

        if ( left_idx < pivot_idx - 1 ) {
          unified_work_list[push_idx].left_idx = left_idx;
          unified_work_list[push_idx++].right_idx = pivot_idx - 1;
        }

        if ( pivot_idx + 1 < right_idx ) {
          unified_work_list[push_idx].left_idx = pivot_idx + 1;
          unified_work_list[push_idx++].right_idx = right_idx;
        }

        unified_work_list_size += num_new_work_items;

      }
    }

  }

}
