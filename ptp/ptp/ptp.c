/********************************************************************************************
*                           版权所有(C) 2015, 
*                                 版权所有
*********************************************************************************************
*    文 件 名:       ptp.c
*    功能描述:       函数入口，线程初始化，环境初始化
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-30
*    函数列表:
                     Init_Identity
                     Init_PPSThread
                     Init_ptpClock
                     Init_RecvRdfs
                     Init_SendThread
                     Init_Thread_Attr
                     main
                     Ptp_Manage_State
                     ThreadRecev
                     ThreadSend
                     Wait_User_Config
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-30
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/
#include "msg.h"
#include "arith.h"

#include "ptp_io.h"
#include "common.h"
#include "comm_fun.h"
#include "ptp_proto.h"
#include "data_type.h"
#include "net_process.h"
#include "unicast_process.h"
#include "fpga_time.h"
#include "ptp.h"
#include "fpga_time.h"
#include "usually_process.h"

PtpClock g_ptpClock[PTP_PORT_COUNT];

timer_t timerid_ms,timerid_pps;

UartDevice g_UartDevice;

struct PtpStatus
{
    char priority1;
    char priority2;
    char timeSource;
    char utcOffset;
    char clockClass;
    char clockAccuracy;
    char blockOutput;
    char reserve;
};

struct PtpReference
{
    int sockfd;
    struct sockaddr_in dest_addr;

};

struct PtpReference g_PtpRef;

struct PtpReferenceAlarm
{
    Uint8 type; /**0:告警信息  */
    Uint8 synAlarm;
    Uint8 announceAlarm;
    Uint8 delayRespAlarm;
    Uint8 notAvailableAlarm;
};

struct PtpReferenceData
{
    Uint8 type;/** 1 表示数据信息 */
    Uint8 reserv1;
    Uint8 reserv2;
    Uint8 reserv3;
    TimeInternal currentTime;
    TimeInternal MeanPathDelay;
    TimeInternal TimeOffset;
    
};

static void Pps_Signal_Handle(int signum)
{
    char buf[10];
    UartDevice *pUartDevice = &g_UartDevice;

    /**非实时性要求处理  */
    pUartDevice->pps_routine_flag = TRUE;
    
}



extern Integer8 bmcDataSetComparison(MsgHeader *headerA,MsgAnnounce *announceA,
		     MsgHeader *headerB,MsgAnnounce *announceB,
		     PtpClock *ptpClock);



Uint8 BmcBestClock(PtpClock *pPtpClock)
{
    short i;
    short bestIndex = 0;
    ForeignMasterRecord BestMasterRecord;

    memset(&BestMasterRecord,0,sizeof(BestMasterRecord));
        
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        if(pPtpClock[i].clockType == PTP_SLAVE)
        {
            bestIndex = i;
            memcpy(&BestMasterRecord,&pPtpClock[i].BestMasterRecord,sizeof(BestMasterRecord));
            break;
        }
    }

    for(i=bestIndex;i<PTP_PORT_COUNT;i++)
    {
        if(pPtpClock[i].clockType == PTP_SLAVE)
        {
            if(bmcDataSetComparison(&BestMasterRecord.header
                                    ,&BestMasterRecord.announce
                                    ,&pPtpClock[i].BestMasterRecord.header
                                    ,&pPtpClock[i].BestMasterRecord.announce
                                    ,&pPtpClock[i]) < 0)
            {
                bestIndex = i;
                memcpy(&BestMasterRecord,&pPtpClock[i].BestMasterRecord,sizeof(BestMasterRecord));

            }
        }
    }
    
    return i;

    
}





/********************************************************************************************
* 函数名称:    ThreadSend
* 功能描述:    发送线程，发送所有消息
* 输入参数:    union sigval v  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void ThreadSend(union sigval v)
{
    PtpClock *pPtpClock = NULL;
    static Uint32 timerIndex = 0;
    int i;

    timerIndex++;
    
    /** 封锁输出，则不发送任何数据 */


    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        pPtpClock = (PtpClock *)&g_ptpClock[i];


        /** 封锁输出，则不收任何数据 */
        if(pPtpClock->outBlockFlag)
            continue;

        switch (pPtpClock->clockType)
        {
            case PTP_MASTER:
                MasterMode_Send(timerIndex,pPtpClock);
                break;
            case PTP_SLAVE:
                SlaveMode_Send(timerIndex,pPtpClock);
                break;
            case PTP_PASSIVE:
                usleep(5);
                break;
                
            default:
                usleep(100);
                break;
        }

    }
    

}


