/********************************************************************************************
*                           版权所有(C) 2015, 
*                                 版权所有
*********************************************************************************************
*    文 件 名:       ptp_io.c
*    功能描述:       ptp参数设置，初始化参数，配置文件操作等
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-2-6
*    函数列表:
                     Init_Identity
                     Init_ptpClock
                     Init_PtpMainParam
                     Init_PtpSendInterval
                     Init_RecvRdfs
                     Init_vlanParameter
                     Ptp_Manage_State
                     Wait_User_Config
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-2-6
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/


#include "ptp_io.h"
#include "arith.h"
#include "msg.h"
#include "net.h"
#include "comm_fun.h"
#include "ptp.h"


#define NETWORK_ENV     "/mnt/network.conf"
#define PTP_CONFIG_FILE "/mnt/ptp.conf"

#define CONFIG_FILE_SIZE 8096
#define LINE_LENGTH      200
#define LINE_COUNT       400
#define STRING_LENGTH    100

static int ptpPortIndex = 0;


typedef void* (*StateMachin_Manger)(PtpClock *pPtpClock);

extern void *Ptp_Manger_State(PtpClock *pPtpClock);
extern void *Slave_Manger_State(PtpClock *pPtpClock);
extern void *Master_Manger_State(PtpClock *pPtpClock);
extern void *Device_Main_State(PtpClock *pPtpClock);


StateMachin_Manger Ptp_State_Machine = Device_Main_State;


Uint32 g_pre_gwip = 0;       /* 当前网关地址 */
NetInfor g_Net_Infor;

void Display_NetInfo(NetInfor *infopt)
{
	Uint8 i,tem[2];
	Uint8 *macptr;
	struct sockaddr_in temsock;

	temsock.sin_addr.s_addr=infopt->ip;
    printf("\n[%s]\n",infopt->ifaceName);
	printf("IP address: \t  %s\r\n",inet_ntoa(temsock.sin_addr));
	temsock.sin_addr.s_addr=infopt->gwip;
	printf("Gateway address:  %s\r\n",inet_ntoa(temsock.sin_addr));
	temsock.sin_addr.s_addr=infopt->mask;
	printf("Subnet Mask: \t  %s\r\n",inet_ntoa(temsock.sin_addr));
    
    printf("MAC address: \t  %02x:%02x:%02x:%02x:%02x:%02x\r\n",infopt->mac[0],infopt->mac[1],infopt->mac[2],infopt->mac[3],infopt->mac[4],infopt->mac[5]);
	printf("\r\n");	
}


Uint8  VerifyIpAddress(Uint8 * src)
{
    #define NS_INADDRSZ 4
    
    static const char digits[] = "0123456789";
    int saw_digit,octets,ch;
    Uint8 tmp[NS_INADDRSZ],*tp;

    saw_digit = 0;
    octets = 0;
    *(tp = tmp) = 0;
    while((ch = *src++) != '\0')
    {
        const char *pch;

        if((pch = strchr(digits,ch)) != NULL)
        {
            Uint32 new = *tp *10 + (pch - digits);
            
            if(saw_digit && *tp == 0)
                return (0);
            if(new > 255)
                return (0);
            *tp = new;
            if(!saw_digit)
            {
                if(++octets > 4)
                    return (0);
            
                saw_digit = 1;           
            }

        }
        else if(ch == '.' && saw_digit)
        {
            if(octets == 4)
                return (0);

            *++tp = 0;
            saw_digit = 0;
        }
        else
        {
            return (0);
        }
        
    }
    if(octets < 4)
        return (0);

    return (1);
}

Uint8 VerifyNetMastAddress(Uint8 * src)
{
    unsigned int b = 0;

    Uint32 i;
    Uint32 n[4];
    
    if(VerifyIpAddress(src) == FALSE)
    {
        return FALSE;
    }
    else
    {
        sscanf(src,"%u.%u.%u.%u",&n[3],&n[2],&n[1],&n[0]);
        for(i= 0;i<4;i++)
        {
            b += n[i]<<(i*8);
        }

        b = ~b + 1;
        if((b&(b-1))==0)
            return TRUE;
    }

    return FALSE;
}

Int16 gf_ChrPosition(Int8 *ptrr,Int8 chrr)
{
	Int16 counter; 
	counter=1;
	while((*ptrr!=chrr))
	{	counter++;
		if ((*ptrr==0)||(counter>=200))
		return(0);
		ptrr++;
	}
	return(counter);
}

void MyStrnCpy(Uint8 *str1,Uint8 *str2,int n)
{

	strncpy(str1,str2,n);
	*(str1+n)='\0';
}

Uint8  VerifyMacAddress(Uint8 * src,Uint8  *tmac)
{
	Uint8 i;
	int tnum;
	char tsrc[25];
	char* tok = src;
	Uint8 k=0;
	Uint8 k1,k2;
	
	for(i = 0; i < 6; i++)
	{
		if(i==5)
		{
			k=strlen(tok);
			k=k+1;
		}
		else
		{
			k=gf_ChrPosition(tok,':');
		}

		if((k<=1)||(k>3))
		{
			PLOG("i=%d,k=%d\n",i,k);
			return 0;
		}
		
		MyStrnCpy(tsrc,tok,k-1);
		if(k==2)
		{
			k1=0;
			if((tsrc[0]>=0x30)&&(tsrc[0]<=0x39))   /* "0"~"9"*/
			{
				k2=tsrc[0]-0x30;
			}
			else if((tsrc[0]>=0x41)&&(tsrc[0]<=0x46))     /*"A"~"F"*/
			{
				k2=tsrc[0]-0x37;
			}
			else if((tsrc[0]>=0x61)&&(tsrc[0]<=0x66))      /*"a"~"f"*/
			{
				k2=tsrc[0]-0x57;
			}
			else
			{
				PLOG("k2=%c,i=%d,k=%d\n",tsrc[0],i,k);
				return 0;
			}

		}
		else if(k==3)
		{
			
			if((tsrc[0]>=0x30)&&(tsrc[0]<=0x39))   /* "0"~"9"*/
			{
				k1=tsrc[0]-0x30;
			}
			else if((tsrc[0]>=0x41)&&(tsrc[0]<=0x46))     /*"A"~"F"*/
			{
				k1=tsrc[0]-0x37;
			}
			else if((tsrc[0]>=0x61)&&(tsrc[0]<=0x66))      /*"a"~"f"*/
			{
				k1=tsrc[0]-0x57;
			}
			else
			{
				PLOG("k1=%c,i=%d,k=%d\n",tsrc[0],i,k);
				return 0;
			}
			if((tsrc[1]>=0x30)&&(tsrc[1]<=0x39))   /* "0"~"9"*/
			{
				k2=tsrc[1]-0x30;
			}
			else if((tsrc[1]>=0x41)&&(tsrc[1]<=0x46))     /*"A"~"F"*/
			{
				k2=tsrc[1]-0x37;
			}
			else if((tsrc[1]>=0x61)&&(tsrc[1]<=0x66))      /*"a"~"f"*/
			{
				k2=tsrc[1]-0x57;
			}
			else
			{
				PLOG("k2=%c,i=%d,k=%d\n",tsrc[1],i,k);
				return 0;
			}
		}
		tnum=k1*0x10+k2;
		if(tnum < 0 || tnum > 255)
		{
			PLOG("i=%d,tnum=%d\n",i,tnum);
			return 0;
		}
		tmac[i]=tnum&0xff;
		tok=tok+k;
		
	}
	
	return 1;		
}


void SaveNetParamToFile(PtpClock *pPtpClock,NetInfor *infopt)
{
    Uint8 line_str[200];
    struct sockaddr_in temsock;
    FILE *network_fd = fopen(pPtpClock->netWorkFileName,"w+");
    
    if(network_fd == NULL)
    {
        printf("can not find network.conf file\n");
        return;
    }
       
    memset(line_str,0,sizeof(line_str));
    
    temsock.sin_addr.s_addr = infopt->ip;
    sprintf(line_str,"%s:%s\n","IP",inet_ntoa(temsock.sin_addr));
    fputs(line_str,network_fd);
    
    memset(line_str,0,sizeof(line_str));
    temsock.sin_addr.s_addr = infopt->gwip;
    sprintf(line_str,"%s:%s\n","GATEWAY",inet_ntoa(temsock.sin_addr));
    fputs(line_str,network_fd);
    
    memset(line_str,0,sizeof(line_str));
    temsock.sin_addr.s_addr = infopt->mask;
    sprintf(line_str,"%s:%s\n","MASK",inet_ntoa(temsock.sin_addr));
    fputs(line_str,network_fd);
    
    memset(line_str,0,sizeof(line_str));
    sprintf(line_str,"%s:%x.%x.%x.%x.%x.%x\n","MAC",infopt->mac[0]
        ,infopt->mac[1]
        ,infopt->mac[2]
        ,infopt->mac[3]
        ,infopt->mac[4]
        ,infopt->mac[5]);
    
    fputs(line_str,network_fd);
    
    fflush(network_fd);
    fclose(network_fd);

}


void SetNetworkToEnv(NetInfor *infopt)
{
    //SetMacAddress(infopt->ifaceName,infopt->mac);

    SetIpAddress(infopt->ifaceName,infopt->ip);
    SetMaskAddress(infopt->ifaceName,infopt->mask);
    
#if 1
    if(AddGateWay(infopt->ifaceName,infopt->gwip))
        g_pre_gwip = infopt->gwip;

    usleep(100);
#endif

}

void SetRouteToEnv(PtpClock *pPtpClock,NetInfor *infopt)
{
    Uint8 route_temp[70];
    
    struct sockaddr_in temsock;
    int iOffset =0;
    
    Uint32 ethx_fd = open(pPtpClock->netRouteFileName,O_RDWR|O_CREAT|O_TRUNC,0666);
    if(ethx_fd < 0)
    {
        printf("open eth config error!\n");
        return;
    }
    
    temsock.sin_addr.s_addr=infopt->ip;
    
    lseek(ethx_fd,0,SEEK_SET);
    write(ethx_fd,"#!/bin/sh\n",10);
    
    memset(route_temp,0,70);
   //sprintf(route_temp,"%s%d\n","ip route del default table ",pPtpClock->routeTable);
    sprintf(route_temp,"%s%d\n","ip route flush table ",pPtpClock->routeTable);
    write(ethx_fd,route_temp,strlen(route_temp));
        
    memset(route_temp,0,70);

    /*ip route add default dev eth0 src 192.168.15.121 table 100*/

    sprintf(route_temp,"%s%s%s%s%s%d\n","ip route add default dev "
                                ,pPtpClock->netEnviroment.ifaceName
                                ," src "
                                ,inet_ntoa(temsock.sin_addr)
                                ," table "
                                ,pPtpClock->routeTable);
    
    write(ethx_fd,route_temp,strlen(route_temp));


    /*ip rule add from 192.168.15.229 table 100*/
    memset(route_temp,0,70);
    sprintf(route_temp,"%s%s%s%d\n","ip rule add from "
                                    ,inet_ntoa(temsock.sin_addr)
                                    ," table "
                                    ,pPtpClock->routeTable);

    write(ethx_fd,route_temp,strlen(route_temp));
    write(ethx_fd,"ip route flush cache\n",strlen("ip route flush cache\n"));
    //fsync(ethx_fd);
   
    close(ethx_fd);


    /**执行命令脚本  */
    memset(route_temp,0,sizeof(route_temp));
    memcpy(route_temp,"sh ",strlen("sh "));
    memcpy(route_temp+3,pPtpClock->netRouteFileName,strlen(pPtpClock->netRouteFileName));
    system(route_temp);


}


