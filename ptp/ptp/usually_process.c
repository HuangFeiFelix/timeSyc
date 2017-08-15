#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "common.h"
#include "usually_process.h"
#include "data_type.h"
#include "ptp.h"
#include "log.h"

#define  OSC_RB         0x01
#define  OSC_XO         0x02

#define OSC_STATUS_HOLD 0x00
#define OSC_STATUS_RREE 0x01
#define OSC_STATUS_LOCK 0x02
#define OSC_STATUS_FAST 0x03

#define TIME_SOURCE_BD              0x00
#define TIME_SOURCE_GPS             0x01
#define TIME_SOURCE_PTP             0x02
#define TIME_SOURCE_PTPOE1          0x03
#define TIME_SOURCE_1PPSTOD_SAT     0x04
#define TIME_SOURCE_1PPSTOD_DEV     0x05
#define TIME_SOURCE_NULL            0xFF



#define NO_LEAP         0x00
#define LEAP61          0x01            //正闰秒
#define LEAP59          0x02            //负闰秒

#define COMM_GET_SEC    3
#define COMM_SET_SEC    4
#define COMM_MODIFY_SEC 5


#define MEDIA_1000FX    0x02
#define MEDIA_BASE_T    0x08

#define PORT_SEED_100M    0
#define PORT_SEED_1000M   1

#define PAGE_ADDR         22




#define CONVERT_INT_BCD(o,i) o = i / 10; \
        o = o<<4; \
        o += i % 10; 

void Init_I2c(SfpCtl *pSfpCtl)
{
    int i2cFd = open("/dev/i2c-1",O_CREAT | O_RDWR);
    if(i2cFd < 0)
        printf("open i2c error\n");


    //7位地址位
    int ret=ioctl(i2cFd,I2C_TENBIT,0);
    if(ret<0)
       printf("i2c set TENBIT error! \n");


    //设置I2C从机地址,即设备地址(SFP)
    ret=ioctl(i2cFd,I2C_SLAVE,0x50);
    if(ret<0)
       printf("i2c set slaveaddr error! \n");

    pSfpCtl->i2cfd = i2cFd;

}

void Init_SfpCtl(SfpCtl *pSfpCtl)
{
    int i;
    pSfpCtl->sockfd= socket(PF_LOCAL, SOCK_DGRAM, 0);
    Init_I2c(pSfpCtl);

    memcpy(pSfpCtl->sfp_port[0].ifr.ifr_name,"eth0",4);

}

unsigned short ReadPagePhy(SfpCtl *pSfpCtl,short port_num,unsigned short page,unsigned short addr)
{
    unsigned short ret;
    struct ifreq ifr = pSfpCtl->sfp_port[port_num].ifr;
     
    struct mii_ioctl_data *mii = (struct mii_ioctl_data*)&ifr.ifr_ifru;

    mii->reg_num    = PAGE_ADDR;
    mii->val_in     = page;
        
    ret = ioctl(pSfpCtl->sockfd, SIOCSMIIREG, &ifr);
    if(ret<0)
        printf("write error\n");
    else
    {
        mii->reg_num = addr;
        ret = ioctl(pSfpCtl->sockfd, SIOCGMIIPHY, &ifr);
    }

   
    return mii->val_out;

}



int WritePhy(SfpCtl *pSfpCtl,short port_num,unsigned short page,unsigned short addr,unsigned short val)
{


    unsigned short ret;
    struct ifreq ifr = pSfpCtl->sfp_port[port_num].ifr;
    struct mii_ioctl_data *mii = (struct mii_ioctl_data*)&ifr.ifr_ifru;
    

    mii->reg_num    = PAGE_ADDR;
    mii->val_in     = page;
    
    ret = ioctl(pSfpCtl->sockfd, SIOCSMIIREG, &ifr);
    if(ret<0)
    {
        printf("write error\n");
        return 0;
    }
    else
    {
            mii->reg_num    = addr;
            mii->val_in     = val;
            ioctl(pSfpCtl->sockfd, SIOCSMIIREG, &ifr);
    }
    return 1;
}


