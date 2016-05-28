//========================================================================
// poly-symm.cc
//========================================================================
// Ported from the PolyBench benchmark suite
//
// Reference:
// ----------
// http://www.cs.ucla.edu/~pouchet/software/polybench/
//
// Description:
// ------------
//
// A benchmark for symm computation.
//
// Dataset:
// --------
//
// We are using standard data set provided by the original benchmark.
//
// Note:
// -----
//
// Need to figure out obtaining a smaller data set.
//

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "symm-common.h"
#include "symm-scalar.h"
#include "symm-xloops.h"
#include "symm-xloops-reg.h"

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

#include "symm.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}
void ordered_for() {}
void mem_for() {}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name,
                     DATA_TYPE dest[], DATA_TYPE ref[], int size )
{
  for ( int i = 0; i < size; i++ ) {
    if ( !( dest[i] == ref[i]) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "dest != ref "
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

typedef void (*impl_func_ptr)(int, int, DATA_TYPE, DATA_TYPE, DATA_TYPE[NI][NJ], DATA_TYPE[NJ][NJ], DATA_TYPE[NI][NJ]);
struct Impl
{
  const char*   str;
  impl_func_ptr func_ptr;
};

Impl impl_table[] =
{
  { "scalar",   &symm::symm_scalar  },
  { "xloops",   &symm::symm_xloops  },
  { "xloops-reg",   &symm::symm_xloops_reg  },
  { "",         0                 },
};

//------------------------------------------------------------------------
// Test harness
//------------------------------------------------------------------------

static void init_array(int ni, int nj,
                       DATA_TYPE *alpha,
                       DATA_TYPE *beta,
                       DATA_TYPE C[NI][NJ],
                       DATA_TYPE A[NJ][NJ],
                       DATA_TYPE B[NI][NJ])
{
  int i, j;

  *alpha = 32412;
  *beta = 2123;
  for (i = 0; i < ni; i++)
    for (j = 0; j < nj; j++) {
      C[i][j] = ((DATA_TYPE) i*j) / ni;
      B[i][j] = ((DATA_TYPE) i*j) / ni;
    }
  for (i = 0; i < nj; i++)
    for (j = 0; j < nj; j++)
      A[i][j] = ((DATA_TYPE) i*j) / ni;
}


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
              "{scalar,xloops,xloops-reg,all}",
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

  DATA_TYPE alpha;
  DATA_TYPE beta;
  DATA_TYPE C[NI][NJ];
  DATA_TYPE A[NJ][NJ];
  DATA_TYPE B[NI][NJ];

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( !impl_all ) {
      init_array (NI, NJ, &alpha, &beta, C, A, B);
      impl_ptr->func_ptr(NI, NJ, alpha, beta, C, A, B);
      verify_results( impl_ptr->str, &C[0][0], C_ref, NI*NJ);
    }
    else {
      Impl* impl_ptr = &impl_table[0];
      while ( impl_ptr->func_ptr != 0 ) {
        init_array (NI, NJ, &alpha, &beta, C, A, B);
        impl_ptr->func_ptr(NI, NJ, alpha, beta, C, A, B);
        verify_results( impl_ptr->str, &C[0][0], C_ref, NI*NJ);
        impl_ptr++;
      }
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute the micro-benchmark
  init_array (NI, NJ, &alpha, &beta, C, A, B);

  if ( warmup ) {
    impl_ptr->func_ptr(NI, NJ, alpha, beta, C, A, B);
  }

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ )
    impl_ptr->func_ptr(NI, NJ, alpha, beta, C, A, B);

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}



