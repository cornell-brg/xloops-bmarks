//========================================================================
// strsearch-xloops.h
//========================================================================

#ifndef STRSEARCH_XLOOPS_H
#define STRSEARCH_XLOOPS_H

namespace strsearch {

  void build_dfa_xloops( int dfa[], char str[], int str_sz );

  int search_xloops( char doc[], char str[], int dfa[], int doc_sz, int str_sz );

  void alg0_xloops( int m[], int * dfa[], char * doc[], char * str[],
                     int doc_sz[], int str_sz[], int n_doc, int n_str );

  void alg1_xloops( int m[], int * dfa[], char * doc[], char * str[],
                     int doc_sz[], int str_sz[], int n_doc, int n_str );

  void strsearch_xloops( int m[], int * dfa[], char * doc[], char * str[],
                         int doc_sz[], int str_sz[], int n_doc, int n_str,
                         int alg);

}

#endif /* STRSEARCH_XLOOPS_H*/

