#include "manager.h"
#include "log.h"

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
#define CMD_NET_WORK_CTL        0x0002
#define CMD_VERSION_INFROMATION 0x0003
#define CMD_GPS_STATUS          0x0004
#define CMD_SYS_SET             0x0005
#define CMD_NET_WORK_PTP        0x0006
#define CMD_NET_WORK_NTP        0x0007
#define CMD_RESET_SYS           0x0008

#define CMD_PTP_CFG_ALL         0x0010
#define CMD_PTP_CFG_NORMAL      0x0011
#define CMD_PTP_CFG_SLAVE       0x0012
#define CMD_PTP_CFG_MASTER      0x0013


#define CMD_NTP_CFG_NORMAL      0x0020
#define CMD_NTP_CFG_MD5_ENABLE  0x0021
#define CMD_NTP_CFG_BLACKLIST   0x0022
#define CMD_NTP_CFG_WHITELSIT   0x0023

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


#define FACTORY_DEV_CODE "QH000001"
#define DEV_TYPE_CODE    "QH000001"
#define SOFT_VERSION     "0.01"
#define FPGA_VERSION     "0.03"

struct Md5key factory_key[10]={ 
                          {0,0,""},
                          {1,16,"0123456789abcdef"},
						  {1,16,"123456789abcdef0"},
						  {1,16,"23456789abcdef01"},
						  {1,16,"3456789abcdef012"},
						  {1,16,"okdidflg_+|-=;'d"},
						  {1,16,"wk3k4kflv,f;ro&"},
						  {1,16,">dfwertrtsdf90*&"},
						  {1,10,"*&^%$45678"},
						 };



void msgUpPackHead(char *buf,struct Head_Frame *pHead)
{
    pHead->h1 = *(char *)(buf + 0);
    pHead->h2 = *(char *)(buf + 1);
    pHead->saddr = *(char *)(buf + 2);
    pHead->daddr = *(char *)(buf + 3);
    pHead->index = ntohs(*(short *)(buf + 4));
    pHead->ctype = *(char *)(buf + 6);;
    pHead->pad_type = *(char *)(buf + 7);;
    pHead->length = ntohl(*(int *)(buf + 8));
}


