#include "fpga_time.h"
#include <sys/mman.h>
#include "ptp.h"

#define PHY_BASEADD             0x40600000      /*控制部分基地址*/


#define FGPA_VERSION_ADDRESS_H    (BASE_ADDR + 0x00) 
#define FGPA_VERSION_ADDRESS_L    (BASE_ADDR + 0x01)

#define ETH0_OFFSET_ADD           (BASE_ADDR + 0x100)       /*网口0基地址*/
#define ETH1_OFFSET_ADD           (BASE_ADDR + 0x200)       /*网口1基地址*/
#define ETH2_OFFSET_ADD           (BASE_ADDR + 0x300)       /*网口2基地址*/
#define ETH3_OFFSET_ADD           (BASE_ADDR + 0x400)       /*网口3基地址*/
#define ETH4_OFFSET_ADD           (BASE_ADDR + 0x500)       /*网口4基地址*/
#define ETH5_OFFSET_ADD           (BASE_ADDR + 0x600)       /*网口5基地址*/
#define ETH6_OFFSET_ADD           (BASE_ADDR + 0x700)       /*网口6基地址*/
#define ETH7_OFFSET_ADD           (BASE_ADDR + 0x800)       /*网口7基地址*/

/**秒脉冲记录的时间戳  */
#define PPS_STROE_NAN_ADD      0x40
#define PPS_STROE_SEC_ADD      0x44


/**FGGA 时间寄存器*/
#define FPGA_RUNNING_SETTIME_ADD 0x70
#define FPGA_RUNNING_NAN_ADD     0x70
#define FPGA_RUNNING_SEC_ADD     0x74

/**FPGA 设置时间戳地址  */
#define FPGA_CORE_TIME_ADD         0xD0
#define FPGA_CORE_SETTIME_ADD      0xD7


/**发送包的时间戳记录地址  */
#define SEND_NAO_OFFSET_ADD  0x10
#define SEND_SEC_OFFSET_ADD  0x14
#define SEND_FIFO_CNT        0x1C
#define SEND_TIME_SET        0x10


/**接收包的时间戳记录地址  */
#define RECV_NAO_OFFSET_ADD  0x00
#define RECV_SEC_OFFSET_ADD  0x04
#define RECV_FIFO_CNT        0x0C
#define RECV_TIME_SET        0x00

/**PTP工作模式  */
#define PTP_WORK_MODE_ADD    0x4D

/**PTP 收包控制  */
#define PTP_RECV_CTL_ADD     0x4E

/**PTP 发包控制  */
#define PTP_SEND_CTL_ADD     0x4F

/**PTP correction 控制 */
#define PDELAY_CORRECTION_NAO 0x40 
#define PDELAY_CORRECTION_SEC 0x44

/**PTP T4时间寄存器，用于填写correction计算  */
#define PDELAY_T4_NAO         0x48
#define PDELAY_T4_SEC         0x4C


#define PAGE_SIZE (5*1024)


#define LED_RUN          0xB0
#define LED_STATUS       0xB1





char *BASE_ADDR;
pthread_mutex_t Fpga_Mutex;


void Init_FpgaMutex(void)
{
    pthread_mutex_init(&Fpga_Mutex,NULL);
}

void Fpga_PthreadLock()
{
    pthread_mutex_lock(&Fpga_Mutex);

}

void Fpga_PthreadUnlock()
{
    pthread_mutex_unlock(&Fpga_Mutex);

}


Uint8 *mmapping() 
{ 
    Uint8 *pMapped_add;
    
	int fd=open("/dev/mem",O_RDWR); 
	if(fd == -1)
	{
		printf("open error\n");
		return NULL; 
	}
	pMapped_add=(unsigned char *)mmap(NULL, PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,fd,PHY_BASEADD); 
	if(pMapped_add == MAP_FAILED)
	{
		close(fd); 
		return NULL; 
	}
    close(fd);

	 //pMapped_add = (char *)malloc(1024 * 2);
    return pMapped_add;
}



