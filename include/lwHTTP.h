/*
 * lwHTTP.h
 *
 *  Created on: May 16, 2012
 *      Author: snorerax
 */

#ifndef LWHTTP_H_
#define LWHTTP_H_

#include "lwHTTPConfig.h"
#include "lwHTTPBaseConn.h"
#include "lwHTTPStack.h"
#include "lwHTTPws.h"
#include "lwHTTP_SiteLoader.h"

#ifdef __cplusplus
extern "C" {
#endif


struct lwHTTPClientValidator;
struct lwHTTPAppCapabilities;
struct lwHTTPBaseConnection;
struct lwHTTPConn_AppHandler;
struct lwHTTPRequestEvaluator;
struct lwHTTPClientValidator;

#define lwHTTPConnState_CLOSED 				0U
#define lwHTTPConnState_WAITING_DATA 		1U
#define lwHTTPConnState_ANSWERING_REQUEST 	2U
//#define lwHTTPConnState_WS 					3U

#define lwHTTPConn_Flag_CloseAfterTransmission			0x00000001U
#define lwHTTPConn_Flag_AfterTX_UpgradeToWS				0x00000002U
#define lwHTTPConn_Flag_IsWS							0x80000000U
#define lwHTTPConn_ClearWSTxEntry						0x40000000U
#define lwHTTPConn_DontCopyOnTrasmit					0x20000000U

struct lwHTTPConnection{
	struct tcp_pcb *tpcb;
	struct lwHTTPConnListener* listener;
	lwHTTPU8 rxBuf[LWHTTP_CONN_RX_BUFFER_SIZE];
	lwHTTPU16 rxBufStart, rxBufEnd;
	lwHTTPU8 txBuffer[LWHTTP_CONN_TX_BUFFER_SIZE];
	char* txBuffPointer;
	lwHTTPU32 txSize, txSentBytes;
	int connectionNumber;
	struct lwHTTPAppCapabilities* caps;

	struct lwHTTPRequestEvaluator* requestEvaluator;
	struct lwHTTPQueryParsingUtils parsingUtils;
	/**
	 * Se consulta este handle cuando la
	 * consulta pedida no es del protocolo web sockets.
	 * es decir, este handle se invoca cuando se ha
	 * recibido una consulta http standard
	 * */
	struct lwHTTPConn_AppHandler* appHandler;
	struct lwHTTPwsContext webSocketContext;
	struct lwWSAppHandler* appWSHandler;
	lwHTTPU8 state;
	lwHTTPU32 flags;
	lwHTTPTime timeoutStartMark;
	void* subAppCtxt;
	struct lwhttpSite* site;
};

struct lwHTTPAppCapabilities{
	struct lwHTTPwsProtocolsList wsProtocolsList;
	struct lwHTTPClientValidator* clientValidator;
};


struct lwHTTPDispatcherAcceptArguments{
	lwHTTPTime actualTime;
#ifdef LWHTTP_USE_lwIP_STACK
	void *arg;
	struct tcp_pcb *newpcb;
	err_t err;
#endif //LWHTTP_USE_lwIP_STACK
};

void lwHTTPTimeGetTime(lwHTTPTime* actualTime);

struct lwHTTPDispatcher{
	struct lwHTTPAppCapabilities caps;
	int nextConnectionToTry;
	struct lwHTTPConnection conns[LWHTTP_MAX_CONNS];
	struct tcp_pcb *pcb;
	int port;
	struct lwhttpSite* site;
};

struct lwHTTPClientValidator{
	/**
	 * Debe retornar 0 si se acepta al conexion y
	 *u otro còdigo si se ha rechazado la conexion
	 * */
	int (*isClientValidQ)(struct lwHTTPDispatcherAcceptArguments* );
};

struct lwHTTPConn_AppHandler{
	int (*lwHTTP_connectionInitialized)(struct lwHTTPConnection* );
	int (*lwHTTP_answerRequest)(struct lwHTTPConnection* );
	int (*lwHTTP_wsInit)(struct lwHTTPConnection* );
	int (*lwHTTP_wsAnswerMessage)(struct lwHTTPConnection* );
	int (*lwHTTP_wsTransmissionUpdate)(struct lwHTTPConnection* );
};

void lwHTTPDispatcher_Init(
		struct lwHTTPDispatcher* dispatcher,
		int port,
		struct lwhttpSite* site
);

int lwHTTPDispatcher_EvaluateAccept(
	struct lwHTTPDispatcher* dispatcher,
	struct lwHTTPDispatcherAcceptArguments* args
);

void lwHTTPConnection_Init(
	struct lwHTTPConnection* conn,
	struct lwhttpSite* site
);

void lwHTTPDispatcher_Evaluate(
		struct lwHTTPDispatcher* dispatcher
);

err_t lwHTTPConnection_DataReceived(
	struct lwHTTPBaseConnection_RcvArgs* args
);

void lwHTTPConnection_WS_TransmitData(
	struct lwHTTPConnection* conn,
	void* data,
	lwHTTPU32 dataLength
);

void lwHTTPConn_AppHandler_Init();

void lwHTTPConnection_Close(struct lwHTTPConnection* conn);

#ifdef __cplusplus
}
#endif

#endif /* LWHTTP_H_ */
