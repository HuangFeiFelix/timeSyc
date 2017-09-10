/********************************************************************************************
*                           ��Ȩ����(C) 2015, *                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       ptp_proto.h
*    ��������:       ��Ϣ�����߳�ģ����ڣ���ӡ��ʾ����
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-1-30
*    �����б�:
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-1-30
                     ����: HuangFei
                     �޸�����: �´����ļ�


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

