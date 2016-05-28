//========================================================================
// sha-scalar.h
//========================================================================

#ifndef SHA_SCALAR_H
#define SHA_SCALAR_H

#include "sha-common.h"

namespace sha {

  void sha_scalar( sha_struct* sha_info, BYTE* src, int size );

}

#endif  /* SHA_SCALAR_H */
