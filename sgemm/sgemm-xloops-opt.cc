//------------------------------------------------------------------------
// sgemm_xloops_opt
//------------------------------------------------------------------------

#include "sgemm-xloops-opt.h"

extern void unordered_for();

#ifdef _MIPS_ARCH_MAVEN
void sgemm_xloops_opt( float C[], float A[], float B[], int size )
{

  __asm__ __volatile__
  (
    "addiu %[C], %[C], -4\n"
    : [C] "+r" (C)
    :
  );

  __asm__ __volatile__
  (
    "addiu %[A], %[A], -4\n"
    : [A] "+r" (A)
    :
  );

  for ( int mm = 0; mm < size; ++mm, unordered_for() ) {
    __asm__ __volatile__
    (
      "addiu.xi %[C], %[C], 4\n"
      : [C] "+r" (C)
      :
    );
    __asm__ __volatile__
    (
      "addiu.xi %[A], %[A], 4\n"
      : [A] "+r" (A)
      :
    );
    for ( int nn = 0; nn < size; ++nn ) {

      float c = 0.0f;

      for ( int i = 0; i < size; ++i ) {
        float a = *(A + i * size);
        float b = B[nn + i * size];
        c += a * b;
      }

      *(C+nn*size) = c;

    }
  }

}
#else
void sgemm_xloops_opt( float C[], float A[], float B[], int size )
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
#endif
