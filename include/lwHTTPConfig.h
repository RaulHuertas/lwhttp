/*
 * lwHTTPlibConfig.h
 *
 *  Created on: 29/04/2012
 *      Author: snorerax
 */

#ifndef LWHTTPLIBCONFIG_H_
#define LWHTTPLIBCONFIG_H_

#include "lwhttp_apineeds.h"


#ifndef LWHTTP_INIT_DEFAULT_CALLBACKS
#define LWHTTP_INIT_DEFAULT_CALLBACKS		1
#endif //LWHTTP_INIT_DEFAULT_CALLBACKS

#ifndef LWHTTP_CONN_TX_BUFFER_SIZE
#define LWHTTP_CONN_TX_BUFFER_SIZE				(4*1024U)
#endif //LWHTTP_CONN_TX_BUFFER_SIZE

#ifndef LWHTTP_CONN_RX_BUFFER_SIZE
#define LWHTTP_CONN_RX_BUFFER_SIZE				256U
#endif 	//LWHTTP_CONN_RX_BUFFER_SIZE

#define LWHTTP_CONN_RX_BUFFER_SIZE_MASK			(LWHTTP_CONN_RX_BUFFER_SIZE-1U)


#ifndef LWHTTP_MAX_CONNS
#define LWHTTP_MAX_CONNS						4
#endif //LWHTTP_MAX_CONNS

#ifndef LWHTTP_MAX_WRITE_SIZE
#define LWHTTP_MAX_WRITE_SIZE					1024
#endif LWHTTP_MAX_WRITE_SIZE

#ifndef LWHTTP_MAX_WS_PROTOCOL_NAME_LEN
#define LWHTTP_MAX_WS_PROTOCOL_NAME_LEN			16
#endif //LWHTTP_MAX_WS_PROTOCOL_NAME_LEN

#ifndef LWHTTP_MAX_WS_PROTOCOLS
#define LWHTTP_MAX_WS_PROTOCOLS					2
#endif //LWHTTP_MAX_WS_PROTOCOLS


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