void msgUpPack_ptp_all(char *data,struct PtpSetCfg *pPtpSetcfg)
{
        
    int i;
    int iOffset = 0;
    
    //pPtpSetcfg->clockType = data[iOffset++];
    pPtpSetcfg->domainNumber = data[iOffset++];
    pPtpSetcfg->domainFilterSwitch = data[iOffset++];
    pPtpSetcfg->protoType = data[iOffset++];
    pPtpSetcfg->modeType = data[iOffset++];
    pPtpSetcfg->transmitDelayType = data[iOffset++];
    pPtpSetcfg->stepType = data[iOffset++];
    pPtpSetcfg->UniNegotiationEnable = data[iOffset++];
    pPtpSetcfg->logSyncInterval = data[iOffset++];
    pPtpSetcfg->logAnnounceInterval = data[iOffset++];
    pPtpSetcfg->logMinDelayReqInterval = data[iOffset++];
    pPtpSetcfg->logMinPdelayReqInterval = data[iOffset++];
    pPtpSetcfg->grandmasterPriority1 = data[iOffset++];
    pPtpSetcfg->grandmasterPriority2 = data[iOffset++];
    pPtpSetcfg->validServerNum = data[iOffset++];

    pPtpSetcfg->UnicastDuration = flip32(*(Uint32 *)(data+iOffset));
    iOffset += 4;
    
    pPtpSetcfg->currentUtcOffset = data[iOffset++];
    

    for(i = 0;i < 10;i++)
    {
       pPtpSetcfg->serverList[i].serverIp = *(Uint32 *)(data+iOffset);
       iOffset += 4;
    }

    for(i = 0;i < 10;i++)
    {
        memcpy(pPtpSetcfg->serverList[i].serverMac,(data+iOffset),6);
        iOffset += 6;
    }

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

int msgPackRespFrameToSend(struct root_data *pRootData,struct Head_Frame *iHead,short msgType,void *sendMsg,int msglen,short subcmd)
{
    int iOffset;
    struct Head_Frame s;
    char buf[1024];

    memset(buf,0,sizeof(buf));
    iOffset = 0;
    
    msgPackHead(&s,iHead->daddr,iHead->saddr,iHead->index,msgType,iHead->pad_type,msglen);
    memcpy(buf+iOffset,&s,sizeof(struct Head_Frame));
    iOffset += sizeof(struct Head_Frame);

    buf[iOffset++] = (subcmd>>8)& 0xff;
    buf[iOffset++] = subcmd & 0xff;
    
    if(sendMsg != NULL)
    {
        memcpy(buf+iOffset,sendMsg,msglen);
        iOffset += msglen;
    }

    buf[iOffset++] = 0x0d;
    buf[iOffset++] = 0x0a;

    AddData_ToSendList(pRootData,ENUM_PC_CTL,buf,iOffset);

    return iOffset;
}


int msgPackFrameToSend(struct root_data *pRootData,struct Head_Frame *iHead,short msgType,void *sendMsg,int msglen)
{
    int iOffset;
    struct Head_Frame s;
    char buf[1024];

    memset(buf,0,sizeof(buf));
    iOffset = 0;
    
    msgPackHead(&s,iHead->daddr,iHead->saddr,iHead->index,msgType,iHead->pad_type,msglen);
    memcpy(buf+iOffset,&s,sizeof(struct Head_Frame));
    iOffset += sizeof(struct Head_Frame);

    if(sendMsg != NULL)
    {
        memcpy(buf+iOffset,sendMsg,msglen);
        iOffset += msglen;
    }

    buf[iOffset++] = 0x0d;
    buf[iOffset++] = 0x0a;

    AddData_ToSendList(pRootData,ENUM_PC_CTL,buf,iOffset);

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

#if 0

void SetRouteToEnv(char *network_cfg,struct NetInfor *infopt)
{
    Uint8 route_temp[70];
    
    struct sockaddr_in temsock;
    int iOffset =0;
    
    Uint32 ethx_fd = open(network_cfg,O_RDWR|O_CREAT|O_TRUNC,0666);
    if(ethx_fd < 0)
    {
        printf("open eth config error!\n");
        return;
    }
    
    temsock.sin_addr.s_addr= infopt->ip;
    
    lseek(ethx_fd,0,SEEK_SET);
    write(ethx_fd,"#!/bin/sh\n",10);
    
    memset(route_temp,0,70);
   //sprintf(route_temp,"%s%d\n","ip route del default table ",pPtpClock->routeTable);
    sprintf(route_temp,"%s%d\n","ip route flush table ",100);
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
    memcpy(route_temp+3,network_cfg,strlen(network_cfg);
    system(route_temp);


}
#endif

void SetNetworkToEnv(struct NetInfor *infopt)
{
    //SetMacAddress(infopt->ifaceName,infopt->mac);
    //sleep(1);

    SetIpAddress(infopt->ifaceName,infopt->ip);
    SetMaskAddress(infopt->ifaceName,infopt->mask);

    //AddGateWay(infopt->ifaceName,infopt->gwip);

}

void SetCmdNetworkToEnv(struct NetInfor *infopt)
{
    SetIpAddress(infopt->ifaceName,infopt->ip);
    SetMaskAddress(infopt->ifaceName,infopt->mask);

    AddGateWay(infopt->ifaceName,infopt->gwip);

}

void Get_Net_FormSysEvn(struct NetInfor *pNetInfor)
{
    char mac[6];
  
    GetIpAddress(pNetInfor->ifaceName,&pNetInfor->ip);
    GetMaskAddress(pNetInfor->ifaceName,&pNetInfor->mask);
    GetMacAddress(pNetInfor->ifaceName,pNetInfor->mac);
        
    GetGateWay(pNetInfor->ifaceName,&pNetInfor->gwip);
}


void SaveNetParamToFile(char *network_cfg,struct NetInfor *infopt)
{
    Uint8 line_str[200];
    struct sockaddr_in temsock;
    FILE *network_fd = fopen(network_cfg,"w+");
    
    if(network_fd == NULL)
    {
        printf("SaveNet can not find network.conf file\n");
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
int Load_comm_ip_address(char *network_cfg,char *ip_address)
{
    Uint8 line_str[50];
    
    Uint8 tile[10];
    Uint8 *pIndex;
    Uint8 *pStr = line_str;
    struct in_addr temsock;

    int i;
    
    memset(line_str,0,sizeof(line_str));
    memset(tile,0,sizeof(tile));
    
    FILE *net_work_fd = fopen(network_cfg,"r");
    if(net_work_fd == NULL)
    {
        printf("can not find ip_address.conf file\n");
        memcpy(ip_address,"127.0.0.1",strlen("127.0.0.1"));
        return -1;
    }
    
    fgets(line_str,sizeof(line_str),net_work_fd);
    pIndex = strchr(line_str,':');
    memcpy(tile,line_str,pIndex-pStr);

    pIndex++;

    if(strcmp("IP",tile) == 0)
    {
        memcpy(ip_address,pIndex,strlen(pIndex));
    }


    fclose(net_work_fd);
}

int Load_NetWorkParam_FromFile(char *network_cfg,struct NetInfor *infopt)
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
    
    FILE *net_work_fd = fopen(network_cfg,"r");
    if(net_work_fd == NULL)
    {
        printf("can not find network.conf file\n");
        Get_Net_FormSysEvn(infopt);
        SaveNetParamToFile(network_cfg,infopt);
        //SetRouteToEnv(network_cfg,infopt);
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
    //SetRouteToEnv(network_cfg,infopt);
    fclose(net_work_fd);
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
    iMask = ~0;
    broadcast_ip = 0 | iMask;
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



void handle_req_dev_infor(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }

    strcpy(buf+iOffset,FACTORY_DEV_CODE);
    iOffset += strlen(FACTORY_DEV_CODE);

    strcpy(buf+iOffset,DEV_TYPE_CODE);
    iOffset += strlen(DEV_TYPE_CODE);
    
    buf[iOffset++] = 1;
    buf[iOffset++] = pRootData->slot_list[1].slot_type;
    buf[iOffset++] = 2;
    buf[iOffset++] = pRootData->slot_list[2].slot_type;
    buf[iOffset++] = 3;
    buf[iOffset++] = pRootData->slot_list[3].slot_type;
    buf[iOffset++] = 4;
    buf[iOffset++] = pRootData->slot_list[4].slot_type;

    msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_DEV_INFORMATION);
}

void handle_req_version(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }


    memcpy(&buf[iOffset],SOFT_VERSION,4);
    iOffset += 4;
    memcpy(&buf[iOffset],FPGA_VERSION,4);
    iOffset += 4;

    msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_VERSION_INFROMATION);

}


void handle_req_network_ctl(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }

    *(int*)(buf+iOffset) = htonl(pRootData->comm_port.ip);
    iOffset += 4;
    *(int*)(buf+iOffset) = htonl(pRootData->comm_port.mask);
    iOffset += 4;
    *(int*)(buf+iOffset) = htonl(pRootData->comm_port.gwip);
    iOffset += 4;
    memcpy(buf+iOffset,pRootData->comm_port.mac,6);
    iOffset += 6;
    
    msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_NET_WORK_CTL);
}

