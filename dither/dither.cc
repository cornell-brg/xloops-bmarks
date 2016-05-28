//========================================================================
// dither
//========================================================================
// This application does grayscale floyd-steinberg image dithering.
// Currently two input images are supported: a gradient image and a
// dragon image.

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-FstreamUtils.h"
#include "stdx-Exception.h"

#include "dither-scalar.h"
#include "dither-xloops.h"
#include "dither-xloops-reg.h"
#include "dither-xloops-reg-opt.h"
#include "dither-xloops-mem.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>

using namespace dither;

// Shorter name for byte type
typedef unsigned char byte;

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
// Include image data
//------------------------------------------------------------------------

#include "dither-gradient-src.dat"
#include "dither-gradient-ref.dat"

#include "dither-dragon-src.dat"
#include "dither-dragon-ref.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}
void ordered_for() {}
void mem_for() {}

//------------------------------------------------------------------------
// dump_dataset
//------------------------------------------------------------------------

void dump_dataset( const std::string& image_name,
                   const std::string& fname, byte image[],
                   int nrows, int ncols )
{
  // Generate reference output

  std::vector<byte> dest(nrows*ncols);
  dither_scalar( &dest[0], image, nrows, ncols );

  // Output dataset

  stdx::CheckedOutputFstream fout(fname);

  fout << "// Data set for " << image_name << "\n\n";

  fout << "byte ref_" << image_name << "["
       << "nrows_" << image_name << "*"
       << "ncols_" << image_name << "] = {\n  ";

  for ( int row_idx = 0; row_idx < nrows; row_idx++ ) {
    for ( int col_idx = 0; col_idx < ncols; col_idx++ ) {
      int idx = row_idx*ncols + col_idx;
      fout << "0x" << std::setw(2) << std::setfill('0') << std::hex
           << static_cast<int>(dest[idx]) << ",";
      if ( idx % 8 == 7 )
        fout << "\n  ";
    }
  }

  fout << "};\n" << std::endl;
}

//------------------------------------------------------------------------
// dump_image
//------------------------------------------------------------------------