void GetEthxSfp_Status(SfpCtl *pSfpCtl,short portNum)
{
    char readAddr = 0x06;
    short link;
    char type;

   
    //SetI2cChannel(portNum);
    usleep(500);
    write(pSfpCtl->i2cfd,&readAddr,1);

    link = read(pSfpCtl->i2cfd,&type,1);

    pSfpCtl->sfp_port[portNum].i2c_link_status = link;
    pSfpCtl->sfp_port[portNum].media_type = type;
    printf("sfp_port[%d] linkStatus=%d,media=%d\n",portNum,link,type);

}


#if 0
void Init_Eth2ToEth7_100MPhy(SfpCtl *pSfpCtl)
{
    unsigned short val;
    int i;
    for(i=2;i<PTP_PORT_COUNT;i++)
    {
        /**cooper speed lsb =1,msb =0 0x01 =100M */
        val = ReadPagePhy(pSfpCtl,i,0,0);
        printf("read port %d phy addr: 0_0 addr value : 0x%x\n\n",i,val);
        val |= (1<<13);
        val &= ~(1<<6);
        WritePhy(pSfpCtl,i,0,0,val);
        
        val = ReadPagePhy(pSfpCtl,i,0,0);
        printf("read port %d phy addr: 0_0 addr value : 0x%x\n\n",i,val);
        val |= (1<<15);
        WritePhy(pSfpCtl,i,0,0,val);

    }

}
#endif

void AutoEthxPhy(SfpCtl *pSfpCtl,short portNum)
{
    unsigned short val;
    short addrOffset = 0;

    if(pSfpCtl->sfp_port[portNum].i2c_link_status > 0)
    {
        printf("Sfp Port[%d] cnnected,config sfp!\n",portNum);
        switch(pSfpCtl->sfp_port[portNum].media_type)
        {
            case MEDIA_BASE_T:
                val = ReadPagePhy(pSfpCtl,portNum,18,20);
                printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_BASE_T\n\n", portNum,val);
                val = 0x04;
                //val |= (1<<5);
                WritePhy(pSfpCtl,portNum,18,20,val);

                val = ReadPagePhy(pSfpCtl,portNum,18,20);
                printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_BASE_T\n\n", portNum,val);
                val |= (1<<15);
                WritePhy(pSfpCtl,portNum,18,20,val);

                break;
            case MEDIA_1000FX:
            case 0x01:
                val = ReadPagePhy(pSfpCtl,portNum,18,20);
                printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_1000FX\n\n", portNum,val);
                val = 0x02;
                val |= (1<<5);
                WritePhy(pSfpCtl,portNum,18,20,val);

                val = ReadPagePhy(pSfpCtl,portNum,18,20);
                printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_1000FX\n\n", portNum,val);
                val |= (1<<15);
                WritePhy(pSfpCtl,portNum,18,20,val);
                break;
            default:
                break;
        }
        
    }

    /**强制连接上  */

    usleep(50000);

    val = ReadPagePhy(pSfpCtl,portNum,0,0);
    printf("read port %d phy page 0 addr value : 0x%x\n\n", portNum,val);

    val = ReadPagePhy(pSfpCtl,portNum,0,1);
    printf("read port %d phy page 0 addr value : 0x%x\n\n",portNum,val);


    val = ReadPagePhy(pSfpCtl,portNum,1,0);
    printf("read port %d phy page 1 addr value : 0x%x\n\n",portNum, val);

    val = ReadPagePhy(pSfpCtl,portNum,1,1);
    printf("read port %d phy page 1 addr value : 0x%x\n\n",portNum, val);


    /**适应千兆还是百兆  */
    val = ReadPagePhy(pSfpCtl,portNum,1,17);
    printf("read port %d phy addr: 1_17 addr value : 0x%x\n\n",portNum, val);

    addrOffset= 0x0100*(portNum+1) +0x004d;
    if(val & (1<<15))
    {
        printf("phy dedect speed 1000M\n");
        pSfpCtl->sfp_port[portNum].speed = PORT_SEED_1000M;
        SetFpgaAddressVal(addrOffset,1);
    }
    else if(val & (1<<14))
    {
        printf("phy dedect speed 100M\n");
        pSfpCtl->sfp_port[portNum].speed = PORT_SEED_100M;
        SetFpgaAddressVal(addrOffset,0);
    }
    else
    {
        printf("phy dedect speed fail,default 1000M\n");
        pSfpCtl->sfp_port[portNum].speed = PORT_SEED_100M;
        SetFpgaAddressVal(addrOffset,0);

    }

}