void handle_req_network_ptp(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }

    *(int*)(buf+iOffset) = htonl(pRootData->ptp_port.ip);
    iOffset += 4;
    *(int*)(buf+iOffset) = htonl(pRootData->ptp_port.mask);
    iOffset += 4;
    *(int*)(buf+iOffset) = htonl(pRootData->ptp_port.gwip);
    iOffset += 4;
    memcpy(buf+iOffset,pRootData->ptp_port.mac,6);
    iOffset += 6;
    
    msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_NET_WORK_PTP);
}

void handle_req_network_ntp(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }

    *(int*)(buf+iOffset) = htonl(pRootData->ntp_port.ip);
    iOffset += 4;
    *(int*)(buf+iOffset) = htonl(pRootData->ntp_port.mask);
    iOffset += 4;
    *(int*)(buf+iOffset) = htonl(pRootData->ntp_port.gwip);
    iOffset += 4;
    memcpy(buf+iOffset,pRootData->ntp_port.mac,6);
    iOffset += 6;
    
    msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_NET_WORK_NTP);
}

void handle_req_gps_status(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;
    struct Satellite_Data *pSatellite = &pRootData->satellite_data;
    
    memset(buf,0,sizeof(buf));
    
    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }


    buf[iOffset++] = pSatellite->satellite_see;
    buf[iOffset++] = pSatellite->satellite_use;
    buf[iOffset++] = pSatellite->time_enable;
    buf[iOffset++] = pSatellite->gps_utc_leaps;
    buf[iOffset++] = pSatellite->leap_enable;
    buf[iOffset++] = pSatellite->leap58or60;
    buf[iOffset++] = pSatellite->eastorwest;
    buf[iOffset++] = pSatellite->northorsouth;
    buf[iOffset++] = (pSatellite->longitude_d>>16)&0xff;
    buf[iOffset++] = (pSatellite->longitude_d)&0xff;
    buf[iOffset++] = (pSatellite->longitude_f>>16)&0xff;
    buf[iOffset++] = (pSatellite->longitude_f)&0xff;
    buf[iOffset++] = (pSatellite->longitude_m>>16)&0xff;
    buf[iOffset++] = (pSatellite->longitude_m)&0xff;

    buf[iOffset++] = (pSatellite->latitude_d>>16)&0xff;
    buf[iOffset++] = (pSatellite->latitude_d)&0xff;
    buf[iOffset++] = (pSatellite->latitude_f>>16)&0xff;
    buf[iOffset++] = (pSatellite->latitude_f)&0xff;
    buf[iOffset++] = (pSatellite->latitude_m>>16)&0xff;
    buf[iOffset++] = (pSatellite->latitude_m)&0xff;
           
    msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_GPS_STATUS);
}

void handle_req_system_setting(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }


    struct clock_info *pClockInfo = &pRootData->clock_info;

    buf[iOffset++] = pClockInfo->ref_type;
    buf[iOffset++] = debug_level;
    buf[iOffset++] = 0;
    buf[iOffset++] = 0;
    
    msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_SYS_SET);
}

