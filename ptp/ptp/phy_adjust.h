/******************************************************************************
*	Copyright (c) 2014, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 
*	Purpose:	     
*	Author:		 Felix(HuangFei)
*	Version:	  	 v0.01
*	Compiler:	 arm-linux-gcc
*	Created on:	 2014-11-13
*******************************************************************************
*	Note:
*	Revision history:  
*                       Created by HuangFei    2014-11-13
******************************************************************************/

#ifndef __PHY_ADJUST_H__
#define __PHY_ADJUST_H__

#include "common.h"
#include "data_type.h"

extern void AdustDp83640SynchronizeMaster(PtpClock *pPtpClock,PhyControl *pPhyControl);

extern void GetCnt_threshold(PtpClock *pPtpClock);



extern PhyControl g_phyControl;



#endif
