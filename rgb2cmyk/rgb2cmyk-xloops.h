//========================================================================
// rgb2cmyk-xloops : Tloops implementation of rgb2cmyk
//========================================================================
// The source and destination arrays are assumed to be previously
// allocated to the given number of rows and columns.

#ifndef RGB2CMYK_XLOOPS_H
#define RGB2CMYK_XLOOPS_H

#include "rgb2cmyk-PixelUtils.h"

namespace rgb2cmyk {

  void rgb2cmyk_xloops( RgbPixel* image_src, CmykPixel* image_dest,
                        int nrows, int ncols );

}

#endif /* RGB2CMYK_XLOOPS_H */

