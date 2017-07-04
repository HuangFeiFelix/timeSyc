/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 NtpdConfigOperation.h
*	Purpose:	     
*	Author:		 Felix(HuangFei)
*	Version:	  	 v01.00
*	Compiler:	 arm-linux-gcc
*	Created on:	 2014-10-5
*******************************************************************************
*	Note:
*	Revision history:  
*                       Created by hf    2014-10-5
******************************************************************************/

#ifndef __NTPD_CONFIG_OPERATION_H__
#define __NTPD_CONFIG_OPERATION_H__


#include "common.h"

extern void Load_Ntpd_Config();
extern void Display_All_Config();

extern void Save_Ntp_Conf();
extern void Save_NetParam_ToFile();
#endif
