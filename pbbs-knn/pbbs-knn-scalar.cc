//========================================================================
// pbbs-knn-scalar.cc
//========================================================================
// Given points in 2 and 3 dimensions, finds the k nearest neighbors for
// each point according to Eucledian distance. Uses a gTree structure
// which works as a quadtree in 2D and an octree in 3D to partition space
// and provide efficient neighbor queries for each point.
//
// This scalar implementation of k-nearest neighbors calls the KNN
// implementation from PBBS. There are two functions, one for two
// dimensions and another for three dimensions.  The only difference is
// the type of the input points.

#include "pbbs-knn-scalar.h"

//------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------

// Artificial neural network which finds the k nearest neighbors for all
// points in tree. Places pointers to them in the ngh field of each
// vertex.

template < int maxK, class vertexT >
void ANN_scalar( vertexT** v, int n, int k ) {

  kNearestNeighbor<vertexT,maxK> T = kNearestNeighbor<vertexT,maxK>( v, n );

  // Reorder the vertices for spatial locality

  vertexT** vr = T.order_vertices();

  // Find nearest k neighbors for each point

  for ( int i = 0; i < n; i++ )
    T.kNearest( vr[i], vr[i]->ngh, k );

  free( vr );
  T.del();

}

//------------------------------------------------------------------------
// Main computation function
//------------------------------------------------------------------------
// v: the input points
// n: the number of points
// k: the number of neighbors to find for each point

// 2-dimensional KNN

void knn_scalar_2d ( vertex_2d** v, int n, int k ) {
    ANN_scalar<10>( v, n, k );
}

// 3-dimensional KNN

void knn_scalar_3d ( vertex_3d** v, int n, int k ) {
    ANN_scalar<10>( v, n, k );
}
