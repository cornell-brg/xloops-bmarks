//========================================================================
// sha-scalar.cc
//========================================================================

#include "sha-scalar.h"

namespace sha{

  //----------------------------------------------------------------------
  // sha_transform
  //----------------------------------------------------------------------

  static void sha_transform(sha_struct *sha_info) {

    int i;

    // SHA-1 variables
    LONG temp, A, B, C, D, E;

    // Message schedule
    LONG W[80];

    // prepare schedule 0-15
    for (i = 0; i < 16; ++i) {
      W[i] = sha_info->data[i];
    }

    // prepare schedule 16-79
    for (i = 16; i < 80; ++i) {
      W[i] = W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16];
      #ifdef USE_MODIFIED_SHA
        W[i] = ROT32(W[i], 1);
      #endif /* USE_MODIFIED_SHA */
    }

    // initialize working variables with previous hash values
    A = sha_info->digest[0];
    B = sha_info->digest[1];
    C = sha_info->digest[2];
    D = sha_info->digest[3];
    E = sha_info->digest[4];

    // unroll SHA-1 schedule processing
    #ifdef UNROLL_LOOPS
    FUNC(1, 0);  FUNC(1, 1);  FUNC(1, 2);  FUNC(1, 3);  FUNC(1, 4);
    FUNC(1, 5);  FUNC(1, 6);  FUNC(1, 7);  FUNC(1, 8);  FUNC(1, 9);
    FUNC(1,10);  FUNC(1,11);  FUNC(1,12);  FUNC(1,13);  FUNC(1,14);
    FUNC(1,15);  FUNC(1,16);  FUNC(1,17);  FUNC(1,18);  FUNC(1,19);

    FUNC(2,20);  FUNC(2,21);  FUNC(2,22);  FUNC(2,23);  FUNC(2,24);
    FUNC(2,25);  FUNC(2,26);  FUNC(2,27);  FUNC(2,28);  FUNC(2,29);
    FUNC(2,30);  FUNC(2,31);  FUNC(2,32);  FUNC(2,33);  FUNC(2,34);
    FUNC(2,35);  FUNC(2,36);  FUNC(2,37);  FUNC(2,38);  FUNC(2,39);

    FUNC(3,40);  FUNC(3,41);  FUNC(3,42);  FUNC(3,43);  FUNC(3,44);
    FUNC(3,45);  FUNC(3,46);  FUNC(3,47);  FUNC(3,48);  FUNC(3,49);
    FUNC(3,50);  FUNC(3,51);  FUNC(3,52);  FUNC(3,53);  FUNC(3,54);
    FUNC(3,55);  FUNC(3,56);  FUNC(3,57);  FUNC(3,58);  FUNC(3,59);

    FUNC(4,60);  FUNC(4,61);  FUNC(4,62);  FUNC(4,63);  FUNC(4,64);
    FUNC(4,65);  FUNC(4,66);  FUNC(4,67);  FUNC(4,68);  FUNC(4,69);
    FUNC(4,70);  FUNC(4,71);  FUNC(4,72);  FUNC(4,73);  FUNC(4,74);
    FUNC(4,75);  FUNC(4,76);  FUNC(4,77);  FUNC(4,78);  FUNC(4,79);
    #else /* !UNROLL_LOOPS */

    // process schedule 0-19
    for ( i = 0; i < 20; ++i ) {
      FUNC( 1, i );
    }

    // process schedule 20-39
    for ( i = 0; i < 20; ++i ) {
      FUNC( 2, i+20 );
    }

    // process schedule 40-59
    for ( i = 0; i < 20; ++i ) {
      FUNC( 3, i+40 );
    }

    // process schedule 60-79
    for ( i = 0; i < 20; ++i ) {
    	FUNC( 4, i+60 );
    }

    #endif /* !UNROLL_LOOPS */