void GetFpgaSendMessageTime(PtpClock *pPtpClock,TimeInternal *pT3)
{
    Uint8 cnt = 0;
    int i = 0;

    /**是否有包  ,没有包延时*/
    while((cnt&0x03) == 0)
    {
        cnt = *(volatile Uint8 *)(pPtpClock->pFpgaAddress + SEND_FIFO_CNT);
        i++;
	    if(i>=1000)
		    break;
    }

    /** 包大于1个，清理剩下最后1个 */
    while((cnt&0x3) > 0x01)
    {
        
        *(volatile Uint8 *)(pPtpClock->pFpgaAddress + SEND_TIME_SET) = 0x01;
        cnt = *(volatile Uint8 *)(pPtpClock->pFpgaAddress + SEND_FIFO_CNT);
        if(cnt == 1)
            break;
    }

    pT3->seconds = *(volatile Uint32*)(pPtpClock->pFpgaAddress + SEND_SEC_OFFSET_ADD);
    pT3->nanoseconds = *(volatile Uint32*)(pPtpClock->pFpgaAddress + SEND_NAO_OFFSET_ADD);

    *(volatile Uint8 *)(pPtpClock->pFpgaAddress + SEND_TIME_SET) = 0x01;

    msync(BASE_ADDR,PAGE_SIZE,MS_ASYNC);
}

void SetFpgaTime(Integer32 sec)
{
    Fpga_PthreadLock();

    *(Uint32 *)(BASE_ADDR + 0xD0) = sec;
    *(Uint8 *)(BASE_ADDR + 0xD4) = 0;
    *(Uint8 *)(BASE_ADDR + 0xD5) = 0;
    *(Uint8 *)(BASE_ADDR + 0xD7) = 0x01;
    
    Fpga_PthreadUnlock();
    msync(BASE_ADDR,PAGE_SIZE,MS_ASYNC);
}


void WriteT4ToFpga(PtpClock *pPtpClock,TimeInternal *RecvTime)
{
    *(Uint32 *)(pPtpClock->pFpgaAddress + PDELAY_T4_NAO) = RecvTime->nanoseconds;
    *(Uint8*)(pPtpClock->pFpgaAddress + PDELAY_T4_SEC) = RecvTime->seconds & 0xff;
     msync(BASE_ADDR,PAGE_SIZE,MS_ASYNC);

}

/********************************************************************************************
* 函数名称:    WriteCorrectionToFpgaCorrection
* 功能描述:    将收到的correction值写入fgpa寄存器
* 输入参数:    PtpClock *pPtpClock    
               Integer64 *correction  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-6-26
               作者:     Felix
               修改内容: 新生成函数

********************************************************************************************/
void WriteCorrectionToFpgaCorrection(PtpClock *pPtpClock,Integer64 *correction)
{
    *(Uint32 *)(pPtpClock->pFpgaAddress + PDELAY_CORRECTION_NAO) = correction->lsb;
    *(Uint32 *)(pPtpClock->pFpgaAddress + PDELAY_CORRECTION_SEC) = correction->msb;
    
    msync(BASE_ADDR,PAGE_SIZE,MS_ASYNC);
}


void GetFpgaRuningTime(TimeInternal *pTime)
{
    Fpga_PthreadLock();

    *(Uint8*)(BASE_ADDR+FPGA_RUNNING_SETTIME_ADD) = 0x00;
    Uint32 sec = *(Uint32*)(BASE_ADDR+FPGA_RUNNING_SEC_ADD);
    Uint32 nao = *(Uint32*)(BASE_ADDR+FPGA_RUNNING_NAN_ADD);
    Fpga_PthreadUnlock();

    pTime->seconds = sec;
    pTime->nanoseconds = nao;
}

void GetFpgaPpsTime(TimeInternal *pTime)
{

    Uint32 sec = *(Uint32*)(BASE_ADDR+PPS_STROE_SEC_ADD);
    Uint32 nao = *(Uint32*)(BASE_ADDR+PPS_STROE_NAN_ADD);

    pTime->seconds = sec;
    pTime->nanoseconds = nao;
}

