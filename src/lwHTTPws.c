#include "lwHTTP.h"
#include "lwHTTPws.h"
#include "lwHTTPUtils.h"
#include "string.h"

#include "lwip/err.h"
#include "lwip/tcp.h"

void lwHTTPwsContext_StartParseAgain(struct lwHTTPwsContext* ctxt){
	ctxt->state = LWHTTPWS_CTXSTATE_WAITING_FIN_OPCODE;
}

void lwHTTPwsContext_Start(struct lwHTTPwsContext* context){
	context->state = LWHTTPWS_CTXSTATE_WAITING_FIN_OPCODE;
	context->flags = 0U;
	context->generalCounter = 0U;
	lwHTTP_WSFrameInit(&context->frameBeingReceived);
	lwHTTP_WSMessageInit(&context->msgRcvd);
	context->txMsgStart = 0U;
	context->txMsgEnd = 0U;
}

void lwHTTP_WSFrameInit(struct lwHTTP_WSFrame* frame){
	frame->payloadPos = 0U;
	frame->payloadLength = 0U;
	frame->flags = 0U;
	frame->len0 = 0U;
	//frame->payloadData[0] = 0U;
}

void lwHTTP_WSMessageInit(struct lwHTTP_WSMessage* msg){
	msg->payloadPos = 0U;
	msg->payloadLength = 0U;
	msg->flags = 0U;
	msg->messageData[0] = 0U;
}

//void lwHTTP_WSMessageInit(struct lwHTTP_WSMessage* msg){
//	msg->payloadPos = 0U;
//	msg->payloadLength = 0U;
//	msg->flags = 0U;
//	msg->messageData[0] = 0U;
//}

