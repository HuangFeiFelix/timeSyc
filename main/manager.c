#include "manager.h"


#define CTL_WORD_DISCOVERY  0x10
#define CTL_WORD_RESPONSE   0x11
#define CTL_WORD_REQ        0x05
#define CTL_WORD_ACK        0x06
#define CTL_WORD_DATA       0x07
#define CTL_WORD_SET        0x08
#define CTL_WORD_NAK        0x15

#define HEAD_FRAME_LENGTH   12
#define SEND_BUFF_LENGTH    1500


#define CMD_HEART_BEAT          0x0000
#define CMD_DEV_INFORMATION     0x0001
#define CMD_NET_WORK_ADDRESS    0x0002
#define CMD_VERSION_INFROMATION 0x0003
#define CMD_GPS_STATUS          0x0004
#define CMD_SYS_SET             0x0005

#define CMD_PTP_CFG_ALL         0x0010
#define CMD_PTP_CFG_NORMAL      0x0011
#define CMD_PTP_CFG_SLAVE       0x0012
#define CMD_PTP_CFG_MASTER      0x0013
#define CMD_PTP_CFG_UNICAST     0x0014

#define CMD_NTP_CFG_NORMAL      0x0020
#define CMD_NTP_CFG_MD5_ENABLE  0x0021
#define CMD_NTP_CFG_MD5_KEY     0x0022
#define CMD_NTP_CFG_BLACKLIST   0x0023
#define CMD_NTP_CFG_WHITELSIT   0x0024

#define CONFIG_FILE_SIZE 8096
#define LINE_LENGTH      200
#define LINE_COUNT       400
#define STRING_LENGTH    100

struct Head_Frame
{
    char h1;
    char h2;
    char saddr;
    char daddr;
    short index;
    char ctype;
    char pad_type;
    int length;
};


struct Discovery_Frame
{
    char h1;
    char h2;
    char saddr;
    char daddr;
    short index;
    char ctype;
    char end1;
    char end2;
};

struct Response_Frame
{
    int ipaddr;
    int port;
};

struct Req_Frame
{
    short cmd;
};

void msgUpPackHead(char *buf,struct Head_Frame *pHead)
{
    struct Head_Frame *ptmpHead = (struct Head_Frame *)buf;
    pHead->h1 = ptmpHead->h1;
    pHead->h2 = ptmpHead->h2;
    pHead->saddr = ptmpHead->saddr;
    pHead->daddr = ptmpHead->daddr;
    pHead->index = ntohs(ptmpHead->index);
    pHead->ctype = ptmpHead->ctype;
    pHead->pad_type = ptmpHead->pad_type;
    pHead->length = ntohl(ptmpHead->length);
}


void msgPackHead(struct Head_Frame *oHead,char saddr,char daddr,short index
    ,char ctype,char pad_type,int length)
{
    oHead->h1 = '$';
    oHead->h2 = '<';
    oHead->saddr =  saddr;
    oHead->daddr = daddr;
    oHead->index = htons(index);
    oHead->ctype = ctype;
    oHead->pad_type = pad_type;
    oHead->length = htonl(length);
}

int msgPackFrame(char *buf,struct Head_Frame *iHead,void *sendMsg,int msglen)
{
    int iOffset = 0;
    memcpy(buf+iOffset,iHead,sizeof(struct Head_Frame));
    iOffset += sizeof(struct Head_Frame);
    memcpy(buf+iOffset,sendMsg,msglen);
    iOffset += msglen;
    buf[iOffset++] = 0x0d;
    buf[iOffset++] = 0x0a;

    return iOffset;
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
			//printf("i=%d,k=%d\n",i,k);
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
				//PLOG("k2=%c,i=%d,k=%d\n",tsrc[0],i,k);
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
				//PLOG("k1=%c,i=%d,k=%d\n",tsrc[0],i,k);
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
				//PLOG("k2=%c,i=%d,k=%d\n",tsrc[1],i,k);
				return 0;
			}
		}
		tnum=k1*0x10+k2;
		if(tnum < 0 || tnum > 255)
		{
			//PLOG("i=%d,tnum=%d\n",i,tnum);
			return 0;
		}
		tmac[i]=tnum&0xff;
		tok=tok+k;
		
	}
	
	return 1;		
}

