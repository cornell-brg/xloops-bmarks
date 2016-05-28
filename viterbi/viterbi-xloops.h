//========================================================================
// viterbi-xloops.h
//========================================================================

#ifndef VITERBI_XLOOPS_H
#define VITERBI_XLOOPS_H

namespace viterbi {
  void viterbi_xloops(unsigned char symbols[], unsigned char msg[],
                      int poly[], int framebits);
}

#endif /* VITERBI_XLOOPS_H*/

