/*
 * lwHTTPUtils.h
 *
 *  Created on: May 20, 2012
 *      Author: snorerax
 */

#ifndef LWHTTPUTILS_H_
#define LWHTTPUTILS_H_
#include "lwHTTP.h"
#include "xtmrctr.h"

extern XTmrCtr* SystemTimerInstancePtr;

void lwHTTPDefaultGetTime(lwHTTPTime*  );

int lwHTTPDefaulIsClientValidQ(
	struct lwHTTPDispatcherAcceptArguments*
);


#endif /* LWHTTPUTILS_H_ */
