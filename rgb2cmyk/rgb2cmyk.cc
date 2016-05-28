//========================================================================
// rgb2cmyk
//========================================================================
// Converts an RGB color scheme based .ppm image file into a CMYK color
// scheme based .ppm file. Takes in a .ppm as command line input used for
// conversion.

#include "pinfo.h"

#include "stdx-Timer.h"
#include "stdx-FstreamUtils.h"
#include "stdx-Exception.h"

#include "rgb2cmyk-scalar.h"
#include "rgb2cmyk-xloops.h"
#include "rgb2cmyk-PixelUtils.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <cstring>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

using namespace rgb2cmyk;

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

#include "rgb2cmyk-normandy-src.dat"
#include "rgb2cmyk-normandy-ref.dat"
#include "rgb2cmyk-fjord-src.dat"
#include "rgb2cmyk-fjord-ref.dat"

//------------------------------------------------------------------------
// XLOOPS
//------------------------------------------------------------------------
// Empty external function definitions for transparent loop specialization
// implementations

void unordered_for() {}

//========================================================================
// extract_header
//========================================================================
// Extracts header information from .ppm input file.

void extract_header( char* file_name, int* dimensions )
{
  const int max_buffer_size = 10;
  char buffer[max_buffer_size];
  std::ifstream fin;

  // Open file stream
  fin.open( file_name, std::ifstream::binary );

  // Check for .ppm image format
  fin >> buffer;
  if( strcmp( buffer, "P6" ) && strcmp( buffer, "P3" ) ) {
    std::cout << "\n ERROR: Unrecognized image format" << std::endl;
    dimensions[0] = -1;
    return;
  }

  // Extract width and height information
  fin >> buffer;
  dimensions[0] = atoi(buffer);
  fin >> buffer;
  dimensions[1] = atoi(buffer);
  fin >> buffer;
  int depth = atoi(buffer);
  if( depth != 255 ){
    std::cout << "\n ERROR: Unsupported depth" << std::endl;
    dimensions[0] = -1;
    return;
  }

  // Close file stream
  fin.close();
}

//========================================================================
// extract_arrays
//========================================================================
// Extracts image map data from .ppm input file and stores into local array.

void extract_arrays( char* file_name, RgbPixel* image_src, int nrows, int ncols )
{
  const int max_buffer_size = 10;
  char buffer[max_buffer_size];
  std::ifstream fin;

  // Open file stream
  fin.open( file_name, std::ifstream::binary );

  // Ignore header info
  fin >> buffer; fin >> buffer; fin >> buffer; fin >> buffer;

  // Extract raster information
  for( int i = 0; i < nrows; i++ ){
    for( int j = 0; j < ncols; j++ ){
      for( int k = 0; k < 3; k++ ){
        fin >> buffer;

        if( k == 0 ){
          image_src[i*ncols+j].set_r(atoi(buffer));
        }
        else if( k == 1 ){
          image_src[i*ncols+j].set_g(atoi(buffer));
        }
        else if( k == 2 ){
          image_src[i*ncols+j].set_b(atoi(buffer));
        }
      }
    }
  }

  // Close file stream
  fin.close();
}

//------------------------------------------------------------------------
// dump_dataset
//------------------------------------------------------------------------
// Two variants, one for dumping the src and one for dumping the dest
// arrays as a .dat file.

// Dump src dataset
void dump_dataset_src( const std::string& file_name, RgbPixel* image_src,
                   int nrows, int ncols )
{
  // Output dataset
  stdx::CheckedOutputFstream fout( file_name + "-src.dat" );

  fout << "// Data set for " << file_name << "\n\n";

  fout << "const int nrows_" << file_name << " = " << nrows << ";\n"
       << "const int ncols_" << file_name << " = " << ncols << ";\n\n";

  fout << "byte image_" << file_name << "[nrows_" << file_name << "*"
       << "ncols_" << file_name << "*3] =\n{\n";

  for ( int row_idx = 0; row_idx < nrows; row_idx++ ) {
    for ( int col_idx = 0; col_idx < ncols; col_idx++ ) {
      int idx = row_idx*ncols + col_idx;
      fout << "  0x" << std::setw(2) << std::setfill('0') << std::hex
           << image_src[idx].get_r() << ", "
           << "0x" << std::setw(2) << std::setfill('0') << std::hex
           << image_src[idx].get_g() << ", "
           << "0x" << std::setw(2) << std::setfill('0') << std::hex
           << image_src[idx].get_b() << ",\n";
    }
  }

  fout << "};\n" << std::endl;

  fout.close();
}

