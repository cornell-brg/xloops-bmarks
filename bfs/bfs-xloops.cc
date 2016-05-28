//========================================================================
// bfs-xloops.cc
//========================================================================

#include <vector>

#include "bfs-xloops.h"

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

}

namespace bfs{

  //----------------------------------------------------------------------
  // bfs_xloops_kernel
  //----------------------------------------------------------------------
  // note: number of threads is passed in as an external argument as xloops
  // currently cannot query the number of parallel threads and additionally
  // declaring it within the function does not result in a 'bne'
  // instruction as LLVM optimizes the branch instruction

  void bfs_xloops_kernel( int q1[], int q2[], Node graph_nodes[],
      Edge graph_edges[], int color[], int cost[], int num_nodes, int* tail )
  {

    int  new_tail = 0;
    int* tail_ptr = &new_tail;

    int white = WHITE;
    int black = BLACK;

    for ( int tid = 0; tid < num_nodes; tid++, unordered_for() ) {
      int pid  = q1[tid];
      color[pid] = black;

      for ( int i = graph_nodes[pid].x;
                  i < graph_nodes[pid].x + graph_nodes[pid].y; i++ ) {

        int id        = graph_edges[i].x;
        int old_color = fetch_xchg( &color[id], black );

        if ( old_color == white ) {
          fetch_xchg( &cost[id], ( cost[pid] + 1 ) );
          int index = fetch_add( tail_ptr, 1 );
          q2[index] = id;
        }
      }
    }

    *tail = new_tail;

  }

  //----------------------------------------------------------------------
  // bfs_xloops
  //----------------------------------------------------------------------

  void bfs_xloops( Node graph_nodes[], Edge graph_edges[], int color[],
    int cost[], int source, int no_of_nodes ){

    int k    = 0;
    int tail = 1;
    int no_frontier_nodes;

    std::vector<int> q1(no_of_nodes);
    std::vector<int> q2(no_of_nodes);

    q1[0] = source;

    do {

      no_frontier_nodes = tail;
      tail = 0;

      if ( no_frontier_nodes == 0 )
        break;

      if ( k%2 == 0 )
        bfs_xloops_kernel( &q1[0], &q2[0], graph_nodes, graph_edges, color,
          cost, no_frontier_nodes, &tail );
      else
        bfs_xloops_kernel( &q2[0], &q1[0], graph_nodes, graph_edges, color,
          cost, no_frontier_nodes, &tail );

      k++;

    } while ( 1 );

  }

}
