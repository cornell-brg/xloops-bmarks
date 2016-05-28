#include "dynprog-xloops.h"

extern void ordered_for();
extern void mem_for();

namespace dynprog {

  // c and W are length*length arrays, c and out are the outputs
  __attribute__((noinline))
  void dynprog_xloops( int length, int *c, int *W, int *out )
  {
    int out_l = 0;

    for (int i = 0; i < length - 1; i++)
    {
      // NOTE: we had to convert the inner loop to use a fake induction
      // variable t to calculate j. Otherwise the compiler fails to use
      // the correct xfor.X instruction, uses bne instead.

      //for ( int j = i + 1; j < length; j++, ordered_for() )
      for ( int t = 0; t < length - 1 - i; t++, mem_for() )
      {
        int j = t + i + 1;
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
