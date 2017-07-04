/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       data_type.h
*    功能描述:       定义ptp所需结构体，宏定义，默认配置
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-4-27
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-4-27
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/

#ifndef __DATA_TYPE_H__
#define __DATA_TYPE_H__

#include "common.h"

#define PTP_PORT_COUNT 1

enum
{
    PTP_SLAVE = 0,
    PTP_MASTER,   
    PTP_PASSIVE,
    NTP_SERVER,
};

/* IP transmission mode */
enum 
{
	IPMODE_MULTICAST = 0,
	IPMODE_UNICAST,
};

enum {
	LI61=0,
	LI59,
	UTCV,
	PTPT, /* this is referred to as PTP in the spec but already defined above */
	TTRA,
	FTRA
};


enum
{
	PTP_ALTERNATE_MASTER = 0x01,
	PTP_TWO_STEP = 0x02,
	PTP_UNICAST = 0x04,
	PTP_PROFILE_SPECIFIC_1 = 0x20,
	PTP_PROFILE_SPECIFIC_2 = 0x40,
	PTP_SECURITY = 0x80,
};

/** 时钟类型 */
enum
{
	ATOMIC_CLOCK=0x10,
    GPS=0x20,
    TERRESTRIAL_RADIO=0x30,
    PTP=0x40,
    NTP=0x50,
    HAND_SET=0x60,
    OTHER=0x90,
    INTERNAL_OSCILLATOR=0xA0
};

/**消息内部 协议类型值  */
enum {
	UDP_IPV4=1,
    UDP_IPV6,
    IEEE_802_3,
    DeviceNet,
    ControlNet,
    PROFINET
};

/**运行模式  */
#define ONE_STEP             0
#define TWO_STEP             1
#define PROTO_IEEE802        0
#define PROTO_UDP_IP         1
#define DELAY_MECANISM_P2P   0
#define DELAY_MECANISM_E2E   1

#define BASE_T_10            0
#define BASE_T_100           1

#define MINUS_VALUE          2
#define PLUS_VALUE           1   
#define CANT_CHOICE          0

/** 发送接收buffer 长度 */
#define PACKET_SIZE  300 

#define PTP_EVENT_PORT                319
#define PTP_GENERAL_PORT              320
#define PTP_E2E_MULTICAST_ADDRESS     "224.0.1.129"
#define PTP_PEER_MULTICAST_ADDRESS    "224.0.0.107"

#define MAC_ADDR_LENGTH               6
#define SUBDOMAIN_ADDRESS_LENGTH      4
#define PORT_ADDRESS_LENGTH           2
#define PTP_UUID_LENGTH               6
#define CLOCK_IDENTITY_LENGTH	      8
#define FLAG_FIELD_LENGTH             2


/* implementation specific constants */
#define DEFAULT_INBOUND_LATENCY      	0       /* in nsec */
#define DEFAULT_OUTBOUND_LATENCY     	0       /* in nsec */
#define DEFAULT_NO_RESET_CLOCK       	FALSE
#define DEFAULT_DOMAIN_NUMBER        	0
#define DEFAULT_DELAY_MECHANISM      	E2E     // TODO
#define DEFAULT_AP                   	10
#define DEFAULT_AI                   	1000
#define DEFAULT_DELAY_S              	6
#define DEFAULT_ANNOUNCE_INTERVAL    	1      /* 0 in 802.1AS */
#define LEAP_SECOND_PAUSE_PERIOD        2      /* how long before/after leap */


/**见1558协议Page55 表5  */
#define DEFAULT_CLOCK_CLASS				248
#define LOCK_CLOCK_CLASS                6
#define UNLOCK_CLOCK_CLASS              7
#define USENSS_CLOCK_CLASS              52


/**见1558协议Page56 表6  */
#define DEFAULT_CLOCK_ACCURACY		    0xFE

#define CLOCK_ACCURACY_25ns		        0x20
#define CLOCK_ACCURACY_100ns		    0x21
#define CLOCK_ACCURACY_250ns		    0x22
#define CLOCK_ACCURACY_1us		        0x23
#define CLOCK_ACCURACY_2P5us		    0x24
#define CLOCK_ACCURACY_10us		        0x25
#define CLOCK_ACCURACY_25us		        0x26
#define CLOCK_ACCURACY_100us		    0x27


#define DEFAULT_PRIORITY1		        128        
#define DEFAULT_PRIORITY2		        128 


