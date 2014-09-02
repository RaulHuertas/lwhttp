#ifndef LWHTTPCONFIG_MICROBLAZE
#define LWHTTPCONFIG_MICROBLAZE


#include "xil_types.h"
#ifdef __MICROBLAZE__
typedef u32 lwHTTPTime;
typedef s64 lwHTTPS64;
typedef u64 lwHTTPU64;
typedef u8 	lwHTTPU8;
typedef s8 	lwHTTPS8;
typedef u16	lwHTTPU16;
typedef s16 lwHTTPS16;
typedef u32 lwHTTPU32;
typedef s32 lwHTTPS32;


#ifdef LWHTTP_DEBUG_VERBOSE
	#define LWHTTPDebug(...) xil_printf( __VA_ARGS__)
#else
	#define LWHTTPDebug(...)
#endif

#endif //__MICROBLAZE__


#endif //LWHTTPCONFIG_MICROBLAZE
