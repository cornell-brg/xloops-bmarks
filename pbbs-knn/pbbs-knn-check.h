//========================================================================
// pbbs-knn-check.h
//========================================================================

#ifndef PBBS_KNN_CHECK_H
#define PBBS_KNN_CHECK_H

#include "pbbs-common-geometry.h"

//template <class pointT>
int check_neighbors(_seq<int> neighbors, _point2d<float>* P, int n, int k, int r,
                    const char* name);

#endif /*PBBS_KNN_CHECK_H*/
