#include "OSAL.h"
#include "AF.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "ZDProfile.h"
#include <string.h>

#include "Coordinator.h"

#include "DebugTrace.h"
#if !defined(WIN32)
#include "OnBoard.h"
#endif

#include "hal_led.h"
#include "hal_lcd.h"
#include "hal_key.h"
#include "hal_uart.h"
//#include "Sensor.h"
#include "aps_groups.h"
#define  SEND_DATA_EVENT  0x01
const cId_t GenericApp_ClusterList[GENERICAPP_MAX_CLUSTERS]={
GENERICAPP_CLUSTERID  \
 };

const SimpleDescriptionFormat_t GenericApp_SimpleDesc=
{
  GENERICAPP_ENDPOINT,
  GENERICAPP_PROFID,
  GENERICAPP_DEVICEID,
  GENERICAPP_DEVICE_VERSION,
  GENERICAPP_FLAGS,
  0,
  (cId_t *)NULL,
  GENERICAPP_MAX_CLUSTERS,
  (cId_t *)GenericApp_ClusterList,
};
endPointDesc_t GenericApp_epDesc;
byte GenericApp_TaskID;
byte GenericApp_TransID;
devStates_t GenericApp_NwkState;
aps_Group_t GenericApp_Group;

void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pckt);
void GenericApp_SendTheMessage(void);
void ShowInfo(void);
void To_string(uint8 *dest,char *src,uint8 length);
void GenericApp_ProcessZDOMsgs(zdoIncomingMsg_t *inMsg);
typedef struct RFTXBUF
{
	uint8 myNWK[4];
	uint8 myMAC[16];
	uint8 pNWK[4];
	uint8 pMAC[16];
	
}RFTX;

