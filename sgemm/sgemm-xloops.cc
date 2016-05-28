//------------------------------------------------------------------------
// sgemm_xloops
//------------------------------------------------------------------------

#include "sgemm-xloops.h"

extern void unordered_for();

void sgemm_xloops( float C[], float A[], float B[], int size )
{

  for ( int mm = 0; mm < size; ++mm, unordered_for() ) {
    for ( int nn = 0; nn < size; ++nn ) {

      float c = 0.0f;

      for ( int i = 0; i < size; ++i ) {
        float a = A[mm + i * size];
        float b = B[nn + i * size];
        c += a * b;
      }
      C[mm+nn*size] = c;

    }
  }

}

