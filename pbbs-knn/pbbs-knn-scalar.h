//========================================================================
// pbbs-knn-scalar.h
//========================================================================
// The interface for k-nearest neighbors

#ifndef PBBS_KNN_SCALAR_H
#define PBBS_KNN_SCALAR_H

#include "pbbs-common-nearestNeighbors.h"
#include "pbbs-knn-vertex.h"

void knn_scalar_2d ( vertex_2d** v, int n, int k );
void knn_scalar_3d ( vertex_3d** v, int n, int k );

#endif /*PBBS_KNN_SCALAR_H*/
