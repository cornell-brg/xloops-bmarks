//========================================================================
// viterbi
//========================================================================
// This application performs viterbi decoding on a frame of encoded data.

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "viterbi-scalar.h"
#include "viterbi-xloops.h"

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

#include "viterbi.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name, unsigned char msg[],
                     unsigned char ref[], int len )
{
  for ( int i = 0; i < len; i++ ) {
    if (msg[i] != ref[i]) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "msg[" << i << "] != ref[" << i << "] "
        << "( " << std::hex << int(msg[i]) << " != "
        << std::hex << int(ref[i]) << " )"
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
// unsigned char msg[(framebits + (K-1))/8 + 1] __attribute__ ((aligned (32))) = {0};
// unsigned char msg[(framebits + (K-1))/8 + 1] = {0};

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

  pi.add_arg( "--impl", "{scalar,xloops}", "scalar",
              "Implementation style" );

  pi.parse_args( argc, argv );

  // Set misc local variables from command line

  int  ntrials    = static_cast<int>(pi.get_long("--ntrials"));
  bool verify     = static_cast<bool>(pi.get_flag("--verify"));
  bool stats      = static_cast<bool>(pi.get_flag("--stats"));
  bool warmup     = static_cast<bool>(pi.get_flag("--warmup"));

  // Setup implementation

  enum Impl
  {
    IMPL_SCALAR,
    IMPL_MT,
    IMPL_SIMT,
    IMPL_XLOOPS,
    IMPL_ALL,
  };

  Impl impl;
  const char* impl_str = pi.get_string("--impl");
  if ( strcmp( impl_str, "scalar" ) == 0 )
    impl = IMPL_SCALAR;
  else if ( strcmp( impl_str, "xloops" ) == 0 )
    impl = IMPL_XLOOPS;
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
      // TODO: reset contents of msg inside viterbi
      unsigned char msg[(framebits + (K-1))/8 + 1] = {0};
      viterbi::viterbi_scalar(syms, msg, polys, framebits);
      verify_results( "scalar", msg, ref, framebits/8);
    }
    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS) ) {
      unsigned char msg[(framebits + (K-1))/8 + 1] = {0};
      viterbi::viterbi_xloops(syms, msg, polys, framebits);
      verify_results( "xloops", msg, ref, framebits/8);
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute viterbi
  unsigned char msg[(framebits + (K-1))/8 + 1] = {0};

  if ( warmup ) {
    if ( impl == IMPL_SCALAR )
      viterbi::viterbi_scalar(syms, msg, polys, framebits);

    else if ( impl == IMPL_XLOOPS )
      viterbi::viterbi_xloops(syms, msg, polys, framebits);
  }

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ ) {

    if ( impl == IMPL_SCALAR )
      viterbi::viterbi_scalar(syms, msg, polys, framebits);

    else if ( impl == IMPL_XLOOPS )
      viterbi::viterbi_xloops(syms, msg, polys, framebits);

  }

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

