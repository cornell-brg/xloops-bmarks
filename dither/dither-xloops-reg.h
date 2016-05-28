//========================================================================
// dither-xloopsreg : Tloops implementation of dither
//========================================================================
// The source and destination arrays are assumed to be previously
// allocated to the given number of rows and columns.

#ifndef DITHER_XLOOPS_REG_H
#define DITHER_XLOOPS_REG_H

namespace dither {

  void dither_xloops_reg( unsigned char dest[], unsigned char src[],
                          int nrows, int ncols );

}

#endif /* DITHER_XLOOPS_REG_H*/

