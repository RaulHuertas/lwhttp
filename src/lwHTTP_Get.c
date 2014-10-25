/*
 * lwHTTP_Get.c
 *
 *  Created on: Jun 10, 2012
 *      Author: snorerax
 */

#include "lwHTTP.h"
#include "lwHTTPParse.h"
#include <string.h>
#include "xsysace.h"
#include "sysace_stdio.h"
#include "murmurHash.h"

void lwHTTP_executeGET(struct lwHTTPConnection* conn){

	int j = 0;
	LWHTTPDebug("LWHTTPDebug: Respondiendo peticion de GET\n");
	char isAHeadRequest = (conn->parsingUtils.requestMethod[0]=='H');


	int responseIndex = -1;
	//SYSACE_FILE *indexFile;


	conn->parsingUtils.responseHeaderLength = 0;
	conn->parsingUtils.responseBodyLength = 0;
	LWHTTPDebug(
		"LWHTTPDebug: start character in GET target: %x %x \n",
		conn->parsingUtils.requestTarget[0],
		conn->parsingUtils.requestTarget[1]
	);


	//checkIfIsWebSockets://verificar si hay una aplicacion
	//web sockets regustrada para este protocolo
	if(
			(conn->parsingUtils.scanFlags & scanFlags_UpgadeWSDetected)&&
			(conn->parsingUtils.scanFlags & scanFlags_ProtocolWSDetected)
	){
		struct lwHTTPAppCapabilities* caps = conn->caps;
		int isWS = -1;
		for(j=0; j<LWHTTP_MAX_WS_PROTOCOLS;j++){
			if(
				strcmp(
					caps->wsProtocolsList.protocolsList[j].protocolName,
					conn->parsingUtils.requestWSProtocol
				)==0
			){
				//protocolo detectado
				LWHTTPDebug("LWHTTPDebug: Conexion WS %s aceptada, protocolo encontrado \n", conn->parsingUtils.requestWSProtocol);
				isWS = j;
				break;
			}
		}
		if(isWS!=-1){
			//Protocolo web sockets aceptado
			lwHTTP_InitWebSocket(conn, isWS);
			return;
		}else{
			LWHTTPDebug("LWHTTPDebug: WS protocol %s not found\n", conn->parsingUtils.requestWSProtocol);
		}
	}

	//No es una consulta de contenido estático
	//
	conn->txBuffPointer = 0;

	//RESOLVER LAS PETICIONES MAS COMUNES
	if( (conn->parsingUtils.requestTarget[0]=='/') && (conn->parsingUtils.requestTarget[1]==0) ){
		responseIndex = conn->site->exportHomeFileIndex;
		goto responder;
	}
//	if( strcmp(conn->parsingUtils.requestTarget,"/index.html")==0 ){
//		responseIndex = conn->site->exportHomeFileIndex;
//		goto responder;
//	}
	if( strcmp(conn->parsingUtils.requestTarget,"/index.htm")==0 ){
		responseIndex = conn->site->exportHomeFileIndex;
		goto responder;
	}
	if( strcmp(conn->parsingUtils.requestTarget,"index.htm")==0 ){
		responseIndex = conn->site->exportHomeFileIndex;
		goto responder;
	}
//	if( strcmp(conn->parsingUtils.requestTarget,"index.html")==0 ){
//		responseIndex = conn->site->exportHomeFileIndex;
//		goto responder;
//	}
	//nos pide otro archivo, hay que buscarlo
	int urlLen = strlen(conn->parsingUtils.requestTarget);
	lwHTTPU32 reqHash = GenerateMurmurHash32(conn->parsingUtils.requestTarget, urlLen, conn->site->seed);
	LWHTTPDebug("LWHTTPDebug: url %s, hash generado %8.8x\n", conn->parsingUtils.requestTarget, reqHash);
	responseIndex = isHashInPack(reqHash, conn->site);
	if( responseIndex == -1 ){
		LWHTTPDebug("lwHTTP: File not found on FS\n");
		goto recursoNoEncontrado;
	}

responder://Buscar si es un archivo en el sistema de archivos

	lwHTTPConnection_startStaticResourceResponse(conn, responseIndex, isAHeadRequest);
	return;//responder

recursoNoEncontrado:
	LWHTTPDebug("LWHTTPDebug: Archivo no encontrado, respondiendo NOT FOUND...\n");
	if(conn->site->exportErrorFileIndex<(conn->site->tupplesN-1)){//Existe página de error
		lwHTTPConnection_startStaticResourceResponse(conn, conn->site->exportErrorFileIndex, 0);
	}else{//Cerrar la conexión
		LWHTTPDebug("LWHTTPDebug: Cerrando conexion\n");
		lwHTTPConnection_Close(conn);
	}

	//conn->flags |= lwHTTPConn_Flag_CloseAfterTransmission;
	return;//recursoNoEncontrado

}