int Load_NetWork(PtpClock *pPtpClock,NetInfor *infopt)
{
    Uint8 line_str[50];
    
    Uint8 tile[10];
    Uint8 *pIndex;
    Uint8 *pStr = line_str;
    struct in_addr temsock;
    
    Uint8 *pMack;
    int i;
    
    memset(line_str,0,sizeof(line_str));
    memset(tile,0,sizeof(tile));
    
    FILE *net_work_fd = fopen(pPtpClock->netWorkFileName,"r");
    if(net_work_fd == NULL)
    {
        printf("can not find network.conf file\n");
            
        Init_NetEnviroment(&pPtpClock->netEnviroment);
        SaveNetParamToFile(pPtpClock,&pPtpClock->netEnviroment);
        SetRouteToEnv(pPtpClock,infopt);
        return -1;
    }
    
    while(fgets(line_str,sizeof(line_str),net_work_fd))
    {
        
        pIndex = strchr(line_str,':');
        memcpy(tile,line_str,pIndex-pStr);

        pIndex++;
        
        if(strcmp("IP",tile) == 0)
        {
            inet_aton(pIndex,&temsock);
            infopt->ip = temsock.s_addr;

        }
        if(strcmp("GATEWAY",tile) == 0)
        {
            inet_aton(pIndex,&temsock);
            infopt->gwip = temsock.s_addr;

        }
        if(strcmp("MASK",tile) == 0)
        {
            inet_aton(pIndex,&temsock);
            infopt->mask = temsock.s_addr;
            
        }

        if(strcmp("MAC",tile) == 0)
        {

            pMack = pIndex;
    		for(i = 0; i < 6; i++)
    		{
    			pMack = strtok(pIndex,".");
    			infopt->mac[i] = strtol(pMack,NULL,16);			
    			pIndex = NULL;
    		}		            
        }

        memset(tile,0,sizeof(tile));
        memset(line_str,0,sizeof(line_str));
    }
    SetNetworkToEnv(infopt);

    fclose(net_work_fd);

    SetRouteToEnv(pPtpClock,infopt);
}



