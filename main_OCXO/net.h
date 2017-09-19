/********************************************************************************************
*                           版权所有(C) 2015, 
*                                 版权所有
*********************************************************************************************
*    文 件 名:       net.h
*    功能描述:       网口ip mac地址 网关，设置文件描述符参数等
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

#ifndef __NET_H__
#define __NET_H__


extern unsigned char SetMacAddress(char *pInterface,char *pMacAddress);

extern unsigned char SetIpAddress(char *pInterface,unsigned int IpAddress);

extern unsigned char SetMaskAddress(char *pInterface,unsigned int IpMask);

extern unsigned char DelGateWay(char *pInterface,unsigned int DelGateWay);

extern unsigned char AddGateWay(char *pInterface,unsigned int NewGateWay);

extern unsigned char SetGateWay(char *pInterface,unsigned int ip,unsigned int NewGateWay);

extern unsigned char GetMacAddress (char* ifaceName, unsigned char* hwAddr);

extern unsigned char GetIpAddress(char* ifaceName,unsigned int *IpAddress);

extern unsigned char GetMaskAddress(char* ifaceName,unsigned int *MaskAddress);

extern unsigned char GetGateWay(char *pInterface,unsigned int *pGateWay);

extern unsigned char SetSock_reUse(int sockfd);

extern unsigned char SetSock_NoCheck(int sockfd);

extern unsigned char SetMutiCastLoop(int sockfd,int val);

extern unsigned char SetMutiCastTTl(int sockfd,int ttl);

extern unsigned char AddIpAddr_ToMultiCast(int sockfd,unsigned int ip,unsigned int multi_ip,int ttl);

extern unsigned char BindSockDevice(char*ifaceNmae,int sockfd);

extern void DisplayInterfaceStatus(char* ifaceName);


#endif