/** 定义数据包长度 */
#define HEADER_LENGTH					34
#define ANNOUNCE_LENGTH					64
#define SYNC_LENGTH					    44
#define FOLLOW_UP_LENGTH				44
#define PDELAY_REQ_LENGTH				54
#define DELAY_REQ_LENGTH				44
#define DELAY_RESP_LENGTH				54
#define PDELAY_RESP_LENGTH 				54
#define PDELAY_RESP_FOLLOW_UP_LENGTH  	54
#define MANAGEMENT_LENGTH				48

#define SIGNAL_REQUEST_LENGTH            54
#define SIGNAL_CANCEL_LENGTH             50
#define SIGNAL_GRANT_LENGTH              56
#define SIGNAL_ACKNOWLEDGE_CANCEL_LENGTH 50


#define IEEE802_HEAD_LEN                18
#define ETHER_HDR_LEN                   8
#define MAXHOSTNAMELEN                  10
#define IFCONF_LENGTH                   10
#define ADJ_FREQ_MAX                    500000
#define NANOSECONDS_MAX                 999999999


/**head 中 消息类型定义  */
#define TYPE_SYN                    0x00
#define TYPE_DELAY_REQ              0x01
#define TYPE_PDELAY_REQ             0x02
#define TYPE_PDELAY_RESP            0x03

#define TYPE_FOLLOWUP               0x08
#define TYPE_DELAY_RESP             0x09
#define TYPE_PDELAY_RESP_FOLLOWUP   0x0a
#define TYPE_ANNOUNCE               0x0b
#define TYPE_SIGNALING              0x0c
#define TYPE_MANAGEMENT             0x0d


/* features, only change to refelect changes in implementation */
#define NUMBER_PORTS      	1

/**版本定义  */
#define PTP_VERSION_1   1
#define PTP_VERRION_2   2

/** 控制字段中消息类型定义 */
#define COTROL_TYPE_SYN         0x00
#define COTROL_TYPE_DELAY_REQ   0x01
#define COTROL_TYPE_FOLLOWUP    0x02
#define COTROL_TYPE_DELAY_RESP  0x03
#define COTROL_TYPE_MANAGEMENT  0x04
#define COTROL_TYPE_ALL_OHTERv  0x05

#define DEFAULT_LOGMSG_INTERVAL 0x7F

#define IFACE_NAME_LENGTH         IF_NAMESIZE
#define NET_ADDRESS_LENGTH        INET_ADDRSTRLEN

/**sinal 消息 tlvType   */
#define REQUEST_UNICAST_TRANSMISSION            0x04
#define GRANT_UNICAST_TRANSMISSION              0x05
#define CANCEL_UNICAST_TRANSMISSION             0x06
#define ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION 0x07

#define SIGNAL_REQUEST_TLV_LENGTH                   6
#define SIGNAL_GRANT_TLV_LENGTH                     8
#define SIGNAL_ACKNOWLEDGE_CANCEL_TLV_LENGTH        2
#define SIGNAL_CANCELL_TLV_LENGTH                   2


#define SIGNAL_SYN_TYPE                         0x00  
#define SIGNAL_ANNOUNCE_TYPE                    0xb0
#define SIGNAL_DELAY_RESP_TYPE                  0x90
#define SIGNAL_PDELAY_RESP_TYPE                 0x30

#define MAX_CLIENT      	200
#define MAX_SERVER          10

/**发送线程中1s 检测执行  */
#define TIMER_1_SECOND      256

#define SIGNAL_MSG_NULL         0
#define SIGNAL_SYN_MSG          1
#define SIGNAL_ANNOUNCE_MSG     2
#define SIGNAL_DELAYRESP_MSG    3
#define SIGNAL_PDELAYRESP_MSG   4


/* UDP/IPv4 dependent */
#ifndef INADDR_LOOPBACK
#define INADDR_LOOPBACK 0x7f000001UL
#endif

typedef enum
{
    LED_PTP_1 = 1,
    LED_PTP_2,
    LED_PTP_3,
    LED_PTP_4,
    LED_PTP_5,
    LED_PTP_6,
    LED_PTP_7,
    LED_PTP_8,
}LedPin;


