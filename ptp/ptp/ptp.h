/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       ptp.h
*    功能描述:       函数入口，线程初始化，环境初始化
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

#ifndef __PTP_H__
#define __PTP_H__

#include "data_type.h"


extern PtpClock g_ptpClock[PTP_PORT_COUNT];
extern UartDevice g_UartDevice;


#endif
