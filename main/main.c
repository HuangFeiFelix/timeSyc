#include "main.h"
#include "log.h"
#include "net.h"
#include "manager.h"
#include "fpga_time.h"
#include "lcd_cmd_process.h"
#include "clock_rb.h"

struct root_data *g_RootData;

#define COMM_GET_SEC    3
#define COMM_SET_SEC    4
#define COMM_MODIFY_SEC 5


int comm_sin_port;
char comm_ip_address[20];
int RbOrXo = 1; /**0: 铷钟，1:OCXO  */


char *ctlEthConfig = "/mnt/network_ctl.cfg";
char *ptpEthConfig = "/mnt/network_ptp.cfg";
char *ntpEthConfig = "/mnt/network_ntp.cfg";
char *ptpConfig = "/mnt/ptp.conf";
char *ntpConfig = "/mnt/ntp/ntp.conf";
char *md5Config = "/mnt/ntp/keys";
char *commIpaddressCfg = "/mnt/comm_ip_address.cfg";


    
#define PATH_PTP_DAEMON "/mnt/ptp"
#define PATH_NTP_DAEMON "/mnt/ntp/ntpd"

int remove_space(char *src,char *out)
{
    char *p = src;
    int i;
    i = strlen(src);

    

    while(i--)
    {
        if(*p == 0x20)
        {
            p++;
        }
        else
        {
            *out = *p;
            p++;
            out++;
        }


    }

    
}

int String2Bytes(unsigned char* szSrc, unsigned char* pDst, int nDstMaxLen)     
{  
    int i;
    if(szSrc == NULL)  
    {  
        return 0;  
    }
    
    int iLen = strlen((char *)szSrc);  
    if (iLen <= 0 || iLen%2 != 0 || pDst == NULL || nDstMaxLen < iLen/2)  
    {  
        return 0;  
    }  
      
    iLen /= 2;  
    //strupr((char *)szSrc);  
    for (i=0; i< iLen; i++)  
    {  
        int iVal = 0;  
        unsigned char *pSrcTemp = szSrc + i*2;  
        sscanf((char *)pSrcTemp, "%02x", &iVal);  
        pDst[i] = (unsigned char)iVal;  
    }  
      
    return iLen;  
}   


void AddData_ToSendList(struct root_data *pRoot,char devType,void *buf,short len)
{
    add_send(buf,len,&pRoot->dev[devType]);
}

char setzda[100] = {"B5 62 06 01 08 00 F0 08 00 01 00 00 00 00 08 60"};
char pubx[100] =   {"B5 62 06 01 08 00 F1 04 00 01 00 00 00 00 05 4C"};
char bdgps[200] = {"B5 62 06 3E 2C 00 00 00 20 05 00 08 10 00 00 00 01 01 01 01 03 00 00 00 01 01 03 08 10 00 01 00 01 01 05 00 03 00 00 00 01 01 06 08 0E 00 00 00 01 01 FC 01"};

void Init_ublox(struct root_data *pRootData)
{
	char txbuf[200];
    unsigned char txbufBin[200];
    int txlen;

	memset(txbuf,0,sizeof(txbuf));
	memset(txbufBin,0,sizeof(txbufBin));

	txlen = remove_space(bdgps,txbuf);
	txlen = String2Bytes(txbuf,txbufBin,200);
    AddData_ToSendList(pRootData,ENUM_GPS,txbufBin,txlen);

	memset(txbuf,0,sizeof(txbuf));
	memset(txbufBin,0,sizeof(txbufBin));

	txlen = remove_space(setzda,txbuf);
	txlen = String2Bytes(txbuf,txbufBin,200);

	AddData_ToSendList(pRootData,ENUM_GPS,txbufBin,txlen);

	memset(txbuf,0,sizeof(txbuf));
	memset(txbufBin,0,sizeof(txbufBin));
	txlen = remove_space(pubx,txbuf);
	txlen = String2Bytes(txbuf,txbufBin,200);

	AddData_ToSendList(pRootData,ENUM_GPS,txbufBin,txlen);

}

