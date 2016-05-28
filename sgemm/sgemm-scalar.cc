
//------------------------------------------------------------------------
// sgemm_scalar
//------------------------------------------------------------------------

#include "sgemm-scalar.h"

// shreesha: external function to tag loops which need a jump insertion
extern void insert_jump();

void sgemm_scalar( float C[], float A[], float B[], int size )
{

  // shreesha: inserting a jump instruction to redirect the control-flow to
  // the loop exit condition. Adding this specifically for the XLOOPS project
  // and it should not affect other projects. Additional, jump instruction
  // seems to influence the branch prediction for o3 executions using the
  // gem5 default tournament predictor and improves the performance of the
  // scalar code and matches the performance of traditional execution.
  for ( int mm = 0; mm < size; ++mm, insert_jump() ) {
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

