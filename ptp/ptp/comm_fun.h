/********************************************************************************************
*                           版权所有(C) 2015, *                                 版权所有
*********************************************************************************************
*    文 件 名:       comm_fun.h
*    功能描述:       公用函数
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
