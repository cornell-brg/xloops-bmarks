//========================================================================
// nearestNeighbors.h
//========================================================================
// Given points in 2 and 3 dimensions, finds the k nearest neighbors for
// each point according to Eucledian distance. Uses an gTree structure
// which works as a quadtree in 2D and an octree in 3D to partition space
// and provide efficient neighbor queries for each point.
//
// This benchmark is taken from the Problem Based Benchmark Suite (PBBS).
//
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef PBBS_COMMON_NN
#define PBBS_COMMON_NN

#include <iostream>
#include <limits>
#include "pbbs-common-sequence.h"
#include "pbbs-common-parallel.h"
#include "pbbs-common-octTree.h"
//using namespace std;

namespace {

// A k-nearest neighbor structure
// Requires vertexT to have pointT and vectT typedefs
template <class vertexT, int maxK>
struct kNearestNeighbor {
  typedef vertexT vertex;
  typedef typename vertexT::pointT point;
  typedef typename point::vectT fvect;

  typedef gTreeNode<point,fvect,vertex> KNN_Tree;
  KNN_Tree *tree; // the octree used to spacially partition the points

  // Generates the KNN octree search structure
  // vertices: the vertices to search
  // n: the number of vertices
  kNearestNeighbor(vertex** vertices, int n)
  {
    tree = KNN_Tree::gTree(vertices, n);
  }

  // Returns the vertices in the search structure in an
  // order that has spacial locality
  vertex** order_vertices()
  {
    return tree->flatten();
  }

  // Deletes the tree
  void del() {
    tree->del();
  }

  struct kNN {
    vertex *query_pt;             // the element for which we are trying to find a NN
    vertex *current_knn[maxK];    // the current k nearest neighbors (nearest last)
    float current_radius[maxK];   // radius of current k nearest neighbors
    int quads;                    // branching factor of search tree
                                  //   (4 for 2D quadtree, 8 for 3D octree)
    int k;                        // the number of neighbors to find

    // Returns the ith smallest element (0 is smallest) up to k-1
    vertex* operator[] ( const int i )
    {
      return current_knn[k-i-1];
    }

    // Sets kNN fields
    // p:  the element for which we want to find a NN
    // kk: the number of neighbors to find
    kNN(vertex *p, int kk)
    {
      if ( kk > maxK ) {
        cout << "k too large in kNN" << endl;
        abort();
      }

      k = kk;
      quads = ( 1 << (p->pt).dimension() );
      query_pt = p;

      // Start search with no nearest neighbors and
      // the maximum radius allowed
      for (int i=0; i<k; i++) {
        current_knn[i]    = (vertex*) NULL;
        current_radius[i] = numeric_limits<float>::max();
      }
    }

    // If point p is closer than current_knn then swap it in
    void update(vertex *p)
    {
      point opt = (p->pt);
      fvect v = (query_pt->pt) - opt;
      float r = v.Length();

      if ( r < current_radius[0] ) {
        current_knn[0]    = p;
        current_radius[0] = r;
        for ( int i = 1; i < k && current_radius[i-1] < current_radius[i]; i++) {
          swap( current_radius[i-1], current_radius[i] );
          swap( current_knn[i-1],    current_knn[i]    );
        }
      }

    }


  //------------------------------------------------------------------------
  // Look for closer neighbors to query_pt in a specific (sub)tree T than
  // the neighbors already found so far
  //------------------------------------------------------------------------

    // Looks for nearest neighbors in boxes for which query_pt is not in.
    // Checks to see if the quadrant corresponding to the given node could
    // contain neighbors nearer to query_pt than those already found. If not,
    // doesn't bother checking the points in the node
    void nearestNghTrim(KNN_Tree *T)
    {
      // First check to see if it's worth looking in this quadrant
      // This is what makes using a space partitioning tree worthwhile
      if ( !(T->center).outOfBox(query_pt->pt, (T->size/2)+current_radius[0]) ) {
        if ( T->IsLeaf() )    // check all points in leaf
          for ( int i = 0; i < T->count; i++ ) {
            update( T->vertices[i] );
          }
        else {              // recurse to lower quadrants
          for ( int j=0; j < quads; j++ ) {
            nearestNghTrim( T->children[j] );
          }
        }
      }
    }

    // Looks for nearest neighbors in the box for which query_pt is in.
    void nearestNgh(KNN_Tree *T)
    {
      if ( T->IsLeaf() )      // check all points in leaf
        for ( int i = 0; i < T->count; i++ ) {
          vertex *candidate_pt = T->vertices[i];
          if ( candidate_pt != query_pt )
            update( candidate_pt );
        }
      else {         // recurse to lower quadrant containing query_pt
        int i = T->findQuadrant( query_pt );
        nearestNgh( T->children[i] );
        for (int j=0; j < quads; j++)
          if (j != i) nearestNghTrim( T->children[j] );
      }
    }
  };

  //------------------------------------------------------------------------
  // Find the nearest neighbors for 1 point
  //------------------------------------------------------------------------

  // Returns the one nearest neighbor to point p
  vertex* nearest(vertex *p)
  {
    kNN nn(p,1);
    nn.nearestNgh(tree);
    return nn[0];
  }

  // Returns the k nearest neighbors to point p
  // This version writes into result
  void kNearest(vertex *p, vertex** result, int k)
  {
    kNN nn(p,k);
    nn.nearestNgh(tree);
    for (int i=0; i < k; i++) {
      result[i] = 0;
    }
    for (int i=0; i < k; i++) {
      result[i] = nn[i];
    }
  }

  // Returns the k nearest neighbors to point p
  // This version allocates the result
  vertex** kNearest(vertex *p, int k)
  {
    vertex** result = newA(vertex*,k);
    kNearest(p,result,k);
    return result;
  }

};

// Ji: moved this block to the actual implementation of each algorithm so
// the parallel_for is pulled out, this makes it easier to separate
// scalar and mt implementations.
//
////------------------------------------------------------------------------
//// Main loop: for each point, find k nearest neighbors
////------------------------------------------------------------------------
//
//// Finds the k nearest neighbors for all points in tree
//// Places pointers to them in the .ngh field of each vertex
//template <int maxK, class vertexT>
//void ANN(vertexT** v, int n, int k) {
//  typedef kNearestNeighbor<vertexT,maxK> kNNT;
//
//  kNNT T = kNNT(v, n);
//
//  // Reorder the vertices for spatial locality
//  vertexT** vr = T.order_vertices();
//
//  // Find nearest k neighbors for each point
//  parallel_for_1 (int i=0; i < n; i++)
//    T.kNearest( vr[i], vr[i]->ngh, k );
//
//  free(vr);
//  T.del();
//}

}

#endif PBBS_COMMON_NN