void AnalysePtpConfigFile(Uint8* pBuf,PtpClock *pPtpClock)
{
    Uint8  Data[LINE_COUNT][LINE_LENGTH];
    Uint8 tile[STRING_LENGTH];
    Uint8 *pLine;
    Uint8 *pIndex;
    Uint8 *pStr;
    int i = 0;
    short value;
    
    memset(Data,0,sizeof(Data));
    
    strtok(pBuf,"\r\n");
    while(pLine = strtok(NULL,"\r\n"))
    {   
        memcpy(Data[i],pLine,strlen(pLine));
        i++;
    }
    //for(i=0;i<LINE_COUNT;i++)
       //printf("%s\n",Data[i]);
    
    for(i=0;i<LINE_COUNT;i++)
    {
        pIndex = strchr(Data[i],'=');
        
        if(pIndex == NULL)
            continue;
        
        pStr = Data[i];
        memset(tile,0,STRING_LENGTH);
        memcpy(tile,Data[i],pIndex-pStr);
        
        pIndex++;

        if(memcmp("ptpengine:clockType",tile,strlen("ptpengine:clockType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpClock->clockType = value;
                      
        }
         if(memcmp("ptpengine:domainFilterSwitch",tile,strlen("ptpengine:domainFilterSwitch")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpClock->domainFilterSwitch= value;
        }
        if(memcmp("ptpengine:domainNumber",tile,strlen("ptpengine:domainNumber")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 255)
                pPtpClock->domainNumber = value;
        }
        if(memcmp("ptpengine:protoType",tile,strlen("ptpengine:protoType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpClock->protoType = value;
        }
        if(memcmp("ptpengine:modeType",tile,strlen("ptpengine:modeType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpClock->modeType = value;
        }
        if(memcmp("ptpengine:transmitDelayType",tile,strlen("ptpengine:transmitDelayType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpClock->transmitDelayType = value;
        }
        if(memcmp("ptpengine:stepType",tile,strlen("ptpengine:stepType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpClock->stepType = value;
        }
        if(memcmp("ptpengine:UniNegotiationEnable",tile,strlen("ptpengine:UniNegotiationEnable")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpClock->UniNegotiationEnable = value;
        }
        
        if(memcmp("ptpengine:validServerNum",tile,strlen("ptpengine:validServerNum")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 10)
                pPtpClock->unicastMultiServer.validServerNum = value;
        }
        if(memcmp("ptpengine:synFreq",tile,strlen("ptpengine:synFreq")) == 0)
        {
            value = atoi(pIndex);
            if(value >= -8 && value <= 4)
            {
                pPtpClock->synSendInterval = Get_MessageInterval(value);
                pPtpClock->logSyncInterval = value;
            }
        }
        if(memcmp("ptpengine:announceFreq",tile,strlen("ptpengine:announceFreq")) == 0)
        {
            value = atoi(pIndex);
            if(value >= -8 && value <= 4)
            {
                pPtpClock->announceSendInterval= Get_MessageInterval(value);
                pPtpClock->logAnnounceInterval= value;
            }

        }
        if(memcmp("ptpengine:delayreqFreq",tile,strlen("ptpengine:delayreqFreq")) == 0)
        {
            value = atoi(pIndex);
            if(value >= -8 && value <= 4)
            {
                pPtpClock->delayreqInterval = Get_MessageInterval(value);
                pPtpClock->logMinDelayReqInterval= value;
            }

        }

        if(memcmp("ptpengine:pdelayreqFreq",tile,strlen("ptpengine:pdelayreqFreq")) == 0)
        {
            value = atoi(pIndex);
            if(value >= -8 && value <= 4)
            {
                pPtpClock->delayreqInterval = Get_MessageInterval(value);
                pPtpClock->logMinPdelayReqInterval = value;
            }

        }


        if(memcmp("ptpengine:UnicastDuration",tile,strlen("ptpengine:UnicastDuration")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 10 && value <= 1000)
            {
                pPtpClock->UnicastDuration = value;
            }
        }
        if(memcmp("ptpengine:currentUtcOffset",tile,strlen("ptpengine:currentUtcOffset")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 255)
            {
                pPtpClock->timePropertiesDS.currentUtcOffset = value;
            }

        }

        if(memcmp("ptpengine:grandmasterPriority1",tile,strlen("ptpengine:grandmasterPriority1")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 255)
            {
                pPtpClock->grandmasterPriority1 = value;
            }

        }
        if(memcmp("ptpengine:grandmasterPriority2",tile,strlen("ptpengine:grandmasterPriority2")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 255)
            {
                pPtpClock->grandmasterPriority2 = value;
            }

        }
        if(memcmp("ptpengine:ServerIp_0",tile,strlen("ptpengine:ServerIp_0")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[0].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[0].valid = TRUE;
            }
            
        }
        if(memcmp("ptpengine:ServerMac_0",tile,strlen("ptpengine:ServerMac_0")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[0].serverMac))
                pPtpClock->unicastMultiServer.serverList[0].valid = TRUE;
        }


        if(memcmp("ptpengine:ServerIp_1",tile,strlen("ptpengine:ServerIp_1")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[1].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[1].valid = TRUE;
            }
        }
        if(memcmp("ptpengine:ServerMac_1",tile,strlen("ptpengine:ServerMac_1")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[1].serverMac))
                pPtpClock->unicastMultiServer.serverList[1].valid = TRUE;

        }

        if(memcmp("ptpengine:ServerIp_2",tile,strlen("ptpengine:ServerIp_2")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[2].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[2].valid = TRUE;

            }
        }
        if(memcmp("ptpengine:ServerMac_2",tile,strlen("ptpengine:ServerMac_2")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[2].serverMac))
                pPtpClock->unicastMultiServer.serverList[2].valid = TRUE;

        }


        if(memcmp("ptpengine:ServerIp_3",tile,strlen("ptpengine:ServerIp_3")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[3].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[3].valid = TRUE;

            }
        }
        if(memcmp("ptpengine:ServerMac_3",tile,strlen("ptpengine:ServerMac_3")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[3].serverMac))
                pPtpClock->unicastMultiServer.serverList[3].valid = TRUE;

        }


        if(memcmp("ptpengine:ServerIp_4",tile,strlen("ptpengine:ServerIp_4")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[4].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[4].valid = TRUE;

            }
        }
        if(memcmp("ptpengine:ServerMac_4",tile,strlen("ptpengine:ServerMac_4")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[4].serverMac))
                pPtpClock->unicastMultiServer.serverList[4].valid = TRUE;

        }


        if(memcmp("ptpengine:ServerIp_5",tile,strlen("ptpengine:ServerIp_5")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[5].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[5].valid = TRUE;

            }
        }
        if(memcmp("ptpengine:ServerMac_5",tile,strlen("ptpengine:ServerMac_5")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[5].serverMac))
                pPtpClock->unicastMultiServer.serverList[5].valid = TRUE;

        }


        if(memcmp("ptpengine:ServerIp_6",tile,strlen("ptpengine:ServerIp_6")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[6].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[6].valid = TRUE;

            }
        }
        if(memcmp("ptpengine:ServerMac_6",tile,strlen("ptpengine:ServerMac_6")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[5].serverMac))   
                pPtpClock->unicastMultiServer.serverList[6].valid = TRUE;

        }

        if(memcmp("ptpengine:ServerIp_7",tile,strlen("ptpengine:ServerIp_7")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[7].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[7].valid = TRUE;

            }
        }
        if(memcmp("ptpengine:ServerMac_7",tile,strlen("ptpengine:ServerMac_7")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[7].serverMac))
                pPtpClock->unicastMultiServer.serverList[7].valid = TRUE;

        }


        if(memcmp("ptpengine:ServerIp_8",tile,strlen("ptpengine:ServerIp_8")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[8].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[8].valid = TRUE;

            }
        }
        if(memcmp("ptpengine:ServerMac_8",tile,strlen("ptpengine:ServerMac_8")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[8].serverMac))
                pPtpClock->unicastMultiServer.serverList[8].valid = TRUE;

        }


        if(memcmp("ptpengine:ServerIp_9",tile,strlen("ptpengine:ServerIp_9")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpClock->unicastMultiServer.serverList[9].serverIp = inet_addr(pIndex);
                pPtpClock->unicastMultiServer.serverList[9].valid = TRUE;

            }
        }
        if(memcmp("ptpengine:ServerMac_9",tile,strlen("ptpengine:ServerMac_9")) == 0)
        {
            if(VerifyMacAddress(pIndex,pPtpClock->unicastMultiServer.serverList[9].serverMac))
                pPtpClock->unicastMultiServer.serverList[9].valid = TRUE;

        }
        if(memcmp("ptpengine:DebugLevel",tile,strlen("ptpengine:DebugLevel")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 2)
            {
                pPtpClock->debugLevel = value;

            }

        }
       
    }

}

int Load_PtpParam_FromFile(PtpClock *pPtpClock)
{
    Uint8 ConfigFileBuf[CONFIG_FILE_SIZE];
    int charactor;
    int i;


    /**只读打开文件  */
    FILE  *ptp_fd = fopen(pPtpClock->ptpFileName,"a+");
    if(ptp_fd == NULL)
    {
        printf("Can not Find ptp.conf file !\n");
        exit(-1);;
    }

    /**读取文件所有数据  */
    i = 0;
    memset(ConfigFileBuf,0,CONFIG_FILE_SIZE);
    while((charactor = fgetc(ptp_fd))!= EOF)
    {
        ConfigFileBuf[i] = charactor;
        i++;
    }
    
    fclose(ptp_fd);
    
    if(i<2)
    {
        printf("Config file Empty!!\n");
        Save_PtpParam_ToFile(pPtpClock,pPtpClock->ptpFileName);
        return -1;
    }
        

    /**分析文件  */
    AnalysePtpConfigFile(ConfigFileBuf,pPtpClock);
    
}



int Save_PtpParam_ToFile(PtpClock *pPtpClock,char *fileName)
{
    
    Uint8 line_str[200];
    int str=0;
    struct sockaddr_in temsock;
    int i;
    
    FILE *ptp_fd = fopen(fileName,"w+");
    if(ptp_fd == NULL)
    {
        printf("can not find ptp.conf file\n");
        return -1;
    }

    memset(line_str,0,sizeof(line_str));
    sprintf(line_str,"%s:%s=%s\n","ptpengine","interface","eth0");
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->clockType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","clockType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->domainNumber;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","domainNumber",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->domainFilterSwitch;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","domainFilterSwitch",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->protoType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","protoType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->modeType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","modeType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->transmitDelayType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","transmitDelayType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->stepType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","stepType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->UniNegotiationEnable;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","UniNegotiationEnable",str);
    fputs(line_str,ptp_fd);
        
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->logSyncInterval;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","synFreq",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->logAnnounceInterval;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","announceFreq",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->logMinDelayReqInterval;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","delayreqFreq",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->logMinDelayReqInterval;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","pdelayreqFreq",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->grandmasterPriority1;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","grandmasterPriority1",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->grandmasterPriority2;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","grandmasterPriority2",str);
    fputs(line_str,ptp_fd);


    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->unicastMultiServer.validServerNum;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","validServerNum",str);
    fputs(line_str,ptp_fd); 
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->UnicastDuration;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","UnicastDuration",str);
    fputs(line_str,ptp_fd); 

    memset(line_str,0,sizeof(line_str));
    str = pPtpClock->timePropertiesDS.currentUtcOffset;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","currentUtcOffset",str);
    fputs(line_str,ptp_fd); 


    for(i=0;i<pPtpClock->unicastMultiServer.validServerNum;i++)
    {
        memset(line_str,0,sizeof(line_str));
        temsock.sin_addr.s_addr = pPtpClock->unicastMultiServer.serverList[i].serverIp;
        sprintf(line_str,"%s:%s%d=%s\n","ptpengine","ServerIp_",i,inet_ntoa(temsock.sin_addr));
        fputs(line_str,ptp_fd); 
        
        memset(line_str,0,sizeof(line_str));
        sprintf(line_str,"%s:%s%d=%02x:%02x:%02x:%02x:%02x:%02x\n","ptpengine","ServerMac_"
                           ,i
                           ,pPtpClock->unicastMultiServer.serverList[i].serverMac[0]
                           ,pPtpClock->unicastMultiServer.serverList[i].serverMac[1]
                           ,pPtpClock->unicastMultiServer.serverList[i].serverMac[2]
                           ,pPtpClock->unicastMultiServer.serverList[i].serverMac[3]
                           ,pPtpClock->unicastMultiServer.serverList[i].serverMac[4]
                           ,pPtpClock->unicastMultiServer.serverList[i].serverMac[5]);
        fputs(line_str,ptp_fd); 

    }
    
    fflush(ptp_fd);
    fclose(ptp_fd);

    return TRUE;
}


void DisplayVlanConfiguration(PtpClock *pPtpClock)
{
    printf("------------------\r\n");
    printf("VLAN CONFIGURATION\r\n");
    printf("------------------\r\n");

    if(pPtpClock->vlanEnable== TRUE)
            printf("Vlan: [On]\n");
    else if(pPtpClock->vlanEnable == FALSE)
        printf("Vlan: [Off]\r\n");

    printf("Vlan Id: [%d]\n",pPtpClock->vlanId);
    printf("Vlan Priority [%d]\n",pPtpClock->vlanPriority);
    
    if(pPtpClock->vlanCfi == TRUE)
        printf("Vlan Cfi: [No Standard]\n");
    else if(pPtpClock->vlanCfi == FALSE)
        printf("Vlan Cfi: [Standard]\r\n");

}

void Display_ServUnicastIpList(UnicastMultiServer *pServer_unicast)
{
    int i = 0; 
    struct sockaddr_in temsock;
    
    printf("\r\n==Unicast Server Ip Table==\r\n");
    printf("Num        IP                \n");
    for(i = 0;i < MAX_SERVER;i++)
    {
        if(pServer_unicast->serverList[i].valid == TRUE)
        {
            temsock.sin_addr.s_addr = pServer_unicast->serverList[i].serverIp;
            printf("%d    %s     \n",i,inet_ntoa(temsock.sin_addr));
        }

    }
    printf("\r\n=============================\r\n");

}


void Display_ServUnicastMacList(UnicastMultiServer *pServer_unicast)
{
    int i = 0; 
    printf("\r\n======Unicast Mac Table======\r\n");
    printf("Num        MAC          \n");
    for(i = 0;i < MAX_SERVER;i++)
    {
        if(pServer_unicast->serverList[i].valid == TRUE)
        {
            printf("%d    %02x:%02x:%02x:%02x:%02x:%02x \n",i
                ,pServer_unicast->serverList[i].serverMac[0]
                ,pServer_unicast->serverList[i].serverMac[1]
                ,pServer_unicast->serverList[i].serverMac[2]
                ,pServer_unicast->serverList[i].serverMac[3]
                ,pServer_unicast->serverList[i].serverMac[4]
                ,pServer_unicast->serverList[i].serverMac[5]);
        }

    }
    printf("\r\n=============================\r\n");

}

void Delete_ServUnicastList(UnicastMultiServer *pServer_unicast)
{
    int i;

    for(i = 0;i < MAX_SERVER;i++)
    {
        pServer_unicast->serverList[i].serverIp = 0;
        pServer_unicast->serverList[i].valid = 0;
        memset(pServer_unicast->serverList[i].serverMac,0,6);
    }
    pServer_unicast->validServerNum = 0;
    
}

void show(char *head,char *data, int len)
{
    int i;
    printf("%s:",head);
    while(1)
    {
        
        printf("0x%x ",*data);
        data++;

        if(*data == 0x0a)
            break;
    }
    printf("\n");
}

void DisplayAllConfiguration(PtpClock *pPtpClock)
{    
    
    struct sockaddr_in temsock;
    int i;

    printf("------------------\r\n");
    printf("PTP CONFIGURATION\r\n");
    printf("------------------\r\n");

    printf("\r\n===================\r\n");
    printf("Send Packet Speed --Config Value\r\n");
    printf("-8(1/256);  -7(1/128); -6(1/64); -5(1/32);\r\n");
    printf("-4(1/16);  -3(1/8);    -2(1/4);  -1(1/2);\r\n");
    printf("0(1);      2(4);       3(8);     4(16);  \r\n");
    printf("explain:16= send 1 Packet per 16 Seconds \r\n");
    printf("explain:1/16= send 16 Packets per 1 Second \r\n");
    printf("======================\r\n");



    if(pPtpClock->protoType == PROTO_UDP_IP)
        printf("ProtocolType: [UDP/IPv4]\n");
    else if(pPtpClock->protoType== PROTO_IEEE802)
        printf("ProtocolType: [IEEE802.3/Ethernet]\r\n");

    if(pPtpClock->modeType== IPMODE_MULTICAST)
        printf("ModeType: [Multicast]\n");
    else if(pPtpClock->modeType== IPMODE_UNICAST)
        printf("ModeType: [Unicast]\r\n");

    if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
        printf("DelayType: [P2P]\n");
    else if(pPtpClock->transmitDelayType== DELAY_MECANISM_E2E)
        printf("DelayType: [E2E]\r\n");

    if(pPtpClock->domainFilterSwitch == TRUE)
        printf("DomainNumber Filter: [On]\n");
    else if(pPtpClock->domainFilterSwitch == FALSE)
        printf("DomainNumber Filter: [Off]\n");

    printf("DomainNumber: [%d]\n",pPtpClock->domainNumber);
    
    if(pPtpClock->clockType == PTP_SLAVE)
    {
        if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
            printf("DelayReq Interval: [%d]\n",pPtpClock->logMinDelayReqInterval);
        if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
            printf("PDelayReq Interval: [%d]\n",pPtpClock->logMinPdelayReqInterval);


        if(pPtpClock->modeType == IPMODE_UNICAST)
        {

            printf("------------------\r\n");
            printf("UNICAST CONFIGURATION\r\n");
            printf("------------------\r\n");

            printf("validServerNum: [%d]\n",pPtpClock->unicastMultiServer.validServerNum);

            if(pPtpClock->protoType == PROTO_UDP_IP)
            {

                Display_ServUnicastIpList(&pPtpClock->unicastMultiServer);
        
            }
            else if(pPtpClock->protoType == PROTO_IEEE802)
            {

                Display_ServUnicastMacList(&pPtpClock->unicastMultiServer);
            }
        
            if(pPtpClock->UniNegotiationEnable == TRUE)
                printf("UnicastNegotiation: [On]\n");
            else if(pPtpClock->UniNegotiationEnable == FALSE)
                printf("UnicastNegotiation: [Off]\r\n");
        
                printf("UnicastNegotiation: [%d s]\n",pPtpClock->UnicastDuration);

        }
        

    }
    else if(pPtpClock->clockType == PTP_MASTER)
    {

        if(pPtpClock->stepType == ONE_STEP)
            printf("StepType: [One Step]\n");
        else if(pPtpClock->stepType== TWO_STEP)
            printf("StepType: [Two Step]\r\n");



        printf("Syn Interval: [%d]\n",pPtpClock->logSyncInterval);
        printf("Announce Interval: [%d]\n",pPtpClock->logAnnounceInterval);

        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            printf("------------------\r\n");
            printf("UNICAST CONFIGURATION\r\n");
            printf("------------------\r\n");
            
            if(pPtpClock->UniNegotiationEnable == TRUE)
                printf("UnicastNegotiation: [On]\n");
            else if(pPtpClock->UniNegotiationEnable == FALSE)
                printf("UnicastNegotiation: [Off]\r\n");
        }

    }
    else
    {
        printf("Ptp ClockType [%d]\n",pPtpClock->clockType);
    }
    DisplayVlanConfiguration(pPtpClock);
}


int EditSaveAllConfig(PtpClock *pPtpClock)
{
    Uint8 str_input[50];
    Uint8 len;
    while(1)
    {
        printf("Input YES to Save all Changes or Input NO to Cancel or Press ENTER to return!\r\n");
        printf("Do You Really Want to Save all Changes ?\\>\r\n");

        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }
        if((strcmp(str_input,"YES")==0) || (strcmp(str_input,"yes")== 0))
        {
            Save_PtpParam_ToFile(pPtpClock,pPtpClock->ptpFileName);
            printf("Save All Changes Successfully !!\r\n");
            break;
        }
        else if(strcmp(str_input,"NO")==0 || (strcmp(str_input,"no") == 0))
        {
            printf("Save all Changes Cancel !!\r\n");
            break;
        }
        else
            printf("Invalid input!\r\n\r\n");
    }
}


