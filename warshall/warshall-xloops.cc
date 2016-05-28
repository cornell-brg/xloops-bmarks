//========================================================================
// warshall-xloops.cc
//========================================================================

#include "warshall-xloops.h"
#include <string.h>

extern void unordered_for();

namespace warshall {

  __attribute__((noinline))
  void warshall_xloops( int n, float *path, float *path_in )
  {
    int i, j, k;

    // initially copy the input path to path
    memcpy( path, path_in, sizeof(float) * n * n );

    for (k = 0; k < n; k++) // for.m or for.o
    {
      // NOTE: technically compiler probably can't find out that the
      // middle loop is a for.u; it would probably infer a for.m. But due
      // to application mechanics, it's safe to have this for.u.
      for (i = 0; i < n; i++, unordered_for()) // for.u or for.m
        for (j = 0; j < n; j++/*, unordered_for()*/) // for.u
          path[i*n+j] = path[i*n+j] < path[i*n+k] + path[k*n+j] ?
                        path[i*n+j] : path[i*n+k] + path[k*n+j];
    }
  }
};
