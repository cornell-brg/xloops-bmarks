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


#ifndef _BENCH_GEOMETRY_IO
#define _BENCH_GEOMETRY_IO
#include "pbbs-common-IO.h"
#include "pbbs-common-parallel.h"
#include "pbbs-common-geometry.h"
//using namespace benchIO;

inline int xToStringLen(point2d a) { 
  return benchIO::xToStringLen(a.x) + benchIO::xToStringLen(a.y) + 1;
}

inline void xToString(char* s, point2d a) { 
  int l = benchIO::xToStringLen(a.x);
  benchIO::xToString(s, a.x);
  s[l] = ' ';
  benchIO::xToString(s+l+1, a.y);
}

inline int xToStringLen(point3d a) { 
  return benchIO::xToStringLen(a.x) +
    benchIO::xToStringLen(a.y) +
    benchIO::xToStringLen(a.z) + 2;
}

inline void xToString(char* s, point3d a) { 
  int lx = benchIO::xToStringLen(a.x);
  int ly = benchIO::xToStringLen(a.y);
  benchIO::xToString(s, a.x);
  s[lx] = ' ';
  benchIO::xToString(s+lx+1, a.y);
  s[lx+ly+1] = ' ';
  benchIO::xToString(s+lx+ly+2, a.z);
}

inline int xToStringLen(triangle a) {
  return benchIO::xToStringLen(a.C[0]) +
    benchIO::xToStringLen(a.C[1]) +
    benchIO::xToStringLen(a.C[2]) + 2;
}

inline void xToString(char* s, triangle a) { 
  int lx = benchIO::xToStringLen(a.C[0]);
  int ly = benchIO::xToStringLen(a.C[1]);
  benchIO::xToString(s, a.C[0]);
  s[lx] = ' ';
  benchIO::xToString(s+lx+1, a.C[1]);
  s[lx+ly+1] = ' ';
  benchIO::xToString(s+lx+ly+2, a.C[2]);
}

namespace benchIO {
namespace {
  //using namespace std;

  std::string HeaderPoint2d = "pbbs_sequencePoint2d";
  std::string HeaderPoint3d = "pbbs_sequencePoint3d";
  std::string HeaderTriangles = "pbbs_triangles";

  template <class pointT>
    int writePointsToFile(pointT* P, intT n, char* fname) {
      std::string Header = (pointT::dim == 2) ? HeaderPoint2d : HeaderPoint3d;
      int r = writeArrayToFile(Header, P, n, fname);
      return r;
    }

  template <class pointT>
    void parsePoints(char** Str, pointT* P, intT n) {
      int d = pointT::dim;
      float* a = newA(float,n*d);
      {parallel_for (long i=0; i < d*n; i++) 
        a[i] = atof(Str[i]);}
        {parallel_for (long i=0; i < n; i++) 
          P[i] = pointT(a+(d*i));}
          free(a);
    }

  template <class pointT>
    _seq<pointT> readPointsFromFile(char* fname) {
      _seq<char> S = readStringFromFile(fname);
      words W = stringToWords(S.A, S.n);
      int d = pointT::dim;
      if (W.m == 0 || W.Strings[0] != (d == 2 ? HeaderPoint2d : HeaderPoint3d)) {
        std::cout << "readPointsFromFile wrong file type" << std::endl;
        abort();
      }
      long n = (W.m-1)/d;
      pointT *P = newA(pointT, n);
      parsePoints(W.Strings + 1, P, n);
      return _seq<pointT>(P, n);
    }

  triangles<point2d> readTrianglesFromFileNodeEle(char* fname) {
    std::string nfilename(fname);
    _seq<char> S = readStringFromFile((char*)nfilename.append(".node").c_str());
    words W = stringToWords(S.A, S.n);
    triangles<point2d> Tr;
    Tr.numPoints = atol(W.Strings[0]);
    if (W.m < 4*Tr.numPoints + 4) {
      std::cout << "readStringFromFileNodeEle inconsistent length" << std::endl;
      abort();
    }

    Tr.P = newA(point2d, Tr.numPoints);
    for(intT i=0; i < Tr.numPoints; i++) 
      Tr.P[i] = point2d(atof(W.Strings[4*i+5]), atof(W.Strings[4*i+6]));

    std::string efilename(fname);
    _seq<char> SN = readStringFromFile((char*)efilename.append(".ele").c_str());
    words WE = stringToWords(SN.A, SN.n);
    Tr.numTriangles = atol(WE.Strings[0]);
    if (WE.m < 4*Tr.numTriangles + 3) {
      std::cout << "readStringFromFileNodeEle inconsistent length" << std::endl;
      abort();
    }

    Tr.T = newA(triangle, Tr.numTriangles);
    for (long i=0; i < Tr.numTriangles; i++)
      for (int j=0; j < 3; j++)
        Tr.T[i].C[j] = atol(WE.Strings[4*i + 4 + j]);

    return Tr;
  }

  template <class pointT>
    triangles<pointT> readTrianglesFromFile(char* fname, intT offset) {
      int d = pointT::dim;
      _seq<char> S = readStringFromFile(fname);
      words W = stringToWords(S.A, S.n);
      if (W.Strings[0] != HeaderTriangles) {
        std::cout << "readTrianglesFromFile wrong file type" << std::endl;
        abort();
      }

      int headerSize = 3;
      triangles<pointT> Tr;
      Tr.numPoints = atol(W.Strings[1]);
      Tr.numTriangles = atol(W.Strings[2]);
      if (W.m != headerSize + 3 * Tr.numTriangles + d * Tr.numPoints) {
        std::cout << "readTrianglesFromFile inconsistent length" << std::endl;
        abort();
      }

      Tr.P = newA(pointT, Tr.numPoints);
      parsePoints(W.Strings + headerSize, Tr.P, Tr.numPoints);

      Tr.T = newA(triangle, Tr.numTriangles);
      char** Triangles = W.Strings + headerSize + d * Tr.numPoints;
      for (long i=0; i < Tr.numTriangles; i++)
        for (int j=0; j < 3; j++)
          Tr.T[i].C[j] = atol(Triangles[3*i + j])-offset;
      return Tr;
    }

  template <class pointT>
    int writeTrianglesToFile(triangles<pointT> Tr, char* fileName) {
      std::ofstream file (fileName, std::ios::binary);
      if (!file.is_open()) {
        std::cout << "Unable to open file: " << fileName << std::endl;
        return 1;
      }
      file << HeaderTriangles << std::endl;
      file << Tr.numPoints << std::endl; 
      file << Tr.numTriangles << std::endl; 
      writeArrayToStream(file, Tr.P, Tr.numPoints);
      writeArrayToStream(file, Tr.T, Tr.numTriangles);
      file.close();
      return 0;
    }

}
};

#endif // _BENCH_GEOMETRY_IO
