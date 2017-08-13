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

#include "hal_key.h"
#include "hal_uart.h"
#include "OSAL_Nv.h"


#define SEND_TO_ALL_EVENT		0x01

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
  GENERICAPP_MAX_CLUSTERS,
  (cId_t *)GenericApp_ClusterList,
  0,
  (cId_t *)NULL
};


endPointDesc_t GenericApp_epDesc;
devStates_t GenericApp_NwkState;

byte GenericApp_TaskID;
byte GenericApp_TransID;
RFTX nodeinfo[3];
uint8 nodenum=0;
//unsigned char uartbuf[128];

void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pckt);
void GenericApp_SendTheMessage(void);
/*
static void rxCB(uint8 port, uint8 event);
static void rxCB(uint8 port, uint8 event)
{
     unsigned char changeline[2]={0x0A,0x0D};
     uint8 buf[8];
     uint8 i=0;
     HalUARTRead(0,buf,8);
      
    if(osal_memcmp(buf,"topology",8))
      {
        for(i=0;i<3;i++)
        {
          HalUARTWrite(0,nodeinfo[i].type,3);
          HalUARTWrite(0,"  NWK:  ",8);
          HalUARTWrite(0,nodeinfo[i].myNWK,4);
          HalUARTWrite(0,"  pNWK:  ",9);
          HalUARTWrite(0,nodeinfo[i].pNWK,4);
          HalUARTWrite(0,changeline,2);
        }
      }
}*/

void GenericApp_Init(byte task_id)
{
	halUARTCfg_t uartConfig;
	
	GenericApp_TaskID		=task_id;
	GenericApp_TransID		=0;
	GenericApp_epDesc.endPoint=GENERICAPP_ENDPOINT;
	GenericApp_epDesc.task_id=&GenericApp_TaskID;
	GenericApp_epDesc.simpleDesc=(SimpleDescriptionFormat_t *)&GenericApp_SimpleDesc;
	GenericApp_epDesc.latencyReq=noLatencyReqs;
	afRegister(&GenericApp_epDesc);

	uartConfig.configured		=TRUE;
	uartConfig.baudRate		=HAL_UART_BR_115200;
	uartConfig.flowControl		=FALSE;
	uartConfig.callBackFunc		=NULL;
	HalUARTOpen(0,&uartConfig);

	
	
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
							case AF_INCOMING_MSG_CMD:
								GenericApp_MessageMSGCB(MSGpkt);
								break;
							
						        default:
									break;
						}
					osal_msg_deallocate((uint8 *) MSGpkt);
					MSGpkt=(afIncomingMSGPacket_t *)osal_msg_receive(GenericApp_TaskID);
				}
			return (events ^SYS_EVENT_MSG);
		}
		
	return 0;
}
void GenericApp_MessageMSGCB(afIncomingMSGPacket_t * pkt)
{
	RFTX rftx;
        unsigned char changeline[2]={0x0A,0x0D};
        switch(pkt->clusterId)
        {
        case GENERICAPP_CLUSTERID:
          osal_memcpy(&rftx,pkt->cmd.Data,sizeof(rftx));
          HalUARTWrite(0,rftx.databuf,sizeof(rftx));
          HalUARTWrite(0,changeline,2);
          break;
        }
}
void GenericApp_SendTheMessage(void)
{
	/*unsigned char *theMessageData="Coordinator send!";
	afAddrType_t my_DstAddr;
	my_DstAddr.addrMode=(afAddrMode_t)AddrGroup;
	my_DstAddr.endPoint=GENERICAPP_ENDPOINT;
	my_DstAddr.addr.shortAddr=GenericApp_Group.ID;
	AF_DataRequest(&my_DstAddr,
		&GenericApp_epDesc,
		GENERICAPP_CLUSTERID,
		osal_strlen(theMessageData)+1,
		theMessageData,
		&GenericApp_TransID,
		AF_DISCV_ROUTE,
		AF_DEFAULT_RADIUS);*/
}