void packmsg_ptp_all(struct PtpSetCfg *pPtpSetcfg,char *buf,int *len)
{
    int i;
    int iOffset = *len;
    
    buf[iOffset++] = pPtpSetcfg->clockType;
    buf[iOffset++] = pPtpSetcfg->domainNumber;
    buf[iOffset++] = pPtpSetcfg->domainFilterSwitch;
    buf[iOffset++] = pPtpSetcfg->protoType;
    buf[iOffset++] = pPtpSetcfg->modeType;
    buf[iOffset++] = pPtpSetcfg->transmitDelayType;
    buf[iOffset++] = pPtpSetcfg->stepType;
    buf[iOffset++] = pPtpSetcfg->UniNegotiationEnable;
    buf[iOffset++] = pPtpSetcfg->logSyncInterval;
    buf[iOffset++] = pPtpSetcfg->logAnnounceInterval;
    buf[iOffset++] = pPtpSetcfg->logMinDelayReqInterval;
    buf[iOffset++] = pPtpSetcfg->logMinPdelayReqInterval;
    buf[iOffset++] = pPtpSetcfg->grandmasterPriority1;
    buf[iOffset++] = pPtpSetcfg->grandmasterPriority2;
    buf[iOffset++] = pPtpSetcfg->validServerNum;
    *(Uint32 *)(buf+iOffset)= flip32(pPtpSetcfg->UnicastDuration);
    iOffset += 4;
    
    buf[iOffset++] = pPtpSetcfg->currentUtcOffset;

    for(i = 0;i < 10;i++)
    {
       *(Uint32 *)(buf+iOffset) = pPtpSetcfg->serverList[i].serverIp;
       iOffset += 4;
    }

    for(i = 0;i < 10;i++)
    {
        memcpy((buf+iOffset),pPtpSetcfg->serverList[i].serverMac,6);
        iOffset += 6;
    }

    *len = iOffset;
    
}

void packmsg_ptp_nomal(struct PtpSetCfg *pPtpSetcfg,char *buf,int *len)
{
    int i;
    int iOffset = *len;
    
    buf[iOffset++] = pPtpSetcfg->clockType;
    buf[iOffset++] = pPtpSetcfg->domainNumber;
    buf[iOffset++] = pPtpSetcfg->domainFilterSwitch;
    buf[iOffset++] = pPtpSetcfg->protoType;
    buf[iOffset++] = pPtpSetcfg->modeType;
    buf[iOffset++] = pPtpSetcfg->transmitDelayType;
    buf[iOffset++] = pPtpSetcfg->stepType;

    *len = iOffset;
}

void packmsg_ptp_master(struct PtpSetCfg *pPtpSetcfg,char *buf,int *len)
{
    int i;
    int iOffset = *len;
    buf[iOffset++] = pPtpSetcfg->logSyncInterval;
    buf[iOffset++] = pPtpSetcfg->logAnnounceInterval;
    buf[iOffset++] = pPtpSetcfg->grandmasterPriority1;
    buf[iOffset++] = pPtpSetcfg->grandmasterPriority2;
    //buf[iOffset++] = pPtpSetcfg->validServerNum;
    buf[iOffset++] = pPtpSetcfg->currentUtcOffset;
    
    *len = iOffset;

}

void packmsg_ptp_slave(struct PtpSetCfg *pPtpSetcfg,char *buf,int *len)
{
    int i;
    int iOffset = *len;
    
    buf[iOffset++] = pPtpSetcfg->logMinDelayReqInterval;
    buf[iOffset++] = pPtpSetcfg->logMinPdelayReqInterval;
    buf[iOffset++] = pPtpSetcfg->UniNegotiationEnable;
    buf[iOffset++] = pPtpSetcfg->validServerNum;
    *(Uint32 *)(buf+iOffset)= flip32(pPtpSetcfg->UnicastDuration);
    iOffset += 4;

    buf[iOffset++] = pPtpSetcfg->currentUtcOffset;

    for(i = 0;i < 10;i++)
    {
       *(Uint32 *)(buf+iOffset) = pPtpSetcfg->serverList[i].serverIp;
       iOffset += 4;
    }

    for(i = 0;i < 10;i++)
    {
        memcpy((buf+iOffset),pPtpSetcfg->serverList[i].serverMac,6);
        iOffset += 6;
    }

    *len = iOffset;

}



void packmsg_ntp_normal(struct NtpSetCfg *pNtpSetCfg,char *buf,int *len)
{
    int i;
    int iOffset = *len;
    
    buf[iOffset++] = pNtpSetCfg->broadcast;
    buf[iOffset++] = pNtpSetCfg->freq_b;
    buf[iOffset++] = pNtpSetCfg->multicast;
    
    buf[iOffset++] = pNtpSetCfg->freq_m;
    buf[iOffset++] = pNtpSetCfg->md5_flag;
    //buf[iOffset++] = pNtpSetCfg->sympassive;


    for(i=1;i<9;i++)
    {
        buf[iOffset++] = pNtpSetCfg->current_key[i].key_valid;
        buf[iOffset++] = pNtpSetCfg->current_key[i].key_length;
        memcpy(buf+iOffset,pNtpSetCfg->current_key[i].key,20);
        iOffset += 20;
    }

    *len = iOffset;

}



void packmsg_ntp_md5_enable(struct NtpSetCfg *pNtpSetCfg,char *buf,int *len)
{
    int i;
    int iOffset = *len;
    
    buf[iOffset++] = pNtpSetCfg->broadcast_key_num;
    buf[iOffset++] = pNtpSetCfg->multicast_key_num;
    buf[iOffset++] = pNtpSetCfg->md5_flag;
    //buf[iOffset++] = pNtpSetCfg->sympassive;


    *len = iOffset;

}