void Init_Ethx_SfpPhy(SfpCtl *pSfpCtl,short portNum)
{
    unsigned short val;
    short addrOffset = 0;
   

    if(pSfpCtl->sfp_port[portNum].i2c_link_status > 0)
    {
        printf("Sfp Port[%d] cnnected,config sfp!\n",portNum);
        switch(pSfpCtl->sfp_port[portNum].media_type)
        {
            case MEDIA_BASE_T:
                val = ReadPagePhy(pSfpCtl,portNum,18,20);
                printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_BASE_T\n\n", portNum,val);
                val = 0x04;
                //val |= (1<<5);
                WritePhy(pSfpCtl,portNum,18,20,val);

                val = ReadPagePhy(pSfpCtl,portNum,18,20);
                printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_BASE_T\n\n", portNum,val);
                val |= (1<<15);
                WritePhy(pSfpCtl,portNum,18,20,val);

                break;
            case MEDIA_1000FX:
                val = ReadPagePhy(pSfpCtl,portNum,18,20);
                printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_1000FX\n\n", portNum,val);
                val = 0x02;
                val |= (1<<5);
                WritePhy(pSfpCtl,portNum,18,20,val);

                val = ReadPagePhy(pSfpCtl,portNum,18,20);
                printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_1000FX\n\n", portNum,val);
                val |= (1<<15);
                WritePhy(pSfpCtl,portNum,18,20,val);
                break;
            default:
                break;
        }
        
    }
    /**未连接上，默认设置光口  */
    else if(pSfpCtl->sfp_port[portNum].i2c_link_status < 0)
    {

        printf("Sfp Port[%d] no cnnected,default config!\n",portNum);
#if 1    
        val = ReadPagePhy(pSfpCtl,portNum,18,20);
        printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_1000FX\n\n", portNum,val);
        val = 0x02;
        val |= (1<<5);
        WritePhy(pSfpCtl,portNum,18,20,val);
        
        val = ReadPagePhy(pSfpCtl,portNum,18,20);
        printf("read port %d phy addr: 18_20 addr value : 0x%x MEDIA_1000FX\n\n", portNum,val);
        val |= (1<<15);
        WritePhy(pSfpCtl,portNum,18,20,val);

#endif
    }

    /**强制连接上  */
    val = ReadPagePhy(pSfpCtl,portNum,1,16);
    printf("read port %d phy addr: 1_16 addr value : 0x%x\n\n",portNum, val);
    val |= (1<<10);
    WritePhy(pSfpCtl,portNum,1,16,val);

    usleep(50000);

    val = ReadPagePhy(pSfpCtl,portNum,0,0);
    printf("read port %d phy page 0 addr value : 0x%x\n\n", portNum,val);
    
    val = ReadPagePhy(pSfpCtl,portNum,0,1);
    printf("read port %d phy page 0 addr value : 0x%x\n\n",portNum,val);
    
    
    val = ReadPagePhy(pSfpCtl,portNum,1,0);
    printf("read port %d phy page 1 addr value : 0x%x\n\n",portNum, val);
    
    val = ReadPagePhy(pSfpCtl,portNum,1,1);
    printf("read port %d phy page 1 addr value : 0x%x\n\n",portNum, val);


    /**适应千兆还是百兆  */
    val = ReadPagePhy(pSfpCtl,portNum,1,17);
    printf("read port %d phy addr: 1_17 addr value : 0x%x\n\n",portNum, val);

    addrOffset= 0x0100*(portNum+1) +0x004d;
    if(val & (1<<15))
    {
        printf("phy dedect speed 1000M\n");
        pSfpCtl->sfp_port[portNum].speed = PORT_SEED_1000M;
        SetFpgaAddressVal(addrOffset,1);
    }
    else if(val & (1<<14))
    {
        printf("phy dedect speed 100M\n");
        pSfpCtl->sfp_port[portNum].speed = PORT_SEED_100M;
        SetFpgaAddressVal(addrOffset,0);
    }
    else
    {
        printf("phy dedect speed fail,default 1000M\n");
        pSfpCtl->sfp_port[portNum].speed = PORT_SEED_1000M;
        SetFpgaAddressVal(addrOffset,0);

    }

}

