//========================================================================
// dither-scalar.cc
//========================================================================

#include "dither-scalar.h"
#include <algorithm>

// Shorter name for byte type
namespace {
  typedef unsigned char byte;
}

namespace dither {

  void dither_scalar( byte dest[], byte src[], int nrows, int ncols )
  {
    // Create input/output error buffers. We use gcc's extension which
    // enables dynamically allocating arrays on the stack.

    int error_a[ncols+2];
    int error_b[ncols+2];

    // Initialize error buffers to all zero.

    for ( int i = 0; i < ncols+2; i++ ) {
      error_a[i] = 0;
      error_b[i] = 0;
    }

    // We will ping-pong between the two error buffers, so initialize
    // two pointers to point to the current input and output error
    // buffer.

    int* error_in  = &error_a[0];
    int* error_out = &error_b[0];

    // This will hold the cross-row (horizontal) error

    int error = 0;

    // Loop over the columns in each row

    for ( int row_idx = 0; row_idx < nrows; row_idx++ ) {
      for ( int col_idx = 0; col_idx < ncols; col_idx++ ) {

        int  idx        = row_idx*ncols + col_idx;
        byte dest_pixel = 0x00u;

        // Only process pixel if it is non-white

        if ( src[idx] != 0x00u ) {

          int error_offset
            = ( 1 * error_in[ col_idx + 2 ] )
            + ( 5 * error_in[ col_idx + 1 ] )
            + ( 3 * error_in[ col_idx     ] )
            + ( 7 * error                   );

          int out = src[idx] + ( error_offset / 16 );

          if ( out > 127 )
            dest_pixel = 0xffu;

          error = out - dest_pixel;
        }
        else
          error = 0;

        error_out[col_idx+1] = error;
        dest[idx] = dest_pixel;
      }

      // Swap the input/output error buffers

      std::swap( error_in, error_out );
    }
  }

}

