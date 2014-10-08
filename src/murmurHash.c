/*
 * murmurHash.c
 *
 *  Created on: Oct 4, 2014
 *      Author: rhuertas
 */

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.

#include "murmurHash.h"
#include <stdlib.h>
#include "xil_types.h"

#if !defined(__BIG_ENDIAN)
  #define __BIG_ENDIAN 4321
#endif
#if !defined(__LITTLE_ENDIAN)
  #define __LITTLE_ENDIAN 1234
#endif

#define __BYTE_ORDER __LITTLE_ENDIAN
//#define UNALIGNED_SAFE



#define READ_UINT32(ptr)   (*((u32*)(ptr)))
#define ROTL32(x,r)  (((u32)x << r) | ((u32)x >> (32 - r)))

#define C1  (0xcc9e2d51U)
#define C2  (0x1b873593U)

/* This is the main processing body of the algorithm. It operates
 * on each full 32-bits of input. */
#define DOBLOCK(h1, k1) do{ \
        k1 *= C1; \
        k1 = ROTL32(k1,15); \
        k1 *= C2; \
        \
        h1 ^= k1; \
        h1 = ROTL32(h1,13); \
        h1 = h1*5+0xe6546b64; \
    }while(0)


/* Append unaligned bytes to carry, forcing hash churn if we have 4 bytes */
/* cnt=bytes to process, h1=name of h1 var, c=carry, n=bytes in c, ptr/len=payload */
#define DOBYTES(cnt, h1, c, n, ptr, len) do{ \
    int _i = cnt; \
    while(_i--) { \
        c = c>>8 | *ptr++<<24; \
        n++; len--; \
        if(n==4) { \
            DOBLOCK(h1, c); \
            n = 0; \
        } \
    } }while(0)


/*---------------------------------------------------------------------------*/

/* Main hashing function. Initialise carry to 0 and h1 to 0 or an initial seed
 * if wanted. Both ph1 and pcarry are required arguments. */
void PMurHash32_Process(u32 *ph1, u32 *pcarry, const void *key, int len)
{
  u32 h1 = *ph1;
  u32 c = *pcarry;

  const u8 *ptr = (u8*)key;
  const u8 *end;

  /* Extract carry count from low 2 bits of c value */
  int n = c & 3;

#if defined(UNALIGNED_SAFE)
  /* This CPU handles unaligned word access */

  /* Consume any carry bytes */
  int i = (4-n) & 3;
  if(i && i <= len) {
    DOBYTES(i, h1, c, n, ptr, len);
  }

  /* Process 32-bit chunks */
  end = ptr + len/4*4;
  for( ; ptr < end ; ptr+=4) {
    u8 k1 = READ_UINT32(ptr);
    DOBLOCK(h1, k1);
  }

#else /*UNALIGNED_SAFE*/
  /* This CPU does not handle unaligned word access */

  /* Consume enough so that the next data byte is word aligned */
  int i = -(long)ptr & 3;
  if(i && i <= len) {
      DOBYTES(i, h1, c, n, ptr, len);
  }

  /* We're now aligned. Process in aligned blocks. Specialise for each possible carry count */
  end = ptr + len/4*4;
  switch(n) { /* how many bytes in c */
  case 0: /* c=[----]  w=[3210]  b=[3210]=w            c'=[----] */
    for( ; ptr < end ; ptr+=4) {
      u32 k1 = READ_UINT32(ptr);
      DOBLOCK(h1, k1);
    }
    break;
  case 1: /* c=[0---]  w=[4321]  b=[3210]=c>>24|w<<8   c'=[4---] */
    for( ; ptr < end ; ptr+=4) {
      u32 k1 = c>>24;
      c = READ_UINT32(ptr);
      k1 |= c<<8;
      DOBLOCK(h1, k1);
    }
    break;
  case 2: /* c=[10--]  w=[5432]  b=[3210]=c>>16|w<<16  c'=[54--] */
    for( ; ptr < end ; ptr+=4) {
    	u32 k1 = c>>16;
      c = READ_UINT32(ptr);
      k1 |= c<<16;
      DOBLOCK(h1, k1);
    }
    break;
  case 3: /* c=[210-]  w=[6543]  b=[3210]=c>>8|w<<24   c'=[654-] */
    for( ; ptr < end ; ptr+=4) {
    	u32 k1 = c>>8;
      c = READ_UINT32(ptr);
      k1 |= c<<24;
      DOBLOCK(h1, k1);
    }
  }
#endif /*UNALIGNED_SAFE*/

  /* Advance over whole 32-bit chunks, possibly leaving 1..3 bytes */
  len -= len/4*4;

  /* Append any remaining bytes into carry */
  DOBYTES(len, h1, c, n, ptr, len);

  /* Copy out new running hash and carry */
  *ph1 = h1;
  *pcarry = (c & ~0xff) | n;
}


