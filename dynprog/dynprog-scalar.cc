#include "dynprog-scalar.h"

namespace dynprog {

  // c and W are length*length arrays, c and out are the outputs
  __attribute__((noinline))
  void dynprog_scalar( int length, int *c, int *W, int *out )
  {
    int out_l = 0;

    for (int i = 0; i < length - 1; i++)
    {
      for (int j = i + 1; j < length; j++)
      {
        int s = 0;
        for (int k = i + 1; k < j; k++)
          s += c[i * length + k] + c[k * length + j];
        c[i * length + j] = s + W[i * length + j];
      }
      out_l += c[length - 1];
    }

    *out = out_l;
  }
};