void AnalysePtpConfigFile(Uint8* pBuf,struct PtpSetCfg *pPtpSetcfg)
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
                pPtpSetcfg->clockType = value;
        }
         if(memcmp("ptpengine:domainFilterSwitch",tile,strlen("ptpengine:domainFilterSwitch")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpSetcfg->domainFilterSwitch= value;
        }
        if(memcmp("ptpengine:domainNumber",tile,strlen("ptpengine:domainNumber")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 255)
                pPtpSetcfg->domainNumber = value;
        }
        if(memcmp("ptpengine:protoType",tile,strlen("ptpengine:protoType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpSetcfg->protoType = value;
        }
        if(memcmp("ptpengine:modeType",tile,strlen("ptpengine:modeType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpSetcfg->modeType = value;
        }
        if(memcmp("ptpengine:transmitDelayType",tile,strlen("ptpengine:transmitDelayType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpSetcfg->transmitDelayType = value;
        }
        if(memcmp("ptpengine:stepType",tile,strlen("ptpengine:stepType")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpSetcfg->stepType = value;
        }
        if(memcmp("ptpengine:UniNegotiationEnable",tile,strlen("ptpengine:UniNegotiationEnable")) == 0)
        {
            value = atoi(pIndex);
            if(value == 0 || value == 1)
                pPtpSetcfg->UniNegotiationEnable = value;
        }
        
        if(memcmp("ptpengine:validServerNum",tile,strlen("ptpengine:validServerNum")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 10)
                pPtpSetcfg->validServerNum = value;
        }
        if(memcmp("ptpengine:synFreq",tile,strlen("ptpengine:synFreq")) == 0)
        {
            value = atoi(pIndex);
            if(value >= -8 && value <= 4)
            {
                pPtpSetcfg->logSyncInterval = value;
            }
        }
        if(memcmp("ptpengine:announceFreq",tile,strlen("ptpengine:announceFreq")) == 0)
        {
            value = atoi(pIndex);
            if(value >= -8 && value <= 4)
            {
                pPtpSetcfg->logAnnounceInterval= value;
            }

        }
        if(memcmp("ptpengine:delayreqFreq",tile,strlen("ptpengine:delayreqFreq")) == 0)
        {
            value = atoi(pIndex);
            if(value >= -8 && value <= 4)
            {
                pPtpSetcfg->logMinDelayReqInterval= value;
            }

        }

        if(memcmp("ptpengine:pdelayreqFreq",tile,strlen("ptpengine:pdelayreqFreq")) == 0)
        {
            value = atoi(pIndex);
            if(value >= -8 && value <= 4)
            {
                pPtpSetcfg->logMinPdelayReqInterval = value;
            }

        }


        if(memcmp("ptpengine:UnicastDuration",tile,strlen("ptpengine:UnicastDuration")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 10 && value <= 1000)
            {
                pPtpSetcfg->UnicastDuration = value;
            }
        }
        if(memcmp("ptpengine:currentUtcOffset",tile,strlen("ptpengine:currentUtcOffset")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 255)
            {
                pPtpSetcfg->currentUtcOffset = value;
            }

        }
        if(memcmp("ptpengine:grandmasterPriority1",tile,strlen("ptpengine:grandmasterPriority1")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 255)
            {
                pPtpSetcfg->grandmasterPriority1 = value;
            }

        }
        if(memcmp("ptpengine:grandmasterPriority2",tile,strlen("ptpengine:grandmasterPriority2")) == 0)
        {
            value = atoi(pIndex);
            if(value >= 0 && value <= 255)
            {
                pPtpSetcfg->grandmasterPriority2 = value;
            }

        }
        if(memcmp("ptpengine:ServerIp_0",tile,strlen("ptpengine:ServerIp_0")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[0].serverIp = inet_addr(pIndex);
            }
            
        }
        if(memcmp("ptpengine:ServerMac_0",tile,strlen("ptpengine:ServerMac_0")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[0].serverMac);
        }


        if(memcmp("ptpengine:ServerIp_1",tile,strlen("ptpengine:ServerIp_1")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[1].serverIp = inet_addr(pIndex);
            }
        }
        if(memcmp("ptpengine:ServerMac_1",tile,strlen("ptpengine:ServerMac_1")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[1].serverMac);

        }

        if(memcmp("ptpengine:ServerIp_2",tile,strlen("ptpengine:ServerIp_2")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[2].serverIp = inet_addr(pIndex);

            }
        }
        if(memcmp("ptpengine:ServerMac_2",tile,strlen("ptpengine:ServerMac_2")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[2].serverMac);

        }


        if(memcmp("ptpengine:ServerIp_3",tile,strlen("ptpengine:ServerIp_3")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[3].serverIp = inet_addr(pIndex);

            }
        }
        if(memcmp("ptpengine:ServerMac_3",tile,strlen("ptpengine:ServerMac_3")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[3].serverMac);
        }


        if(memcmp("ptpengine:ServerIp_4",tile,strlen("ptpengine:ServerIp_4")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[4].serverIp = inet_addr(pIndex);

            }
        }
        if(memcmp("ptpengine:ServerMac_4",tile,strlen("ptpengine:ServerMac_4")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[4].serverMac);

        }


        if(memcmp("ptpengine:ServerIp_5",tile,strlen("ptpengine:ServerIp_5")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[5].serverIp = inet_addr(pIndex);
            }
        }
        if(memcmp("ptpengine:ServerMac_5",tile,strlen("ptpengine:ServerMac_5")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[5].serverMac);
        }


        if(memcmp("ptpengine:ServerIp_6",tile,strlen("ptpengine:ServerIp_6")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[6].serverIp = inet_addr(pIndex);
            }
        }
        if(memcmp("ptpengine:ServerMac_6",tile,strlen("ptpengine:ServerMac_6")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[5].serverMac);   
        }

        if(memcmp("ptpengine:ServerIp_7",tile,strlen("ptpengine:ServerIp_7")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[7].serverIp = inet_addr(pIndex);

            }
        }
        if(memcmp("ptpengine:ServerMac_7",tile,strlen("ptpengine:ServerMac_7")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[7].serverMac);

        }


        if(memcmp("ptpengine:ServerIp_8",tile,strlen("ptpengine:ServerIp_8")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[8].serverIp = inet_addr(pIndex);

            }
        }
        if(memcmp("ptpengine:ServerMac_8",tile,strlen("ptpengine:ServerMac_8")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[8].serverMac);

        }


        if(memcmp("ptpengine:ServerIp_9",tile,strlen("ptpengine:ServerIp_9")) == 0)
        {
            if(VerifyIpAddress(pIndex))
            {
                pPtpSetcfg->serverList[9].serverIp = inet_addr(pIndex);

            }
        }
        if(memcmp("ptpengine:ServerMac_9",tile,strlen("ptpengine:ServerMac_9")) == 0)
        {
            VerifyMacAddress(pIndex,pPtpSetcfg->serverList[9].serverMac);

        }

       
    }

}

int Load_PtpParam_FromFile(struct PtpSetCfg *pPtpSetcfg,char *fileName)
{
    Uint8 ConfigFileBuf[CONFIG_FILE_SIZE];
    int charactor;
    int i;


    /**只读打开文件  */
    FILE  *ptp_fd = fopen(fileName,"a+");
    if(ptp_fd == NULL)
    {
        printf("Can not Find ptp.conf file !\n");
        return -1;
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
        return -1;
    }
        

    /**分析文件  */
    AnalysePtpConfigFile(ConfigFileBuf,pPtpSetcfg);
    
}


int Save_PtpParam_ToFile(struct PtpSetCfg *pPtpSetcfg,char *fileName)
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
    str = pPtpSetcfg->clockType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","clockType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->domainNumber;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","domainNumber",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->domainFilterSwitch;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","domainFilterSwitch",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->protoType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","protoType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->modeType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","modeType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->transmitDelayType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","transmitDelayType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->stepType;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","stepType",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->UniNegotiationEnable;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","UniNegotiationEnable",str);
    fputs(line_str,ptp_fd);
        
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->logSyncInterval;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","synFreq",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->logAnnounceInterval;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","announceFreq",str);
    fputs(line_str,ptp_fd);
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->logMinDelayReqInterval;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","delayreqFreq",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->logMinDelayReqInterval;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","pdelayreqFreq",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->grandmasterPriority1;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","grandmasterPriority1",str);
    fputs(line_str,ptp_fd);

    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->grandmasterPriority2;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","grandmasterPriority2",str);
    fputs(line_str,ptp_fd);


    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->validServerNum;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","validServerNum",str);
    fputs(line_str,ptp_fd); 
    
    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->UnicastDuration;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","UnicastDuration",str);
    fputs(line_str,ptp_fd); 

    memset(line_str,0,sizeof(line_str));
    str = pPtpSetcfg->currentUtcOffset;
    sprintf(line_str,"%s:%s=%d\n","ptpengine","currentUtcOffset",str);
    fputs(line_str,ptp_fd); 


    for(i=0;i<pPtpSetcfg->validServerNum;i++)
    {
        memset(line_str,0,sizeof(line_str));
        temsock.sin_addr.s_addr = pPtpSetcfg->serverList[i].serverIp;
        sprintf(line_str,"%s:%s%d=%s\n","ptpengine","ServerIp_",i,inet_ntoa(temsock.sin_addr));
        fputs(line_str,ptp_fd); 
        
        memset(line_str,0,sizeof(line_str));
        sprintf(line_str,"%s:%s%d=%02x:%02x:%02x:%02x:%02x:%02x\n","ptpengine","ServerMac_"
                           ,i
                           ,pPtpSetcfg->serverList[i].serverMac[0]
                           ,pPtpSetcfg->serverList[i].serverMac[1]
                           ,pPtpSetcfg->serverList[i].serverMac[2]
                           ,pPtpSetcfg->serverList[i].serverMac[3]
                           ,pPtpSetcfg->serverList[i].serverMac[4]
                           ,pPtpSetcfg->serverList[i].serverMac[5]);
        fputs(line_str,ptp_fd); 

    }
    
    fflush(ptp_fd);
    fclose(ptp_fd);

    return TRUE;
}


void handle_discovery_message(struct root_data *pRootData,char *buf,int len)
{
    struct Head_Frame msgHead;
    struct Response_Frame responseMsg;
    char sendBuf[SEND_BUFF_LENGTH];
    int sendlen;
    
    if(buf[0] != '$' && buf[1] != '<')
        return;

    if(buf[6] != CTL_WORD_DISCOVERY)
        return;

    struct Discovery_Frame *pDiscoveryFrame = (struct Discovery_Frame *)buf;
    
    msgPackHead(&msgHead,pDiscoveryFrame->daddr,pDiscoveryFrame->saddr
        ,ntohs(pDiscoveryFrame->index),CTL_WORD_RESPONSE,0x01,8);

    responseMsg.ipaddr = htonl(pRootData->dev[ENUM_PC_DISCOVER].net_attr.ip);
    responseMsg.port  = htonl(pRootData->dev[ENUM_PC_DISCOVER].net_attr.sin_port);

    memset(sendBuf,0,SEND_BUFF_LENGTH);
    sendlen = msgPackFrame(sendBuf,&msgHead,&responseMsg,sizeof(responseMsg));
    
    AddData_ToSendList(pRootData,ENUM_PC_DISCOVER,sendBuf,sendlen);
    
}

void process_req_ptp_cfg_all(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        

    }
    else
    {
        
    }
    
}

void process_pc_ctl_req(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *buf)
{
    int iOffset = 0;
    struct Req_Frame *pReqFrame = (struct Req_Frame *)(buf+HEAD_FRAME_LENGTH);
    short cmd_type = ntohs(pReqFrame->cmd);


    switch(cmd_type)
    {

     case CMD_DEV_INFORMATION:
        break;
     case CMD_NET_WORK_ADDRESS:
        break;
     case CMD_VERSION_INFROMATION:
        break;
     case CMD_GPS_STATUS:
        break;
     case CMD_SYS_SET:
        break;
     case CMD_PTP_CFG_ALL:
        
        break;
     case CMD_NTP_CFG_NORMAL:
        break;
     case CMD_NTP_CFG_BLACKLIST:
        break;
     case CMD_NTP_CFG_WHITELSIT:
        break;
       
    }

}

void process_pc_ctl_set(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *buf)
{
    int iOffset = 0;
    struct Req_Frame *pReqFrame = (struct Req_Frame *)(buf+HEAD_FRAME_LENGTH);
    short cmd_type = ntohs(pReqFrame->cmd);

    switch(cmd_type)
    {

    case CMD_SYS_SET:
       break;
    case CMD_PTP_CFG_ALL:
       break;
    case CMD_PTP_CFG_NORMAL:
       break;
    case CMD_PTP_CFG_SLAVE:
       break;
    case CMD_PTP_CFG_MASTER:
       break;
    case CMD_PTP_CFG_UNICAST:
       break;
    case CMD_NTP_CFG_NORMAL:
       break;
    case CMD_NTP_CFG_MD5_ENABLE:
       break;
    case CMD_NTP_CFG_MD5_KEY:
       break;
    case CMD_NTP_CFG_BLACKLIST:
       break;
    case CMD_NTP_CFG_WHITELSIT:
       break;       
    }

}

void handle_pc_ctl_message(struct root_data *pRootData,char *buf,int len)
{
    struct Head_Frame msgHead;
    
    if(buf[0] != '$' && buf[1] != '<')
        return;

    msgUpPackHead(buf,&msgHead);

    switch(msgHead.ctype)
    {
        case CTL_WORD_REQ:
            process_pc_ctl_req(pRootData,&msgHead,buf);
            
            break;
        case CTL_WORD_ACK:
            break;
        case CTL_WORD_DATA:
            break;
        case CTL_WORD_SET:
            process_pc_ctl_set(pRootData,&msgHead,buf);
            break;
        case CTL_WORD_NAK:
            break;
        default:
            break;
         
    }
    
}

