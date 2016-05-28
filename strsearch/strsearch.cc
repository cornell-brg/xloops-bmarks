//========================================================================
// strsearch
//========================================================================
// This application kernel performs the Knuth-Morris-Pratt algorithm to
// find strings within documents. The KMP search and DFA construction
// algorithms are implemented as described in:
// http://en.wikipedia.org/w/index.php?title=Knuth-Morris-Pratt_algorithm
// http://oak.cs.ucla.edu/cs144/notes/ir.txt

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "strsearch-scalar.h"
#include "strsearch-xloops.h"

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

#include "strsearch-small.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name, int m[], int m_ref[], int len, int ver )
{
  int k = 0;
  if (!ver) {
    // check version where str is outer loop
    for ( int i = 0; i < len*len; i++ ) {
      if ( m[i] != m_ref[i] ) {
        std::cout << "  [ FAILED ] " << name << " : "
          << "m[" << i << "] != m_ref[" << i << "] "
          << "( " << m[i] << " != " << m_ref[i] << " )"
          << std::endl;
        return;
      }
    }
  } else {
    // check version where doc is outer loop
    for ( int i = 0; i < len; i++ ) {
      for ( int j = 0; j < len; j++ ) {
        if ( m[j*len+i] != m_ref[k] ) {
          std::cout << "  [ FAILED ] " << name << " : "
            << "m[" << j*len+i << "] != m_ref[" << k << "] "
            << "( " << m[j*len+i] << " != " << m_ref[k] << " )"
            << std::endl;
          return;
        }
        k++;
      }
    }
  }
  std::cout << "  [ passed ] " << name << std::endl;
}

//------------------------------------------------------------------------
// Test harness
//------------------------------------------------------------------------

// HACK: Destination array to enforce alignment
// int g_m[57*57] __attribute__ ((aligned (32)));
// int g_m[57*57];

int main( int argc, char* argv[] )
{
  // Handle any uncaught exceptions

  stdx::set_terminate();

  // Command line options

  pinfo::ProgramInfo pi;

  pi.add_arg( "--ntrials",  "int",         "1",     "Number of trials" );
  pi.add_arg( "--verify",   NULL,          NULL,    "Verify an implementation" );
  pi.add_arg( "--stats",    NULL,          NULL,    "Ouptut stats about run" );
  pi.add_arg( "--warmup",   NULL,          NULL,   "Warmup the cache" );

  pi.add_arg( "--impl", "{scalar,xloops}", "scalar",
              "Implementation style" );
  pi.add_arg( "--alg",     "{str, doc}",  "str",
              "Use ut per string, or ut per document" );

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

  enum Alg
  {
    ALG_STR,
    ALG_DOC,
    ALG_ALL,
  };

  Impl impl;
  Alg alg;

  const char* impl_str = pi.get_string("--impl");
  const char* alg_str = pi.get_string("--alg");

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

  if ( strcmp( alg_str, "str" ) == 0 )
    alg = ALG_STR;
  else if ( strcmp( alg_str, "doc" ) == 0 )
    alg = ALG_DOC;
  else if ( strcmp( alg_str, "all" ) == 0 ) {
    alg = ALG_ALL;
    if ( !verify ) {
      std::cout
        << "\n ERROR: --alg all only valid for verification\n"
        << std::endl;
      return 1;
    }
  }
  else {
    std::cout
      << "\n ERROR: Unrecognized algorithm "
      << "(" << impl_str << ")\n" << std::endl;
    return 1;
  }

  // Generate int * and char * arrays to reference DFAs and strings since
  // we can't really use std::vector and std::string in vfetch blocks

  std::vector<int> dfa[len];
  int * dfa_ptr[len];
  char * doc_ptr[len];
  char * str_ptr[len];
  int str_sz[len];
  int doc_sz[len];
  for (int i = 0; i < len; i++) {
    // allocate space for DFAs
    dfa[i] = std::vector<int>(str[i].size() + 1);
    // create an array of string and doc sizes
    doc_sz[i] = static_cast<int>(doc[i].size());
    str_sz[i] = static_cast<int>(str[i].size());
    // create pointers to doc, str, and dfa objects
    dfa_ptr[i] = &dfa[i][0];
    str_ptr[i] = &str[i][0];
    doc_ptr[i] = &doc[i][0];
  }

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0];
    std::cout << " --alg " << alg_str << "\n" << std::endl;

    if ( (impl == IMPL_ALL) || (impl == IMPL_SCALAR) ) {
      std::vector<int> m(len * len);
      strsearch::strsearch_scalar( &m[0], dfa_ptr, doc_ptr, str_ptr,
                                   doc_sz, str_sz, len, len, alg);
      verify_results( "scalar", &m[0], m_ref, len, alg);
    }
    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS) ) {
      std::vector<int> m(len * len);
      strsearch::strsearch_xloops( &m[0], dfa_ptr, doc_ptr, str_ptr,
                                 doc_sz, str_sz, len, len, alg);
      verify_results( "xloops", &m[0], m_ref, len, alg);
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute strsearch

  std::vector<int> m(len * len);

  if ( warmup ) {
    if ( impl == IMPL_SCALAR )
      strsearch::strsearch_scalar( &m[0], dfa_ptr, doc_ptr, str_ptr,
                                   doc_sz, str_sz, len, len, alg);

    else if ( impl == IMPL_XLOOPS )
      strsearch::strsearch_xloops( &m[0], dfa_ptr, doc_ptr, str_ptr,
                                 doc_sz, str_sz, len, len, alg);
  }

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ ) {

    if ( impl == IMPL_SCALAR )
      strsearch::strsearch_scalar( &m[0], dfa_ptr, doc_ptr, str_ptr,
                                   doc_sz, str_sz, len, len, alg);

    else if ( impl == IMPL_XLOOPS )
      strsearch::strsearch_xloops( &m[0], dfa_ptr, doc_ptr, str_ptr,
                                   doc_sz, str_sz, len, len, alg);

  }

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

