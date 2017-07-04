#include "uart_handle.h"
#include "ntp_type.h"
#include "ClientIpFilter.h"
#include "list_data.h"
#include "ptp.h"
#include "net.h"

extern NTP_ENVIRONMENT g_Ntp_Enviroment;  /* 全局NTP 环境参数结构体 */
extern NTPD_STATUS g_ntpd_status;

char g_ntp_enable[8];/**控制ntp使能  */

short gNtpdRestart = 0;/**如果修改参数，则递减3秒后重启ntpd  */


#define UART_LEN 1
#define BUF_SIZE 1024

#define LIST_BEFORE 0
#define LIST_AFTER  1

#define EOT 0x04
#define ENQ 0x05
#define ACK 0x06
#define NAK 0x15

#define ON_TIME_INTERVAL 1000

#define DISK_TYPE 'F'


#define SetBit(n, m)		(n |= 1 << m )
#define ClrBit(n, m)		(n &= ~(1 << m) )
#define GetBit(a, b, c, d)	\
do{					\
	if(c & (1 << d))	\
	{				\
		SetBit(a, b);	\
	}				\
	else				\
	{				\
		ClrBit(a, b);	\
	}				\
}while(0)				\


typedef struct{
	int cur;
	int flags;
	char buf[BUF_SIZE];
}UartBuf;

typedef struct{
    char dest;
    char src;
    char l_len;
    char h_len;
    char command;
}FrameHead;

typedef struct{
    char dest;
    char src;
    char l_len;
    char h_len;
    char discType;/**盘类型  */
    char command;
}DataFrameHead;


unsigned char CrcCal(unsigned char  *pData, unsigned int len)
{
	unsigned int i = 0;
	unsigned char  crc = 0;

	// 计算数据校验码
	for (i = 0; i < len; i++)
	{
		crc ^= *pData++;
	}

	return crc;
}


void CreatRecvFrame(UartBuf *pUart, char data)
{    

    static char last_data = 0;

	if (pUart->flags == 0x01)
	{
		pUart->buf[pUart->cur++] = data;
        //printf("cur %x\n",pUart->buf[pUart->cur-1]);
    }

    
	if (data == 'S' && last_data == 'W')
	{
		pUart->flags = 0x01;
		pUart->cur = 0;
	}
    else if (data == 0x0a && last_data == 0x0d)
	{
		pUart->flags = 0x10;
		pUart->buf[pUart->cur++] = data;
		pUart->buf[pUart->cur] = '\0';
  	}

    last_data = data;
    //printf("%x-------\n",data);
}

void CreatSendDataFrame(UartDevice *pUartDevice,Uint8 flag,char *pSrcBuf,short SrcLen)
{
    short nIndex = 0;
    char oBuf[BUF_SIZE];
    struct Uart_List_Head *p_Sendlist_head = pUartDevice->pSendListHead;
    
    memset(oBuf,0,BUF_SIZE);

    /**帧头  */
    oBuf[nIndex++] = 'W';       
    oBuf[nIndex++] = 'S';

    /**目的地址  */
    oBuf[nIndex++] = pUartDevice->dest_num;
    /**源地址  */
    oBuf[nIndex++] = pUartDevice->slot_num;

    /**数据长度  */
    oBuf[nIndex++] = (SrcLen) & 0xff;
    oBuf[nIndex++] = (SrcLen>>8) & 0xff;
    
    /**拷贝传输数据内容  */
    memcpy(oBuf+nIndex,pSrcBuf,SrcLen);
    nIndex += SrcLen;

    /**校验值 */
    oBuf[nIndex++] = 0x01;

    /**帧尾  */
    oBuf[nIndex++] = 0x0d;
    oBuf[nIndex++] = 0x0a;

    add_send_data_after(oBuf,nIndex,p_Sendlist_head);

}

void CreatSendCommandFrame(UartDevice *pUartDevice,char command,short len)
{
    short nIndex = 0;
    char oBuf[50];
    struct Uart_List_Head *p_Sendlist_head = pUartDevice->pSendListHead;

    
    memset(oBuf,0,sizeof(oBuf));

    
    oBuf[nIndex++] = 'W';
    oBuf[nIndex++] = 'S';
    oBuf[nIndex++] = pUartDevice->dest_num;
    oBuf[nIndex++] = pUartDevice->slot_num;

    oBuf[nIndex++] = 1;
    oBuf[nIndex++] = 0;

    oBuf[nIndex++] = command;

    /**校验值 */
    oBuf[nIndex++] = 0x01;

    /**结束值  */
    oBuf[nIndex++] = 0x0d;
    oBuf[nIndex++] = 0x0a;

    add_send_data_before(oBuf,nIndex,p_Sendlist_head);
}


void SendAckCommand(UartDevice *pUartDevice)
{
    short nIndex = 0;
    char oBuf[50];
    
    memset(oBuf,0,sizeof(oBuf));

    oBuf[nIndex++] = 'W';
    oBuf[nIndex++] = 'S';
    oBuf[nIndex++] = pUartDevice->dest_num;
    oBuf[nIndex++] = pUartDevice->slot_num;

    oBuf[nIndex++] = 1;
    oBuf[nIndex++] = 0;

    oBuf[nIndex++] = ACK;

    /**校验值 */
    oBuf[nIndex++] = 0x01;

    /**结束值  */
    oBuf[nIndex++] = 0x0d;
    oBuf[nIndex++] = 0x0a;
    
    write(pUartDevice->comfd,oBuf,nIndex);

    printf("====>send ack!!\n");
}

void SendEotCommand(UartDevice *pUartDevice)
{
    short nIndex = 0;
    char oBuf[50];
    
    memset(oBuf,0,sizeof(oBuf));

    oBuf[nIndex++] = 'W';
    oBuf[nIndex++] = 'S';
    oBuf[nIndex++] = pUartDevice->dest_num;
    oBuf[nIndex++] = pUartDevice->slot_num;

    oBuf[nIndex++] = 1;
    oBuf[nIndex++] = 0;

    oBuf[nIndex++] = EOT;

    /**校验值 */
    oBuf[nIndex++] = 0x01;

    /**结束值  */
    oBuf[nIndex++] = 0x0d;
    oBuf[nIndex++] = 0x0a;
    
    write(pUartDevice->comfd,oBuf,nIndex);

    printf("====>send Eot!!\n");
}


void SendUartData(UartDevice *pUartDevice)
{
    struct Uart_Data *pSendData=NULL,*pTemp=NULL;
    struct Uart_List_Head *p_Sendlist_head = pUartDevice->pSendListHead;
    int i;
    
    list_for_each_entry_safe(pSendData,pTemp,&p_Sendlist_head->list_head_uart,list_head)
    {
        write(pUartDevice->comfd,pSendData->data,pSendData->len);
        pSendData->send_cnt++;

        /**发送通一个数据发送3次，则删掉这个数据  */
        if(pSendData->send_cnt>3)
            delete_data(pSendData,p_Sendlist_head);
        #if 0
        for(i=0;i<pSendData->len;i++)
            printf("%x ",pSendData->data[i]);
        printf("\n");
        #endif
        printf("=======>send data\n");
        break;
    }
}


void DeletSendUartData(UartDevice *pUartDevice)
{
    struct Uart_Data *pSendData=NULL,*pTemp=NULL;
    struct Uart_List_Head *p_Sendlist_head = pUartDevice->pSendListHead;
    int i;
    
    list_for_each_entry_safe(pSendData,pTemp,&p_Sendlist_head->list_head_uart,list_head)
    {       
        delete_data(pSendData,p_Sendlist_head);
        break;
    }
}