void EditClockType(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("ClockType Configuration\n");
    printf("===================\r\n");
    
    if(pPtpClock->clockType == PTP_MASTER)
        printf("ClockType: [PTP_MASTER]\n");
    else if(pPtpClock->clockType== PTP_SLAVE)
        printf("ClockType: [PTP_SLAVE]\r\n");
    else 
        printf("ClockType [Other]\n");
        
    while(1)
    {
        printf("Input 0:Slave;1:Master;or Press ENTER to return(e.g. 0)\\>\r\n");
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->clockType = PTP_SLAVE;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->clockType = PTP_MASTER;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! ClockType Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }
}


void EditProtoType(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("ProtocolType Configuration\n");
    printf("===================\r\n");
    
    if(pPtpClock->protoType == PROTO_UDP_IP)
        printf("ProtocolType: [UDP/IPv4]\n");
    else if(pPtpClock->protoType== PROTO_IEEE802)
        printf("ProtocolType: [IEEE802.3/Ethernet]\r\n");
    else 
        printf("ProtocolType error\n");
        
    while(1)
    {
        printf("Input 0:IEEE802.3/Ethernet;1:UDP/IPv4;or Press ENTER to return(e.g. 0)\\>\r\n");
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->protoType = PROTO_IEEE802;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->protoType = PROTO_UDP_IP;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! ProtocolType Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }
}

void EditModeType(PtpClock *pPtpClock)
{
    Uint8 my_choice;

    printf("\r\n===================\r\n");
    printf("ModeType Configration\n");
    printf("===================\r\n");

    if(pPtpClock->modeType== IPMODE_MULTICAST)
        printf("ModeType: [Multicast]\n");
    else if(pPtpClock->modeType== IPMODE_UNICAST)
        printf("ModeType: [Unicast]\r\n");
    else 
        printf("ModeType error\n");

    while(1)
    {
        printf("Input 0:Multicast;1:Unicast;or Press ENTER to return(e.g. 0)\\>\r\n");
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->modeType = IPMODE_MULTICAST;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->modeType = IPMODE_UNICAST;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! ModeType Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }

}

void EditTransmitDelayType(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("P2P/E2E Configration\n");
    printf("===================\r\n");

    if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
        printf("DelayType: [P2P]\n");
    else if(pPtpClock->transmitDelayType== DELAY_MECANISM_E2E)
        printf("DelayType: [E2E]\r\n");
    else 
        printf("transmitDelayType error\n");

    while(1)
    {
        printf("Input 0:P2P;1:E2E;or Press ENTER to return(e.g. 0)\\>\r\n");
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->transmitDelayType = DELAY_MECANISM_P2P;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->transmitDelayType = DELAY_MECANISM_E2E;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! TransmitDelayType Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }

}


void EditStepType(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("StepType Configration\n");
    printf("===================\r\n");

    if(pPtpClock->stepType == ONE_STEP)
        printf("StepType: [One Step]\n");
    else if(pPtpClock->stepType== TWO_STEP)
        printf("StepType: [Two Step]\r\n");
    else 
        printf("StepType error\n");

    while(1)
    {
        printf("Input 0:One Step;1:Two Step;or Press ENTER to return(e.g. 0)\\>\r\n");
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->stepType = ONE_STEP;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->stepType = TWO_STEP;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! StepType Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }

}



void EditSynInterval(PtpClock *pPtpClock)
{
    Uint8 str_input[50];
    short num;
    short len;
    Uint8 ret;

    printf("\r\n===================\r\n");
    printf("Syn Interval Configration\r\n");
    printf("======================\r\n");

    printf("\r\n===================\r\n");
    printf("Send Packet Speed --Config Value\r\n");
    printf("-8(1/256);  -7(1/128); -6(1/64); -5(1/32);\r\n");
    printf("-4(1/16);  -3(1/8);    -2(1/4);  -1(1/2);\r\n");
    printf("0(1);      2(4);       3(8);     4(16);  \r\n");
    printf("explain:16= send 1 Packet per 16 Seconds \r\n");
    printf("explain:1/16= send 16 Packets per 1 Second \r\n");
    printf("======================\r\n");



    printf("Syn Interval: [%d]\n",pPtpClock->logSyncInterval);

    while(1)
    {
        printf("Input Syn Interval Config(-8 ~ 4);or Press ENTER To Return(e.g. 0)\\>\r\n");
        
        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateIntervalNum(str_input,len,&num);
        if(ret == FALSE)
        {
            printf("Invalid number, input again! Syn Interval Value Must Be -8 ~ 4 !!\r\n\r\n");
            continue;
        }
        else
        {
            if(num < -8 || num >4)
            {
                printf("Invalid number, input again! Syn Interval Value Must Be -8 ~4 !!\r\n\r\n");
                continue;
        
            }
            else
            {

                pPtpClock->logSyncInterval = num;
                pPtpClock->synSendInterval = Get_MessageInterval(pPtpClock->logSyncInterval);
                printf("success!!\n");
                break;
            }
        
        }

    }

}

void EditAnnounceInterval(PtpClock *pPtpClock)
{

    Uint8 str_input[50];
    short num;
    short len;
    Uint8 ret;

    printf("\r\n===================\r\n");
    printf("Announce Interval Configration\r\n");
    printf("======================\r\n");

    printf("\r\n===================\r\n");
    printf("Send Packet Speed --Config Value\r\n");
    printf("-8(1/256);  -7(1/128); -6(1/64); -5(1/32);\r\n");
    printf("-4(1/16);  -3(1/8);    -2(1/4);  -1(1/2);\r\n");
    printf("0(1);      2(4);       3(8);     4(16);  \r\n");
    printf("explain:16= send 1 Packet per 16 Seconds \r\n");
    printf("explain:1/16= send 16 Packets per 1 Second \r\n");
    printf("======================\r\n");



    printf("Announce Interval: [%d]\n",pPtpClock->logAnnounceInterval);

    while(1)
    {
        printf("Input Announce Interval Config(-8 ~ 4);or Press ENTER To Return(e.g. 0)\\>\r\n");
        
        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateIntervalNum(str_input,len,&num);
        if(ret == FALSE)
        {
            printf("Invalid number, input again! Announce Interval Value Must Be -8 ~ 4!!\r\n\r\n");
            continue;
        }
        else
        {
            if(num < -8 || num >4)
            {
                printf("Invalid number, input again! Announce Interval Value Must Be -8 ~ 4!!\r\n\r\n");
                continue;
        
            }
            else
            {
                pPtpClock->logAnnounceInterval = num;
                pPtpClock->announceSendInterval = Get_MessageInterval(pPtpClock->logAnnounceInterval);
                printf("success!!\n");
                break;
            }
        
        }

    }

}

void EditDelayReqInterval(PtpClock *pPtpClock)
{
    Uint8 str_input[50];
    short num;
    short len;
    Uint8 ret;

    printf("\r\n===================\r\n");
    printf("DelayReq Interval Configration\r\n");
    printf("======================\r\n");

    printf("\r\n===================\r\n");
    printf("Send Packet Speed --Config Value\r\n");
    printf("-8(1/256);  -7(1/128); -6(1/64); -5(1/32);\r\n");
    printf("-4(1/16);  -3(1/8);    -2(1/4);  -1(1/2);\r\n");
    printf("0(1);      2(4);       3(8);     4(16);  \r\n");
    printf("explain:16= send 1 Packet per 16 Seconds \r\n");
    printf("explain:1/16= send 16 Packets per 1 Second \r\n");
    printf("======================\r\n");



    if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
        printf("DelayReq Interval: [%d]\n",pPtpClock->logMinDelayReqInterval);
    else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
        printf("PDelayReq Interval: [%d]\n",pPtpClock->logMinPdelayReqInterval);

    while(1)
    {

        if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
            printf("Input DelayReq Interval Config(-8 ~ 4);or Press ENTER To Return(e.g. 0)\\>\r\n");
        else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
            printf("Input PDelayReq Interval Config(-8 ~ 4);or Press ENTER To Return(e.g. 0)\\>\r\n");
            
        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateIntervalNum(str_input,len,&num);
        if(ret == FALSE)
        {
            printf("Invalid number, input again! Interval Value Must Be -8 ~ 4 !!\r\n\r\n");
            continue;
        }
        else
        {
            if(num < -8 || num > 4)
            {
                printf("Invalid number, input again! Interval Value Must Be -8 ~ 4 !!\r\n\r\n");
                continue;

            }
            else
            {
                if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
                {
                    pPtpClock->logMinDelayReqInterval = num;
                    pPtpClock->delayreqInterval = Get_MessageInterval(pPtpClock->logMinDelayReqInterval);

                }
                else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
                {
                    pPtpClock->logMinPdelayReqInterval = num;
                    pPtpClock->delayreqInterval = Get_MessageInterval(pPtpClock->logMinPdelayReqInterval);
                }
                
                printf("success!!\n");
                break;
            }

        }
      

    }

}

