/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 NetWorkConfig.c
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
#include "NetWorkConfig.h"
#include "comm_fun.h"
#include "ntp_type.h"

/* ȫ��NTP ���������ṹ�� */
NTP_ENVIRONMENT g_Ntp_Enviroment;  
NTPD_STATUS g_ntpd_status;


int g_ntp_socket = 0;
int g_ntp_socket_len = 0;
struct sockaddr_in g_ntp_client;



/**********************************************************************
 * Function:      Send_SocketTo_Ntpd
 * Description:  ���͵�ǰ������Ϣ��ntpd���޸�ntpd ״̬
 * Input:           leap :ʱ��Դ״̬��stratum: ʱ��Դ�ȼ� pRefid:ʱ������
                       len :����,precision:����
 * Return:        
 * Others:        
**********************************************************************/
void Send_SocketTo_Ntpd(Uint8 leap,Uint8 stratum,Uint8 *pRefid,int len,Sint8 precision)
{
    
    g_ntpd_status.leap = leap;
    g_ntpd_status.stratum = stratum;
    g_ntpd_status.precision = precision;
    
    sendto(g_ntp_socket,(char*)&g_ntpd_status,sizeof(NTPD_STATUS),0,(struct sockaddr *)&g_ntp_client,g_ntp_socket_len);
}

/**********************************************************************
 * Function:      Init_Ntpd_Socket
 * Description:   ��ʼ����NTPD ͨ�ŵ�socket�׽���
 * Input:          
 * Return:        
 * Others:        
**********************************************************************/
int Init_Ntpd_Socket()
{
    int sockfd;
    
    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd < 0){
        fprintf(stderr,"create socket error:%s\n",strerror(errno));
        return -errno;
    }
    	
	memset(&g_ntp_client,0,sizeof(g_ntp_client));
	g_ntp_client.sin_family = AF_INET;
	g_ntp_client.sin_port = htons(9988);
	g_ntp_client.sin_addr.s_addr = inet_addr("127.0.0.1");

    g_ntp_socket = sockfd;
    g_ntp_socket_len = sizeof(g_ntp_client);

    g_ntpd_status.leap = LEAP_NOWARNING;
    g_ntpd_status.stratum = 0;
    memcpy(g_ntpd_status.refid,"GNSS",4);
}

