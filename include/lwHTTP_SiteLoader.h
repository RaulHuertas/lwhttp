#ifndef LWHTTP_SITE_LOADER_HPP
#define LWHTTP_SITE_LOADER_HPP

#include "lwHTTPConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LWHTTP_MAX_RESOURCES
#define LWHTTP_MAX_RESOURCES 	8
#endif
#ifndef LWHTTP_MAX_TUPLES
#define LWHTTP_MAX_TUPLES 		8
#endif
#ifndef LWHTTP_MAX_ENTRYS
#define LWHTTP_MAX_ENTRYS 		8
#endif



#ifndef LWHTTP_MAX_URL_SIZE
#define LWHTTP_MAX_URL_SIZE 		511
#endif



struct lwHTTPSiteResource{
	lwHTTPU32 listIndex;
	char url[LWHTTP_MAX_URL_SIZE+1];
	lwHTTPU32 urlHash;
};

struct lwHTTPSiteTuple{
	lwHTTPU32 urlHash;
	lwHTTPU32 position;
	lwHTTPU32 headerSize;
	lwHTTPU32 totalSize;
};

struct lwHTTPSiteEntry{
	lwHTTPU32 entireSize;
	lwHTTPU32 headerSize;
	lwHTTPU32 position;
};

struct lwhttpSite{
	lwHTTPU32 dataSize;
	lwHTTPU32 tupplesN;
	lwHTTPU32 seed;
	lwHTTPU32 exportHomeFileIndex;
	lwHTTPU32 exportErrorFileIndex;
	//struct lwHTTPSiteResource resources[LWHTTP_MAX_RESOURCES];
	//struct lwHTTPSiteTuple tuples[LWHTTP_MAX_TUPLES];
	//struct lwHTTPSiteEntry entries[LWHTTP_MAX_ENTRYS];
	struct lwHTTPSiteTuple tuples[LWHTTP_MAX_TUPLES];
	char* data;
};


void lwHTTPSite_Init(struct lwhttpSite* site);



#define LOAD_SITE_ERROR_FILENOTFOUND	-1
#define LOAD_SITE_ERROR_FILETOOBIG		-2
#define LOAD_SITE_ERROR_READ_ERROR		-3
/**
 * Esta función debe definirse en el código de la aplicación
 * */
int lwHTTPSite_LoadFromFile(const char* filename, struct lwhttpSite* site);

int isHashInPack(lwHTTPU32 hash, struct lwhttpSite* site);








#ifdef __cplusplus
}
#endif



#endif //LWHTTP_SITE_LOADER_HPP
