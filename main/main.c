#include "main.h"
#include "log.h"
#include "net.h"
#include "manager.h"
#include "fpga_time.h"

struct root_data *g_RootData;
#define COMM_GET_SEC            0x83
#define COMM_SET_SEC            0x84
#define COMM_MODIFY_SEC         0x85



void AddData_ToSendList(struct root_data *pRoot,char devType,char *buf,short len)
{
    add_send(buf,len,&pRoot->dev[devType]);
}

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

void InitSlotLocal(struct SlotList *pSlotLocal)
{
    int ret;
    char *ptpConfig = "/mnt/ptp.cfg";
    char *ntpConfig = "/mnt/ntp.cfg";
    
    Load_PtpParam_FromFile(pSlotLocal->pPtpSetcfg,ptpConfig);
    
}

void InitSlotList(struct root_data *pRootData)
{
    int i;

    struct SlotList *pSlotList = &pRootData->slot_list[ENUM_SLOT_COR_ADDR];
    pSlotList->slot_type = SLOT_TYPE_CORE;
    pSlotList->pNtpSetCfg = (struct NtpSetCfg*)malloc(sizeof(struct NtpSetCfg));
    memset(pSlotList->pNtpSetCfg,0,sizeof(struct NtpSetCfg));
    
    pSlotList->pPtpSetcfg = (struct PtpSetCfg*)malloc(sizeof(struct PtpSetCfg));
    memset(pSlotList->pPtpSetcfg,0,sizeof(struct PtpSetCfg));
    
    pSlotList->pPtpStatusCfg = (struct PtpStatusCfg*)malloc(sizeof(struct PtpStatusCfg));
    memset(pSlotList->pPtpStatusCfg,0,sizeof(struct PtpStatusCfg));

    InitSlotLocal(pSlotList);
    
    for(i = 1;i < SLOT_COUNT;i++)
    {
        pRootData->slot_list[i].slot_type = 0;
        pRootData->slot_list[i].pNtpSetCfg = NULL;
        pRootData->slot_list[i].pPtpSetcfg = NULL;
        pRootData->slot_list[i].pPtpStatusCfg = NULL;
    }

}



void InitDev(struct root_data *pRootData)
{
    int ip;
    
    init_dev_head(&pRootData->dev_head);
    GetIpAddress(pRootData->comm_iface,&ip);

    
    /**初始化UI 设备 ,UI设备通过串口通信id =0*/
#if 0
    pRootData->dev[ENUM_LCD].com_attr.com_port = ENUM_LCD;  //串口1
    pRootData->dev[ENUM_LCD].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_LCD],&pRootData->dev_head,COMM_DEVICE,ENUM_LCD);
#endif
#if 0
    pRoot->dev[ENUM_UI].net_attr.sin_port= 9977;  //串口1
    pRoot->dev[ENUM_UI].net_attr.ip = inet_addr("192.168.15.127");
    init_add_dev(&pRoot->dev[ENUM_UI],&pRoot->dev_head,UDP_DEVICE,ENUM_UI);
#endif


#if 0
    /**接收机串口设备 通信id=2  */
    pRootData->dev[ENUM_GPS].com_attr.com_port = ENUM_GPS;  //串口1
    pRootData->dev[ENUM_GPS].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_GPS],&pRootData->dev_head,COMM_DEVICE,ENUM_GPS);
    
    //钟控串口，控制钟控
    pRootData->dev[ENUM_RB].com_attr.com_port = ENUM_RB;   
    pRootData->dev[ENUM_RB].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_RB],&pRootData->dev_head,COMM_DEVICE,ENUM_RB);
#endif

#if 0
   pRootData->dev[ENUM_PC_DISCOVER].net_attr.sin_port = pRootData->comm_port;  //
   pRootData->dev[ENUM_PC_DISCOVER].net_attr.ip = ip;
   init_add_dev(&pRootData->dev[ENUM_PC_DISCOVER],&pRootData->dev_head,UDP_DEVICE,ENUM_PC_DISCOVER);