/********************************************************************************************
* 函数名称:    ThreadRecev
* 功能描述:    接收线程，接收所有消息并处理，采用select机制
* 输入参数:    void *p  
* 输出参数:    无
* 返回值:      void
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void *ThreadRecev0(void *p)
{
    PtpClock *pPtpClock = (PtpClock *)&g_ptpClock[0];
    
    Init_RecvRdfs(pPtpClock);
  
    
    while(1)
    {
        
        /** 封锁输出，则不收任何数据 */
        if(pPtpClock->outBlockFlag)
        {
            usleep(10);
            continue;
        }
        
        switch (pPtpClock->clockType)
        {
            case PTP_MASTER:
                MasterMode_Recv(pPtpClock);
                break;
            case PTP_SLAVE:
                SlaveMode_Recv(pPtpClock);
                break;
            case PTP_PASSIVE:
                sleep(1);
                break;
                
            default:
                sleep(1);
                break;
        }
    }
}

void updatePtpOffset_ToMainRountine(PtpClock *pPtpClock)
{
    struct PtpReferenceData ptpRefData;
    struct PtpReference *pPtpref = &g_PtpRef;
    int len;
    
    ptpRefData.type = 1;
    ptpRefData.currentTime = pPtpClock->T1;
    ptpRefData.TimeOffset = pPtpClock->TimeOffset;
    ptpRefData.MeanPathDelay = pPtpClock->MeanPathDelay;

    len = sendto(pPtpref->sockfd,&ptpRefData,sizeof(ptpRefData),0,(struct sockaddr *)&pPtpref->dest_addr,sizeof(struct sockaddr_in));
}


void *ThreadSendPtpReference(void *p)
{
    char opt = 1;
    int len;
    PtpClock *pPtpClock = (PtpClock *)&g_ptpClock[0];
    struct PtpReferenceAlarm ptpAlarm;
    struct PtpReference *pPtpref = &g_PtpRef;
    
    memset(pPtpref,0,sizeof(struct PtpReference));
    memset(&ptpAlarm,0,sizeof(ptpAlarm));
    
    if ((pPtpref->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
            perror("socket error\n");
    }

    pPtpref->dest_addr.sin_family = AF_INET;
    pPtpref->dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    pPtpref->dest_addr.sin_port = htons(9900);


    while(1)
    {
        ptpAlarm.type = 0;
        ptpAlarm.synAlarm = pPtpClock->synAlarm;
        ptpAlarm.announceAlarm  = pPtpClock->announceAlarm;
        ptpAlarm.delayRespAlarm = pPtpClock->delayRespAlarm;
        ptpAlarm.notAvailableAlarm = pPtpClock->notAvailableAlarm;

        len = sendto(pPtpref->sockfd,&ptpAlarm,sizeof(ptpAlarm),0,(struct sockaddr *)&pPtpref->dest_addr,sizeof(struct sockaddr_in));
        sleep(1);
    }
    
}


void *ThreadRecePtpStatus(void *p)
{

    int fd = -1;
    char opt = 1;
    PtpClock *pPtpClock = (PtpClock *)&g_ptpClock[0];
    char rBuf[1500];
    int len;
    struct PtpStatus *pPtpStatus;
    
    struct sockaddr_in sockaddr;
    
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
            perror("socket error\n");
    }
    memset(&sockaddr,0,sizeof(sockaddr));
    
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(9900);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt,sizeof(opt));
    //setsockopt(fd,SOL_SOCKET,SO_BROADCAST,(const char*)&opt,sizeof(opt));
    
    if (0 > bind(fd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)))
    {
        printf("UDP socket bind udp error!\n");
        close(fd);
    
    }
    
    pPtpStatus = (struct PtpStatus *)(rBuf);
    while(1)
    {
        
        len = recvfrom(fd,rBuf,1500,0,NULL,NULL);
        if(len > 0)
        {
            printf("ptp recv status %d\n",pPtpClock->outBlockFlag);
            if(pPtpStatus->blockOutput == 1)
            {
                pPtpClock->outBlockFlag = 1;
                continue;
            }
            else
                pPtpClock->outBlockFlag = 0;

            
            pPtpClock->grandmasterPriority1 = pPtpStatus->priority1;
            pPtpClock->grandmasterPriority2 = pPtpStatus->priority2;
            pPtpClock->timePropertiesDS.timeSource = pPtpStatus->timeSource;
            pPtpClock->timePropertiesDS.currentUtcOffset = pPtpStatus->utcOffset;
            pPtpClock->grandmasterClockQuality.clockClass = pPtpStatus->clockClass;
            pPtpClock->grandmasterClockQuality.clockAccuracy = pPtpStatus->clockAccuracy;
            
        }

    }
}


