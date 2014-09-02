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


void lwHTTP_executeGET(struct lwHTTPConnection* conn){

	int j = 0;
	LWHTTPDebug("LWHTTPDebug: Respondiendo peticion de GET\n");
	char isAHeadRequest = (conn->parsingUtils.requestMethod[0]=='H');
	char fileAsked[16];
	char fileName[20]="WS";
	SYSACE_FILE *indexFile;
	lwHTTPU32 nBytesReadedFromFile;
	//lwHTTP32 nBytes;
	memset(fileAsked, 0, sizeof(fileAsked));
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


	//RESOLVER LAS PETICIONES MAS COMUNES
	if( (conn->parsingUtils.requestTarget[0]=='/') && (conn->parsingUtils.requestTarget[1]==0) ){
		strcpy(fileAsked, "\\index.htm");
		LWHTTPDebug("LWHTTPDebug: db1\n");
		goto responder;
	}
	if( strcmp(conn->parsingUtils.requestTarget,"/index.html")==0 ){
		strcpy(fileAsked, "\\index.htm");
		LWHTTPDebug("LWHTTPDebug: db2\n");
		goto responder;
	}
	if( strcmp(conn->parsingUtils.requestTarget,"/index.htm")==0 ){
		strcpy(fileAsked, "\\index.htm");
		LWHTTPDebug("LWHTTPDebug: db3\n");
		goto responder;
	}
	if( strcmp(conn->parsingUtils.requestTarget,"index.htm")==0 ){
		strcpy(fileAsked, "\\index.htm");
		LWHTTPDebug("LWHTTPDebug: db4\n");
		goto responder;
	}
	if( strcmp(conn->parsingUtils.requestTarget,"index.html")==0 ){
		strcpy(fileAsked, "\\index.htm");
		LWHTTPDebug("LWHTTPDebug: db5\n");
		goto responder;
	}
	//nos pide otro archivo, hay que buscarlo
	conn->parsingUtils.requestTarget[12]=0;
	strcpy(fileAsked, conn->parsingUtils.requestTarget);
	LWHTTPDebug("LWHTTPDebug: %s \n", fileAsked);


responder://Buscar si es un archivo en el sistema de archivos
	//Reemplazar los / con '\'
	j=0;
	while(	fileAsked[j]!= 0){
		if(	fileAsked[j]=='/'	){
			fileAsked[j]='\\';
		}
		j++;
	}
	if(fileAsked[0]!='\\'){
		strcat(fileName,"\\");
	}
	strcat(fileName,fileAsked);//el path completo
	LWHTTPDebug("LWHTTPDebug: Archivo a enviar: %s\n", fileName);
	indexFile = sysace_fopen(fileName, "r");
	if( indexFile == 0 ){
		LWHTTPDebug("File not found on FS\n");
		goto recursoNoEncontrado;
	}
	//Archivo a enviar encontrado
	while(1){
		nBytesReadedFromFile = sysace_fread(
				&conn->parsingUtils.responseBody[conn->parsingUtils.responseBodyLength],
				1,
				128,
				indexFile
		);
		if(nBytesReadedFromFile>0){
			conn->parsingUtils.responseBodyLength+=nBytesReadedFromFile;
		}
		if( nBytesReadedFromFile<128 ){
			break;
		}
	}
	sysace_fclose(indexFile);
	LWHTTPDebug("LWHTTPDebug: Longitud del archivo  a enviar %d\n", conn->parsingUtils.responseBodyLength);
	conn->parsingUtils.responseHeaderLength += sprintf(
			&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],
			lwHTTP_Template_BasicGETResponse,
			conn->parsingUtils.HTTPVersion
	);
	if(isAHeadRequest){//HEAD, Es solamente un head, no enviar el cuerpo
		conn->parsingUtils.responseBodyLength = 0;
	}
	//grabamos la longitud de la respuesta
	conn->parsingUtils.responseHeaderLength += sprintf(
			&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],//solo se usa como buffer temporal,
			lwHTTP_Template_Attr_ContentLength,
			conn->parsingUtils.responseBodyLength
	);
	conn->parsingUtils.responseHeaderLength += sprintf(
			&conn->parsingUtils.responseHeader[conn->parsingUtils.responseHeaderLength],
			lwHTTP_Template_FinalHeaderLine
	);
	lwHTTPConnection_startResponse(conn);
	return;//responder

recursoNoEncontrado:
	LWHTTPDebug("Archivo no encontrado, respondiendo NOT FOUND...");
	conn->parsingUtils.responseHeaderLength+=sprintf(
		conn->parsingUtils.responseHeader,
		lwHTTP_Template_NotFound,
		conn->parsingUtils.HTTPVersion
	);
	lwHTTPConnection_startResponse(conn);
	return;//recursoNoEncontrado

}







