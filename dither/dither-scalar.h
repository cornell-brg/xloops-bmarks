//========================================================================
// dither-scalar : Scalar implementation of dither
//========================================================================
// The source and destination arrays are assumed to be previously
// allocated to the given number of rows and columns.

#ifndef DITHER_SCALAR_H
#define DITHER_SCALAR_H

namespace dither {

  void dither_scalar( unsigned char dest[], unsigned char src[],
                      int nrows, int ncols );

}

#endif /* DITHER_SCALAR_H */