void CopyPtpClockToPtpCommand0Param(PtpParam_Command0 *pPtpCommand0,PtpClock *pPtpClock)
{
   int i;
   
   pPtpCommand0->portEnable = pPtpClock->portEnable;
   pPtpCommand0->portStatus = pPtpClock->clockType;
   pPtpCommand0->domainNumber = pPtpClock->domainNumber;
   pPtpCommand0->protoType = pPtpClock->protoType;
   pPtpCommand0->modeType = pPtpClock->modeType;
   pPtpCommand0->transmitDelayType = pPtpClock->transmitDelayType;
   pPtpCommand0->stepType = pPtpClock->stepType;
   pPtpCommand0->UniNegotiationEnable = pPtpClock->UniNegotiationEnable;
   pPtpCommand0->grandmasterPriority1 = pPtpClock->grandmasterPriority1;
   pPtpCommand0->grandmasterPriority2 = pPtpClock->grandmasterPriority2;
   pPtpCommand0->profile = pPtpClock->profile;
   pPtpCommand0->logSyncInterval = pPtpClock->logSyncInterval;
   pPtpCommand0->logAnnounceInterval = pPtpClock->logAnnounceInterval;
   pPtpCommand0->logMinDelayReqInterval = pPtpClock->logMinDelayReqInterval;
   pPtpCommand0->currentUtcOffset = pPtpClock->timePropertiesDS.currentUtcOffset;
   pPtpCommand0->vlanEnable = pPtpClock->vlanEnable;
   pPtpCommand0->vlanPriority = pPtpClock->vlanPriority;
   pPtpCommand0->vlanId = pPtpClock->vlanId;
   pPtpCommand0->vlanCfi = pPtpClock->vlanCfi;
   pPtpCommand0->UnicastDuration = pPtpClock->UnicastDuration;
   pPtpCommand0->validServerNum = pPtpClock->unicastMultiServer.validServerNum;

    for(i=0;i<10;i++)
    {
       pPtpCommand0->serverIp[i] = pPtpClock->unicastMultiServer.serverList[i].serverIp;
    }
    
}

void CopyNtpParamToNtpCommand0Param(NtpParam_Command0 *pNtpCommand0)
{
    int i;

    for(i=0;i<PTP_PORT_COUNT;i++)
    {
       pNtpCommand0->portEnable[i] = g_ntp_enable[i];
       pNtpCommand0->portStatus[i] = 1;
    }
    
    pNtpCommand0->broadCastMode = g_Ntp_Parameter.broadcast;

    pNtpCommand0->multiCastMode = g_Ntp_Parameter.multicast;
     
    pNtpCommand0->symmetricMode = g_Ntp_Parameter.sympassive;

    pNtpCommand0->Md5Enable = g_Ntp_Enviroment.md5_flag;

    for(i=1;i<9;i++)
    {
        pNtpCommand0->md5Key[i].key_valid = TRUE;
        pNtpCommand0->md5Key[i].key_length = g_Ntp_Enviroment.current_key[i].key_length;
        strcpy(pNtpCommand0->md5Key[i].key,g_Ntp_Enviroment.current_key[i].key);
    }
}

void msgPackAlarmByte(UartDevice *pUartDevice,Alarm_Command *pAlarmCommand)
{
    PadAlarm *pPadAlarm = &pUartDevice->padAlarm;
    PtpClock *pPtpClock = pUartDevice->pPtpClock;

    if(pPadAlarm->slot_1_100Alarm)
        SetBit(pAlarmCommand->alarmByte_1,0);
    else
        ClrBit(pAlarmCommand->alarmByte_1,0);

    if(pPadAlarm->slot_2_100Alarm)
        SetBit(pAlarmCommand->alarmByte_1,1);
    else
        ClrBit(pAlarmCommand->alarmByte_1,1);

    if(pPadAlarm->slot_1_1ppsAlarm) 
        SetBit(pAlarmCommand->alarmByte_1,2);
    else
        ClrBit(pAlarmCommand->alarmByte_1,2);

    if(pPadAlarm->slot_2_1ppsAlarm)
        SetBit(pAlarmCommand->alarmByte_1,3);
    else
        ClrBit(pAlarmCommand->alarmByte_1,3);

    if(pPadAlarm->slot_1_TodAlarm)  
        SetBit(pAlarmCommand->alarmByte_1,4);
    else
        ClrBit(pAlarmCommand->alarmByte_1,4);


    if(pPadAlarm->slot_2_TodAlarm)
        SetBit(pAlarmCommand->alarmByte_1,5);
    else
        ClrBit(pAlarmCommand->alarmByte_1,5);

    if(pUartDevice->alarmOutput)
        SetBit(pAlarmCommand->alarmByte_1,6);
    else
        ClrBit(pAlarmCommand->alarmByte_1,6);


    /**PTP 0 */
    if(pUartDevice->diskAlarm)
    {
        SetBit(pAlarmCommand->alarmByte_2,0);
    }
    else
    {
        if(pPtpClock[0].notAvailableAlarm)
            SetBit(pAlarmCommand->alarmByte_2,0);
        else
            ClrBit(pAlarmCommand->alarmByte_2,0);
        
        if(pPtpClock[0].synAlarm)
            SetBit(pAlarmCommand->alarmByte_2,1);
        else
            ClrBit(pAlarmCommand->alarmByte_2,1);
        
        if(pPtpClock[0].announceAlarm)
            SetBit(pAlarmCommand->alarmByte_2,2);
        else
            ClrBit(pAlarmCommand->alarmByte_2,2);
        
        if(pPtpClock[0].delayRespAlarm)
            SetBit(pAlarmCommand->alarmByte_2,3);
        else
            ClrBit(pAlarmCommand->alarmByte_2,3);
        
        if(pPtpClock[0].InclockClassAlarm)
            SetBit(pAlarmCommand->alarmByte_2,4);
        else
            ClrBit(pAlarmCommand->alarmByte_2,4);

    }
    

    /**PTP   1*/
    if(pUartDevice->diskAlarm)
    {
        SetBit(pAlarmCommand->alarmByte_3,0);
    }
    else
    {
        if(pPtpClock[1].notAvailableAlarm)
            SetBit(pAlarmCommand->alarmByte_3,0);
        else
            ClrBit(pAlarmCommand->alarmByte_3,0);
        
        if(pPtpClock[1].synAlarm)
            SetBit(pAlarmCommand->alarmByte_3,1);
        else
            ClrBit(pAlarmCommand->alarmByte_3,1);
        
        if(pPtpClock[1].announceAlarm)
            SetBit(pAlarmCommand->alarmByte_3,2);
        else
            ClrBit(pAlarmCommand->alarmByte_3,2);
        
        if(pPtpClock[1].delayRespAlarm)
            SetBit(pAlarmCommand->alarmByte_3,3);
        else
            ClrBit(pAlarmCommand->alarmByte_3,3);
        
        if(pPtpClock[1].InclockClassAlarm)
            SetBit(pAlarmCommand->alarmByte_3,4);
        else
            ClrBit(pAlarmCommand->alarmByte_3,4);
        
        
        if(pPadAlarm->master100Alarm)
            SetBit(pAlarmCommand->alarmByte_4,0);
        else
            ClrBit(pAlarmCommand->alarmByte_4,0);
        
        if(pPadAlarm->master1ppsTodAlarm)
            SetBit(pAlarmCommand->alarmByte_4,1);
        else
            ClrBit(pAlarmCommand->alarmByte_4,1);
    }
    

    if(pPadAlarm->twoMasterAlarm)
        SetBit(pAlarmCommand->alarmByte_4,2);
    else
        ClrBit(pAlarmCommand->alarmByte_4,2);

    if(pPadAlarm->twoBackupAlarm)
        SetBit(pAlarmCommand->alarmByte_4,3);
    else
        ClrBit(pAlarmCommand->alarmByte_4,3);
            
    if(pPadAlarm->slot_1_TodCrcAlarm);
        
    if(pPadAlarm->slot_2_TodCrcAlarm);

    if(pPadAlarm->slot_1_RcuAlarm);
       
    if(pPadAlarm->slot_2_RcuAlarm);
        
    if(pPadAlarm->slot_1_Master_Backup);
    if(pPadAlarm->slot_2_Master_Backup);


}
void msgPackAlarmOnTimeUp(UartDevice *pUartDevice,Uint16 nCount)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    Alarm_Command alarmCommand;
    
    if(nCount%1000 == 0)
    {
        memset(oData,0,sizeof(oData));
        memset(&alarmCommand,0,sizeof(alarmCommand));

        /**盘类型  */
        oData[nOffset++] = DISK_TYPE;

        /**命令码 11  */
        oData[nOffset++] = 11;
        
        alarmCommand = pUartDevice->alarmCommand;
             
        memcpy(oData+nOffset,&alarmCommand,sizeof(alarmCommand));
        nOffset += sizeof(alarmCommand);
                          
        CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);
    }

}

