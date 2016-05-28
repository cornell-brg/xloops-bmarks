//========================================================================
// rsort-scalar : Scalar implementation of radix sort
//========================================================================
// The source and destination arrays are assumed to be previously
// allocated to the given size. The radix_lg parameter is the log base
// two of the radix. The source integers are assumed to be all positive.

#ifndef RSORT_SCALAR_H
#define RSORT_SCALAR_H

namespace rsort {

  void rsort_scalar( int dest[], int src[], int size, int radix_lg = 4 );

}

#endif /* RSORT_SCALAR_H */

