//========================================================================
// symm-scalar.h
//========================================================================

#ifndef SYMM_SCALAR_H
#define SYMM_SCALAR_H

#include "symm-common.h"

namespace symm {

  void symm_scalar(int, int, DATA_TYPE, DATA_TYPE, DATA_TYPE[NI][NJ], DATA_TYPE[NJ][NJ], DATA_TYPE[NI][NJ]);

}

#endif /* SYMM_SCALAR_H */ 