void msgPackVersionTimeUp(UartDevice *pUartDevice)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    

    memset(oData,0,sizeof(oData));

    /**盘类型  */
    oData[nOffset++] = DISK_TYPE;

    /**命令码 10  */
    oData[nOffset++] = 10;

    memcpy(oData + nOffset,(char *)&pUartDevice->version,sizeof(Version_Command));
    nOffset += sizeof(Version_Command);

    CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);

}

void msgPackParamRequest(UartDevice *pUartDevice)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    

    memset(oData,0,sizeof(oData));

    /**盘类型  */
    oData[nOffset++] = DISK_TYPE;

    /**命令码 10  */
    oData[nOffset++] = 28;
    oData[nOffset++] = 255;

    CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);

}



void msgPackNetOnTimeUp(UartDevice *pUartDevice,Uint16 nCount)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    int i;
    NetWork_Command netWork;
    
    if(nCount%ON_TIME_INTERVAL == 0)
    {
        memset(oData,0,sizeof(oData));

        /**盘类型  */
        oData[nOffset++] = DISK_TYPE;

        /**命令码 12  */
        oData[nOffset++] = 12;

        for(i=0;i<PTP_PORT_COUNT;i++)
        {
            oData[nOffset++] = i;
            
            netWork.ip = g_ptpClock[i].netEnviroment.ip;
            netWork.gwip = g_ptpClock[i].netEnviroment.gwip;
            netWork.mask = g_ptpClock[i].netEnviroment.mask;
            memcpy(netWork.mac,g_ptpClock[i].netEnviroment.mac,6);
            memcpy(oData + nOffset,(char *)&netWork,sizeof(netWork));
            nOffset += sizeof(netWork);

        }
        
        CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);
    }

}

void msgPackPtpOnTimeUp(UartDevice *pUartDevice,Uint16 nCount)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    int i;
    PtpParam_Command0 ptpCommand0;
    
    if(nCount%ON_TIME_INTERVAL == 0)
    {
        memset(oData,0,sizeof(oData));
        memset(&ptpCommand0,0,sizeof(ptpCommand0));
        
        /**盘类型  */
        oData[nOffset++] = DISK_TYPE;

        
        /**命令码 13  */
        oData[nOffset++] = 13;


        /**命令0，全部所有参数  */
        oData[nOffset++] = 0;

        for(i=0;i<PTP_PORT_COUNT;i++)
        {
            
            oData[nOffset++] = i;
            CopyPtpClockToPtpCommand0Param(&ptpCommand0,&g_ptpClock[i]);

            memcpy(oData + nOffset,(char *)&ptpCommand0,sizeof(ptpCommand0));
            nOffset += sizeof(ptpCommand0);

        }
        
        CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);
    }

}


void msgPackNtpOnTimeUp(UartDevice *pUartDevice,Uint16 nCount)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    int i;
    NtpParam_Command0 NtpCommand0;
    
    if(nCount%ON_TIME_INTERVAL == 0)
    {
        memset(oData,0,sizeof(oData));
        memset(&NtpCommand0,0,sizeof(NtpCommand0));

        
        /**盘类型  */
        oData[nOffset++] = DISK_TYPE;

        
        /**命令码 16  */
        oData[nOffset++] = 16;


        /**命令0，全部所有参数  */
        oData[nOffset++] = 0;

        CopyNtpParamToNtpCommand0Param(&NtpCommand0);

        memcpy(oData + nOffset,(char *)&NtpCommand0,sizeof(NtpCommand0));
        nOffset += sizeof(NtpCommand0);
        
        CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);
    }

}


void msgPackAlarmChangeCommand(UartDevice *pUartDevice,Uint16 nCount)
{
    short nIndex = 0;
    char oBuf[50];
    Alarm_Command alarmCommand;
    PtpClock *pPtpClock = pUartDevice->pPtpClock;

    memset(&alarmCommand,0,sizeof(alarmCommand));

    if(nCount%5 == 0)
    {
        msgPackAlarmByte(pUartDevice,&alarmCommand);
        if(memcmp(&pUartDevice->alarmCommand,&alarmCommand,sizeof(alarmCommand)) != 0)
        {
            memset(oBuf,0,sizeof(oBuf));
                
            /**盘类型  */
            oBuf[nIndex++] = DISK_TYPE;

            /**命令码 20  */
            oBuf[nIndex++] = 20;

            pUartDevice->alarmCommand = alarmCommand;

            memcpy(oBuf+nIndex,&alarmCommand,sizeof(alarmCommand));
            nIndex += sizeof(alarmCommand); 

            CreatSendDataFrame(pUartDevice,LIST_AFTER,oBuf,nIndex);
        }
    }

}


void msgPackNtpChangeCommand(UartDevice *pUartDevice,Uint8 cmdType,char *src,short len)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    char *pStart = src; 


    memset(oData,0,sizeof(oData));

    /**盘类型  */
    oData[nOffset++] = DISK_TYPE;

    /**命令码 20  */
    oData[nOffset++] = 20;
    
    /**命令0  */
    oData[nOffset++] = cmdType;

    memcpy(oData + nOffset,pStart,len);
    nOffset += len;

    CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);


}

void msgPackPtpChangeCommand(UartDevice *pUartDevice,Uint8 cmdType,char *src,short len)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    char *pStart = src; 


    memset(oData,0,sizeof(oData));

    
    /**盘类型  */
    oData[nOffset++] = DISK_TYPE;

    
    /**命令码 22  */
    oData[nOffset++] = 22;


    /**命令0  */
    oData[nOffset++] = cmdType;

    memcpy(oData + nOffset,pStart,len);
    nOffset += len;

    CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);


}

void msgPackNetChangeCommand(UartDevice *pUartDevice,Uint8 cmdType,char *src,short len)
{
    char oData[BUF_SIZE];
    short nOffset = 0;
    char *pStart = src; 

    memset(oData,0,sizeof(oData));

    /**盘类型  */
    oData[nOffset++] = DISK_TYPE;

    
    /**命令码 21  */
    oData[nOffset++] = 21;
   
    memcpy(oData + nOffset,pStart,len);
    nOffset += len;

    CreatSendDataFrame(pUartDevice,LIST_AFTER,oData,nOffset);

}

