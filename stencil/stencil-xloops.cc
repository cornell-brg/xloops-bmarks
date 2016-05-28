#include "stencil-xloops.h"
extern void ordered_for();
extern void parallel_for();

namespace stencil {

  __attribute__((noinline))
    void stencil_xloops (int tsteps, int n, DATA_TYPE A[N][N])
    {
      int t, i, j;
      for (t = 0; t <= _PB_TSTEPS - 1; t++)
        for (i = 1; i<= _PB_N - 2; i++)
          // for (j = 1; j <= _PB_N - 2; j++)
          for (int k = 0; k < _PB_N - 2; k++, parallel_for()) {
            j = k+1;
            A[i][j] = (A[i-1][j-1] + A[i-1][j] + A[i-1][j+1]
                       + A[i][j-1] + A[i][j] + A[i][j+1]
                       + A[i+1][j-1] + A[i+1][j] + A[i+1][j+1])/9.0;
          }
    }

};
