//========================================================================
// kmeans-xloops-opt.h
//========================================================================
// shreesha: XLOOPS implementation where an alternative algorithm is used
// for better performance. Illustrates app tuning for XLOOPS architectures

#ifndef KMEANS_XLOOPS_OPT_H
#define KMEANS_XLOOPS_OPT_H

namespace kmeans {

  void stage0_xloops_opt( float c[], float ex[], int ex_2_cl[],
                  int ex_len, int c_len, int f_len );

  float stage1_xloops_opt( float c[], float ex[], int ex_2_cl[],
                   int ex_len, int c_len, int f_len );

  void stage2_xloops_opt( float c[], float ex[], int ex_2_cl[],
                  int ex_len, int c_len, int f_len, int nthreads );

  void kmeans_xloops_opt( float c[], float ex[], int ex_len,
                  int c_len, int f_len, float threshold, int nthreads );

}

#endif /* KMEANS_XLOOPS_OPT_H */

