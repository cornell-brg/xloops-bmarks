//========================================================================
// rsort-xloops : Tloops implementation of radix sort
//========================================================================
// The source and destination arrays are assumed to be previously
// allocated to the given size. The radix_lg parameter is the log base
// two of the radix. The source integers are assumed to be all positive.

#ifndef RSORT_XLOOPS_H
#define RSORT_XLOOPS_H

namespace rsort {

  void rsort_xloops( int dest[], int src[], int size, int radix_lg );

}

#endif /* RSORT_XLOOPS_H */