void packmsg_ntp_blacklist(struct NtpSetCfg *pNtpSetCfg,char *buf,int *len)
{
    int i;
    int iOffset = *len;
    
    buf[iOffset++] = pNtpSetCfg->blacklist;
    buf[iOffset++] = pNtpSetCfg->blacklist_num;

    for(i=0;i<16;i++)
    {
        
        buf[iOffset++] = pNtpSetCfg->blacklist_flag[i];
        
        *(int *)(buf+iOffset) = htonl(pNtpSetCfg->blacklist_ip[i]);
        iOffset += 4;
        
        *(int *)(buf+iOffset) = htonl(pNtpSetCfg->blacklist_mask[i]);
        iOffset += 4;
    }


    *len = iOffset;

}

void packmsg_ntp_whitlelist(struct NtpSetCfg *pNtpSetCfg,char *buf,int *len)
{
    int i;
    int iOffset = *len;
    
    buf[iOffset++] = pNtpSetCfg->whitelist;
    buf[iOffset++] = pNtpSetCfg->whitelist_num;

    for(i=0;i<16;i++)
    {
        
        buf[iOffset++] = pNtpSetCfg->whitelist_flag[i];
        
        *(int *)(buf+iOffset) = htonl(pNtpSetCfg->whitelist_ip[i]);
        iOffset += 4;
        
        *(int *)(buf+iOffset) = htonl(pNtpSetCfg->whitelist_mask[i]);
        iOffset += 4;
    }


    *len = iOffset;


}


void handle_req_ptp_all(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[500];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        packmsg_ptp_all(pSlotList->pPtpSetcfg,buf,&iOffset);
        msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_PTP_CFG_ALL);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }



}

void handle_req_ptp_normal(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[500];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        packmsg_ptp_nomal(pSlotList->pPtpSetcfg,buf,&iOffset);
        msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_PTP_CFG_NORMAL);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }

}

void handle_req_ptp_master(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[500];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        packmsg_ptp_master(pSlotList->pPtpSetcfg,buf,&iOffset);
        msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_PTP_CFG_MASTER);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }

}


void handle_req_ptp_slave(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[500];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        packmsg_ptp_slave(pSlotList->pPtpSetcfg,buf,&iOffset);
        msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_PTP_CFG_SLAVE);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }


}

void handle_req_ntp_normal(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[500];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        packmsg_ntp_normal(pSlotList->pNtpSetCfg,buf,&iOffset);
        msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_NTP_CFG_NORMAL);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }


}


void handle_req_ntp_md5_enable(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[500];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        packmsg_ntp_md5_enable(pSlotList->pNtpSetCfg,buf,&iOffset);
        msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_NTP_CFG_MD5_ENABLE);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }


}

void handle_req_ntp_blacklist(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[500];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        packmsg_ntp_blacklist(pSlotList->pNtpSetCfg,buf,&iOffset);
        msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_NTP_CFG_BLACKLIST);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }


}

void handle_req_ntp_whitlelist(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[500];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        packmsg_ntp_whitlelist(pSlotList->pNtpSetCfg,buf,&iOffset);
        msgPackRespFrameToSend(pRootData,pHeadFrame,CTL_WORD_DATA,buf,iOffset,CMD_NTP_CFG_WHITELSIT);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }


}


void handle_no_cmd_error(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0; 
    memset(buf,0,sizeof(buf));

    buf[iOffset++] = 0x00;
    buf[iOffset++] = 0x02;/**无效命令  */
    msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);

}


void handle_set_network_ctl(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[100];
    int iOffset = 0;
    int index = 0;

    struct NetInfor m_network_ctl;

    
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }

    memcpy(&m_network_ctl,&pRootData->comm_port,sizeof(struct NetInfor));

    
    m_network_ctl.ip = ntohl(*(int*)(data+index));
    index += 4;
    m_network_ctl.mask = ntohl(*(int*)(data+index));
    index += 4;
    m_network_ctl.gwip = ntohl(*(int*)(data+index));
    index += 4;
    memcpy(m_network_ctl.mac,data+index,6);
    index += 6;

    if(memcmp(&m_network_ctl,&pRootData->comm_port,sizeof(struct NetInfor)) != 0)
    {
        memcpy(&pRootData->comm_port,&m_network_ctl,sizeof(struct NetInfor));
        SaveNetParamToFile(ctlEthConfig,&pRootData->comm_port);
    }
    
    msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);
}

void handle_set_network_ptp(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[100];
    int iOffset = 0;
    int index = 0;

    struct NetInfor m_network_ctl;

    
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }

    memcpy(&m_network_ctl,&pRootData->ptp_port,sizeof(struct NetInfor));

    m_network_ctl.ip = ntohl(*(int*)(data+index));
    index += 4;
    m_network_ctl.mask = ntohl(*(int*)(data+index));
    index += 4;
    m_network_ctl.gwip = ntohl(*(int*)(data+index));
    index += 4;
    memcpy(m_network_ctl.mac,data+index,6);
    index += 6;

    msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);


    if(memcmp(&m_network_ctl,&pRootData->ptp_port,sizeof(struct NetInfor)) != 0)
    {
        memcpy(&pRootData->ptp_port,&m_network_ctl,sizeof(struct NetInfor));
        SaveNetParamToFile(ptpEthConfig,&pRootData->ptp_port);
        usleep(2000);
        SetCmdNetworkToEnv(&pRootData->ptp_port);
        usleep(2000);
        stop_ptp_daemon();
        usleep(2000);
        start_ptp_daemon();
    }
    
    
}