void start_ptp_daemon()
{
    system("/mnt/ptp &");
}


void stop_ptp_daemon()
{
    system("pkill ptp");
}


void start_ntp_daemon()
{
    system("/mnt/ntp/ntpd -c /mnt/ntp/ntp.conf");

}

void stop_ntp_daemon()
{
    system("pkill ntp");

}

void reset_sys_daemon()
{
    system("reboot");

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

    Load_PtpParam_FromFile(pSlotLocal->pPtpSetcfg,ptpConfig);
    Load_NtpdParam_FromFile(pSlotLocal->pNtpSetCfg,ntpConfig,md5Config);
    
}
void InitLoadRtcTime(struct root_data *pRootData)
{
    struct timeval tv;


    //tv.tv_sec += 19;
    //tv.tv_sec += 18;
    
    struct Satellite_Data *pSateData = &pRootData->satellite_data;
    gettimeofday(&tv,NULL);
    pSateData->time = tv.tv_sec;

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
    
    for(i = 1;i <= SLOT_COUNT;i++)
    {
        pRootData->slot_list[i].slot_type = 0;
        pRootData->slot_list[i].pNtpSetCfg = NULL;
        pRootData->slot_list[i].pPtpSetcfg = NULL;
        pRootData->slot_list[i].pPtpStatusCfg = NULL;
    }

    /** =1 使用OCXO */
    if(RbOrXo == 1)
    {
        SetFpgaAddressVal(0x84,0x01);
    }
}

void InitNetInforAll(struct root_data *pRootData)
{
    
    strcpy(pRootData->ptp_port.ifaceName,"eth0");
    strcpy(pRootData->ntp_port.ifaceName,"eth1");
    strcpy(pRootData->comm_port.ifaceName,"eth2");
    Load_NetWorkParam_FromFile(ctlEthConfig,&pRootData->comm_port);
    Load_NetWorkParam_FromFile(ntpEthConfig,&pRootData->ntp_port);
    Load_NetWorkParam_FromFile(ptpEthConfig,&pRootData->ptp_port);

    memset(comm_ip_address,0,sizeof(comm_ip_address));
    Load_comm_ip_address(commIpaddressCfg,comm_ip_address);
    
}

void InitDev(struct root_data *pRootData)
{    
    init_dev_head(&pRootData->dev_head);
    
    /**初始化UI 设备 ,UI设备通过串口通信id =0*/
#if 1
    pRootData->dev[ENUM_LCD].com_attr.com_port = ENUM_LCD;  //串口1
    pRootData->dev[ENUM_LCD].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_LCD],&pRootData->dev_head,COMM_DEVICE,ENUM_LCD);
#endif
#if 0
    pRoot->dev[ENUM_UI].net_attr.sin_port= 9977;  //串口1
    pRoot->dev[ENUM_UI].net_attr.ip = inet_addr("192.168.15.127");
    init_add_dev(&pRoot->dev[ENUM_UI],&pRoot->dev_head,UDP_DEVICE,ENUM_UI);
#endif



    /**接收机串口设备 通信id=2  */
    pRootData->dev[ENUM_GPS].com_attr.com_port = ENUM_GPS;  //串口1
    pRootData->dev[ENUM_GPS].com_attr.baud_rate = 9600;
    init_add_dev(&pRootData->dev[ENUM_GPS],&pRootData->dev_head,COMM_DEVICE,ENUM_GPS);
    
