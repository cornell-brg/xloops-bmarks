//========================================================================
// knapsack
//========================================================================
// This app solves the unbounded knapsack problem, which is about
// maximizing the value that can be carried in a knapsack that has a
// limited weight capacity by picking items that have a certain weight and
// value. The unbounded problem, as opposed to bounded (1/0) problem,
// assumes we can pick as many of each item to fill up the knapsack. This
// is an NP-complete problem and the best solution uses dynamic
// programming.

#include "pinfo.h"

#include "knapsack-scalar.h"
#include "knapsack-xloops.h"

#include "stdx-Timer.h"
#include "stdx-FstreamUtils.h"
#include "stdx-Exception.h"

#include <cstring>
#include <iostream>
#include <vector>
#include <cmath>

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
// Include dataset
//------------------------------------------------------------------------

#include "knapsack.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void mem_for() {}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name,
                     int dest[], int ref[], int size )
{
  for ( int i = 0; i < size; i++ ) {
    if ( dest[i] != ref[i] ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "dest[" << i << "] != ref[" << i << "] "
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

typedef void (*impl_func_ptr)(int*,int*,int*,int,int);
struct Impl
{
  const char*   str;
  impl_func_ptr func_ptr;
};

Impl impl_table[] =
{
  { "scalar",       &knapsack::knapsack_scalar  },
  { "xloops",       &knapsack::knapsack_xloops  },
  { "",             0                    },
};

//------------------------------------------------------------------------
// Test Harness
//------------------------------------------------------------------------

// HACK: Destination array to enforce alignment
// int g_dest[1000] __attribute__ ((aligned (32)));
// int g_dest[1000];

int main( int argc, char* argv[] )
{
  // Handle any uncaught exceptions

  stdx::set_terminate();

  // Command line options

  pinfo::ProgramInfo pi;

  pi.add_arg( "--ntrials",  "int", "1",    "Number of trials" );
  pi.add_arg( "--verify",   NULL,  NULL,   "Verify an implementation" );
  pi.add_arg( "--stats",    NULL,  NULL,   "Ouptut stats about run" );
  pi.add_arg( "--warmup",   NULL,  NULL,   "Warmup the cache" );

  std::string impl_list_str;
  Impl* impl_ptr_tmp = &impl_table[0];
  impl_list_str += "{";
  impl_list_str += impl_ptr_tmp->str;
  impl_ptr_tmp++;
  while ( impl_ptr_tmp->func_ptr != 0 ) {
    impl_list_str += ",";
    impl_list_str += impl_ptr_tmp->str;
    impl_ptr_tmp++;
  }
  impl_list_str += "}";

  pi.add_arg( "--impl", impl_list_str.c_str(),
              "scalar", "Implementation style" );
  pi.add_arg( "--weights", "{small,large}",
              "large", "Weights of the items, large > 10" );

  pi.parse_args( argc, argv );

  // Set misc local variables from command line

  int  ntrials  = static_cast<int>(pi.get_long("--ntrials"));
  bool verify   = static_cast<bool>(pi.get_flag("--verify"));
  bool stats    = static_cast<bool>(pi.get_flag("--stats"));
  bool warmup   = static_cast<bool>(pi.get_flag("--warmup"));

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

  // Pick the weights and values

  const char *weights_str = pi.get_string("--weights");

  int *weights, *values, *ref;

  if ( !strcmp( weights_str, "small" ) ) {
    weights = small_weight_arr;
    values  = small_value_arr;
    ref     = small_ref;
  } else if ( !strcmp( weights_str, "large" ) ) {
    weights = large_weight_arr;
    values  = large_value_arr;
    ref     = large_ref;
  } else {
    std::cout
      << "\n ERROR: Unrecognized weight "
      << "(" << weights_str << ")\n" << std::endl;
    return 1;
  }

  // Initialize the m array

  int m[max_weight];

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( !impl_all ) {
      impl_ptr->func_ptr( m, weights, values, num_items, max_weight );
      verify_results( impl_ptr->str, m, ref, max_weight );
    }
    else {
      Impl* impl_ptr = &impl_table[0];
      while ( impl_ptr->func_ptr != 0 ) {
        impl_ptr->func_ptr( m, weights, values, num_items, max_weight );
        verify_results( impl_ptr->str, m, ref, max_weight );
        impl_ptr++;
      }
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute the micro-benchmark

  if ( warmup )
    impl_ptr->func_ptr( m, weights, values, num_items, max_weight );

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ )
    impl_ptr->func_ptr( m, weights, values, num_items, max_weight );

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