#endif
   
   pRootData->dev[ENUM_PC_CTL].net_attr.sin_port = pRootData->comm_port;  //
   pRootData->dev[ENUM_PC_CTL].net_attr.ip = ip;
   init_add_dev(&pRootData->dev[ENUM_PC_CTL],&pRootData->dev_head,TCP_DEVICE,ENUM_PC_CTL);

   #if 0 
    /**1pps + tod 测试口  */
    pRootData->dev[ENUM_PPS_TOD].com_attr.com_port = ENUM_PPS_TOD;   
    pRootData->dev[ENUM_PPS_TOD].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_PPS_TOD],&pRootData->dev_head,COMM_DEVICE,ENUM_PPS_TOD);
 
    /**中断设备，打开ptp_dev 字符设备驱动  */
    strcpy(pRootData->dev[ENUM_PTP_DEV].int_attr.path,"/dev/ptp_dev");
    memset(pRootData->dev[ENUM_PTP_DEV].int_attr.buf,0,sizeof(pRootData->dev[ENUM_PTP_DEV].int_attr.buf));
    init_add_dev(&pRootData->dev[ENUM_PTP_DEV],&pRootData->dev_head,INIT_DEVICE,ENUM_PTP_DEV);
    
 
    //网络设备初始化,用于探针
    pRootData->dev[ENUM_PROBE].net_attr.sin_port = 9977;
    pRootData->dev[ENUM_PROBE].net_attr.ip = inet_addr("192.168.15.127");
    init_add_dev(&pRootData->dev[ENUM_PROBE],&pRootData->dev_head,UDP_DEVICE,ENUM_PROBE);
    #endif
    /**本地PTP   */
    pRootData->dev[ENUM_IPC_PTP].net_attr.sin_port = 8899;
    pRootData->dev[ENUM_IPC_PTP].net_attr.ip = inet_addr("127.0.0.1");
    init_add_dev(&pRootData->dev[ENUM_IPC_PTP],&pRootData->dev_head,UDP_DEVICE,ENUM_IPC_PTP);

    pRootData->dev[ENUM_IPC_NTP].net_attr.sin_port = 9999;
    pRootData->dev[ENUM_IPC_NTP].net_attr.ip = inet_addr("127.0.0.1");
    init_add_dev(&pRootData->dev[ENUM_IPC_NTP],&pRootData->dev_head,UDP_DEVICE,ENUM_IPC_NTP);

}

void *DataHandle_Thread(void *arg)
{

	struct root_data *pRootData = (struct root_data *)arg;
	struct dev_head *p_dev_head = NULL;
	struct device *p_dev = NULL;
	struct data_list *p_data_list = NULL, *tmp_data_list = NULL;
	p_dev_head = (struct dev_head *)&pRootData->dev_head;

    int i;
	while(1)
    {
		sem_wait(&p_dev_head->sem[0]);
        //遍历设备链表
		list_for_each_entry(p_dev,&p_dev_head->list_head,list_head)	
		{
            //遍历数据接收链表
			list_for_each_entry_safe(p_data_list,tmp_data_list,&p_dev->data_head[0].list_head,list_head)
			{
				switch (p_dev->dev_id)
                {
    				case ENUM_BUS:
                        
#if 0
                        for(i=0;i<p_data_list->recv_lev.len;i++)
                            printf("%d ",p_data_list->recv_lev.data[i]);
                        printf("\n");

                        
                        ProcessUiData(pRoot,&p_data_list->recv_lev);
#endif
    					break;
                    case ENUM_LCD:
                         //printf("dev_id=  %d,com1\n",p_dev->dev_id);
                         ProcessLcdMessage(pRootData,p_data_list->recv_lev.data,p_data_list->recv_lev.len);
                        break;
    				case ENUM_GPS:
                        //printf("dev_id=  %d,com1:receive:%s",p_dev->dev_id,p_data_list->recv_lev.data);
                        SatelliteHandle(&pRootData->satellite_data,p_data_list->recv_lev.data);
    					break;
    				case ENUM_RB:
                        
    					printf("dev_id=  %d,com2:receive:%s\n",p_dev->dev_id,p_data_list->recv_lev.data);
                        
                        break;
    				case ENUM_PC_CTL:					
#if 0
                        printf("dev_id=  %d,com3:receive: ",p_dev->dev_id);

                        for(i=0;i<p_data_list->recv_lev.len;i++)
                            printf("%x ",p_data_list->recv_lev.data[i]);
                        printf("\n");
#endif
                        printf("handle TCP pc_ctl_message %d\n",p_dev->dev_id);

                        handle_pc_ctl_message(pRootData,p_data_list->recv_lev.data,p_data_list->recv_lev.len);


                        break;
                    case ENUM_PC_DISCOVER:
                        printf("handle discovery %d\n",p_dev->dev_id);
                        handle_discovery_message(pRootData,p_data_list->recv_lev.data,p_data_list->recv_lev.len);
                        break;
    				case ENUM_LOCAL_DEV:
    					printf("dev_id=  %d,ptp recv\n",p_dev->dev_id);
    					break;
                    case ENUM_IPC_NTP:
                        
                        break;
                    case ENUM_IPC_PTP:
                        
                        break;
    				default:
    					break;
				}
				del_data(p_data_list, &p_dev->data_head[0], p_dev);
			}
		}
	}
	return NULL;
}

