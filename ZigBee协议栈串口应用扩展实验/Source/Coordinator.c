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
byte GenericApp_TaskID;
byte GenericApp_TransID;
unsigned char uartbuf[128];

void GenericApp_MessageMSGCB(afIncomingMSGPacket_t *pckt);
void GenericApp_SendTheMessage(void);
/*static void rxCB(uint8 port, uint8 event);
static void rxCB(uint8 port, uint8 event)
{
	HalUARTRead(0, uartbuf, 16);
	if(osal_memcmp(uartbuf,"www.wlwmaker.com",16))
		{
			HalUARTWrite(0, uartbuf,16);
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
	unsigned char buffer[10];
	switch(pkt->clusterId)
		{
			case GENERICAPP_CLUSTERID:
				osal_memcpy(buffer, pkt->cmd.Data, 10);
                            HalUARTWrite(0,buffer,10);
				break;
		}
}