void GetSystemPpsTime(TimeInternal *pTime)
{

    Uint32 sec = *(Uint32*)(BASE_ADDR+0x74);
    //Uint32 nao = *(Uint32*)(BASE_ADDR+PPS_STROE_NAN_ADD);

    pTime->seconds = sec;
    pTime->nanoseconds = 0;
    
}
#if 1
void GetFpgaRecvMessageTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,TimeInternal *pTime)
{
    Uint8 sec_tmp = pPtpClock->CurentTime.seconds & 0xff;

    //printf("T4.sec=%d T4.reser=%d\n",sec_tmp,pMsgHeader->reserved1);
    
    if(sec_tmp == pMsgHeader->reserved1)
    {
        pTime->seconds = pPtpClock->CurentTime.seconds;
        pTime->nanoseconds = pMsgHeader->reserved2 & 0x3fffffff;
    }
    else if((sec_tmp == 0x0) && (pMsgHeader->reserved1 == 0xff))
    {
        pTime->seconds = pPtpClock->CurentTime.seconds - 1;
        pTime->nanoseconds = pMsgHeader->reserved2 & 0x3fffffff;
    
    }
    else if((sec_tmp == 0xff) && (pMsgHeader->reserved1 == 0x0))
    {
        pTime->seconds = pPtpClock->CurentTime.seconds + 1;
        pTime->nanoseconds = pMsgHeader->reserved2 & 0x3fffffff;
    }
    else
    {
        pTime->seconds = pPtpClock->CurentTime.seconds;
        pTime->seconds = pTime->seconds & 0xffffff00;
        pTime->seconds = pTime->seconds | pMsgHeader->reserved1;
        
        pTime->nanoseconds = pMsgHeader->reserved2 & 0x3fffffff;
        
    }

}
#endif
#if 0
void GetFpgaRecvMessageTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,TimeInternal *pTime)
{
    Fpga_PthreadLock();
    Uint8 sec_tmp = pPtpClock->CurentTime.seconds & 0xff;
    
    if(sec_tmp == pMsgHeader->reserved1)
    {
        pTime->seconds = pPtpClock->CurentTime.seconds;
        pTime->nanoseconds = pMsgHeader->reserved2 & 0x3fffffff;
    }
    else if(sec_tmp != pMsgHeader->reserved1)
    {
        pTime->seconds = pPtpClock->CurentTime.seconds;
        pTime->seconds = pTime->seconds & 0xffffff00;
        pTime->seconds = pTime->seconds + pMsgHeader->reserved1;
        
        pTime->nanoseconds = pMsgHeader->reserved2 & 0x3fffffff;

    }
    Fpga_PthreadUnlock();
}
#endif


void ReInit_FpgaCore(PtpClock *pPtpClock)
{
    *(Uint8*)(pPtpClock->pFpgaAddress + PTP_WORK_MODE_ADD) = 0x0e;
            
    if(pPtpClock->stepType == ONE_STEP)
    {
        *(Uint8*)(pPtpClock->pFpgaAddress + PTP_RECV_CTL_ADD) = 0x0f;
        *(Uint8*)(pPtpClock->pFpgaAddress + PTP_SEND_CTL_ADD) = 0x0f;
    }
    else if(pPtpClock->stepType == TWO_STEP)
    {
        *(Uint8*)(pPtpClock->pFpgaAddress + PTP_RECV_CTL_ADD) = 0x0f;
        *(Uint8*)(pPtpClock->pFpgaAddress + PTP_SEND_CTL_ADD) = 0x07;
    }
      
        
    *(Uint8*)(BASE_ADDR+0x90) = 0x00;

}

