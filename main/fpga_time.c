#include "fpga_time.h"
#include <sys/mman.h>
#include "main.h"

#define PHY_BASEADD             0x40600000      /*控制部分基地址*/


#define FGPA_VERSION_ADDRESS_H    (BASE_ADDR + 0x00) 
#define FGPA_VERSION_ADDRESS_L    (BASE_ADDR + 0x01)

#define ETH0_OFFSET_ADD           (BASE_ADDR + 0x100)       /*网口0基地址*/
#define ETH1_OFFSET_ADD           (BASE_ADDR + 0x200)       /*网口1基地址*/


/**秒脉冲记录的时间戳  */
#define PPS_PHASE_OFFSET_ADD      0x10
#define PPS_PHASE_SYMBOL_ADD      0x13


/**秒脉冲记录的时间戳  */
#define PPS_PTP_STROE_NAN_ADD      0x50
#define PPS_PTP_STROE_SEC_ADD      0x54

/**秒脉冲记录的时间戳  */
#define PPS_GPS_STROE_NAN_ADD      0x60
#define PPS_GPS_STROE_SEC_ADD      0x69


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

#define LED_ALARM               0x88
#define LED_RUN                 0x89
#define LED_SAT_STATUS          0x8A
#define LED_LOCK_STATUS         0x8C



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

    Uint32 sec = *(Uint32*)(BASE_ADDR+PPS_PTP_STROE_SEC_ADD);
    Uint32 nao = *(Uint32*)(BASE_ADDR+PPS_PTP_STROE_NAN_ADD);

    pTime->seconds = sec;
    pTime->nanoseconds = nao;
}

void Init_FpgaCore()
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
}

void Control_LedRun(Boolean val)
{
    *(Uint8*)(BASE_ADDR + LED_RUN) = val;
}

void Control_LedAlarm(char val)
{
    *(Uint8*)(BASE_ADDR + LED_ALARM) = val;
}

void Control_LedSatStatus(char val)
{
    *(Uint8*)(BASE_ADDR + LED_SAT_STATUS) = val;
}

void Control_LedLockStatus(Boolean val)
{
    *(Uint8*)(BASE_ADDR + LED_LOCK_STATUS) = val;
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


void CollectAlarm(struct root_data *pRootData)
{
    struct clock_alarm_data *pAlarmData = &pRootData->clock_info.alarmData;
    
    char val = *(Uint8*)(BASE_ADDR+0x05);
    if(val&0x01)
        pAlarmData->alarmDisk= TRUE;
    else
        pAlarmData->alarmDisk = FALSE;
    
    val = *(Uint8*)(BASE_ADDR+0x08);

    /**1号槽位 RCU告警  */
    if(val&0x01)
        pAlarmData->alarmBd1pps = TRUE;
    else 
        pAlarmData->alarmBd1pps = FALSE;

  
    /**读取0x08  */
    val = *(Uint8*)(BASE_ADDR+0x09);

    /**1号槽位100m 检波告警 ，1，告警，0，不告警*/
    if(val&0x01)
            pAlarmData->alarmVcxo100M = TRUE;
        else 
            pAlarmData->alarmVcxo100M = FALSE;
    
     /**2号槽位100m 检波告警 ，1，告警，0，不告警*/
    if(val&0x02)
        pAlarmData->alarmRb10M = TRUE;
    else
        pAlarmData->alarmRb10M = FALSE;


    /**1号槽位1pps检波告警，1，告警，0，不告警  */
    if(val&0x04)
        pAlarmData->alarmXo10M = TRUE;
    else
        pAlarmData->alarmXo10M = FALSE;

    /**读取0x0A  */
    val = *(Uint8*)(BASE_ADDR+0x0a);

    if(val&0x01)
        pAlarmData->vcxoLock= TRUE;
    else
        pAlarmData->vcxoLock= FALSE;

    if(pRootData->satellite_data.antenna != 2 || pAlarmData->alarmBd1pps == TRUE)
        pAlarmData->alarmSatellite = TRUE;
    else
        pAlarmData->alarmSatellite = FALSE;
       


}



