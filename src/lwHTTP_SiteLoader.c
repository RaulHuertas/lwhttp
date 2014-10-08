#include "lwHTTP_SiteLoader.h"



void lwHTTPSite_Init(struct lwhttpSite* site){
	site->dataSize = 0;
	site->tupplesN = 0;
	site->seed = 0;
	site->exportHomeFileIndex = 0;
	site->exportErrorFileIndex = 0;
	site->data = 0;
}

int isHashInPack(lwHTTPU32 hash, struct lwhttpSite* site){
	int ret = -1;
	int t;
	for(t=0;t<site->tupplesN;t++){
		if( site->tuples[t].urlHash == hash ){
			ret = t;
			break;
		}
	}
	return ret;

}


















