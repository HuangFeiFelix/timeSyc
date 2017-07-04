/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 ClientIpFilter.h
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
#ifndef __CLIENT_IP_FILETER_H__
#define __CLIENT_IP_FILETER_H__

#include "common.h"
#include "ntp_type.h"


extern NTP_PARAMETER g_Ntp_Parameter;
extern NtpDevice g_NtpDevice;

extern void *Ip_Filter_State(void);


#endif

