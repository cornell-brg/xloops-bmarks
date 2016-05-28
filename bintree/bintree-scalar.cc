//========================================================================
// bintree-scalar.cc
//========================================================================

#include "bintree-scalar.h"

namespace bintree {

  // insert a node to the tree at the root
  void insert_node_scalar( Node *root, Node *new_node ) {
    // pick the child based on v. child is a reference to Node pointer
    Node *&child = new_node->value < root->value ?
                   root->left : root->right;

    // add a new child if the child is null
    if ( child == NULL )
      child = new_node;
    else
      insert_node_scalar( child, new_node );
  }

  void bintree_scalar( int n, int *A, Node *nodes ) {

    for ( int i = 0; i < n; i++ )
      nodes[i] = Node( A[i] );

    Node *root = &nodes[0];

    // add the rest of the nodes
    for ( int i = 1; i < n; i++ ) {
      insert_node_scalar( root, &nodes[i] );
    }

  }
}
