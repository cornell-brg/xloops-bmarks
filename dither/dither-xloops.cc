//========================================================================
// dither-xloops.cc
//========================================================================

#include "dither-xloops.h"
#include <algorithm>

extern void unordered_for();

// Shorter name for byte type
namespace {
  typedef unsigned char byte;
}

namespace {

  //----------------------------------------------------------------------
  // dither_xloops_inner
  //----------------------------------------------------------------------

  // we actually want to inline this...
  //__attribute__ ((noinline))
  inline void dither_xloops_inner( byte dest[], byte src[],
                          int* error_in0, int* error_in1,
                          int* error_in2, int* error_out,
                          int ncols, int row_idx, int col_idx, int len )
  {

    for ( int i = 0; i < len; i++, unordered_for() ) {

      int tmp_row_idx = row_idx + i;
      int tmp_col_idx = col_idx - (2*i);
      int idx = tmp_row_idx*ncols + tmp_col_idx;

      int src_pixel = src[ idx ];
      int dest_pixel = 0x00u;
      int error;

      if ( src_pixel != 0x00u ) {

        int error_offset
          = ( 1 * error_in0[ row_idx + i + 0 ] )
          + ( 5 * error_in1[ row_idx + i + 0 ] )
          + ( 3 * error_in2[ row_idx + i + 0 ] )
          + ( 7 * error_in0[ row_idx + i + 1 ] );

        int out = src_pixel + ( error_offset / 16 );

        if ( out > 127 )
          dest_pixel = 0xffu;

        error = out - dest_pixel;
      }
      else
        error = 0;

      error_out[ row_idx + i + 1 ] = error;
      dest[ idx ] = dest_pixel;
    }
  }

}

namespace dither {

  //----------------------------------------------------------------------
  // dither_xloops
  //----------------------------------------------------------------------

  void dither_xloops( byte dest[], byte src[], int nrows, int ncols )
  {

    // Create input/output error buffers. We use gcc's extension which
    // enables dynamically allocating arrays on the stack.

    int error_a[ nrows + 2 ];
    int error_b[ nrows + 2 ];
    int error_c[ nrows + 2 ];
    int error_d[ nrows + 2 ];

    // Initialize error buffers to all zero. We should probably use an
    // XLOOP here so that we can essentially use vector stores.

    for ( int i = 0; i < ncols+2; i++ ) {
      error_a[i] = 0;
      error_b[i] = 0;
      error_c[i] = 0;
      error_d[i] = 0;
    }

    // We will rotate around the error buffers, so initialize pointers
    // to point to the current input and output error buffers.

    int* error_in0 = &error_a[0];
    int* error_in1 = &error_b[0];
    int* error_in2 = &error_c[0];
    int* error_out = &error_d[0];

    // First stage where we move the main thread across the top row

    for ( int col_idx = 0; col_idx < ncols; col_idx++ ) {
      int row_idx = 0;

      // Calculate number of threads we can stripe across diagonal

      int len = std::min ( ((col_idx/2) + 1), nrows );

      // The actual work is refactored into a helper function

      dither_xloops_inner( dest, src, error_in0, error_in1,
                         error_in2, error_out, ncols,
                         row_idx, col_idx, len );

      // Rotate the error buffers

      int* temp = error_out;
      error_out = error_in2;
      error_in2 = error_in1;
      error_in1 = error_in0;
      error_in0 = temp;
    }

    // Second stage where we move the main thread down the right side,
    // bouncing between col_idx = (ncols-1) and (ncols-2). Count is n
    // and not row_idx because we do this loop twice per row.

    for ( int n = 0; n < (nrows*2); n++ ) {

      // We spend two passes on each row

      int row_idx = n / 2 + 1;
      int col_idx = ncols - ((n+1) % 2) - 1;

      // Calculate number of VPs we can stripe across diagonal

      int len = std::min( (nrows - row_idx), ((col_idx / 2) + 1) );

      // The actual work is refactored into a helper function

      dither_xloops_inner( dest, src, error_in0, error_in1,
                         error_in2, error_out, ncols,
                         row_idx, col_idx, len );

      // Rotate the error buffers

      int* temp = error_out;
      error_out = error_in2;
      error_in2 = error_in1;
      error_in1 = error_in0;
      error_in0 = temp;
    }

  }

}