typedef struct
{
    UInteger16 checkRecvAlarm_Counter20s;   /**检测接收包告警计数  */
    UInteger16 displayStauts_Counter10s;    /**打印消息周期计数  */
    UInteger16 sendControl_index;           /**发送signal 计数  */
    UInteger16 signalDuration_index;        /**signal 消息生存时间计数  */
    
}PtpdCounters;




typedef struct
{
    Uint8 h_dest[6];
    Uint8 h_source[6];
    Uint8 vlanType1;
    Uint8 vlanType2;
    Uint8 PriCfi;
    Uint8 vlanId;
    Uint8 h_proto_1;
    Uint8 h_proto_2;
}VlanHeader;

typedef struct
{
    Uint8 h_dest[6];
    Uint8 h_source[6];
    Uint8 h_proto_1;
    Uint8 h_proto_2;
}Ieee802Head;

typedef struct {
    Integer32 seconds;
    Integer32 nanoseconds;
}TimeInternal;


typedef struct {
    UInteger48 secondsField;
    UInteger32 nanosecondsField;
}Timestamp;


typedef char ClockIdentity[CLOCK_IDENTITY_LENGTH];


typedef struct {
    ClockIdentity clockIdentity;
    UInteger16 portNumber;
}PortIdentity;




typedef struct {
    Enumeration16 networkProtocol;
    UInteger16 addressLength;
    Octet* addressField;
}PortAddress;


typedef struct {
    UInteger8 clockClass;
    Enumeration8 clockAccuracy;
    UInteger16 offsetScaledLogVariance;
}ClockQuality;


typedef struct {
    NibbleUpper transportSpecific;
    Enumeration4Lower messageType;
    NibbleUpper reserved0;
    UInteger4Lower versionPTP;
    UInteger16 messageLength;
    UInteger8 domainNumber;
    Octet reserved1;
    Octet flagField0;
    Octet flagField1;
    Integer64 correctionField;
    UInteger32 reserved2;
    PortIdentity sourcePortIdentity;
    UInteger16 sequenceId;
    UInteger8 controlField;
    Integer8 logMessageInterval;
}MsgHeader;



typedef struct {
    Timestamp originTimestamp;
    Integer16 currentUtcOffset;
    UInteger8 grandmasterPriority1;
    ClockQuality grandmasterClockQuality;
    UInteger8 grandmasterPriority2;
    ClockIdentity grandmasterIdentity;
    UInteger16 stepsRemoved;
    Enumeration8 timeSource;
}MsgAnnounce;



typedef struct {
    Timestamp originTimestamp;
}MsgSync;


typedef struct {
    Timestamp originTimestamp;
}MsgDelayReq;

typedef struct {
    Timestamp receiveTimestamp;
    PortIdentity requestingPortIdentity;
}MsgDelayResp;


typedef struct {
    Timestamp preciseOriginTimestamp;
}MsgFollowUp;


typedef struct {
    Timestamp originTimestamp;
}MsgPDelayReq;


typedef struct {
    Timestamp requestReceiptTimestamp;
    PortIdentity requestingPortIdentity;
}MsgPDelayResp;


typedef struct {
    Timestamp responseOriginTimestamp;
    PortIdentity requestingPortIdentity;
}MsgPDelayRespFollowUp;


typedef struct {
    PortIdentity targetPortIdentity;
    UInteger16 tlvType;
    UInteger16 lengthField;
}TlvNormal;

typedef struct {
    TlvNormal tlv;
    UInteger8  messageType;
    Integer8   logInterMessagePeriod;
    UInteger32 durationField;
}SignalReq;

typedef struct {
    TlvNormal  tlv;
    UInteger8  messageType;
    Integer8   logInterMessagePeriod;
    UInteger32 durationField;
    Integer8   reserved;
    Integer8   R;
}SignalGrant;

typedef struct {
    TlvNormal  tlv;
    UInteger8  messageType;
    Integer8   reserved;
}SignalCancel;

typedef struct {
    TlvNormal  tlv;
    UInteger8  messageType;
    Integer8   reserved;
}SignalAckCancel;

typedef struct {
    TlvNormal tlv;
    UInteger8  messageType;
}MsgSignaling;



typedef struct {
    Enumeration16 tlvType;
    UInteger16 lengthField;
    Enumeration16 managementId;
    Octet* dataField;
}ManagementTLV;

typedef struct
{
    UInteger16 currentUtcOffset;
    Boolean currentUtcOffsetValid;
    Boolean leap59;
    Boolean leap61;
    Boolean timeTraceable;
    Boolean frequencyTraceable;
    Boolean ptpTimescale;
    Enumeration8 timeSource;
}TimePropertiesDS;

