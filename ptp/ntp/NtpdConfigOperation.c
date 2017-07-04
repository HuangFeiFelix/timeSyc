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
#include "NtpdConfigOperation.h"
#include "ClientIpFilter.h"
#include "NetWorkConfig.h"
#include "Md5Manager.h"
#include "log.h"

#define CONFIG_FILE_SIZE 4096

/* 限制所有ip访问 */
#define RESTRICT_ALL "restrict default nomodify notrap noquery ignore"
#define RESTRICT_IP6 "restrict -6 default nomodify notrap nopeer noquery ignore"

/* 默认打开所有ip访问 */
#define DEFAULT_ALL "restrict default nomodify notrap"
#define DEFAULT_IP6 "restrict -6 default nomodify notrap nopeer"

/* 本地时钟源 16s轮询 */
#define SERVER_ADDR "server 127.127.1.0 maxpoll 4 minpoll 4"
#define fUDE_ADDR "fudge 127.127.1.0 stratum"

/* ID:GPS */
#define REFID_GPS "refid GNSS"
#define REFID_REF "refid REF"
#define REFID_LOC "refid LOC"


/* ID:LOCL */
#define REFID_LOCL "refid LOCL"

/* 黑名单控制 */
#define BLACK_LIST_TAIL "notrap nomodify noquery ignore"

/* 白名单控制 */
#define WHITLE_LIST_TAIL "notrap nomodify"

/* 随机密钥 */
#define AUTO_KEY "autokey"

extern char g_ntp_enable[8];

/**********************************************************************
 * Function:      Init_Keys
 * Description:   读取keys文件，分析该文件初始化
 *				  g_Ntp_Enviroment.current_key数组变量
 * Input:         void 
 * Return:        void
 * Others:        
**********************************************************************/
Int8 Init_Keys()
{
    Uint16 i;
    Uint8 Data[50];
    Uint8 Keys[50];
    int   Num;
    Uint8 Type;
    Uint8 TrustKey[10];
    Uint8 *pChoice;
    
    
    FILE *md5_file_fd = fopen(NTP_MD5_FILE,"r"); 
    if(md5_file_fd == NULL)
    {
        printf("can not find md5 file\n");
        return -1;
    }

    i = 1;
    while(fgets(Data,sizeof(Data),md5_file_fd))
    {
        memset(Keys,0,sizeof(Keys));
        
        sscanf(Data,"%d %c %s",&Num,&Type,Keys);
        memcpy(g_Ntp_Enviroment.current_key[i].key,Keys,strlen(Keys));
        g_Ntp_Enviroment.current_key[i].key_length = strlen(Keys);
        
        memset(Data,0,sizeof(Data));
        i++;
    }
    fclose(md5_file_fd);

}

/**********************************************************************
 * Function:      Load_BroadCast
 * Description:   分析广播配置信息
 * Input:         pData[200][200]：数组地址,index：偏移位置
 * Return:        void
 * Others:        
**********************************************************************/
void Load_BroadCast(Uint8 pData[200][200],int index)
{
    Uint16 iLine;
    Uint16 i = 0;

    Uint8  title[50];
    Uint8  ip[50];
    Uint8  key[10];
    Uint8  poll[10];

    Uint32 poll_num;
    int key_num;
    
    iLine = index+1;
    if(strcmp(pData[iLine],"#end") == 0)
    {
        printf("Broadcast close\n");
        g_Ntp_Parameter.broadcast = FALSE;
        return;
    }

    g_Ntp_Parameter.broadcast = TRUE;
    
    memset(title,0,sizeof(title));
    memset(ip,0,sizeof(ip));
    memset(key,0,sizeof(key));
    memset(poll,0,sizeof(poll));
    
    if(strstr(pData[iLine],"autokey") != NULL)
    {
        sscanf(pData[iLine],"%s %s %s %s %d",title,ip,key,poll,&poll_num);
        key_num = 0;
    }
    else
    {
        sscanf(pData[iLine],"%s %s %s %d %s %d",title,ip,key,&key_num,poll,&poll_num);
    }
    
    g_Ntp_Parameter.broadcast_key_num = key_num;
    g_Ntp_Parameter.freq_b = Caculate_Second(poll_num);
    
    PLOG("%s %s %s %d %s %d\n",title,ip,key,key_num,poll,poll_num);
    iLine++;
    
}

