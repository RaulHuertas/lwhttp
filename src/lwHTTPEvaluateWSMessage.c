/*
 * lwHTTPEvaluateWSMessage.c
 *
 *  Created on: Jun 30, 2012
 *      Author: snorerax
 */


#include "lwHTTP.h"
#include "lwHTTPws.h"
#include "lwHTTPUtils.h"
#include "string.h"

#include "lwip/err.h"
#include "lwip/tcp.h"

void lwHTTP_WSMaskData(
	lwHTTPU8* dst,
	const lwHTTPU8* org,
	const lwHTTPU8* maskKey, int length)
{
	int j;
	for(j=0;j<length;j++){
		dst[j] = org[j]^maskKey[j&0x03U];
	}
}

void lwHTTPContext_EvaluateReceivedWSMessage(struct lwHTTPConnection* conn){
	//Lo primero que hay que revisar es si se ha habido
	//un error en el ultimo frame recibido
	//en ese caso cerrar al conexion
	int j;
	struct lwHTTPwsContext* ctxt = &conn->webSocketContext;
	struct lwHTTP_WSFrame* frame = &ctxt->frameBeingReceived;
	if(frame->flags&lwHTTP_WSFlags_ERROR_FRAME_FMSK){
		lwHTTPConnection_Close(conn);
		return;
	}
	if(frame->flags&lwHTTP_WSFlags_CTRL_FRAME_FMSK){
		lwHTTPU8 opcode = frame->flags&lwHTTP_WSFlags_OPCODE_FMSK;
		//Verificar si es un comando CLOSE
		if(opcode == lwHTTP_WSOPCODE_CLOSE){
			LWHTTPDebug("WS: CLOSE command executed\n");
			lwHTTPConnection_Close(conn);
			conn->appWSHandler->endConnCB(conn);
			return;
		}
		//Verificar si es un comando PONG
		if(opcode == lwHTTP_WSOPCODE_PONG){
			LWHTTPDebug("WS: PONG ignored\n");
			//No hacer nada por ahora
			return;
		}
		if(opcode == lwHTTP_WSOPCODE_PING){
			LWHTTPDebug("WS: ANSWERING PING\n");
			//responder con un PONG
			conn->txBuffer[0]=LWHTTP_WSFRAME_FIN_BMSK|lwHTTP_WSOPCODE_PONG;
			conn->txBuffer[1]=frame->len0;
			conn->txSize = 2U;
			char lengthEndOffSet;
			switch(frame->len0){
				case 127U:{//Usar longitud de 64 bits
					conn->txBuffer[2] = (frame->payloadLength&0xFF00000000000000ULL)>>56;
					conn->txBuffer[3] = (frame->payloadLength&0x00FF000000000000ULL)>>48;
					conn->txBuffer[4] = (frame->payloadLength&0x0000FF0000000000ULL)>>40;
					conn->txBuffer[5] = (frame->payloadLength&0x000000FF00000000ULL)>>32;
					conn->txBuffer[6] = (frame->payloadLength&0x00000000FF000000ULL)>>24;
					conn->txBuffer[7] = (frame->payloadLength&0x0000000000FF0000ULL)>>16;
					conn->txBuffer[8] = (frame->payloadLength&0x000000000000FF00ULL)>>8;
					conn->txBuffer[9] = (frame->payloadLength&0x00000000000000FFULL);
				    lengthEndOffSet=10;
				    conn->txSize+=8;
				break;
				}
				case 126U:{
					conn->txBuffer[2] = (frame->payloadLength&0xFF00000000000000ULL)>>56;
					conn->txBuffer[3] = (frame->payloadLength&0x00FF000000000000ULL)>>48;
					lengthEndOffSet=4;
					conn->txSize+=2;
				break;
				}
				default:{
					lengthEndOffSet=2;
				break;
				}
			}
			lwHTTP_WSMaskData(
				&conn->txBuffer[(int)lengthEndOffSet],
				&ctxt->controlFrameBeingReceived.messageData[0],
				&frame->maskingKey[0],
				frame->payloadLength
			);
			conn->txSize+=frame->payloadLength;
			return;
		}
		return;
	}
	//INTERPRETAR EL MENSAJE
	//desenmascarar los datos
	LWHTTPDebug("WS: Datos recibidos: ");
	for(j = 0; j<ctxt->msgRcvd.payloadLength;j++){
		LWHTTPDebug("%2.2x", ctxt->msgRcvd.messageData[j]);
	}
	LWHTTPDebug("\n");
	conn->appWSHandler->recvMsgCB(conn);
	ctxt->msgRcvd.payloadLength = 0;
	ctxt->msgRcvd.payloadPos = 0;
}


