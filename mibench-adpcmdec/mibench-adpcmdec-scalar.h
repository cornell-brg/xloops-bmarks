//========================================================================
// mibench-adpcmdec-scalar.h
//========================================================================

#ifndef MIBENCH_ADPCMDEC_SCALAR_H
#define MIBENCH_ADPCMDEC_SCALAR_H

#include "mibench-adpcmdec-common.h"

namespace mibench_adpcmdec {

  void mibench_adpcmdec_scalar ( char indata[], short outdata[],
                                 int len, adpcm_state* state);

}

#endif  /* MIBENCH_ADPCMDEC_SCALAR_H */