void Init_FpgaCore(PtpClock *pPtpClock)
{
    int i;
    int val = 0;

    
    Init_FpgaMutex();
    
    BASE_ADDR = mmapping();
    if(BASE_ADDR == NULL)
    {
        printf("mapping error,exit!\n");
        exit(-1);
    }

    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        val= (i+1)*0x100;
        pPtpClock[i].pFpgaAddress = BASE_ADDR + val;
    }

    /**初始化工作模式  */
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        *(Uint8*)(pPtpClock[i].pFpgaAddress + PTP_WORK_MODE_ADD) |= 0x0e;
        
        if(pPtpClock[i].stepType == ONE_STEP)
        {
            *(Uint8*)(pPtpClock[i].pFpgaAddress + PTP_RECV_CTL_ADD) = 0x0f;
            *(Uint8*)(pPtpClock[i].pFpgaAddress + PTP_SEND_CTL_ADD) = 0x0f;
        }
        else if(pPtpClock[i].stepType == TWO_STEP)
        {
            *(Uint8*)(pPtpClock[i].pFpgaAddress + PTP_RECV_CTL_ADD) = 0x0f;
            *(Uint8*)(pPtpClock[i].pFpgaAddress + PTP_SEND_CTL_ADD) = 0x07;
        }
    }
    
    //SetFpgaTime(254872);

    /**设置自动调节小数秒的值  ，每次bns*/
    //*(Uint8*)(BASE_ADDR + 0xE8) = 0x0b;

    /**设置为自动调节小数秒  */
    //*(Uint8*)(BASE_ADDR + 0xE9) = 0x01;

    
    /**设置强制告警  */
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        pPtpClock[i].synAlarm          = TRUE;
        pPtpClock[i].announceAlarm     = TRUE;
        pPtpClock[i].delayRespAlarm    = TRUE;
        pPtpClock[i].notAvailableAlarm = TRUE;
    }
    
}

void SetCpuVersion(char *version)
{
    (*(Uint8*)(BASE_ADDR+0x80)) = version[0]<<4;
    (*(Uint8*)(BASE_ADDR+0x80)) += version[1];

    (*(Uint8*)(BASE_ADDR+0x81)) = version[2]<<4;
    (*(Uint8*)(BASE_ADDR+0x81)) += version[3];

}

void GetFpgaVersion(char *version)
{
    version[0] = (*(Uint8*)(BASE_ADDR)>>4) & 0x0f;
    version[1] = (*(Uint8*)(BASE_ADDR)) & 0x0f;
    
    version[2] = (*(Uint8*)(BASE_ADDR+0x01)>>4) & 0x0f;
    version[3] = (*(Uint8*)(BASE_ADDR+0x01)) & 0x0f;
}
void GetPcbVersion(char *version)
{
    version[0] = (*(Uint8*)(BASE_ADDR)>>4) & 0x0f;
    version[1] = (*(Uint8*)(BASE_ADDR)) & 0x0f;
    
    version[2] = (*(Uint8*)(BASE_ADDR+0x01)>>4) & 0x0f;
    version[3] = (*(Uint8*)(BASE_ADDR+0x01)) & 0x0f;

}

Uint8 GetSlot()
{
    return *(Uint8*)(BASE_ADDR + 0x04);
}

void Control_LedRun(Boolean val)
{
    *(Uint8*)(BASE_ADDR + LED_RUN) = val;
}

