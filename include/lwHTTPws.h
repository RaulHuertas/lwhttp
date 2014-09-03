/*
 * lwHTTPws.h
 *
 *  Created on: May 20, 2012
 *      Author: snorerax
 */

#ifndef LWHTTPWS_H_
#define LWHTTPWS_H_
#include "lwHTTPConfig.h"


#ifdef __cplusplus
extern "C" {
#endif


struct lwHTTPConnection;

typedef int (*lwWSConnStarted)(struct lwHTTPConnection* conn);
typedef int (*lwWSConnFinished)(struct lwHTTPConnection* conn);
typedef int (*lwWSMsgReceived)(struct lwHTTPConnection* conn);

struct lwWSAppHandler{
	lwWSConnStarted startCB;
	lwWSMsgReceived recvMsgCB;
	lwWSConnFinished endConnCB;
};

struct lwHTTPwsProtocolDescription{
	/**
	 *Identificador del protocolo
	 *Identificador dejarlo todo en cero
	 *para ignorarlo durante la
	 *revision de busqueda disponibles
	 * */
	char protocolName[LWHTTP_MAX_WS_PROTOCOL_NAME_LEN];
	struct lwWSAppHandler handler;
};

struct lwHTTPwsProtocolsList{
	struct lwHTTPwsProtocolDescription protocolsList[LWHTTP_MAX_WS_PROTOCOLS];
};

/**
 *la carga maxima de bytes, disminuida en dos
 * */
#define LWHTTP_WSFRAME_MAX_PAYLOAD					1024U
#define LWHTTP_WSMESSAGE_MAX_PAYLOAD				2048U
#define LWHTTP_WSMESSAGE_MAX_TX_MSG_SIZE			2048U
#define LWHTTP_WSMESSAGE_MAX_QUEUE					16U
#define LWHTTP_WSMESSAGE_MAX_QUEUE_MASK				(LWHTTP_WSMESSAGE_MAX_QUEUE-1U)

#define LWHTTP_WSFRAME_FIN_BMSK						0x80U
#define LWHTTP_WSFRAME_OPCODE_BMSK					0x0FU
#define LWHTTP_WSFRAME_MSK_BMSK						0x80U
#define LWHTTP_WSFRAME_LENGTH_BMSK					0x7FU

#define lwHTTP_WSFlags_FIN_FMSK						0x80U
#define lwHTTP_WSFlags_OPCODE_FMSK					0x0FU
#define lwHTTP_WSFlags_CTRL_FRAME_FMSK				0x20U
#define lwHTTP_WSFlags_ERROR_FRAME_FMSK				0x10U
#define	 lwHTTP_WSFlags_MASK_FMSK					0x40U

#define lwHTTP_WSOPCODE_CONT	0x00U
#define lwHTTP_WSOPCODE_TEXT	0x01U
#define lwHTTP_WSOPCODE_BINARY	0x02U
#define lwHTTP_WSOPCODE_CLOSE	0x08U
#define lwHTTP_WSOPCODE_PING	0x09U
#define lwHTTP_WSOPCODE_PONG	0x0AU

/**
 * Estructura que representa un frame decodificado
 **/
struct lwHTTP_WSFrame{
	lwHTTPU64 payloadPos;
	lwHTTPU64 payloadLength;
	lwHTTPU8 maskingKey[4];
	lwHTTPU8	flags;
	lwHTTPU8	len0;
	//lwHTTP8 payloadData[LWHTTP_WSFRAME_MAX_PAYLOAD];
};

struct lwHTTP_WSCtrlMessage{
	lwHTTPU8 messageData[LWHTTP_WSMESSAGE_MAX_PAYLOAD+4];
};

struct lwHTTP_WSMessage{
	lwHTTPU64 payloadPos;
	lwHTTPU64 payloadLength;
	lwHTTPU8	flags;
	lwHTTPU8 messageData[LWHTTP_WSMESSAGE_MAX_PAYLOAD+4];
};

struct lwHTTP_WSTXMessage{
	lwHTTPU32	length;
	lwHTTPU8 messageData[LWHTTP_WSMESSAGE_MAX_TX_MSG_SIZE];
};


//void lwHTTP_WSFrameHeader_Init(struct lwHTTP_WSFrameHeader* frameH);
void lwHTTP_WSFrameInit(struct lwHTTP_WSFrame* frame);
void lwHTTP_WSMessageInit(struct lwHTTP_WSMessage* msg);

#define LWHTTPWS_CTXSTATE_WAITING_FIN_OPCODE		0x00U
#define LWHTTPWS_CTXSTATE_WAITING_LENGTH_SHORT		0x01U
#define LWHTTPWS_CTXSTATE_WAITING_LENGTH_MEDIUM		0x02U
#define LWHTTPWS_CTXSTATE_WAITING_LENGTH_LARGE		0x03U
#define LWHTTPWS_CTXSTATE_WAITING_MASK				0x04U
#define LWHTTPWS_CTXSTATE_WAITING_PAYLOAD			0x05U
#define LWHTTPWS_CTXSTATE_FINISHED					0x80U

struct lwHTTPwsContext{
	lwHTTPU8 flags;
	struct lwHTTP_WSFrame frameBeingReceived;
	struct lwHTTP_WSMessage msgRcvd;
	struct lwHTTP_WSCtrlMessage controlFrameBeingReceived;
	lwHTTPU8 state;
	/**
	 * Este counter se usa en varias partes
	 * del analisis de la amquian de estados
	 * */
	lwHTTPU8 generalCounter;
	lwHTTPU8 orginalLength;

	//Variables para enviar mensajes
	//hacia el cliente
	lwHTTPU8 txMsgStart;
	lwHTTPU8 txMsgEnd;
	struct lwHTTP_WSTXMessage txMsgs[LWHTTP_WSMESSAGE_MAX_QUEUE];
};


//int lwHTTPCtx_AppendMessage(
//	struct lwHTTPConnection* conn,
//	struct lwHTTP_WSMessage* msg
//);

/***
 * Retorna:
 * 0 => Se ha agregado el mensjae, queda mas espacio
 * 1 => Se ha agregado el mensjae, pero el buffer quedo lleno
 *-1 => Buffer de envio lleno
 *
 *length ha de ser a lo mucho 125
 */
int lwHTTPCtx_TxData(
	struct lwHTTPConnection* conn,
	const char* data,
	char length,
	char isText
);

void lwHTTPContext_EvaluateReceivedWSMessage(
	struct lwHTTPConnection* conn
);

void lwHTTPwsContext_StartParseAgain(
	struct lwHTTPwsContext* context
);

void lwHTTPwsContext_Start(
	struct lwHTTPwsContext*
);

void lwHTTPwsParseCharacter(
	struct lwHTTPConnection* conn,
	lwHTTPU8 a
);

void lwHTTPwsEvaluateFrameEnd(
	struct lwHTTPConnection* conn
);

void lwHTTPConnection_startResponse(
	struct lwHTTPConnection* conn
);

/**
 * dst y origen son distintos
 * */
void lwHTTP_WSMaskData(
	lwHTTPU8* dst,
	const lwHTTPU8* org,
	const lwHTTPU8* maskkey,
	int length
);


#ifdef __cplusplus
}
#endif

#endif /* LWHTTPWS_H_ */