void GenericApp_Init(byte task_id)
{

	halUARTCfg_t uartConfig;

	GenericApp_TaskID		=task_id;
	GenericApp_NwkState	=DEV_INIT;
	GenericApp_TransID		=0;
	GenericApp_epDesc.endPoint=GENERICAPP_ENDPOINT;
	GenericApp_epDesc.task_id=&GenericApp_TaskID;
	GenericApp_epDesc.simpleDesc=(SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
	GenericApp_epDesc.latencyReq=noLatencyReqs;
	afRegister(&GenericApp_epDesc);
	/*GenericApp_Group.ID=0x0001;
	GenericApp_Group.name[0]=6;
	osal_memcpy(&(GenericApp_Group.name[1]),"Group1",6);
	*/
	uartConfig.configured		=TRUE;
	uartConfig.baudRate		=HAL_UART_BR_115200;
	uartConfig.flowControl		=FALSE;
	uartConfig.callBackFunc		=NULL;
	HalUARTOpen(0,&uartConfig);
	ZDO_RegisterForZDOMsg(GenericApp_TaskID,IEEE_addr_rsp);
	
	
}

UINT16 GenericApp_ProcessEvent(byte tadk_id,UINT16 events)
{
	afIncomingMSGPacket_t *MSGpkt;
	if(events&SYS_EVENT_MSG)
		{
			MSGpkt=(afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
			while(MSGpkt)
				{
					switch(MSGpkt->hdr.event)
						{
							case ZDO_CB_MSG:
								GenericApp_ProcessZDOMsgs((zdoIncomingMsg_t * )MSGpkt);
								break;
								
							case AF_INCOMING_MSG_CMD:
								GenericApp_MessageMSGCB(MSGpkt);
								break;
						      	case ZDO_STATE_CHANGE:
									GenericApp_NwkState=(devStates_t)(MSGpkt->hdr.status);
									if(GenericApp_NwkState==DEV_ROUTER)
										{
											//	aps_AddGroup(GENERICAPP_ENDPOINT,&GenericApp_Group);
											osal_set_event(GenericApp_TaskID,SEND_DATA_EVENT);
											
										}
									break;
								
						        default:
									break;
						}
					osal_msg_deallocate((uint8 *) MSGpkt);
					MSGpkt=(afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
				}
			return (events ^SYS_EVENT_MSG);
		
		}
	if(events&SEND_DATA_EVENT)
		{
			ShowInfo();
			ZDP_IEEEAddrReq(0x0000,0,0, 0);
			
			osal_start_timerEx(GenericApp_TaskID, SEND_DATA_EVENT, 5000);
			return (events^SEND_DATA_EVENT);
		}

	return 0;
}
void GenericApp_ProcessZDOMsgs(zdoIncomingMsg_t *inMsg)
{
	char buf[16];
	char changeline[2]={0x0A,0x0D};
	switch(inMsg->clusterID)
		{
			case IEEE_addr_rsp:
					{
						ZDO_NwkIEEEAddrResp_t *pRsp=ZDO_ParseAddrRsp( inMsg);
						if(pRsp)
							{
								if(pRsp->status==ZSuccess)
									{
										To_string(buf,pRsp->extAddr,8);
										HalUARTWrite(0,"Coordinator MAC: ",osal_strlen("Coordinator MAC: "));
										HalUARTWrite(0,buf,16);
										HalUARTWrite(0,changeline,2);
										
									}
								osal_mem_free(pRsp);
							}
							
					}
			break;
					
		}
}
void ShowInfo(void)
{
	RFTX rftx;
	uint16 nwk;
	uint8 buf[8];
	uint8 changeline[2]={0x0A,0x0D};
	nwk=NLME_GetShortAddr();
	To_string(rftx.myNWK,(uint8 *)&nwk,2);
	To_string(rftx.myMAC,NLME_GetExtAddr(),8);
	nwk=NLME_GetCoordShortAddr();
	To_string(rftx.pNWK,(uint8 *)&nwk,2);
	NLME_GetCoordExtAddr(buf);
	To_string(rftx.pMAC,buf,8);
	HalUARTWrite(0,"NWK:",osal_strlen(" NWK:"));
	HalUARTWrite(0,rftx.myNWK,4);
	
	HalUARTWrite(0,"  MAC:",osal_strlen("  MAC:"));
	HalUARTWrite(0,rftx.myMAC,16);
	
	HalUARTWrite(0,"  p-NWK:",osal_strlen("  p-NWk"));
	HalUARTWrite(0,rftx.pNWK,4);
	HalUARTWrite(0,"  p-MAC:",osal_strlen("  p-MAC"));
	HalUARTWrite(0,rftx.pMAC,16);
	
	HalUARTWrite(0,changeline,2);
	
	
	
	
}

void To_string(uint8 * dest, char * src, uint8 length)
{
	uint8 *xad;
	uint8 i=0;
	uint8 ch;
	xad=src+length-1;
	for(i=0;i<length;i++,xad--)
		{
			ch=(*xad>>4)&0x0F;
			dest[i<<1]=ch+((ch<10)?'0':'7');
			ch=*xad&0x0F;
			dest[(i<<1)+1]=ch+((ch<10)?'0':'7');
			
		}
}
void GenericApp_MessageMSGCB(afIncomingMSGPacket_t * pkt)
{
	char buf[18];
	switch(pkt->clusterId)
		{
                case GENERICAPP_CLUSTERID:
				osal_memcpy(buf,pkt->cmd.Data,osal_strlen("Coordinator send!")+1);
				HalUARTWrite(0, buf,osal_strlen("Coordinator send!")+1);
				if(osal_memcmp(buf,"Coordinator send!",osal_strlen("Coordinator send!")+1));
					{
						GenericApp_SendTheMessage();
					}
				break;
		}
}
void GenericApp_SendTheMessage(void)
{
	unsigned char *theMessageData="Router received!";
	afAddrType_t my_DstAddr;
	my_DstAddr.addrMode=(afAddrMode_t)Addr16Bit;
	my_DstAddr.endPoint=GENERICAPP_ENDPOINT;
	my_DstAddr.addr.shortAddr=0x0000;
	AF_DataRequest(&my_DstAddr, &GenericApp_epDesc, GENERICAPP_CLUSTERID, 
					osal_strlen((char *)theMessageData)+1,
					theMessageData,
					&GenericApp_TransID,
					AF_DISCV_ROUTE,
					AF_DEFAULT_RADIUS);
	HalLedSet(HAL_LED_2,HAL_LED_MODE_TOGGLE);

		
}





