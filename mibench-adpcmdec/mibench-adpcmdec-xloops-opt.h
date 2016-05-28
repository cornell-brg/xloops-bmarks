//========================================================================
// mibench-adpcmdec-xloops.h
//========================================================================

#ifndef MIBENCH_ADPCMDEC_XLOOPS_OPT_H
#define MIBENCH_ADPCMDEC_XLOOPS_OPT_H

#include "mibench-adpcmdec-common.h"

namespace mibench_adpcmdec {

  void mibench_adpcmdec_xloops_opt( char indata[], short outdata[],
                                    int len, adpcm_state* state);

}

#endif  /* MIBENCH_ADPCMDEC_XLOOPS_OPT_H */
