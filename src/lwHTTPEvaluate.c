/*
 * lwHTTPEvaluate.c
 *
 *  Created on: Jun 9, 2012
 *      Author: snorerax
 */

#include "lwHTTP.h"
#include "lwHTTPParse.h"
#include "lwHTTPUtils.h"
#include "string.h"


void lwHTTPConnection_Close(struct lwHTTPConnection* conn){
	tcp_close(conn->tpcb);
	tcp_recv(conn->tpcb, 0);
	conn->state = lwHTTPConnState_CLOSED;
}

void lwHTTPDispatcher_EvaluateConn(
		struct lwHTTPConnection* conn
){

	switch(conn->state){
		case lwHTTPConnState_WAITING_DATA:{
			//Si hay datos que transmitir,
			//pasar al estado de trnsmitir
			if(conn->flags&lwHTTPConn_Flag_IsWS){
				struct lwHTTPwsContext* wsCtx = &conn->webSocketContext;
				if(wsCtx->txMsgStart!=wsCtx->txMsgEnd){
					LWHTTPDebug("LWHTTPDebug: Enviando frame WS address %d\n",
							(char*)&wsCtx->txMsgs[wsCtx->txMsgStart].messageData[0]
					);
					memcpy(
						 (char*)&conn->txBuffer[0],
						 (char*)&wsCtx->txMsgs[wsCtx->txMsgStart].messageData[0],
						 wsCtx->txMsgs[wsCtx->txMsgStart].length
					);
					conn->txSize = wsCtx->txMsgs[wsCtx->txMsgStart].length;
					conn->txSentBytes = 0U;
					conn->state = lwHTTPConnState_ANSWERING_REQUEST;//a transmitir
					conn->flags|=lwHTTPConn_ClearWSTxEntry;
					break;
				}
			}
			if(conn->rxBufEnd!=conn->rxBufStart){
				while(conn->rxBufEnd!=conn->rxBufStart){

					if(conn->flags&lwHTTPConn_Flag_IsWS){
						//Los datos que se rciben son frames
						//WS
						lwHTTPwsParseCharacter(
								conn,
								conn->rxBuf[conn->rxBufStart]
						);
						if(conn->webSocketContext.state==LWHTTPWS_CTXSTATE_FINISHED){
							//if(conn->webSocketContext.state==LWHTTPWS_CTXSTATE_FINISHED){
							lwHTTPContext_EvaluateReceivedWSMessage(conn);
							lwHTTPwsContext_StartParseAgain(&conn->webSocketContext);
							//}
						}

					}else{
						lwHTTP_parseCharacter(
								conn,
								conn->rxBuf[conn->rxBufStart]
						);
						if(conn->parsingUtils.parseState == LWHTTP_PARSE_STATE_FINISHED){
							lwHTTPConnection_execRequest(conn);
							conn->parsingUtils.parseState = LWHTTP_PARSE_STATE_PROCESSING;
							lwHTTPQueryParsingUtils_Start(&conn->parsingUtils);
						}
					}

					conn->rxBufStart++;conn->rxBufStart&=LWHTTP_CONN_RX_BUFFER_SIZE_MASK;
				}
			}else{
				lwHTTPTime actualTime;
				lwHTTPDefaultGetTime(&actualTime);
				//lwHTTPTime elapsedTime = (conn->timeoutStartMark-actualTime);
				lwHTTPTime elapsedTime = (actualTime-conn->timeoutStartMark);
//				LWHTTPDebug(
//					"LWHTTPDebug: cnxtn %d elpTime %d\n",
//					conn->connectionNumber,
//					elapsedTime
//				);
				if(elapsedTime>LWHTTP_MAX_CONN_TIMEOUT){
					lwHTTPConnection_Close(conn);
					LWHTTPDebug(
						"LWHTTPDebug: cerrando conexion %d por timeout %d %d %d \n",
						conn->connectionNumber,
						conn->timeoutStartMark,
						actualTime,
						elapsedTime
					);
				}
			}

			break;
		}
		case lwHTTPConnState_ANSWERING_REQUEST:{
			//LWHTTPDebug("LWHTTPDebug: params recevd %x %x", conn, conn->tpcb );
			LWHTTPDebug("LWHTTPDebug: sent bytes %d\n", conn->txSentBytes);
			lwHTTPU16 bytesToWriteNow = (conn->txSize-conn->txSentBytes);
			if(bytesToWriteNow>LWHTTP_MAX_WRITE_SIZE){
				bytesToWriteNow = LWHTTP_MAX_WRITE_SIZE;
			}
			LWHTTPDebug("transmitiendo %d bytes \n", bytesToWriteNow);
			err_t result = tcp_write(
				conn->tpcb,
				&conn->txBuffer[conn->txSentBytes],
				bytesToWriteNow,
				TCP_WRITE_FLAG_COPY
			);
			LWHTTPDebug("LWHTTPDebug: Orden de escritura enviada\n");
			switch(result){
				case ERR_OK:{
					conn->txSentBytes+=bytesToWriteNow;
					if(conn->txSentBytes==conn->txSize){
						//Ya se enviaron todos los datos
						conn->state = lwHTTPConnState_WAITING_DATA;
						LWHTTPDebug(
							"LWHTTPDebug: conn %d Se han enviado todos los datos\n",
							conn->connectionNumber
						);
						if(conn->flags&lwHTTPConn_Flag_CloseAfterTransmission){
							lwHTTPConnection_Close(conn);
							conn->flags&=~lwHTTPConn_Flag_CloseAfterTransmission;
						}
						if(conn->flags&lwHTTPConn_Flag_AfterTX_UpgradeToWS){
							//lwHTTPConnection_Close(conn);
							//Ir al estado de Websockets
							conn->flags&=~lwHTTPConn_Flag_AfterTX_UpgradeToWS;
							conn->flags|=lwHTTPConn_Flag_IsWS;
						}
						if(conn->flags&lwHTTPConn_ClearWSTxEntry){
							struct lwHTTPwsContext* wsCtx = &conn->webSocketContext;
							LWHTTPDebug("WS_TX: Deleting index %d\n", wsCtx->txMsgStart);
							wsCtx->txMsgStart++;
							wsCtx->txMsgStart&=LWHTTP_WSMESSAGE_MAX_QUEUE_MASK;
						}
					}
					break;
				}
				case ERR_MEM:{
					//No hace nada, cuando se regrese a evaluar
					//el estado de esta conexion
					//se podran enviar mas datos
					break;
				}
				default:{
					LWHTTPDebug("LWHTTPDebug: conn %d Error %x transmitiendo datos\n", conn->connectionNumber, result);
					lwHTTPConnection_Close(conn);
					break;
				}
			}

			break;
		}

	}



}


void lwHTTPDispatcher_Evaluate(
		struct lwHTTPDispatcher* dispatcher
){

	int i = 0;
	struct lwHTTPConnection* conn;

	for(i = 0; i<LWHTTP_MAX_CONNS; i++){
		conn = (struct lwHTTPConnection* )&(dispatcher->conns[i]);
		if(conn->state==lwHTTPConnState_CLOSED){
			continue;
		}
		lwHTTPDispatcher_EvaluateConn(conn);
	}

}