void Init_Eth0_SfpPhy(SfpCtl *pSfpCtl)
{    
    

    /**从i2c 读取当前sfp  0状态  */
    GetEthxSfp_Status(pSfpCtl,0);
    Init_Ethx_SfpPhy(pSfpCtl,0);
        
}

void AutoAdjust_Sfp(SfpCtl *pSfpCtl,int i)
{
    SfpCtl tmpSfp;
    memset(&tmpSfp,0,sizeof(tmpSfp));
    tmpSfp.i2cfd = pSfpCtl->i2cfd;
            
    GetEthxSfp_Status(&tmpSfp,i);

    if(tmpSfp.sfp_port[i].i2c_link_status != pSfpCtl->sfp_port[i].i2c_link_status
        || (tmpSfp.sfp_port[i].media_type != pSfpCtl->sfp_port[i].media_type))
    {
        /**sfp 从连接到断开  */
        if(tmpSfp.sfp_port[i].i2c_link_status < 0)
        {
            pSfpCtl->sfp_port[i].i2c_link_status = tmpSfp.sfp_port[i].i2c_link_status;
                
        }
        else if(tmpSfp.sfp_port[i].i2c_link_status > 0)
        {
            /**sfp 从断开到连接*/
            pSfpCtl->sfp_port[i].i2c_link_status = tmpSfp.sfp_port[i].i2c_link_status;
            pSfpCtl->sfp_port[i].media_type = tmpSfp.sfp_port[i].media_type;
            
            AutoEthxPhy(pSfpCtl,i);
        }
                 
    }
        
}

Uint32 YearToDay(Uint16 year)
{
	Uint8 LeapYear;
	Uint32 OneYear;
	
	LeapYear = ((year%4 == 0) && (year%100 != 0)) || (year%400 == 0);
	if( LeapYear ) 
	OneYear = 366;
	else
	OneYear = 365;
	return OneYear;
}


Uint32 AllYearSecond(Uint32 endyear,Uint32 day,Uint32 hour,Uint32 minute,Uint32 second)
{
	Uint32 AllYear_second;
	Uint16 a;
	
	AllYear_second = 0;
	for(a=1970; a<endyear; a++)
	{
		AllYear_second += YearToDay(a);
	}
	
	AllYear_second *= 86400;/*到当前整年的秒数*/
	
	AllYear_second += ((day-1)*86400 + hour*3600 + minute*60 + second);/*到当前时间的秒数*/
  
	return AllYear_second;
}

#if 0
unsigned long DclsConvertToAllSecond(char *pDcls_buf)
{
    unsigned int year = 0;
    unsigned int day = 0;
    unsigned int hour = 0;
    unsigned int minute = 0;
    unsigned int second = 0;
    unsigned char time_zone = 0;
    unsigned long all_second = 0;

    /**获得秒数  */
    second = pDcls_buf[0]&0x0F;
    second += ((pDcls_buf[0]>>4)&0x0f) *10;

    /**获得分钟数  */
    minute = pDcls_buf[1]&0x0F;
    minute += ((pDcls_buf[1]>>4)&0x0f) *10;

    /**获得小时  */
    hour = pDcls_buf[2]&0x0F;
    hour += ((pDcls_buf[2]>>4)&0x0f) *10;
    
    time_zone = (pDcls_buf[3]&0x07);
    if((pDcls_buf[7]&0x08) == 0x08)
    {
        hour += time_zone;
    }
    else
    {
        hour -= time_zone;
    }

    day = pDcls_buf[4]&0x0F;
    day += ((pDcls_buf[4]>>4)&0x0f) *10;
    day += (pDcls_buf[5]&0x03)*100;

    year = pDcls_buf[6]&0x0F;
    year += ((pDcls_buf[6]>>4)&0x0f) *10;

    year += (pDcls_buf[7]&0x0F) * 100;
        
    year +=((pDcls_buf[7]>>4)&0x0f) *1000;
    
    all_second = AllYearSecond(year,day,hour,minute,second);


    printf("src sec=%d min=%d hou=%d day=%d year=%d\n",second,minute,hour,day,year);
    return all_second;
}
#endif
#if 0