void handle_set_network_ntp(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[100];
    int iOffset = 0;
    int index = 0;

    struct NetInfor m_network_ctl;

    
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }

    memcpy(&m_network_ctl,&pRootData->ntp_port,sizeof(struct NetInfor));

    
    m_network_ctl.ip = ntohl(*(int*)(data+index));
    index += 4;
    m_network_ctl.mask = ntohl(*(int*)(data+index));
    index += 4;
    m_network_ctl.gwip = ntohl(*(int*)(data+index));
    index += 4;
    memcpy(m_network_ctl.mac,data+index,6);
    index += 6;

    msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);
    
    if(memcmp(&m_network_ctl,&pRootData->ntp_port,sizeof(struct NetInfor)) != 0)
    {
        memcpy(&pRootData->ntp_port,&m_network_ctl,sizeof(struct NetInfor));
        SaveNetParamToFile(ntpEthConfig,&pRootData->ntp_port);
        usleep(2000);
        SetCmdNetworkToEnv(&pRootData->ntp_port);
        usleep(2000);
        stop_ntp_daemon();
        usleep(2000);
        start_ntp_daemon();
    }
    
}

void handle_data_resetsys(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;
    
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }
    
    msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);

    usleep(2000);
    reset_sys_daemon();
}



void handle_set_system_setting(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[100];
    int iOffset = 0;
    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }

    struct clock_info *pClockInfo = &pRootData->clock_info;
    pClockInfo->ref_type = data[0];
    debug_level = data[1];

    msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);
}

void handle_set_ptp_all(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[500];
    int iOffset = 0;
    char set_success = FALSE;;
    
    struct PtpSetCfg m_ptpset_cfg;
    
    memset(buf,0,sizeof(buf));
    
    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
        msgUpPack_ptp_all(data,&m_ptpset_cfg);
        if(memcmp(pSlotList->pPtpSetcfg,&m_ptpset_cfg,sizeof(struct PtpSetCfg)) != 0)
            set_success = TRUE;
        
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }

    /**下发指令生效  */
    if(set_success)
    {
        memcpy(pSlotList->pPtpSetcfg,&m_ptpset_cfg,sizeof(struct PtpSetCfg));
        Save_PtpParam_ToFile(pSlotList->pPtpSetcfg,ptpConfig);
        stop_ptp_daemon();
        usleep(20000);
        start_ptp_daemon();
    }
}


void handle_set_ntp_normal(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[500];
    int iOffset = 0;
    int index = 0;
    int i;
    char set_success = FALSE;;
    
    struct NtpSetCfg m_ntpset_cfg;
    
    memset(buf,0,sizeof(buf));
    
    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
       
        memcpy(&m_ntpset_cfg,pSlotList->pNtpSetCfg,sizeof(struct NtpSetCfg));

        m_ntpset_cfg.broadcast = data[index++];
        m_ntpset_cfg.freq_b = data[index++];;
        m_ntpset_cfg.multicast = data[index++];;

        m_ntpset_cfg.freq_m = data[index++];;
        m_ntpset_cfg.md5_flag = data[index++];;
        //pSlotList->pNtpSetCfg->sympassive = data[index++];;

        for(i = 1;i < 9;i++)
        {
            m_ntpset_cfg.current_key[i].key_valid = data[index++];
            m_ntpset_cfg.current_key[i].key_length = data[index++];
            memcpy(m_ntpset_cfg.current_key[i].key,buf+index,20);
            index += 20;

        }

        if(memcmp(pSlotList->pNtpSetCfg,&m_ntpset_cfg,sizeof(struct NtpSetCfg)) != 0)
            set_success = TRUE;
    
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }

    /**下发指令生效  */
    if(set_success)
    {
        memcpy(pSlotList->pNtpSetCfg,&m_ntpset_cfg,sizeof(struct NtpSetCfg));
        Save_NtpParm_ToFile(pSlotList->pNtpSetCfg,ntpConfig);
        stop_ntp_daemon();
        usleep(20000);
        start_ntp_daemon();
    }
}


void handle_set_ntp_md5_enable(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[500];
    int iOffset = 0;
    int index = 0;
    int i;
    char set_success = FALSE;;
    
    struct NtpSetCfg m_ntpset_cfg;
    
    memset(buf,0,sizeof(buf));
    
    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
       
        memcpy(&m_ntpset_cfg,pSlotList->pNtpSetCfg,sizeof(struct NtpSetCfg));

        m_ntpset_cfg.broadcast_key_num = data[index++];
        m_ntpset_cfg.multicast_key_num = data[index++];
        m_ntpset_cfg.md5_flag = data[index++];
        //pSlotList->pNtpSetCfg->sympassive = data[index++];

        if(memcmp(pSlotList->pNtpSetCfg,&m_ntpset_cfg,sizeof(struct NtpSetCfg)) != 0)
            set_success = TRUE;
    
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }

    /**下发指令生效  */
    if(set_success)
    {
        memcpy(pSlotList->pNtpSetCfg,&m_ntpset_cfg,sizeof(struct NtpSetCfg));
        Save_NtpParm_ToFile(pSlotList->pNtpSetCfg,ntpConfig);
        stop_ntp_daemon();
        usleep(20000);
        start_ntp_daemon();
    }
}


