//========================================================================
// rgb2cmyk-PixelUtils.cc
//========================================================================

#include "rgb2cmyk-PixelUtils.h"

namespace rgb2cmyk {

  RgbPixel::RgbPixel() : m_r(0), m_g(0), m_b(0) { }
  RgbPixel::RgbPixel( byte r, byte g, byte b ) :
                         m_r(r), m_g(g), m_b(b) { }

  /* shreesha: Defining all the setters and getters in the header file
     will inline these functions else the scalar code generated would
     see function call references */

  //byte RgbPixel::get_r() { return m_r; }
  //byte RgbPixel::get_g() { return m_g; }
  //byte RgbPixel::get_b() { return m_b; }

  //void RgbPixel::set_r( byte r ) { m_r = r; }
  //void RgbPixel::set_g( byte g ) { m_g = g; }
  //void RgbPixel::set_b( byte b ) { m_b = b; }

  CmykPixel::CmykPixel() : m_c(0), m_m(0), m_y(0), m_k(0) { }
  CmykPixel::CmykPixel( byte c, byte m, byte y, byte k ) :
                           m_c(c), m_m(m), m_y(y), m_k(k) { }

  //byte CmykPixel::get_c() { return m_c; }
  //byte CmykPixel::get_m() { return m_m; }
  //byte CmykPixel::get_y() { return m_y; }
  //byte CmykPixel::get_k() { return m_k; }

  //void CmykPixel::set_c( byte c ) { m_c = c; }
  //void CmykPixel::set_m( byte m ) { m_m = m; }
  //void CmykPixel::set_y( byte y ) { m_y = y; }
  //void CmykPixel::set_k( byte k ) { m_k = k; }

}