typedef struct
{
    PortIdentity foreignMasterPortIdentity;
    UInteger16 foreignMasterAnnounceMessages;

    MsgAnnounce announce;
    MsgHeader header;
    UInteger16 changeCnt;
}ForeignMasterRecord;


typedef struct 
{
    char ifaceName[5];  /** 所在网口号 */
    
    Uint32 ip;          /** 本机IP 地址 */
    Uint32 gwip;		/** 本机网关*/
    Uint32 mask;		/** 本机子网掩码地址*/
    Uint8  mac[6];		/**本机MAC 地址 */
}NetInfor,*PNetInfor;

typedef struct
{
    /*UDP IP*/
    Sint32 eventSock, generalSock; 
    Sint32 PeventSock, PgeneralSock;

    /*IEEE802.3*/
    Integer32 IeeeE2ESock,IeeeP2PSock;
    
    Uint32 multicastAddr;
    Uint32 unicastAddr;
     
    Uint8 UnicastMac[6];
    
    unsigned char E2EMulticastMac[6];   
    unsigned char P2PMulticastMac[6];

    fd_set global_rdfs;
    int maxfd;
}NetComm,*PNetComm;



typedef struct                          
{
	Uint8 valid;		 				  /** client有效标记，1:有效，0:无效 */
    
	Uint8 send_sync_enable;	              /** 单播协商关:1，单播协商开grand之后为1 */
	Uint8 send_announce_enable;	          /**  单播协商关:1,    单播协商开grand 之后为1*/
	Uint32 client_ip;					  /**client ip地址*/
	Uint8 client_mac[6];				  /**client mac地址*/
	int    client_counter;                /**更新client参数计数*/
	Uint32 durationField;                 /**老化时间,单位秒*/
	
}ClienList;                              /**单播模式下，server收到的cllient参数表*/

typedef struct
{
    ClienList clientList[MAX_CLIENT];     /** 客户端列表 */
    Uint16 syn_index;                     /**syn 发送的位置偏移  */
    Uint16 announce_index;                /**announce 发送的位置偏移  */
}UnicastClient;

typedef struct
{
    Uint8 currentUtcOffset;     //当前utc 偏移量
    Uint8 priority1;            //时钟优先级1 
    Uint8 priority2;            //时钟优先级2
    
    PortIdentity portIdentity;  //时钟ID
    
    ClockQuality clockQuality;  // 时钟质量
    Uint8 grandmastIdentity[8];
    Uint8 setpRemoved;          //路径距离
    Uint8 timeSource;           //时钟源
}AnnounceParam;


typedef struct
{
    Uint32 serverIp;            /** 单播服务器ip */
    char  serverMac[6];         /** 单播服务器mac */
    Uint8 valid;                /**  ip 是否有效*/
    Uint8 recv_sigsyn_flag;     /** 1: 收到  0: 未收到 */
    Uint8 recv_sigann_flag;     /** 1: 收到  0: 未收到 */
    Uint8 recv_sigdela_flag;    /** 1: 收到  0: 未收到 */
    Uint8 sig_alarm;            /** 告警1， 不告警0 */

    Uint8 send_delayreq_enable;  /**1,可以发送，0，不能发送  */
    Uint8 finishT1T2_flag;      /**  1 :完成 0 : 不完成*/
    
    TimeInternal T1;
    TimeInternal T2;
    TimeInternal T3;
    TimeInternal T4;
    TimeInternal T5;
    TimeInternal T6;

    TimeInternal SynCorrection;
    TimeInternal FollowUpCorrection;
    TimeInternal DelayrespCorrection;
    TimeInternal PdelayrespCorrection;
    TimeInternal PdelayrespFollowUpCorrection;

    Integer8   logInterMessagePeriod;
    
    TimeInternal pathMeanDelay;
    TimeInternal timeOffset;
    
}ServerList;

typedef struct
{
    ServerList serverList[MAX_SERVER];   /** 服务器ip地址或者mac地址列表 */
    Uint16 index;                       /** 所在列表位置 */
    Uint16 delay_index;                 /** 用于控制 delayreq发送 */  
    Uint8 validServerNum;              /** 有效的服务器ip数量 */               
}UnicastMultiServer;

