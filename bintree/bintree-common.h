//========================================================================
// bintree-common.h
//========================================================================

#ifndef BINTREE_COMMON_H
#define BINTREE_COMMON_H

#ifndef NULL
#define NULL 0
#endif

namespace bintree {

  class Node {
  public:
    Node *parent, *left, *right;
    int value;

    Node() {}
    Node( int v ) : parent(NULL), left(NULL), right(NULL), value(v) {}
  };

}

#endif /* BINTREE_COMMON_H */
