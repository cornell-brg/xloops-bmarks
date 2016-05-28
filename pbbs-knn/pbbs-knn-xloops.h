//========================================================================
// pbbs-knn-xloops.h
//========================================================================
// The interface for the xloops implementation of k-nearest neighbors

#ifndef PBBS_KNN_XLOOPS_H
#define PBBS_KNN_XLOOPS_H

#include "pbbs-knn-vertex.h"

void knn_xloops_2d ( vertex_2d** v, int n, int k );

void knn_xloops_3d ( vertex_3d** v, int n, int k );

#endif /*PBBS_KNN_XLOOPS_H*/
