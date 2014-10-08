#ifndef LWHTTP_MURMUR_HASH_HPP
#define LWHTTP_MURMUR_HASH_HPP



#include "lwHTTPConfig.h"


#ifdef __cplusplus
extern "C" {
#endif





lwHTTPU32 GenerateMurmurHash32(void* input, int length, lwHTTPU32 seed);





#ifdef __cplusplus
}
#endif


#endif //LWHTTP_MURMUR_HASH_HPP