void Control_LedStatus(LedPin port,Boolean val)
{
    switch(port)
    {
        case LED_PTP_1:
            if(val)
                *(Uint8*)(BASE_ADDR + LED_STATUS) |= 0x01;
            else
                *(Uint8*)(BASE_ADDR + LED_STATUS) &= ~0x01;
            
            break;
        case LED_PTP_2:
            if(val)
                *(Uint8*)(BASE_ADDR + LED_STATUS) |= (0x01<<1);
            else
                *(Uint8*)(BASE_ADDR + LED_STATUS) &= ~(0x01<<1);
            break;
        case LED_PTP_3:
            if(val)
                *(Uint8*)(BASE_ADDR + LED_STATUS) |= (0x01<<2);
            else
                *(Uint8*)(BASE_ADDR + LED_STATUS) &= ~(0x01<<2);
            
            break;
        case LED_PTP_4:
            if(val)
                *(Uint8*)(BASE_ADDR + LED_STATUS) |= (0x01<<3);
            else
                *(Uint8*)(BASE_ADDR + LED_STATUS) &= ~(0x01<<3);

            break;
        case LED_PTP_5:
            if(val)
                *(Uint8*)(BASE_ADDR + LED_STATUS) |= (0x01<<4);
            else
                *(Uint8*)(BASE_ADDR + LED_STATUS) &= ~(0x01<<4);
            
            break;
        case LED_PTP_6:
            if(val)
                *(Uint8*)(BASE_ADDR + LED_STATUS) |= (0x01<<5);
            else
                *(Uint8*)(BASE_ADDR + LED_STATUS) &= ~(0x01<<5);

            break;
        case LED_PTP_7:
            if(val)
                *(Uint8*)(BASE_ADDR + LED_STATUS) |= (0x01<<6);
            else
                *(Uint8*)(BASE_ADDR + LED_STATUS) &= ~(0x01<<6);

            break;
        case LED_PTP_8:
            if(val)
                *(Uint8*)(BASE_ADDR + LED_STATUS) |= (0x01<<7);
            else
                *(Uint8*)(BASE_ADDR + LED_STATUS) &= ~(0x01<<7);
            
            break;
        default:
            break;
    }
    
}

void SetI2cChannel(Uint8 val)
{
    *(unsigned char *)(BASE_ADDR+0x85) = val;
}

void SetFpgaAddressVal(short offset,Uint8 val)
{
    /**去掉最低位  */
    *(unsigned char *)(BASE_ADDR+offset) &= 0xfe;
    
    *(unsigned char *)(BASE_ADDR+offset) |= val;

}

Uint8 GetFpgaAddressVal(short offset)
{
    return *(unsigned char *)(BASE_ADDR+offset);
}



void BlockPtpOutput(PtpClock* pPtpClock)
{
    short i;
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        pPtpClock[i].outBlockFlag = TRUE;
    }

}
void ResumePtpOutput(PtpClock* pPtpClock)
{
    short i;
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        pPtpClock[i].outBlockFlag = FALSE;
    }

}

void BlockNtpOutput(PtpClock* pPtpClock)
{
    short i;
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        *(Uint8*)(pPtpClock[i].pFpgaAddress + PTP_WORK_MODE_ADD) |= 0x10;
    }
}

void ResumeNtpOutput(PtpClock* pPtpClock)
{
    short i;
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        *(Uint8*)(pPtpClock[i].pFpgaAddress + PTP_WORK_MODE_ADD) &= 0xef;
    }

}

void getTime(TimeInternal * time)
{
#if 1
	struct timeval tv;
	gettimeofday(&tv, 0);
	time->seconds = tv.tv_sec;
	time->nanoseconds = tv.tv_usec * 1000;
#endif
}


void SetSysTime(TimeInternal * pTime)
{
    struct timeval tv;
    tv.tv_sec = pTime->seconds;
    tv.tv_usec = pTime->nanoseconds / 1000;
    settimeofday(&tv,NULL);

}

void GetBackBusData(char *m_buf,char *s_buf)
{

    char val = *(Uint8*)(BASE_ADDR+0x05);
    if((val&0x01) == 0x00)
    {
        memcpy(m_buf,BASE_ADDR+0x10,16);
        memcpy(s_buf,BASE_ADDR+0x20,16);

    }
    else if((val&0x01) == 0x01)
    {
        memcpy(m_buf,BASE_ADDR+0x20,16);
        memcpy(s_buf,BASE_ADDR+0x10,16);

    }

    printf("===0x05_addr=%x\n",val);

    val = *(Uint8*)(BASE_ADDR+0x0c);
    printf("===0x0c_addr=%x\n",val);
}

