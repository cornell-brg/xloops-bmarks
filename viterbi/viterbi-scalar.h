//========================================================================
// viterbi-scalar.h
//========================================================================

#ifndef VITERBI_SCALAR_H
#define VITERBI_SCALAR_H

namespace viterbi {
  void viterbi_scalar(unsigned char symbols[], unsigned char msg[],
                      int poly[], int framebits);
}

#endif /* VITERBI_SCALAR_H */

