//========================================================================
// mibench-adpcmdec
//========================================================================
// Ported from the MiBench Suite
//
// Reference:
// ----------
//
// Guthaus, Matthew R., et al. "MiBench: A free, commercially
// representative embedded benchmark suite." Workload Characterization,
// 2001. WWC-4. 2001 IEEE International Workshop on. IEEE, 2001.
//
// URL:
// ----
//
// http://www.eecs.umich.edu/mibench/
//
// Description:
// ------------
//
// The application kernel captures the computation pattern seen in ADPCM
// decompression algorithms where the step size of the digital quantization
// is adaptively selected based on the signal amplitude iteself. In each
// iteration of the loop a new input value is combined with previous
// history to determine the step size for the reconstructed output.
//
// Dataset:
// --------
//
// Dataset is derived from 'small.adpcm' present in MiBench ADPCM suite.
// First ten thousand bytes of input compressed signal comprises the
// source data.
//
// NOTE:
// -----
//
// MT, SIMT implementations are not implemented as there is not much
// data-parallelism available and there are two loop-carried dependences.

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "mibench-adpcmdec-common.h"
#include "mibench-adpcmdec-scalar.h"
#include "mibench-adpcmdec-xloops.h"
#include "mibench-adpcmdec-xloops-opt.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <stdio.h>

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

#include "mibench-adpcmdec-src.dat"
#include "mibench-adpcmdec-ref.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void ordered_for()   {}

// NOTE: because xloops-opt implementation uses a .s file, this causes a
// linking error when we compile this natively, so we just fail if running
// this natively

#ifndef _MIPS_ARCH_MAVEN
namespace mibench_adpcmdec {
  void mibench_adpcmdec_xloops_opt( char indata[], short outdata[],
                                    int len, adpcm_state *state ) {
    std::cout << "  [ FAILED ] xloops-opt : "
      << "this impl is not supported natively" << std::endl;
  }
}
#endif

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name, short dest[], short ref[],
                     int size )
{
  for ( int i = 0; i < size; i++ ) {
    if ( !( dest[i] == ref[i] ) ) {
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

typedef void (*impl_func_ptr)(char[],short[],int,
                              mibench_adpcmdec::adpcm_state*);

struct Impl
{
  const char*   str;
  impl_func_ptr func_ptr;
};

Impl impl_table[] =
{
  { "scalar",     &mibench_adpcmdec::mibench_adpcmdec_scalar     },
  { "xloops",     &mibench_adpcmdec::mibench_adpcmdec_xloops     },
  { "xloops-opt", &mibench_adpcmdec::mibench_adpcmdec_xloops_opt },
  { "",           0                                              },
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

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    std::vector<short> dest(len,0);
    if ( !impl_all ) {
      mibench_adpcmdec::adpcm_state state;
      impl_ptr->func_ptr( &src[0], &dest[0], len, &state );
      verify_results( impl_ptr->str, &dest[0], &ref[0], len );
    }
    else {
      Impl* impl_ptr = &impl_table[0];
      while ( impl_ptr->func_ptr != 0 ) {
        dest.clear();
        dest.resize(len);
        mibench_adpcmdec::adpcm_state state;
        impl_ptr->func_ptr( &src[0], &dest[0], len, &state );
        verify_results( impl_ptr->str, &dest[0], &ref[0], len );
        impl_ptr++;
      }
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute the micro-benchmark

  mibench_adpcmdec::adpcm_state state;
  std::vector<short> dest(len,0);
  if ( warmup )
    impl_ptr->func_ptr( &src[0], &dest[0], len, &state );

  // reset values
  dest.clear();
  dest.resize(len);
  state.valprev = 0;
  state.index   = 0;

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ )
    impl_ptr->func_ptr( &src[0], &dest[0], len, &state );

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

