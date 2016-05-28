//========================================================================
// bfs-scalar.cc
//========================================================================

#include <deque>

#include "bfs-scalar.h"

namespace bfs{

  __attribute__((noinline))
  void bfs_scalar( Node h_graph_nodes[], Edge h_graph_edges[],
	                 int color[], int h_cost[], int source, int no_of_nodes ) {

    std::deque<int> wavefront;
	  wavefront.push_back( source );
	  color[source] = GRAY;
	  int index;

	  while ( !wavefront.empty() ) {

		  index = wavefront.front();
		  wavefront.pop_front();

		  for( int i = h_graph_nodes[index].x;
			     i < (h_graph_nodes[index].y + h_graph_nodes[index].x); i++ ) {

			  int id = h_graph_edges[i].x;

			  if ( color[id] == WHITE ) {
				  h_cost[id] = h_cost[index] + 1;
				  wavefront.push_back( id );
				  color[id] = BLACK;
        }

		  }

	  }

	}

}
