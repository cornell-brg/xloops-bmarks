#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cassert>

#include "huffman-scalar.h"

extern void unordered_for();
extern void ordered_for();
extern void mem_for();

namespace huffman {

#define swap_(I,J) do { int t_; t_ = a[(I)];	\
  a[(I)] = a[(J)]; a[(J)] = t_; } while(0)
  static void _heap_sort(heap_t *heap)
  {
    int i=1, j=2; /* gnome sort */
    int *a = heap->h;

    while(i < heap->n) { /* smaller values are kept at the end */
      if ( heap->f[a[i-1]] >= heap->f[a[i]] ) {
        i = j; j++;
      } else {
        swap_(i-1, i);
        i--;
        i = (i==0) ? j++ : i;
      }
    }
  }
#undef swap_

  static int _heap_remove(heap_t *heap)
  {
    if ( heap->n > 0 ) {
      heap->n--;
      return heap->h[heap->n];
    }
    return -1;
  }

  /* huffmann code generator */
  void huffman_scalar(char *data, int dsize, huffcode_t *codes, int csize)
  {
    int freqs[BYTES];

    for (int i = 0; i < csize; i++)
      freqs[i] = 0;

    for (int i = 0; i < dsize; i++)
      freqs[(int)data[i]]++;

    heap_t *heap;
    int efreqs[BYTES*2];
    int preds[BYTES*2];
    int i, extf=BYTES;
    int r1, r2;

    //memcpy(efreqs, freqs, sizeof(int)*BYTES);
    //memset(&efreqs[BYTES], 0, sizeof(int)*BYTES);
    //memset(codes, 0, sizeof(huffcode_t)*BYTES);
    //
    for (int i=0; i<BYTES; i++)
      efreqs[i] = freqs[i];

    for (int i=0; i<BYTES; i++) {
      efreqs[BYTES+i] = 0;
      codes[i].nbits = 0;
      codes[i].code = 0;
    }

    //heap = _heap_create(BYTES*2, efreqs);
    heap_t heapp;
    int hh[BYTES*2];
    heapp.h = hh;
    heapp.s = heapp.cs = BYTES*2;
    heapp.n = 0;
    heapp.f = efreqs;
    heap = &heapp;


    for (i=0; i < BYTES; i++)
      if (efreqs[i] > 0) {
        //_heap_add(heap, i);
        // assert (heap->n +1 <= heap->s && "no dynamic allocation!");
        heap->h[heap->n] = i;
        heap->n++;
        _heap_sort(heap);
      }

    while (heap->n > 1)
    {
      r1 = _heap_remove(heap);
      r2 = _heap_remove(heap);
      efreqs[extf] = efreqs[r1] + efreqs[r2];
      //_heap_add(heap, extf);
      {
        // assert (heap->n +1 <= heap->s && "no dynamic allocation!");
        heap->h[heap->n] = extf;
        heap->n++;
        _heap_sort(heap);
      }
      preds[r1] = extf;
      preds[r2] = -extf;
      extf++;
    }
    r1 = _heap_remove(heap);
    preds[r1] = r1;
    //_heap_destroy(heap);

    int bc, bn, ix;
    for(i=0; i < BYTES; i++) {
      bc=0; bn=0;
      if (efreqs[i] == 0)
        continue;
      ix = i;
      while( abs(preds[ix]) != ix ) {
        bc |= ((preds[ix] >= 0) ? 1 : 0 ) << bn;
        ix = abs(preds[ix]);
        bn++;
      }
      codes[i].nbits = bn;
      codes[i].code = bc;
    }
  }
}