#if 1
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
   
   pRootData->dev[ENUM_PC_CTL].net_attr.sin_port = comm_sin_port;//
   pRootData->dev[ENUM_PC_CTL].net_attr.ip = inet_addr(comm_ip_address);
   init_add_dev(&pRootData->dev[ENUM_PC_CTL],&pRootData->dev_head,UDP_DEVICE,ENUM_PC_CTL);

    /**本地PTP   */
    pRootData->dev[ENUM_IPC_PTP].net_attr.sin_port = 9900;
    pRootData->dev[ENUM_IPC_PTP].net_attr.ip = inet_addr("127.0.0.1");
    init_add_dev(&pRootData->dev[ENUM_IPC_PTP],&pRootData->dev_head,INIT_DEVICE,ENUM_IPC_PTP);

    pRootData->dev[ENUM_IPC_NTP].net_attr.sin_port = 9988;
    pRootData->dev[ENUM_IPC_NTP].net_attr.ip = inet_addr("127.0.0.1");
    init_add_dev(&pRootData->dev[ENUM_IPC_NTP],&pRootData->dev_head,INIT_DEVICE,ENUM_IPC_NTP);

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
                        printf("dev_id=  %d,udp:receive: ",p_dev->dev_id);

                        for(i=0;i<p_data_list->recv_lev.len;i++)
                            printf("%x ",p_data_list->recv_lev.data[i]);
                        printf("\n");
#endif
                        printf("handle UDP pc_ctl_message %d\n",p_dev->dev_id);

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
                    count = sendto(p_dev->fd,p_data_list->send_lev.data,len,0,(struct sockaddr *)&p_dev->dest_addr,sizeof(struct sockaddr_in));
                    
                    del_data(p_data_list, &p_dev->data_head[1],p_dev);
                }
                else if(p_dev->type == TCP_DEVICE)
                {
                    len = p_data_list->send_lev.len;
                    count = send(p_dev->fd, p_data_list->send_lev.data, len,0);
                    
                    del_data(p_data_list, &p_dev->data_head[1], p_dev);

                }
                else if(p_dev->type == COMM_DEVICE)
                {
                    len = p_data_list->send_lev.len;
                    //printf("send =%d %x \n",len,p_data_list->send_lev.data[0]);
                    
                    count = write(p_dev->fd, p_data_list->send_lev.data, len);
                    
                    del_data(p_data_list, &p_dev->data_head[1], p_dev);

                }
                else if(p_dev->type == INIT_DEVICE)
                {
                    len = p_data_list->send_lev.len;
                    count = sendto(p_dev->fd,p_data_list->send_lev.data,len,0,(struct sockaddr *)&p_dev->dest_addr,sizeof(struct sockaddr_in));
                    
                    del_data(p_data_list, &p_dev->data_head[1],p_dev);

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

    struct root_data *pRootData = g_RootData;
    struct clock_info *pClock_info = &pRootData->clock_info;
    struct collect_data *p_collect_data = &pClock_info->data_1Hz;
    struct clock_alarm_data *pClockAlarm = &pClock_info->alarmData;
    int phaseOffset = 0;
    int ph;
    //static int index = 0;
    
    if(pClock_info->ref_type == REF_SATLITE 
        && pClockAlarm->alarmBd1pps == 0 
        && pClockAlarm->alarmSatellite == 0
        && pClock_info->run_times > RUN_TIME)
    {

            phaseOffset = Get_Pps_Rb_PhaseOffset();
            phaseOffset = phaseOffset * 4;
            ph = Kalman_Filter(phaseOffset,1);
            printf("readPhase=%d collect_phase=%d, count=%d\n",phaseOffset,ph,p_collect_data->ph_number_counter);
            collect_phase(&pClock_info->data_1Hz,0,phaseOffset);      

    }

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

    int fd = pRoot->dev_fd;

    if((pClockInfo->ref_type ==  REF_SATLITE))
    {
        printf("GPS TIme:%d, leap:%d\n",pSateData->time+1,pSateData->gps_utc_leaps);

        /**卫星下来是utc时间补充闰秒  */
        Time = pSateData->time + pSateData->gps_utc_leaps;
        Time += 19;
        
        Time += 1;/**+1 表示当前秒数  */
    }
    else if(pClockInfo->ref_type == REF_PTP)
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
            SetFpgaTime(Time);
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

void display_alarm_to_lcd(struct root_data *pRootData)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;
    struct clock_alarm_data *pClockAlarm = &pClockInfo->alarmData;
    if(pClockAlarm->alarmSatellite = TRUE)
        SetTextValue(WARN_SCREEN_ID,4,"YES");
    else
        SetTextValue(WARN_SCREEN_ID,4,"NO");

    if(pClockAlarm->alarmPtp == TRUE)
        SetTextValue(WARN_SCREEN_ID,5,"YES");
    else
        SetTextValue(WARN_SCREEN_ID,5,"NO");

    if(RbOrXo == 1)
    {
        if(pClockAlarm->alarmVcxo100M == TRUE || pClockAlarm->alarmXo10M == TRUE || pClockAlarm->vcxoLock == 0)
            SetTextValue(WARN_SCREEN_ID,6,"YES");
        else
            SetTextValue(WARN_SCREEN_ID,6,"NO");

    }
    else
    {
        if(pClockAlarm->alarmVcxo100M == TRUE || pClockAlarm->alarmRb10M == TRUE || pClockAlarm->vcxoLock == 0)
            SetTextValue(WARN_SCREEN_ID,6,"YES");
        else
            SetTextValue(WARN_SCREEN_ID,6,"NO");
    }


}


