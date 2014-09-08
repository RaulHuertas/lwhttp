/*
 * lwHTTPws_AppendTX.c
 *
 *  Created on: Jul 1, 2012
 *      Author: snorerax
 */

#include "lwHTTP.h"
#include "lwHTTPws.h"
#include "lwHTTPUtils.h"
#include "string.h"

int lwHTTPCtx_TxData(
	struct lwHTTPConnection* conn,
	const char* data,
	char length,
	char isText
){
	struct lwHTTPwsContext* ctx = &conn->webSocketContext;
	lwHTTPU8 nextEnd = ctx->txMsgEnd+1;
	nextEnd&=LWHTTP_WSMESSAGE_MAX_QUEUE_MASK;
	if(nextEnd==ctx->txMsgStart){
		return -1;//Buffer esta lleno
	}

	lwHTTPU8* const out = &ctx->txMsgs[ctx->txMsgEnd].messageData[0];


	if(isText){
		out[0] = lwHTTP_WSFlags_FIN_FMSK|lwHTTP_WSOPCODE_TEXT;
	}else{
		out[0] = lwHTTP_WSFlags_FIN_FMSK|lwHTTP_WSOPCODE_BINARY;
	}
	out[1] = length;//No masking

	memcpy(&out[2], data, length);
	ctx->txMsgs[ctx->txMsgEnd].length = length+2;
	LWHTTPDebug("WS_TX: Appending index %d, address 0x%x, nuevos indices %d %d\n",
					ctx->txMsgEnd,
					out,
					ctx->txMsgStart,
					nextEnd
	);
	ctx->txMsgEnd = nextEnd;
	nextEnd = ctx->txMsgEnd+1;
	nextEnd&=LWHTTP_WSMESSAGE_MAX_QUEUE_MASK;

	LWHTTPDebug("LWHTTPDebug lwHTTPCtx_TxData: conn->state 0x%x\n",
			conn->state
	);

	if(nextEnd==ctx->txMsgStart){
		return 1;
	}else{
		return 0;
	}

}