void EditUnicastIp(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    Uint8 str_input[50];
    struct sockaddr_in temsock;

    printf("\r\n===================\r\n");
    printf("Unicast Ip Configration\n");
    printf("===================\r\n");
    
    temsock.sin_addr.s_addr = pPtpClock->unicastMultiServer.serverList[0].serverIp;

    printf("UnicastServer Ip Address: %s \r\n",inet_ntoa(temsock.sin_addr));

    while(1)
    {
        printf("Input Server Ip or Press ENTER to retrun(e.g. 192.168.15.25)\\>\r\n");
        memset(str_input,0,sizeof(str_input));

        Get_SerialString(str_input,sizeof(str_input));
        if(str_input[0]== 0x0d || str_input[0] == 0x0a)
        {
            break;
        }
        if(!VerifyIpAddress(str_input))
        {
            printf("\aInvalid address!\r\n\r\n");
        }
        else
        {
            pPtpClock->unicastMultiServer.serverList[0].serverIp = inet_addr(str_input);
            printf("Ip Address Success!!\n");
            break;
        }

    }
}

void EditUnicastMac(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    Uint8 str_input[50];

    printf("\r\n===================\r\n");
    printf("Unicast Mac Configration\n");
    printf("===================\r\n");
    
    printf("UnicastServer Mac Address: %02x:%02x:%02x:%02x:%02x:%02x \r\n"
        ,pPtpClock->unicastMultiServer.serverList[0].serverMac[0]
        ,pPtpClock->unicastMultiServer.serverList[0].serverMac[1]
        ,pPtpClock->unicastMultiServer.serverList[0].serverMac[2]
        ,pPtpClock->unicastMultiServer.serverList[0].serverMac[3]
        ,pPtpClock->unicastMultiServer.serverList[0].serverMac[4]
        ,pPtpClock->unicastMultiServer.serverList[0].serverMac[5]);

    while(1)
    {
        printf("Input Server Mac or Press ENTER to retrun(e.g. 12:34:56:78:ab:cd)\\>\r\n");
        memset(str_input,0,sizeof(str_input));

        Get_SerialString(str_input,sizeof(str_input));
        if(str_input[0]== 0x0d || str_input[0] == 0x0a)
        {
            break;
        }
        if(!VerifyMacAddress(str_input,pPtpClock->unicastMultiServer.serverList[0].serverMac))
        {
            printf("\aInvalid address!\r\n\r\n");
        }
        else
        {
            printf("Mac Address Success!!\n");
            break;
        }

    }
}


void Edit_UnicastServerIp(UnicastMultiServer *pServer_unicast)
{
    Uint8 ret = 0;
    Uint8 input;
    short num = 0;
    Uint8 i;
    Uint8 str_input[50];
    Uint8 len;


    /**输入 IP  */
    while(1)
    {
        Display_ServUnicastIpList(pServer_unicast);
        printf("Press D to delete all lists\r\n");
        printf("Input number N to Select num or Press ENTER to retrun: (N range:0,1,2,3...9)\r\n\\>");

        input = Get_SerialChar();
        
        if(input == 0x0d || input ==0x0a)
        {
            break;
        }
        else if(input == 'd' || input == 'D')
        {
            Delete_ServUnicastList(pServer_unicast);
            continue;

        }
        else
        {
            num = input - '0';
            
            if(num>9 || num<0)
            {
                printf("Invalid number, input again!\r\n\r\n");
                continue;
            }
            else
            {
                printf("Input number: %d Successful !!\n",num);
            }

        }
            
        /*输入ip地址*/
        while(1)
        {
            printf("Input Server IP or Press ENTER to retrun(e.g. 192.168.15.25)\r\n\\>");
            memset(str_input,0,sizeof(str_input));
            
            Get_SerialString(str_input,sizeof(str_input));
            if(str_input[0]== 0x0d || str_input[0] == 0x0a)
            {
                break;
            }
            if(!VerifyIpAddress(str_input))
            {
                printf("\aInvalid address!\r\n\r\n");
            }
            else
            {
                pServer_unicast->serverList[num].serverIp = inet_addr(str_input);
                pServer_unicast->serverList[num].valid = TRUE;
                printf("Ip Address Success!!\n");
                break;
            }
        }
        
    }
    
}

void Edit_UnicastServerMac(UnicastMultiServer *pServer_unicast)
{
    Uint8 ret = 0;
    Uint8 input;
    short num = 0;
    Uint8 i;
    Uint8 str_input[50];
    Uint8 len;


    /**输入 Mac */
    while(1)
    {
        Display_ServUnicastMacList(pServer_unicast);
        printf("Press D to delete all lists\r\n");
        printf("Input number N to Select num or Press ENTER to retrun: (N range:0,1,2,3...9)\r\n\\>");

        input = Get_SerialChar();

        if(input == 0x0d || input ==0x0a)
        {
            break;
        }
        else if(input == 'd' || input == 'D')
        {
            Delete_ServUnicastList(pServer_unicast);
            continue;

        }
        else
        {
            num = input - '0';
            
            if(num>9 || num<0)
            {
                printf("Invalid number, input again!\r\n\r\n");
                continue;
            }
            else
            {
                printf("Input number: %d Successful !!\n",num);
            }

        }

        /*输入Mac地址*/
        while(1)
        {
            printf("Input MAC address or Press ENTER to retrun(e.g. 12:34:56:78:ab:cd)\r\n\\>");
            memset(str_input,0,sizeof(str_input));
            
            Get_SerialString(str_input,sizeof(str_input));
            if(str_input[0]== 0x0d || str_input[0] == 0x0a)
            {
                break;
            }

            if(!VerifyMacAddress(str_input,pServer_unicast->serverList[num].serverMac))
            {
                printf("\aInvalid address!\r\n\r\n");
            }
            else
            {
                pServer_unicast->serverList[num].valid = TRUE;
                printf("Mac Address Success!!\n");
                break;
            }
        }
        
    }

}


void Edit_ValidServerNumber(UnicastMultiServer *pServer_unicast)
{
    Uint8 ret = 0;
    Uint8 input;
    Uint32 num = 0;
    Uint8 i;
    Uint8 str_input[50];
    Uint8 len;

    
    /** Validate Num */
    while(1)
    {   
        printf("\nValid Num [%d]\n",pServer_unicast->validServerNum);
        printf("Input Valid Server Num or Press ENTER to return (e.g. 0~10)\r\n\\>");
        
        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));
        
        ret = ValidtateNum(str_input,len,&num);
        if(ret == 2)
        {
            break;
        }
        else if(ret == FALSE)
        {
            printf("Invalid number, input again!\r\n\r\n");
        }
        else
        {
            if(num > 10)
            {
                printf("Invalid number, input again!\r\n\r\n");
            }
            else
            {
                pServer_unicast->validServerNum = num;
                printf("success!!\n");
                break;
            }
        }

    }

}


void EditUnicastNegotiation(PtpClock *pPtpClock)
{
    Uint8 my_choice;

    printf("\r\n===================\r\n");
    printf("UnicastNegotiation Configration\n");
    printf("===================\r\n");

    if(pPtpClock->UniNegotiationEnable == TRUE)
        printf("UnicastNegotiation: [On]\n");
    else if(pPtpClock->UniNegotiationEnable == FALSE)
        printf("UnicastNegotiation: [Off]\r\n");
    else 
        printf("UnicastNegotiation error\n");

    while(1)
    {
        printf("Input 0:Off;1:On;or Press ENTER to return(e.g. 1)\\>\r\n");
        
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->UniNegotiationEnable = FALSE;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->UniNegotiationEnable = TRUE;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! UnicastNegotiation Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }

}

void EditUnicastDuration(PtpClock *pPtpClock)
{
    
    Uint8 str_input[50];
    Uint32 num;
    short len;
    Uint8 ret;
    
    printf("\r\n===================\r\n");
    printf("UnicastDuration Configration\n");
    printf("===================\r\n");

    printf("UnicastNegotiation: [%d s]\n",pPtpClock->UnicastDuration);

    while(1)
    {
        printf("Input UnicastDuration(10~1000);or Press ENTER To Return(e.g. 300)\\>\r\n");

        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateNum(str_input,len,&num);

        if(ret == FALSE)
        {
            printf("Invalid number, input again! UnicastDuration Value Must Be 10~1000 !!\r\n\r\n");
            continue;

        }
        else
        {
            if(num < 10 || num >1000)
            {
                printf("Invalid number, input again! UnicastDuration Value Must Be 10~1000 !!\r\n\r\n");
                continue;

            }
            else
            {
                pPtpClock->UnicastDuration = num;
                printf("Input Success\n");
                break;

            }

        }
        
    }

}

void EditCurrentUtcOffset(PtpClock *pPtpClock)
{
    
    Uint8 str_input[50];
    Uint32 num;
    short len;
    Uint8 ret;
    
    printf("\r\n===================\r\n");
    printf("CurrentUtcOffset Configration\n");
    printf("===================\r\n");

    printf("CurrentUtcOffset: [%d s]\n",pPtpClock->timePropertiesDS.currentUtcOffset);

    while(1)
    {
        printf("Input currentUtcOffset(34~100);or Press ENTER To Return(e.g. 35)\\>\r\n");

        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateNum(str_input,len,&num);

        if(ret == FALSE)
        {
            printf("Invalid number, input again! currentUtcOffset Value Must Be 34~100 !!\r\n\r\n");
            continue;

        }
        else
        {
            if(num < 34 || num >100)
            {
                printf("Invalid number, input again! currentUtcOffset Value Must Be 34~100 !!\r\n\r\n");
                continue;

            }
            else
            {
                pPtpClock->timePropertiesDS.currentUtcOffset = num;
                printf("Input Success\n");
                break;

            }

        }
        
    }

}

