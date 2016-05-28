//========================================================================
// kmeans
//========================================================================
// This application kernel performs a kmeans clustering algorithm.

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "kmeans-scalar.h"
#include "kmeans-xloops.h"
#include "kmeans-xloops-opt.h"

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

#include "kmeans-color100.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}
void parallel_for() {}
void mem_for() {}
void ordered_for() {}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name, float c[], float ref[], int len )
{
  for ( int i = 0; i < len; i++ ) {
    float diff = c[i] - c_ref[i];
    if ( !(diff < 0.000001 && diff > -0.000001) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "c[" << i << "] != c_ref[" << i << "] "
        << "( " << c[i] << " != " << ref[i] << " )"
        << std::endl;
      return;
    }
  }
  std::cout << "  [ passed ] " << name << std::endl;
}

//------------------------------------------------------------------------
// Test harness
//------------------------------------------------------------------------

// HACK: Destination array to enforce alignment
// float g_c[4*9] __attribute__ ((aligned (32)));
// float g_c[4*9];

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
  pi.add_arg( "--t",       "float", "0.001", "Convergence threshold" );

  pi.add_arg( "--impl", "{scalar,xloops,xloops-opt}", "scalar",
              "Implementation style" );
  pi.add_arg( "--nthreads",  "int", "16",   "Number of threads for xloops opt" );

  pi.parse_args( argc, argv );

  // Set misc local variables from command line

  int  ntrials    = static_cast<int>(pi.get_long("--ntrials"));
  bool verify     = static_cast<bool>(pi.get_flag("--verify"));
  bool stats      = static_cast<bool>(pi.get_flag("--stats"));
  bool warmup     = static_cast<bool>(pi.get_flag("--warmup"));
  int  nthreads   = static_cast<int>(pi.get_long("--nthreads"));
//  float threshold = static_cast<float>(pi.get_flag("--t"));

  // pinfo does not interpret floats correctly from command line, set to
  // 0 by default and cannot be overloaded.

  float threshold = 0.001;

  // Setup implementation

  enum Impl
  {
    IMPL_SCALAR,
    IMPL_MT,
    IMPL_SIMT,
    IMPL_XLOOPS,
    IMPL_XLOOPS_OPT,
    IMPL_ALL,
  };

  Impl impl;
  const char* impl_str = pi.get_string("--impl");
  if ( strcmp( impl_str, "scalar" ) == 0 )
    impl = IMPL_SCALAR;
  else if ( strcmp( impl_str, "xloops" ) == 0 )
    impl = IMPL_XLOOPS;
  else if ( strcmp( impl_str, "xloops-opt" ) == 0 )
    impl = IMPL_XLOOPS_OPT;
  else if ( strcmp( impl_str, "all" ) == 0 ) {
    impl = IMPL_ALL;
    if ( !verify ) {
      std::cout
        << "\n ERROR: --impl all only valid for verification\n"
        << std::endl;
      return 1;
    }
  }
  else {
    std::cout
      << "\n ERROR: Unrecognized implementation "
      << "(" << impl_str << ")\n" << std::endl;
    return 1;
  }

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( (impl == IMPL_ALL) || (impl == IMPL_SCALAR) ) {
      std::vector<float> c(c_len*f_len);
      kmeans::kmeans_scalar( &c[0], ex, ex_len, c_len, f_len, threshold );
      verify_results( "scalar", &c[0], c_ref, c_len*f_len);
    }
    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS_OPT) ) {
      std::vector<float> c(c_len*f_len);
      kmeans::kmeans_xloops_opt( &c[0], ex, ex_len, c_len, f_len, threshold,
                                                            nthreads );
      verify_results( "xloops-opt", &c[0], c_ref, c_len*f_len);
    }
    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS) ) {
      std::vector<float> c(c_len*f_len);
      kmeans::kmeans_xloops( &c[0], ex, ex_len, c_len, f_len, threshold );
      verify_results( "xloops", &c[0], c_ref, c_len*f_len);
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute kmeans

  std::vector<float> c(c_len*f_len);

  if ( warmup ) {
    if ( impl == IMPL_SCALAR )
      kmeans::kmeans_scalar( &c[0], ex, ex_len, c_len, f_len, threshold );

    else if ( impl == IMPL_XLOOPS_OPT )
      kmeans::kmeans_xloops_opt( &c[0], ex, ex_len, c_len, f_len, threshold,
                                                            nthreads );

    else if ( impl == IMPL_XLOOPS )
      kmeans::kmeans_xloops( &c[0], ex, ex_len, c_len, f_len, threshold );
  }

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ ) {

    if ( impl == IMPL_SCALAR )
      kmeans::kmeans_scalar( &c[0], ex, ex_len, c_len, f_len, threshold );

    else if ( impl == IMPL_XLOOPS_OPT )
      kmeans::kmeans_xloops_opt( &c[0], ex, ex_len, c_len, f_len, threshold,
                                                            nthreads );

    else if ( impl == IMPL_XLOOPS )
      kmeans::kmeans_xloops( &c[0], ex, ex_len, c_len, f_len, threshold );
  }

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

