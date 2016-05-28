//========================================================================
// kmeans-xloops.h
//========================================================================

#ifndef KMEANS_XLOOPS_H
#define KMEANS_XLOOPS_H

namespace kmeans {

  void stage0_xloops( float c[], float ex[], int ex_2_cl[],
                  int ex_len, int c_len, int f_len );

  float stage1_xloops( float c[], float ex[], int ex_2_cl[],
                   int ex_len, int c_len, int f_len );

  void stage2_xloops( float c[], float ex[], int ex_2_cl[],
                  int ex_len, int c_len, int f_len, int nthreads );

  void kmeans_xloops( float c[], float ex[], int ex_len,
                  int c_len, int f_len, float threshold );

}

#endif /* KMEANS_XLOOPS_H */