/**********************************************************************
 * Function:      Load_MultiCast
 * Description:   分析多播配置信息
 * Input:         pData[200][200]：数组地址,index：偏移位置
 * Return:        void
 * Others:        
**********************************************************************/
void Load_MultiCast(Uint8 pData[200][200],int index)
{
    Uint16 iLine;
    Uint16 i = 0;
    
    Uint8  title[50];
    Uint8  ip[50];
    Uint8  key[10];
    Uint8  poll[10];

    Uint32 poll_num;
    int    key_num;
    
    iLine = index+1;
    if(strcmp(pData[iLine],"#end") == 0)
    {
        printf("Multicast close\n");
        g_Ntp_Parameter.multicast = FALSE;
        return;
    }
    
    g_Ntp_Parameter.multicast = TRUE;
    
    memset(title,0,sizeof(title));
    memset(ip,0,sizeof(ip));
    memset(key,0,sizeof(key));
    memset(poll,0,sizeof(poll));

    if(strstr(pData[iLine],"autokey") != NULL)
    {
        sscanf(pData[iLine],"%s %s %s %s %d",title,ip,key,poll,&poll_num);
        key_num = 0;
    }
    else
    {
        sscanf(pData[iLine],"%s %s %s %d %s %d",title,ip,key,&key_num,poll,&poll_num);
    }
    
    g_Ntp_Parameter.multicast_key_num = key_num;
    g_Ntp_Parameter.freq_m = Caculate_Second(poll_num);
        
    PLOG("%s %s %s %d %s %d\n",title,ip,key,key_num,poll,poll_num);
    
    iLine++;

}

/**********************************************************************
 * Function:      Load_BlackList
 * Description:   分析黑名单配置信息
 * Input:         pData[200][200]：数组地址,index：偏移位置
 * Return:        void
 * Others:        
**********************************************************************/
void Load_BlackList(Uint8 pData[200][200],int index)
{
    Uint16 iLine;
    Uint16 i = 0;

    Uint8  title[50];
    Uint8  ip[50];
    Uint8  mask_str[10];
    Uint8  mask[50];
    Uint8  other[50];
    Uint8  flag;
    
    iLine = index+1;
    if(strcmp(pData[iLine],"#open") == 0)
    {
        PLOG("Blacklist open\n");
        g_Ntp_Parameter.blacklist = TRUE;
    }
    else if(strcmp(pData[iLine],"#close") == 0)
    {
        PLOG("Blacklist close\n");
        g_Ntp_Parameter.blacklist = FALSE;
    }
    
    iLine = iLine+1;
    if(strcmp(pData[iLine],"#end") == 0)
    {
        PLOG("load blacklist NULL\n");
        return;
    }
    
    while(strcmp(pData[iLine+i],"#end") != 0)
    {
        memset(title,0,sizeof(title));
        memset(ip,0,sizeof(ip));
        memset(mask_str,0,sizeof(mask_str));
        memset(mask,0,sizeof(mask));
        memset(other,0,sizeof(other));
        if(g_Ntp_Parameter.blacklist == TRUE)
        {
            
            sscanf(pData[iLine+i],"%s %s %s %s %s",title,ip,mask_str,mask,other);
        }

        else
        {
            sscanf(pData[iLine+i],"%c %s %s %s %s %s",&flag,title,ip,mask_str,mask,other);
        }
        
        g_Ntp_Parameter.blacklist_ip[i] = inet_addr(ip);
        g_Ntp_Parameter.blacklist_mask[i] = inet_addr(mask);
        g_Ntp_Parameter.blacklist_flag[i] = TRUE;
        
        i++;
    }
    
}

/**********************************************************************
 * Function:      Load_WhiltList
 * Description:   分析白名单配置信息
 * Input:         pData[200][200]：数组地址,index：偏移位置
 * Return:        void
 * Others:        
**********************************************************************/
void Load_WhiltList(Uint8 pData[200][200],int index)
{
    Uint16 iLine;
    Uint16 i = 0;

    Uint8  title[50];
    Uint8  ip[50];
    Uint8  mask_str[10];
    Uint8  mask[50];
    Uint8  other[50];
    Uint8  flag;
    
    iLine = index+1;
    if(strcmp(pData[iLine],"#open") == 0)
    {
        PLOG("Whitlelist open\n");
        g_Ntp_Parameter.whitelist = TRUE;
    }
    else if(strcmp(pData[iLine],"#close") == 0)
    {
        PLOG("Whitlelist close\n");
        g_Ntp_Parameter.whitelist = FALSE;
    }

    iLine = iLine+1;
    if(strcmp(pData[iLine],"#end") == 0)
    {
        PLOG("load whithlist NULL\n");
        return;
    }
    
    while(strcmp(pData[iLine+i],"#end") != 0)
    {
        
        memset(title,0,sizeof(title));
        memset(ip,0,sizeof(ip));
        memset(mask_str,0,sizeof(mask_str));
        memset(mask,0,sizeof(mask));
        memset(other,0,sizeof(other));
        
        if(g_Ntp_Parameter.whitelist == TRUE)
        {
            sscanf(pData[iLine+i],"%s %s %s %s %s",title,ip,mask_str,mask,other);
        }
        else
        {
            sscanf(pData[iLine+i],"%c %s %s %s %s %s",&flag,title,ip,mask_str,mask,other);
        }
        
        g_Ntp_Parameter.whitelist_ip[i] = inet_addr(ip);
        g_Ntp_Parameter.whitelist_mask[i] = inet_addr(mask);
        g_Ntp_Parameter.whitelist_flag[i] = TRUE;
        
        i++;
    }

}

