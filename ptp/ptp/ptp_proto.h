/********************************************************************************************
*                           版权所有(C) 2015, *                                 版权所有
*********************************************************************************************
*    文 件 名:       ptp_proto.h
*    功能描述:       消息处理，线程模块入口，打印显示函数
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-30
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-30
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/


#ifndef __PTP_PROTO_H__
#define __PTP_PROTO_H__

#include "common.h"

#include "data_type.h"

extern void Process_Msg(PtpClock *pPtpClock,char *buf,int recv_len,void *pAddr);

extern void MasterMode_Send(Uint32 timerIndex,PtpClock *pPtpClock);

extern void SlaveMode_Send(Uint32 timerIndex,PtpClock *pPtpClock);

extern void MasterMode_Recv(PtpClock *pPtpClock);

extern void SlaveMode_Recv(PtpClock *pPtpClock);

extern void updatePtpOffset_ToMainRountine(PtpClock *pPtpClock);

#endif