void handle_set_ntp_blacklist(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[500];
    int iOffset = 0;
    int index = 0;
    int i;
    int num = 0;
    char set_success = FALSE;;
    
    struct NtpSetCfg m_ntpset_cfg;
    
    memset(buf,0,sizeof(buf));
    
    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
       
        memcpy(&m_ntpset_cfg,pSlotList->pNtpSetCfg,sizeof(struct NtpSetCfg));

        m_ntpset_cfg.blacklist = data[index++];
        m_ntpset_cfg.blacklist_num = data[index++];
        
        for(i=0;i<m_ntpset_cfg.blacklist_num;i++)
        {
            
            m_ntpset_cfg.blacklist_flag[i] = data[index++];
            
            m_ntpset_cfg.blacklist_ip[i] = htonl(*(int *)(buf+index));
            index += 4;
            
            m_ntpset_cfg.blacklist_mask[i] = htonl(*(int *)(buf+index));
            index += 4;
        }

        if(memcmp(pSlotList->pNtpSetCfg,&m_ntpset_cfg,sizeof(struct NtpSetCfg)) != 0)
            set_success = TRUE;
    
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }

    /**下发指令生效  */
    if(set_success)
    {
        memcpy(pSlotList->pNtpSetCfg,&m_ntpset_cfg,sizeof(struct NtpSetCfg));
        Save_NtpParm_ToFile(pSlotList->pNtpSetCfg,ntpConfig);
        stop_ntp_daemon();
        usleep(20000);
        start_ntp_daemon();
    }
}



void handle_set_ntp_whitelist(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *data)
{
    char buf[500];
    int iOffset = 0;
    int index = 0;
    int i;
    char set_success = FALSE;;
    int num = 0;
    
    struct NtpSetCfg m_ntpset_cfg;
    
    memset(buf,0,sizeof(buf));
    
    int dIndex = pHeadFrame->daddr;
    struct SlotList *pSlotList = &pRootData->slot_list[dIndex];
    if(pHeadFrame->pad_type == pSlotList->slot_type)
    {
       
        memcpy(&m_ntpset_cfg,pSlotList->pNtpSetCfg,sizeof(struct NtpSetCfg));

        m_ntpset_cfg.whitelist = data[index++];
        m_ntpset_cfg.whitelist_num = data[index++];
        
        for(i=0;i<m_ntpset_cfg.whitelist_num;i++)
        {
            
            m_ntpset_cfg.whitelist_flag[i] = data[index++];
            
            m_ntpset_cfg.whitelist_ip[i] = htonl(*(int *)(buf+index));
            index += 4;
            
            m_ntpset_cfg.whitelist_mask[i] = htonl(*(int *)(buf+index));
            index += 4;
        }

        if(memcmp(pSlotList->pNtpSetCfg,&m_ntpset_cfg,sizeof(struct NtpSetCfg)) != 0)
            set_success = TRUE;
    
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);
    }
    else
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
    }

    /**下发指令生效  */
    if(set_success)
    {
        memcpy(pSlotList->pNtpSetCfg,&m_ntpset_cfg,sizeof(struct NtpSetCfg));
        Save_NtpParm_ToFile(pSlotList->pNtpSetCfg,ntpConfig);
        stop_ntp_daemon();
        usleep(20000);
        start_ntp_daemon();
    }
}

void inssue_pps_data(struct root_data *pRootData)
{
    char buf[100];
    int iOffset = 0;
    unsigned short index = 0;
    
    struct clock_info *pClockInfo = &pRootData->clock_info;
    struct clock_alarm_data *pClockAlarm = &pClockInfo->alarmData;

    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(20171);
    addr.sin_addr.s_addr = pRootData->dev[ENUM_PC_CTL].net_attr.ip;

    
    memset(buf,0,sizeof(buf));

    buf[iOffset++] = '$';
    buf[iOffset++] = '<';
    buf[iOffset++] = 0x00;
    buf[iOffset++] = 0xff;
    *(unsigned short *)(buf + iOffset) = flip16(index);
    iOffset += 2;
    index++;

    buf[iOffset++] = CTL_WORD_DATA;
    buf[iOffset++] = 'A';
    *(int *)(buf + iOffset) = flip32(30);
    iOffset += 4;

    /**cmd type  */
    buf[iOffset++] = 0;
    buf[iOffset++] = 0;
    
    buf[iOffset++] = pClockInfo->ref_type;
    buf[iOffset++] = pClockInfo->workStatus;
    buf[iOffset++] = pClockAlarm->alarmSatellite;
    buf[iOffset++] = pClockAlarm->alarmPtp;
     *(int *)(buf + iOffset) = flip32(4);
    iOffset += 4;

    memcpy(buf+iOffset,pRootData->current_time,20);
    iOffset += 20;

    buf[iOffset++] = 0x0d;
    buf[iOffset++] = 0x0a;

    sendto(sockfd,buf,iOffset,0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));

    close(sockfd);
   
}

