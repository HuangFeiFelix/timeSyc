/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       ptp_io.h
*    功能描述:       ptp参数设置，初始化参数，配置文件操作等
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-2-6
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-2-6
                     作者: HuangFei
                     修改内容: 新创建文件


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
