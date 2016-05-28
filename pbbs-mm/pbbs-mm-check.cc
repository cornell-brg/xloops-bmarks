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


#include <iostream>
#include <algorithm>
#include <cstring>

#include "pbbs-mm-check.h"

// Checks for every vertex if locally maximally matched
int check(edgeArray<int> *EA, int* EI, int nMatches, const char *name) {
  edge<int>* E = EA->E;
  int m = EA->nonZeros;
  int n = max(EA->numCols,EA->numRows);
  int* V = newA(int, n);
  parallel_for (int i=0; i < n; i++) V[i] = -1;
  bool *flags = newA(bool,m);
  parallel_for (int i=0; i < m; i++) flags[i] = 0;

  parallel_for (int i=0; i < nMatches; i++) {
    int idx = EI[i];
    V[E[idx].u] = V[E[idx].v] = idx;
    flags[idx] = 1;
  }

  for (int i=0; i < m; i++) {
    int u = E[i].u;
    int v = E[i].v;
    if (flags[i]) {
      if (V[u] != i) {
        std::cout << "  [ FAILED ] " << name << " : "
                  << "edges share vertex " << u << std::endl;
        return 1;
      }
      if (V[v] != i) {
        std::cout << "  [ FAILED ] " << name << " : "
                  << "edges share vertex " << v << std::endl;
        return 1;
      }
    } else {

      if (u != v && V[u] == -1 && V[v] == -1) {
        std::cout << "  [ FAILED ] " << name << " : "
                  << " neither endpoint matched for edge "
                  << i << std::endl;
        return 1;
      }
    }
  }

  free(V);
  free(flags);

  std::cout << "  [ passed ] " << name << std::endl;
  return 0;
}

#ifdef NATIVE_COMPILE
int main(int argc, char* argv[]) {
  commandLine P(argc,argv,"<impl> <inFile> <outfile>");
  pair<char*,char*> fnames = P.IOFileNames();
  char *impl_name = P.getArgument(2);

  std::cout << fnames.first << " " << fnames.second << std::endl;

  edgeArray<int> E = benchIO::readEdgeArrayFromFile<int>(fnames.first);
  _seq<int> Out = benchIO::readIntArrayFromFile<int>(fnames.second);
  return check(&E, Out.A, Out.n, impl_name);
}
#endif