void SecondConvertToDcls(char *buf,unsigned long all_second)
{
    struct tm *tm;
    int day_month[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    unsigned int year = 0;
    unsigned int mon = 0;
    unsigned int day = 0;
    unsigned int hour = 0;
    unsigned int minute = 0;
    unsigned int second = 0;
    Uint8 leap_year;
    int i;

    all_second += 2;/**校准下一秒  */

    tm = gmtime(&all_second);
    year = tm->tm_year+1900;
    mon = tm->tm_mon+1;
    day = tm->tm_mday;
    hour = tm->tm_hour;
    minute = tm->tm_min;
    second = tm->tm_sec;

    for(i=1;i<mon;i++)
    {
        day += day_month[i];
    }
    
    if(((year%4 == 0) && (year%100 != 0)) || (year%400 == 0))
    {
        leap_year = 1;
        day += 1;

    }
    else
    {
        leap_year = 0;
    }

    printf("sec=%d min=%d hou=%d day=%d year=%d\n",second,minute,hour,day,year);


    CONVERT_INT_BCD(buf[0],second)
    CONVERT_INT_BCD(buf[1],minute)
    CONVERT_INT_BCD(buf[2],hour)
    buf[3] = 0;                 /**时区，暂时不填  */
    
    buf[5] = day / 100;
    day = day%100;
    CONVERT_INT_BCD(buf[4],day)

    CONVERT_INT_BCD(buf[6],year%100);
    CONVERT_INT_BCD(buf[7],year/100)

    buf[8] = 0;                 /**闰秒以后再加  */
    
    if(leap_year)
        buf[9] = 0x01;
    else
        buf[9] = 0x00;
    


}
#endif

#if 0
void SetDclsTimeToFpga(UartDevice *pUartDevice,unsigned long all_second)
{
    char dcls_buf[11];

    memset(dcls_buf,0,sizeof(dcls_buf));

    SecondConvertToDcls(dcls_buf,all_second);
    
    dcls_buf[10] = 0x00;
    SetFpgaDcls(dcls_buf);
    printf("=====------------------set dcls time\n");

}
#endif


#if 0
void HandleBackBusData(UartDevice *pUartDevice,char *back_buf)
{
    BackBusData *pBackBusData = (BackBusData*)back_buf;
    BackBusData *pLocalBusData = &pUartDevice->backBusParam;
    PtpClock* pPtpClock = pUartDevice->pPtpClock;
    NTPD_STATUS *pNtpStatus = pUartDevice->pNtpDev->ntpStatus;
    
    int i = 0;
    if(flip32(pBackBusData->sec)%20==0)
    {
        /**填充NTP 时间源  */
        memset(pNtpStatus->refid,0,5);
        if(pBackBusData->timeSource == 0x00 || pBackBusData->timeSource == 0x01)
            memcpy(pNtpStatus->refid,"GNSS",strlen("GNSS"));
        else if(pBackBusData->timeSource == 0x02 || pBackBusData->timeSource ==0x03)
            memcpy(pNtpStatus->refid,"REF",strlen("REF"));
        else if(pBackBusData->timeSource == 0x04 || pBackBusData->timeSource == 0x05)
            memcpy(pNtpStatus->refid,"REF",strlen("REF"));
        else
            memcpy(pNtpStatus->refid,"LOC",strlen("LOC"));

        /**填充NTP 时钟等级  */
        if(pBackBusData->clockClass == 0xff)
            pNtpStatus->stratum = 15;
        else
            pNtpStatus->stratum = 0;
        
        /**填充NTP 精度  */
        pNtpStatus->precision = pBackBusData->ntpClockAccuracy;

        /**填充NTP 闰秒  */
        switch (pBackBusData->leapFlag)
        {
            case 0x00:
                pNtpStatus->leap = LEAP_NOWARNING;

                break;
            case 0x01:
                pNtpStatus->leap = LEAP_ADDSECOND;
                break;
            case 0x02:
                pNtpStatus->leap = LEAP_DELSECOND;
                break;
            default:
                pNtpStatus->leap = LEAP_NOTINSYNC;
                break;
        }
        /**发送给ntpd  */
        //Send_SocketTo_Ntpd(pNtpStatus->leap,pNtpStatus->stratum,pNtpStatus->refid,4,pNtpStatus->precision);
    }
    
    /**填充PTP  */
    for(i=0;i<PTP_PORT_COUNT;i++)
    {
         if(pPtpClock[i].clockType == PTP_MASTER)
        {
            /**填充PTP 时间源  */
            if(pBackBusData->timeSource == 0x00 || pBackBusData->timeSource == 0x01)
                pPtpClock[i].timePropertiesDS.timeSource = 0x20;
            else if(pBackBusData->timeSource == 0x02 || pBackBusData->timeSource ==0x03)
                pPtpClock[i].timePropertiesDS.timeSource = 0x40;
            else if(pBackBusData->timeSource == 0x04 || pBackBusData->timeSource == 0x05)
                pPtpClock[i].timePropertiesDS.timeSource = 0x90;
            else
                pPtpClock[i].timePropertiesDS.timeSource = 0xA0;

            
            pPtpClock[i].timePropertiesDS.currentUtcOffset = pBackBusData->leapSec;

            switch (pBackBusData->leapFlag)
            {
                case 0x00:
                    pPtpClock[i].timePropertiesDS.leap59 = 0;
                    pPtpClock[i].timePropertiesDS.leap61 = 0;
                    break;
                case 0x01:
                    pPtpClock[i].timePropertiesDS.leap61 = 1;
                    pPtpClock[i].timePropertiesDS.leap59 = 0;
                    break;
                case 0x02:
                    pPtpClock[i].timePropertiesDS.leap61 = 0;
                    pPtpClock[i].timePropertiesDS.leap59 = 1;
                    break;
                default:
                    pPtpClock[i].timePropertiesDS.leap59 = 0;
                    pPtpClock[i].timePropertiesDS.leap61 = 0;
                    break;
            }
            
            pPtpClock[i].grandmasterClockQuality.clockClass = pBackBusData->clockClass;
            pPtpClock[i].grandmasterClockQuality.clockAccuracy = pBackBusData->ptpClockAccuracy;
            
        }

    }

    memcpy(pLocalBusData,pBackBusData,sizeof(BackBusData));
}
#endif

void MaintainCoreTime(UartDevice *pUartDevice,TimeInternal *ptptTime)
{
    
    char m_backBusBuf[16];
    char s_backBusBuf[16];
    char dcls_buf[10];
    
    static Uint32 previousBusSec = 0;

    Uint32 ntp_second_from_1970 =0;
    Uint32 dcls_second_from_1970 =0;
    Uint32 Time = 0;
    
    static short  secErrorCnt = 0;
    static short  ntpsecErrorCnt = 0;
    static short  dclssecErrorCnt = 0;

    static unsigned short modify_internal = 0;
    
    Uint32 fpga_ntp_second_from_1970 =0;

    memset(m_backBusBuf,0,sizeof(m_backBusBuf));
    memset(s_backBusBuf,0,sizeof(s_backBusBuf));
    memset(dcls_buf,0,sizeof(dcls_buf));
    
    GetBackBusData(m_backBusBuf,s_backBusBuf);
    BackBusData *pBackBusData = (BackBusData*)m_backBusBuf;
    BackBusData *pBackBusData_s = (BackBusData*)s_backBusBuf;

    printf("m_time:%d, m_leap:%d\n",flip32(pBackBusData->sec)+1,pBackBusData->leapSec);
    printf("s_time:%d, s_leap:%d\n",flip32(pBackBusData_s->sec)+1,pBackBusData_s->leapSec);
    
    Time = flip32(pBackBusData->sec);
    Time++;/**+1 表示当前秒数  */
    

    ntp_second_from_1970 = ioctl(pUartDevice->dev_fd,COMM_GET_SEC,0);

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
            pUartDevice->powerOn = FALSE;
            SetFpgaAddressVal(0xb2,0x01);

        }
    }
    else if(ptptTime->seconds == Time)
        secErrorCnt = 0;


    
    /**使用本地ptp时间，防止背板出错时，时间出问题  */
    fpga_ntp_second_from_1970 = Time - pUartDevice->backBusParam.leapSec;

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
            ioctl(pUartDevice->dev_fd,COMM_SET_SEC,fpga_ntp_second_from_1970);
                        
            ioctl(pUartDevice->dev_fd,COMM_MODIFY_SEC,1);

            ntpsecErrorCnt = 0;
        }

    }
    else if(ntp_second_from_1970 == fpga_ntp_second_from_1970)
    {
        modify_internal++;
        printf("------6-------\n");
        if(modify_internal%4)
            ioctl(pUartDevice->dev_fd,COMM_MODIFY_SEC,1);
        ntpsecErrorCnt = 0;
    }
    
}

