//========================================================================
// pbbs-knn-xloops.cc
//========================================================================
// Given points in 2 and 3 dimensions, finds the k nearest neighbors for
// each point according to Eucledian distance. Uses a gTree structure
// which works as a quadtree in 2D and an octree in 3D to partition space
// and provide efficient neighbor queries for each point.
//
// This implementation uses XLOOPS to execute two hotspots in the code
// in parallel. The first parallelized loop takes care of updating
// the current search radius and the nearest neighbor found so far for
// each point. The second parallelized loop checks which quadrants at
// the current tree depth could contain a nearer neighbor than the
// nearest neighbor found so far.
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

#include <iostream>
#include <limits>
#include <cfloat>
#include "pbbs-common-sequence.h"
#include "pbbs-common-parallel.h"
#include "pbbs-common-octTree.h"
#include "pbbs-knn-xloops.h"
#define maxK 10  // the maximum number of neighbors the algorithm can
                 // keep track of

extern void unordered_for();
extern void mem_for();

namespace {

// A k-nearest neighbor structure.
// Contains a space-partitioning tree to divide the points for
// efficient neighbor lookups as well as a structure used
// to find the nearest neighbors to an individual point.
struct kNearestNeighbor {
  typedef vertex_2d vertex;
  typedef vertex_2d::pointT point;
  typedef point::vectT fvect;

  typedef gTreeNode<point,fvect,vertex> KNN_Tree;
  KNN_Tree *tree; // the quadtree/octree used to spatially partition
                  // the points

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
  
  // The structure used to search for the nearest neighbors to one point given a
  // built tree. Contains fields to keep track of the current nearest neighbors
  // as well as the current search radius. Also contains the methods required
  // to find the k-nearest neighbors to a point.
  struct kNN {
    vertex *query_pt;           // the element for which we are trying to find a NN
    vertex *current_knn[maxK];  // the current k nearest neighbors (nearest last)
    float current_radius[maxK]; // radius of current k nearest neighbors
    int quads;                  // branching factor of search tree
                                //   (4 for 2D quadtree, 8 for 3D octree)
    int k;                      // the number of neighbors to find for each point

    // Returns the ith smallest element (0 is smallest) up to k-1
    inline vertex* operator[] ( const int i )
    { 
      return current_knn[k-i-1];
    }

    // Constructor: sets kNN fields
    // p:  the point for which we want to find a nearest neighbor
    // kk: the number of neighbors to find
    inline kNN(vertex *p, int kk)
    {
      k = kk;
      quads = ( 1 << (p->pt).dimension() ); 
      query_pt = p;
     
      // Start search with no nearest neighbors and 
      // the maximum radius allowed
      float max_float = numeric_limits<float>::max();     
      for ( int i = 0; i < k; i++ ) {
        current_knn[i]    = NULL; 
        current_radius[i] = max_float;
      }
    } 

    //------------------------------------------------------------------------
    // Look for closer neighbors to query_pt in a specific (sub)tree T than
    // the neighbors already found so far
    //------------------------------------------------------------------------
      
    // Updates current search radius and best neighbors found so far based
    // on the input octree leaf. count is the number of points in that
    // leaf.
    __attribute__ ((noinline))
    void update_nearest_neighbor(KNN_Tree *T, int count)
    { 
      for ( int i = 0; i < count; i++, mem_for() ) {

        // Get a point p from the leaf
        vertex* p = T->vertices[i];
         
        if ( p != query_pt ) {

          // Find the distance to that point
          point opt = (p->pt);
          fvect diff = (query_pt->pt) - opt;

          // This function uses a custom version of the square root function
          // to ensure inlining by the compiler
          float dist = diff.Length();

          if ( dist < current_radius[0] ) {
            // If this point is closer than the closest point so far, update
            // the current search radius and the current nearest point
            current_knn[0]    = p;
            current_radius[0] = dist;
            int j = 1;
              
            // Maintain order of nearest neighbors found so far
            while ( j < k && current_radius[j-1] < current_radius[j]) {
              swap( current_radius[j-1], current_radius[j] );
              swap( current_knn[j-1],    current_knn[j]    );
              j++;
            }

          }

        }
      }
    }

    // Looks for nearest neighbors in boxes for which query_pt is not in.
    // Checks to see if the quadrant corresponding to the given node could
    // contain neighbors nearer to query_pt than those already found. If not,
    // doesn't bother checking the points in the node
    void nearestNghTrim(KNN_Tree *T)
    {
      if ( T->IsLeaf() ) {
        // Check all points in leaf   
        update_nearest_neighbor(T, T->count);
      } else {
        // Recurse to lower quadrants.         
        // check_child[m] is true iff child quadrant m can contain a nearer
        // neighbor than the current nearest neighbor. If check_child[m] is
        // false, knn will not bother searching that quadrant. This is what
        // makes having a space-partitioning data structure worthwile.
        bool check_child[4] = {false}; // initialize to false

        // Assembly hack to prevent the compiler from unrolling the following
        // loop. This way, we can run iterations in parallel.
        int quads_to_check = 4;

        #ifdef _MIPS_ARCH_MAVEN 
        __asm__ volatile ( "addiu %0,%1,0" : "=r" (quads_to_check):
                           "r" (quads_to_check));
        #endif

        // For each quadrant, determine whether a nearer neighbor than the
        // nearest neighbor found so far could be located there.
        for ( int j = 0; j < quads_to_check; j++, unordered_for() ) {
           float search_radius = (T->children[j]->size/2)+current_radius[0];
           if ( !(T->children[j]->center).outOfBox( query_pt->pt, search_radius) )
              check_child[j] = true;
        }
        
        // Only recurse for the quadrants which could contain a nearer
        // neighbor.
        for ( int j = 0; j < quads; j++ ) {
          if ( check_child[j] ) nearestNghTrim( T->children[j] );
        } 
      
      }
    }
     
