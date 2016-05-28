//========================================================================
// strsearch-scalar.h
//========================================================================

#ifndef STRSEARCH_SCALAR_H
#define STRSEARCH_SCALAR_H

namespace strsearch {

  void build_dfa_scalar( int dfa[], char str[], int str_sz );

  int search_scalar( char doc[], char str[], int dfa[], int doc_sz, int str_sz );

  void alg0_scalar( int m[], int * dfa[], char * doc[], char * str[],
                     int doc_sz[], int str_sz[], int n_doc, int n_str );

  void alg1_scalar( int m[], int * dfa[], char * doc[], char * str[],
                     int doc_sz[], int str_sz[], int n_doc, int n_str );

  void strsearch_scalar( int m[], int * dfa[], char * doc[], char * str[],
                         int doc_sz[], int str_sz[], int n_doc, int n_str,
                         int alg);

}

#endif /* STRSEARCH_SCALAR_H */

