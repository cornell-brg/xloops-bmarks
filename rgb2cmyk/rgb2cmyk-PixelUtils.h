//========================================================================
// rgb2cmyk-PixelUtils.h
//========================================================================

#ifndef RGB2CMYK_PIXELUTILS_H
#define RGB2CMYK_PIXELUTILS_H
#include <iostream>

typedef unsigned char byte;

namespace rgb2cmyk {
  class RgbPixel
  {
    public:
      RgbPixel();
      RgbPixel( byte r, byte g, byte b );

      byte get_r(){ return m_r; }
      byte get_g(){ return m_g; }
      byte get_b(){ return m_b; }

      void set_r( byte r ){ m_r = r; }
      void set_g( byte g ){ m_g = g; }
      void set_b( byte b ){ m_b = b; }

      byte m_r, m_g, m_b;

    private:
  };

  class CmykPixel
  {
    public:
      CmykPixel();
      CmykPixel( byte c, byte m, byte y, byte k );

      byte get_c(){ return m_c; }
      byte get_m(){ return m_m; }
      byte get_y(){ return m_y; }
      byte get_k(){ return m_k; }

      void set_c( byte c ){ m_c = c; }
      void set_m( byte m ){ m_m = m; }
      void set_y( byte y ){ m_y = y; }
      void set_k( byte k ){ m_k = k; }

      byte m_c, m_m, m_y, m_k;

    private:
  };
}
#endif