typedef struct 
{
    Uint8 line_type;                /**线路类型，直连:0，经过交换机:1 */
    Uint8 ctl_phase_type;           /**调相控制类型0:微调，1:快速调整  */

    short over2sec_counter;         /**超过2s计数器  */
    short over1us_counter;          /**超过1us计数器  */
    short low1us_counter;           /**小于1us计数器  */
    short over5us_cnt;              /**微调判断超过5us计数器  */
    short low5us_cnt;               /**微调判断小于5us 计数器  */
    short in_fastmodify_cnt;        /**处于快捕状态计数  */
    
    Slonglong64 mean_path;          /**线路延时  */
    Slonglong64 time_offset;        /**时间偏差  */ 

    Slonglong64 time_offset_buf[10];/** 时间偏差缓存buffer  */
    Slonglong64 phase_adjsut;       /**相位调整值  */
    Slonglong64 averag_tmp;         /**平均线路时延  */


    short cnt_threshold;          /**计数门限，10 x logsynInterval */
    short ratediff_cnt;           /**用于采样ratediff 计数 */

    Uint8 diff_index;                   /**频差缓存的索引  */
    Slonglong64 diff_buf[5];            /**频偏缓存  */
    Slonglong64 calman_diff_buf[5];     /**经卡尔曼滤波后缓存  */
    
    TimeInternal lastT1;                /**上一次T1 值  */
    TimeInternal lastT2;                /** 上一次T2 值 */

    Slonglong64 RateDiff;               /**RateDiff 调整值  */
    Slonglong64 OverMaxRateCnt;         /**超过rateDiff门限计数  */
    
}PhyControl;

typedef struct 
{
    Uint8 portEnable;              /** 1:enable,  0: disable */
    Uint8 clockType;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** 域号 */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */
    Uint8 profile;                /**  profile 配置*/
    
    Uint8 versionNumber;          /** 版本 */
    Uint32 UnicastDuration;       /** 单播协商机制，老化时间 */
    LOG_DEBUG debugLevel;             /**debug 信息   */

    /**vlan 相关变量  */
    Uint8 vlanEnable;             /** 0:disable, 1:enable */
    Uint8 vlanPriority;           /** 优先级0~7 */
    Uint8 vlanId;                 /** ID 0~255 */
    Uint8 vlanCfi;                /** 0:标准Mac，1:非标准Mac , 默认为0*/
    

    /**域号过滤开关  */
    Uint8 domainFilterSwitch;            /** 域号过滤开关0:关，1:开 */
    
    /** announce 消息主时钟的时候赋给值*/
    /** 从时钟的时候获取最佳主时钟值 */
    TimePropertiesDS timePropertiesDS;
    UInteger16 stepsRemoved;


    Uint8 initDp83640Sucess;    /**初始化83640 完成1:完成   0: 未完成  */
    
    /**最佳主时钟  */
	PortIdentity parentPortIdentity;
	ClockIdentity grandmasterIdentity;
	ClockQuality grandmasterClockQuality;
	UInteger8 grandmasterPriority1;
	UInteger8 grandmasterPriority2;

    /**最佳主时钟，用于BMC 计算  */
    ForeignMasterRecord BestMasterRecord;
    Uint8 InitBmcflag;     
    Uint32 BestMasterIp;
    Uint8 BestMasterMac[6];


    /** head 中的发包频率 */
    Integer8 logSyncInterval;
    Integer8 logAnnounceInterval;
    Integer8 logMinPdelayReqInterval;
    Integer8 logMinDelayReqInterval;

    
    /**本地网口环境  */
    NetInfor netEnviroment;

    /**本地ProtIdentity  */
    PortIdentity portIdentity;  

    /**发送接收环境 */
    NetComm  netComm;


    /**计数发送周期  */
    Uint16 synSendInterval;
    Uint16 announceSendInterval;
    Uint16 delayreqInterval;
    Uint16 signalSendInterval;


    /**发送的sequneceId  */
    Uint16 sentSyncSequenceId;              
    Uint16 sentAnnounceSequenceId;
    Uint16 sentDelayReqSequenceId;
    Uint16 sentPDelayReqSequenceId;
    Uint16 sentSignalSequenceId;

    /**收到的sequenceId  */
    Uint16 recvSynReqSequenceId;
    Uint16 recvDelayReqSequenceId;
    Uint16 recvPDelayReqSequenceId;


    /**接收到消息标志  */
    Uint8 recvSynFlag;
    Uint8 recvAnnounceFlag;
    Uint8 recvDelayRespFlag;

    /** 报警  */
    Uint8 synAlarm;
    Uint8 announceAlarm;
    Uint8 delayRespAlarm;
    Uint8 InclockClassAlarm;
    Uint8 notAvailableAlarm;

    TimeInternal T1;
    TimeInternal T2;
    TimeInternal T3;
    TimeInternal T4;
    TimeInternal T5;
    TimeInternal T6;
    
    TimeInternal MSDelay;
    TimeInternal SMDelay;
    TimeInternal SMDelay2;

    TimeInternal SynCorrection;
    TimeInternal FollowUpCorrection;
    TimeInternal DelayrespCorrection;
    TimeInternal PdelayrespCorrection;
    TimeInternal PdelayrespFollowUpCorrection;

    TimeInternal MeanPathDelay;
    TimeInternal TimeOffset;

    //Uint8 finishT3T4;                       /**完成T3 T4接收  */
    Uint8 finishPtp;                        /**完成一次数据收集，用于传送给UI  */

    TimeInternal CurentTime;                /**实时时间  */
    Uint16 oneSecondTime;                   /**用于发送线程中1s处理  */
    Uint8  netAccessSpeed;                  /** 网口速率0:10 Base-T,   1: 100 Base-T */

    UnicastMultiServer unicastMultiServer;  /**单播多服务器控制，从时钟使用  */
    UnicastClient      unicastClient;       /** 客户端控制，主时钟使用 */

    
    char msgObuf[PACKET_SIZE];              /**发送缓存  */
    char msgIbuf[PACKET_SIZE];              /** 接收缓存 */

    
    PtpdCounters counters;                  /**计数器  */


    PhyControl phyControl;                  /**83640 操作  */

    char ptpFileName[40];                   /**PTP 配置文件保存地址  */
    char netWorkFileName[40];               /**网络配置文件保存地址  */
    char netRouteFileName[40];
    int routeTable;
    
    char *pFpgaAddress;
    
    Uint8 outBlockFlag;                 /**1:封锁输出，0，不封锁  */
    
}PtpClock,*PPtpClock;