void EditDomainNumber(PtpClock *pPtpClock)
{
    
    Uint8 str_input[50];
    Uint8 my_choice;
    Uint32 num;
    short len;
    Uint8 ret;
    
    
    printf("\r\n===================\r\n");
    printf("DomainNumber Configration\n");
    printf("===================\r\n");

    if(pPtpClock->domainFilterSwitch == TRUE)
        printf("DomainNumber Filter [On]\n");
    else if(pPtpClock->domainFilterSwitch == FALSE)
        printf("DomainNumber Filter [Off]\n");
    else
        printf("DomainNumber Filter error\n");
    
    printf("DomainNumber: [%d]\n",pPtpClock->domainNumber);

    while(1)
    {
        printf("Input 0:Off;1:On;or Press ENTER to return(e.g. 1)\\>\r\n");
        
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->domainFilterSwitch = FALSE;
            printf("Input Success\n");
            return;
        }
        else if(my_choice == '1')
        {
            pPtpClock->domainFilterSwitch = TRUE;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid domainFilterSwitch, input again!  Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }


    while(1)
    {
        printf("Input DomainNumber(0~255);or Press ENTER To Return(e.g. 0)\\>\r\n");

        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateNum(str_input,len,&num);

        if(ret == FALSE)
        {
            printf("Invalid number, input again! DomainNumber Value Must Be 0~255 !!\r\n\r\n");
            continue;

        }
        else
        {
            if(num >255)
            {
                printf("Invalid number, input again! DomainNumber Value Must Be 0~255 !!\r\n\r\n");
                continue;

            }
            else
            {
                pPtpClock->domainNumber = num;
                printf("Input Success\n");
                break;

            }

        }
        
    }

}


void EditPrintDebug(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("Debug Print Configuration\r\n");
    printf("===================\r\n");

    
    if(pPtpClock->debugLevel == 0)
        printf("debugLevel: [Off]\n");
    else if(pPtpClock->debugLevel == 1)
        printf("debugLevel: [Msg Debug On]\r\n");
    else
        printf("debugLevel error\n");
        
    while(1)
    {
        printf("Input 0:Off;1:Msg Debug On;or Press ENTER to return(e.g. 0)\\>\r\n");
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->debugLevel = 0;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->debugLevel = 1;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! debugLevel Value Must Be 0/1 !!\r\n\r\n");
            printf("");
            continue;
        }
        
    }



}

void EditVlanSwitch(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("Vlan Configration\n");
    printf("===================\r\n");

    if(pPtpClock->vlanEnable== TRUE)
        printf("Vlan: [On]\n");
    else if(pPtpClock->vlanEnable == FALSE)
        printf("Vlan: [Off]\r\n");
    else 
        printf("Vlan error\n");

    while(1)
    {
        printf("Input 0:Off;1:On;or Press ENTER to return(e.g. 1)\\>\r\n");
        
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->vlanEnable= FALSE;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->vlanEnable = TRUE;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! vlanEnable Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }

}

void EditVlanId(PtpClock *pPtpClock)
{
    Uint8 str_input[50];
    Uint32 num;
    short len;
    Uint8 ret;

    printf("\r\n===================\r\n");
    printf("Vlan Id Configration\n");
    printf("===================\r\n");

    printf("Vlan Id: [%d]\n",pPtpClock->vlanId);

    while(1)
    {
        printf("Input VlanId(0~255);or Press ENTER To Return(e.g. 1)\\>\r\n");

        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateNum(str_input,len,&num);

        if(ret == FALSE)
        {
            printf("Invalid number, input again! VlanId Value Must Be 0~255 !!\r\n\r\n");
            continue;

        }
        else
        {
            if(num >255)
            {
                printf("Invalid number, input again! VlanId Value Must Be 0~255 !!\r\n\r\n");
                continue;

            }
            else
            {
                pPtpClock->vlanId = num;
                printf("Input Success\n");
                break;

            }

        }
        
    }

}

void EditVlanPriority(PtpClock *pPtpClock)
{
    Uint8 my_choice;
        
    printf("\r\n===================\r\n");
    printf("Vlan Priority Configration\n");
    printf("===================\r\n");

    printf("Vlan Priority [%d]\n",pPtpClock->vlanPriority);


    while(1)
    {
        printf("Input Vlan Priority(0~7);or Press ENTER to return(e.g. 1)\\>\r\n");
        
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        if(my_choice >= '0' && my_choice <= '7')
        {
            pPtpClock->vlanPriority = my_choice - '0';
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! VlanPriority Value Must Be 0~7 !!\r\n\r\n");
            continue;
        }
        
    }

}

void EditVlanStandard(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("Vlan Standard Configration\n");
    printf("===================\r\n");

    if(pPtpClock->vlanCfi == TRUE)
        printf("Vlan Cfi: [No Standard]\n");
    else if(pPtpClock->vlanCfi == FALSE)
        printf("Vlan Cfi: [Standard]\r\n");
    else 
        printf("Vlan error\n");

    while(1)
    {
        printf("Input 0:Standard;1:No Standard;or Press ENTER to return(e.g. 1)\\>\r\n");
        
        my_choice = Get_SerialChar();
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        else if(my_choice == '0')
        {
            pPtpClock->vlanCfi= FALSE;
            printf("Input Success\n");
            break;
        }
        else if(my_choice == '1')
        {
            pPtpClock->vlanCfi = TRUE;
            printf("Input Success\n");
            break;
        }
        else
        {
            printf("Invalid number, input again! Vlan Standard Value Must Be 0/1 !!\r\n\r\n");
            continue;
        }
        
    }

}




void *UnicastConfigState(PtpClock *pPtpClock)
{
    
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("Unicast Configuration\r\n");
    printf("===================\r\n");

    if(pPtpClock->protoType == PROTO_UDP_IP)
        printf("1 : Unicast Ip Configuration\r\n");
    else if(pPtpClock->protoType == PROTO_IEEE802)
        printf("1 : Unicast Mac Configuration\r\n");
    
    printf("2 : Unicast Negotiation\r\n");
    printf("3 : Unicast Duration\r\n");
    printf("4 : Unicast Valid Number Configuration\r\n");
    printf("R : Return\r\n");
    printf("===================\r\n");
    printf("Select ? \r\n");


    my_choice = Get_SerialChar();

    switch (my_choice)
    {
        case '1':
            if(pPtpClock->protoType == PROTO_UDP_IP)
                Edit_UnicastServerIp(&pPtpClock->unicastMultiServer);
                //EditUnicastIp(pPtpClock);
            else if(pPtpClock->protoType == PROTO_IEEE802)
                Edit_UnicastServerMac(&pPtpClock->unicastMultiServer);
                //EditUnicastMac(pPtpClock);
            break;
        case '2':
            EditUnicastNegotiation(pPtpClock);
            break;
        case '3':
            EditUnicastDuration(pPtpClock);
            break;
        case '4':
            Edit_ValidServerNumber(&pPtpClock->unicastMultiServer);
            break;
        case 'R':
        case 'r':
            return(void*)Ptp_Manger_State;
        default:
            printf("Invalid Selection!!\n");
            break;
    }
    return (void*)UnicastConfigState;
}


void *VlanConfigState(PtpClock *pPtpClock)
{
    
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("Vlan Configuration\r\n");
    printf("===================\r\n");
    
    printf("1 : Vlan Switch \r\n");
    printf("2 : Vlan Id Configuration\r\n");
    printf("3 : Vlan Priority Configuration\r\n");
    printf("4 : Vlan Standard Selection\r\n");
    printf("D : Display Vlan Configuration\r\n");
    printf("R : Return\r\n");
    printf("===================\r\n");
	printf("Select ? \r\n");
    my_choice = Get_SerialChar();

    switch (my_choice)
    {
        case '1':
            EditVlanSwitch(pPtpClock);
            break;
        case '2':
            EditVlanId(pPtpClock);
            break;
        case '3':
            EditVlanPriority(pPtpClock);
            break;
        case '4':
            EditVlanStandard(pPtpClock);
            break;
        case 'D':
        case 'd':
            DisplayVlanConfiguration(pPtpClock);
            break;
        case 'R':
        case 'r':
            return(void*)Ptp_Manger_State;
        default:
            printf("Invalid Selection!!\n");
            break;
    }
    return (void*)VlanConfigState;
}


void *ProfileConfigState(PtpClock *pPtpClock)
{
    printf("\r\n===================\r\n");
    printf("Profile Configuration\r\n");
    printf("===================\r\n");
    
    printf("R : Return\r\n");
}

void *Slave_Manger_State(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("Slave Configuration\r\n");
    printf("===================\r\n");
    
    printf("1 : IEEE 802.3/Ethernet or UDP/IP Configuration\r\n");
    printf("2 : Multicast or Unicast Mode Configuration\r\n");
    printf("3 : P2P or E2E Configuration\r\n");
    printf("4 : Delay_Req Send Interval Configuration\r\n");
    
    printf("5 : Vlan Configuration\r\n");
    printf("6 : DomainNumber Configuration\r\n");
    //printf("6 : Profile Configuration\r\n");

    if(pPtpClock->modeType == IPMODE_UNICAST)
        printf("U : Unicast Configuration\r\n");
    
    printf("D : Display Configuration\r\n");
    printf("R : Return\r\n");
    printf("===================\r\n");
    printf("Select ? \r\n");
    
    my_choice = Get_SerialChar();

    switch (my_choice)
    {
        case '1':
            EditProtoType(pPtpClock);
            break;
        case '2':
            EditModeType(pPtpClock);
            break;
        case '3':
            EditTransmitDelayType(pPtpClock);
            break;
        case '4':
            EditDelayReqInterval(pPtpClock);
            break;
        case '5':
            return (void *)VlanConfigState;

        case '6':
            EditDomainNumber(pPtpClock);
            break;
         
        case 'U':
        case 'u':
            return (void *)UnicastConfigState;

            break;
        case 'D':
        case 'd':
            DisplayAllConfiguration(pPtpClock);
            break;
            
        case 'R':
        case 'r':
            return (void *)Device_Main_State;
        default:
            printf("Invalid Selection!!\n");
            break;
    }

    return (void*)Slave_Manger_State;
}