void *ThreadRecev1(void *p)
{
    PtpClock *pPtpClock = (PtpClock *)&g_ptpClock[1];
    
    Init_RecvRdfs(pPtpClock);
  
    
    while(1)
    {

        /** 封锁输出，则不收任何数据 */
        if(pPtpClock->outBlockFlag)
        {
            usleep(10);
            continue;
        }
        
        switch (pPtpClock->clockType)
        {
            case PTP_MASTER:
                MasterMode_Recv(pPtpClock);
                break;
            case PTP_SLAVE:
                SlaveMode_Recv(pPtpClock);
                break;
            case PTP_PASSIVE:
                sleep(1);
                break;
                
            default:
                sleep(1);
                break;
        }
    }
}


/********************************************************************************************
* 函数名称:    Init_SendThread
* 功能描述:    初始化发送线程，初始化发送线程执行间隔5ms
* 输入参数:    timer_t *pTimerid_ms  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_SendThread(timer_t *pTimerid_ms)
{
    
    struct itimerspec it; 
    struct sigevent evp_ms; 
    
    memset(&evp_ms, 0, sizeof(struct sigevent));
    
    evp_ms.sigev_value.sival_int = 1;
    evp_ms.sigev_notify = SIGEV_THREAD;
    evp_ms.sigev_notify_function = ThreadSend;


     if (timer_create(CLOCK_REALTIME, &evp_ms, pTimerid_ms) == -1) 
    { 
        perror("fail to timer_create ms"); 
        exit(-1); 
    } 

    it.it_interval.tv_sec = 0;
    it.it_interval.tv_nsec = 3906250; 
    it.it_value.tv_sec = 0; 
    it.it_value.tv_nsec = 3906250; 
    if (timer_settime(*pTimerid_ms, 0, &it, NULL) == -1) 
    { 
        perror("fail to timer_settime ms"); 
        exit(-1); 
    }

}



/********************************************************************************************
* 函数名称:    Init_PPSThread
* 功能描述:    初始化秒脉冲线程，秒脉冲执行周期为1s，用于日常处理
* 输入参数:    timer_t *pTimerid_pps  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_PPSThread(timer_t *pTimerid_pps)
{
    struct itimerspec it; 
    struct sigevent evp_pps; 

    memset(&evp_pps, 0, sizeof(struct sigevent));    
     
    evp_pps.sigev_value.sival_int = 2;
    evp_pps.sigev_notify = SIGEV_THREAD;
    evp_pps.sigev_notify_function = Pps_Signal_Handle;
    
    
    if (timer_create(CLOCK_REALTIME, &evp_pps, pTimerid_pps) == -1) 
    { 
        perror("fail to timer_create pps"); 
        exit(-1); 
    } 
    
    it.it_interval.tv_sec = 1; 
    it.it_interval.tv_nsec = 0; 
    it.it_value.tv_sec = 1; 
    it.it_value.tv_nsec = 0; 
    if (timer_settime(*pTimerid_pps, 0, &it, NULL) == -1) 
    { 
        perror("fail to timer_settime pps"); 
        exit(-1); 
    } 
    
}

/********************************************************************************************
* 函数名称:    Init_Thread_Attr
* 功能描述:    初始化线程属性，占用优先级
* 输入参数:    pthread_attr_t *pThreadAttr  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_Thread_Attr(pthread_attr_t *pThreadAttr)
{
    int ret;
    int policy;

    struct sched_param thread_param;
    
    ret = pthread_attr_init(pThreadAttr);
    if(ret != 0)
    {
        PLOG("pthread_attr_init error!\n");
    }
 
    ret = pthread_attr_setscope(pThreadAttr,PTHREAD_SCOPE_SYSTEM);
    if(ret != 0)
    {
        PLOG("PTHREAD_SCOPE_SYSTEM error!\n");
    }
 
    ret = pthread_attr_setschedpolicy(pThreadAttr,SCHED_RR);
    if(ret != 0)
    {
        PLOG("pthread_attr_setschedpolicy error!\n");
    }
        
    pthread_attr_getschedparam(pThreadAttr,  &thread_param);
    PLOG("schedparam :%d\n",thread_param.sched_priority);

    /**优先级  */
    pthread_attr_getschedpolicy(pThreadAttr,&policy);
    if(policy == SCHED_FIFO)
         PLOG("Schedpolicy:SCHED_FIFO\n");
    if(policy==SCHED_RR)
         PLOG("Schedpolicy:SCHED_RR\n");
    if(policy==SCHED_OTHER)
         PLOG("Schedpolicy:SCHED_OTHER\n");
 
    /*param.sched_priority=30;*/
    pthread_attr_setschedparam(pThreadAttr,  &thread_param);

}


