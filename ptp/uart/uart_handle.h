#ifndef __UART_HANDLE_H__
#define __UART_HANDLE_H__

#include "common.h"

#include "list.h"
#include "list_data.h"
#include "data_type.h"




extern void *Thread_UartDataHandle(void *arg);

extern void MsgPackAlarmChangeCommand(UartDevice *pUartDevice);

extern void CheckIsRestartNtpd();

extern void msgPackVersionTimeUp(UartDevice *pUartDevice);

extern void msgPackNetOnTimeUp(UartDevice *pUartDevice,Uint16 nCount);

extern void msgPackPtpOnTimeUp(UartDevice *pUartDevice,Uint16 nCount);

extern void msgPackNtpOnTimeUp(UartDevice *pUartDevice,Uint16 nCount);

extern void msgPackAlarmOnTimeUp(UartDevice *pUartDevice,Uint16 nCount);

#endif
