//========================================================================
// huffman-common.h
//========================================================================

#ifndef HUFFMAN_COMMON_H
#define HUFFMAN_COMMON_H

#define MAXBITSPERCODE 10000
#define BYTES 256

struct huffcode {
  int nbits;
  int code;
};
typedef struct huffcode huffcode_t;

struct huffheap {
  int *h;
  int n, s, cs;
  int *f;
};
typedef struct huffheap heap_t;

#endif /* HUFFMAN_COMMON_H */

