/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 NtpMain.h
*	Purpose:	     
*	Author:		 Felix(HuangFei)
*	Version:	        v01.00
*	Compiler:	 arm-linux-gcc
*	Created on:	 2014-9-17
*******************************************************************************
*	Note:
*	Revision history:  
*                       Created by hf    2014-9-17
******************************************************************************/


#ifndef __NTP_MAIN_H__
#define __NTP_MAIN_H__


extern int Init_NtpControl();

extern void Ntp_Manage_Configuration(void);
extern void *Ntp_Manger_State(void);
extern void Ntpd_ServerManage(void);
extern void * Mode_Config_State(void);
extern void Factory_Manange();
extern void Sys_Init();
extern void Stop_Ntp_Server();
extern void Start_Ntp_Server();



#endif