void *DataRecv_Thread(void *arg){
	struct device *p_dev = NULL, *tmp_dev = NULL;
	struct dev_head *p_dev_head = (struct dev_head *)arg;
    short ret;
    struct   timeval   timeout;   
 
	while(1)
    {
        timeout.tv_sec=1;   
        timeout.tv_usec=0;  
		p_dev_head->tmp_set = p_dev_head->fd_set;
        
        ret = select(p_dev_head->max_fd + 1,&p_dev_head->tmp_set, NULL, NULL,&timeout);
        if(ret <0)
        {
             perror("select fail ..!!\n");
        }
        if(ret)
        {
            //遍历设备链表
            list_for_each_entry_safe(p_dev,tmp_dev,&p_dev_head->list_head,list_head)    
                p_dev->recv_data(p_dev, p_dev_head);

        }

	}
	return NULL ;
}




void *DataSend_Thread(void *arg) {
	int fd = -1, len = 0, count = 0;
	struct device *p_dev = NULL;
	struct data_list *p_data_list = NULL, *tmp_data_list = NULL, *p_net_data =NULL;
	struct dev_head *p_dev_head = (struct dev_head *) arg;
	while(1)
    {
		sem_wait(&p_dev_head->sem[1]);
		list_for_each_entry(p_dev,&p_dev_head->list_head,list_head)		//遍历设备链表
		{
            list_for_each_entry_safe(p_data_list,tmp_data_list,&p_dev->data_head[1].list_head,list_head)    //遍历数据发送链表
            {
                
                if(p_dev->type == UDP_DEVICE)
                {
                    len = p_data_list->send_lev.len;
                    sendto(p_dev->fd,p_data_list->send_lev.data,len,0,(struct sockaddr *)&p_dev->dest_addr,sizeof(struct sockaddr_in));
                    del_data(p_data_list, &p_dev->data_head[1],p_dev);
                }
                else if(p_dev->type == TCP_DEVICE)
                {
                    len = p_data_list->send_lev.len;
                    count = send(p_dev->fd, p_data_list->send_lev.data, len,0);
                    
                    del_data(p_data_list, &p_dev->data_head[1], p_dev);

                }
                else
                {
                    len = p_data_list->send_lev.len;
                    count = write(p_dev->fd, p_data_list->send_lev.data, len);
                    
                    del_data(p_data_list, &p_dev->data_head[1], p_dev);
                    
                }
                
            }

        }
        
	}
	return NULL;
}

static void Pps_Signal_Handle(int signum)
{
    /**非实时性要求处理  */
    g_RootData->flag_usuallyRoutine = TRUE;
    
}


void Init_PpsDev(char *dev_name)
{
    int fd;
    int Oflags; 
       
    fd = open(dev_name, O_RDWR);
    if(fd<0)
    {
        printf("can not find ptp_dev\n");
        return;
    }

    g_RootData->dev_fd = fd;

    fcntl(fd, F_SETOWN, getpid());    
    Oflags = fcntl(fd, F_GETFL);     
    fcntl(fd, F_SETFL, Oflags | FASYNC);
    
    signal(SIGIO, Pps_Signal_Handle);
}

