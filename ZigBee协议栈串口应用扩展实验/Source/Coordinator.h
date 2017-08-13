#ifndef COORDINATOR_H
#define COORDINATOR_H
#include "ZComDef.h"
#define GENERICAPP_ENDPOINT 10
#define GENERICAPP_PROFID   0x0F04
#define GENERICAPP_DEVICEID 0x0001
#define GENERICAPP_DEVICE_VERSION 0
#define GENERICAPP_FLAGS          0
#define GENERICAPP_MAX_CLUSTERS   1
#define GENERICAPP_CLUSTERID      1

extern void GenericApp_Init(byte task_id);
extern UINT16 GenericApp_ProcessEvent(byte tadk_id,UINT16 events);
#endif