void msgPackOffset1sOnTimeUp(UartDevice *pUartDevice,PtpClock *pPtpClock,short bestIndex)
{

    char oData[BUF_SIZE];
    short nOffset = 0;
    int i;

    memset(oData,0,sizeof(oData));
    
    /**盘类型  */
    oData[nOffset++] = DISK_TYPE;

    /**命令码 15  */
    oData[nOffset++] = 15;

    oData[nOffset++] = bestIndex;
    
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        //if((pPtpClock[i].clockType == PTP_SLAVE) && (pPtpClock[i].notAvailableAlarm == FALSE))
        {
            oData[nOffset++] = i;
            memcpy(oData + nOffset,&pPtpClock[i].TimeOffset,sizeof(TimeInternal));
            nOffset += sizeof(TimeInternal);
        }
    }

    CreatSendDataFrame(pUartDevice,1,oData,nOffset);


}

void msgPackSlaveUpOnTimeUp(UartDevice *pUartDevice,PtpClock *pPtpClock,Uint16 nCount)
{
    char oData[1024];
    short nOffset = 0;
    int i;
    
    SlaveUp_Command0 slaveCommand0;

    if(nCount%10== 0)
    {
        memset(oData,0,sizeof(oData));

        /**盘类型  */
        oData[nOffset++] = DISK_TYPE;

        /**命令码 13  */
        oData[nOffset++] = 14;

        
        for(i=0;i<PTP_PORT_COUNT;i++)
        {
            if(pPtpClock[i].clockType == PTP_SLAVE &&(pPtpClock[i].notAvailableAlarm == FALSE))
            {
                slaveCommand0.grandmasterPriority1 = pPtpClock[i].grandmasterPriority1;
                slaveCommand0.grandmasterPriority2 = pPtpClock[i].grandmasterPriority2;
                slaveCommand0.timeSource = pPtpClock[i].timePropertiesDS.timeSource;
                slaveCommand0.leapFlag = pPtpClock[i].timePropertiesDS.leap61;
                slaveCommand0.currentUtcOffset = pPtpClock[i].timePropertiesDS.currentUtcOffset;
                slaveCommand0.grandmasterClockQuality = pPtpClock[i].grandmasterClockQuality;
                slaveCommand0.sourcePortIdentity = pPtpClock[i].parentPortIdentity;
                memcpy(&slaveCommand0.grandmasterIdentity,&pPtpClock[i].grandmasterIdentity,sizeof(ClockIdentity));
        
                slaveCommand0.BestMasterIp = pPtpClock[i].BestMasterIp;
                memcpy(slaveCommand0.BestMasterMac,pPtpClock[i].BestMasterMac,6);
        
                oData[nOffset++] = i;
                memcpy(oData + nOffset,&slaveCommand0,sizeof(slaveCommand0));
                nOffset += sizeof(slaveCommand0);
            }
        }
        //oData[nOffset++] = ';';
    
        CreatSendDataFrame(pUartDevice,1,oData,nOffset);
    }
    

}


void CopyNtpCommand0ParamToNtpParam(NtpParam_Command0 *pNtpCommand0)
{
    int i;

    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        g_ntp_enable[i] = pNtpCommand0->portEnable[i];
    }
    g_Ntp_Parameter.broadcast = pNtpCommand0->broadCastMode;
    g_Ntp_Parameter.multicast = pNtpCommand0->multiCastMode;
    g_Ntp_Parameter.sympassive = pNtpCommand0->symmetricMode;

    g_Ntp_Enviroment.md5_flag = pNtpCommand0->Md5Enable;

    for(i=1;i<9;i++)
    {
        g_Ntp_Enviroment.current_key[i].key_valid = TRUE;
        g_Ntp_Enviroment.current_key[i].key_length = pNtpCommand0->md5Key[i].key_length;
        strcpy(g_Ntp_Enviroment.current_key[i].key,pNtpCommand0->md5Key[i].key);
    }
}

void CopyNtpCommand1ParamToNtpParam(NtpParam_Command1 *pNtpCommand1)
{
    g_ntp_enable[pNtpCommand1->ethx] = pNtpCommand1->portEnable;
}

void CopyNtpCommand2ParamToNtpParam(NtpParam_Command2 *pNtpCommand2)
{
   g_Ntp_Enviroment.md5_flag = pNtpCommand2->Md5Enable;

}


void CopyNtpCommand3ParamToNtpParam(NtpParam_Command3 *pNtpCommand3)
{
    g_Ntp_Enviroment.current_key[pNtpCommand3->keyIndex].key_valid = TRUE;
    g_Ntp_Enviroment.current_key[pNtpCommand3->keyIndex].key_length = pNtpCommand3->md5Key.key_length;
    memcpy(g_Ntp_Enviroment.current_key[pNtpCommand3->keyIndex].key
        ,pNtpCommand3->md5Key.key,sizeof(pNtpCommand3->md5Key.key));

}


Uint8 CompareNtpCommand0(NtpParam_Command0 *pNtpCommand0)
{
    
    Uint8 cFlag = FALSE;
    int i;

    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        if(pNtpCommand0->portEnable[i] != g_ntp_enable[i])
        {
            cFlag = TRUE;
            break;
        }
    }

    if(pNtpCommand0->broadCastMode != g_Ntp_Parameter.broadcast)
        cFlag = TRUE;
    if(pNtpCommand0->multiCastMode != g_Ntp_Parameter.multicast)
        cFlag = TRUE;
    if(pNtpCommand0->symmetricMode != g_Ntp_Parameter.sympassive)
        cFlag = TRUE;

    if(pNtpCommand0->Md5Enable != g_Ntp_Enviroment.md5_flag)
        cFlag = TRUE;

    for(i=1;i<9;i++)
    {
        if(strcmp(pNtpCommand0->md5Key[i].key,g_Ntp_Enviroment.current_key[i].key) != 0)
        {
            cFlag = TRUE;
            break;
        }
    }

    return cFlag;
}

Uint8 CompareNtpCommand1(NtpParam_Command1 *pNtpCommand1)
{   
    
    if(g_ntp_enable[pNtpCommand1->ethx] != pNtpCommand1->portEnable)
        return TRUE;
    else
        return FALSE;
}
Uint8 CompareNtpCommand2(NtpParam_Command2 *pNtpCommand2)
{
    if(pNtpCommand2->Md5Enable != g_Ntp_Enviroment.md5_flag)
        return TRUE;
    else 
        return FALSE;
    
}
Uint8 CompareNtpCommand3(NtpParam_Command3 *pNtpCommand3)
{
    
    Uint8 cFlag = FALSE;
    if(memcmp(g_Ntp_Enviroment.current_key[pNtpCommand3->keyIndex].key
        ,pNtpCommand3->md5Key.key,sizeof(pNtpCommand3->md5Key.key) != 0))
    cFlag = TRUE;

    return cFlag;
}





