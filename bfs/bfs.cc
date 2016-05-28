//========================================================================
// bfs
//========================================================================
// Ported from the PARBOIL benchmark suite
//
// Reference:
// ----------
//
// Background: "Introduction to Algorithms" textbook
//
// Serial version: PARBOIL base
//
// CUDA version: PARBOIL cuda
// Implementing Breadth first search on CUDA using algorithm given in
// DAC'10 paper "An Effective GPU Implementation of Breadth-First Search"
//
// Description:
// ------------
//
// A benchmark for a Breadth-First Search. It reads a near regular graph as
// input and produces the cost (distance from source) of each node in the
// input graph.
//
// Dataset:
// --------
//
// This kernel is present in both the Rodinia and PARBOIL benchmark suite
// which is based on the IIIT-BFS CUDA implementation. Current dataset has
// been borrowed from the Rodinia benchmark suite (graph4096.txt) as it was
// the smallest available dataset.
//
// Note:
// -----
//
// Need to figure out obtaining a smaller data set. The graph
// representation in the data sets have been modified to cater to the GPU
// implementation.

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "bfs-common.h"
#include "bfs-scalar.h"
#include "bfs-xloops.h"
#include "bfs-xloops-unified.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <iomanip>
#include <vector>

//------------------------------------------------------------------------
// Support for enabling stats
//------------------------------------------------------------------------

#ifdef _MIPS_ARCH_MAVEN
#include <machine/cop0.h>
#define set_stats_en( value_ ) \
  maven_set_cop0_stats_enable( value_ );
#else
#define set_stats_en( value_ )
#endif

//------------------------------------------------------------------------
// Include input and reference data
//------------------------------------------------------------------------

#include "bfs-src.dat"
#include "bfs-ref.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name,
                     int dest[], int ref[], int size )
{
  for ( int i = 0; i < size; i++ ) {
    if ( !( dest[i] == ref[i] ) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "dest[ " << i << " ] != ref[ " << i << " ] "
        << "( " << dest[i] << " != " << ref[i] << " )"
        << " addr: " << std::hex << &dest[i]
        << std::endl;
      return;
    }
  }
  std::cout << "  [ passed ] " << name << std::endl;
}

//------------------------------------------------------------------------
// Implementation Table
//------------------------------------------------------------------------
// This table contains one structure for each of the above
// implementations. Each structure includes the name of the
// implementation used for command line parsing and verification output,
// and a function pointer to the actual implementation function.

typedef void (*impl_func_ptr)(bfs::Node[],bfs::Edge[],int[],int[],int,int);
struct Impl
{
  const char*   str;
  impl_func_ptr func_ptr;
};

Impl impl_table[] =
{
  { "scalar",   &bfs::bfs_scalar  },
  { "xloops",   &bfs::bfs_xloops  },
  { "xloops-unified",   &bfs::bfs_xloops_unified  },
  { "",         0                 },
};

//------------------------------------------------------------------------
// Test harness
//------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
  // Handle any uncaught exceptions

  stdx::set_terminate();

  // Command line options

  pinfo::ProgramInfo pi;

  pi.add_arg( "--ntrials",  "int",  "1",     "Number of trials" );
  pi.add_arg( "--verify",   NULL,   NULL,    "Verify an implementation" );
  pi.add_arg( "--stats",    NULL,   NULL,    "Ouptut stats about run" );
  pi.add_arg( "--warmup",   NULL,   NULL,    "Warmup the cache" );

  pi.add_arg( "--impl",
              "{scalar,xloops,xloops-unified,all}",
              "scalar", "Implementation style" );

  pi.parse_args( argc, argv );

  // Set misc local variables from command line

  int  ntrials    = static_cast<int>(pi.get_long("--ntrials"));
  bool verify     = static_cast<bool>(pi.get_flag("--verify"));
  bool stats      = static_cast<bool>(pi.get_flag("--stats"));
  bool warmup     = static_cast<bool>(pi.get_flag("--warmup"));

  // Setup implementation

  const char* impl_str = pi.get_string("--impl");
  bool impl_all = ( strcmp( impl_str, "all" ) == 0 );
  Impl* impl_ptr = &impl_table[0];
  while ( !impl_all && (impl_ptr->func_ptr != 0) ) {
    if ( strcmp( impl_str, impl_ptr->str ) == 0 )
      break;
    impl_ptr++;
  }

  if ( impl_all && !verify ) {
    std::cout
      << "\n ERROR: --impl all only valid for verification\n"
      << std::endl;
    return 1;
  }

  if ( !impl_all && (impl_ptr->func_ptr == 0) ) {
    std::cout
      << "\n ERROR: Unrecognized implementation "
      << "(" << impl_str << ")\n" << std::endl;
    return 1;
  }

  std::vector<int>  color(no_of_nodes, WHITE);
  std::vector<int>  h_cost(no_of_nodes, INF);

  for (int i = 0; i < no_of_nodes; i++) {
    h_cost[i] = INF;
    color[i] = WHITE;
  }
  h_cost[source] = 0;

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( !impl_all ) {
      h_cost[source] = 0;
      impl_ptr->func_ptr( &h_graph_nodes[0], &h_graph_edges[0],
                          &color[0], &h_cost[0], source, no_of_nodes );
      verify_results( impl_ptr->str, &h_cost[0], &ref[0], no_of_nodes );
    }
    else {
      Impl* impl_ptr = &impl_table[0];
      while ( impl_ptr->func_ptr != 0 ) {
        // reset the values of cost and color so that values don't
        // propagate
        for (int i = 0; i < no_of_nodes; i++) {
          h_cost[i] = INF;
          color[i] = WHITE;
        }
        h_cost[source] = 0;
        impl_ptr->func_ptr( &h_graph_nodes[0], &h_graph_edges[0],
                            &color[0], &h_cost[0], source, no_of_nodes );
        verify_results( impl_ptr->str, &h_cost[0], &ref[0], no_of_nodes );
        impl_ptr++;
      }
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute the micro-benchmark

  if ( warmup ) {
    impl_ptr->func_ptr( &h_graph_nodes[0], &h_graph_edges[0],
                        &color[0], &h_cost[0], source, no_of_nodes );
    for ( int i = 0; i < no_of_nodes; i++ ) {
      h_cost[i] = INF;
      color[i] = WHITE;
    }
  }
  h_cost[source] = 0;

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ )
    impl_ptr->func_ptr( &h_graph_nodes[0], &h_graph_edges[0],
                        &color[0], &h_cost[0], source, no_of_nodes );

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

