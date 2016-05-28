//========================================================================
// bintree-xloops.cc
//========================================================================

#include "bintree-xloops.h"

extern void unordered_for();
extern void mem_for();

namespace bintree {

  // insert a node to the tree at the root
  inline void insert_node_xloops( Node *root, Node *new_node ) {
    // pick the child based on v. child is a reference to Node pointer
    Node *&child = new_node->value < root->value ?
                   root->left : root->right;

    // add a new child if the child is null
    if ( child == NULL )
      child = new_node;
    else
      insert_node_xloops( child, new_node );
  }

  void bintree_xloops( int n, int *A, Node *nodes ) {

    for ( int i = 0; i < n; i++ , unordered_for() ) {
      nodes[i] = Node( A[i] );
    }

    Node *root = &nodes[0];
    Node *nodes_rest = &nodes[1];

    // NOTE: we add this pseudo-write to nodes_rest, because otherwise the
    // for loop below does the increment to i first as opposed to last and
    // this breaks for.m tagging. The assembly line "modifies" nodes_rest,
    // so it doesn't know it's equal to nodes[1].
    #ifdef _MIPS_ARCH_MAVEN
    __asm__ __volatile__( "addiu %0, %1, 0"
                          : "=r"(nodes_rest)
                          : "r"(nodes_rest) );
    #endif

    // add the rest of the nodes
    for ( int i = 0; i < n-1; i++, mem_for() ) {
      insert_node_xloops( root, &nodes_rest[i] );
    }

  }
}
