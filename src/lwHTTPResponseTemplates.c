/*
 * lwHTTPResponseTemplates.c
 *
 *  Created on: Jun 6, 2012
 *      Author: snorerax
 */

#include "lwHTTPParse.h"

const char* lwHTTP_Template_BadRequest = "%s 400 Bad Request\r\n";
const char* lwHTTP_Template_NotFound = "%s 404 Not Found\r\n";
const char* lwHTTP_Template_BasicGETResponse = "%s 200 OK\r\n";
const char* lwHTTP_Template_Attr_ContentLength = "Content-Length: %d\r\n";
const char* lwHTTP_Template_Attr_ContentType = "Content-Type: %s\r\n";
const char* lwHTTP_Template_FinalHeaderLine = "\r\n";
const char* lwHTTP_Template_WSReponseHeader = "HTTP/1.1 101 Switching Protocols\r\n";
const char* lwHTTP_Template_WSUpgradeAttrname = "Upgrade";
const char* lwHTTP_Template_WSUpgradeWS = "Upgrade: websocket\r\n";
const char* lwHTTP_Template_WSUpgradeWSComp = "Upgrade: websocket";
const char* lwHTTP_Template_WSConnWS = "Connection: Upgrade\r\n";
const char* lwHTTP_Template_WSAccept = "Sec-WebSocket-Accept: %s\r\n";
const char* lwHTTP_Template_WSProtocolResponse = "Sec-WebSocket-Protocol: %s\r\n";
const char* lwHTTP_Template_WSProtocolResponseAttrName = "Sec-WebSocket-Protocol:";

