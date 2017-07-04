/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       bmc.h
*    功能描述:       BMC模块，负责最佳主时钟选择
*    作者:           黄飞
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-19
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-19
                     作者: 黄飞
                     修改内容: 新创建文件


*********************************************************************************************/


#ifndef __BMC_H__
#define __BMC_H__

#include "common.h"
#include "data_type.h"


extern void BmcIp(MsgHeader *headerA,MsgAnnounce *announceA,Uint32 *ip,PtpClock *pPtpClock);
extern void BmcMac(MsgHeader *headerA,MsgAnnounce *announceA,char *pMac,PtpClock *pPtpClock);



#endif