void DisplayAlarm(UartDevice *pUartDevice)
{

    PadAlarm *pPadAlarm = &pUartDevice->padAlarm;

    printf("------------------------------------------------\n");
    printf("disk_alarm=%x\n",pUartDevice->diskAlarm);
    printf("slot_1_RcuAlarm=%x,slot_2_RcuAlarm=%x\n",pPadAlarm->slot_1_RcuAlarm,pPadAlarm->slot_2_RcuAlarm);
    printf("slot_1_Master_Backup=%x,slot_2_Master_Backup=%x\n",pPadAlarm->slot_1_Master_Backup,pPadAlarm->slot_2_Master_Backup);
    
    printf("slot_1_100Alarm=%x,slot_2_100Alarm=%x\n",pPadAlarm->slot_1_100Alarm,pPadAlarm->slot_2_100Alarm);
    printf("slot_1_1ppsAlarm=%x,slot_2_1ppsAlarm=%x\n",pPadAlarm->slot_1_1ppsAlarm,pPadAlarm->slot_2_1ppsAlarm);
    printf("slot_1_TodAlarm=%x,slot_2_TodAlarm=%x\n",pPadAlarm->slot_1_TodAlarm,pPadAlarm->slot_2_TodAlarm);
    
    printf("master100Alarm=%x,master1ppsTodAlarm=%x\n",pPadAlarm->master100Alarm,pPadAlarm->master1ppsTodAlarm);
    printf("twoMasterAlarm=%x,twoBackupAlarm=%x\n",pPadAlarm->twoMasterAlarm,pPadAlarm->twoBackupAlarm);
    printf("slot_1_TodCrcAlarm=%x,slot_2_TodCrcAlarm=%x\n",pPadAlarm->slot_1_TodCrcAlarm,pPadAlarm->slot_2_TodCrcAlarm);
    printf("------------------------------------------------\n");

}