/**********************************************************************
 * Function:      Load_ServerAddr
 * Description:   分析服务源配置信息
 * Input:         pData[200][200]：数组地址,index：偏移位置
 * Return:        void
 * Others:        
**********************************************************************/
static void Load_ServerAddr(Uint8 pData[200][200],int index)
{
    Uint16 iLine;
    Uint16 i = 0;

    Uint8  title[50];
    Uint8  stratum;
    Uint8  refid[50];


    iLine = index+1;
    if(strcmp(pData[iLine],"#end") == 0)
    {
        PLOG("Server close\n");
        return;
    }
    //g_Ntp_Parameter.sympassive = TRUE;

}

/**********************************************************************
 * Function:      Load_Keys
 * Description:   分析MD5密钥配置信息
 * Input:         pData[200][200]：数组地址,index：偏移位置
 * Return:        void
 * Others:        
**********************************************************************/
void Load_Keys(Uint8 pData[200][200],int index)
{
    Uint16 iLine;
    Uint16 i = 0;
    
    Uint8  title[50];
    Uint8  ip[50];
    Uint8  key[10];
    int    key_num;

    Uint8 TrustKey[20];
    Uint8 *pChoice;
    
    iLine = index+1;
    Init_Keys();
    
    if(strcmp(pData[iLine],"#end") == 0)
    {
        printf("key do not open\n");
        g_Ntp_Enviroment.md5_flag = g_Ntp_Enviroment.md5_flag & MD5_DISABLE;
        return;
    }
    g_Ntp_Enviroment.md5_flag = g_Ntp_Enviroment.md5_flag | MD5_ENABLE;
    
    for(i = 0;i < 5; i++)
    {
        memset(TrustKey,0,sizeof(TrustKey));
        memcpy(TrustKey,pData[i+iLine],10);

        if(strcmp("trustedkey",TrustKey) == 0)
        {
            strtok(pData[i+iLine]," ");
            while(pChoice = strtok(NULL," "))
           {
                PLOG("find trust key %d\n",*pChoice);
                g_Ntp_Enviroment.current_key[*pChoice-0x30].key_valid = TRUE;
           }
        }
    }
    
}

void Load_EthxEnable(Uint8 pData[200][200],int index)
{
    Uint16 iLine;
    Uint16 i = 0;

    Uint8  title[50];
    Uint8  portNum;
    Uint8  enable;

    iLine = index+1;
    while(strcmp(pData[iLine+i],"#end") != 0)
    {
        
        memset(title,0,sizeof(title));
        
        sscanf(pData[iLine],"%s %c %c",title,&portNum,&enable);
        g_ntp_enable[portNum - 30] = enable- '0';
        
        i++;

    }

}

