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

#define MD5_ENABLE (0x01)
#define MD5_DISABLE (0x00)

#define TOTAL_KEY_NO  8
#define ENTER_CHAR 0x0a

#define	LEAP_NOWARNING	0x0	/* normal, no leap second warning */
#define	LEAP_ADDSECOND	0x1	/* last minute of day has 61 seconds */
#define	LEAP_DELSECOND	0x2	/* last minute of day has 59 seconds */
#define	LEAP_NOTINSYNC	0x3	/* overload, clock is free running */

#define STRATUM_0_PRESION -20
#define STRATUM_1_PRESION -20
#define STRATUM_2_PRESION -18
#define STRATUM_3_PRESION -15


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

Uint32 CaculateSecond(Uint32 input)
{
    Uint16 i;
    Uint32 second =1;

    for(i = 0;i<input;i++)
        second = second * 2;

    return second;
}

Uint32 Caculatefrequency(Uint32 input)
{
    Uint32 i = 0;
    while(input)
    {
        input = input / 2;
        i++;
    }
    return i -1;
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


void Load_BlackList(struct NtpSetCfg *pNtpSetcfg,Uint8 pData[200][200],int index)
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
        printf("Blacklist open\n");
        pNtpSetcfg->blacklist = TRUE;
    }
    else if(strcmp(pData[iLine],"#close") == 0)
    {
        printf("Blacklist close\n");
        pNtpSetcfg->blacklist = FALSE;
    }
    
    iLine = iLine+1;
    if(strcmp(pData[iLine],"#end") == 0)
    {
        printf("load blacklist NULL\n");
        return;
    }
    
    while(strcmp(pData[iLine+i],"#end") != 0)
    {
        memset(title,0,sizeof(title));
        memset(ip,0,sizeof(ip));
        memset(mask_str,0,sizeof(mask_str));
        memset(mask,0,sizeof(mask));
        memset(other,0,sizeof(other));
        if(pNtpSetcfg->blacklist == TRUE)
        {
            
            sscanf(pData[iLine+i],"%s %s %s %s %s",title,ip,mask_str,mask,other);
        }

        else
        {
            sscanf(pData[iLine+i],"%c %s %s %s %s %s",&flag,title,ip,mask_str,mask,other);
        }
        
        pNtpSetcfg->blacklist_ip[i] = inet_addr(ip);
        pNtpSetcfg->blacklist_mask[i] = inet_addr(mask);
        pNtpSetcfg->blacklist_flag[i] = TRUE;
        
        i++;
    }
    
}

void Load_WhiltList(struct NtpSetCfg *pNtpSetcfg,Uint8 pData[200][200],int index)
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
        printf("Whitlelist open\n");
        pNtpSetcfg->whitelist = TRUE;
    }
    else if(strcmp(pData[iLine],"#close") == 0)
    {
        printf("Whitlelist close\n");
        pNtpSetcfg->whitelist = FALSE;
    }

    iLine = iLine+1;
    if(strcmp(pData[iLine],"#end") == 0)
    {
        printf("load whithlist NULL\n");
        return;
    }
    
    while(strcmp(pData[iLine+i],"#end") != 0)
    {
        
        memset(title,0,sizeof(title));
        memset(ip,0,sizeof(ip));
        memset(mask_str,0,sizeof(mask_str));
        memset(mask,0,sizeof(mask));
        memset(other,0,sizeof(other));
        
        if(pNtpSetcfg->whitelist == TRUE)
        {
            sscanf(pData[iLine+i],"%s %s %s %s %s",title,ip,mask_str,mask,other);
        }
        else
        {
            sscanf(pData[iLine+i],"%c %s %s %s %s %s",&flag,title,ip,mask_str,mask,other);
        }
        
        pNtpSetcfg->whitelist_ip[i] = inet_addr(ip);
        pNtpSetcfg->whitelist_mask[i] = inet_addr(mask);
        pNtpSetcfg->whitelist_flag[i] = TRUE;
        
        i++;
    }

}