typedef struct 
{    
    char portStatus;  /**端口打开还是关闭  */
    Uint32 ip;          /** 本机IP 地址 */
    Uint32 gwip;		/** 本机网关*/
    Uint32 mask;		/** 本机子网掩码地址*/
    Uint8  mac[6];		/**本机MAC 地址 */

    
}__attribute__((packed))NetWork_Command;




typedef struct 
{
    Uint8 portEnable;              /** 1:enable,  0: disable */
    Uint8 portStatus;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** 域号 */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */

    Uint8 grandmasterPriority1;   /** 优先级 */
	Uint8 grandmasterPriority2;
    Uint8 profile;                /**  profile 配置*/

    Uint8 loalPriority;         /**本地优先级  */
    Uint8 announceRecvTimeOut;  /**announce数据超时时间  */
    
    char logSyncInterval;                /** syn发包频率 */
    char logAnnounceInterval;           
    char logMinDelayReqInterval;
    
    UInteger16 currentUtcOffset;  

    /**vlan 相关变量  */
    Uint8 vlanEnable;             /** 0:disable, 1:enable */
    Uint8 vlanPriority;           /** 优先级0~7 */
    Uint8 vlanId;                 /** ID 0~255 */
    Uint8 vlanCfi;                /** 0:标准Mac，1:非标准Mac , 默认为0*/


    Uint32 UnicastDuration;       /** 单播协商机制，老化时间 */
    
    Uint8 validServerNum;         /** 单播有效服务器数量 */
    Uint32 serverIp[10];          /** 单播服务器ip */ /** 单播ip或则mac地址 */

}__attribute__((packed))PtpParam_Command0;


/**所有ptp参数  */
typedef struct 
{    
    Uint8 portEnable;              /** 1:enable,  0: disable */
    Uint8 portStatus;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** 域号 */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */
}PtpParam_Command1;


/**profile  */
typedef struct 
{   
    Uint8 profile;                /**  profile 配置*/
    Uint8 loalPriority;           /**本地优先级  */ 

}PtpParam_Command2;


