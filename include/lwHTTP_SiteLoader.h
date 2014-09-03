#ifndef LWHTTP_SITE_LOADER_HPP
#define LWHTTP_SITE_LOADER_HPP

#include "lwHTTPConfig.h"

#ifdef __cplusplus
extern "C" {
#endif



struct lwhttpSite{
	lwHTTPU32 dataSize;
	lwHTTPU32 tupplesN;
	lwHTTPU32 seed;
	lwHTTPU32 exportHomeFileIndex;
	lwHTTPU32 exportErrorFileIndex;
};


void lwHTTPSite_Init(struct lwhttpSite* site);


int lwHTTPSite_LoadFromFile(const char* filename, struct lwhttpSite* site);










#ifdef __cplusplus
}
#endif



#endif //LWHTTP_SITE_LOADER_HPP