/* Finalize a hash. To match the original Murmur3A the total_length must be provided */
u32 PMurHash32_Result(u32 h, u32 carry, u32 total_length)
{
	u32 k1;
  int n = carry & 3;
  if(n) {
    k1 = carry >> (4-n)*8;
    k1 *= C1; k1 = ROTL32(k1,15); k1 *= C2; h ^= k1;
  }
  h ^= total_length;

  /* fmix */
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

/* Murmur3A compatable all-at-once */
u32 PMurHash32(u32 seed, const void *key, int len)
{
	u32 h1=seed, carry=0;
  PMurHash32_Process(&h1, &carry, key, len);
  return PMurHash32_Result(h1, carry, len);
}

lwHTTPU32 GenerateMurmurHash32(void* input, int length, lwHTTPU32 seed){
	lwHTTPU32 result = PMurHash32(seed, input, length);
	return result;


}


//
//inline u32 rotl32 ( u32 x, Xint8 r )
//{
//  return (x << r) | (x >> (32 - r));
//}
//
//inline u64 rotl64 ( u64 x, Xint8 r )
//{
//  return (x << r) | (x >> (64 - r));
//}
//
//
//#define	ROTL32(x,y)	rotl32(x,y)
//#define ROTL64(x,y)	rotl64(x,y)
//#define BIG_CONSTANT(x) (x##LLU)
//
//
//inline u32 getblock32 ( const u32 * p, int i )
//{
//  return p[i];
//}
//
//inline u64 getblock64 ( const u64 * p, int i )
//{
//  return p[i];
//}
//
//inline u32 fmix32 ( u32 h )
//{
//  h ^= h >> 16;
//  h *= 0x85ebca6b;
//  h ^= h >> 13;
//  h *= 0xc2b2ae35;
//  h ^= h >> 16;
//
//  return h;
//}
//
//inline u64 fmix64 ( u64 k )
//{
//  k ^= k >> 33;
//  k *= BIG_CONSTANT(0xff51afd7ed558ccd);
//  k ^= k >> 33;
//  k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
//  k ^= k >> 33;
//
//  return k;
//}
//
//void MurmurHash3_x86_32 ( const void * key, int len,
//                          u32 seed, void * out )
//{
//  const u8 * data = (const u8*)key;
//  const int nblocks = len / 4;
//  int i;
//  u32 h1 = seed;
//
//  const u32 c1 = 0xcc9e2d51;
//  const u32 c2 = 0x1b873593;
//
//  //----------
//  // body
//
//  const u32 * blocks = (const u32 *)(data + nblocks*4);
//
//  for(i = -nblocks; i; i++)
//  {
//    u32 k1 = getblock32(blocks,i);
//
//    k1 *= c1;
//    k1 = ROTL32(k1,15);
//    k1 *= c2;
//
//    h1 ^= k1;
//    h1 = ROTL32(h1,13);
//    h1 = h1*5+0xe6546b64;
//  }
//
//  //----------
//  // tail
//
//  const u8 * tail = (const u8*)(data + nblocks*4);
//
//  u32 k1 = 0;
//
//  switch(len & 3)
//  {
//  case 3: k1 ^= tail[2] << 16;
//  case 2: k1 ^= tail[1] << 8;
//  case 1: k1 ^= tail[0];
//          k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
//  };
//
//  //----------
//  // finalization
//
//  h1 ^= len;
//
//  h1 = fmix32(h1);
//
//  *(u32*)out = h1;
//}
//
//
//lwHTTPU32 GenerateMurmurHash32(void* input, int length, lwHTTPU32 seed){
//	lwHTTPU32 result;
//	MurmurHash3_x86_32(input, length, seed, &result);
//	return result;
//
//
//}