    // compute the intermediate hash value
    sha_info->digest[0] += A;
    sha_info->digest[1] += B;
    sha_info->digest[2] += C;
    sha_info->digest[3] += D;
    sha_info->digest[4] += E;
  }

  //----------------------------------------------------------------------
  // byte_reverse
  //----------------------------------------------------------------------
  // function changes endianess of the data

  #ifdef LITTLE_ENDIAN
  static void byte_reverse(LONG *buffer, int count) {
    int i;
    BYTE ct[4], *cp;

    count /= sizeof(LONG);
    cp = (BYTE *) buffer;

    for (i = 0; i < count; ++i) {
      ct[0] = cp[0];
      ct[1] = cp[1];
      ct[2] = cp[2];
      ct[3] = cp[3];
      cp[0] = ct[3];
      cp[1] = ct[2];
      cp[2] = ct[1];
      cp[3] = ct[0];
      cp += sizeof(LONG);
    }
  }
  #endif /* LITTLE_ENDIAN */

  //----------------------------------------------------------------------
  // sha_final
  //----------------------------------------------------------------------
  // final round of computation

  static void sha_final(sha_struct *sha_info) {

    int  count;
    LONG lo_bit_count, hi_bit_count;

    lo_bit_count = sha_info->count_lo;
    hi_bit_count = sha_info->count_hi;
    count = (int) ((lo_bit_count >> 3) & 0x3f);
    ((BYTE *) sha_info->data)[count++] = 0x80;

    if (count > 56) {
      //memset((BYTE *) &sha_info->data + count, 0, 64 - count);
      BYTE* data_ptr = (BYTE*) (sha_info->data);
      for ( int j = 0; j < (64-count); j++ )
        data_ptr[j+count] = 0;

      #ifdef LITTLE_ENDIAN
      byte_reverse(sha_info->data, SHA_BLOCKSIZE);
      #endif /* LITTLE_ENDIAN */

      sha_transform(sha_info);

      //memset(&sha_info->data, 0, 56);
      for ( int j = 0; j < 56; j++ )
        data_ptr[j] = 0;
    }
    else {
      //memset((BYTE *) &sha_info->data + count, 0, 56 - count);
      BYTE* data_ptr = (BYTE*) (sha_info->data);
      for ( int j = 0; j < (56-count); j++ )
        data_ptr[j+count] = 0;
    }

    #ifdef LITTLE_ENDIAN
    byte_reverse(sha_info->data, SHA_BLOCKSIZE);
    #endif /* LITTLE_ENDIAN */

    sha_info->data[14] = hi_bit_count;
    sha_info->data[15] = lo_bit_count;
    sha_transform(sha_info);

  }

  //----------------------------------------------------------------------
  // sha_update
  //----------------------------------------------------------------------

  static void sha_update(sha_struct *sha_info, BYTE *buffer, int count) {

    if ( ( sha_info->count_lo + ( (LONG) count << 3) )
         < sha_info->count_lo )
      ++sha_info->count_hi;

    sha_info->count_lo += (LONG) count << 3;
    sha_info->count_hi += (LONG) count >> 29;

    while (count >= SHA_BLOCKSIZE) {

      //memcpy(sha_info->data, buffer, SHA_BLOCKSIZE);
      BYTE* data_ptr = (BYTE*)(sha_info->data);
      for( int j = 0; j < SHA_BLOCKSIZE; j++ )
        data_ptr[j] = buffer[j];

      #ifdef LITTLE_ENDIAN
      byte_reverse(sha_info->data, SHA_BLOCKSIZE);
      #endif /* LITTLE_ENDIAN */

      sha_transform(sha_info);

      // update pointers
      buffer += SHA_BLOCKSIZE;
      count  -= SHA_BLOCKSIZE;

    }

    //memcpy(sha_info->data, buffer, count);
    BYTE* data_ptr = (BYTE*)(sha_info->data);
    for( int j = 0; j < count; j++ )
      data_ptr[j] = buffer[j];

  }


  //----------------------------------------------------------------------
  // sha_scalar
  //----------------------------------------------------------------------

  void sha_scalar( sha_struct* sha_info, BYTE* src, int size ) {

    BYTE data[BLOCK_SIZE];
    int i             = 0;    // pointer index for copying data
    int bytes_copied  = 0;    // number of bytes copied in the current round
    int bytes_left    = size; // number of bytes remaining to be worked on

    // initialize sha_struct
    sha_init( sha_info );

    // replaces the fread opearation in MiBench bmark
    do {

      // number of bytes to copy
      bytes_copied  = ( bytes_left - BLOCK_SIZE ) > 0 ? BLOCK_SIZE : bytes_left;

      //memcpy( data, (src+i), bytes_copied );
      for ( int j = 0; j < bytes_copied; j++ )
        data[j] = src[i+j];

      // update message digest
      sha_update( sha_info, data, bytes_copied );

      // stripmine for next round
      i          = i + bytes_copied;
      bytes_left = bytes_left - bytes_copied;

    } while( bytes_left > 0 );

    // final computation
    sha_final( sha_info );

  }

}