void display_clockstate_to_lcd(struct root_data *pRootData)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;
    struct Satellite_Data *p_satellite_data = &pRootData->satellite_data;
    char szbuf[50];

    
    if(pClockInfo->ref_type == 0)
        SetTextValue(CLOCK_STATUS_SCREEN_ID,12,"GPS/BD");
    else
        SetTextValue(CLOCK_STATUS_SCREEN_ID,12,"PTP");

    switch(pClockInfo->workStatus)
    {
        case 0:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"FREE");
            break;
        case 1:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"FAST");
            break;
        case 2:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"LOCK");
            break;
        case 3:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"HOLD");
            break;
        default:
            SetTextValue(CLOCK_STATUS_SCREEN_ID,13,"ERROR");
            break;
    }
    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%d",pClockInfo->data_1Hz.phase_offset);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,14,szbuf);

    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%d",p_satellite_data->satellite_see);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,15,szbuf);

    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%d",p_satellite_data->satellite_use);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,16,szbuf);

    if(p_satellite_data->time_enable == TRUE)
        SetTextValue(CLOCK_STATUS_SCREEN_ID,17,"YES");
    else
        SetTextValue(CLOCK_STATUS_SCREEN_ID,17,"NO");

    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%d",p_satellite_data->gps_utc_leaps);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,18,szbuf);

    memset(szbuf,0,sizeof(szbuf));
    sprintf(szbuf,"%c %d`%d'%d'' %c %d`%d'%d''"
        ,p_satellite_data->eastorwest,p_satellite_data->longitude_d,p_satellite_data->longitude_f,p_satellite_data->longitude_m
        ,p_satellite_data->northorsouth,p_satellite_data->latitude_d,p_satellite_data->latitude_f,p_satellite_data->latitude_m);
    SetTextValue(CLOCK_STATUS_SCREEN_ID,19,szbuf);
    
}



void display_lcd_running_status(struct root_data *pRootData)
{

    switch(pRootData->lcd_sreen_id)
    {
        
        case MAIN_SCREEN_ID:
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);
            break;
        case CLOCK_STATUS_SCREEN_ID:
            display_clockstate_to_lcd(pRootData);
            break;
        case WARN_SCREEN_ID:
            display_alarm_to_lcd(pRootData);

            break;

        case ETH_CTL_SCREEN_ID:       
        case ETH_PTP_SCREEN_ID:      
        case ETH_NTP_SCREEN_ID:
            break;
        default:
            break;
    }
    
}