void Analyse_NtpParam_Command0(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    NtpParam_Command0 NtpCommand0;
    short len = 0;
    char *pStart = pData; 

    memcpy(&NtpCommand0,pData,sizeof(NtpCommand0));
    if(CompareNtpCommand0(&NtpCommand0))
    {
        printf("===>ntp set parm yes\n");
        CopyNtpCommand0ParamToNtpParam(&NtpCommand0);
        //Save_Ntp_Conf();
        
        len = sizeof(NtpCommand0);

        //msgPackNtpChangeCommand(pUartDevice,0,pStart,len);
        
        gNtpdRestart = TRUE;

    }
 
}
void Analyse_NtpParam_Command1(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    NtpParam_Command1 NtpCommand1;
    short len = 0;
    char *pStart = pData; 

    memcpy(&NtpCommand1,pData,sizeof(NtpCommand1));
    if(CompareNtpCommand1(&NtpCommand1))
    {
        CopyNtpCommand1ParamToNtpParam(&NtpCommand1);
        //Save_Ntp_Conf();
        
        len = sizeof(NtpCommand1);

        //msgPackNtpChangeCommand(pUartDevice,1,pStart,len);
        
        gNtpdRestart = TRUE;

    }
}

void Analyse_NtpParam_Command2(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    NtpParam_Command2 NtpCommand2;
    short len = 0;
    char *pStart = pData; 

    memcpy(&NtpCommand2,pData,sizeof(NtpCommand2));
    if(CompareNtpCommand2(&NtpCommand2))
    {
        CopyNtpCommand2ParamToNtpParam(&NtpCommand2);
        //Save_Ntp_Conf();
        
        len = sizeof(NtpCommand2);
        
        //msgPackNtpChangeCommand(pUartDevice,2,pStart,len);
        
        gNtpdRestart = TRUE;

    }

}

void Analyse_NtpParam_Command3(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    NtpParam_Command3 NtpCommand3;
    short len = 0;
    char *pStart = pData; 

    
    memcpy(&NtpCommand3,pData,sizeof(NtpCommand3));
    if(CompareNtpCommand3(&NtpCommand3))
    {
        CopyNtpCommand3ParamToNtpParam(&NtpCommand3);
        //Write_Md5ToKeyFile(g_Ntp_Enviroment.current_key,8);

        len = sizeof(NtpCommand3);
        
        //msgPackNtpChangeCommand(pUartDevice,3,pStart,len);
        
        gNtpdRestart = TRUE;

    }
    
}




void CopyPtpCommand0ParamToPtpClock(PtpClock *pPtpClock,PtpParam_Command0 *pPtpCommand0)
{
    int i;
    Uint8 tmp;
    tmp = pPtpClock->portEnable;
    
    pPtpClock->portEnable = FALSE;
    
    pPtpClock->clockType = pPtpCommand0->portStatus;
    pPtpClock->domainNumber = pPtpCommand0->domainNumber;
    pPtpClock->protoType = pPtpCommand0->protoType;
    pPtpClock->modeType = pPtpCommand0->modeType;
    pPtpClock->transmitDelayType = pPtpCommand0->transmitDelayType;
    pPtpClock->stepType = pPtpCommand0->stepType;
    pPtpClock->UniNegotiationEnable = pPtpCommand0->UniNegotiationEnable;
    pPtpClock->grandmasterPriority1 = pPtpCommand0->grandmasterPriority1;
    pPtpClock->grandmasterPriority2 = pPtpCommand0->grandmasterPriority2;
    pPtpClock->profile = pPtpCommand0->profile;


    pPtpClock->logSyncInterval = pPtpCommand0->logSyncInterval;    
    pPtpClock->synSendInterval = Get_MessageInterval(pPtpClock->logSyncInterval);

    pPtpClock->logAnnounceInterval = pPtpCommand0->logAnnounceInterval;
    pPtpClock->announceSendInterval = Get_MessageInterval(pPtpClock->logAnnounceInterval);


    pPtpClock->logMinDelayReqInterval= pPtpCommand0->logMinDelayReqInterval;
    pPtpClock->logMinPdelayReqInterval= pPtpCommand0->logMinDelayReqInterval;
    pPtpClock->delayreqInterval = Get_MessageInterval(pPtpClock->logMinPdelayReqInterval);


    
    pPtpClock->timePropertiesDS.currentUtcOffset = pPtpCommand0->currentUtcOffset;
    pPtpClock->vlanEnable = pPtpCommand0->vlanEnable;
    pPtpClock->vlanPriority = pPtpCommand0->vlanPriority;
    pPtpClock->vlanId = pPtpCommand0->vlanId;
    pPtpClock->vlanCfi = pPtpCommand0->vlanCfi;
    pPtpClock->UnicastDuration = pPtpCommand0->UnicastDuration;
    pPtpClock->unicastMultiServer.validServerNum = pPtpCommand0->validServerNum;

    for(i=0;i<10;i++)
    {
        pPtpClock->unicastMultiServer.serverList[i].serverIp = pPtpCommand0->serverIp[i];
    }




    if(pPtpCommand0->portEnable == tmp)     
        pPtpClock->portEnable = tmp;
    else
        pPtpClock->portEnable = pPtpCommand0->portEnable;
      

    
}

void CopyPtpCommand1ParamToPtpClock(PtpClock *pPtpClock,PtpParam_Command1 *pPtpCommand1)
{
    Uint8 tmp;
    tmp = pPtpClock->portEnable;
    pPtpClock->portEnable = FALSE;

    pPtpClock->clockType = pPtpCommand1->portStatus;
    pPtpClock->domainNumber = pPtpCommand1->domainNumber;
    pPtpClock->protoType = pPtpCommand1->protoType;
    pPtpClock->modeType = pPtpCommand1->modeType;
    pPtpClock->transmitDelayType = pPtpCommand1->transmitDelayType;
    pPtpClock->stepType = pPtpCommand1->stepType;
    pPtpClock->UniNegotiationEnable = pPtpCommand1->UniNegotiationEnable;
        
    if(pPtpCommand1->portEnable == tmp)     
        pPtpClock->portEnable = tmp;
    else
        pPtpClock->portEnable = pPtpCommand1->portEnable;


}

void CopyPtpCommand2ParamToPtpClock(PtpClock *pPtpClock,PtpParam_Command2 *pPtpCommand2)
{
    
    //pPtpClock->portEnable = FALSE;
    pPtpClock->profile = pPtpCommand2->profile;

    //pPtpClock->portEnable = TRUE;
}

void CopyPtpCommand3ParamToPtpClock(PtpClock *pPtpClock,PtpParam_Command3 *pPtpCommand3)
{
    Uint8 tmp;
    tmp = pPtpClock->portEnable;
    pPtpClock->portEnable = FALSE;
    
    pPtpClock->grandmasterPriority1 = pPtpCommand3->grandmasterPriority1;
    pPtpClock->grandmasterPriority2 = pPtpCommand3->grandmasterPriority2;

    pPtpClock->logSyncInterval = pPtpCommand3->logSyncInterval;    
    pPtpClock->synSendInterval = Get_MessageInterval(pPtpClock->logSyncInterval);

    pPtpClock->logAnnounceInterval = pPtpCommand3->logAnnounceInterval;
    pPtpClock->announceSendInterval = Get_MessageInterval(pPtpClock->logAnnounceInterval);

    pPtpClock->timePropertiesDS.currentUtcOffset = pPtpCommand3->currentUtcOffset;
        
    pPtpClock->portEnable = tmp;
}

void CopyPtpCommand4ParamToPtpClock(PtpClock *pPtpClock,PtpParam_Command4 *pPtpCommand4)
{
    Uint8 tmp;
    tmp = pPtpClock->portEnable;
    pPtpClock->portEnable = FALSE;
    
    pPtpClock->vlanEnable = pPtpCommand4->vlanEnable;
    pPtpClock->vlanPriority = pPtpCommand4->vlanPriority;
    pPtpClock->vlanId = pPtpCommand4->vlanId;
    pPtpClock->vlanCfi = pPtpCommand4->vlanCfi;

    pPtpClock->portEnable = tmp;
}