void HandleAlarm(UartDevice *pUartDevice)
{
    PadAlarm *pPadAlarm = &pUartDevice->padAlarm;
    PtpClock *pPtpClock = pUartDevice->pPtpClock;
    static char s_alarmOutput = 1;
    
    static Uint16 cnt = 0;
    static Uint16 alarmCnt = 0;
    
    if(pUartDevice->diskAlarm)
        pUartDevice->alarmOutput = TRUE;
    else
        pUartDevice->alarmOutput = FALSE;

    
    if(s_alarmOutput != pUartDevice->alarmOutput)
    {
        if((pUartDevice->alarmOutput == TRUE) && (pUartDevice->powerOn == FALSE))
        {

            //Control_LedStatus(LED_PTP_1,0);
            //Control_LedStatus(LED_PTP_2,0);
            
            BlockNtpOutput(pPtpClock);
            BlockPtpOutput(pPtpClock);
            s_alarmOutput = pUartDevice->alarmOutput;

        }
        else if((pUartDevice->alarmOutput == FALSE) && (pUartDevice->powerOn == FALSE))
        {
            /**告警15s确认  */
            if(alarmCnt >= 15)
            {
                //Control_LedStatus(LED_PTP_1,1);
                //Control_LedStatus(LED_PTP_2,1);
                
                ResumeNtpOutput(pPtpClock);
                ResumePtpOutput(pPtpClock);
                
                s_alarmOutput = pUartDevice->alarmOutput;

                alarmCnt = 0;

            }
            alarmCnt++;

        }
        
    }

    
    if(!pUartDevice->diskAlarm)
    {
#if 0
        if(pPtpClock[0].notAvailableAlarm == FALSE)
            Control_LedStatus(LED_PTP_1,1);
        else
            Control_LedStatus(LED_PTP_1,0);
#endif

    }

    /**15s 软件确认  */
    if(pPadAlarm->master1ppsTodAlarm)
    {
        SetFpgaAddressVal(0xb3,0x00);
    }
    else
    {
        if(cnt >= 15)
        {
            SetFpgaAddressVal(0xb3,0x01);
            cnt = 0;
        }
        cnt++;
    }

    
}

