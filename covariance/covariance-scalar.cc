#include "covariance-scalar.h"

namespace covariance {

  __attribute__((noinline))
    void covariance_scalar(int m, int n,
                           float float_n,
                           float data[M][N],
                           float symmat[M][M],
                           float mean[M]) {
    int i, j, j1, j2;

    /* Determine mean of column vectors of input data matrix */
    for (j = 0; j < M; j++) {
      mean[j] = 0.0;
      for (i = 0; i < N; i++)
        mean[j] += data[i][j];
      mean[j] /= float_n;
    }

    /* Center the column vectors. */
    for (i = 0; i < N; i++)
      for (j = 0; j < M; j++)
        data[i][j] -= mean[j];

    /* Calculate the m * m covariance matrix. */
    for (j1 = 0; j1 < M; j1++)
      for (j2 = j1; j2 < M; j2++) {
        symmat[j1][j2] = 0.0;
        for (i = 0; i < N; i++)
          symmat[j1][j2] += data[i][j1] * data[i][j2];
        symmat[j2][j1] = symmat[j1][j2];
      }
  }
}
