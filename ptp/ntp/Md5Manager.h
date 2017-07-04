/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	  Md5_Manager_State.h
*	Purpose:	     
*	Author:		 Felix(HuangFei)
*	Version:	  	 v01.00
*	Compiler:	 arm-linux-gcc
*	Created on:	 2014-9-17
*******************************************************************************
*	Note:
*	Revision history:  
*                       Created by hf    2014-9-17
******************************************************************************/
#ifndef __MD_MANAGER_H_
#define __MD_MANAGER_H_

#include "common.h"
#include "ntp_type.h"

#define MD5_ENABLE (0x01)
#define MD5_DISABLE (0x00)

typedef struct 
{
    Uint16 broadcast_current_keyno;/* 指定的广播密钥号 */
    Uint16 multicast_current_keyno;/* 指定的多播密钥号 */
    Uint16 config_md5_flag;	/* MD5 标识 */
    Uint16 config_key_length;/* 密钥长度 */
    
}MD5_CONTROL,*PMD5_CONTROL;


extern void * Md5_Config_State(void);
extern void Set_Md5_ctl(void);
extern void Get_Md5_ctl(void);
extern Uint8 Write_Md5ToKeyFile(MD5_KEY *pKey,Uint16 iCount);
extern Uint8 Read_Md5FromKeyFile(MD5_KEY *pKey);

extern MD5_KEY factory_key[10];

#endif
