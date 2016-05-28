//========================================================================
// rsort
//========================================================================
// This application uses a radix sort to sort an array of integers.

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "rsort-scalar.h"
#include "rsort-xloops.h"
#include "rsort-xloops-opt.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace rsort;

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

#include "rsort.dat"

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
// dump_dataset
//------------------------------------------------------------------------

void dump_dataset( const std::string& fname, int size )
{
  // Generate input

  std::vector<int> src(size);
  for ( int i = 0; i < size; i++ )
    src.at(i) = static_cast<int>(std::rand());

  // Generate reference output

  std::vector<int> dest = src;
  stdx::sort( stdx::mk_irange(dest) );

  // Output dataset

  stdx::CheckedOutputFstream fout(fname);

  fout << "// Data set for " << fname << "\n\n";
  fout << "int size = " << size << ";\n\n";

  fout << "int src[] = {\n";
  for ( int i = 0; i < size; i++ )
    fout << "  " << src[i] << ",\n";
  fout << "};\n" << std::endl;

  fout << "int ref[] = {\n";
  for ( int i = 0; i < size; i++ )
    fout << "  " << dest[i] << ",\n";
  fout << "};\n" << std::endl;
}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name, int dest[], int ref[], int size )
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
// Test harness
//------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
  // Handle any uncaught exceptions

  stdx::set_terminate();

  // Command line options

  pinfo::ProgramInfo pi;

  pi.add_arg( "--radix",    "int", "16",   "Radix to use for sort" );
  pi.add_arg( "--ntrials",  "int", "1",    "Number of trials" );
  pi.add_arg( "--verify",   NULL,  NULL,   "Verify an implementation" );
  pi.add_arg( "--stats",    NULL,  NULL,   "Ouptut stats about run" );
  pi.add_arg( "--warmup",   NULL,  NULL,   "Warmup the cache" );

  pi.add_arg( "--dataset-size", "int", "1000",
              "Size of array for generated dataset" );

  pi.add_arg( "--dump-dataset", "fname", "none",
              "Dump generated dataset to given file" );

#ifdef _MIPS_ARCH_MAVEN
  pi.add_arg( "--impl", "{scalar,xloops,xloops-opt}", "scalar",
              "Implementation style" );
#else
  pi.add_arg( "--impl", "{scalar,xloops,xloops-opt}", "scalar",
              "Implementation style" );
#endif
  pi.add_arg( "--nthreads",  "int", "16",   "Number of threads for xloops opt code" );

  pi.parse_args( argc, argv );

  // Set misc local variables from command line

  int  radix      = static_cast<int>(pi.get_long("--radix"));
  int  ntrials    = static_cast<int>(pi.get_long("--ntrials"));
  bool verify     = static_cast<bool>(pi.get_flag("--verify"));
  bool stats      = static_cast<bool>(pi.get_flag("--stats"));
  bool warmup     = static_cast<bool>(pi.get_flag("--warmup"));
  int  dataset_sz = static_cast<int>(pi.get_long("--dataset-size"));
  int  nthreads   = static_cast<int>(pi.get_long("--nthreads"));


  // alex: sort based on dataset_sz if this is specified?
  size = dataset_sz;

  // Compute radix_lg from radix givin on command line. We do this here
  // to avoid pushing this into the timed portions of the app.

  if ( !stdx::is_pow2(radix) ) {
    std::cout << "\n ERROR: --radix must be power of two\n" << std::endl;
    return 1;
  }

  int radix_lg = stdx::lg(radix);

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

  // Dump data if --dump-dataset given on command line

  const char* dump_dataset_fname = pi.get_string("--dump-dataset");
  if ( strcmp( dump_dataset_fname, "none" ) != 0 ) {
    dump_dataset( dump_dataset_fname, dataset_sz );
    return 0;
  }


  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( (impl == IMPL_ALL) || (impl == IMPL_SCALAR) ) {
      std::vector<int> dest(size);
      rsort_scalar( &dest[0], src, size, radix_lg );
      verify_results( "scalar", &dest[0], ref, size );
    }

    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS) ) {
      std::vector<int> dest(size);
      rsort_xloops( &dest[0], src, size, radix_lg );
      verify_results( "xloops", &dest[0], ref, size );
    }

    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS_OPT) ) {
      std::vector<int> dest(size);
      rsort_xloops_opt( &dest[0], src, size, radix_lg, nthreads );
      verify_results( "xloops-opt", &dest[0], ref, size );
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute rsort

  std::vector<int> dest(size);

  if ( warmup ) {
    if ( impl == IMPL_SCALAR )
      rsort_scalar( &dest[0], src, size, radix_lg );

    else if ( impl == IMPL_XLOOPS )
      rsort_xloops( &dest[0], src, size, radix_lg );

    else if ( impl == IMPL_XLOOPS_OPT )
      rsort_xloops_opt( &dest[0], src, size, radix_lg, nthreads );
  }

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ ) {

    if ( impl == IMPL_SCALAR )
      rsort_scalar( &dest[0], src, size, radix_lg );

    else if ( impl == IMPL_XLOOPS )
      rsort_xloops( &dest[0], src, size, radix_lg );

    else if ( impl == IMPL_XLOOPS_OPT )
      rsort_xloops_opt( &dest[0], src, size, radix_lg, nthreads );

  }

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