void lwHTTPwsParseCharacter(struct lwHTTPConnection* conn, lwHTTPU8 a){
	struct lwHTTPwsContext* ctxt = &conn->webSocketContext;
	struct lwHTTP_WSFrame* frame = &ctxt->frameBeingReceived;
	switch(ctxt->state){
		case LWHTTPWS_CTXSTATE_WAITING_FIN_OPCODE:{
			LWHTTPDebug("WS: OPCODE detected %x\n", a);
			//Leer fin y opcode
			//i9gnorar los bits reservados
			frame->flags = a;
			frame->flags&=(lwHTTP_WSFlags_FIN_FMSK|lwHTTP_WSFlags_OPCODE_FMSK);
			lwHTTPU8 opcode = ctxt->frameBeingReceived.flags&lwHTTP_WSFlags_OPCODE_FMSK;
			if(
				((opcode)==lwHTTP_WSOPCODE_CLOSE)||
				((opcode)==lwHTTP_WSOPCODE_PING)||
				((opcode)==lwHTTP_WSOPCODE_PONG)
			){
				frame->flags|=lwHTTP_WSFlags_CTRL_FRAME_FMSK;
				frame->payloadPos = 0U;
				LWHTTPDebug("WS: recibido frame de control\n");
				//frame->frame.payloadLength = 0U;
			}else if(
				((opcode)==lwHTTP_WSOPCODE_TEXT)||
				((opcode)==lwHTTP_WSOPCODE_BINARY)
			){
				frame->payloadPos = 0U;
				ctxt->msgRcvd.payloadPos = 0U;
				ctxt->msgRcvd.payloadLength = 0U;
			}else if(((opcode)==lwHTTP_WSOPCODE_CONT)){
				//es solo un frame de continuacion
			}else {//codigo de opcode invalido
				//cerrar al conexion de inmediato
				frame->flags|=lwHTTP_WSFlags_ERROR_FRAME_FMSK;
				ctxt->state = LWHTTPWS_CTXSTATE_FINISHED;
			}
			ctxt->state = LWHTTPWS_CTXSTATE_WAITING_LENGTH_SHORT;
			break;
		}
		case LWHTTPWS_CTXSTATE_WAITING_LENGTH_SHORT:{
			//detectar si esta enmascarado
			lwHTTPU8 maskedQ =a&LWHTTP_WSFRAME_MSK_BMSK;
			if(maskedQ){
				ctxt->frameBeingReceived.flags|=lwHTTP_WSFlags_MASK_FMSK;
			}
			//Byte de longitud
			frame->len0 = (a&LWHTTP_WSFRAME_LENGTH_BMSK);
			LWHTTPDebug("WS: len0 detected %x\n", frame->len0);
			switch(ctxt->frameBeingReceived.len0){
				case 127U:{ctxt->state = LWHTTPWS_CTXSTATE_WAITING_LENGTH_LARGE;break;}
				case 126U:{ctxt->state = LWHTTPWS_CTXSTATE_WAITING_LENGTH_MEDIUM;break;}
				default:{
					frame->payloadLength = ctxt->frameBeingReceived.len0;
					if(maskedQ){
						ctxt->state = LWHTTPWS_CTXSTATE_WAITING_MASK;
					}else{
						if(frame->payloadLength==0){
							lwHTTPwsEvaluateFrameEnd(conn);
						}else{
							ctxt->state = LWHTTPWS_CTXSTATE_WAITING_PAYLOAD;
						}
					}
					break;
				}
			}
			frame->payloadPos = 0U;
			ctxt->generalCounter = 0U;
			LWHTTPDebug("WS: WLS frame payload pos: %d\n", (int) frame->payloadPos);
			break;
		}
		case LWHTTPWS_CTXSTATE_WAITING_LENGTH_MEDIUM:{//Se esta leyendo el primer byte
			switch(ctxt->generalCounter){
				case 0:{
					frame->payloadLength=((lwHTTPU16)a)<<8;
					ctxt->generalCounter++;
					break;
				}
				case 1:{
					frame->payloadLength|=((lwHTTPU16)a);
					//Ver si tiene mascara o no
					if(ctxt->frameBeingReceived.flags&lwHTTP_WSFlags_MASK_FMSK){
						ctxt->state = LWHTTPWS_CTXSTATE_WAITING_MASK;
					}else{
						if(frame->payloadLength==0){
							lwHTTPwsEvaluateFrameEnd(conn);
						}else{
							ctxt->state = LWHTTPWS_CTXSTATE_WAITING_PAYLOAD;
						}
					}
					ctxt->generalCounter = 0;
					if(frame->payloadLength>LWHTTP_WSFRAME_MAX_PAYLOAD){
						//frame muy largo
						//cerrar al conexion
						lwHTTPConnection_Close(conn);
					}
					break;
				}
			}
			break;
		}
		case LWHTTPWS_CTXSTATE_WAITING_LENGTH_LARGE:{
			switch(ctxt->generalCounter){
				case 0:{
					frame->payloadLength=((lwHTTPU64)a)<<56;
					ctxt->generalCounter++;
					break;
				}
				case 1:{
					frame->payloadLength|=((lwHTTPU64)a)<<48;
					ctxt->generalCounter++;
					break;
				}
				case 2:{
					frame->payloadLength|=((lwHTTPU64)a)<<40;
					ctxt->generalCounter++;
					break;
				}
				case 3:{
					frame->payloadLength|=((lwHTTPU64)a)<<32;
					ctxt->generalCounter++;
					break;
				}
				case 4:{
					frame->payloadLength|=((lwHTTPU64)a)<<24;
					ctxt->generalCounter++;
					break;
				}
				case 5:{
					frame->payloadLength|=((lwHTTPU64)a)<<16;
					ctxt->generalCounter++;
					break;
				}
				case 6:{
					frame->payloadLength|=((lwHTTPU64)a)<<8;
					ctxt->generalCounter++;
					break;
				}
				case 7:{
					frame->payloadLength|=((lwHTTPU64)a);
					if(ctxt->frameBeingReceived.flags&lwHTTP_WSFlags_MASK_FMSK){
						ctxt->state = LWHTTPWS_CTXSTATE_WAITING_MASK;
					}else{
						if(frame->payloadLength==0){
							lwHTTPwsEvaluateFrameEnd(conn);
						}else{
							ctxt->state = LWHTTPWS_CTXSTATE_WAITING_PAYLOAD;
						}
					}
					ctxt->generalCounter = 0;
					break;
				}
			}
			break;
		}
		case LWHTTPWS_CTXSTATE_WAITING_MASK:{
			frame->maskingKey[ctxt->generalCounter] = a;
			ctxt->generalCounter++;
			if(ctxt->generalCounter==4){
				if(frame->payloadLength==0){
					lwHTTPwsEvaluateFrameEnd(conn);
				}else{
					ctxt->state = LWHTTPWS_CTXSTATE_WAITING_PAYLOAD;
				}
				ctxt->generalCounter = 0;
				LWHTTPDebug("WS: WM frame payload pos: %d\n", (int) frame->payloadPos);
				LWHTTPDebug("WS: length detected %d\n", (int)frame->payloadLength);
				LWHTTPDebug(
					"WS: mask detected %2.2x %2.2x %2.2x %2.2x \n",
					frame->maskingKey[0],
					frame->maskingKey[1],
					frame->maskingKey[2],
					frame->maskingKey[3]
				);
			}
			break;
		}
		case LWHTTPWS_CTXSTATE_WAITING_PAYLOAD:{
			//Ir agregando los datos al buffer de entrada
			//del cuadro
			if(ctxt->flags&lwHTTP_WSFlags_CTRL_FRAME_FMSK){
				ctxt->controlFrameBeingReceived.messageData[ frame->payloadPos ] = a;
			}else{
				ctxt->msgRcvd.messageData[ ctxt->msgRcvd.payloadPos ] = a;
				ctxt->msgRcvd.payloadPos++;
			}
			frame->payloadPos++;
			LWHTTPDebug("WS: WP frame payload pos: %d\n", (int) frame->payloadPos);
			if(frame->payloadLength==frame->payloadPos){//Si ya se ha leido este frame entero
				//El frame se ha leido por completo
				lwHTTPwsEvaluateFrameEnd(conn);
			}
			break;
		}
	}
	//ctxt->frameBeingReceived.payloadPos++;

}