// Dump ref dataset
void dump_dataset_ref( const std::string& file_name, CmykPixel* image_dest,
                       int nrows, int ncols )
{
  // Output dataset
  stdx::CheckedOutputFstream fout( file_name + "-ref.dat" );

  fout << "// Data set for " << file_name << "\n\n";

  fout << "const int nrows_" << file_name << " = " << nrows << ";\n"
       << "const int ncols_" << file_name << " = " << ncols << ";\n\n";

  fout << "byte ref_" << file_name << "[nrows_" << file_name << "*"
       << "ncols_" << file_name << "*4] =\n{\n";

  for ( int row_idx = 0; row_idx < nrows; row_idx++ ) {
    for ( int col_idx = 0; col_idx < ncols; col_idx++ ) {
      int idx = row_idx*ncols + col_idx;
      fout << "  0x" << std::setw(2) << std::setfill('0') << std::hex
           << image_dest[idx].get_c() << ", "
           << "0x" << std::setw(2) << std::setfill('0') << std::hex
           << image_dest[idx].get_m() << ", "
           << "0x" << std::setw(2) << std::setfill('0') << std::hex
           << image_dest[idx].get_y() << ", "
           << "0x" << std::setw(2) << std::setfill('0') << std::hex
           << image_dest[idx].get_k() << ",\n";
    }
  }

  fout << "};\n" << std::endl;

  fout.close();
}

//------------------------------------------------------------------------
// dump_image
//------------------------------------------------------------------------
// Takes a result image map array and dumps it as a .tiff file for viewing

void dump_image( const char* file_name, CmykPixel* image_dest, int nrows, int ncols )
{
  std::ofstream fout;

  // Open file stream
  fout.open( file_name, std::ofstream::binary );

  // Write header (little endian)
  char temp_h0[] = { 0x49, 0x49, 0x2a, 0x00 };
  fout.write( temp_h0, 4 );

  // Caculate offset to IFD
  int offset = nrows*ncols*4 + 8;
  fout.write( reinterpret_cast<char*>( &offset ), 4 );

  // Write raster info
  for ( int i = 0; i < nrows; i++ ) {
    for ( int j = 0; j < ncols; j++ ) {
      int idx = i*ncols + j;
      for ( int k = 0; k < 4; k++ ){
        if(k == 0)
          fout.put( image_dest[idx].get_c() & 0xff );
        else if(k == 1)
          fout.put( image_dest[idx].get_m() & 0xff );
        else if(k == 2)
          fout.put( image_dest[idx].get_y() & 0xff );
        else if(k == 3)
          fout.put( image_dest[idx].get_k() & 0xff );
      }
    }
  }

  // Write IFD
  int num_entries = 14;
  fout.write( reinterpret_cast<char*>( &num_entries ), 2 ); // Number of entries

  char temp_f1[] = { 0x00, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00 }; // Image width
  fout.write( temp_f1, 8 );
  fout.write( reinterpret_cast<char*>( &ncols ), 2 );
  char temp_f0[] = { 0x00, 0x00 };
  fout.write( temp_f0, 2 );

  char temp_f2[] = { 0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00 }; // Image height
  fout.write( temp_f2, 8 );
  fout.write( reinterpret_cast<char*>( &nrows ), 2 );
  fout.write( temp_f0, 2 );

  char temp_f3[] = { 0x02, 0x01, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00 }; // Bits per sample
  fout.write( temp_f3, 8 );
  int offset_f0 = offset + 4 + 12*num_entries + 2;
  fout.write( reinterpret_cast<char*>( &offset_f0 ), 4 );

  char temp_f4[] = { 0x03, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
                     0x01, 0x00, 0x00, 0x00 }; // Compression (uncompressed)
  fout.write( temp_f4, 12 );

  char temp_f5[] = { 0x06, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
                     0x05, 0x00, 0x00, 0x00 }; // Photometric interpretation
  fout.write( temp_f5, 12 );

  char temp_f6[] = { 0x11, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
                     0x08, 0x00, 0x00, 0x00 }; // Strip offsets
  fout.write( temp_f6, 12 );

  char temp_f7[] = { 0x12, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
                     0x01, 0x00, 0x00, 0x00 }; // Orientation
  fout.write( temp_f7, 12 );

  char temp_f8[] = { 0x15, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
                     0x04, 0x00, 0x00, 0x00 }; // Samples per pixel
  fout.write( temp_f8, 12 );

  char temp_f9[] = { 0x16, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00 }; // Rows per strip
  fout.write( temp_f9, 8 );
  fout.write( reinterpret_cast<char*>( &nrows ), 2 );
  fout.write( temp_f0, 2 );

  char temp_f10[] = { 0x17, 0x01, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00 }; // Strip byte counts
  fout.write( temp_f10, 8 );
  int strip_count = nrows*ncols*4;
  fout.write( reinterpret_cast<char*>( &strip_count ), 4 );

  char temp_f11[] = { 0x18, 0x01, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00 }; // Min sample value
  fout.write( temp_f11, 8 );
  int offset_f1 = offset + 12 + 12*num_entries + 2;
  fout.write( reinterpret_cast<char*>( &offset_f1 ), 4 );

  char temp_f12[] = { 0x19, 0x01, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00 }; // Max sample value
  fout.write( temp_f12, 8 );
  int offset_f2 = offset + 20 + 12*num_entries + 2;
  fout.write( reinterpret_cast<char*>( &offset_f2 ), 4 );

  char temp_f13[] = { 0x1c, 0x01, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00,
                      0x01, 0x00, 0x00, 0x00 }; // Planar config
  fout.write( temp_f13, 12 );

  char temp_f14[] = { 0x53, 0x01, 0x03, 0x00, 0x04, 0x00, 0x00, 0x00 }; // Sample format
  fout.write( temp_f14, 8 );
  int offset_f3 = offset + 28 + 12*num_entries + 2;
  fout.write( reinterpret_cast<char*>( &offset_f3 ), 4 );

  char temp_f15[] = { 0x00, 0x00, 0x00, 0x00 }; // Termination code
  fout.write( temp_f15, 4 );

  char temp_f16[] = { 0x08, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0x00 }; // Bits per sample data
  fout.write( temp_f16, 8 );

  fout.write( temp_f15, 4 ); // Min sample value data
  fout.write( temp_f15, 4 );

  char temp_f17[] = { 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00 }; // Max sample value data
  fout.write( temp_f17, 8 );

  char temp_f18[] = { 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00 }; // Sample format data
  fout.write( temp_f18, 8 );

  // Close file stream
  fout.close();
}