void Load_BroadCast(struct NtpSetCfg *pNtpSetcfg,Uint8 pData[200][200],int index)
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
        pNtpSetcfg->broadcast = FALSE;
        return;
    }

    pNtpSetcfg->broadcast = TRUE;
    
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
    
    pNtpSetcfg->broadcast_key_num = key_num;
    pNtpSetcfg->freq_b = CaculateSecond(poll_num);
    
    printf("%s %s %s %d %s %d\n",title,ip,key,key_num,poll,poll_num);
    iLine++;
    
}

void Load_MultiCast(struct NtpSetCfg *pNtpSetcfg,Uint8 pData[200][200],int index)
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
        pNtpSetcfg->multicast = FALSE;
        return;
    }
    
    pNtpSetcfg->multicast = TRUE;
    
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
    
    pNtpSetcfg->multicast_key_num = key_num;
    pNtpSetcfg->freq_m = CaculateSecond(poll_num);
        
    printf("%s %s %s %d %s %d\n",title,ip,key,key_num,poll,poll_num);
    
    iLine++;

}

static void Load_ServerAddr(struct NtpSetCfg *pNtpSetcfg,Uint8 pData[200][200],int index)
{
    Uint16 iLine;
    Uint16 i = 0;

    Uint8  title[50];
    Uint8  stratum;
    Uint8  refid[50];


    iLine = index+1;
    if(strcmp(pData[iLine],"#end") == 0)
    {
        printf("Server close\n");
        return;
    }
    //g_Ntp_Parameter.sympassive = TRUE;

}

void Load_Keys(struct NtpSetCfg *pNtpSetcfg,Uint8 pData[200][200],int index)
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
    
    if(strcmp(pData[iLine],"#end") == 0)
    {
        printf("key do not open\n");
        pNtpSetcfg->md5_flag = pNtpSetcfg->md5_flag & MD5_DISABLE;
        return;
    }
    pNtpSetcfg->md5_flag = pNtpSetcfg->md5_flag | MD5_ENABLE;
    
    for(i = 0;i < 5; i++)
    {
        memset(TrustKey,0,sizeof(TrustKey));
        memcpy(TrustKey,pData[i+iLine],10);

        if(strcmp("trustedkey",TrustKey) == 0)
        {
            strtok(pData[i+iLine]," ");
            while(pChoice = strtok(NULL," "))
           {
                printf("find trust key %d\n",*pChoice);
                pNtpSetcfg->current_key[*pChoice-0x30].key_valid = TRUE;
           }
        }
    }
    
}



