//========================================================================
// kmeans-scalar.h
//========================================================================

#ifndef KMEANS_SCALAR_H
#define KMEANS_SCALAR_H

namespace kmeans {

  void stage0_scalar( float c[], float ex[], int ex_2_cl[],
                      int ex_len, int c_len, int f_len );

  float stage1_scalar( float c[], float ex[], int ex_2_cl[],
                       int ex_len, int c_len, int f_len );

  void stage2_scalar( float c[], float ex[], int ex_2_cl[],
                      int ex_len, int c_len, int f_len );

  void kmeans_scalar( float c[], float ex[], int ex_len,
                      int c_len, int f_len, float threshold );

}

#endif /* KMEANS_SCALAR_H */

