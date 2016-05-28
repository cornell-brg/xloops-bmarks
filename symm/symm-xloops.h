//========================================================================
// symm-scalar.h
//========================================================================

#ifndef SYMM_XLOOPS_H
#define SYMM_XLOOPS_H

#include "symm-common.h"

namespace symm {

  void symm_xloops(int, int, DATA_TYPE, DATA_TYPE, DATA_TYPE[NI][NJ], DATA_TYPE[NJ][NJ], DATA_TYPE[NI][NJ]);
}

#endif /* SYMM_XLOOPS_H */