//------------------------------------------------------------------------
// verify_results
//------------------------------------------------------------------------

void verify_results( const char* name, CmykPixel* image_dest,
                     CmykPixel* image_ref, int nrows, int ncols )
{
  for ( int i = 0; i < nrows*ncols; i++ ) {

    if ( !( image_dest[i].get_c() == image_ref[i].get_c() ) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "image_dest[" << i << "].m_c != image_ref[" << i << "].m_c "
        << "( " << static_cast<int>(image_dest[i].get_c())
                << " != " << static_cast<int>(image_ref[i].get_c()) << " )"
        << std::endl;
      return;
    }
    if ( !( image_dest[i].get_m() == image_ref[i].get_m() ) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "image_dest[" << i << "].m_m != image_ref[" << i << "].m_m "
        << "( " << static_cast<int>(image_dest[i].get_m())
                << " != " << static_cast<int>(image_ref[i].get_m()) << " )"
        << std::endl;
      return;
    }
    if ( !( image_dest[i].get_y() == image_ref[i].get_y() ) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "image_dest[" << i << "].m_y != image_ref[" << i << "].m_y "
        << "( " << static_cast<int>(image_dest[i].get_y())
                << " != " << static_cast<int>(image_ref[i].get_y()) << " )"
        << std::endl;
      return;
    }
    if ( !( image_dest[i].get_k() == image_ref[i].get_k() ) ) {
      std::cout << "  [ FAILED ] " << name << " : "
        << "image_dest[" << i << "].m_k != image_ref[" << i << "].m_k "
        << "( " << static_cast<int>(image_dest[i].get_k())
                << " != " << static_cast<int>(image_ref[i].get_k()) << " )"
        << std::endl;
      return;
    }

  }
  std::cout << "  [ passed ] " << name << std::endl;
}

//========================================================================
// Test Harness
//========================================================================

// HACK: Destination array to enforce alignment
// CmykPixel g_image_dest [100*100] __attribute__ ((aligned (32)));
// CmykPixel g_image_dest [100*100];

