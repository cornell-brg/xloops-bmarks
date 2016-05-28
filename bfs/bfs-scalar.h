//========================================================================
// bfs-scalar.h
//========================================================================

#ifndef BFS_SCALAR_H
#define BFS_SCALAR_H

#include "bfs-common.h"

namespace bfs {

  void bfs_scalar( Node h_graph_nodes[], Edge h_graph_edges[],
                   int color[], int h_cost[], int source, int no_of_nodes );

}

#endif /* BFS_SCALAR_H */ 