void SelectBestClockPhase(PtpClock *pPtpClock,short *pBestIndex,Uint16 nCount)
{
    int i;

    short iBest = 0;
    short cnt = 0;
    
    if(nCount%5 == 0)
    {
        for(i=0;i<PTP_PORT_COUNT;i++)
        {
            if((pPtpClock[i].clockType == PTP_SLAVE) && (pPtpClock[i].notAvailableAlarm == FALSE))
            {
                if(bmcDataSetComparison(&pPtpClock[iBest].BestMasterRecord.header
                    ,&pPtpClock[iBest].BestMasterRecord.announce
                    ,&pPtpClock[i].BestMasterRecord.header
                    ,&pPtpClock[i].BestMasterRecord.announce) <= 0)
                {
                    iBest = i;
                }
            }
            else
            {
                cnt++;
            }
        }

        if(cnt >= PTP_PORT_COUNT)
            *pBestIndex = 0xff;
        else
            *pBestIndex = iBest;

    }
}


void *ThreadUsuallyProcess(void *p)
{
    time_t current_time;
    Uint32 UtcTime;
    TimeInternal timeTmp;
    struct tm *tm;
    static Uint16 nTimeCnt = 0;
    UartDevice *pUartDevice = &g_UartDevice;

    short bestIndex = 0;
    int i;
    
    Boolean ledRun = FALSE;

    printf("usually process...\n");


    Init_SfpCtl(&pUartDevice->sfpCtl);
    Init_Eth0_SfpPhy(&pUartDevice->sfpCtl);

    while(1)
	{

        if(pUartDevice->pps_routine_flag)
        {
            //printf("========================1PPS Start ============================\n");
             /**处理其他  */
            nTimeCnt++;


            /**获取当前运行时间  */
            GetFpgaRuningTime(&timeTmp);
            //printf("fgpa Running time:sec=%d,nao=%d \n",timeTmp.seconds,timeTmp.nanoseconds);
#if 0

            GetFpgaPpsTime(&timeTmp);
            printf("fgpa lock time:sec=%d,nao=%d \n",timeTmp.seconds,timeTmp.nanoseconds);

            //GetSystemPpsTime(&timeTmp); 
            printf("fgpa system time:sec=%d,nao=%d ",timeTmp.seconds,timeTmp.nanoseconds);
  #endif      
            for(i=0;i<PTP_PORT_COUNT;i++)
                g_ptpClock[i].CurentTime= timeTmp;
            
            current_time = timeTmp.seconds;
            tm = gmtime(&current_time);
            //printf("fgpatime:%d/%d/%d %d:%d:%d\n",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);

            /**核心时间维护  */
#if 0
            MaintainCoreTime(pUartDevice,&timeTmp);

            CollectAlarm(pUartDevice);
            DisplayAlarm(pUartDevice);
            if(!pUartDevice->powerOn)
                HandleAlarm(pUartDevice);
            SelectBestClockPhase(g_ptpClock,&bestIndex,nTimeCnt);
#endif
            
#if 0
            /**1000s定时上报  */    
            msgPackSlaveUpOnTimeUp(pUartDevice,g_ptpClock,nTimeCnt);
            
            msgPackNetOnTimeUp(pUartDevice,nTimeCnt+1);
            msgPackPtpOnTimeUp(pUartDevice,nTimeCnt+2);
            msgPackNtpOnTimeUp(pUartDevice,nTimeCnt+3);
            msgPackAlarmOnTimeUp(pUartDevice,nTimeCnt+4);
            
            msgPackOffset1sOnTimeUp(pUartDevice,g_ptpClock,bestIndex);
            msgPackAlarmChangeCommand(pUartDevice,nTimeCnt);
#endif

            if(nTimeCnt % 7 == 0)
                AutoAdjust_Sfp(&pUartDevice->sfpCtl,0);

            pUartDevice->pps_routine_flag = FALSE;
        
            //printf("========================1PPS End ============================\n\n\n");
        }
        
       
        usleep(10);
    }
}

void error_die(const char *sc)
{
    perror(sc);
    exit(1);
}