    // Recurses down the tree until it reaches the leaf which contains the
    // query point. Checks the distances to all the points in that leaf first,
    // updating the nearest neighbor found so far and the current search radius.
    // Afterwards, it moves up the tree and calls nearestNghTrim on the quadrants
    // which do not contain the query point and which may contain a point closer
    // to the query point than the nearest neighbor found so far.
    void nearestNgh(KNN_Tree *T)
    {
      if ( T->IsLeaf() ) {
        // Check all points in leaf containing the query point
        update_nearest_neighbor(T, T->count); 
      } else {       
        // Recurse down to lower quadrant containing query_pt
        int i = T->findQuadrant( query_pt );
        nearestNgh( T->children[i] );
       
        // check_child[m] is true iff child quadrant m can contain a nearer
        // neighbor than the current nearest neighbor. If check_child[m] is
        // false, knn will not bother searching that quadrant. This is what
        // makes having a space-partitioning data structure worthwile.
        bool check_child[4] = {false}; // initialize to false
         
        // Assembly hack to prevent the compiler from unrolling the following
        // loop. This way, we can run iterations in parallel.
        int quads_to_check = 4;

        #ifdef _MIPS_ARCH_MAVEN
        __asm__ volatile ( "addiu %0,%1,0" : "=r" (quads_to_check):
                           "r" (quads_to_check)); 
        #endif      
  
        // For each quadrant, determine whether a nearer neighbor than the
        // nearest neighbor found so far could be located there. 
        for ( int j = 0; j < quads_to_check; j++, unordered_for() ) {
          float search_radius = (T->children[j]->size/2)+current_radius[0];
          if ( !(T->children[j]->center).outOfBox( query_pt->pt, search_radius ) )
            check_child[j] = true;
        } 
       
        // Only recurse for the quadrants which could contain a nearer
        // neighbor. Don't recurse on the quadrant which contains the query
        // point, because it has already been checked.
        for ( int j = 0; j < quads; j++ ) {
          if ( j != i && check_child[j] ) nearestNghTrim( T->children[j] );
        }

      }
    }
  };

  //------------------------------------------------------------------------
  // Find the nearest neighbors for 1 point
  //------------------------------------------------------------------------  

  // Returns the k nearest neighbors to point p
  // This version writes into result
  void kNearest(vertex *p, vertex** result, int k)
  { 
    
    kNN nn(p,k);  // create a search structure to find the k nearest
                  // neighbors to the given point

    nn.nearestNgh(tree); // use the search structure to find the nearest
                         // neighbors to the given point

    // Write the result into an array
    for ( int i = 0; i < k; i++ ){
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


} /* end namespace */

//------------------------------------------------------------------------
// Main loop: for each point, find k nearest neighbors
//------------------------------------------------------------------------
// Finds the k nearest neighbors for all points in tree
// Places pointers to them in the .ngh field of each vertex
//
// v: the input points
// n: the number of points
// k: the number of neighbors to find for each point

// 2-dimensional KNN
void knn_xloops_2d ( vertex_2d** v, int n, int k ) { 
  typedef kNearestNeighbor kNNT;

  // Build the quadtree for the input points
  kNNT T = kNNT(v, n);
 
  // Reorder the vertices for spatial locality
  vertex_2d** vr = T.order_vertices();
  
  // For all n points, find k nearest neighbors
  // This loop has large amounts of parallelism -- once the search tree
  // has been built, the k neighbors for each point can be found
  // independently. However, this loop is too large for xloops, which
  // is intended for more fine-grain parallelism.  
  for ( int i = 0; i < n; i++ ) { 
    T.kNearest( vr[i], vr[i]->ngh, k );
  }

  free(vr);
  T.del();
}

// 3-dimensional KNN: unsupported
void knn_xloops_3d ( vertex_3d** v, int n, int k ) {
/*
  typedef kNearestNeighbor<vertex_3d,10> kNNT;

  kNNT T = kNNT(v, n);

  // Reorder the vertices for spatial locality
  vertex_3d** vr = T.order_vertices();

  // For all n points, find k nearest neighbors
  for ( int i = 0; i < n; i++, unordered_for() ) { 
    T.kNearest( vr[i], vr[i]->ngh, k );
  }

  free(vr);
  T.del();
*/
} 