void CopyPtpCommand5ParamToPtpClock(PtpClock *pPtpClock,PtpParam_Command5 *pPtpCommand5)
{
    Uint8 tmp;
    tmp = pPtpClock->portEnable;
    pPtpClock->portEnable = FALSE;

    pPtpClock->UniNegotiationEnable = pPtpCommand5->UniNegotiationEnable;
     
    pPtpClock->logMinDelayReqInterval= pPtpCommand5->logMinDelayReqInterval;
    pPtpClock->logMinPdelayReqInterval= pPtpCommand5->logMinDelayReqInterval;
    pPtpClock->delayreqInterval = Get_MessageInterval(pPtpClock->logMinDelayReqInterval);


    pPtpClock->UnicastDuration = pPtpCommand5->UnicastDuration;
    pPtpClock->unicastMultiServer.validServerNum = pPtpCommand5->validServerNum;
    pPtpClock->unicastMultiServer.serverList[pPtpCommand5->serverIndex].serverIp = pPtpCommand5->serverIp;

    pPtpClock->portEnable = tmp;
}


Uint8 ComparePtpCommand0(PtpParam_Command0 *pPtpCommand0,PtpClock *pPtpClock)
{
    Uint8 cFlag = FALSE;
    int i;
    
    if(pPtpCommand0->portEnable != pPtpClock->portEnable)
        cFlag = TRUE;
    if(pPtpCommand0->portStatus != pPtpClock->clockType)
        cFlag = TRUE;
    if(pPtpCommand0->domainNumber!= pPtpClock->domainNumber)
        cFlag = TRUE;
    if(pPtpCommand0->protoType != pPtpClock->protoType)
        cFlag = TRUE;
    if(pPtpCommand0->modeType != pPtpClock->modeType)
        cFlag = TRUE;
    if(pPtpCommand0->transmitDelayType != pPtpClock->transmitDelayType)
        cFlag = TRUE;
    if(pPtpCommand0->stepType != pPtpClock->stepType)
        cFlag = TRUE;
    if(pPtpCommand0->UniNegotiationEnable != pPtpClock->UniNegotiationEnable)
        cFlag = TRUE;    
    if(pPtpCommand0->grandmasterPriority1 != pPtpClock->grandmasterPriority1)
        cFlag = TRUE;
    if(pPtpCommand0->grandmasterPriority2 != pPtpClock->grandmasterPriority2)
        cFlag = TRUE;
    if(pPtpCommand0->profile != pPtpClock->profile)
        cFlag = TRUE;
    if(pPtpCommand0->logSyncInterval != pPtpClock->logSyncInterval)
        cFlag = TRUE;
    if(pPtpCommand0->logAnnounceInterval != pPtpClock->logAnnounceInterval)
        cFlag = TRUE;
    if((pPtpCommand0->logMinDelayReqInterval != pPtpClock->logMinDelayReqInterval)
        || (pPtpCommand0->logMinDelayReqInterval != pPtpClock->logMinPdelayReqInterval))
        cFlag = TRUE;
    if(pPtpCommand0->currentUtcOffset != pPtpClock->timePropertiesDS.currentUtcOffset)
        cFlag = TRUE;
    if(pPtpCommand0->vlanEnable != pPtpClock->vlanEnable)
        cFlag = TRUE;
    if(pPtpCommand0->vlanPriority != pPtpClock->vlanPriority)
        cFlag = TRUE;
    if(pPtpCommand0->vlanId != pPtpClock->vlanId)
        cFlag = TRUE;
    if(pPtpCommand0->vlanCfi != pPtpClock->vlanCfi)
        cFlag = TRUE;
    if(pPtpCommand0->UnicastDuration != pPtpClock->UnicastDuration)
        cFlag = TRUE;    
    if(pPtpCommand0->validServerNum != pPtpClock->unicastMultiServer.validServerNum)
        cFlag = TRUE;

    for(i=0;i<10;i++)
    {
        if(pPtpCommand0->serverIp[i] != pPtpClock->unicastMultiServer.serverList[i].serverIp)
        {
            cFlag = TRUE;
            break;
        }
            
    }

    return cFlag;
}

Uint8 ComparePtpCommand1(PtpParam_Command1 *pPtpCommand1,PtpClock *pPtpClock)
{
    Uint8 cFlag = FALSE;
    
    if(pPtpCommand1->portEnable != pPtpClock->portEnable)
        cFlag = TRUE;
    if(pPtpCommand1->portStatus != pPtpClock->clockType)
        cFlag = TRUE;
    if(pPtpCommand1->domainNumber!= pPtpClock->domainNumber)
        cFlag = TRUE;
    if(pPtpCommand1->protoType != pPtpClock->protoType)
        cFlag = TRUE;
    if(pPtpCommand1->modeType != pPtpClock->modeType)
        cFlag = TRUE;
    if(pPtpCommand1->transmitDelayType != pPtpClock->transmitDelayType)
        cFlag = TRUE;
    if(pPtpCommand1->stepType != pPtpClock->stepType)
        cFlag = TRUE;
    if(pPtpCommand1->UniNegotiationEnable != pPtpClock->UniNegotiationEnable)
        cFlag = TRUE;    
    
    return cFlag;
}

Uint8 ComparePtpCommand2(PtpParam_Command2 *pPtpCommand2,PtpClock *pPtpClock)
{
    Uint8 cFlag = FALSE;
    
    if(pPtpCommand2->profile != pPtpClock->profile)
        cFlag = TRUE;

    
    return cFlag;
}

Uint8 ComparePtpCommand3(PtpParam_Command3 *pPtpCommand3,PtpClock *pPtpClock)
{
    Uint8 cFlag = FALSE;
    
    if(pPtpCommand3->grandmasterPriority1 != pPtpClock->grandmasterPriority1)
        cFlag = TRUE;
        
    if(pPtpCommand3->grandmasterPriority2 != pPtpClock->grandmasterPriority2)
        cFlag = TRUE;

        
    if(pPtpCommand3->logSyncInterval != pPtpClock->logSyncInterval)
        cFlag = TRUE;
        
    if(pPtpCommand3->logAnnounceInterval != pPtpClock->logAnnounceInterval)
        cFlag = TRUE;
 
    if(pPtpCommand3->currentUtcOffset != pPtpClock->timePropertiesDS.currentUtcOffset)
        cFlag = TRUE;
        
        
    return cFlag;
}

Uint8 ComparePtpCommand4(PtpParam_Command4 *pPtpCommand4,PtpClock *pPtpClock)
{
    Uint8 cFlag = FALSE;
    
    if(pPtpCommand4->vlanEnable != pPtpClock->vlanEnable)
        cFlag = TRUE;
    if(pPtpCommand4->vlanPriority != pPtpClock->vlanPriority)
        cFlag = TRUE;
    if(pPtpCommand4->vlanId != pPtpClock->vlanId)
        cFlag = TRUE;
    if(pPtpCommand4->vlanCfi != pPtpClock->vlanCfi)
        cFlag = TRUE;

   
    return cFlag;
}

Uint8 ComparePtpCommand5(PtpParam_Command5 *pPtpCommand5,PtpClock *pPtpClock)
{
    Uint8 cFlag = FALSE;
    
    if(pPtpCommand5->UniNegotiationEnable != pPtpClock->UniNegotiationEnable)
        cFlag = TRUE;  
    if((pPtpCommand5->logMinDelayReqInterval != pPtpClock->logMinDelayReqInterval)
        || (pPtpCommand5->logMinDelayReqInterval != pPtpClock->logMinPdelayReqInterval))
        cFlag = TRUE;

    if(pPtpCommand5->UnicastDuration != pPtpClock->UnicastDuration)
        cFlag = TRUE;    
    if(pPtpCommand5->validServerNum != pPtpClock->unicastMultiServer.validServerNum)
        cFlag = TRUE;
    if(pPtpCommand5->serverIp !=  pPtpClock->unicastMultiServer.serverList[pPtpCommand5->serverIndex].serverIp)
        cFlag = TRUE;

    
    return cFlag;
}



