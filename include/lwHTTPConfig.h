/*
 * lwHTTPlibConfig.h
 *
 *  Created on: 29/04/2012
 *      Author: snorerax
 */

#ifndef LWHTTPLIBCONFIG_H_
#define LWHTTPLIBCONFIG_H_

#include "lwhttp_apineeds.h"

#define LWHTTP_DEBUG_VERBOSE 				1
#define LWHTTP_INIT_DEFAULT_CALLBACKS		1



#define LWHTTP_CONN_TX_BUFFER_SIZE				(4U*1024U*1024U)

#define LWHTTP_CONN_RX_BUFFER_SIZE				1024U
#define LWHTTP_CONN_RX_BUFFER_SIZE_MASK			(LWHTTP_CONN_RX_BUFFER_SIZE-1U)

#define LWHTTP_MAX_CONNS						8

#define LWHTTP_MAX_WRITE_SIZE					1024
//#define LWHTTP_MAX_WRITE_SIZE_MASK				(LWHTTP_MAX_WRITE_SIZE-1)

#define LWHTTP_MAX_WS_PROTOCOL_NAME_LEN			16
#define LWHTTP_MAX_WS_PROTOCOLS					4





#ifdef LWHTTP_USE_lwIP_STACK
#include "lwip/err.h"
#include "lwip/tcp.h"
#endif //USE_LW_STACK

/**Tiempo en ms*/
#ifdef __MICROBLAZE__
    #include "lwHTTPConfig_microblaze.h"
#else
    typedef u32_t lwHTTPTime;
    typedef long long lwHTTP64;
    typedef unsigned char 	lwHTTP8;
    typedef unsigned short 	lwHTTP16;
    typedef unsigned int 		lwHTTP32;
#endif






#endif /* LWHTTPLIBCONFIG_H_ */
