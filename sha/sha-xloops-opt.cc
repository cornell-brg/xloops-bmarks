//========================================================================
// sha-xloops-opt.cc
//========================================================================
// Hand coded assembly for SHA xloops

#include "sha-xloops-opt.h"

extern void ordered_for();
extern void unordered_for();

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
    for (i = 0; i < 16; i++, unordered_for() ) {
      W[i] = sha_info->data[i];
    }

    // prepare schedule 16-79
    for (i = 16; i < 80; i++) {
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

    #ifdef _MIPS_ARCH_MAVEN
    // process schedule 0-19
    LONG t0, t1, t2, t3, t4, t5;
    for ( i = 0; i < 20; i++, ordered_for() ) {

      __asm__ __volatile__
      (

        // W[i]
        "sll   %[t2], %[i], 0x2   \n" // calc offset
        "addu  %[t3], %[W], %[t2] \n" // addr calc
        "lw    %[t3], 0(%[t3])    \n" // W[i]

        // C = ROT32( B, 30 )
        "srl   %[t4], %[B],  0x2  \n"
        "sll   %[t5], %[B],  0x1e \n"

        // f1(B,C,D)
        "nor   %[t0], %[B],  $0   \n" // t0 -  ~B
        "and   %[t0], %[t0], %[D] \n" // t0 - ( ~B & D  )
        "and   %[t1], %[C],  %[B] \n" // t1 - ( B  & C  )
        "or    %[t1], %[t1], %[t0]\n" // t1 - ( t1 | t0 )

        // f1(B,C,D) + E + W[i] + CONST1
        "addu  %[t1], %[t1], %[E] \n"
        "addu  %[t1], %[t3], %[t1]\n"
        "addu  %[t1], %[t1], %[C1]\n"

        // B = A
        "addiu %[B],  %[A], 0    \n"

        // E = D
        "addiu %[E],  %[D],  0    \n"

        // D = C
        "addiu %[D],  %[C],  0    \n"

        // C = ROT32( B, 30 )
        "or    %[t4], %[t5], %[t4]\n"
        "addiu %[C],  %[t4], 0    \n"

        // A = ROT32(A,5) + temp
        "srl   %[t4], %[A],  0x1b \n"
        "sll   %[t5], %[A],  0x5  \n"
        "or    %[t4], %[t5], %[t4]\n"
        "addu  %[A],  %[t4], %[t1]\n"

        : [t0] "=r" (t0),
          [t1] "=r" (t1),
          [t2] "=r" (t2),
          [t3] "=r" (t3),
          [t4] "=r" (t4),
          [t5] "=r" (t5)
        : [A]  "r"  (A),
          [B]  "r"  (B),
          [C]  "r"  (C),
          [D]  "r"  (D),
          [E]  "r"  (E),
          [W]  "r"  (&W[0]),
          [C1] "r"  (CONST1),
          [i]  "r"  (i)
      );

    }

    // process schedule 20-39
    for ( i = 0; i < 20; i++, ordered_for() ) {

      __asm__ __volatile__
      (

        // W[i+20]
        "addiu %[t1], %[i], 20    \n" // t1 - ( i + 20 )
        "sll   %[t1], %[t1],0x2   \n" // calc offset
        "addu  %[t2], %[W], %[t1] \n" // addr calc
        "lw    %[t2], 0(%[t2])    \n" // W[i+20]

        // C = ROT32( B, 30 )
        "srl   %[t4], %[B],  0x2  \n"
        "sll   %[t5], %[B],  0x1e \n"

        // f2(B,C,D)
        "xor   %[t0], %[B], %[C]  \n" // t0 - ( B ^ C  )
        "xor   %[t0], %[D], %[t0] \n" // t0 - ( D ^ t0 )

        // f2(B,C,D) + E + W[i] + CONST2
        "addu  %[t0], %[t0], %[E] \n"
        "addu  %[t0], %[t2], %[t0]\n"
        "addu  %[t0], %[t0], %[C1]\n"

        // B = A
        "addiu %[B],  %[A], 0    \n"

        // E = D
        "addiu %[E],  %[D],  0    \n"

        // D = C
        "addiu %[D],  %[C],  0    \n"

        // C = ROT32( B, 30 )
        "or    %[t4], %[t5], %[t4]\n"
        "addiu %[C],  %[t4], 0    \n"


        // A = ROT32(A,5) + temp
        "srl   %[t4], %[A],  0x1b \n"
        "sll   %[t5], %[A],  0x5  \n"
        "or    %[t4], %[t5], %[t4]\n"
        "addu  %[A],  %[t4], %[t0]\n"

        : [t0] "=r" (t0),
          [t1] "=r" (t1),
          [t2] "=r" (t2),
          [t4] "=r" (t4),
          [t5] "=r" (t5)
        : [A]  "r"  (A),
          [B]  "r"  (B),
          [C]  "r"  (C),
          [D]  "r"  (D),
          [E]  "r"  (E),
          [W]  "r"  (&W[0]),
          [C1] "r"  (CONST2),
          [i]  "r"  (i)
      );

    }

    // process schedule 40-59
    for ( i = 0; i < 20; i++, ordered_for() ) {

      __asm__ __volatile__
      (
        // W[i+40]
        "addiu %[t0], %[i], 40    \n" // t0 - ( i + 40 )
        "sll   %[t2], %[t0],0x2   \n" // calc offset
        "addu  %[t3], %[W], %[t2] \n" // addr calc
        "lw    %[t3], 0(%[t3])    \n" // W[i]

        // C = ROT32( B, 30 )
        "srl   %[t4], %[B],  0x2  \n"
        "sll   %[t5], %[B],  0x1e \n"

        // f3(B,C,D)
        "and   %[t1], %[B],  %[D] \n" // t1 - ( B & D )
        "and   %[t0], %[B],  %[C] \n" // t0 - ( B & C )
        "or    %[t1], %[t0], %[t1]\n" // t0 - ( t1 | t0 )
        "and   %[t0], %[C],  %[D] \n" // t0 - ( C & D )
        "or    %[t1], %[t1], %[t0]\n" // t1 - ( t1 | t0 )

        // f3(B,C,D) + E + W[i] + CONST3
        "addu  %[t1], %[t1], %[E] \n"
        "addu  %[t1], %[t3], %[t1]\n"
        "addu  %[t1], %[t1], %[C1]\n"

        // B = A
        "addiu %[B],  %[A], 0    \n"

        // E = D
        "addiu %[E],  %[D],  0    \n"

        // D = C
        "addiu %[D],  %[C],  0    \n"

        // C = ROT32( B, 30 )
        "or    %[t4], %[t5], %[t4]\n"
        "addiu %[C],  %[t4], 0    \n"


        // A = ROT32(A,5) + temp
        "srl   %[t4], %[A],  0x1b \n"
        "sll   %[t5], %[A],  0x5  \n"
        "or    %[t4], %[t5], %[t4]\n"
        "addu  %[A],  %[t4], %[t1]\n"

        : [t0] "=r" (t0),
          [t1] "=r" (t1),
          [t2] "=r" (t2),
          [t3] "=r" (t3),
          [t4] "=r" (t4),
          [t5] "=r" (t5)
        : [A]  "r"  (A),
          [B]  "r"  (B),
          [C]  "r"  (C),
          [D]  "r"  (D),
          [E]  "r"  (E),
          [W]  "r"  (&W[0]),
          [C1] "r"  (CONST3),
          [i]  "r"  (i)
      );

    }

    // process schedule 60-79
    for ( i = 0; i < 20; i++, ordered_for() ) {

      __asm__ __volatile__
      (

        // W[i+60]
        "addiu %[t1], %[i], 60    \n" // t1 - ( i + 20 )
        "sll   %[t1], %[t1],0x2   \n" // calc offset
        "addu  %[t2], %[W], %[t1] \n" // addr calc
        "lw    %[t2], 0(%[t2])    \n" // W[i+20]

        // C = ROT32( B, 30 )
        "srl   %[t4], %[B],  0x2  \n"
        "sll   %[t5], %[B],  0x1e \n"

        // f4(B,C,D)
        "xor   %[t0], %[B], %[C]  \n" // t0 - ( B ^ C  )
        "xor   %[t0], %[D], %[t0] \n" // t0 - ( D ^ t0 )

        // f4(B,C,D) + E + W[i] + CONST4
        "addu  %[t0], %[t0], %[E] \n"
        "addu  %[t0], %[t2], %[t0]\n"
        "addu  %[t0], %[t0], %[C1]\n"

        // B = A
        "addiu %[B],  %[A], 0    \n"

        // E = D
        "addiu %[E],  %[D],  0    \n"

        // D = C
        "addiu %[D],  %[C],  0    \n"

        // C = ROT32( B, 30 )
        "or    %[t4], %[t5], %[t4]\n"
        "addiu %[C],  %[t4], 0    \n"

        // A = ROT32(A,5) + temp
        "srl   %[t4], %[A],  0x1b \n"
        "sll   %[t5], %[A],  0x5  \n"
        "or    %[t4], %[t5], %[t4]\n"
        "addu  %[A],  %[t4], %[t0]\n"

        : [t0] "=r" (t0),
          [t1] "=r" (t1),
          [t2] "=r" (t2),
          [t4] "=r" (t4),
          [t5] "=r" (t5)
        : [A]  "r"  (A),
          [B]  "r"  (B),
          [C]  "r"  (C),
          [D]  "r"  (D),
          [E]  "r"  (E),
          [W]  "r"  (&W[0]),
          [C1] "r"  (CONST4),
          [i]  "r"  (i)
      );

    }
    #else
    // process schedule 0-19
    for ( i = 0; i < 20; i++, ordered_for() ) {
      FUNC( 1, i );
    }

    // process schedule 20-39
    for ( i = 0; i < 20; i++, ordered_for() ) {
      FUNC( 2, i+20 );
    }

    // process schedule 40-59
    for ( i = 0; i < 20; i++, ordered_for() ) {
      FUNC( 3, i+40 );
    }

    // process schedule 60-79
    for ( i = 0; i < 20; i++, ordered_for() ) {
      FUNC( 4, i+60 );
    }
    #endif

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
      for ( int j = 0; j < (64-count); j++, unordered_for() )
        data_ptr[j+count] = 0;

      #ifdef LITTLE_ENDIAN
      byte_reverse(sha_info->data, SHA_BLOCKSIZE);
      #endif /* LITTLE_ENDIAN */

      sha_transform(sha_info);

      //memset(&sha_info->data, 0, 56);
      for ( int j = 0; j < 56; j++, unordered_for() )
        data_ptr[j] = 0;
    }
    else {
      //memset((BYTE *) &sha_info->data + count, 0, 56 - count);
      BYTE* data_ptr = (BYTE*) (sha_info->data);
      for ( int j = 0; j < (56-count); j++, unordered_for() )
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
      for( int j = 0; j < SHA_BLOCKSIZE; j++, unordered_for() )
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
    for( int j = 0; j < count; j++, unordered_for() )
      data_ptr[j] = buffer[j];

  }


  //----------------------------------------------------------------------
  // sha_xloops_opt
  //----------------------------------------------------------------------

  void sha_xloops_opt( sha_struct* sha_info, BYTE* src, int size ) {

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
      for ( int j = 0; j < bytes_copied; j++, unordered_for() )
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
