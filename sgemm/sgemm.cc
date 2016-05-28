//========================================================================
// sgemm
//========================================================================
// This is a simple micro-benchmark which implements a square matrix-matrix
// multiply of floating point values. It is inspired by the PARBOIL
// benchmark suite. It can generate its own dataset which is statically
// included as part of the binary. To generate a new dataset simply use
// the -dump-dataset command line option, place the new dataset in the
// subproject directory, and make sure you correctly inlude the dataset.
//
// Note:
// -----
//
// The implemented kernel assumes only square matrices ( size = pow(2,n) ).
// PARBOIL code implements a rectangular matrix-matrix multiplication
// kernel. The simt version of the code is similar to the cuda_base
// version.

#include "pinfo.h"

#include "sgemm-scalar.h"
#include "sgemm-xloops.h"
#include "sgemm-xloops-opt.h"

#include "stdx-Timer.h"
#include "stdx-MathUtils.h"
#include "stdx-FstreamUtils.h"
#include "stdx-Exception.h"

#include <cstring>
#include <iostream>
#include <vector>
#include <math.h>

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
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}
// shreesha: external function to tag loops which need a jump insertion
void insert_jump() {}

//------------------------------------------------------------------------
// calc_log2
//------------------------------------------------------------------------
// This function computes log 2 of an N bit integer in O(lg(N)) operations.
// Borrowed from the Standford bit-twiddling hacks page.
// http://graphics.stanford.edu/~seander/bithacks.html

static const unsigned int b[] = {0xAAAAAAAA, 0xCCCCCCCC, 0xF0F0F0F0,
  0xFF00FF00, 0xFFFF0000};

inline unsigned int calc_log2( int v )
{
  register unsigned int r = ( v & b[0] ) != 0;
  for ( int i = 4; i > 0; i-- )
    r |= ((v & b[i]) != 0) << i;
  return( r );
}

//------------------------------------------------------------------------
// Include dataset (generated with -dump-dataset command line arg)
//------------------------------------------------------------------------

#include "sgemm.dat"

//------------------------------------------------------------------------
// dump_dataset
//------------------------------------------------------------------------

void dump_dataset( const std::string& fname, int size )
{
  // Generate inputs

  std::vector<float> src0(size*size), src1(size*size);
  for ( int i = 0; i < size*size; i++ ) {
    src0.at(i) =  drand48 ();
    src1.at(i) =  drand48 ();
  }

  // Generate reference outputs

  std::vector<float> ref(size*size);
  sgemm_scalar( &ref[0], &src0[0], &src1[0], size );

  // Output dataset

  stdx::DefaultStdoutFstream fout(fname);

  fout << "// Data set for sgemm\n\n";
  fout << "int dataset_sz = " << size*size << ";\n\n";

  fout << "float src0[] = {\n";
  for ( int i = 0; i < size*size; i++ )
    fout << "  " << src0[i] << ",\n";
  fout << "};\n" << std::endl;

  fout << "float src1[] = {\n";
  for ( int i = 0; i < size*size; i++ )
    fout << "  " << src1[i] << ",\n";
  fout << "};\n" << std::endl;

  fout << "float ref[] = {\n";
  for ( int i = 0; i < size*size; i++ )
    fout << "  " << ref[i] << ",\n";
  fout << "};\n" << std::endl;

}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name,
                     float dest[], float ref[], int size )
{
  for ( int i = 0; i < size*size; i++ ) {
    float diff = dest[i] - ref[i];
    if ( !(diff < 0.0001 && diff > -0.0001) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "dest[" << i << "] != ref[" << i << "] "
        << "( " << dest[i] << " != " << ref[i] << " ), "
        << "diff = "<< diff << std::endl;
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

typedef void (*impl_func_ptr)(float[],float[],float[],int);
struct Impl
{
  const char*   str;
  impl_func_ptr func_ptr;
};

Impl impl_table[] =
{
  { "scalar",       &sgemm_scalar       },
  { "xloops",       &sgemm_xloops       },
  { "xloops-opt",   &sgemm_xloops_opt   },
  { "",             0                   },
};

//------------------------------------------------------------------------
// Test harness
//------------------------------------------------------------------------

// HACK: Destination array to enforce alignment
// float g_dest[1024] __attribute__ ((aligned (32)));
// float g_dest[1024];

int main( int argc, char* argv[] )
{
  // Handle any uncaught exceptions

  stdx::set_terminate();

  // Command line options

  pinfo::ProgramInfo pi;

  pi.add_arg( "--size",     "int", "32", "Size of arrays" );
  pi.add_arg( "--ntrials",  "int", "1",    "Number of trials" );
  pi.add_arg( "--verify",   NULL,  NULL,   "Verify an implementation" );
  pi.add_arg( "--stats",    NULL,  NULL,   "Ouptut stats about run" );
  pi.add_arg( "--warmup",   NULL,  NULL,   "Warmup the cache" );

  pi.add_arg( "--dump-dataset", "fname", "none",
              "Dump generated dataset to given file" );

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
              "scalar,xloops", "Implementation style" );

  pi.parse_args( argc, argv );

  // Set misc local variables from command line

  int  size     = static_cast<int>(pi.get_long("--size"));
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

  // Dump data if --dump-dataset given on command line

  const char* dump_dataset_fname = pi.get_string("--dump-dataset");
  if ( strcmp( dump_dataset_fname, "none" ) != 0 ) {
    dump_dataset( dump_dataset_fname, size );
    return 0;
  }

  // Verify that size given on command line is not too large

  if ( size > dataset_sz ) {
    std::cout
      << "\n ERROR: Size is larger than dataset ( " << size << " > "
      << dataset_sz << " ). Either decrease \n size or regenerate "
      << "a larger dataset with the --dump-dataset command \n line "
      << "option.\n" << std::endl;
    return 1;
  }

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( !impl_all ) {
      std::vector<float> dest(size*size);
      impl_ptr->func_ptr( &dest[0], src0, src1, size );
      verify_results( impl_ptr->str, &dest[0], ref, size );
    }
    else {
      Impl* impl_ptr = &impl_table[0];
      while ( impl_ptr->func_ptr != 0 ) {
        std::vector<float> dest(size*size);
        impl_ptr->func_ptr( &dest[0], src0, src1, size );
        verify_results( impl_ptr->str, &dest[0], ref, size );
        impl_ptr++;
      }
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute the micro-benchmark

  std::vector<float> dest(size*size);

  if ( warmup )
    impl_ptr->func_ptr( &dest[0], src0, src1, size );

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ )
    impl_ptr->func_ptr( &dest[0], src0, src1, size );

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Return zero upon successful completion

  return 0;
}

