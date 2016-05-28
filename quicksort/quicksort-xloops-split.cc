//========================================================================
// quicksort-xloops-split.cc
//========================================================================
// This version parallelizes across the frontier using two work lists: an
// input work list and an output worklist. The inner for loop iterates
// over the work items in the input work list, processes these work
// items, and (potentially) adds new work to the output work list. After
// the inner for loop is finished we essentially swap the input and
// output work lists, clear the output work list, and go through
// processing the input work list again. The inner for loop could be an
// xfor.u. We do not need any synchronization when pulling from the input
// work list since each iteration will be effectively processing an
// independent work item. We do need synchronization when pushing to the
// output work list since multiple iterations can be doing this as the
// same time. We can use "work chunking" so we only need a single atomic
// memory operation. This scheme is simple, but it requires a barrier
// after each inner for loop which can inhibit load balancing.

#include "quicksort-xloops-split.h"

extern void unordered_for();

namespace quicksort {

  __attribute__ ((noinline))
  void quicksort_xloops_split( int* dest, int* src, int size )
  {

    // In-place quicksort, so we copy src to dest. This is an example of an
    // xfor.u where the bound (i.e., size) does not change.

    for ( int i = 0; i < size; i++ /* xfor.u */ )
      dest[i] = src[i];

    // Create two worklists

    WorkItem work_list_a[size];
    WorkItem work_list_b[size];

    WorkItem* work_list_in = &work_list_a[0];
    int work_list_in_size  = 0;

    WorkItem* work_list_out = &work_list_b[0];
    int work_list_out_size = 0;

    // Add first work item to the input work list

    work_list_in[0].left_idx = 0;
    work_list_in[0].right_idx = size-1;

    work_list_in_size = 1;

    while ( work_list_in_size != 0 ) {

      // Iterate over the split_work_list. This is an example of an xfor.u where
      // the bound (i.e., work_list_in_size) does not change while we are
      // executing this loop..

      for ( int i = 0; i < work_list_in_size; i++, unordered_for() ) {

        // Get work item from the input work list. No need for any kind of
        // synchronization; every iteration will get a unique work item.

        WorkItem work_item = work_list_in[i];
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

          // Use an atomic memory operation to reserve entries in work
          // list. Note that by calculating how many we need we can use a
          // single atomic memory operation instead of two atomic memory
          // operations which might be slightly more efficient.

          int push_idx = fetch_add( &work_list_out_size, num_new_work_items );

          // Add the left and right partitions to work list if they are
          // valid.

          if ( left_idx < pivot_idx - 1 ) {
            work_list_out[push_idx].left_idx = left_idx;
            work_list_out[push_idx++].right_idx = pivot_idx - 1;
          }

          if ( pivot_idx + 1 < right_idx ) {
            work_list_out[push_idx].left_idx = pivot_idx + 1;
            work_list_out[push_idx++].right_idx = right_idx;
          }

        }
      }

      // Swap the input and output work lists

      WorkItem* work_list_temp = &work_list_in[0];
      work_list_in  = work_list_out;
      work_list_out = work_list_temp;

      work_list_in_size  = work_list_out_size;
      work_list_out_size = 0;

    }

  }

}
