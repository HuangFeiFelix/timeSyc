/********************************************************************************************
*                           ��Ȩ����(C) 2015, *                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       comm_fun.h
*    ��������:       ���ú���
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


#ifndef __COMMFUN_H__
#define __COMMFUN_H__

#include "common.h"
#include "log.h"

extern Uint8 Get_SerialChar();

extern Uint8 Get_SerialString(Uint8 *string,Uint16 str_len);


extern Uint32 Caculate_frequency(Uint32 input);

extern Uint32 Caculate_Second(Uint32 input);

extern Uint32 Get_MessageInterval(Sint8 Interval);

extern Uint8 ValidtateNum(char *input,int len,Uint32 *pNum);
extern Uint8 ValidtateIntervalNum(char *input,int len,short *pNum);

#endif