void *Master_Manger_State(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("Master Configuration\r\n");
    printf("===================\r\n");
    
    printf("1 : IEEE 802.3/Ethernet or UDP/IP Configuration\r\n");
    printf("2 : Multicast or Unicast Configuration\r\n");    
    printf("3 : P2P or E2E Configuration\r\n");
    printf("4 : One Step or Two Step Configuration\r\n");
    printf("5 : Sync Send Interval Configuration\r\n");
    printf("6 : Announce Send Interval Configuration\r\n");
    printf("7 : Vlan Configuration\r\n");
    
    printf("8 : DomainNumber Configuration\r\n");
    printf("L : UTC to TAI Leap Seconds Configuration\r\n");

    //printf("8 : Profile Configuration\r\n");
    
    if(pPtpClock->modeType == IPMODE_UNICAST)
        printf("U : Unicast Configuration\r\n");
    
    printf("D : Display Configuration\r\n");
    
    printf("R : Return\r\n");
    printf("===================\r\n");
    printf("Select ? \r\n");

    my_choice = Get_SerialChar();

    switch (my_choice)
    {
        case '1':
            EditProtoType(pPtpClock);
            break;
        case '2':
            EditModeType(pPtpClock);
            break;
        case '3':
            EditTransmitDelayType(pPtpClock);
            break;
        case '4':
            EditStepType(pPtpClock);
            break;
        case '5':
            EditSynInterval(pPtpClock);
            break;
            
        case '6':
            EditAnnounceInterval(pPtpClock);
            break;
        case '7':
            return (void *)VlanConfigState;
        case '8':
            EditDomainNumber(pPtpClock);
            break;
        case 'L':
        case 'l':
            EditCurrentUtcOffset(pPtpClock);
            break;
            
        case 'U':
        case 'u':
            EditUnicastNegotiation(pPtpClock);
            break;

            break;
        case 'D':
        case 'd':
            DisplayAllConfiguration(pPtpClock);
            break;
            
        case 'R':
        case 'r':
            return (void *)Device_Main_State;;
        default:
            printf("Invalid Selection!!\n");
            break;
    }
    
    return (void*)Master_Manger_State;


}

void *Ptp_Manger_State(PtpClock *pPtpClock)
{
    if(pPtpClock->clockType == PTP_MASTER)
        return (void*)Master_Manger_State;
    else if(pPtpClock->clockType == PTP_SLAVE)
        return (void *)Slave_Manger_State;
}

void *NetWork_Config_State(PtpClock *pPtpClock)
{
	Uint8   i;
    Uint8   len = 0;
    Uint8   addr_str[30];
    Uint8   tmac[6];
    Uint8   my_choice;


    printf("------------------\r\n");
	printf("NetWork Configuration\r\n");
	printf("------------------\r\n");
	
	printf("1: IP address\r\n");
	printf("2: Gateway address\r\n");
	printf("3: Subnet Mask\r\n");
	printf("4: MAC address\r\n");
    printf("S: Save the Change\r\n");
	printf("D: Display Net Configuration\r\n");
    printf("R: Return\r\n");
	printf("------------------\r\n");
	printf("Select ?\r\n");

    my_choice = Get_SerialChar();

    switch (my_choice)
    {
        case '1' :
            
            while(1)
            {
                memset(addr_str,0,sizeof(addr_str));
    			printf("Input source IP or Press ENTER to retrun(e.g. 192.168.15.25)\r\n\\>");
                len = Get_SerialString(addr_str,sizeof(addr_str));
                if(len == 1)
                {
                    break;
                }
                                    
                if(!VerifyIpAddress(addr_str))
                {
                    printf("\aInvalid address!!\r\n\r\n");
                    continue;
                }
                
                printf("Successfully input IP address!!\r\n");
                g_Net_Infor.ip = inet_addr(addr_str);
                                
    		    break;
            }
            break;

    	case '2':
            while(1)
            {
                memset(addr_str,0,sizeof(addr_str));
    			printf("Input GateWay IP or Press ENTER to retrun(e.g. 192.168.15.255)\r\n\\>");
                len = Get_SerialString(addr_str,sizeof(addr_str));
                if(len == 1)
                {
                    break;
                }
                                    
                if(!VerifyIpAddress(addr_str))
                {
                    printf("\aInvalid address!!\r\n\r\n");
                    continue;
                }
                
                printf("Successfully input Gateway address!!\r\n");
                
                
                g_Net_Infor.gwip= inet_addr(addr_str);
                
                break;
            }
            break;

        case '3':

            while(1)
            {
                printf("Input Subnet mask or Press ENTER to retrun(e.g. 255.255.255.0)\r\n\\>");
                len = Get_SerialString(addr_str,sizeof(addr_str));
                if(len == 1)
                {
                    break;
                }
                
                if(!VerifyNetMastAddress(addr_str))
                {
                    printf("\aInvalid address!!\r\n\r\n");
                    continue;
                }
                
                printf("Successfully input Subnet mask!!\r\n");
                g_Net_Infor.mask = inet_addr(addr_str);
                
                break;
            }
            break;

        case '4':
            while(1)
            {
                printf("Input MAC address or Press ENTER to retrun(e.g. 12:34:56:78:ab:cd)\r\n\\>");
                len = Get_SerialString(addr_str,sizeof(addr_str));
                if(len == 1)
                {
                    break;
                }
                if(!VerifyMacAddress(addr_str,tmac))
                {
                    printf("\aInvalid MAC address!!\r\n\r\n");
                    continue;
                }
                
                printf("Successfully input MAC address!!\r\n");
                
                memcpy(g_Net_Infor.mac,tmac,6);
                
                break;
            }
            break;

        case 'D':
        case 'd':
            Display_NetInfo(&g_Net_Infor);
            break;
        case 'S':
        case 's':
            pPtpClock->netEnviroment = g_Net_Infor;
            
            SaveNetParamToFile(pPtpClock,&g_Net_Infor);
            SetNetworkToEnv(&g_Net_Infor);
            printf("Successfully Save NetWork Config !!\r\n");
            SetRouteToEnv(pPtpClock,&g_Net_Infor);
            sleep(3);
            break;
        case 'R':
        case 'r':
            
           return (void *)Device_Main_State;

        default:
            printf("\aInvalid selection!\r\n\r\n");
            break;
    }

    
    return(void*)NetWork_Config_State;
}

void PreSelectWitchPort(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    int index;
    
    printf("\r\n===================\r\n");
    printf("SELECT PORT\r\n");
    printf("===================\r\n");

    printf("Selected ConfigurePort: [%d], All Port: [%d]\n",ptpPortIndex,PTP_PORT_COUNT);

    
    while(1)
    {
        printf("Select Port(0-%d);or Press ENTER To Return(e.g. 0)\\>\r\n",PTP_PORT_COUNT);
        
        my_choice = Get_SerialChar();
        index = my_choice - '0';
        
        if(my_choice == 0x0a || my_choice == 0x0d)
        {
            break;
        }
        
        if(index < PTP_PORT_COUNT)
        {
            ptpPortIndex = index;
            printf("success !!\n");
            break;
        }
        else 
        {
            printf("invalid Select ! Please input again!\n");
            
        }
    }

}

void *Device_Main_State(PtpClock *pPtpClock)
{
    Uint8 my_choice;
    
    printf("\r\n===================\r\n");
    printf("PTP[%d] Configuration\r\n",ptpPortIndex);
    printf("===================\r\n");
    
    printf("C : Select Config Port\r\n");
    printf("N : Network Configuration\r\n");
    printf("P : PTP Configuration\r\n");    
    printf("M : Master or Slave\r\n");
    printf("D : Debug Print (Authorized user only!)\r\n");
    printf("S : Save Configuration\r\n");
    printf("R : Return\r\n");
    printf("===================\r\n");
    printf("Select ? \r\n");
    
    my_choice = Get_SerialChar();
    
    switch (my_choice)
    {
        case 'N':
        case 'n':
            g_Net_Infor = pPtpClock->netEnviroment;
            return (void *)NetWork_Config_State;
        case 'P':
        case 'p':
            return (void *)Ptp_Manger_State;
        case 'M':
        case 'm':
            EditClockType(pPtpClock);
            break;
        case 'D':
        case 'd':
            EditPrintDebug(pPtpClock);
            break;
        case 'c':
        case 'C':
            PreSelectWitchPort(pPtpClock);
            break;
            
        case 'S':
        case 's':
            EditSaveAllConfig(pPtpClock);
            break;
        case 'R':
        case 'r':
            return NULL;
        default:
            printf("Invalid Selection!!\n");
            break;
    }

    return (void *)Device_Main_State;
}

/********************************************************************************************
* 函数名称:    Ptp_Manage_Configuration
* 功能描述:    串口配置，采用状态机方式
* 输入参数:    无
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Ptp_Manage_Configuration(void)
{
    Uint8 buf[30];
    while(1)
    {
        Ptp_State_Machine = (StateMachin_Manger)(*Ptp_State_Machine)(&g_ptpClock[ptpPortIndex]);
        if(Ptp_State_Machine == NULL)
        {
            Ptp_State_Machine = Device_Main_State;
            break;
        }
    }
}

void PtpNtp_Configuration()
{
    Uint8 my_choice;

    
    while(1)
    {
        printf("\r\n===================\r\n");
        printf("PTP-2 Configuration\r\n");
        printf("===================\r\n");
        
        printf("1 : PTP Configuration\r\n");
        printf("2 : NTP Configuration\r\n");
        printf("Q : Quit\r\n");
        printf("===================\r\n");
        printf("Select ? \r\n");
        my_choice = Get_SerialChar();

        switch (my_choice)
        {
            case '1':
                Ptp_Manage_Configuration();
                break;
            case '2':
                
                break;
            case 'Q':
            case 'q':
                printf("Quit !\r\n");
                break;
            default:
                printf("Invalid Selection!!\n");
                break;
        }
        if(my_choice == 'q' || my_choice == 'Q')
        {
            break;
        }
        if(my_choice == '$')
            exit(-1);

    }
}




/********************************************************************************************
* 函数名称:    Wait_User_Config
* 功能描述:    超时3s等待串口输入，如果超时则运行ptp，如果检测有回车，则进入配置
               界面
* 输入参数:    无
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Wait_User_Config(PtpClock *pPtpClock)
{
	struct timeval timeout;
	fd_set fs_read;
    
    int ret;
    int wait_cnt = 3;

	/*用来查询设备状态（读，写，异常）的改变，在timeout时间内*/

    printf("\r\nPress ENTER to enter the Configuration Menu :   ");
    fflush(stdout);
    
    while(wait_cnt)
    {
        timeout.tv_sec = 1;									
	    timeout.tv_usec = 0;
        FD_ZERO(&fs_read);
        FD_SET(0,&fs_read);

        ret = select(1,&fs_read,NULL,NULL,&timeout);
        if(ret)
        {
            Get_SerialChar();
            PtpNtp_Configuration();
            break;
        }
        else
        {
            printf("\b\b%d ",wait_cnt);
            fflush(stdout);
            wait_cnt--;
        }

    }


}


