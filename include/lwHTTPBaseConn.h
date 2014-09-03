/*
 * lwConn.h
 *
 *  Created on: May 20, 2012
 *      Author: snorerax
 */

#ifndef LWCONN_H_
#define LWCONN_H_

#include "lwHTTPConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LW_CONN_STATE_CLOSED 			0
#define LW_CONN_STATE_WAITING_DATA 		1
#define LW_CONN_STATE_TRANSMIT_DATA 	2

struct lwHTTPConnListener;
struct lwHTTPConnection;



struct lwHTTPBaseConnection_RcvArgs{
	#ifdef LWHTTP_USE_lwIP_STACK
	void *arg;
	struct tcp_pcb *tpcb;
	struct pbuf *p;
	err_t err;
	#endif //
};


#ifdef __cplusplus
extern "C" {
#endif


#endif /* LWCONN_H_ */