void MaintainCoreTime(struct root_data *pRoot,TimeInternal *ptptTime)
{        
    struct clock_info *pClockInfo = &pRoot->clock_info;
    struct clock_alarm_data *pAlarmData = &pClockInfo->alarmData;
    
    static Uint32 previousBusSec = 0;

    Uint32 ntp_second_from_1970 =0;
    Uint32 Time = 0;

    static short  secErrorCnt = 0;
    static short  ntpsecErrorCnt = 0;

    static unsigned short modify_internal = 0;

    Uint32 fpga_ntp_second_from_1970 =0;
    struct Satellite_Data *pSateData = &pRoot->satellite_data;       

    int fd = pRoot->dev[ENUM_LOCAL_DEV].fd;

    if((pClockInfo->ref_type ==  REF_SATLITE))
    {
        printf("GPS TIme:%d, leap:%d\n",pSateData->time+1,pSateData->gps_utc_leaps);

        /**卫星下来是utc时间补充闰秒  */
        Time = pSateData->time + pSateData->gps_utc_leaps;
        Time += 19;
        
        Time += 1;/**+1 表示当前秒数  */
    }
    else if(pClockInfo->ref_type == REF_10MHZ)
    {
        Time= ptptTime->seconds;
    }
    else
    {
        Time= ptptTime->seconds;
    }


    ntp_second_from_1970 = ioctl(fd,COMM_GET_SEC,0);
        

    /**连续性判断  */
    if(abs(Time - previousBusSec) != 1)
    {
        printf("-------1------\n");
        previousBusSec = Time;
        return;
    }

    previousBusSec = Time;

    /**时间连续10s 都不相同  */
    if(ptptTime->seconds != Time)
    {
        printf("------4-------\n");
        secErrorCnt++;
        if(secErrorCnt>5)
        {
            printf("=========setFpgaTime==%d===\n",Time);
            /**写下一秒的值fgpa 内部处理  */
            //SetFpgaTime(Time);
            secErrorCnt = 0;
        }
    }
    else if(ptptTime->seconds == Time)
        secErrorCnt = 0;


    /**使用本地ptp时间，防止背板出错时，时间出问题  */
    fpga_ntp_second_from_1970 = Time - pSateData->gps_utc_leaps;
    fpga_ntp_second_from_1970 -= 19;
    

    printf("ntp_second_from_1970=%d,fpga_ntp_second_from_1970=%d\n"
        ,ntp_second_from_1970
        ,fpga_ntp_second_from_1970);


    /**ntp 时间跟新系统时间用于ntpd  */
    if(ntp_second_from_1970 != fpga_ntp_second_from_1970)
    {
        ntpsecErrorCnt++;
        if(ntpsecErrorCnt>10)
        {
            printf("------5-------\n");
            ioctl(fd,COMM_SET_SEC,fpga_ntp_second_from_1970);
                        
            ioctl(fd,COMM_MODIFY_SEC,1);

            ntpsecErrorCnt = 0;
        }

    }
    else if(ntp_second_from_1970 == fpga_ntp_second_from_1970)
    {
        modify_internal++;
        printf("------6-------\n");
        if(modify_internal%4)
            ioctl(fd,COMM_MODIFY_SEC,1);
        ntpsecErrorCnt = 0;
    }
            
}


void *ThreadUsuallyProcess(void *arg)
{
    time_t current_time;

    Uint32 UtcTime;
    TimeInternal timeTmp;
    struct tm *tm;
    static Uint16 nTimeCnt = 0;
    struct root_data *pRootData = g_RootData;
    struct clock_info *pClockInfo = &pRootData->clock_info;

    while(1)
    {
        /**1s执行，  */
        if(pRootData->flag_usuallyRoutine)
        {
            
            printf("========================1PPS Start ============================\n");

            GetFpgaPpsTime(&timeTmp); 
            printf("fgpa system time:sec=%d,nao=%d \n",timeTmp.seconds,timeTmp.nanoseconds);

            current_time = timeTmp.seconds;
            tm = gmtime(&current_time);
            //printf("fgpatime:%d/%d/%d %d:%d:%d\n",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
          
            
            /**钟控处理  */
            Display_SatelliteData(&pRootData->satellite_data);

            
            //ClockStateProcess(pClockInfo);
            
            /**核心时间维护  */
            //MaintainCoreTime(pRootData,&timeTmp);
                       
             
            /**LED  处理*/
            Control_LedRun(nTimeCnt%2);
            nTimeCnt++;
            printf("========================1PPS End ============================\n\n\n");
            pRootData->flag_usuallyRoutine = FALSE;
        }

        /**测试上报  */
       
        //ClockHandleProcess(pClockInfo);

        usleep(10);
    }

    

}

