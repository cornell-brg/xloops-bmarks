#include "symm-scalar.h"
#include "stdio.h"

namespace symm {

  __attribute__((noinline))
    void symm_scalar(int ni, int nj,
                     DATA_TYPE alpha,
                     DATA_TYPE beta,
                     DATA_TYPE C[NI][NJ],
                     DATA_TYPE A[NJ][NJ],
                     DATA_TYPE B[NI][NJ])
    {
      int i, j, k;
      DATA_TYPE acc;

      /*  C := alpha*A*B + beta*C, A is symetric */
      for (i = 0; i < NI; i++)
        for (j = 0; j < NJ; j++)
        {
          acc = 0;
          for (k = 0; k < j - 1; k++)
          {
            C[k][j] += alpha * A[k][i] * B[i][j];
            acc += B[k][j] * A[k][i];
          }
          C[i][j] = beta * C[i][j];
          C[i][j] += alpha * A[i][i] * B[i][j];
          C[i][j] += alpha * acc;
        }
    }
};
