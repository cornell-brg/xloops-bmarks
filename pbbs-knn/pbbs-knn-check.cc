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
#include "float.h"
#include <algorithm>
#include <cstring>
#include "pbbs-common-parallel.h"
#include "pbbs-common-geometry.h"
#include "pbbs-common-sequence.h"
#include "pbbs-common-geometryIO.h"
#include "pbbs-common-parseCommandLine.h"

#include "pbbs-knn-check.h"

using namespace benchIO;

//template <class pointT>
int check_neighbors(_seq<int> neighbors, _point2d<float>* P, int n, int k, int r,
                    const char *name) { 
  if (neighbors.n != k * n) {
    std::cout << "  [ FAILED ] " << name << ": wrong length, n = " << n 
	 << " k = " << k << " neighbors = " << neighbors.n << std::endl;
    return 1;
  }

  for (int j = 0; j < r; j++) {
    int jj = utils::hash(j) % n;
    //std::cout << "Checking random point " << jj << std::endl;
    float* distances = newA(float,n);
    parallel_for ( int i = 0; i < n; i++ ) { 
      if (i == jj) distances[i] = DBL_MAX;
      else distances[i] = (P[jj] - P[i]).Length();
    }
    float minD = sequence::reduce<float>(distances, n, utils::minF<float>());
    
    float d = (P[jj] - P[(neighbors.A)[k*jj]]).Length();

    float errorTolerance = 1e-6;
    if ((d - minD) / (d + minD)  > errorTolerance) {
      std::cout << "  [ FAILED ] " << name << ": for point "
           << jj << " min distance reported is: " << d 
	   << " actual is: " << minD << std::endl;
      return 1;
    }
  }
  std::cout << "  [ passed ] " << name << std::endl;
  return 0;
}

#ifdef NATIVE_COMPILE
int main(int argc, char* argv[]) {
  commandLine P(argc,argv,
		"[-k {1,...,10}] [-d {2,3}] [-r <numtests>] <inFile> <outfile>");
  std::pair<char*,char*> fnames = P.IOFileNames();
  char* iFile = fnames.first;
  char* oFile = fnames.second; 
  // number of random points to test
  int r = P.getOptionIntValue("-r",10);

  int k = P.getOptionIntValue("-k",1);
  int d = P.getOptionIntValue("-d",2);
  if (k > 10 || k < 1) P.badArgument();
  if (d < 2 || d > 3) P.badArgument();

  _seq<int> neighbors = readIntArrayFromFile<int>(oFile);

 // if (d == 2) {
    _seq<point2d> PIn = readPointsFromFile<point2d>(iFile);
    int n = PIn.n;
    //point2d* P = PIn.A;
    return check_neighbors(neighbors, PIn.A, PIn.n, k, r, "native");
 // } else if (d == 3) {
 //   _seq<point3d> PIn = readPointsFromFile<point3d>(iFile);
 //   int n = PIn.n;
 //   point3d* P = PIn.A;
 //  return check_neighbors(neighbors, PIn.A, PIn.n, k, r);
 //  } else return 1;
}
#endif

