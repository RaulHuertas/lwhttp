/*
 * lwHTTPStack.h
 *
 *  Created on: May 20, 2012
 *      Author: snorerax
 */

#ifndef LWHTTPSTACK_H_
#define LWHTTPSTACK_H_

#define LWHTTPLIB_PORT												80
#define LWHTTPLIB_MAX_ATTRB_NAME_MAX_LEN 							512
#define LWHTTPLIB_MAX_ATTRB_VALUE_MAX_LEN 							512
#define LWHTTPLIB_MAX_ATTRB_STRING_MAX_LEN 							512
#define LWHTTPLIB_MAX_ATTRBUTESN 									16
#define LWHTTPLIB_RESPONSE_HEADER_LENGHT							512
#define LWHTTPLIB_MAX_RESPONSE_BODY_LENGHT							1048576

#define LWHTTP_PARSE_STATE_PROCESSING 	0
#define LWHTTP_PARSE_STATE_FINISHED 	1

#define LWHTTP_ERROR_CODE_NONE 			0
#define LWHTTP_ERROR_CODE_GENERIC 		1
#define LWHTTP_ERROR_CODE_UNKNOWN 		2
#define LWHTTP_ERROR_CODE_BADFORMAT 	3

struct lwHTTPAttrEntry{
	char attrName[LWHTTPLIB_MAX_ATTRB_NAME_MAX_LEN];
	char attrValue[LWHTTPLIB_MAX_ATTRB_VALUE_MAX_LEN];
};

struct lwHTTPQueryAttributes{
	struct lwHTTPAttrEntry attrbs[LWHTTPLIB_MAX_ATTRBUTESN];
};

#define LWHTTP_ERROR_CODE_NONE 			0
#define LWHTTP_ERROR_CODE_GENERIC 		1
#define LWHTTP_ERROR_CODE_UNKNOWN 		2
#define LWHTTP_ERROR_CODE_BADFORMAT 	3

#define scanFlags_SimpleTarget					0x01U
#define scanFlags_UpgadeWSDetected		0x02U
#define scanFlags_ProtocolWSDetected		0x04U

struct lwHTTPQueryParsingUtils{
	char lineBeingScaned[LWHTTPLIB_MAX_ATTRB_STRING_MAX_LEN];
	lwHTTPU16 scanedPointer;
	char requestMethod[16];
	char requestTarget[256];
	char requestWSProtocol[256];
	char scanFlags;//si el target es "/"
	char HTTPVersion[64];
	struct lwHTTPQueryAttributes attrbs;
	char firstLineReaded;
	lwHTTPU16 attrsN;
	lwHTTPU16 headerEnd;
	int parseState;
	int parsingRequestResponseErrorCode;
	char responseHeader[LWHTTPLIB_RESPONSE_HEADER_LENGHT+2];//2 mas para cuando se use como buffer temporal
	lwHTTPU32 responseHeaderLength;
	char  responseBody[LWHTTPLIB_MAX_RESPONSE_BODY_LENGHT];
	lwHTTPU32 responseBodyLength;
};

void lwHTTPQueryParsingUtils_Start(struct lwHTTPQueryParsingUtils* utils);

#endif /* LWHTTPSTACK_H_ */
