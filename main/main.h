#ifndef __MAIN_H__
#define __MAIN_H__

#include "common.h"
#include "list_ctrl.h"
#include "satellite_api.h"
#include "clock_rb.h"

#define SLOT_COUNT 4
#define SLOT_TYPE_CORE 'A'
#define SLOT_TYPE_PTP  'B'
#define SLOT_TYPE_NTP  'C'

#define MAIN_SCREEN_ID          0x00
#define CLOCK_STATUS_SCREEN_ID  0x01
#define PARAM_SETING_SCREEN_ID  0x02
#define WARN_SCREEN_ID          0x03
#define ETH_CTL_SCREEN_ID       0x04
#define ETH_PTP_SCREEN_ID       0x05
#define ETH_NTP_SCREEN_ID       0x06

#define CLOCK_STATUS_WIDGET_ID      12
#define CLOCK_SETTING_WIDGET_ID     13
#define CLOCK_WARNNING_WIDGET_ID    14

#define	LEAP_NOWARNING	0x0	/* normal, no leap second warning */
#define	LEAP_ADDSECOND	0x1	/* last minute of day has 61 seconds */
#define	LEAP_DELSECOND	0x2	/* last minute of day has 59 seconds */
#define	LEAP_NOTINSYNC	0x3	/* overload, clock is free running */

#define STRATUM_0_PRESION -20
#define STRATUM_1_PRESION -20
#define STRATUM_2_PRESION -18
#define STRATUM_3_PRESION -15


enum
{
    ENUM_SLOT_COR_ADDR = 0x00,
    ENUM_SLOT_1_ADDR,
    ENUM_SLOT_2_ADDR,
    ENUM_SLOT_3_ADDR,
    ENUM_SLOT_4_ADDR,
    ENUM_SLOT_PC_ADDR = 0xff
};



struct ServerList
{
    Uint32 serverIp;            /** 单播服务器ip */
    char  serverMac[6];         /** 单播服务器mac */    
};


struct PtpSetCfg
{
    Uint8 clockType;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** 域号 */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */
    Uint8 domainFilterSwitch;            /** 域号过滤开关0:关，1:开 */
    Uint32 UnicastDuration;       /** 单播协商机制，老化时间 */
    Integer8 logSyncInterval;
    Integer8 logAnnounceInterval;
    
    Integer8 logMinDelayReqInterval;
    Integer8 logMinPdelayReqInterval;
   
    UInteger8 grandmasterPriority1;
    UInteger8 grandmasterPriority2;
    Uint8 validServerNum;              /** 有效的服务器ip数量 */
    Uint8 currentUtcOffset;
    struct ServerList serverList[10];
};

struct PtpStatusCfg
{
    Uint8 synAlarm;
    Uint8 announceAlarm;
    Uint8 delayRespAlarm;
    Uint8 notAvailableAlarm;
    long long MeanPathDelay;
    long long TimeOffset;
    
};

struct Md5key
{
	Uint8 key_valid;		//the md5 key is valid or not:1 valid ; 2 invalid
	Uint8 key_length;		//the length of the md5 key
	Uint8 key[20];		    //the md5 key in ASCII 
	
};


struct NtpSetCfg
{
	Uint8       net_flag;			//the flag about network parameter
	Uint8       md5_flag;			//the flag about md5 authentication   bit1--1=Enable md5;0=Disable md5 ;    bit2--0= broadcast Closed;1=setted key number;  bit3--0= multicast Closed;1=setted key number;
	struct Md5key     current_key[10];    //the space to store md5 key structure
    unsigned char stratum;   //时钟源等级
    unsigned char leap;      //闰秒
    signed char precision;   //精度
    unsigned char refid[5];  //ID
	Uint32    freq_b;                   // broadcast send period   ntpd do not have
	Uint32    freq_m;                   // multicast send period   ntpd do not have
    Uint8     broadcast;                    // flag: 0 stop 1 run
    Uint8     broadcast_key_num;
	Uint8     multicast;                    // flag
    Uint8     multicast_key_num;
    Uint8     sympassive;                   // flag
	Uint8     blacklist;                    // flag
	Uint8     whitelist;                    // flag

    Uint8     blacklist_num;
    Uint8     whitelist_num;
	Uint8     blacklist_flag[16];           //黑名单标识列表
	Uint8     whitelist_flag[16];           //白名单标识列表
	Uint32    blacklist_ip[16];             //黑名单ip地址列表
	Uint32    blacklist_mask[16];	        //黑名单子网掩码列表
	Uint32    whitelist_ip[16];             // 白名单ip地址列表
	Uint32    whitelist_mask[16];	        // 白名单子网掩码列表

};

struct SlotList
{
    char slot_type;     /**盘类型  */
    struct NtpSetCfg *pNtpSetCfg;
    struct PtpSetCfg *pPtpSetcfg;
    struct PtpStatusCfg *pPtpStatusCfg; 
};


struct root_data{

		struct dev_head dev_head;	//设备信息头
        struct device dev[12];		//使用设备信息
        int dev_fd;
        
        struct SlotList slot_list[6];

        int lcd_sreen_id;
        
        char Version[4];

        /*钟控算法*/
        struct clock_info clock_info;    
              
        /*卫星数据*/
        struct Satellite_Data satellite_data;  

        FILE *file_fd;

        Uint8 flag_usuallyRoutine;

        struct NetInfor comm_port;
        struct NetInfor ptp_port;
        struct NetInfor ntp_port;

        int ctl_sin_port;
        char ctl_ip_address[20];

        
        char current_time[30];
        
        
		pthread_attr_t pattr;	//线程属性
		pthread_t p_usual;		//日常事务处理
		pthread_t p_recv;		//数据接收线程
		pthread_t p_send;		//数据发送线程
		pthread_t p_handle;		//数据处理线程
};

struct NtpdStatus
{
    unsigned char stratum;   //时钟源等级
    unsigned char leap;      //闰秒
    signed char precision;   //精度
    unsigned char refid[5];  //ID
};

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
    
};

extern struct root_data *g_RootData;
extern char *ctlEthConfig;
extern char *ptpEthConfig;
extern char *ntpEthConfig;
extern char *ptpConfig;
extern char *ntpConfig;
extern char *md5Config;


extern void start_ptp_daemon();
extern void stop_ptp_daemon();
extern void start_ntp_daemon();
extern void stop_ntp_daemon();

#endif