void Init_PtpDev()
{
    int fd;
    int Oflags; 
    
    UartDevice *pUartDevice = &g_UartDevice;
    
    
    signal(SIGIO, Pps_Signal_Handle);
    
    fd = open("/dev/ptp_dev", O_RDWR);
    if(fd<0)
        printf("can not find ptp_dev\n");

    pUartDevice->dev_fd = fd;

    fcntl(fd, F_SETOWN, getpid());    
    Oflags = fcntl(fd, F_GETFL);     
    fcntl(fd, F_SETFL, Oflags | FASYNC);
    
}


void Init_StartStatus(UartDevice *pUartDevice)
{
    /**强制告警，强制封锁输出  */
    BlockNtpOutput(pUartDevice->pPtpClock);
    BlockPtpOutput(pUartDevice->pPtpClock);
    pUartDevice->powerOn = 1;
    //pUartDevice->dclsRate = 6;

    //Control_LedStatus(LED_PTP_1,0);
    //Control_LedStatus(LED_PTP_2,0);


}


/********************************************************************************************
* 函数名称:    main
* 功能描述:     程序入口
* 输入参数:    int argc      
               char *argv[]  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-30
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
int main(int argc,char *argv[])
{
    int ret;
    int i;
    
    pthread_attr_t threadAttr;
    pthread_t Id_ThreadRecv0;
    pthread_t Id_ThreadRecv1;
    
    pthread_t Id_ThreadSend;
    pthread_t Id_ThreadUsually;
    
    PtpClock *pPtpClock = g_ptpClock;

    for(i=0;i<PTP_PORT_COUNT;i++)
    {
        /**初始化ptp参数  */
        /**初始化默认参数  */
        Init_ptpClock(&pPtpClock[i],i);
        
        Load_PtpParam_FromFile(&pPtpClock[i]);

        Init_NetEnviroment(&pPtpClock->netEnviroment);
        /**从当前文件读取ip netmask gateway   */
        //Load_NetWork(&pPtpClock[i],&pPtpClock[i].netEnviroment);
    }

        
    /**等待串口配置  */
    //Wait_User_Config(pPtpClock);
    
    for(i=0;i<PTP_PORT_COUNT;i++)
    {

        /**初始化网络环境，通信socket等  */
        Init_NetComm(&pPtpClock[i],&pPtpClock[i].netComm);

        /**初始化本地portIdentiy  */
        Init_Identity(&pPtpClock[i],NUMBER_PORTS);

        /**初始化单播客户端，用于主模式下  */
        Init_UnicastClient(&pPtpClock[i]);

    }

    /**初始化fpga time  */
    Init_FpgaCore(g_ptpClock);
    
    /**初始化ptp  */
    //Init_PtpDev();
    Init_SendThread(&timerid_ms);
    Init_PPSThread(&timerid_pps);

    /**初始化线程属性，用于接收线程使用  */
    Init_Thread_Attr(&threadAttr);

    /**初始化线程PPS 线程  */
    ret = pthread_create(&Id_ThreadUsually,&threadAttr,ThreadUsuallyProcess,NULL);
    if(ret == 0)
    {
        printf("ThreadUsually success!\n");
    }
    else
    {
        printf("ThreadUsually error!\n");
    }


    ret = pthread_create(&Id_ThreadRecv0,&threadAttr,ThreadRecev0,NULL);
    if(ret == 0)
	{
		printf("ThreadRecev success!\n");
	}
	else
	{
		printf("ThreadRecev error!\n");
	}

    /**maseter 接收  */
    if(g_ptpClock[0].clockType == 1)
    {
        ret = pthread_create(&Id_ThreadRecv1,&threadAttr,ThreadRecePtpStatus,NULL);
        if(ret == 0)
        {
            printf("ThreadRecePtpStatus success!\n");
        }
        else
        {
            printf("ThreadRecePtpStatus error!\n");
        }

    }
    /**slave 发送  */
    else if(g_ptpClock[0].clockType == 0)
    {
        ret = pthread_create(&Id_ThreadRecv1,&threadAttr,ThreadSendPtpReference,NULL);
        if(ret == 0)
        {
            printf("ThreadSendPtpReference success!\n");
        }
        else
        {
            printf("ThreadSendPtpReference error!\n");
        }

    }
    
    pthread_join(Id_ThreadRecv0,NULL);
    pthread_join(Id_ThreadRecv1,NULL);


}

