/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 NetWorkConfig.h
*	Purpose:	     
*	Author:		 Felix(HuangFei)
*	Version:	  	 v01.00
*	Compiler:	 arm-linux-gcc
*	Created on:	 2014-9-23
*******************************************************************************
*	Note:
*	Revision history:  
*                       Created by hf    2014-9-23
******************************************************************************/

#ifndef __NET_WORK_CONFIG_H__
#define __NET_WORK_CONFIG_H__


#include "common.h"
#include "ntp_type.h"
/************************************************************************/
/* 出厂设置的ip gatway mask mac                                         */
/************************************************************************/
#define FACTORY_IP  					"192.168.1.244"
#define FACTORY_GWIP        			"192.168.1.1"
#define FACTORY_MASK    				"255.255.255.0"
#define FACTORY_MAC      				"00.60.6e.11.13.a1"

extern Uint8  VerifyIpAddress(Uint8 * src);
extern Uint8 VerifyNetMastAddress(Uint8 * src);
extern void *Ntp_Manger_State(void);
extern int Init_Ntpd_Socket();
extern void Send_SocketTo_Ntpd(Uint8 leap,Uint8 stratum,Uint8 *pRefid,int len,Sint8 precsion);

extern NET_INFOR g_Net_Infor;
extern NTP_ENVIRONMENT g_Ntp_Enviroment;
extern NTPD_STATUS g_ntpd_status;



#endif
