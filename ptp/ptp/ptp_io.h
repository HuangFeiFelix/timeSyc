/********************************************************************************************
*                           ��Ȩ����(C) 2015, ���ſ�ѧ���������о���
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       ptp_io.h
*    ��������:       ptp�������ã���ʼ�������������ļ�������
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-2-6
*    �����б�:
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-2-6
                     ����: HuangFei
                     �޸�����: �´����ļ�


*********************************************************************************************/

#ifndef __PTP_IO_H__
#define __PTP_IO_H__

#include "common.h"
#include "data_type.h"



extern void Init_RecvRdfs(PtpClock *pPtpClock);
extern Uint8 Init_ptpClock(PtpClock *pPtpClock,int index);
extern void Init_Identity(PtpClock *pPtpClock,UInteger16 PortNumber);
extern int Load_NetWork(PtpClock *pPtpClock,NetInfor *infopt);

extern void Display_NetInfo(NetInfor *infopt);
extern void show(char *head,char *data, int len);

#endif
