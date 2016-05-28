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

#include "xloops-common-memset.h"
#include "pbbs-mm-xloops.h"
#include <cstring>

extern void mem_for(); // for.m
extern void ordered_for(); // for.r
extern void unordered_for(); // for.u
extern void parallel_for(); // for.o

namespace {

  int maximal_matching( edge<int>* edges, int* vertices,
                        int* out, int num_edges ) {

    int k = 0;

    // enable stat 2
    //__asm__ volatile ( "stat 0x12" );

    // note that pbbs typedefs parallel_for, so adding empty function for
    // this at pbbs-mm.cc causes problems. Instead, we'll use a different
    // for loop for native compile. TODO: fix this once we use a different
    // tag

    // Ji: commenting this out for now since it's causing issues during
    // compilation, PLEASE UNCOMMENT IF GOING BACK TO XLOOPS!
    #ifdef _MIPS_ARCH_MAVEN
    // note that it is possible to encode this as a for.m/a and a final
    // fixup in for.r, but that is much slower
//    for ( int i = 0; i < num_edges; i++, parallel_for() ) {
    for ( int i = 0; i < num_edges; i++ ) {
    #else
    for ( int i = 0; i < num_edges; i++ ) {
    #endif
      // two vertices that this edge connects
      int v = edges[i].v;
      int u = edges[i].u;
      // vertices array initially holds -1s, and the opposite connected
      // vertex is stored in its place
      if (vertices[v] < 0 && vertices[u] < 0) {
        vertices[v] = u;
        vertices[u] = v;
        // the k below creates a register dependency. instead we could use
        // matched_edges bitmask-like array to mark inclusion, and have a
        // separate for.r loop to put these to the out array
        out[k++] = i;
        //matched_edges[i] = 1;
      }
    }
    // disable stat 2
    //__asm__ volatile ( "stat 0x02" );

    // the following is an alternate implementation that splits for.o into
    // for.m and for.r

    // enable stat 3
    //__asm__ volatile ( "stat 0x13" );

    //// put edges to out array
    //for ( int i = 0; i < num_edges; i++ , ordered_for() ) {
    //  if ( matched_edges[i] )
    //    out[k++] = i;
    //}

    //// disable stat 3
    //__asm__ volatile ( "stat 0x03" );

    return k;

  }

}

// Finds a maximal matching of the graph
// Returns cross pointers between vertices, or -1 if unmatched
void matching_xloops( int *out, int *size, edgeArray<int> *edge_array ) {

  // enable stat 1
  //__asm__ volatile ( "stat 0x11" );

  // number of edges (m)
  int num_edges    = edge_array->nonZeros;
  // higest indexed vertex, i.e., the number of vertices (n)
  int num_vertices = edge_array->numRows;

  int* vertices = newA(int, num_vertices);
  // the matched edges is for the alternative implementation which has a
  // for.m and a for.r loop
  //int* matched_edges = newA(int, num_edges);

  // we use specialized xloops memset
  memset_xloops( vertices, -1, num_vertices * sizeof(int) );

  //memset_xloops( matched_edges, 0, num_edges * sizeof(int) );


  // disable stat 1
  //__asm__ volatile ( "stat 0x01" );

  // size is the number of total matching
  // out contains all of the matchings
  *size = maximal_matching( edge_array->E, vertices, out, num_edges);

  free(vertices);

}

