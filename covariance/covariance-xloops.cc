#include "covariance-xloops.h"

extern void ordered_for();
extern void unordered_for();

namespace covariance {

  __attribute__((noinline))
    void covariance_xloops (int m, int n,
                            float float_n,
                            float data[M][N],
                            float symmat[M][M],
                            float mean[M]) {
    int i, j, j1, j2;

    /* Determine mean of column vectors of input data matrix */
    for (j = 0; j < M; j++, unordered_for()) {
      mean[j] = 0.0;
      for (i = 0; i < N; i++)
        mean[j] += data[i][j];
      mean[j] /= float_n;
    }

    /* Center the column vectors. */
    for (i = 0; i < N; i++, unordered_for())
      for (j = 0; j < M; j++)
        data[i][j] -= mean[j];

    /* Calculate the m * m covariance matrix. */
    for (j1 = 0; j1 < M; j1++)
      for (j2 = j1; j2 < M; j2++) {
        float temp = 0.0;
        for (i = 0; i < N; i++, ordered_for())
          temp += data[i][j1] * data[i][j2];
        symmat[j1][j2] = temp;
        symmat[j2][j1] = symmat[j1][j2];
      }
  }
}
