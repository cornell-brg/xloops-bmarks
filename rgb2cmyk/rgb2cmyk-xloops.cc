//========================================================================
// rgb2cmyk-xloops.cc
//========================================================================

#include "rgb2cmyk-xloops.h"

#include <algorithm>

extern void unordered_for();

namespace rgb2cmyk {

  void rgb2cmyk_xloops( RgbPixel* image_src, CmykPixel* image_dest,
                        int nrows, int ncols )
  {
    int temp_c;
    int temp_m;
    int temp_y;
    int temp_k;
    int idx;

    for( int i = 0; i < nrows; i++, unordered_for() ){
      for( int j = 0; j < ncols; j++ ){

        // Calculate index
        idx = i*ncols + j;

        // Calculate intermediate values
        temp_c = 255 - image_src[idx].get_r();
        temp_m = 255 - image_src[idx].get_g();
        temp_y = 255 - image_src[idx].get_b();
        temp_k = std::min( temp_c, std::min( temp_m, temp_y ) );
        image_dest[idx].set_k(temp_k);

        // Store final values into result arrays
        image_dest[idx].set_c( temp_c - temp_k );
        image_dest[idx].set_m( temp_m - temp_k );
        image_dest[idx].set_y( temp_y - temp_k );
      }
    }
  }

}
