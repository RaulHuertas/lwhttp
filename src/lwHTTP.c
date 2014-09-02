/*
 * lwHTTP.c
 *
 *  Created on: May 29, 2012
 *      Author: snorerax
 */

#include "lwHTTP.h"
#include "lwHTTPUtils.h"
#include "string.h"

#include "lwip/err.h"
#include "lwip/tcp.h"





void lwHTTPConnection_Init(
	struct lwHTTPConnection* conn
){

	conn->listener = 0;
	conn->txSize = 0;
	//conn->conn.connState = LW_CONN_STATE_CLOSED;
	conn->caps = 0;
	conn->appWSHandler = 0;
	conn->requestEvaluator = 0;
	lwHTTPQueryParsingUtils_Start( &(conn->parsingUtils) );
	lwHTTPwsContext_Start(&(conn->webSocketContext));
	lwHTTP_WSMessageInit(&conn->webSocketContext.msgRcvd);
	conn->appHandler = 0;
	conn->rxBufEnd = 0;
	conn->rxBufStart = 0;
	conn->tpcb = 0;
	conn->subAppCtxt = 0;
}

err_t lwHTTPDispatcher_AcceptCallback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	LWHTTPDebug("Accept HTTP recibido\n\r");
	struct lwHTTPDispatcherAcceptArguments argsP;
	argsP.arg = arg;
	argsP.newpcb = newpcb;
	argsP.err = err;
	return lwHTTPDispatcher_EvaluateAccept(
			arg,
			&argsP
	);
}

err_t lwHTTPDispatcher_ReceiveCallback(void *arg, struct tcp_pcb *tpcb,
                               struct pbuf *p, err_t err);

void lwHTTPDispatcher_Init(
		struct lwHTTPDispatcher* dispatcher,
		int port
){
	err_t err;
	dispatcher->caps.clientValidator = 0;
	memset(
		&dispatcher->caps.wsProtocolsList,
		0,
		sizeof(dispatcher->caps.wsProtocolsList)
	);
	memset(
		&dispatcher->conns,
		0,
		sizeof(dispatcher->conns	)
	);
	dispatcher->pcb = tcp_new();
	dispatcher->port = port;
	dispatcher->nextConnectionToTry = 0;
	/* bind to specified @port */
	err = tcp_bind(dispatcher->pcb, IP_ADDR_ANY, port);
	if (err != ERR_OK) {
		LWHTTPDebug("Unable to bind to port %d: err = %d\n\r", port, err);
	}
	tcp_arg(dispatcher->pcb, dispatcher);
	dispatcher->pcb = tcp_listen(dispatcher->pcb);
	tcp_accept(dispatcher->pcb, lwHTTPDispatcher_AcceptCallback);

}

err_t lwHTTPDispatcher_ReceiveCallback(
	void *arg, struct tcp_pcb *tpcb,
    struct pbuf *p, err_t err)
{
	struct lwHTTPBaseConnection_RcvArgs rcvargs;
	rcvargs.arg = arg;
	rcvargs.tpcb = tpcb;
	rcvargs.p = p;
	rcvargs.err = err;
	return lwHTTPConnection_DataReceived(
			&rcvargs
	);
}

int lwHTTPDispatcher_EvaluateAccept(
	struct lwHTTPDispatcher* dispatcher,
	struct lwHTTPDispatcherAcceptArguments* args
){
	int j = 0;
	signed int connToUse = -1;
	LWHTTPDebug("Intento de conexion recibido \n\r");

	for(j=0; j<LWHTTP_MAX_CONNS;j++){
		if(dispatcher->conns[dispatcher->nextConnectionToTry].state==lwHTTPConnState_CLOSED){
			connToUse = dispatcher->nextConnectionToTry;
		}
		dispatcher->nextConnectionToTry++;
		dispatcher->nextConnectionToTry%=LWHTTP_MAX_CONNS;
		if(connToUse!=-1){
			break;
		}
	}
	if(connToUse==-1){	//No hay slots disponibles
						//para aceptar mas conexiones
		tcp_close(args->newpcb);
		tcp_recv(args->newpcb, NULL);
		LWHTTPDebug("Intento de conexion rechazado, NO HAY SLOTS DISPONIBLES \n\r");
		return ERR_OK;
	}
	//Se puede aceptar la conexion
	LWHTTPDebug("Se puede aceptar la conexion, slot %d \n\r", connToUse);
	lwHTTPConnection_Init(&dispatcher->conns[connToUse]);
	dispatcher->conns[connToUse].state = lwHTTPConnState_WAITING_DATA;
	dispatcher->conns[connToUse].connectionNumber = connToUse;
	dispatcher->conns[connToUse].tpcb = args->newpcb;
	dispatcher->conns[connToUse].caps = &dispatcher->caps;
	tcp_recv(	args->newpcb, lwHTTPDispatcher_ReceiveCallback			);
	tcp_arg(	args->newpcb, (void*)&(dispatcher->conns[connToUse]) 	);
	lwHTTPDefaultGetTime(	&(dispatcher->conns[connToUse].timeoutStartMark)	);
	return ERR_OK;
}

void lwHTTPQueryParsingUtils_Start(
	struct lwHTTPQueryParsingUtils* utils
){
	utils->scanedPointer = 0;
	memset(&utils->attrbs, 0, sizeof(utils->attrbs) );
	utils->firstLineReaded = 0;
	utils->attrsN = 0;
	utils->headerEnd = 0;
	utils->parseState = LWHTTP_PARSE_STATE_PROCESSING;
	utils->parsingRequestResponseErrorCode = LWHTTP_ERROR_CODE_NONE;
	utils->responseHeaderLength = 0U;
	utils->responseBodyLength = 0U;
	utils->scanFlags = 0U;
	utils->requestWSProtocol[0] = 0;
	utils->HTTPVersion[0] = 0;
	utils->requestMethod[0] = 0;
	utils->requestTarget[0] = 0;
}




