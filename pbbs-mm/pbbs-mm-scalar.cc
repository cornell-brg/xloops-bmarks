//========================================================================
// pbbs-mm-scalar.cc
//========================================================================
// Given an undirected graph, maximal matching finds a matching (i.e.,
// a set of pairwise non-adjacent edges) for which every edge in the
// graph has an intersection with at least one of the edges in the
// matching. The maximal matching can be thought of as a minimum subset
// of the edges in the graph which intersect with every edge in the
// graph.
//
// This code is part of the Problem Based Benchmark Suite (PBBS)
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

#include "pbbs-mm-scalar.h"

//------------------------------------------------------------------------
// Utility functions
//------------------------------------------------------------------------

namespace {

  int maximal_matching( edge<int>* edges, int* vertices,
                        int* out, int num_edges ) {
    int k = 0;

    for ( int i = 0; i < num_edges; i++ ) {

      // Endpoints of this edge

      int v = edges[i].v;
      int u = edges[i].u;

      // vertices array initially holds -1s, and the opposite connected
      // vertex is stored in its place

      if ( ( vertices[v] < 0 ) && ( vertices[u] < 0 ) ) {

        vertices[v] = u;
        vertices[u] = v;
        out[k++]    = i;

      }

    }

    return k;

  }

}

//------------------------------------------------------------------------
// Main computation function
//------------------------------------------------------------------------

// Finds a maximal matching of the graph. Returns cross pointers between
// vertices, or -1 if unmatched.

void matching_scalar( int *out, int *size, edgeArray<int> *edge_array ) {

  // Parse graph properties

  int num_edges    = edge_array->nonZeros;
  int num_vertices = edge_array->numRows; // highest indexed vertex

  // Initialize array of vertices of edges in maximal matching

  int* vertices = newA( int, num_vertices );

  for ( int i = 0; i < num_vertices; i++ )
    vertices[i] = -1;

  // Return all the edges in the maximal matching in out as well as the
  // number of edges in size.

  *size = maximal_matching( edge_array->E, vertices, out, num_edges );

  free( vertices );

}