/**主要用于主时钟相关参数  */
typedef struct 
{
    Uint8 grandmasterPriority1;       /** 优先级 */
	Uint8 grandmasterPriority2;
    char logSyncInterval;            /** syn发包频率 */
    char logAnnounceInterval;           
    Uint8 announceRecvTimeOut;      /**announce数据超时时间  */
    Uint8 preserve;
    UInteger16 currentUtcOffset;
    
}PtpParam_Command3;




/**用于vlan控制  */
typedef struct 
{
    /**vlan 相关变量  */
    Uint8 vlanEnable;             /** 0:disable, 1:enable */
    Uint8 vlanPriority;           /** 优先级0~7 */
    Uint8 vlanId;                 /** ID 0~255 */
    Uint8 vlanCfi;                /** 0:标准Mac，1:非标准Mac , 默认为0*/

}PtpParam_Command4;


/**主要用于从时钟相关参数  */
typedef struct 
{
    Uint8 UniNegotiationEnable;       /** 0:disable , 1: enable */
    Uint8 logMinDelayReqInterval;
    Uint8 validServerNum;             /** 单播有效服务器数量 */
    Uint8 serverIndex;                /**服务器ip地址在ip单播多服务器数组中位置  */
    Uint32 serverIp;                  /** 单播ip或则mac地址 */
    Uint32 UnicastDuration;           /** 单播协商机制，老化时间 */

}PtpParam_Command5;



typedef struct 
{
    Uint8 portEnable[8];              /** 1:enable,  0: disable */
    Uint8 portStatus[8];              /** 0:Client    1:Server,    2: passive */

    Uint8 broadCastMode;              /** 1:enable,  0: disable */
    Uint8 multiCastMode;              /** 1:enable,  0: disable */
    Uint8 symmetricMode;              /** 1:enable,  0: disable */
    
    Uint8 Md5Enable;
    //MD5_KEY md5Key[10];            /** 局部md密钥列表 */

    

}NtpParam_Command0;

typedef struct 
{   
    Uint8 ethx;
    Uint8 portEnable;              /** 1:enable,  0: disable */

}NtpParam_Command1;


typedef struct 
{
    Uint8 Md5Enable;
}NtpParam_Command2;


typedef struct 
{
    Uint8 keyIndex;            /**ntp md5 修改索引  */
    //MD5_KEY md5Key;            /** 局部md密钥列表 */
}NtpParam_Command3;


/**控制盘内时钟等级  */
typedef struct 
{
    Enumeration8 timeSource;
    UInteger8 clockClass;
    Enumeration8 clockAccuracy;

}ClockParam_Command0;



typedef struct 
{
    Uint8 grandmasterPriority1;   /** 优先级 */
	Uint8 grandmasterPriority2;
    
    Uint8 timeSource;
    Uint8 leapFlag;             /**  */
    UInteger16 currentUtcOffset;
    ClockQuality grandmasterClockQuality;
    ClockIdentity grandmasterIdentity;      
    PortIdentity sourcePortIdentity;
    Uint32 BestMasterIp;
    Uint8 BestMasterMac[6];
    Uint8 stepsRemoved;

}SlaveUp_Command0;


typedef struct 
{
    TimeInternal Offset;

}SlaveUp_Command1;


/**报警  */
typedef struct 
{
    char alarmByte_1;
    char alarmByte_2;
    char alarmByte_3;
    char alarmByte_4;
}Alarm_Command;


/**版本  */
typedef struct 
{
    char CpuVersion[4];
    char FpgaVersion[4];
    char PcbVersion[4];
}Version_Command;



typedef struct
{
    Uint8 slot_1_RcuAlarm;          /**1号槽位rcu告警1，告警，0，不告警  */
    Uint8 slot_2_RcuAlarm;          /**2号槽位rcu告警1，告警，0，不告警  */
    Uint8 slot_1_Master_Backup;     /**1号槽位主备信息，0,主用 1，备用  */
    Uint8 slot_2_Master_Backup;     /**2号槽位主备信息，0,主用 1，备用  */
    Uint8 slot_1_100Alarm;          /**1号槽位100m 检波告警 ，1，告警，0，不告警*/
    Uint8 slot_2_100Alarm;          /**2号槽位100m 检波告警，1，告警，0，不告警*/
    Uint8 slot_1_1ppsAlarm;         /**1号槽位1pps检波告警，1，告警，0，不告警  */
    Uint8 slot_2_1ppsAlarm;         /**2号槽位1pps检波告警，1，告警，0，不告警 */
    Uint8 slot_1_TodAlarm;          /**1号槽位tod检波告警，1，告警，0，不告警  */
    Uint8 slot_2_TodAlarm;          /**2号槽位tod检波告警，1，告警，0，不告警  */
    Uint8 master100Alarm;           /**主用100m 告警  */
    Uint8 master1ppsTodAlarm;       /**主用1pps 告警  */
    Uint8 twoMasterAlarm;           /**两路都是主用告警  */
    Uint8 twoBackupAlarm;           /**两路都是备用告警  */
    Uint8 slot_1_TodCrcAlarm;       /**1号槽位tod crc告警  1，告警，0，不告警*/
    Uint8 slot_2_TodCrcAlarm;       /**2号槽位tod crc告警  1，告警，0，不告警*/
    
}PadAlarm;