void Analyse_PtpParam_Command0(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    PtpParam_Command0 ptpCommand0;
    short len = 0;
    char *pStart = pData; /**数据位eth 0  */

    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        if(*pData == i)
        {            
            pData += 1;
            memcpy(&ptpCommand0,pData,sizeof(ptpCommand0));
            if(ComparePtpCommand0(&ptpCommand0,&g_ptpClock[i]))
            {
                printf("PTP Set Command0 port %d!!\n",i);
                CopyPtpCommand0ParamToPtpClock(&g_ptpClock[i],&ptpCommand0);
                //Save_PtpParam_ToFile(&g_ptpClock[i],g_ptpClock[i].ptpFileName);

                Init_NetComm(&g_ptpClock[i],&g_ptpClock[i].netComm);
                Init_RecvRdfs(&g_ptpClock[i]);
                ReInit_FpgaCore(&g_ptpClock[i]);
                Init_UnicastClient(&g_ptpClock[i]);
            }
                
            pData += sizeof(ptpCommand0);
        }
    }
        
    len = sizeof(ptpCommand0)+1;
    len = len * PTP_PORT_COUNT;

    //msgPackPtpChangeCommand(pUartDevice,0,pStart,len);

    
}

void Analyse_PtpParam_Command1(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    PtpParam_Command1 ptpCommand1;
    short len = 0;
    char *pStart = pData; 


    i = *pData;
    if(i>PTP_PORT_COUNT)
        return;
    
    pData += 1;
    memcpy(&ptpCommand1,pData,sizeof(ptpCommand1));
    
    
    if(ComparePtpCommand1(&ptpCommand1,&g_ptpClock[i]))
    {
        CopyPtpCommand1ParamToPtpClock(&g_ptpClock[i],&ptpCommand1);
        printf("PTP Set Command1 port %d!!\n",i);
        //Save_PtpParam_ToFile(&g_ptpClock[i],g_ptpClock[i].ptpFileName);
        Init_NetComm(&g_ptpClock[i],&g_ptpClock[i].netComm);
        Init_RecvRdfs(&g_ptpClock[i]);
        ReInit_FpgaCore(&g_ptpClock[i]);
        Init_UnicastClient(&g_ptpClock[i]);
    }
    
    len = sizeof(ptpCommand1) + 1;
    //msgPackPtpChangeCommand(pUartDevice,1,pStart,len);


}
void Analyse_PtpParam_Command2(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    PtpParam_Command2 ptpCommand2;
    short len = 0;
    char *pStart = pData; 

    i = *pData;
    if(i>PTP_PORT_COUNT)
        return;

    pData += 1;
    
    memcpy(&ptpCommand2,pData,sizeof(ptpCommand2));
    if(ComparePtpCommand2(&ptpCommand2,&g_ptpClock[i]))
    {
        printf("PTP Set Command2 port %d!!\n",i);
        CopyPtpCommand2ParamToPtpClock(&g_ptpClock[i],&ptpCommand2);
        //Save_PtpParam_ToFile(&g_ptpClock[i],g_ptpClock[i].ptpFileName);
        
    }

    len = sizeof(ptpCommand2) + 1;
    //msgPackPtpChangeCommand(pUartDevice,2,pStart,len);
    
    
}
void Analyse_PtpParam_Command3(UartDevice *pUartDevice,char *pData)
{

    int i = 0;
    PtpParam_Command3 ptpCommand3;
    short len = 0;
    char *pStart = pData; 


    i = *pData;
    if(i>=PTP_PORT_COUNT)
        return;

    pData += 1;

    memcpy(&ptpCommand3,pData,sizeof(ptpCommand3));
    if(ComparePtpCommand3(&ptpCommand3,&g_ptpClock[i]))
    {
        printf("PTP Set Command3 port %d!!\n",i);
        CopyPtpCommand3ParamToPtpClock(&g_ptpClock[i],&ptpCommand3);
        
        //Save_PtpParam_ToFile(&g_ptpClock[i],g_ptpClock[i].ptpFileName);
        
        len = sizeof(ptpCommand3) + 1;
        //msgPackPtpChangeCommand(pUartDevice,3,pStart,len);
        
    }

}

void Analyse_PtpParam_Command4(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    PtpParam_Command4 ptpCommand4;
    short len = 0;
    char *pStart = pData; 

    i = *pData;
    if(i>PTP_PORT_COUNT)
        return;

    pData += 1;

    memcpy(&ptpCommand4,pData,sizeof(ptpCommand4));
    if(ComparePtpCommand4(&ptpCommand4,&g_ptpClock[i]))
    {
        printf("PTP Set Command4 port %d!!\n",i);
        CopyPtpCommand4ParamToPtpClock(&g_ptpClock[i],&ptpCommand4);
        //Save_PtpParam_ToFile(&g_ptpClock[i],g_ptpClock[i].ptpFileName);

        len = sizeof(ptpCommand4) + 1;
        //msgPackPtpChangeCommand(pUartDevice,4,pStart,len);
    }



}

void Analyse_PtpParam_Command5(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    PtpParam_Command5 ptpCommand5;
    short len = 0;
    char *pStart = pData; 

    i = *pData;
    if(i>PTP_PORT_COUNT)
        return;

    pData += 1;

    memcpy(&ptpCommand5,pData,sizeof(ptpCommand5));
    if(ComparePtpCommand5(&ptpCommand5,&g_ptpClock[i]))
    {
        printf("PTP Set Command5 port %d!!\n",i);
        CopyPtpCommand5ParamToPtpClock(&g_ptpClock[i],&ptpCommand5);
        //Save_PtpParam_ToFile(&g_ptpClock[i],g_ptpClock[i].ptpFileName);

        Init_UnicastClient(&g_ptpClock[i]);
        len = sizeof(ptpCommand5) + 1;
        //msgPackPtpChangeCommand(pUartDevice,5,pStart,len);
    }
    
}


void CopyNetCommand0ParamToPtpClock(PtpClock *pPtpClock,NetWork_Command *pNetCommand)
{
        
    pPtpClock->netEnviroment.ip = pNetCommand->ip;

    pPtpClock->netEnviroment.gwip = pNetCommand->gwip;

    pPtpClock->netEnviroment.mask = pNetCommand->mask;

}

