//========================================================================
// strsearch-scalar.cc
//========================================================================

#include "strsearch-scalar.h"


namespace strsearch {

  //----------------------------------------------------------------------
  // Build the DFAs
  //----------------------------------------------------------------------

  void build_dfa_scalar( int dfa[], char str[], int str_sz )
  {
    dfa[0] = -1;
    dfa[1] = 0;
    int i = 2;  // current position we are computing in the DFA
    int j = 0;  // character of the current candidate substring

    while (i < str_sz) {
      if (str[i-1] == str[j]) {
        dfa[i] = j + 1;
        i = i + 1;
        j = j + 1;
      } else if (j > 0) {
        j = dfa[j];
      } else {
        dfa[i] = 0;
        i = i + 1;
      }
    }
  }

  //----------------------------------------------------------------------
  // Find each substring str in doc
  //----------------------------------------------------------------------

  int search_scalar( char doc[], char str[], int dfa[], int doc_sz, int str_sz )
  {
    int j = 0;     // start of current match in doc
    int index = 0; // current char in str

    while (j+index < doc_sz) {
      // check for character match
      if (str[index] == doc[j+index]) {
        index++;
        // entire string matches, search done
        if (index == str_sz)
          return 1;
      // character mismatch
      } else {
        // update str and doc indexes
        j = j + index - dfa[index];
        if (index > 0)
          index = dfa[index];
      }
    }

    // no match, return 0
    return 0;
  }

  //----------------------------------------------------------------------
  // All uts search for the same string in different docs
  //----------------------------------------------------------------------

  void alg0_scalar( int m[], int * dfa[], char * doc[], char * str[],
                    int doc_sz[], int str_sz[], int n_doc, int n_str )
  {
    int k = 0;

    for (int i = 0; i < n_str; i++) {
      for (int j = 0; j < n_doc; j++) {
        m[k++] = search_scalar( doc[j], str[i], dfa[i], doc_sz[j], str_sz[i] );
      }
    }
  }

  //----------------------------------------------------------------------
  // All uts search through the same doc for different strings
  //----------------------------------------------------------------------

  void alg1_scalar( int m[], int * dfa[], char * doc[], char * str[],
                    int doc_sz[], int str_sz[], int n_doc, int n_str )
  {
    int k = 0;

    for (int j = 0; j < n_doc; j++) {
      for (int i = 0; i < n_str; i++) {
        m[k++] = search_scalar( doc[j], str[i], dfa[i], doc_sz[j], str_sz[i] );
      }
    }
  }

  //----------------------------------------------------------------------
  // Run Stringsearch
  //----------------------------------------------------------------------

  void strsearch_scalar( int m[], int * dfa[], char * doc[], char * str[],
                         int doc_sz[], int str_sz[], int n_doc, int n_str,
                         int alg )
  {
    // build the DFAs for each pattern
    for (int i = 0; i < n_str; i++)
      build_dfa_scalar( dfa[i], str[i], str_sz[i]);

    // use the DFAs to find strings in docs
    if (!alg)
      alg0_scalar( m, dfa, doc, str, doc_sz, str_sz, n_doc, n_str );
    else
      alg1_scalar( m, dfa, doc, str, doc_sz, str_sz, n_doc, n_str );
  }
}
