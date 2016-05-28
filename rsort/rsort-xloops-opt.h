//========================================================================
// rsort-xloops-opt : Tloops optimized implementation of radix sort
//========================================================================
// The source and destination arrays are assumed to be previously
// allocated to the given size. The radix_lg parameter is the log base
// two of the radix. The source integers are assumed to be all positive.

// shreesha: XLOOPS implementation where an alternative algorithm is used
// for better performance. Illustrates app tuning for XLOOPS architectures

#ifndef RSORT_XLOOPS_OPT_H
#define RSORT_XLOOPS_OPT_H

namespace rsort {

  void rsort_xloops_opt( int dest[], int src[], int size, int radix_lg,
                         int nthreads );

}

#endif /* RSORT_XLOOPS_OPT_H */