void Analyse_NetParam_Command0(UartDevice *pUartDevice,char *pData)
{
    int i = 0;
    NetWork_Command netWorkCommand0;
    short len = 0;
    char *pStart = pData; 
    Uint8 tmp;
    
    i = *pData;
    if(i<PTP_PORT_COUNT)
    {
        pData += 1;
        memcpy(&netWorkCommand0,pData,sizeof(netWorkCommand0));
       
       printf("sizeof()=====%d=====%d===%d\n",i,netWorkCommand0.ip>>24&0xff,g_ptpClock[i].netEnviroment.ip>>24&0xff);
        if(netWorkCommand0.ip != g_ptpClock[i].netEnviroment.ip)
        {
            printf("set net success port %d ip \n",i);
            tmp =  g_ptpClock[i].portEnable;
            g_ptpClock[i].portEnable = FALSE;
            
            g_ptpClock[i].netEnviroment.ip = netWorkCommand0.ip;
            SetIpAddress(g_ptpClock[i].netEnviroment.ifaceName,netWorkCommand0.ip);

            Init_NetComm(&g_ptpClock[i],&g_ptpClock[i].netComm);
            Init_RecvRdfs(&g_ptpClock[i]);
            SetRouteToEnv(&g_ptpClock[i],&g_ptpClock[i].netEnviroment);

            AddGateWay(g_ptpClock[i].netEnviroment.ifaceName,netWorkCommand0.gwip);
            g_ptpClock[i].netEnviroment.gwip = netWorkCommand0.gwip;
            
            g_ptpClock[i].portEnable = TRUE;
            
            /**还原状态  */
            g_ptpClock[i].portEnable = tmp;

        }
        
        if(netWorkCommand0.gwip != g_ptpClock[i].netEnviroment.gwip)
        {
            printf("set net success port %d gwip \n",i);
            
            g_ptpClock[i].netEnviroment.gwip = netWorkCommand0.gwip;

        }

        
        if(netWorkCommand0.mask != g_ptpClock[i].netEnviroment.mask)
        {
            printf("set net success port %d mask \n",i);
            g_ptpClock[i].netEnviroment.mask = netWorkCommand0.mask;
            SetMaskAddress(g_ptpClock[i].netEnviroment.ifaceName,netWorkCommand0.mask);

        }
        
    }
        
    len = sizeof(netWorkCommand0)+1;
    len = len * PTP_PORT_COUNT;
    //msgPackNetChangeCommand(pUartDevice,0,pStart,len);


}

void ProcessUartNtpCommand(UartDevice *pUartDevice,char *pData)
{
    char command = *pData;
    
    pData = pData + 1;
    switch(command)
    {
        case 0:
            Analyse_NtpParam_Command0(pUartDevice,pData);
            break;
        case 1:
            Analyse_NtpParam_Command1(pUartDevice,pData);
            break;
        case 2:
            Analyse_NtpParam_Command2(pUartDevice,pData);
            break;
        case 3:
            Analyse_NtpParam_Command3(pUartDevice,pData);
            break;
        default:
            break;
    }
}


void ProcessUartPtpCommand(UartDevice *pUartDevice,char *pData)
{
    char command = *pData;

    pData = pData + 1;
    switch(command)
    {
        case 0:
            Analyse_PtpParam_Command0(pUartDevice,pData);
            break;
        case 1:
            Analyse_PtpParam_Command1(pUartDevice,pData);
            break;
        case 2:
            Analyse_PtpParam_Command2(pUartDevice,pData);
            break;
        case 3:
            Analyse_PtpParam_Command3(pUartDevice,pData);
            break;
        case 4:
            Analyse_PtpParam_Command4(pUartDevice,pData);
            break;
        case 5:
            Analyse_PtpParam_Command5(pUartDevice,pData);
            break;
            
        default:
            break;
    }

}

void ProcessUartNetCommand(UartDevice *pUartDevice,char *pData)
{
    Analyse_NetParam_Command0(pUartDevice,pData);
}


void ProcessDclsCommand(UartDevice *pUartDevice,char *pData)
{
    if(*pData != pUartDevice->dclsRate)
        pUartDevice->dclsRate = *pData;
}

void ProcessUartSetCommand(UartDevice *pUartDevice,char *pRecvBuf,short len)
{    
    char *pData;
    short nOffset = 0;
    short ret;
    DataFrameHead *pFrameHead = (DataFrameHead*)pRecvBuf;

    pData = pRecvBuf + sizeof(DataFrameHead);

    switch(pFrameHead->command)
    {
        case 0:

            break;
        case 1:
            printf("Net Set Command !!\n");
            ProcessUartNetCommand(pUartDevice,pData);
            SendAckCommand(pUartDevice);
            break;
        case 2:
            msgPackVersionTimeUp(pUartDevice);
            SendAckCommand(pUartDevice);
            break;
        case 3:
            printf("PTP Set Command !!\n");
            ProcessUartPtpCommand(pUartDevice,pData);
            SendAckCommand(pUartDevice);

            break;
        case 4:
            
            printf("NTP Set Command !!\n");
            
            ProcessUartNtpCommand(pUartDevice,pData);
            SendAckCommand(pUartDevice);

            break;
        case 5:
            printf("dcls rate Command !!\n");
            ProcessDclsCommand(pUartDevice,pData);
            break;
        case 10:

            break;
        case 11:
            break;
        case 12:
            break;
        case 13:
            break;
        case 14:
            break;
        case 15:
            break;
        case 16:
            break;
        case 20:
            break;
        case 21:
            break;
        case 22:
            break;

        case 23:
            break;
        case 24:
            break;
        default:
            printf("unknow command Word !!\n");
            break;

    }
    
}

void ProcessUartRecvData(UartDevice *pUartDevice,struct Uart_Data *pRecvData)
{
    struct Uart_List_Head *pSendListHead = pUartDevice->pSendListHead;
    
    static short index = 0;
    FrameHead *pFrameHead = (FrameHead*)pRecvData->data;

    //printf("pFrameHead: %x %x %x %x\n",pFrameHead->dest,pFrameHead->src,pFrameHead->h_len,pFrameHead->l_len);

    short len = pFrameHead->l_len;
    len += (pFrameHead->h_len<<8);

    /**数据长度为1 ，是控制字  */
    if(len == 1)
    {
        switch (pFrameHead->command)
        {
            /**ENQ  */
            case ENQ:
                printf("==>Recv Enq !!\n");
                pUartDevice->dest_num = pFrameHead->src;
                
                if(pSendListHead->count == 0)
                    SendEotCommand(pUartDevice);
                else
                    SendUartData(pUartDevice);

                break;

            /**ACK  */
            case ACK:
                printf("==>Recv Ack !!\n");
                DeletSendUartData(pUartDevice); 
                break;

            /**NAK  */
            case NAK:
                printf("==>Recv Nak !!\n");
                               
                break;

            /**EOT  */
            case EOT:
                printf("==>Recv Eot !!\n");
                break;

            default:
                break;
        }
        
    }
    /**数据长度不是1 ，是其他 设置参数 */
    else
        ProcessUartSetCommand(pUartDevice,pRecvData->data,len);
        

          
}




void *Thread_UartDataHandle(void *arg)
{
    int len;
    char data;
    
    UartBuf uart_buf;
    int data_len;

    UartDevice *pUartDevice = (UartDevice*)arg;
    struct Uart_List_Head *p_Recvlist_head = pUartDevice->pRecvListHead;
    
    memset(&uart_buf,0,sizeof(uart_buf));

    while(1)
    {
        /**接收数据  */
        len = read(pUartDevice->comfd,&data,UART_LEN);
        if(len > 0)
        {
            CreatRecvFrame(&uart_buf,data);
            if(uart_buf.flags == 0x10)
            {
                uart_buf.flags = 0x00;
                
                FrameHead *pFrameHead = (FrameHead*)uart_buf.buf;

                if(pUartDevice->slot_num == pFrameHead->dest)
                {
                    add_recv_data_after(uart_buf.buf,uart_buf.cur,p_Recvlist_head);
                }
            }
        }        

    }
}


void CheckIsRestartNtpd()
{
    static short index = 0;
    
    if(gNtpdRestart == TRUE)
        index++;
    if(index >=3)
    {
        gNtpdRestart = FALSE;
        Restart_Ntp_Server();
        index = 0;
    }
}