void display_alarm_information(struct clock_alarm_data *pClockAlarm)
{
   
    printf("alarmBd1pps=%d alarmPtp=%d alarmVcxo100M=%d\n"
        ,pClockAlarm->alarmBd1pps,pClockAlarm->alarmPtp,pClockAlarm->alarmVcxo100M);
    printf("alarmRb10M=%d alarmXo10M=%d vcxoLock=%d\n"
        ,pClockAlarm->alarmRb10M,pClockAlarm->alarmXo10M,pClockAlarm->vcxoLock);
    printf("alarmDisk=%d alarmSatellite=%d\n"
        ,pClockAlarm->alarmDisk,pClockAlarm->alarmSatellite);
}


void updatePtpStatusAndNtpStatus(struct root_data *pRootData,Uint16 nTimeCnt)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;
    struct Satellite_Data *pSateData = &pRootData->satellite_data;     
    struct NtpdStatus mNtpStatus;
    struct PtpStatus mPtpStatus;

    
    if(nTimeCnt%6 == 0)
    {
        memset(&mNtpStatus,0,sizeof(mNtpStatus));
        memset(&mPtpStatus,0,sizeof(mPtpStatus));
        switch(pClockInfo->workStatus)
        {
            case 0:
                memcpy(mNtpStatus.refid,"LOC",strlen("LOC"));
                mNtpStatus.stratum = 1;
                mNtpStatus.leap = LEAP_NOWARNING;
                mNtpStatus.precision = STRATUM_3_PRESION;

                mPtpStatus.blockOutput = 0;
                mPtpStatus.blockOutput = 0;
                mPtpStatus.priority1 = 127;
                mPtpStatus.priority2 = 127;
                mPtpStatus.timeSource = 0xa0;
                mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                mPtpStatus.clockClass = 248;
                mPtpStatus.clockAccuracy = 0x23;
                break;
            case 1:
                if(pClockInfo->ref_type == 0)
                {
                    memcpy(mNtpStatus.refid,"BD",strlen("BD"));
                    mNtpStatus.stratum = 0;
                    mNtpStatus.leap = LEAP_NOWARNING;
                    mNtpStatus.precision = STRATUM_2_PRESION;

                    mPtpStatus.blockOutput = 0;
                    mPtpStatus.priority1 = 127;
                    mPtpStatus.priority2 = 127;
                    mPtpStatus.timeSource = 0x20;
                    mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                    mPtpStatus.clockClass = 7;
                    mPtpStatus.clockAccuracy = 0x21;
                }
                else
                {
                    memcpy(mNtpStatus.refid,"REF",strlen("REF"));
                    mNtpStatus.stratum = 0;
                    mNtpStatus.leap = LEAP_NOWARNING;
                    mNtpStatus.precision = STRATUM_2_PRESION;

                    mPtpStatus.blockOutput = 0;
                    mPtpStatus.priority1 = 127;
                    mPtpStatus.priority2 = 127;
                    mPtpStatus.timeSource = 0x40;
                    mPtpStatus.utcOffset = 37;
                    mPtpStatus.clockClass = 7;
                    mPtpStatus.clockAccuracy = 0x21;


                }
                break;
            case 2:
                if(pClockInfo->ref_type == 0)
                {
                    memcpy(mNtpStatus.refid,"BD",strlen("BD"));
                    mNtpStatus.stratum = 0;
                    mNtpStatus.leap = LEAP_NOWARNING;
                    mNtpStatus.precision = STRATUM_1_PRESION;

                    mPtpStatus.blockOutput = 0;
                    mPtpStatus.priority1 = 0;
                    mPtpStatus.priority2 = 127;
                    mPtpStatus.timeSource = 0x20;
                    mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                    mPtpStatus.clockClass = 6;
                    mPtpStatus.clockAccuracy = 0x20;

                }
                else
                {
                    memcpy(mNtpStatus.refid,"REF",strlen("REF"));
                    mNtpStatus.stratum = 0;
                    mNtpStatus.leap = LEAP_NOWARNING;
                    mNtpStatus.precision = STRATUM_1_PRESION;

                    mPtpStatus.blockOutput = 0;
                    mPtpStatus.priority1 = 0;
                    mPtpStatus.priority2 = 127;
                    mPtpStatus.timeSource = 0x40;
                    mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                    mPtpStatus.clockClass = 6;
                    mPtpStatus.clockAccuracy = 0x20;

                }   
                break;
            case 3:
                memcpy(mNtpStatus.refid,"LOC",strlen("LOC"));
                mNtpStatus.stratum = 0;
                mNtpStatus.leap = LEAP_NOWARNING;
                mNtpStatus.precision = STRATUM_3_PRESION;

                mPtpStatus.blockOutput = 0;
                mPtpStatus.priority1 = 127;
                mPtpStatus.priority2 = 127;
                mPtpStatus.timeSource = 0xa0;
                mPtpStatus.utcOffset = pSateData->gps_utc_leaps + 19;
                mPtpStatus.clockClass = 248;
                mPtpStatus.clockAccuracy = 0x22;
                break;
           default:
                break;
        }

        AddData_ToSendList(pRootData,ENUM_IPC_NTP,(void *)&mNtpStatus,sizeof(mNtpStatus));
        AddData_ToSendList(pRootData,ENUM_IPC_PTP,(void *)&mPtpStatus,sizeof(mPtpStatus));

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
    struct clock_alarm_data *pClockAlarm = &pClockInfo->alarmData;
    struct Satellite_Data *pSateData = &pRootData->satellite_data;   

    while(1)
    {
        /**1s执行，  */
        if(pRootData->flag_usuallyRoutine)
        {
            
            printf("========================1PPS Start ============================\n");

            GetFpgaPpsTime(&timeTmp); 
            printf("fgpa system time:sec=%d,nao=%d \n",timeTmp.seconds,timeTmp.nanoseconds);
            current_time = timeTmp.seconds;
            current_time += 28800;
            current_time -= 19;
            current_time -= pSateData->gps_utc_leaps;
            
            tm = gmtime(&current_time);

            memset(pRootData->current_time,0,sizeof(pRootData->current_time));
            sprintf(pRootData->current_time,"%d-%d-%d %02d:%02d:%02d",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
            printf("%s\n",pRootData->current_time);
          
            /**核心时间维护  */
            MaintainCoreTime(pRootData,&timeTmp);

            CollectAlarm(pRootData);
            Display_SatelliteData(&pRootData->satellite_data);
            display_alarm_information(pClockAlarm);

            if(RbOrXo == 1)
            {
                ClockStateProcess_OCXO(pClockInfo);

            }
            else
            {
                ClockStateProcess(pClockInfo);

            }
            
            display_lcd_running_status(pRootData);     
            inssue_pps_data(pRootData);
            
            /**LED  处理*/
            Control_LedRun(nTimeCnt%2);

            if(pClockAlarm->alarmPtp == 1 || pClockAlarm->alarmDisk == 1)
                Control_LedAlarm(0x01);
            else
                Control_LedAlarm(0x00);

            if(pClockAlarm->alarmSatellite == 1)
                Control_LedLockStatus(0x00);
            else
                Control_LedLockStatus(0x01);

            if(pClockInfo->workStatus == FREE)
                Control_LedSatStatus(0x00);
            else if(pClockInfo->workStatus == HOLD)
                Control_LedSatStatus(0x10);
            else
                Control_LedSatStatus(0x01);



            updatePtpStatusAndNtpStatus(pRootData,nTimeCnt);             
            
            nTimeCnt++;
            printf("========================1PPS End ============================\n\n\n");
            pRootData->flag_usuallyRoutine = FALSE;
        }

        /**测试上报  */
        if(RbOrXo == 1)
            ClockHandleProcess_OCXO(pClockInfo);
        else
            ClockHandleProcess(pClockInfo);

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

void IntExit(int a)
{
    system("pkill ptp");
    system("pkill ntpd");
    exit(-1);
}

void Wait_For_EnvConfig()
{
	struct timeval timeout;
	fd_set fs_read;
    
    int ret;
    int wait_cnt = 3;

	/*用来查询设备状态（读，写，异常）的改变，在timeout时间内*/

    printf("\r\nWatting for Env Init finish... :   ");
    fflush(stdout);
    
    while(wait_cnt)
    {
        timeout.tv_sec = 1;									
	    timeout.tv_usec = 0;
        FD_ZERO(&fs_read);
        FD_SET(0,&fs_read);

        ret = select(1,&fs_read,NULL,NULL,&timeout);

        printf("\b\b%d ",wait_cnt);
        fflush(stdout);
        wait_cnt--;

    }

    printf("\n");
}



void set_gsp_module1(struct root_data *pRootData)
{
    char buf[100];
    int iOffset = 0;
    int i;
    unsigned char CK_A;
    unsigned char CK_B;
    
    memset(buf,0,sizeof(buf));

    buf[iOffset++] = 0xb5;
    buf[iOffset++] = 0x62;
    buf[iOffset++] = 0x06;
    buf[iOffset++] = 0x13;
    
    buf[iOffset++] = 4;
    buf[iOffset++] = 0;
    
    buf[iOffset++] = 0;
    buf[iOffset++] = 0;
    buf[iOffset++] = 0;
    buf[iOffset++] = 0;
    
    for(i = 2;i<10;i++)
    {
        CK_A = CK_A + buf[i];
        CK_B = CK_B + CK_A;
    }
    buf[iOffset++] = CK_A;
    buf[iOffset++] = CK_B;
    AddData_ToSendList(pRootData,ENUM_GPS,buf,iOffset);    
}


void set_gsp_module(struct root_data *pRootData)
{
    char buf[100];
    int iOffset = 0;
    int i;
    char CK_A;
    char CK_B;
    
    memset(buf,0,sizeof(buf));

    buf[iOffset++] = 0xb5;
    buf[iOffset++] = 0x62;
    buf[iOffset++] = 0x06;
    buf[iOffset++] = 0x17;
    
    buf[iOffset++] = 0;
    buf[iOffset++] = 20;
    
    buf[iOffset++] = 0;
    buf[iOffset++] = 0x40;
    buf[iOffset++] = 0;
    buf[iOffset++] = 0;
    
    iOffset += 4;
    
    buf[iOffset++] = 0;
    buf[iOffset++] = 1;
    buf[iOffset++] = 1;
    buf[iOffset++] = 1;
    buf[iOffset++] = 0;
    buf[iOffset++] = 0;
    iOffset += 6;

    for(i = 2;i<24+2;i++)
    {
        CK_A = CK_A + buf[i];
        CK_B = CK_B + CK_A;
    }
    buf[iOffset++] = CK_A;
    buf[iOffset++] = CK_B;
    AddData_ToSendList(pRootData,ENUM_GPS,buf,iOffset);    
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
    comm_sin_port = 20170;
    
    while((c = getopt(argc, argv, "dVhp:l:f:")) != -1)
    {
        switch(c)
        {
            case 'd':
                daemonize();
                break;
            case 'p':
              
                printf("the port is %s\n", optarg);
                comm_sin_port = atoi(optarg);
                break;
            case 'V':

                break;
            case 'l':
                break;

            case 'h':
            default:
                print_usage();
                exit(0);
        }
    }
    InitNetInforAll(g_RootData);
    Init_RbClockCenter(&g_RootData->clock_info);
    Init_FpgaCore();
    Init_Thread_Attr(&g_RootData->pattr);
    Init_PpsDev("/dev/ptp_dev");
    InitDev(g_RootData);
    InitSlotList(g_RootData);    
    signal(SIGINT, IntExit);
    InitLoadRtcTime(g_RootData);
    Wait_For_EnvConfig();

    start_ntp_daemon();
    start_ptp_daemon();

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

	//Init_ublox(g_RootData);

    pthread_join(g_RootData->p_usual,NULL);
    pthread_join(g_RootData->p_recv,NULL);
    pthread_join(g_RootData->p_send,NULL);
    pthread_join(g_RootData->p_handle,NULL);
    

	return TRUE;
}

