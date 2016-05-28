//========================================================================
// bfs-xloops-unified.h
//========================================================================

#ifndef BFS_XLOOPS_UNIFIED_H
#define BFS_XLOOPS_UNIFIED_H

#include "bfs-common.h"

namespace bfs{

  void bfs_xloops_unified( Node h_graph_nodes[], Edge h_graph_edges[],
               int color[], int h_cost[], int source, int no_of_nodes );

}

#endif /* BFS_XLOOPS_UNIFIED_H */