void handle_data_sysreset(struct root_data *pRootData,struct Head_Frame *pHeadFrame)
{
    char buf[100];
    int iOffset = 0;

    memset(buf,0,sizeof(buf));

    if(pHeadFrame->daddr != ENUM_SLOT_COR_ADDR)
    {
        buf[iOffset++] = 0x00;
        buf[iOffset++] = 0x01;/**盘类型不匹配  */
        msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_NAK,buf,iOffset);
        return;
    }    
    msgPackFrameToSend(pRootData,pHeadFrame,CTL_WORD_ACK,NULL,iOffset);

    reset_sys_daemon();
}

void process_pc_ctl_req(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *buf)
{
    int iOffset = 0;
    
    short cmd_type = buf[HEAD_FRAME_LENGTH]<<8 | buf[HEAD_FRAME_LENGTH+1];

    switch(cmd_type)
    {

     case CMD_DEV_INFORMATION:

        handle_req_dev_infor(pRootData,pHeadFrame);
        break;
     case CMD_NET_WORK_CTL:

        handle_req_network_ctl(pRootData,pHeadFrame);
        break;

     case CMD_NET_WORK_PTP:

        handle_req_network_ptp(pRootData,pHeadFrame);
        break;
     case CMD_NET_WORK_NTP:

        handle_req_network_ntp(pRootData,pHeadFrame);
        break;

     case CMD_VERSION_INFROMATION:

        handle_req_version(pRootData,pHeadFrame);
        break;
     case CMD_GPS_STATUS:

        handle_req_gps_status(pRootData,pHeadFrame);
        break;
     case CMD_SYS_SET:

        handle_req_system_setting(pRootData,pHeadFrame);
        break;
     case CMD_PTP_CFG_ALL:
        handle_req_ptp_all(pRootData,pHeadFrame);
        break;

     case CMD_PTP_CFG_NORMAL:
        handle_req_ptp_normal(pRootData,pHeadFrame);
        break;
     case CMD_PTP_CFG_SLAVE:
        handle_req_ptp_slave(pRootData,pHeadFrame);
        break;
     case CMD_PTP_CFG_MASTER:
        handle_req_ptp_master(pRootData,pHeadFrame);
        break;
             
     case CMD_NTP_CFG_NORMAL:
        handle_req_ntp_normal(pRootData,pHeadFrame);
        break;
     case CMD_NTP_CFG_MD5_ENABLE:
        handle_req_ntp_md5_enable(pRootData,pHeadFrame);
        break;
     case CMD_NTP_CFG_BLACKLIST:
        handle_req_ntp_blacklist(pRootData,pHeadFrame);
        break;
     case CMD_NTP_CFG_WHITELSIT:
        handle_req_ntp_whitlelist(pRootData,pHeadFrame);
        break;
     default:
        handle_no_cmd_error(pRootData,pHeadFrame);
        break;
    }

}

void process_pc_ctl_set(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *buf)
{
    int iOffset = 0;

    short cmd_type = buf[HEAD_FRAME_LENGTH]<<8 | buf[HEAD_FRAME_LENGTH+1];
    char *data = buf + sizeof(struct Head_Frame) + 2;
    
    switch(cmd_type)
    {
    case CMD_NET_WORK_CTL:
       handle_set_network_ctl(pRootData,pHeadFrame,data);
       break;
    case CMD_NET_WORK_PTP:
       handle_set_network_ptp(pRootData,pHeadFrame,data);
       break;
    case CMD_NET_WORK_NTP:
       handle_set_network_ntp(pRootData,pHeadFrame,data);
       break;

    case CMD_SYS_SET:
       handle_set_system_setting(pRootData,pHeadFrame,data);
       break;
    case CMD_PTP_CFG_ALL:
       handle_set_ptp_all(pRootData,pHeadFrame,data);
       break;
    case CMD_NTP_CFG_NORMAL:
       handle_set_ntp_normal(pRootData,pHeadFrame,data);
       break;
    case CMD_NTP_CFG_MD5_ENABLE:
       handle_set_ntp_md5_enable(pRootData,pHeadFrame,data);
       break;
    case CMD_NTP_CFG_BLACKLIST:
       handle_set_ntp_blacklist(pRootData,pHeadFrame,data);
       break;
    case CMD_NTP_CFG_WHITELSIT:
       handle_set_ntp_whitelist(pRootData,pHeadFrame,data);
       break;
    default:
       handle_no_cmd_error(pRootData,pHeadFrame);
       break;
    }

}

void process_pc_data(struct root_data *pRootData,struct Head_Frame *pHeadFrame,char *buf)
{
    int iOffset = 0;

    short cmd_type = buf[HEAD_FRAME_LENGTH]<<8 | buf[HEAD_FRAME_LENGTH+1];
    char *data = buf + sizeof(struct Head_Frame);
    
    switch(cmd_type)
    {
    case CMD_RESET_SYS:
       handle_data_sysreset(pRootData,pHeadFrame);
       break;

    default:
        handle_no_cmd_error(pRootData,pHeadFrame);
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
            process_pc_data(pRootData,&msgHead,buf);
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