void AnalyseNtpdConfigFile(struct NtpSetCfg *pNtpSetcfg,Uint8* pBuf)
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
            
            Load_BlackList(pNtpSetcfg,Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#whiltlist") == 0)
        {
            Load_WhiltList(pNtpSetcfg,Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#broadcast") == 0)
        {
            Load_BroadCast(pNtpSetcfg,Data,i);
        }
    }

    
    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#multicast") == 0)
        {
            Load_MultiCast(pNtpSetcfg,Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#server") == 0)
        {
            Load_ServerAddr(pNtpSetcfg,Data,i);
        }
    }

    for(i = 0;i < 200;i++)
    {
        if(strcmp(Data[i],"#keys") == 0)
        {
            Load_Keys(pNtpSetcfg,Data,i);
        }
    }

}


Uint8 Read_Md5FromKeyFile(struct NtpSetCfg *pNtpSetcfg,char *fileName)
{
    Uint16 i;
    Uint8 Data[50];
    Uint8 num;
    Uint8 type;
    Uint8 key_str[20];
    FILE *md5_file_fd = fopen(fileName,"r");
    if(md5_file_fd == NULL)
    {
        printf("can not find md5 file\n");
        return -1;
    }
    
    i = 1;
    memset(Data,0,sizeof(Data));
    
    while(fgets(Data,sizeof(Data),md5_file_fd))
    {
        memset(key_str,0,sizeof(key_str));

        sscanf(Data,"%c %c %s",&num,&type,key_str);
        memcpy(pNtpSetcfg->current_key[i].key,key_str,20);
        pNtpSetcfg->current_key[i].key_length = strlen(key_str);
        pNtpSetcfg->current_key[i].key_valid = TRUE;
        
        memset(Data,0,sizeof(Data));
        i++;
    }
    
     
    fclose(md5_file_fd);
}


void Load_NtpdParam_FromFile(struct NtpSetCfg *pNtpSetcfg,char *ntpCfgFile,char *md5CfgFile)
{

    Uint16 i;
    Uint8 Data[50];
    Uint8 Keys[50];

    Uint8 ConfigFileBuf[CONFIG_FILE_SIZE];
    int charactor;
    
    FILE *ntp_cfg_fp = fopen(ntpCfgFile,"r");
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

    Read_Md5FromKeyFile(pNtpSetcfg,md5CfgFile);
    
    AnalyseNtpdConfigFile(pNtpSetcfg,ConfigFileBuf);

    fclose(ntp_cfg_fp);
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

Uint8 Save_Md5_ToKeyFile(struct NtpSetCfg *pNtpSetcfg,char *fileName)
{
    Uint16 i;
    Uint8 Data[50];
    
    FILE *md5_file_fd = fopen(fileName,"w+");
    if(md5_file_fd == NULL)
    {
        printf("can not find md5 file\n");
        return -1;
    }

    for(i = 1;i <= TOTAL_KEY_NO; i++)
    {
        memset(Data,0,sizeof(Data));
        if(pNtpSetcfg->current_key[i].key_length == 0)
        {
            continue;
        }

        sprintf(Data,"%d %c %s\n",i,'M',pNtpSetcfg->current_key[i].key);

        fputs(Data,md5_file_fd);
    }
    fflush(md5_file_fd);
    fclose(md5_file_fd);
    return TRUE;
}

static void Save_BlackList_Open(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
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
        if(pNtpSetcfg->blacklist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = pNtpSetcfg->blacklist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = pNtpSetcfg->blacklist_mask[i];
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

static void Save_BlackList_Close(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
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
        if(pNtpSetcfg->blacklist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = pNtpSetcfg->blacklist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = pNtpSetcfg->blacklist_mask[i];
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

static void Save_BlackList(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    
    struct sockaddr_in temsock;
    
    Uint8 line_str[200];
    Uint8 ip[20];
    Uint8 mask[20];


    for(i = 0;i < 16;i++)
    {
        if(pNtpSetcfg->blacklist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = pNtpSetcfg->blacklist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = pNtpSetcfg->blacklist_mask[i];
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

static void Save_WhiltList_Open(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
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
        if(pNtpSetcfg->whitelist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = pNtpSetcfg->whitelist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = pNtpSetcfg->whitelist_mask[i];
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

static void Save_WhiltList_Close(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
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
        if(pNtpSetcfg->whitelist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = pNtpSetcfg->whitelist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = pNtpSetcfg->whitelist_mask[i];
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

static void Save_WhiltList(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    
    struct sockaddr_in temsock;
    
    Uint8 line_str[200];
    Uint8 ip[20];
    Uint8 mask[20];
    
    for(i = 0;i < 16;i++)
    {
        if(pNtpSetcfg->whitelist_flag[i] == TRUE)
        {
            memset(line_str,0,sizeof(line_str));
            memset(ip,0,sizeof(ip));
            memset(mask,0,sizeof(mask));
            
            temsock.sin_addr.s_addr = pNtpSetcfg->whitelist_ip[i];
            memcpy(ip,inet_ntoa(temsock.sin_addr),strlen(inet_ntoa(temsock.sin_addr)));

            temsock.sin_addr.s_addr = pNtpSetcfg->whitelist_mask[i];
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

static void Save_BroadCast(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    Uint8 line_str[200];
    struct sockaddr_in temsock;
    Uint32 iMask;
    Uint32 broadcast_ip;
    Uint32 freq = Caculatefrequency(pNtpSetcfg->freq_b);
        
    memset(line_str,0,sizeof(line_str));

    /*计算广播地址*/
    iMask = ~pNtpSetcfg->runconf.mask;
    broadcast_ip = pNtpSetcfg->runconf.ip | iMask;
    temsock.sin_addr.s_addr = broadcast_ip;

    if(pNtpSetcfg->broadcast_key_num == 0)
    {
        sprintf(line_str,"%s %s %s %s %d","broadcast",inet_ntoa(temsock.sin_addr),"autokey","minpoll",freq);
    }
    else
    {
        sprintf(line_str,"%s %s %s %d %s %d","broadcast",inet_ntoa(temsock.sin_addr),"key",pNtpSetcfg->broadcast_key_num,"minpoll",freq);
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
static void Save_MultiCast(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
{
    int i = 0;
    Uint16 iOffset  = *offset;
    Uint8 line_str[200];
    struct sockaddr_in temsock;
    Uint32 freq = Caculatefrequency(pNtpSetcfg->freq_m);
    
    memset(line_str,0,sizeof(line_str));

    if(pNtpSetcfg->multicast_key_num == 0)
    {
        sprintf(line_str,"%s %s %s %s %d","broadcast","224.0.1.1","autokey","minpoll",freq);
    }
    else
    {
        sprintf(line_str,"%s %s %s %d %s %d","broadcast","224.0.1.1","key",pNtpSetcfg->multicast_key_num,"minpoll",freq);
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
static void Save_ServerAddr(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
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
static void Save_keys(struct NtpSetCfg *pNtpSetcfg,Uint8 *Buf,Uint16 *offset)
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
        if(pNtpSetcfg->current_key[i].key_valid)
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

void Save_NtpParm_ToFile(struct NtpSetCfg *pNtpSetcfg,char *fileName)
{
    Uint16 i;
    Uint16 iOffset;
    Uint8 Data[50];
    Uint8 Keys[50];
    Uint8 ConfigFileBuf[CONFIG_FILE_SIZE];

    FILE *ntp_cfg_fp = fopen(fileName,"w+");
    if(ntp_cfg_fp == NULL)
    {
        printf("can not find config file\n");
        return;
    }

    memset(ConfigFileBuf,0,CONFIG_FILE_SIZE);
    
    iOffset = 0;

    /*写入限制*/
    if((pNtpSetcfg->whitelist == TRUE))
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
    if(pNtpSetcfg->blacklist == TRUE)
    {
        Save_BlackList_Open(pNtpSetcfg,ConfigFileBuf,&iOffset);
    }
    else
    {
        Save_BlackList_Close(pNtpSetcfg,ConfigFileBuf,&iOffset);
    }
    
    
    /*写入白名单*/  
    if(pNtpSetcfg->whitelist == TRUE)
    {
        Save_WhiltList_Open(pNtpSetcfg,ConfigFileBuf,&iOffset);
    }
    else
    {
        Save_WhiltList_Close(pNtpSetcfg,ConfigFileBuf,&iOffset);
    }

    /*写入广播*/
    memcpy(ConfigFileBuf + iOffset,"#broadcast",strlen("#broadcast"));
    iOffset += strlen("#broadcast");
    ConfigFileBuf[iOffset++] = ENTER_CHAR;
    
    if(pNtpSetcfg->broadcast == TRUE)
    {
        Save_BroadCast(pNtpSetcfg,ConfigFileBuf,&iOffset);
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
    
    if(pNtpSetcfg->multicast == TRUE)
    {
        Save_MultiCast(pNtpSetcfg,ConfigFileBuf,&iOffset);
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
    Save_ServerAddr(pNtpSetcfg,ConfigFileBuf,&iOffset);
    
    #if 0
    if(g_Ntp_Parameter.out_block == 0)
    {
        Save_ServerAddr(ConfigFileBuf,&iOffset);
    }
    else
    {
        memcpy(ConfigFileBuf + iOffset,"#end",strlen("#end"));
        iOffset += strlen("#end");
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
    } 
    #endif
    
    /*写入md 5 */
    memcpy(ConfigFileBuf + iOffset,"#keys",strlen("#keys"));
    iOffset += strlen("#keys");
    ConfigFileBuf[iOffset++] = ENTER_CHAR;
    
    if((pNtpSetcfg->md5_flag & MD5_ENABLE) == MD5_ENABLE)
    {
        Save_keys(pNtpSetcfg,ConfigFileBuf,&iOffset);
    }
    else
    {
        memcpy(ConfigFileBuf + iOffset,"#end",strlen("#end"));
        iOffset += strlen("#end");
        ConfigFileBuf[iOffset++] = ENTER_CHAR;
    }


#if 0
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
    
#endif
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