void CollectAlarm(UartDevice *pUartDevice)
{
    PadAlarm *pPadAlarm = &pUartDevice->padAlarm;
    
    char val = *(Uint8*)(BASE_ADDR+0x05);
    if(val&0x02)
        pUartDevice->diskAlarm = TRUE;
    else
        pUartDevice->diskAlarm = FALSE;

    
    val = *(Uint8*)(BASE_ADDR+0x06);

    /**1号槽位 RCU告警  */
    if(val&0x01)
        pPadAlarm->slot_1_RcuAlarm = TRUE;
    else 
        pPadAlarm->slot_1_RcuAlarm = FALSE;

     /**2号槽位RCU告警  */
    if(val&0x02)
        pPadAlarm->slot_2_RcuAlarm = TRUE;
    else
        pPadAlarm->slot_2_RcuAlarm = FALSE;

    /**1号槽位RCU 主备 告警*/
    if(val&0x04)
        pPadAlarm->slot_1_Master_Backup = TRUE;
    else
        pPadAlarm->slot_1_Master_Backup = FALSE;

    /**2号槽位RCU 主备 告警*/
    if(val&0x08)
        pPadAlarm->slot_2_Master_Backup = TRUE;
    else
        pPadAlarm->slot_2_Master_Backup = FALSE;

    /**读取0x07  */
    val = *(Uint8*)(BASE_ADDR+0x07);


    /**1号槽位100m 检波告警 ，1，告警，0，不告警*/
    if(val&0x01)
            pPadAlarm->slot_1_100Alarm = TRUE;
        else 
            pPadAlarm->slot_1_100Alarm = FALSE;
    
     /**2号槽位100m 检波告警 ，1，告警，0，不告警*/
    if(val&0x02)
        pPadAlarm->slot_2_100Alarm = TRUE;
    else
        pPadAlarm->slot_2_100Alarm = FALSE;


    /**1号槽位1pps检波告警，1，告警，0，不告警  */
    if(val&0x10)
        pPadAlarm->slot_1_1ppsAlarm = TRUE;
    else
        pPadAlarm->slot_1_1ppsAlarm = FALSE;

    /**2号槽位1pps检波告警，1，告警，0，不告警  */
    if(val&0x20)
        pPadAlarm->slot_2_1ppsAlarm = TRUE;
    else
        pPadAlarm->slot_2_1ppsAlarm = FALSE;

    /**1号槽位tod检波告警，1，告警，0，不告警  */
    if(val&0x40)
        pPadAlarm->slot_1_TodAlarm = TRUE;
    else
        pPadAlarm->slot_1_TodAlarm = FALSE;
    
    /**2号槽位tod检波告警，1，告警，0，不告警  */
    if(val&0x80)
        pPadAlarm->slot_2_TodAlarm = TRUE;
    else
        pPadAlarm->slot_2_TodAlarm = FALSE;


    /**读取0x08  */
    val = *(Uint8*)(BASE_ADDR+0x08);

    /**主用100m 告警  */
    if(val&0x01)
        pPadAlarm->master100Alarm = TRUE;
    else
        pPadAlarm->master100Alarm = FALSE;
    
    /**主用1pps 告警  */
    if(val&0x02)
        pPadAlarm->master1ppsTodAlarm = TRUE;
    else
        pPadAlarm->master1ppsTodAlarm = FALSE;
    
    /**两路都是主用告警  */
    if(val&0x04)
        pPadAlarm->twoMasterAlarm = TRUE;
    else
        pPadAlarm->twoMasterAlarm = FALSE;
    
    /**两路都是备用告警  */
    if(val&0x08)
        pPadAlarm->twoBackupAlarm = TRUE;
    else
        pPadAlarm->twoBackupAlarm = FALSE;


    /**读取0x09  */
    val = *(Uint8*)(BASE_ADDR+0x09);
    
    /**1号槽位tod crc告警  1，告警，0，不告警*/
    if(val&0x01)
        pPadAlarm->slot_1_TodCrcAlarm = TRUE;
    else
        pPadAlarm->slot_1_TodCrcAlarm = FALSE;
    
    /**2号槽位tod crc告警  1，告警，0，不告警*/
    if(val&0x02)
        pPadAlarm->slot_2_TodCrcAlarm = TRUE;
    else
        pPadAlarm->slot_2_TodCrcAlarm = FALSE;


}
