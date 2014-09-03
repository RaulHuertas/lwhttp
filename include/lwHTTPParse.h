/*
 * lwHTTPParse.h
 *
 *  Created on: Jun 5, 2012
 *      Author: snorerax
 */

#ifndef LWHTTPPARSE_H_
#define LWHTTPPARSE_H_

#include "lwHTTP.h"


#ifdef __cplusplus
extern "C" {
#endif

#define HTTP_V_1_1	"HTTP/1.1"
#define HTTP_WSKeyAttrName	"Sec-WebSocket-Key:"
#define HTTP_WSVersionAttrName	"Sec-WebSocket-Version:"
#define HTTP_WSVersion	"13"

void lwHTTPConnection_ParseRcvdData(
	struct lwHTTPBaseConnection_RcvArgs* args
);

void lwHTTP_parseCharacter(struct lwHTTPConnection* conn, char a);

void lwHTTPConnection_startResponse(
	struct lwHTTPConnection* conn
);

void lwHTTPConnection_execRequest(
	struct lwHTTPConnection* conn
);

void lwHTTP_answerRequest(
	struct lwHTTPConnection* conn
);

void lwHTTP_executeGET(struct lwHTTPConnection* conn);

void lwHTTP_InitWebSocket(struct lwHTTPConnection*, int);



extern const char* lwHTTP_Template_BadRequest;
extern const char* lwHTTP_Template_NotFound;
extern const char* lwHTTP_Template_BasicGETResponse;
extern const char* lwHTTP_Template_Attr_ContentLength;
extern const char* lwHTTP_Template_Attr_ContentType;
extern const char* lwHTTP_Template_FinalHeaderLine;
extern const char* lwHTTP_Template_WSReponseHeader;
extern const char* lwHTTP_Template_WSUpgradeWS;
extern const char* lwHTTP_Template_WSUpgradeWSComp;
extern const char* lwHTTP_Template_WSConnWS;
extern const char* lwHTTP_Template_WSAccept;
extern const char* lwHTTP_Template_WSProtocolResponse;
extern const char* lwHTTP_Template_WSProtocolResponseAttrName;



#ifdef __cplusplus
}
#endif




#endif /* LWHTTPPARSE_H_ */