/**********************************************************************
 * Function:      Analyse_NtpdConfig
 * Description:   将读取的ntp.conf配置信息进行分析
 * Input:         pBuf:配置信息的存储地址 
 * Return:        
 * Others:        
**********************************************************************/
void Analyse_NtpdConfig(Uint8* pBuf)
{
    Uint8 *pLine;
    Uint8  Data[200][200];
    int i = 0;

    memset(Data,0,sizeof(Data));
    
    strtok(pBuf,"\r\n");
    while(pLine = strtok(NULL,"\r\n"))
    {   
        memcpy(Data[i],pLine,strlen(pLine));
        i++;
    }
    
    for(i = 0;i < 200;i++)
    {

        if(strcmp(Data[i],"#blacklist") == 0)
        {
            
            Load_BlackList(Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#whiltlist") == 0)
        {
            Load_WhiltList(Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#broadcast") == 0)
        {
            Load_BroadCast(Data,i);
        }
    }

    
    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#multicast") == 0)
        {
            Load_MultiCast(Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#server") == 0)
        {
            Load_ServerAddr(Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#keys") == 0)
        {
            Load_Keys(Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#ethxEnable") == 0)
        {
            Load_EthxEnable(Data,i);
        }
    }

}

/**********************************************************************
 * Function:      Load_Ntpd_Config
 * Description:   从ntp.conf读取内容到ConfigFileBuf缓存中，
 *			      从keys读取内容到g_Ntp_Enviroment.current_key中。
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Load_Ntpd_Config()
{

    Uint16 i;
    Uint8 Data[50];
    Uint8 Keys[50];

    Uint8 ConfigFileBuf[CONFIG_FILE_SIZE];
    int charactor;
    
    FILE *ntp_cfg_fp = fopen(NTP_CONFIG_FILE,"r");
    if(ntp_cfg_fp == NULL)
    {
        printf("can not find config file\n");
        return;
    }
    
    i = 0;
    memset(ConfigFileBuf,0,CONFIG_FILE_SIZE);
    while( (charactor = fgetc(ntp_cfg_fp))!= EOF)
    {
        ConfigFileBuf[i] = charactor;
        i++;
    }

    Read_Md5FromKeyFile(g_Ntp_Enviroment.current_key);
    
    Analyse_NtpdConfig(ConfigFileBuf);

    fclose(ntp_cfg_fp);
}


/**********************************************************************
 * Function:      Display_All_Config
 * Description:   打印所有配置信息
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Display_All_Config()
{
#if 0
    printf("------------------\r\n");
    printf("NETWORK CONFIGURATION\r\n");
    printf("------------------\r\n");

    Display_Netinfo(&g_Ntp_Enviroment.runconf);
#endif

    /*MD5 CONFIG*/
    printf("------------------\r\n");
	printf("MD5 CONFIGURATION\r\n");
	printf("------------------\r\n");
    printf("MD5 authentication ");    
    if((g_Ntp_Enviroment.md5_flag & MD5_ENABLE) == MD5_ENABLE)
    {
        printf("[Enable]\r\n");
    }
    else
    {
        printf("[Disable]\r\n");
    }
    
    if((g_Ntp_Parameter.multicast_key_num == 0) && (g_Ntp_Parameter.broadcast_key_num == 0))
    {
        printf("Multicast key num [Random],Broadcast key num [Random]\r\n");

    }
    else if((g_Ntp_Parameter.multicast_key_num >= 1) && (g_Ntp_Parameter.broadcast_key_num == 0))
    {
        printf("Multicast key num [%d],Broadcast key num [Random]\r\n"
           ,g_Ntp_Parameter.multicast_key_num);

    }
    else if((g_Ntp_Parameter.multicast_key_num == 0) && (g_Ntp_Parameter.broadcast_key_num >= 1))
    {
        printf("Multicast key num [Random],Broadcast key num [%d]\r\n"
           ,g_Ntp_Parameter.broadcast_key_num);
    }
    else
    {
        printf("Multicast key num [%d],Broadcast key num [%d]\r\n"
           ,g_Ntp_Parameter.multicast_key_num
           ,g_Ntp_Parameter.broadcast_key_num);
    }

    /*SERVER MODE CONFIG*/
    printf("\r\n-------------------\r\n");
    printf("SERVER MODE CONFIGURATION\r\n");
    printf("-------------------\r\n");

    printf("Broadcast ");
    if(g_Ntp_Parameter.broadcast == FALSE)
        printf("[Off]\r\n");
    else
        printf("[On] Send period:%d(s)\r\n",g_Ntp_Parameter.freq_b);
    
    printf("Multicast ");
    if(g_Ntp_Parameter.multicast == FALSE)
        printf("[Off]\r\n");
    else
        printf("[On]Send period:%d(s)\r\n",g_Ntp_Parameter.freq_m);

#if 0
    printf("Symmetric passive ");
    if(g_Ntp_Parameter.sympassive == FALSE)
        printf("[Off]\r\n");
    else
        printf("[On]\r\n");
#endif

    /*CLIENT IP Filter*/
    printf("\r\n-------------------\r\n");
    printf("CLIENT IP Filter\r\n");
    printf("-------------------\r\n");
    printf("Mode--");
    if((g_Ntp_Parameter.blacklist==1) && (g_Ntp_Parameter.whitelist== 0))
        printf("Blacklist enabled!!\r\n");
    else if((g_Ntp_Parameter.blacklist==0) && (g_Ntp_Parameter.whitelist== 1))
        printf("Whitelist enabled!!\r\n");
    else if((g_Ntp_Parameter.blacklist==1) && (g_Ntp_Parameter.whitelist== 1))
        printf("Both lists enabled!!\r\n");
    else if((g_Ntp_Parameter.blacklist==0) && (g_Ntp_Parameter.whitelist== 0))
        printf("Both lists disabled!!\r\n");
    else 
        printf("Mode Error !!\r\n");

}

/**********************************************************************
 * Function:      Save_BlackList_Open
 * Description:  黑名单打开时候 保存黑名单配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_BlackList_Open(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    
    struct sockaddr_in temsock;
    
    Uint8 line_str[200];
    Uint8 ip[20];
    Uint8 mask[20];

    memcpy(Buf + iOffset,"#blacklist",strlen("#blacklist"));
    iOffset += strlen("#blacklist");
    Buf[iOffset++] = ENTER_CHAR;
    
    memcpy(Buf + iOffset,"#open",strlen("#open"));
    iOffset += strlen("#open");
    Buf[iOffset++] = ENTER_CHAR;


    for(i = 0;i < 16;i++)
    {
        if(g_Ntp_Parameter.blacklist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = g_Ntp_Parameter.blacklist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = g_Ntp_Parameter.blacklist_mask[i];
            memcpy(mask,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));
 
            sprintf(line_str,"%s %s %s %s %s","restrict",ip
                ,"mask",mask,BLACK_LIST_TAIL);
            
            memcpy(Buf + iOffset ,line_str,strlen(line_str));
            iOffset += strlen(line_str);

            Buf[iOffset++] = ENTER_CHAR;

        }
    }
    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;
}

/**********************************************************************
 * Function:      Save_BlackList_Close
 * Description:  黑名单关闭的时候 保存黑名单配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_BlackList_Close(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    
    struct sockaddr_in temsock;
    
    Uint8 line_str[200];
    Uint8 ip[20];
    Uint8 mask[20];

    memcpy(Buf + iOffset,"#blacklist",strlen("#blacklist"));
    iOffset += strlen("#blacklist");
    Buf[iOffset++] = ENTER_CHAR;
    
    memcpy(Buf + iOffset,"#close",strlen("#close"));
    iOffset += strlen("#close");
    Buf[iOffset++] = ENTER_CHAR;


    for(i = 0;i < 16;i++)
    {
        if(g_Ntp_Parameter.blacklist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = g_Ntp_Parameter.blacklist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = g_Ntp_Parameter.blacklist_mask[i];
            memcpy(mask,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));
 
            sprintf(line_str,"# %s %s %s %s %s","restrict",ip
                ,"mask",mask,BLACK_LIST_TAIL);
            
            memcpy(Buf + iOffset ,line_str,strlen(line_str));
            iOffset += strlen(line_str);

            Buf[iOffset++] = ENTER_CHAR;

        }
    }
    
    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;
}


/**********************************************************************
 * Function:      Save_BlackList
 * Description:   保存黑名单配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_BlackList(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    
    struct sockaddr_in temsock;
    
    Uint8 line_str[200];
    Uint8 ip[20];
    Uint8 mask[20];


    for(i = 0;i < 16;i++)
    {
        if(g_Ntp_Parameter.blacklist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = g_Ntp_Parameter.blacklist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = g_Ntp_Parameter.blacklist_mask[i];
            memcpy(mask,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));
 
            sprintf(line_str,"%s %s %s %s %s","restrict",ip
                ,"mask",mask,BLACK_LIST_TAIL);
            
            memcpy(Buf + iOffset ,line_str,strlen(line_str));
            iOffset += strlen(line_str);

            Buf[iOffset++] = ENTER_CHAR;

        }
    }
    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;
}

/**********************************************************************
 * Function:      Save_WhiltList_Open
 * Description:  白名单打开时候 保存白名单配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_WhiltList_Open(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    
    struct sockaddr_in temsock;
    
    Uint8 line_str[200];
    Uint8 ip[20];
    Uint8 mask[20];

    memcpy(Buf + iOffset,"#whiltlist",strlen("#whiltlist"));
    iOffset += strlen("#whiltlist");
    Buf[iOffset++] = ENTER_CHAR;
    
    memcpy(Buf + iOffset,"#open",strlen("#open"));
    iOffset += strlen("#open");
    Buf[iOffset++] = ENTER_CHAR;

    for(i = 0;i < 16;i++)
    {
        if(g_Ntp_Parameter.whitelist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = g_Ntp_Parameter.whitelist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = g_Ntp_Parameter.whitelist_mask[i];
            memcpy(mask,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));
 
            sprintf(line_str,"%s %s %s %s %s","restrict",ip
                ,"mask",mask,WHITLE_LIST_TAIL);

            memcpy(Buf + iOffset ,line_str,strlen(line_str));
            iOffset += strlen(line_str);

            Buf[iOffset++] = ENTER_CHAR;

        }
    }

    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;

}

/**********************************************************************
 * Function:      Save_WhiltList_Close
 * Description:  白名单关闭时候 保存白名单配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_WhiltList_Close(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    
    struct sockaddr_in temsock;
    
    Uint8 line_str[200];
    Uint8 ip[20];
    Uint8 mask[20];

    memcpy(Buf + iOffset,"#whiltlist",strlen("#whiltlist"));
    iOffset += strlen("#whiltlist");
    Buf[iOffset++] = ENTER_CHAR;
    
    memcpy(Buf + iOffset,"#close",strlen("#close"));
    iOffset += strlen("#close");
    Buf[iOffset++] = ENTER_CHAR;

    
    for(i = 0;i < 16;i++)
    {
        if(g_Ntp_Parameter.whitelist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = g_Ntp_Parameter.whitelist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = g_Ntp_Parameter.whitelist_mask[i];
            memcpy(mask,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));
 
            sprintf(line_str,"# %s %s %s %s %s","restrict",ip
                ,"mask",mask,WHITLE_LIST_TAIL);

            memcpy(Buf + iOffset ,line_str,strlen(line_str));
            iOffset += strlen(line_str);

            Buf[iOffset++] = ENTER_CHAR;

        }
    }

    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;

}

/**********************************************************************
 * Function:      Save_WhiltList
 * Description:   保存白名单配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_WhiltList(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    
    struct sockaddr_in temsock;
    
    Uint8 line_str[200];
    Uint8 ip[20];
    Uint8 mask[20];
    
    for(i = 0;i < 16;i++)
    {
        if(g_Ntp_Parameter.whitelist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = g_Ntp_Parameter.whitelist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = g_Ntp_Parameter.whitelist_mask[i];
            memcpy(mask,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));
 
            sprintf(line_str,"%s %s %s %s %s","restrict",ip
                ,"mask",mask,WHITLE_LIST_TAIL);

            memcpy(Buf + iOffset ,line_str,strlen(line_str));
            iOffset += strlen(line_str);

            Buf[iOffset++] = ENTER_CHAR;

        }
    }

    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;

}




/**********************************************************************
 * Function:      Save_BroadCast
 * Description:   保存广播配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_BroadCast(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    Uint8 line_str[200];
    struct sockaddr_in temsock;
    Uint32 iMask;
    Uint32 broadcast_ip;
    Uint32 freq = Caculate_frequency(g_Ntp_Parameter.freq_b);
        
    memset(line_str,0,sizeof(line_str));

    /*计算广播地址*/
    iMask = ~g_Ntp_Enviroment.runconf.mask;
    broadcast_ip = g_Ntp_Enviroment.runconf.ip | iMask;
    temsock.sin_addr.s_addr = broadcast_ip;

    if(g_Ntp_Parameter.broadcast_key_num == 0)
    {
        sprintf(line_str,"%s %s %s %s %d","broadcast",inet_ntoa(temsock.sin_addr),"autokey","minpoll",freq);
    }
    else
    {
        sprintf(line_str,"%s %s %s %d %s %d","broadcast",inet_ntoa(temsock.sin_addr),"key",g_Ntp_Parameter.broadcast_key_num,"minpoll",freq);
    }
    

    memcpy(Buf + iOffset ,line_str,strlen(line_str));
    iOffset += strlen(line_str);
    Buf[iOffset++] = ENTER_CHAR;
     
    memcpy(Buf + iOffset,"broadcastclient",strlen("broadcastclient"));
    iOffset += strlen("broadcastclient");
    Buf[iOffset++] = ENTER_CHAR;
    
    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;
}


/**********************************************************************
 * Function:      Save_MultiCast
 * Description:   保存多播配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_MultiCast(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    Uint8 line_str[200];
    struct sockaddr_in temsock;
    Uint32 freq = Caculate_frequency(g_Ntp_Parameter.freq_m);
    
    memset(line_str,0,sizeof(line_str));

    if(g_Ntp_Parameter.multicast_key_num == 0)
    {
        sprintf(line_str,"%s %s %s %s %d","broadcast","224.0.1.1","autokey","minpoll",freq);
    }
    else
    {
        sprintf(line_str,"%s %s %s %d %s %d","broadcast","224.0.1.1","key",g_Ntp_Parameter.multicast_key_num,"minpoll",freq);
    }
    
    memcpy(Buf + iOffset ,line_str,strlen(line_str));
    iOffset += strlen(line_str);
    Buf[iOffset++] = ENTER_CHAR;

    memcpy(Buf + iOffset,"multicastclient 224.0.1.1",strlen("multicastclient 224.0.1.1"));
    iOffset += strlen("multicastclient 224.0.1.1");
    Buf[iOffset++] = ENTER_CHAR;

    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;

}



/**********************************************************************
 * Function:      Save_ServerAddr
 * Description:   保存服务源配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_ServerAddr(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    Uint8 line_str[200];
    struct sockaddr_in temsock;

    memset(line_str,0,sizeof(line_str));

    sprintf(line_str,"%s %d %s",fUDE_ADDR,0,REFID_LOC);
    
    memcpy(Buf + iOffset ,line_str,strlen(line_str));
    iOffset += strlen(line_str);
    Buf[iOffset++] = ENTER_CHAR;
    
    memcpy(Buf + iOffset,SERVER_ADDR,strlen(SERVER_ADDR));
    iOffset += strlen(SERVER_ADDR);
    Buf[iOffset++] = ENTER_CHAR;

    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    Buf[iOffset++] = ENTER_CHAR;
    
    *offset = iOffset;

}


/**********************************************************************
 * Function:      Save_keys
 * Description:   保存密钥配置
 * Input:         Buf：写入缓存地址,offset：写入缓存偏移量
 * Return:        void
 * Others:        
**********************************************************************/
static void Save_keys(Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    Uint8 line_str[50];
    struct sockaddr_in temsock;
    
    memcpy(Buf + iOffset,"enable auth",strlen("enable auth"));
    iOffset += strlen("enable auth");
    Buf[iOffset++] = ENTER_CHAR;

    memcpy(Buf + iOffset,"keys    /etc/ntp/keys",strlen("keys    /etc/ntp/keys"));
    iOffset += strlen("keys    /etc/ntp/keys");
    Buf[iOffset++] = ENTER_CHAR;

    memcpy(Buf + iOffset,"trustedkey",strlen("trustedkey"));
    iOffset += strlen("trustedkey");

    
    for(i = 1;i <= TOTAL_KEY_NO;i++)
    {
        if(g_Ntp_Enviroment.current_key[i].key_valid)
        {
            Buf[iOffset++] = ' ';
            Buf[iOffset++] = i + '0';
            
        }
        
    }
    Buf[iOffset++] = ENTER_CHAR;
    
    memcpy(Buf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    Buf[iOffset++] = ENTER_CHAR;
    *offset = iOffset;

}

/**********************************************************************
 * Function:      Save_Ntp_Conf
 * Description:   保存NTP 参数配置
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Save_Ntp_Conf()
{
    Uint16 i;
    Uint16 iOffset;
    Uint8 Data[50];
    Uint8 Keys[50];
    Uint8 ConfigFileBuf[CONFIG_FILE_SIZE];

    FILE *ntp_cfg_fp = fopen(NTP_CONFIG_FILE,"w+");
    if(ntp_cfg_fp == NULL)
    {
        printf("can not find config file\n");
        return;
    }

    memset(ConfigFileBuf,0,CONFIG_FILE_SIZE);
    
    iOffset = 0;

    /*写入限制*/
    if((g_Ntp_Parameter.whitelist == TRUE))
    {
        memcpy(ConfigFileBuf + iOffset,"#restrict",strlen("#restrict"));
        iOffset += strlen("#restrict");
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
        /*写禁止所有ip*/
        memcpy(ConfigFileBuf + iOffset,RESTRICT_ALL,strlen(RESTRICT_ALL));
        iOffset += strlen(RESTRICT_ALL);
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
        
        /*写禁止ip 6*/
        memcpy(ConfigFileBuf + iOffset,RESTRICT_IP6,strlen(RESTRICT_IP6));
        iOffset += strlen(RESTRICT_IP6);
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
        
        memcpy(ConfigFileBuf + iOffset,"#end",strlen("#end"));
        iOffset += strlen("#end");
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
        ConfigFileBuf[iOffset++] = ENTER_CHAR;

    }
    else
    {
        memcpy(ConfigFileBuf + iOffset,"#restrict",strlen("#restrict"));
        iOffset += strlen("#restrict");
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
       
       memcpy(ConfigFileBuf + iOffset,"#end",strlen("#end"));
       iOffset += strlen("#end");
       ConfigFileBuf[iOffset++] = ENTER_CHAR;
       ConfigFileBuf[iOffset++] = ENTER_CHAR;

    }

    /*写入黑名单*/
    if(g_Ntp_Parameter.blacklist == TRUE)
    {
        Save_BlackList_Open(ConfigFileBuf,&iOffset);
    }
    else
    {
        Save_BlackList_Close(ConfigFileBuf,&iOffset);
    }
    
    
    /*写入白名单*/  
    if(g_Ntp_Parameter.whitelist == TRUE)
    {
        Save_WhiltList_Open(ConfigFileBuf,&iOffset);
    }
    else
    {
        Save_WhiltList_Close(ConfigFileBuf,&iOffset);
    }

    /*写入广播*/
    memcpy(ConfigFileBuf + iOffset,"#broadcast",strlen("#broadcast"));
    iOffset += strlen("#broadcast");
    ConfigFileBuf[iOffset++] = ENTER_CHAR;
    
    if(g_Ntp_Parameter.broadcast == TRUE)
    {
        Save_BroadCast(ConfigFileBuf,&iOffset);
    }
    else
    {
        memcpy(ConfigFileBuf + iOffset,"#end",strlen("#end"));
        iOffset += strlen("#end");
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
    }
    
    /*写入多播*/
    memcpy(ConfigFileBuf + iOffset,"#multicast",strlen("#multicast"));
    iOffset += strlen("#multicast");
    ConfigFileBuf[iOffset++] = ENTER_CHAR;
    
    if(g_Ntp_Parameter.multicast == TRUE)
    {
        Save_MultiCast(ConfigFileBuf,&iOffset);
    }
    else
    {
        memcpy(ConfigFileBuf + iOffset,"#end",strlen("#end"));
        iOffset += strlen("#end");
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
    }

    /*写入服务器地址*/
    memcpy(ConfigFileBuf + iOffset,"#server",strlen("#server"));
    iOffset += strlen("#server");
    ConfigFileBuf[iOffset++] = ENTER_CHAR;
    Save_ServerAddr(ConfigFileBuf,&iOffset);
    
    
    /*写入md 5 */
    memcpy(ConfigFileBuf + iOffset,"#keys",strlen("#keys"));
    iOffset += strlen("#keys");
    ConfigFileBuf[iOffset++] = ENTER_CHAR;
    
    if((g_Ntp_Enviroment.md5_flag & MD5_ENABLE) == MD5_ENABLE)
    {
        Save_keys(ConfigFileBuf,&iOffset);
    }
    else
    {
        memcpy(ConfigFileBuf + iOffset,"#end",strlen("#end"));
        iOffset += strlen("#end");
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
    }


    /**存入ethx  */
    memcpy(ConfigFileBuf + iOffset,"#ethxEnable",strlen("#ethxEnable"));
    iOffset += strlen("#ethxEnable");
    ConfigFileBuf[iOffset++] = ENTER_CHAR;
    
    for(i= 0;i<8;i++)
    {
        memcpy(ConfigFileBuf + iOffset,"#ethx",strlen("#ethx"));
        iOffset += strlen("#ethx");
        ConfigFileBuf[iOffset++] = ' ';
        ConfigFileBuf[iOffset++] = i + '0';
        ConfigFileBuf[iOffset++] = ' ';
        ConfigFileBuf[iOffset++] = g_ntp_enable[i] + '0';
        
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
    }
    
    memcpy(ConfigFileBuf + iOffset,"#end",strlen("#end"));
    iOffset += strlen("#end");
    ConfigFileBuf[iOffset++] = ENTER_CHAR;
    
    for(i = 0;i< iOffset;i++)
    {
        fputc(ConfigFileBuf[i],ntp_cfg_fp);
    }

    fflush(ntp_cfg_fp);
    fclose(ntp_cfg_fp);

}

/**********************************************************************
 * Function:      Save_All_Changes
 * Description:   保存所有配置信息
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Save_All_Changes()
{
    Uint8 ret;

    ret = Write_Md5ToKeyFile(g_Ntp_Enviroment.current_key,TOTAL_KEY_NO);
    if(ret == TRUE)
    {
        PLOG("Save to file md key successful !!\n");
    }
    
    Save_Ntp_Conf();
   
}