void lwHTTPwsEvaluateFrameEnd(struct lwHTTPConnection* conn){
	struct lwHTTPwsContext* ctxt = &conn->webSocketContext;
	struct lwHTTP_WSFrame* frame = &ctxt->frameBeingReceived;
	if(//frame de control
		ctxt->frameBeingReceived.flags&lwHTTP_WSFlags_CTRL_FRAME_FMSK
	){
		LWHTTPDebug("WS: procediendo a interpretar frame de control\n");
		ctxt->state = LWHTTPWS_CTXSTATE_FINISHED;
	}else if(frame->flags&lwHTTP_WSFlags_FIN_FMSK){//Si es uno final
		lwHTTP_WSMaskData(
			&ctxt->msgRcvd.messageData[ctxt->msgRcvd.payloadLength],
			&ctxt->msgRcvd.messageData[ctxt->msgRcvd.payloadLength],
			&frame->maskingKey[0],
			frame->payloadLength
		);
		ctxt->msgRcvd.payloadLength+=frame->payloadLength;
		ctxt->state = LWHTTPWS_CTXSTATE_FINISHED;
		LWHTTPDebug("WS: procediendo a interpretar mensaje\n");
	}else{//esperar el siguiente frame para completar el mensaje
		LWHTTPDebug("WS: esperadno mas frames para interpretar el mensaje\n");
		lwHTTP_WSMaskData(
			&ctxt->msgRcvd.messageData[ctxt->msgRcvd.payloadLength],
			&ctxt->msgRcvd.messageData[ctxt->msgRcvd.payloadLength],
			&frame->maskingKey[0],
			frame->payloadLength
		);
		ctxt->msgRcvd.payloadLength+=frame->payloadLength;
		ctxt->state = LWHTTPWS_CTXSTATE_WAITING_FIN_OPCODE;
		frame->flags = 0;
	}

}