int main( int argc, char* argv[] )
{
  // Handle uncaught exceptions

  stdx::set_terminate();

  // Dimension Variables

  int nrows;
  int ncols;

  // Image arrays

  std::vector<RgbPixel> image_src;
  std::vector<CmykPixel> image_dest;
  std::vector<CmykPixel> image_ref;

  bool src_alloc = false;

  // Command line options

  pinfo::ProgramInfo pi;

  pi.add_arg( "--image",        "{normandy, fjord}", "normandy", "Input image, or specify custom .ppm format" );
  pi.add_arg( "--ntrials",      "int", "1",      "Number of trials" );
  pi.add_arg( "--verify",       NULL, NULL,      "Verify an implementation" );
  pi.add_arg( "--stats",        NULL, NULL,      "Output stats about run" );
  pi.add_arg( "--warmup",       NULL, NULL,      "Warmup the cache" );
  pi.add_arg( "--dump-image",   NULL, NULL,      "Dump output image to .tiff file" );
  pi.add_arg( "--dump-dataset", "{src, ref}", "none",         "Dump source or reference dataset to .dat file" );
  pi.add_arg( "--impl",         "{scalar,xloops}", "scalar", "Implementation style" );

  pi.parse_args( argc, argv );

  // Set variables from options

  char* file_name = const_cast<char*>(pi.get_string("--image"));
  int ntrials     = static_cast<int>(pi.get_long("--ntrials"));
  bool verify     = static_cast<bool>(pi.get_flag("--verify"));
  bool stats      = static_cast<bool>(pi.get_flag("--stats"));
  bool warmup     = static_cast<bool>(pi.get_flag("--warmup"));
  bool dump_img   = static_cast<bool>(pi.get_flag("--dump-image"));
  char* dump_dset  = const_cast<char*>(pi.get_string("--dump-dataset"));

  if (    strcmp( dump_dset, "src" )
       && strcmp( dump_dset, "ref" )
       && strcmp( dump_dset, "none" ) )
  {
    std::cout
      << "\n ERROR: --dump-dataset argument unknown, use {src, ref}\n"
      << std::endl;
    return 1;
  }

  // Use hard-coded images by default, but run extraction procedure for
  // custom .ppm images

  if ( !strcmp( file_name, "normandy" ) ) {
    nrows = nrows_normandy;
    ncols = ncols_normandy;

    image_src.resize( nrows*ncols );
    image_ref.resize( nrows*ncols );

    for( int i = 0; i < nrows; i++ ){
      for( int j = 0; j < ncols; j++ ){
        for( int k = 0; k < 4; k++ ){
          if( k == 0 ){
            image_src[i*ncols+j].set_r(image_normandy[3*i*ncols + 3*j + k]);
            image_ref[i*ncols+j].set_c(ref_normandy[4*i*ncols + 4*j + k]);
          }
          else if( k == 1 ){
            image_src[i*ncols+j].set_g(image_normandy[3*i*ncols + 3*j + k]);
            image_ref[i*ncols+j].set_m(ref_normandy[4*i*ncols + 4*j + k]);
          }
          else if( k == 2 ){
            image_src[i*ncols+j].set_b(image_normandy[3*i*ncols + 3*j + k]);
            image_ref[i*ncols+j].set_y(ref_normandy[4*i*ncols + 4*j + k]);
          }
          else if( k == 3 ){
            image_ref[i*ncols+j].set_k(ref_normandy[4*i*ncols + 4*j + k]);
          }
        }
      }
    }

  }
  else if( !strcmp( file_name, "fjord" ) ) {
    nrows = nrows_fjord;
    ncols = ncols_fjord;

    image_src.resize( nrows*ncols );
    image_ref.resize( nrows*ncols );

    for( int i = 0; i < nrows; i++ ){
      for( int j = 0; j < ncols; j++ ){
        for( int k = 0; k < 4; k++ ){
          if( k == 0 ){
            image_src[i*ncols+j].set_r(image_fjord[3*i*ncols + 3*j + k]);
            image_ref[i*ncols+j].set_c(ref_fjord[4*i*ncols + 4*j + k]);
          }
          else if( k == 1 ){
            image_src[i*ncols+j].set_g(image_fjord[3*i*ncols + 3*j + k]);
            image_ref[i*ncols+j].set_m(ref_fjord[4*i*ncols + 4*j + k]);
          }
          else if( k == 2 ){
            image_src[i*ncols+j].set_b(image_fjord[3*i*ncols + 3*j + k]);
            image_ref[i*ncols+j].set_y(ref_fjord[4*i*ncols + 4*j + k]);
          }
          else if( k == 3 ){
            image_ref[i*ncols+j].set_k(ref_fjord[4*i*ncols + 4*j + k]);
          }
        }
      }
    }

  }
  else {

    if(verify) {
      std::cout
        << "\n ERROR: --image custom .ppm not allowed for verification, use {normandy, fjord}\n"
        << std::endl;
      return 1;
    }

    // Extract header information
    int dimensions[2];
    extract_header(file_name, dimensions);
    if(dimensions[0] < 0)
      return 1;
    nrows = dimensions[1];
    ncols = dimensions[0];

    // Initialize arrays
    image_src.resize( nrows*ncols );
    src_alloc = true;

    // Extract data from .ppm into arrays
    extract_arrays( file_name, &image_src[0], nrows, ncols );

  }

  // Initialize result arrays
  image_dest.resize( nrows*ncols );

  // Set implementation style
  enum Impl
  {
    IMPL_SCALAR,
    IMPL_MT,
    IMPL_SIMT,
    IMPL_XLOOPS,
    IMPL_ALL,
  };

  Impl impl;
  char* impl_str = const_cast<char*>(pi.get_string("--impl"));
  if( !strcmp( impl_str, "scalar" ) ) {
    impl = IMPL_SCALAR;
  }
  else if( !strcmp( impl_str, "xloops" ) ) {
    impl = IMPL_XLOOPS;
  }
  else if( !strcmp( impl_str, "all" ) ) {
    impl = IMPL_ALL;
    if(!verify) {
      std::cout
        << "\n ERROR: --impl all only valid for verification\n"
        << std::endl;
      return 1;
    }
  }
  else {
    std::cout << "\n ERROR: Unrecognized implementation " << "(" << impl_str
              << ")\n" << std::endl;
    return 1;
  }

  // Verify the impl
  if(verify) {
    std::cout << "\n Unit Tests : " << argv[0] << "\n" << std::endl;

    if ( (impl == IMPL_ALL) || (impl == IMPL_SCALAR) ) {
      rgb2cmyk_scalar( &image_src[0], &image_dest[0], nrows, ncols );
      verify_results( "scalar", &image_dest[0], &image_ref[0], nrows, ncols );
    }

    if ( (impl == IMPL_ALL) || (impl == IMPL_XLOOPS) ) {
      image_dest.clear();
      image_dest.resize( nrows*ncols );
      rgb2cmyk_xloops( &image_src[0], &image_dest[0], nrows, ncols );
      verify_results( "xloops", &image_dest[0], &image_ref[0], nrows, ncols );
    }

    std::cout << std::endl;
    return 0;
  }

  if ( warmup ) {
    if( impl == IMPL_SCALAR )
      rgb2cmyk_scalar( &image_src[0], &image_dest[0], nrows, ncols );
    else if( impl == IMPL_XLOOPS )
      rgb2cmyk_xloops( &image_src[0], &image_dest[0], nrows, ncols );
  }

  // Perform the conversion
  stdx::Timer timer;
  timer.start();
  set_stats_en(true);

  for( int i = 0; i < ntrials; i++ ) {
    if( impl == IMPL_SCALAR )
      rgb2cmyk_scalar( &image_src[0], &image_dest[0], nrows, ncols );
    else if( impl == IMPL_XLOOPS )
      rgb2cmyk_xloops( &image_src[0], &image_dest[0], nrows, ncols );
  }

  set_stats_en(false);
  timer.stop();

  // Dump dataset to .dat if desired
  if( !strcmp( dump_dset, "src" ) ){
    std::string str = std::string(file_name);
    if(src_alloc)
      str = str.substr( 0, str.length() - 4 );
    dump_dataset_src( str, &image_src[0], nrows, ncols );
  }
  else if( !strcmp( dump_dset, "ref" ) ){
    std::string str = std::string(file_name);
    if(src_alloc)
      str = str.substr( 0, str.length() - 4 );
    dump_dataset_ref( str, &image_dest[0], nrows, ncols );
  }

  // Dump image to .ppm if desired
  if(dump_img){
    std::string str = std::string(file_name);
    if(src_alloc)
      str = str.substr( 0, str.length() - 4 );
    str = str + "-cmyk.tiff";
    dump_image( str.c_str(), &image_dest[0], nrows, ncols );
  }

  // Display execution time
  if(stats)
    std::cout << "Execution Time = " << timer.get_elapsed() << std::endl;

  return 0;
}