typedef struct 
{
    struct ifreq ifr;
    short i2c_link_status;   /**0,down 1,up  */
    short media_type;        /**0,copper,1,Fiber  */
    short speed;             /**0,100M,1,1000M  */
}SfpPort;


typedef struct
{
    int sockfd;
    int i2cfd;
    SfpPort sfp_port[PTP_PORT_COUNT];
    
}SfpCtl;

typedef struct 
{
    
    Uint32 sec;
    Uint8 leapSec;          /**闰秒偏差  */
    Uint8 leapFlag;         /**闰秒标记  */
    Uint8 timeSource;       /**时间源  */
    Uint8 freqSource;       /**频率源  */
    Uint8 oscType;          /**钟类型  */
    Uint8 oscState;         /**钟状态  */
    
    Uint8 clockClass;       /**时钟等级  */
    Uint8 ptpClockAccuracy;    /**PTP时钟精度  */
    Uint8 dclsClockAccuracy;    /**Dcls时钟精度  */
    Uint8 ntpClockAccuracy;    /**Ntp时钟精度  */
    Uint8 ppsTodClockAccuracy;    /**ppsTod时钟精度  */
    
    Uint8 preserve;      /**保留  */
}BackBusData;


typedef struct 
{
    int comfd;                  /**通信串口文件描述符  */
    int slot_num;               /**槽位号  */
    int dest_num;               /** 目的槽位 */

    int dev_fd;
    Version_Command version;    /**版本消息  */
    PadAlarm padAlarm;          /**单盘告警信息  */
    Uint8 diskAlarm;            /** 单盘告警 */
    Uint8 alarmOutput;
    
    //Uint32 interval_time;              /**记录本地时间  */

    BackBusData backBusParam;
   
    Uint8 pps_routine_flag;
    Uint8 powerOn;
    
    PtpClock *pPtpClock;
    SfpCtl   sfpCtl;      /**sfp 操作  */
    
    Alarm_Command alarmCommand;
    
}UartDevice;



typedef struct 
{
    Uint8 portEnable;              /** 1:enable,  0: disable */
    Uint8 clockType;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** 域号 */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */

    /**vlan 相关变量  */
    Uint8 vlanEnable;             /** 0:disable, 1:enable */
    Uint8 vlanPriority;           /** 优先级0~7 */
    Uint8 vlanId;                 /** ID 0~255 */
    Uint8 vlanCfi;                /** 0:标准Mac，1:非标准Mac , 默认为0*/

    Uint8 validServerNum;              /** 有效的服务器ip数量 */
    Uint32 UnicastDuration;       /** 单播协商机制，老化时间 */


    /**人工输入的发包频率 */
    Uint8 synFreq;                
    Uint8 announceFreq;           
    Uint8 delayreqFreq;
    Uint8 signalFreq;
}CommPtpPararmeter;

typedef struct 
{
	ClockIdentity grandmasterIdentity;
	ClockQuality grandmasterClockQuality;
	UInteger8 grandmasterPriority1;
	UInteger8 grandmasterPriority2;

    /**最佳主时钟  */
    Uint32 BestMasterIp;
    Uint8 BestMasterMac[6];

    UInteger16 currentUtcOffset;
    Enumeration8 timeSource;

    TimeInternal T1;
    TimeInternal T2;
    TimeInternal T3;
    TimeInternal T4;
    
    TimeInternal MeanPathDelay;
    TimeInternal TimeOffset;

    Slonglong64 timeAdjustValue;
    
}CommPtpStatus;





#endif
