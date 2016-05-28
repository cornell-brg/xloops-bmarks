//========================================================================
// pbbs-knn: K-Nearest Neighbors (KNN)
//========================================================================

#include "pinfo.h"

// pbbs common stuff:
#include "pbbs-common-geometryIO.h"

#include "pbbs-knn-scalar.h"
#include "pbbs-knn-xloops.h"
#include "pbbs-knn-check.h"

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
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}
void mem_for() {}

//------------------------------------------------------------------------
// Implementation Table
//------------------------------------------------------------------------
// This table contains one structure for each of the above
// implementations. Each structure includes the name of the
// implementation used for command line parsing and verification output,
// and a function pointer to the actual implementation function.

typedef void (*impl_func_ptr)(vertex_2d**, int, int );
struct Impl
{
  const char*   str;
  impl_func_ptr func_ptr;
};

Impl impl_table[] =
{
  { "scalar",       &knn_scalar_2d       },
  { "xloops",       &knn_xloops_2d       },
  { "",             0                    },
};

//------------------------------------------------------------------------
// Test Harness
//------------------------------------------------------------------------

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

  pi.add_arg( "--input-file",  "fname", "none", "Input data file" );
  pi.add_arg( "--output-file", "fname", "none", "Output data file" );
  pi.add_arg( "--k",           "int",   "1"   , "Number of neighbors to find" );

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

  // Read the points from file

  char* input_filename  = (char*) pi.get_string("--input-file");
  char* output_filename = (char*) pi.get_string("--output-file");

  if ( !strcmp( input_filename, "none" ) ) {
    std::cout
      << "\n ERROR: need to specify an input dataset file\n"
      << std::endl;
    return 1;
  }

  _seq<point2d> in_points =
              benchIO::readPointsFromFile<point2d>( input_filename );

  int n  = in_points.n;
  _point2d<float>* pts  = in_points.A;

  // Read the k-nearest neighbors parameters

  int k = static_cast<int>(pi.get_long("--k"));

  if ( k < 1 || k > 10 ) {
    std::cout
      << "\n ERROR: k must be in range [1,10]\n"
      << std::endl;
    return 1;
  }

  int m = n*k; // the total number of neighbors to find

  vertex_2d** v = newA(vertex_2d*,n);
  vertex_2d* vv = newA(vertex_2d, n);

  for( int i = 0; i < n; i++ ){
    v[i] = new (&vv[i]) vertex_2d(pts[i],i);
  }

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( !impl_all ) {
      impl_ptr->func_ptr( v, n, k );
      int* Pout = newA( int, m );
      for ( int i = 0; i < n; i++ ) {
        for ( int j = 0; j < k; j++ ) {
          Pout[k*i + j] = ( v[i]->ngh[j] )->identifier;
        }
      }
      _seq<intT> neighbors = _seq<intT>( Pout, m );
      check_neighbors( neighbors, pts, n, k, 10, impl_ptr->str );
    } else {
      Impl *impl_ptr = &impl_table[0];
      while ( impl_ptr->func_ptr != 0 ) {
        impl_ptr->func_ptr( v, n, k );
        int* Pout = newA( int, m );
        for ( int i = 0; i < n; i++ ) {
          for ( int j = 0; j < k; j++ ) {
            Pout[k*i + j] = ( v[i]->ngh[j] )->identifier;
          }
        }
        _seq<intT> neighbors = _seq<intT>( Pout, m );
        check_neighbors( neighbors, pts, n, k, 10, impl_ptr->str );
        impl_ptr++;
      }
    }

    std::cout << std::endl;
    return 0;

  }

  // Warmup if necessary

  if ( warmup ) {
    impl_ptr->func_ptr( v, n, k );
  }

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ )
    impl_ptr->func_ptr( v, n, k );

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Write out the neighbors to file if specified

  if ( strcmp( output_filename, "none" ) ) {
    int* Pout = newA( int, m );
    for ( int i = 0; i < n; i++) {
      for ( int j = 0; j < k; j++ ) {
        Pout[k*i + j] = ( v[i]->ngh[j] )->identifier;
      }
    }
    benchIO::writeIntArrayToFile( Pout, m, output_filename );
  }

  return 0;
}

