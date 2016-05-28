//========================================================================
// dither-scalar.cc
//========================================================================

#include "dither-scalar.h"
#include <algorithm>

extern void mem_for();

// Shorter name for byte type
namespace {
  typedef unsigned char byte;
}

namespace dither {

  void dither_xloops_mem( byte dest[], byte src[], int nrows, int ncols )
  {
    // Create input/output error buffers. We use gcc's extension which
    // enables dynamically allocating arrays on the stack.

    int error_arr[nrows+1][ncols+2];

    // Initialize error buffers to all zero.

    for ( int i = 0; i < nrows; i++ ) {
      for ( int j = 0; j < ncols+2; j++ ) {
        error_arr[i][j] = 0;
      }
    }

    // This will hold the cross-row (horizontal) error


    // Loop over the columns in each row

    for ( int row_idx = 0; row_idx < nrows; row_idx++, mem_for() ) {
      int error = 0;
      for ( int col_idx = 0; col_idx < ncols; col_idx++ ) {

        int  idx        = row_idx*ncols + col_idx;
        byte dest_pixel = 0x00u;

        // Only process pixel if it is non-white

        if ( src[idx] != 0x00u ) {

          int error_offset
            = ( 1 * error_arr[ row_idx ][ col_idx + 2 ] )
            + ( 5 * error_arr[ row_idx ][ col_idx + 1 ] )
            + ( 3 * error_arr[ row_idx ][ col_idx     ] )
            + ( 7 * error                               );

          int out = src[idx] + ( error_offset / 16 );

          if ( out > 127 )
            dest_pixel = 0xffu;

          error = out - dest_pixel;
        }
        else
          error = 0;

        error_arr[row_idx+1][col_idx+1] = error;
        dest[idx] = dest_pixel;
      }

    }
  }

}

