/*
 * lwHTTPUtils.h
 *
 *  Created on: May 20, 2012
 *      Author: snorerax
 */

#ifndef LWHTTPUTILS_H_
#define LWHTTPUTILS_H_
#include "lwHTTP.h"

#ifdef __cplusplus
extern "C" {
#endif

void lwHTTPDefaultGetTime(lwHTTPTime*  );

int lwHTTPDefaulIsClientValidQ(
	struct lwHTTPDispatcherAcceptArguments*
);

#ifdef __cplusplus
}
#endif

#endif /* LWHTTPUTILS_H_ */
