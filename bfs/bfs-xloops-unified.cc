//========================================================================
// bfs-xloops.cc
//========================================================================

#include <vector>

#include "bfs-xloops-unified.h"

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

  inline int fetch_xchg( volatile int* ptr, int value )
  {
    #ifdef _MIPS_ARCH_MAVEN
      int res;
      __asm__ volatile
        ( "amo.xchg %0, %1, %2"
          : "=r"(res) : "r"(ptr), "r"(value) : "memory" );
      return res;
    #else
      int temp = *ptr;
      *ptr = value;
      return temp;
    #endif
  }

  inline int fetch_min( volatile int* ptr, int value )
  {
    #ifdef _MIPS_ARCH_MAVEN
      int res;
      __asm__ volatile
        ( "amo.min %0, %1, %2"
          : "=r"(res) : "r"(ptr), "r"(value) : "memory" );
      return res;
    #else
      int temp = *ptr;
      *ptr = std::min( temp, value );
      return temp;
    #endif
  }

}

namespace bfs{

  //----------------------------------------------------------------------
  // bfs_xloops_unified
  //----------------------------------------------------------------------
  // This version supports a more dynamic form of parallelism using a
  // single work list. A single for loop iterates over all items in the
  // work list, but the key point is that we can add items to the end of
  // this work list even as we are processing work items earlier in the
  // work list.
  // Like the quicksort-xloops-unified, This scheme can potentially enable
  // much better load balancing, but it requires support for dynamically
  // increasing the for loop bound.

  __attribute__((noinline))
  void bfs_xloops_unified( Node h_graph_nodes[], Edge h_graph_edges[],
	                 int color[], int h_cost[], int source, int no_of_nodes ) {

    int node_queue[no_of_nodes];

    node_queue[0] = source;
    int num_queue_items = 1;

    int tail = 1;
    int *tail_ptr = &tail;

    for ( int j = 0; j < num_queue_items; j++, unordered_for() ) {

      // using a dummy value for j ensures j stays in a single register
      // instead of two

      int j_;
      #ifdef _MIPS_ARCH_MAVEN
      __asm__ volatile ( "addiu %0, %1, 0" : "=r" (j_) : "r" (j) );
      #endif
      int node_idx = node_queue[j_];

		  for ( int i = 0; i < h_graph_nodes[node_idx].y; i++ ) {

        int edge_idx = i + h_graph_nodes[ node_idx ].x;
			  int target_node_idx = h_graph_edges[ edge_idx ].x;

        // we compare the old value of the cost to determine if we have
        // chenged the cost

        int new_cost = h_cost[ node_idx ] + 1;

        if ( fetch_min( &h_cost[ target_node_idx ], new_cost ) > new_cost ) {

          int index = fetch_add( tail_ptr, 1 );
          node_queue[ index ] = target_node_idx;

          // this is to prevent compiler from rearranging the bump before
          // the atomic op

          __asm__ volatile ( "" : : "r" (num_queue_items) );

          num_queue_items++;
        }

		  }
	  }
	}

}
