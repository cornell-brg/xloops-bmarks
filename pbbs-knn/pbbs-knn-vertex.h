//========================================================================
// pbbs-knn-vertex.h
//========================================================================
// The 2d and 3d point data structures for keeping track of the k-nearest
// neighbors to each point.

#ifndef PBBS_KNN_VERTEX_H
#define PBBS_KNN_VERTEX_H

#include "pbbs-common-geometry.h"

struct vertex_2d {
  typedef point2d pointT;
  int identifier;
  pointT pt;           // the point itself
  vertex_2d* ngh[10];  // the list of neighbors
  vertex_2d(pointT p, int id) : pt(p), identifier(id) {}
};

struct vertex_3d {
  typedef point3d pointT;
  int identifier;
  pointT pt;           // the point itself
  vertex_3d* ngh[10];  // the list of neighbors
  vertex_3d(pointT p, int id) : pt(p), identifier(id) {}
};

#endif /*PBBS_KNN_VERTEX_H*/