void dump_image( const std::string& fname, byte image[],
                 int nrows, int ncols )
{
  stdx::CheckedOutputFstream fout(fname);
  fout << "P3 " << ncols << " " << nrows << " 255\n";
  for ( int ridx = 0; ridx < nrows; ridx++ ) {
    for ( int cidx = 0; cidx < ncols; cidx++ ) {
      int idx = ridx*ncols + cidx;
      for ( int i = 0; i < 3; i++ )
        fout << static_cast<int>(image[idx]) << " ";
      fout << "\n";
    }
  }
}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name, byte dest[], byte ref[],
                     int nrows, int ncols )
{
  for ( int i = 0; i < nrows*ncols; i++ ) {
    if ( !( dest[i] == ref[i] ) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "dest[" << i << "] != ref[" << i << "] "
        << "( " << static_cast<int>(dest[i])
                << " != " << static_cast<int>(ref[i]) << " )"
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
// byte g_dest[400*400] __attribute__ ((aligned (32)));
// byte g_dest[400*400];

int main( int argc, char* argv[] )
{

  // Handle any uncaught exceptions

  stdx::set_terminate();

  // Command line options

  pinfo::ProgramInfo pi;

  pi.add_arg( "--image",    "{gradient,dragon}", "gradient", "Input image" );
  pi.add_arg( "--ntrials",  "int", "1",    "Number of trials" );
  pi.add_arg( "--verify",   NULL,  NULL,   "Verify an implementation" );
  pi.add_arg( "--stats",    NULL,  NULL,   "Ouptut stats about run" );
  pi.add_arg( "--warmup",   NULL,  NULL,   "Warmup the cache" );

  pi.add_arg( "--dump-images", NULL, NULL,
              "Dump input and output image to ppm files" );

  pi.add_arg( "--dump-dataset", "fname", "none",
              "Dump generated dataset to given file" );

#ifdef _MIPS_ARCH_MAVEN
  pi.add_arg( "--impl", "{scalar,xloops,xloops-reg,xloops-reg-opt,xloops-mem}", "scalar",
              "Implementation style" );
#else
  pi.add_arg( "--impl", "{scalar}", "scalar",
              "Implementation style" );
#endif

  pi.parse_args( argc, argv );

  // Set misc local variables from command line

  int  ntrials = static_cast<int>(pi.get_long("--ntrials"));
  bool verify  = static_cast<bool>(pi.get_flag("--verify"));
  bool stats   = static_cast<bool>(pi.get_flag("--stats"));
  bool warmup  = static_cast<bool>(pi.get_flag("--warmup"));

  // Choose input image

  int   nrows = 0;
  int   ncols = 0;
  byte* image = 0;
  byte* ref;

  const char* image_name = pi.get_string("--image");
  if ( strcmp( image_name, "gradient" ) == 0 ) {
    nrows = nrows_gradient;
    ncols = ncols_gradient;
    image = image_gradient;
    ref = ref_gradient;
  }
  else if ( strcmp( image_name, "dragon" ) == 0 ) {
    nrows = nrows_dragon;
    ncols = ncols_dragon;
    image = image_dragon;
    ref = ref_dragon;
  }
  else {
    std::cout
      << "\n ERROR: Unrecognized image "
      << "(" << image_name << ")\n" << std::endl;
    return 1;
  }

  // Setup implementation

  enum Impl
  {
    IMPL_SCALAR,
    IMPL_MT,
    IMPL_SIMT,
    IMPL_XLOOPS,
    IMPL_XLOOPS_REG,
    IMPL_XLOOPS_REG_OPT,
    IMPL_XLOOPS_MEM,
    IMPL_ALL,
  };

  Impl impl;
  const char* impl_str = pi.get_string("--impl");
  if ( strcmp( impl_str, "scalar" ) == 0 )
    impl = IMPL_SCALAR;
  else if ( strcmp( impl_str, "xloops" ) == 0 )
    impl = IMPL_XLOOPS;
  else if ( strcmp( impl_str, "xloops-reg" ) == 0 )
    impl = IMPL_XLOOPS_REG;
  else if ( strcmp( impl_str, "xloops-reg-opt" ) == 0 )
    impl = IMPL_XLOOPS_REG_OPT;
  else if ( strcmp( impl_str, "xloops-mem" ) == 0 )
    impl = IMPL_XLOOPS_MEM;
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
    dump_dataset( image_name, dump_dataset_fname, image, nrows, ncols );
    return 0;
  }

  // Verify the implementations

  if ( verify ) {

    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( (impl == IMPL_ALL) || (impl == IMPL_SCALAR) ) {
      std::vector<byte> dest(nrows*ncols);
      dither_scalar( &dest[0], image, nrows, ncols );
      verify_results( "scalar", &dest[0], ref, nrows, ncols );
    }

    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS) ) {
      std::vector<byte> dest(nrows*ncols);
      dither_xloops( &dest[0], image, nrows, ncols );
      verify_results( "xloops", &dest[0], ref, nrows, ncols );
    }

    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS_REG) ) {
      std::vector<byte> dest(nrows*ncols);
      dither_xloops_reg( &dest[0], image, nrows, ncols );
      verify_results( "xloops-reg", &dest[0], ref, nrows, ncols );
    }

    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS_REG_OPT) ) {
      std::vector<byte> dest(nrows*ncols);
      dither_xloops_reg_opt( &dest[0], image, nrows, ncols );
      verify_results( "xloops-reg-opt", &dest[0], ref, nrows, ncols );
    }

    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS_MEM) ) {
      std::vector<byte> dest(nrows*ncols);
      dither_xloops_mem( &dest[0], image, nrows, ncols );
      verify_results( "xloops-mem", &dest[0], ref, nrows, ncols );
    }

    std::cout << std::endl;
    return 0;
  }

  // Execute dither

  std::vector<byte> dest(nrows*ncols,0x00u);

  if ( warmup ) {
    if ( impl == IMPL_SCALAR )
      dither_scalar( &dest[0], image, nrows, ncols );

    else if ( impl == IMPL_XLOOPS )
      dither_xloops( &dest[0], image, nrows, ncols );

    else if ( impl == IMPL_XLOOPS_REG )
      dither_xloops_reg( &dest[0], image, nrows, ncols );

    else if ( impl == IMPL_XLOOPS_REG_OPT )
      dither_xloops_reg_opt( &dest[0], image, nrows, ncols );

    else if ( impl == IMPL_XLOOPS_MEM )
      dither_xloops_mem( &dest[0], image, nrows, ncols );
  }

  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for ( int i = 0; i < ntrials; i++ ) {

    if ( impl == IMPL_SCALAR )
      dither_scalar( &dest[0], image, nrows, ncols );

    else if ( impl == IMPL_XLOOPS )
      dither_xloops( &dest[0], image, nrows, ncols );

    else if ( impl == IMPL_XLOOPS_REG )
      dither_xloops_reg( &dest[0], image, nrows, ncols );

    else if ( impl == IMPL_XLOOPS_REG_OPT )
      dither_xloops_reg_opt( &dest[0], image, nrows, ncols );

    else if ( impl == IMPL_XLOOPS_MEM )
      dither_xloops_mem( &dest[0], image, nrows, ncols );
  }

  set_stats_en(false);
  timer.stop();

  // Display stats

  if ( stats )
    std::cout << "runtime = " << timer.get_elapsed() << std::endl;

  // Dump images if desired

  if ( pi.get_flag("--dump-images") ) {
    std::string str = std::string(image_name);
    dump_image( "dither-"+str+"-src.ppm",  image,    nrows, ncols );
    dump_image( "dither-"+str+"-dest.ppm", &dest[0], nrows, ncols );
  }

  // Return zero upon successful completion

  return 0;
}

