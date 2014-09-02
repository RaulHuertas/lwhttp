/*
 * lwHTTPAnswerRequest.c
 *
 *  Created on: Jun 10, 2012
 *      Author: snorerax
 */

#include "lwHTTP.h"
#include "lwHTTPParse.h"
#include "string.h"


void lwHTTP_answerRequest(struct lwHTTPConnection* conn){
	conn->flags = 0U;
	while(1){//esto no es un bucle
		if(strcmp(conn->parsingUtils.requestMethod, "GET")==0){
			lwHTTP_executeGET(conn);
			break;
		}
		if(strcmp(conn->parsingUtils.requestMethod, "HEAD")==0){
			lwHTTP_executeGET(conn);
			break;
		}
		//Si se llega hasta aqui es porque el metodo
		//solicitado es invalido
		conn->parsingUtils.responseHeaderLength+=sprintf(
			conn->parsingUtils.responseHeader,
			lwHTTP_Template_BadRequest,
			conn->parsingUtils.HTTPVersion
		);
		//conn->flags|=lwHTTPConn_Flag_CloseAfterTransmission;
		lwHTTPConnection_startResponse(conn);
		break;
	}
}

void lwHTTPConnection_startResponse(struct lwHTTPConnection* conn){
	//Juntar la cabecera con el BODY de la respuesta
	memcpy(
		conn->txBuffer,
		conn->parsingUtils.responseHeader,
		conn->parsingUtils.responseHeaderLength
	);
	memcpy(
		&conn->txBuffer[conn->parsingUtils.responseHeaderLength],
		conn->parsingUtils.responseBody,
		conn->parsingUtils.responseBodyLength
	);
	conn->txSize = conn->parsingUtils.responseHeaderLength+conn->parsingUtils.responseBodyLength;
	conn->state = lwHTTPConnState_ANSWERING_REQUEST;
	LWHTTPDebug("LWHTTPDebug> Header to send: %s", conn->parsingUtils.responseHeader);
	conn->txSentBytes = 0;
}