static int print_usage(void) {
    printf("Usage: -i ethname(eth0, eth1)[options]\n");
    printf("\
Options:\n\
    -p Server port.\n\
    -d Running in deamon.\n\
    -l Log level.\n\
    -h Show usage.\n\
    -V Show version and exit.\n"); 
}

int daemonize(void)
{
    int pid, i;

    switch(fork())
    {
        /* fork error */
        case -1:
            perror("fork()");
            exit(1);

            /* child process */
        case 0:
            /* obtain a new process group */
            if( (pid = setsid()) < 0)
            {
                perror("setsid()");
                exit(1);
            }

            /* close all descriptors */
            for (i=getdtablesize();i>=0;--i) close(i);

            i = open("/dev/null", O_RDWR); /* open stdin */
            dup(i); /* stdout */
            dup(i); /* stderr */

            umask(027);
            chdir("/"); /* chdir to /tmp ? */

            return pid;

            /* parent process */
        default:
            exit(0);
    }
}

int main(int argc,char *argv[])
{
    int ret,err;
    pthread_attr_t threadAttr;
    pthread_t Id_ThreadUsually;
    int c;
    int val;
    
    g_RootData = (struct root_data *)malloc(sizeof(struct root_data));
	if(!g_RootData)
		printf("malloc root_para error\n");

	bzero(g_RootData,sizeof(struct root_data));
    g_RootData->comm_port = 20170;
    strcpy(g_RootData->comm_iface,"eth0");
    
    while((c = getopt(argc, argv, "dVhi:p:l:f:")) != -1)
    {
        switch(c)
        {
            case 'd':
                daemonize();
                break;
            case 'p':
              
                printf("the port is %s\n", optarg);
                g_RootData->comm_port = atoi(optarg);
                break;
            case 'V':

                break;
            case 'l':
                break;
            case 'i':
                if(!GetIpAddress(optarg,&val))
                {
                    printf("ethname unknown\n");
                    print_usage();
                    exit(0);
                }
                strcpy(g_RootData->comm_iface,optarg);
                break;
            case 'h':
            default:
                print_usage();
                exit(0);
        }
    }

    Init_FpgaCore();
    Init_Thread_Attr(&g_RootData->pattr);
    Init_PpsDev("/dev/ptp_dev");
    InitDev(g_RootData);
    InitSlotList(g_RootData);
    

    /** 创建日常处理线程 */
	err = pthread_create(&g_RootData->p_usual,&g_RootData->pattr,(void *)ThreadUsuallyProcess,(void *)g_RootData);	
	if(err == 0)
	{
		printf("dailymanage_pthread succes!\n");
	}
	else
	{
		printf("dailymanage_pthread error!\n");
		return FALSE;
	}

    /**创建数据接收线程  */
	err = pthread_create(&g_RootData->p_recv,&g_RootData->pattr,(void *)DataRecv_Thread,(void *)&g_RootData->dev_head);	
	if(err == 0)
	{
		printf("datarecv_pthread succes!\n");
	}
	else
	{
		printf("datarecv_pthread error!\n");
		return FALSE;
	}

    /**创建数据发送线程  */
	err = pthread_create(&g_RootData->p_send,&g_RootData->pattr,(void *)DataSend_Thread,(void *)&g_RootData->dev_head);	/*数据发送线程*/
	if(err == 0)
	{
		printf("datasend_pthread succes!\n");
	}else
	{
		printf("datasend_pthread error!\n");
		return FALSE;
	}

    /**创建数据处理线程  */
	err = pthread_create(&g_RootData->p_handle,&g_RootData->pattr,(void *)DataHandle_Thread,(void *)g_RootData);/*数据处理线程*/
	if(err == 0)
	{
		printf("datahandle_pthread succes!\n");
	}else
	{
		printf("datahandle_pthread error!\n");
		return FALSE;
	}

    pthread_join(g_RootData->p_usual,NULL);
    pthread_join(g_RootData->p_recv,NULL);
    pthread_join(g_RootData->p_send,NULL);
    pthread_join(g_RootData->p_handle,NULL);
    

	return TRUE;
}

