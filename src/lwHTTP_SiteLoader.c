#include "lwHTTP_SiteLoader.h"



void lwHTTPSite_Init(struct lwhttpSite* site){
	site->dataSize = 0;
	site->tupplesN = 0;
	site->seed = 0;
	site->exportHomeFileIndex = 0;
	site->exportErrorFileIndex = 0;
	site->data = 0;
}




















