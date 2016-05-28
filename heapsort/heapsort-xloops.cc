#include "heapsort-xloops.h"

extern void mem_for();

namespace xheapsort {

#define exchange(A, i, j) \
  { int tmp = A[i]; A[i] = A[j]; A[j] = tmp;}

#define parent(n) ((n-1)/2)
#define pre_node(n) (n-1)
#define next_node(n) (n+1)
#define lchild(n) (2*n+1)
#define rchild(n) (2*n+2)
#define root 0

  // Heapifies starting from parent p, and recursively trickling down the
  // small element to the children. This doesn't have much parallelism
  // inside
  static void heapify( int *A, int p, int n ) {
    while ( lchild(p) <= n ) {
      int l = lchild(p);
      int t = A[p] > A[l] ? p: l;

      int r = rchild(p);
      if (r <= n && A[r] > A[t])
        t = r;
      if (p == t)
        break;
      exchange(A, p, t);
      p = t;
    }
  }

  __attribute__((noinline))
  void heapsort_xloops( int n, int *A )
  {

    // Build the heap: uses in-place storage of A to build a heap starting
    // from the end of the array. Starts heapifying from the parent of
    // rightmost element, and heapifies the parents going left.
    for ( int i = 0; i < parent(n) + 1; i++, mem_for() ) {
      int start = parent(n) - i;
      heapify(A, start, n);
    }

    // Heap sort: once all of the elements are heapified, get the maximum
    // element from the top of the heap, move it to the end of the A, and
    // preserve heap condition for the rest of the heap
    for ( int i = 0; i < n; i++, mem_for() ) {
      int start = n - i;
      exchange(A, 0, start);
      heapify(A, 0, pre_node(start));
    }
  }
};
