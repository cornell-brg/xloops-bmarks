//========================================================================
// bintree.cc
//========================================================================
// Constructs a binary tree from a random input array

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "bintree-scalar.h"
#include "bintree-xloops.h"

#include <cstring>
#include <iostream>
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

#include "bintree.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}
void ordered_for() {}
void mem_for() {}

void print_tree( bintree::Node *tree, int level ) {
  if ( tree != NULL ) {
    std::cout << "  " << tree->value << "," << std::endl;
    //std::cout << "level: " << level << " val: " << tree->value << std::endl;
    print_tree( tree->left,  level+1 );
    print_tree( tree->right, level+1 );
  }
}

void get_preorder( bintree::Node *tree, int *preorder, int &i ) {
  if ( tree != NULL ) {
    preorder[i++] = tree->value;
    get_preorder( tree->left,  preorder, i );
    get_preorder( tree->right, preorder, i );
  }
}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name,
                     int *dest, int *ref, int size )
{
  for ( int i = 0; i < size; i++ ) {
    if ( dest[i] != ref[i] ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "dest[ " << i << " ] != ref[ " << i << " ] "
        << "( " << dest[i] << " != " << ref[i] << " )"
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

typedef void (*impl_func_ptr)(int, int *, bintree::Node *);
struct Impl
{
  const char*   str;
  impl_func_ptr func_ptr;
};

Impl impl_table[] =
{
  { "scalar",   &bintree::bintree_scalar  },
  { "xloops",   &bintree::bintree_xloops  },
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
              "{scalar,xloops,all}",
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

  int n = 1000;
  bintree::Node *tree = new bintree::Node[n];
  int preorder[n];

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( !impl_all ) {
      impl_ptr->func_ptr(n, A, tree);
      int i = 0;
      memset( preorder, 0, sizeof(int) * n );
      get_preorder( tree, preorder, i );
      verify_results( impl_ptr->str, preorder, preorder_ref, n);
    }
    else {
      Impl* impl_ptr = &impl_table[0];
      while ( impl_ptr->func_ptr != 0 ) {
        impl_ptr->func_ptr(n, A, tree);
        int i = 0;
        memset( preorder, 0, sizeof(int) * n );
        get_preorder( tree, preorder, i );
        verify_results( impl_ptr->str, preorder, preorder_ref, n);
        impl_ptr++;
      }
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute the micro-benchmark

  if ( warmup ) {
    impl_ptr->func_ptr(n, A, tree);
  }

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ )
    impl_ptr->func_ptr(n, A, tree);

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}


