//========================================================================
// sha
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
// SHA-1 is a cryptographic hash function designed by the United States
// National Security Agency and published by the United States NIST as a
// U.S. Federal Information Processing Standard. SHA-1 produces a 160-bit
// (20-byte) hash value. A SHA-1 hash value is typically expressed as a
// hexadecimal number, 40 digits long.
//
// Dataset:
// --------
//
// Dataset is generated using dump-dataset or can be any text.
//
// NOTE:
// -----
//
// 1.) MT, SIMT implementations are not implemented as there is not much
// data-parallelism available in SHA-1 application
//
// 2.) "ror" instruction has to be disabled in the gcc cross compiler for
// the app to run on the isa simulator. Maybe implement this instruction?

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-InputRange.h"
#include "stdx-Exception.h"

#include "sha-common.h"
#include "sha-scalar.h"
#include "sha-xloops.h"
#include "sha-xloops-opt.h"

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

#include "sha.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void ordered_for()   {}
void unordered_for() {}

//------------------------------------------------------------------------
// dump_dataset
//------------------------------------------------------------------------

void dump_dataset( const std::string& fname, int size )
{

  // Filename
  stdx::CheckedOutputFstream fout( fname + ".dat" );

  // Generate inputs

  fout << "// Dataset for sha\n\n";
  fout << "int size = " << size << ";\n\n";

  std::vector<BYTE> src(size);
  fout << "BYTE src [] = {\n";
  for ( int i = 0; i < size; i++ ) {
    src.at(i) = static_cast<int>(stdx::rand_int(26)+65);
    fout << "  " << (int)src[i] << ",\n";
  }
  fout << "};\n\n";

  // Generate reference output

  sha::sha_struct sha_info;

  sha_scalar( &sha_info, &src[0], size );

  fout << "unsigned int ref [] = {\n";
  for ( int i = 0 ; i < 5; i ++ )
    fout << "  " << sha_info.digest[i] << ",\n";
  fout << "};";

}


//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name,
                     sha::sha_struct sha_hash, LONG ref[] )
{
  // Check the message digest which is 20 bytes or 5 words
  for ( int i = 0; i < 5; i++ ) {
    if ( !( sha_hash.digest[i] == ref[i] ) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "digest[" << i << "] != ref [" << i << "] "
        << "( " << sha_hash.digest[i] << " != " << ref[i] << " )"
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

typedef void (*impl_func_ptr)(sha::sha_struct*,BYTE*,int);
//typedef void (*impl_func_ptr)(sha::sha_struct*,FILE*);

struct Impl
{
  const char*   str;
  impl_func_ptr func_ptr;
};

Impl impl_table[] =
{
  { "scalar",     &sha::sha_scalar     },
  { "xloops",     &sha::sha_xloops     },
  { "xloops-opt", &sha::sha_xloops_opt },
  { "",         0                      },
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

  pi.add_arg( "--size",     "int", "1000",   "Size of message in bytes" );
  pi.add_arg( "--ntrials",  "int",  "1",     "Number of trials" );
  pi.add_arg( "--verify",   NULL,   NULL,    "Verify an implementation" );
  pi.add_arg( "--stats",    NULL,   NULL,    "Ouptut stats about run" );
  pi.add_arg( "--warmup",   NULL,   NULL,    "Warmup the cache" );

  pi.add_arg( "--dump-dataset", "fname", "none",
              "Dump generated dataset to given file" );

  pi.add_arg( "--impl",
              "{scalar,xloops,xloops-opt,all}",
              "scalar", "Implementation style" );

  pi.parse_args( argc, argv );

  // Set misc local variables from command line

  int  size       = static_cast<int>(pi.get_long("--size"));
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

  // Dump data if --dump-dataset given on command line

  const char* dump_dataset_fname = pi.get_string("--dump-dataset");
  if ( strcmp( dump_dataset_fname, "none" ) != 0 ) {
    dump_dataset( dump_dataset_fname, size );
    return 0;
  }

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( !impl_all ) {
      sha::sha_struct sha_info;
      impl_ptr->func_ptr( &sha_info, &src[0], size );
      verify_results( impl_ptr->str, sha_info, &ref[0] );
    }
    else {
      Impl* impl_ptr = &impl_table[0];
      while ( impl_ptr->func_ptr != 0 ) {
        sha::sha_struct sha_info;
        impl_ptr->func_ptr( &sha_info, &src[0], size );
        verify_results( impl_ptr->str, sha_info, &ref[0] );
        impl_ptr++;
      }
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute the micro-benchmark

  sha::sha_struct sha_info;
  if ( warmup )
      impl_ptr->func_ptr( &sha_info, &src[0], size );

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ )
    impl_ptr->func_ptr( &sha_info, &src[0], size );

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

