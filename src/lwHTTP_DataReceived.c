#include "lwHTTP.h"
#include "lwHTTPUtils.h"
#include "lwHTTPParse.h"
#include "string.h"

#include "lwip/err.h"
#include "lwip/tcp.h"

err_t lwHTTPConnection_DataReceived(
	struct lwHTTPBaseConnection_RcvArgs* args
){

	struct lwHTTPConnection* conn = (struct lwHTTPConnection*)args->arg;
	if (!args->p) {
		if(conn->flags&lwHTTPConn_Flag_IsWS){
			conn->appWSHandler->endConnCB(conn);
		}
		lwHTTPConnection_Close(conn);
		LWHTTPDebug("LWHTTPDebug: Conexion %d finalizada por el extremo remoto\n", conn->connectionNumber);
		return ERR_OK;
	}
	//Datos recibidos
	lwHTTPU8 rcvdChar;
	lwHTTPU16 nBytesAceptados = 0U;
	if(args->p->len > 0U){
		LWHTTPDebug("LWHTTPDebug: Conn %d, %d datos recibidos :)\n\r",conn->connectionNumber, (int)args->p->len);

		lwHTTPU16 nextEnd = conn->rxBufEnd+1U;
		nextEnd&=LWHTTP_CONN_RX_BUFFER_SIZE_MASK;
		while(nextEnd!=conn->rxBufStart){
			rcvdChar = conn->rxBuf[conn->rxBufEnd] = ((lwHTTPU8*)args->p->payload)[nBytesAceptados];
			conn->rxBufEnd = nextEnd;
			nextEnd++;nextEnd&=LWHTTP_CONN_RX_BUFFER_SIZE_MASK;
			nBytesAceptados++;
			LWHTTPDebug("%2.2x", rcvdChar );
			if(nBytesAceptados==args->p->len){
				break;
			}
		}
		LWHTTPDebug("new end %d, new start %d\n\n", conn->rxBufEnd, conn->rxBufStart );
	}
	LWHTTPDebug("LWHTTPDebug: Conn %d tcp_recved %d\n\r", conn->connectionNumber, nBytesAceptados);
	tcp_recved(args->tpcb, nBytesAceptados);
	lwHTTPDefaultGetTime(	&conn->timeoutStartMark	);
	//Examinar los datos recibidos

	return ERR_OK;
}