/********************************************************************************************
* 函数名称:    Init_RecvRdfs
* 功能描述:    初始化接收的rdfs，不同模式下，放入rdfs的sockfd不同
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_RecvRdfs(PtpClock *pPtpClock)
{
    int maxfd;
    int ret;
    fd_set current_rdfs;

    FD_ZERO(&current_rdfs);
    maxfd = 0;

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
        {
            FD_SET(pPtpClock->netComm.IeeeE2ESock,&current_rdfs);
            maxfd = Max_Fd(maxfd,pPtpClock->netComm.IeeeE2ESock);
        }
        else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
        {
            FD_SET(pPtpClock->netComm.IeeeE2ESock,&current_rdfs);
            //FD_SET(pPtpClock->netComm.IeeeP2PSock,&current_rdfs);
            maxfd = Max_Fd(maxfd,pPtpClock->netComm.IeeeE2ESock);
            //maxfd = Max_Fd(maxfd,pPtpClock->netComm.IeeeP2PSock);
        }

    }
    else if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            FD_SET(pPtpClock->netComm.eventSock,&current_rdfs);          
            FD_SET(pPtpClock->netComm.generalSock,&current_rdfs);
            //FD_SET(pPtpClock->netComm.PeventSock,&current_rdfs);
            //FD_SET(pPtpClock->netComm.PgeneralSock,&current_rdfs);
            maxfd = Max_Fd(maxfd,pPtpClock->netComm.eventSock);
            maxfd = Max_Fd(maxfd,pPtpClock->netComm.generalSock);
            //maxfd = Max_Fd(maxfd,pPtpClock->netComm.PeventSock);
            //maxfd = Max_Fd(maxfd,pPtpClock->netComm.PgeneralSock);
        }
        else if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            FD_SET(pPtpClock->netComm.eventSock,&current_rdfs);
            FD_SET(pPtpClock->netComm.generalSock,&current_rdfs);
            FD_SET(pPtpClock->netComm.PeventSock,&current_rdfs);
            FD_SET(pPtpClock->netComm.PgeneralSock,&current_rdfs);

            maxfd = Max_Fd(maxfd,pPtpClock->netComm.eventSock);
            maxfd = Max_Fd(maxfd,pPtpClock->netComm.generalSock);
            maxfd = Max_Fd(maxfd,pPtpClock->netComm.PeventSock);
            maxfd = Max_Fd(maxfd,pPtpClock->netComm.PgeneralSock);

        }

    }

    /**赋值rdfs，用于select  */
    pPtpClock->netComm.global_rdfs = current_rdfs;
    pPtpClock->netComm.maxfd = maxfd;

}

/********************************************************************************************
* 函数名称:    Init_vlanParameter
* 功能描述:    vlan相关参数初始化
* 输入参数:    PtpClock *pPtpClock  
               Uint8 vlanEnable     
               Uint8 vlanPriority   
               Uint8 vlanCfi        
               Uint8 vlanId         
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_vlanParameter(PtpClock *pPtpClock
    ,Uint8 vlanEnable
    ,Uint8 vlanPriority
    ,Uint8 vlanCfi
    ,Uint8 vlanId)
{
    pPtpClock->vlanEnable = vlanEnable;
    pPtpClock->vlanPriority = vlanPriority;
    pPtpClock->vlanCfi = vlanCfi;
    pPtpClock->vlanId = vlanId;
}


/********************************************************************************************
* 函数名称:    Init_PtpMainParam
* 功能描述:    初始化ptp主要运行参数
* 输入参数:    PtpClock *pPtpClock      
               Uint8 versionNumer       
               char ifname[4]           
               Uint8 clockType          
               Uint8 domainNumber       
               Uint8 protoType          
               Uint8 modeType           
               Uint8 stepType           
               Uint8 transmitDelayType  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_PtpMainParam(PtpClock *pPtpClock,Uint8 versionNumer
    ,char ifname[4]
    ,Uint8 clockType,Uint8 domainNumber,Uint8 protoType
    ,Uint8 modeType,Uint8 stepType,Uint8 transmitDelayType
    )
{
    pPtpClock->versionNumber = versionNumer;
    memcpy(pPtpClock->netEnviroment.ifaceName,ifname,4);

    pPtpClock->clockType = clockType;
    pPtpClock->domainNumber = domainNumber;
    pPtpClock->protoType = protoType;

    
    pPtpClock->modeType = modeType;
    pPtpClock->stepType = stepType;
    pPtpClock->transmitDelayType = transmitDelayType;
    
}


/********************************************************************************************
* 函数名称:    Init_PtpSendInterval
* 功能描述:    初始化syn、announce、delay、signal 发送周期
* 输入参数:    PtpClock *pPtpClock     
               Uint8 synInterval       
               Uint8 announceInterval  
               Uint8 delayInterval     
              
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_PtpSendInterval(PtpClock *pPtpClock
    ,Sint8 synInterval
    ,Sint8 announceInterval
    ,Sint8 delayInterval
    )
{

    pPtpClock->logAnnounceInterval = announceInterval;
    pPtpClock->logSyncInterval = synInterval;
    pPtpClock->logMinDelayReqInterval = delayInterval;
    pPtpClock->logMinPdelayReqInterval= delayInterval;


    pPtpClock->synSendInterval = Get_MessageInterval(synInterval);
    pPtpClock->announceSendInterval = Get_MessageInterval(announceInterval);
    pPtpClock->delayreqInterval = Get_MessageInterval(delayInterval);


}

/********************************************************************************************
* 函数名称:    Init_ptpClock
* 功能描述:    初始化ptp clock参数
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Uint8 Init_ptpClock(PtpClock *pPtpClock,int index)
{
    int i = 0;
    int j = 0;
    Uint8 netInterface[10];
    Uint8 filename[40];

    memset(pPtpClock,0,sizeof(PtpClock));

    pPtpClock->portEnable = TRUE;
    pPtpClock->InitBmcflag = TRUE;

    
    memset(filename,0,sizeof(filename));
    memset(netInterface,0,sizeof(netInterface));

    if(index == 0)
    {
        sprintf(filename,"%s","/mnt/ptp_s.conf");
        memcpy(pPtpClock->ptpFileName,filename,strlen(filename));
        sprintf(netInterface,"%s%d","eth",index);

        /**初始值ptp 主要参数  */
        Init_PtpMainParam(pPtpClock
                            ,PTP_VERRION_2
                            ,netInterface
                            ,PTP_SLAVE
                            ,0
                            ,PROTO_UDP_IP
                            ,IPMODE_UNICAST
                            ,ONE_STEP
                            ,DELAY_MECANISM_E2E);

    }
    else if(index == 1)
    {
        sprintf(filename,"%s","/mnt/ptp_m.conf");
        memcpy(pPtpClock->ptpFileName,filename,strlen(filename));
        sprintf(netInterface,"%s%d","eth",index);

        /**初始值ptp 主要参数  */
        Init_PtpMainParam(pPtpClock
                            ,PTP_VERRION_2
                            ,netInterface
                            ,PTP_MASTER
                            ,0
                            ,PROTO_UDP_IP
                            ,IPMODE_UNICAST
                            ,ONE_STEP
                            ,DELAY_MECANISM_E2E);

    }

    
    /**初始化发送周期  */
    Init_PtpSendInterval(pPtpClock,0,0,0);

    /**初始化vlan  */
    Init_vlanParameter(pPtpClock
                        ,FALSE
                        ,0x03
                        ,FALSE
                        ,0x02);

    pPtpClock->domainFilterSwitch = FALSE;/**默认为关  */

    /** 准备Announce 消息 */
    pPtpClock->timePropertiesDS.currentUtcOffsetValid  = TRUE;
    pPtpClock->timePropertiesDS.currentUtcOffset = 36;
    pPtpClock->timePropertiesDS.frequencyTraceable = TRUE;
    pPtpClock->timePropertiesDS.leap59 = 0;
    pPtpClock->timePropertiesDS.leap61 = 0;

#if 1
    pPtpClock->timePropertiesDS.ptpTimescale = TRUE;
    pPtpClock->timePropertiesDS.timeSource = INTERNAL_OSCILLATOR;
    pPtpClock->timePropertiesDS.timeTraceable = TRUE;

    pPtpClock->grandmasterPriority1 = DEFAULT_PRIORITY1;
    pPtpClock->grandmasterPriority2 = DEFAULT_PRIORITY1;
    pPtpClock->grandmasterClockQuality.clockAccuracy = DEFAULT_CLOCK_ACCURACY;
    pPtpClock->grandmasterClockQuality.clockClass = DEFAULT_CLOCK_CLASS;
    pPtpClock->grandmasterClockQuality.offsetScaledLogVariance = 0;
#endif

#if 0
    pPtpClock->timePropertiesDS.ptpTimescale = TRUE;
    pPtpClock->timePropertiesDS.timeSource = GPS;
    pPtpClock->timePropertiesDS.timeTraceable = TRUE;
    
    pPtpClock->grandmasterPriority1 = DEFAULT_PRIORITY1;
    pPtpClock->grandmasterPriority2 = DEFAULT_PRIORITY1;
    pPtpClock->grandmasterClockQuality.clockAccuracy = CLOCK_ACCURACY_25ns;
    pPtpClock->grandmasterClockQuality.clockClass = LOCK_CLOCK_CLASS;
    pPtpClock->grandmasterClockQuality.offsetScaledLogVariance = 0;
#endif

    /**初始化单播多服务器及单播协商  */
    pPtpClock->UniNegotiationEnable = FALSE;
    pPtpClock->UnicastDuration = 300;
    pPtpClock->unicastMultiServer.validServerNum = 1;
    pPtpClock->unicastMultiServer.serverList[0].serverIp = inet_addr("192.168.1.233");
    pPtpClock->unicastMultiServer.serverList[0].valid = TRUE;
    
    pPtpClock->unicastMultiServer.serverList[0].serverMac[0] = 0x00;
    pPtpClock->unicastMultiServer.serverList[0].serverMac[1] = 0x60;
    pPtpClock->unicastMultiServer.serverList[0].serverMac[2] = 0x6e;
    pPtpClock->unicastMultiServer.serverList[0].serverMac[3] = 0x14;
    pPtpClock->unicastMultiServer.serverList[0].serverMac[4] = 0x04;
    pPtpClock->unicastMultiServer.serverList[0].serverMac[5] = 0x12;
    
    return 1;

}

/********************************************************************************************
* 函数名称:    Init_Identity
* 功能描述:    初始化本机用于PTP传输的Identitiy
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_Identity(PtpClock *pPtpClock,UInteger16 PortNumber)
{
    int i = 0;
    int j = 0;

    for (i=0;i<CLOCK_IDENTITY_LENGTH;i++)
    {
        if (i==3)
            pPtpClock->portIdentity.clockIdentity[i]=0xFF;
        else if(i==4)
            pPtpClock->portIdentity.clockIdentity[i]=0xFE;
        else
        {
          pPtpClock->portIdentity.clockIdentity[i]=pPtpClock->netEnviroment.mac[j];
          j++;
        }
    }
    
    pPtpClock->portIdentity.portNumber = PortNumber;

    if(pPtpClock->clockType == PTP_MASTER) 
        copyClockIdentity(pPtpClock->grandmasterIdentity,pPtpClock->portIdentity.clockIdentity);

}

