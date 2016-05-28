//========================================================================
// bfs-common.h
//========================================================================

#ifndef BFS_COMMON_H
#define BFS_COMMON_H

#define INF   2147483647 //2^31-1
#define WHITE 16677217
#define GRAY  16677218
#define GRAY0 16677219
#define GRAY1 16677220
#define BLACK 16677221

namespace bfs {

  struct Node{
      int x;
      int y;
  };
  
  struct Edge{
      int x;
      int y;
  };

}

#endif /* BFS_COMMON_H */
