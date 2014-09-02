/*
 * lwHTTPParse.c
 *
 *  Created on: Jun 6, 2012
 *      Author: snorerax
 */
#include "lwHTTPParse.h"
#include "string.h"


void lwHTTP_parseCharacter(struct lwHTTPConnection* conn, char a){
	switch(a){
		case 0x0DU:{
			break;
		}
		case 0x0AU:{//Fin de linea
			conn->parsingUtils.lineBeingScaned[conn->parsingUtils.scanedPointer]=0x00;
			if(!conn->parsingUtils.firstLineReaded){
				if(
					3!=sscanf(conn->parsingUtils.lineBeingScaned,
								"%s %s %s",
								conn->parsingUtils.requestMethod,
								conn->parsingUtils.requestTarget,
								conn->parsingUtils.HTTPVersion)
				){
					//Query no es correcta
					conn->parsingUtils.parsingRequestResponseErrorCode = LWHTTP_ERROR_CODE_BADFORMAT;
					conn->parsingUtils.parseState = LWHTTP_PARSE_STATE_FINISHED;
					break;
				}
				LWHTTPDebug("    HTTPVersion : %s\n", conn->parsingUtils.HTTPVersion );
				conn->parsingUtils.firstLineReaded = 1;
				LWHTTPDebug("    QUERY requestMethod: %s\n", conn->parsingUtils.requestMethod );
				LWHTTPDebug("    QUERY requestTarget: %s\n", conn->parsingUtils.requestTarget );
				if(strcmp(conn->parsingUtils.requestTarget, "/")){
					conn->parsingUtils.scanFlags |= scanFlags_SimpleTarget;
				}
			}else{
				//la cabecera de consulta ya esta leida
				//Leer el resto de atributos
				if(conn->parsingUtils.scanedPointer==0){	//linea vacia
													//fin de consulta
					conn->parsingUtils.parsingRequestResponseErrorCode = LWHTTP_ERROR_CODE_NONE;
					conn->parsingUtils.parseState = LWHTTP_PARSE_STATE_FINISHED;
					LWHTTPDebug("Fin de consulta\n");
					break;
				}

				if(
					2!=sscanf(
							conn->parsingUtils.lineBeingScaned,
							"%s %s",
							conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrName,
							conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrValue)
				){
					//Query no es correcta
					conn->parsingUtils.parsingRequestResponseErrorCode = LWHTTP_ERROR_CODE_BADFORMAT;
					conn->parsingUtils.parseState = LWHTTP_PARSE_STATE_FINISHED;
					break;
				}
				//El valor del atributo puede consistir en varias cadenas. Se necesita copiar todas
				int attrLen = strlen(conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrName);
				strcpy(
					conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrValue,
					&(conn->parsingUtils.lineBeingScaned[attrLen+1])//+1 para eliminar el primer espacio en blanco
				);
				LWHTTPDebug("    ATTR interpretado: \"%s\"\n", conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrName );
				LWHTTPDebug("    ATTR value interpretado: \"%s\"\n", conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrValue );
				//Verificar que si el target es "/" se pueda evaluar si se ha recibido
				//una peticion de conexion por websockets
				if(strcmp(conn->parsingUtils.lineBeingScaned, lwHTTP_Template_WSUpgradeWSComp)==0){	//Comprobar si se esta pidendo usar
					//WebSockets
					conn->parsingUtils.scanFlags |= scanFlags_UpgadeWSDetected;
					LWHTTPDebug("WS: upgrade detected\n");
				}
				if(strcmp(conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrName, lwHTTP_Template_WSProtocolResponseAttrName)==0){
					conn->parsingUtils.scanFlags |= scanFlags_ProtocolWSDetected;
					LWHTTPDebug("WS: protocol detected: %s\n", conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrValue);
					strcpy(conn->parsingUtils.requestWSProtocol, conn->parsingUtils.attrbs.attrbs[conn->parsingUtils.attrsN].attrValue);
				}
				conn->parsingUtils.attrsN++;//Se ha podido interpretar otro atributo
			}
			conn->parsingUtils.scanedPointer = 0;
			break;
		}
		default:{
			//Mientras los datos recibido no sobrepasen los
			//limites de los buffers asignados, guardar los
			//caracteres recibidos
			if(conn->parsingUtils.scanedPointer<(LWHTTPLIB_MAX_ATTRB_STRING_MAX_LEN-1)){
				conn->parsingUtils.lineBeingScaned[conn->parsingUtils.scanedPointer]=a;
				conn->parsingUtils.scanedPointer++;

			}
			break;
		}
	}
}





void lwHTTPConnection_execRequest(struct lwHTTPConnection* conn){

	conn->parsingUtils.responseBodyLength = 0;
	conn->parsingUtils.responseHeaderLength = 0;
	conn->flags = 0U;
	switch(conn->parsingUtils.parsingRequestResponseErrorCode){
		case LWHTTP_ERROR_CODE_NONE:{
			LWHTTPDebug("Conexion %d, peticion valida\n", conn->connectionNumber);
			lwHTTP_answerRequest(conn);
			break;
		}
		case LWHTTP_ERROR_CODE_GENERIC:{
			conn->parsingUtils.responseHeaderLength+=sprintf(
				conn->parsingUtils.responseHeader,
				lwHTTP_Template_BadRequest,
				conn->parsingUtils.HTTPVersion
			);
			//conn->flags|=lwHTTPConn_Flag_CloseAfterTransmission;
			lwHTTPConnection_startResponse(conn);

			break;
		}
		case LWHTTP_ERROR_CODE_UNKNOWN:{
			conn->parsingUtils.responseHeaderLength+=sprintf(
				conn->parsingUtils.responseHeader,
				lwHTTP_Template_BadRequest,
				conn->parsingUtils.HTTPVersion
			);
			//conn->flags|=lwHTTPConn_Flag_CloseAfterTransmission;
			lwHTTPConnection_startResponse(conn);
			break;
		}
		case LWHTTP_ERROR_CODE_BADFORMAT:{
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

}

void lwHTTPConnection_ParseRcvdData(
	struct lwHTTPBaseConnection_RcvArgs* args
){
	int j = 0;
	struct lwHTTPConnection* conn = (struct lwHTTPConnection* )args->arg;
	for(j=0;j<args->p->len; j++){
		lwHTTP_parseCharacter(
				conn,
				((char*)args->p->payload)[j]
		);
		if(conn->parsingUtils.parseState == LWHTTP_PARSE_STATE_FINISHED){
			lwHTTPConnection_execRequest(conn);
			conn->parsingUtils.parseState = LWHTTP_PARSE_STATE_PROCESSING;
			lwHTTPQueryParsingUtils_Start(&conn->parsingUtils);
		}

	}
}


