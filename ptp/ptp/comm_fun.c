/********************************************************************************************
*                           版权所有(C) 2015,*                                 版权所有
*********************************************************************************************
*    文 件 名:       comm_fun.c
*    功能描述:       公用函数
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-30
*    函数列表:
                     Caculate_frequency
                     Caculate_Second
                     Display_NetInfo
                     Get_MessageInterval
                     Get_SerialChar
                     Get_SerialString
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-30
                     作者: HuangFei
                     修改内容: 新创建文件

                   2.日期: 2015-2-6
                     作者: HuangFei
                     修改内容: 增加显示函数

*********************************************************************************************/


#include "comm_fun.h"
#include <sys/select.h>



/********************************************************************************************
* 函数名称:    Get_SerialString
* 功能描述:     从标准输入读取1字符串，放入string当中
* 输入参数:    Uint8*string    
               Uint16 str_len  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Uint8 Get_SerialString(Uint8*string,Uint16 str_len)
{
    
    Uint8 input[50];
    int len = 0;

    memset(input,0,sizeof(input));
    
    len = read(0,input,50);
    if(len == 1)
    {
        string[0] = input[0];
        return len;
    }
    if(len >= str_len)
    {
        memcpy(string,input,str_len);
        return str_len;
    }
    else
    {
        memcpy(string,input,len);
        string[len-1] = '\0';

        return len;
    }
}

/********************************************************************************************
* 函数名称:    Get_SerialChar
* 功能描述:    从标准输入读取1个字符
* 输入参数:    无
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Uint8 Get_SerialChar()
{
    Uint8 input[200];
    int len = 0;

    memset(input,0,sizeof(input));
    
    len = read(0,input,200);

    if(len == 1)
    {
        return input[0];
    }
    else if(len == 2)
    {
        if((input[1] == 0x0a) || ( input[1] == 0x0d))
        {
            return input[0];
        }
        else
        {
            return 0xff;
        }
    }
    else
    {
        return 0xff;    
    }
}


/********************************************************************************************
* 函数名称:    Caculate_frequency
* 功能描述:    计算多播、广播的发送周期，按照ntpd格式
* 输入参数:    Uint32 input  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Uint32 Caculate_frequency(Uint32 input)
{
    Uint32 i = 0;
    while(input)
    {
        input = input / 2;
        i++;
    }
    return i -1;
}


/********************************************************************************************
* 函数名称:    Caculate_Second
* 功能描述:     得到广播、多播发送周期秒数
* 输入参数:    Uint32 input  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Uint32 Caculate_Second(Uint32 input)
{
    Uint16 i;
    Uint32 second =1;

    for(i = 0;i<input;i++)
        second = second * 2;

    return second;
}


/********************************************************************************************
* 函数名称:    Get_MessageInterval
* 功能描述:    根据发送的loginterval 获得的发送计数值
* 输入参数:    Uint8 Interval  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Uint32 Get_MessageInterval(Sint8 Interval)
{
    Uint32 synRate = 0;
    
    switch (Interval)
    {
        case 4:
            synRate = 16 * 256;
            break;
        case 3:
            synRate = 8 * 256;
            break;
        case 2:
            synRate = 4 * 256;
            break;
        case 1:
            synRate = 2 * 256;
            break;
        case 0:
            synRate = 1 * 256;
            break;
        case -1:
            synRate = 256/2;
            break;
        case -2:
            synRate = 256/4;
            break;
        case -3:
            synRate = 256/8;
            break;
            
        case -4:
            synRate = 256/16;
            break;
        case -5:
            synRate = 256/32;
            break;
        case -6:
            synRate = 256/64;
            break;  
        case -7:
            synRate = 256/128;
            break;

        case -8:
            synRate = 1;
            break;

        default:
            synRate = 1 * 256;
            break;
        
    }
    return synRate;
}

Uint8 ValidtateNum(char *input,int len,Uint32 *pNum)
{
     
    int i;

    
    if((len == 1) && (input[0] == 0x0a || input[0] == 0x0d))/** 表示输入的是回车,返回2 */
    {
        return 2;
    }
    else if(len == 2)/** 输入0-9 */ 
    {
        if((input[0] > '9') || (input[0] < '0'))
            return FALSE;
        *pNum = input[0] - '0';
    }
    else
    {
        if(input[0] == '0')
        {
            return FALSE;
        }

        for(i = 0;i < len-1;i++)
        {
            if((input[i] > '9') || (input[i] < '0'))
            {   
                return FALSE;
            }
        }
        *pNum = atoi(input);
    }
    
    return TRUE;
}


Uint8 ValidtateIntervalNum(char *input,int len,short *pNum)
{
    int i;

    /** 表示输入的是回车,返回2 */
    if((len == 1) && (input[0] == 0x0a || input[0] == 0x0d))
    {
        return 2;
    }
    else if(len == 2)/** 输入0-9 */ 
    {
        if((input[0] > '9') || (input[0] < '0'))
            return FALSE;
        *pNum = input[0] - '0';
    }
    else
    {
        if(input[0] == '0')
            return FALSE;
        
        if(input[0] == '-')
        {
            for(i = 1;i < len-1;i++)
            {
            
                if((input[i] > '9') || (input[i] < '0'))
                {   
                    return FALSE;
                }
            }
            
            *pNum = atoi(input);

        }
        else
        {
            for(i = 0;i < len-1;i++)
            {
            
                if((input[i] > '9') || (input[i] < '0'))
                {   
                    return FALSE;
                }
            }
            *pNum = atoi(input);
        }
    }

    return TRUE;

}


