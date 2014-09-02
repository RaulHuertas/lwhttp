/*
 * lwHTTPWSEvaluation.c
 *
 *  Created on: Jun 14, 2012
 *      Author: snorerax
 */

#include "lwHTTPParse.h"
#include "lwHTTPws.h"
#include "string.h"
#include "shalib.h"
#include "lwhttpb64/lwhttp_cencode.h"


unsigned char concated[56];
unsigned char WSGUID [] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
Sha sha1TestCtxt;
unsigned char sha1Result[20];
unsigned char encoded64String[64];



void lwHTTP_InitWebSocket(struct lwHTTPConnection* conn, int protocol){
	LWHTTPDebug("inicialziando web sockets\n");
	struct lwHTTPAppCapabilities* caps = conn->caps;
	int j=0;
	if(strcmp(conn->parsingUtils.HTTPVersion, HTTP_V_1_1)!=0){
		goto badRequest;
	}
	//buscar version de web socket
	int versionValidQ = 0;
	for(j=0; j<conn->parsingUtils.attrsN; j++){
		if(strcmp(conn->parsingUtils.attrbs.attrbs[j].attrName, HTTP_WSVersionAttrName)){
			if(strcmp(conn->parsingUtils.attrbs.attrbs[j].attrValue, HTTP_WSVersion)){
				versionValidQ = 1;
			}
			break;
		}
	}
	if(!versionValidQ){
		LWHTTPDebug("WSDebug: version de WS invalida\n");
		goto badRequest;
	}
	int keyFound = -1;
	//Hasta aqui la cabecera es conforme, buscar la clave
	for(j=0; j<conn->parsingUtils.attrsN; j++){
		if(strcmp(conn->parsingUtils.attrbs.attrbs[j].attrName, HTTP_WSKeyAttrName)==0){
			keyFound = j;
			break;
		}
	}
	if(keyFound==-1){//no se encontro el atributo con la llave
		LWHTTPDebug("WSDebug: key not found\n");
		goto badRequest;
	}
	//Se encontro la llave
	int keyLength = strlen(conn->parsingUtils.attrbs.attrbs[keyFound].attrValue);
	if(keyLength!=24){
		LWHTTPDebug("WSDebug: Longitud de la llave no es 24(%d, last char %c)\n", keyLength, conn->parsingUtils.attrbs.attrbs[keyFound].attrValue[keyLength-1]);
		goto badRequest;
	}

	memcpy(concated, conn->parsingUtils.attrbs.attrbs[j].attrValue, 24);
	memcpy(&concated[24], WSGUID, 36);



	//Esta unica linea se encargan de calcuar el sha
	lwHTTP_CalculateSHA1(&sha1TestCtxt, concated, 60, sha1Result);



	LWHTTPDebug("WSDebug: SHA1 calculado: %x %x %x %x\n", sha1Result[0], sha1Result[1], sha1Result[2], sha1Result[3]);
	LWHTTPDebug("WSDebug:                 %x %x %x %x\n", sha1Result[4], sha1Result[5], sha1Result[6], sha1Result[7]);
	LWHTTPDebug("WSDebug:                 %x %x %x %x\n", sha1Result[8], sha1Result[9], sha1Result[10], sha1Result[11]);
	LWHTTPDebug("WSDebug:                 %x %x %x %x\n", sha1Result[12], sha1Result[13], sha1Result[14], sha1Result[15]);
	LWHTTPDebug("WSDebug:                 %x %x %x %x\n", sha1Result[16], sha1Result[17], sha1Result[18], sha1Result[19]);
	//encoded64String;
	base64_encodestate e64Ctxt;
	base64_init_encodestate(&e64Ctxt);
	int outSize = base64_encode_block((char*)&sha1Result[0], 20, (char*)&encoded64String[0], &e64Ctxt);
	LWHTTPDebug("WSDebug: base64_encode_blockend outSize %d\n", outSize);
	outSize += base64_encode_blockend((char*)&encoded64String[outSize], &e64Ctxt);
	LWHTTPDebug("WSDebug: base64_encode_blockend outSize %d\n", outSize);
	encoded64String[outSize-1] = 0;

//preparar la respuesta
	LWHTTPDebug("WSDebug: preparando respuesta para aceptar la conexion\n");
	conn->parsingUtils.responseHeaderLength+=sprintf(
		&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],
		lwHTTP_Template_WSReponseHeader
	);
	conn->parsingUtils.responseHeaderLength += sprintf(
			&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],
			lwHTTP_Template_WSUpgradeWS
	);
	conn->parsingUtils.responseHeaderLength += sprintf(
			&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],
			lwHTTP_Template_WSConnWS
	);
	conn->parsingUtils.responseHeaderLength += sprintf(
			&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],
			lwHTTP_Template_WSAccept,
			encoded64String
	);
	conn->parsingUtils.responseHeaderLength += sprintf(
				&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],
				lwHTTP_Template_WSProtocolResponse,
				caps->wsProtocolsList.protocolsList[protocol].protocolName
	);
	conn->parsingUtils.responseHeaderLength += sprintf(
				&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],
				lwHTTP_Template_FinalHeaderLine
	);
	lwHTTPConnection_startResponse(conn);
	conn->appWSHandler = &caps->wsProtocolsList.protocolsList[protocol].handler;
	conn->appWSHandler->startCB(conn);
	conn->flags|=lwHTTPConn_Flag_IsWS;
	return;

	//return bad request
badRequest:
	LWHTTPDebug("WSDebug: Query no valida, respondiendo BAD REQUEST...\n");
	conn->parsingUtils.responseHeaderLength+=sprintf(
		conn->parsingUtils.responseHeader,
		lwHTTP_Template_BadRequest,
		conn->parsingUtils.HTTPVersion
	);
	//conn->flags|=lwHTTPConn_Flag_CloseAfterTransmission;
	lwHTTPConnection_startResponse(conn);
	return;
}
