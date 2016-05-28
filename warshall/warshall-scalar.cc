//========================================================================
// warshall-scalar.cc
//========================================================================

#include "warshall-scalar.h"
#include <string.h>

namespace warshall {

  __attribute__((noinline))
  void warshall_scalar( int n, float *path, float *path_in )
  {
    int i, j, k;

    // initially copy the input path to path
    memcpy( path, path_in, sizeof(float) * n * n );

    for (k = 0; k < n; k++)
    {
      for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
          path[i*n+j] = path[i*n+j] < path[i*n+k] + path[k*n+j] ?
                        path[i*n+j] : path[i*n+k] + path[k*n+j];
    }
  }
};
