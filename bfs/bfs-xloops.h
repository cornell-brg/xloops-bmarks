//========================================================================
// bfs-xloops.h
//========================================================================

#ifndef BFS_XLOOPS_H
#define BFS_XLOOPS_H

#include "bfs-common.h"

namespace bfs{

  void bfs_xloops( Node h_graph_nodes[], Edge h_graph_edges[],
               int color[], int h_cost[], int source, int no_of_nodes );

}

#endif /* BFS_XLOOPS_H */
