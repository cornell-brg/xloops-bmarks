//========================================================================
// sha-common.h
//========================================================================

#ifndef SHA_COMMON_H
#define SHA_COMMON_H

// Each SHA-1 message block is 512 bits or 64 bytes long
#define SHA_BLOCKSIZE  64

// SHA-1 message digest is 160 bits or 20 bytes long
#define SHA_DIGESTSIZE 20

// SHA-1 f()-functions
#define f1(x,y,z) ((x & y) | (~x & z))
#define f2(x,y,z) (x ^ y ^ z)
#define f3(x,y,z) ((x & y) | (x & z) | (y & z))
#define f4(x,y,z) (x ^ y ^ z)

// SHA-1 constants
#define CONST1    0x5a827999L
#define CONST2    0x6ed9eba1L
#define CONST3    0x8f1bbcdcL
#define CONST4    0xca62c1d6L

//  32-bit rotate left macro
#define ROT32(x,n)  ((x << n) | (x >> (32 - n)))

// SHA-1 transformation function
#define FUNC(n,i)           \
    temp = ROT32(A,5) + f##n(B,C,D) + E + W[i] + CONST##n;  \
    E = D; D = C; C = ROT32(B,30); B = A; A = temp

// stream message length
#define BLOCK_SIZE 8192

// short names
typedef unsigned char BYTE;
typedef unsigned int  LONG;

namespace sha {

  // SHA-1 data structure
  struct sha_struct {
    LONG digest[5];          // Message digest
    LONG count_lo, count_hi; // 64-bit count
    LONG data[16];           // Message block
  };

  inline
  void sha_init  ( sha_struct* sha_info )
  {
    sha_info->digest[0] = 0x67452301L;
    sha_info->digest[1] = 0xefcdab89L;
    sha_info->digest[2] = 0x98badcfeL;
    sha_info->digest[3] = 0x10325476L;
    sha_info->digest[4] = 0xc3d2e1f0L;
    sha_info->count_lo  = 0L;
    sha_info->count_hi  = 0L;
  }

}

#endif  /*SHA_COMMON_H*/
