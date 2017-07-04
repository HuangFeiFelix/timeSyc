/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 PingManager.h
*	Purpose:	     
*	Author:		 Felix(HuangFei)
*	Version:	  	 v01.00
*	Compiler:	 arm-linux-gcc
*	Created on:	 2014-9-22
*******************************************************************************
*	Note:
*	Revision history:  
*                       Created by hf    2014-9-22
******************************************************************************/


#include <stdio.h>
#include <string.h>
#include "PingManager.h"
#include "comm_fun.h"

extern void *Ntp_Manger_State(void);

/**********************************************************************
 * Function:      Ping_Command_3_count
 * Description:   ping 命令，只ping 3次
 * Input:          
 * Return:        
 * Others:        
**********************************************************************/
void Ping_Command_3_count()
{
    Uint8 IpAddress[20];
    Uint8 Command[100];
    Uint8 Ioffset = 0;
    
    memset(Command,0,sizeof(Command));
    memset(IpAddress,0,sizeof(IpAddress));

    printf("PING:");
    
    memcpy(Command,"ping -c 3 ",strlen("ping -c 3 "));
    Ioffset += strlen("ping -c 3 ");

    Get_SerialString(IpAddress,sizeof(IpAddress));
    memcpy(&Command[Ioffset],IpAddress,strlen(IpAddress));
    
    system(Command);

}

/**********************************************************************
 * Function:      Ping_Command
 * Description:   自定义ping命令
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Ping_Command()
{
    Uint8 Command[100];
    memset(Command,0,sizeof(Command));

    printf("//>");
    Get_SerialString(Command,sizeof(Command));

    system(Command);
    
}

/**********************************************************************
 * Function:      Ping_Manager_State
 * Description:   Ping 管理界面
 * Input:         void
 * Return:        void*
 * Others:        
**********************************************************************/
void *Ping_Manager_State(void)
{
    Uint8 Input[20];
    
    Uint8 my_choice;
    
    memset(Input,0,sizeof(Input));
    
    printf("\r\n------------------\r\n");
    printf("PING TEST\r\n");
    printf("------------------\r\n");

    printf("Input your Ping Command\r\n");
    Ping_Command();

    return (void *)Ntp_Manger_State;
}

