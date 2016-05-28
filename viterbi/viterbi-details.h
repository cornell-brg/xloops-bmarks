//========================================================================
// viterbi-details.h : Implementation header
//========================================================================

#ifndef VITERBI_DETAILS_H
#define VITERBI_DETAILS_H

namespace viterbi {
namespace details {

// constraint length (number of memory registers) 
// implementation requires K <= 8
const int K = 7;
// number of symbol bits per input bit
const int rate = 2;
// number of possible decoder states
const int STATES (1 << (K-1));
//#define RENORMALIZE_THRESHOLD 2000000000

}}

#endif /* VITERBI_DETAILS_H */
