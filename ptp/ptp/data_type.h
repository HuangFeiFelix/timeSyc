/********************************************************************************************
*                           ��Ȩ����(C) 2015, ���ſ�ѧ���������о���
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       data_type.h
*    ��������:       ����ptp����ṹ�壬�궨�壬Ĭ������
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-4-27
*    �����б�:
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-4-27
                     ����: HuangFei
                     �޸�����: �´����ļ�


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

/** ʱ������ */
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

/**��Ϣ�ڲ� Э������ֵ  */
enum {
	UDP_IPV4=1,
    UDP_IPV6,
    IEEE_802_3,
    DeviceNet,
    ControlNet,
    PROFINET
};

/**����ģʽ  */
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

/** ���ͽ���buffer ���� */
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


/**��1558Э��Page55 ��5  */
#define DEFAULT_CLOCK_CLASS				248
#define LOCK_CLOCK_CLASS                6
#define UNLOCK_CLOCK_CLASS              7
#define USENSS_CLOCK_CLASS              52


/**��1558Э��Page56 ��6  */
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


/** �������ݰ����� */
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


/**head �� ��Ϣ���Ͷ���  */
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

/**�汾����  */
#define PTP_VERSION_1   1
#define PTP_VERRION_2   2

/** �����ֶ�����Ϣ���Ͷ��� */
#define COTROL_TYPE_SYN         0x00
#define COTROL_TYPE_DELAY_REQ   0x01
#define COTROL_TYPE_FOLLOWUP    0x02
#define COTROL_TYPE_DELAY_RESP  0x03
#define COTROL_TYPE_MANAGEMENT  0x04
#define COTROL_TYPE_ALL_OHTERv  0x05

#define DEFAULT_LOGMSG_INTERVAL 0x7F

#define IFACE_NAME_LENGTH         IF_NAMESIZE
#define NET_ADDRESS_LENGTH        INET_ADDRSTRLEN

/**sinal ��Ϣ tlvType   */
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

/**�����߳���1s ���ִ��  */
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
    UInteger16 checkRecvAlarm_Counter20s;   /**�����հ��澯����  */
    UInteger16 displayStauts_Counter10s;    /**��ӡ��Ϣ���ڼ���  */
    UInteger16 sendControl_index;           /**����signal ����  */
    UInteger16 signalDuration_index;        /**signal ��Ϣ����ʱ�����  */
    
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
    char ifaceName[5];  /** �������ں� */
    
    Uint32 ip;          /** ����IP ��ַ */
    Uint32 gwip;		/** ��������*/
    Uint32 mask;		/** �������������ַ*/
    Uint8  mac[6];		/**����MAC ��ַ */
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
	Uint8 valid;		 				  /** client��Ч��ǣ�1:��Ч��0:��Ч */
    
	Uint8 send_sync_enable;	              /** ����Э�̹�:1������Э�̿�grand֮��Ϊ1 */
	Uint8 send_announce_enable;	          /**  ����Э�̹�:1,    ����Э�̿�grand ֮��Ϊ1*/
	Uint32 client_ip;					  /**client ip��ַ*/
	Uint8 client_mac[6];				  /**client mac��ַ*/
	int    client_counter;                /**����client��������*/
	Uint32 durationField;                 /**�ϻ�ʱ��,��λ��*/
	
}ClienList;                              /**����ģʽ�£�server�յ���cllient������*/

typedef struct
{
    ClienList clientList[MAX_CLIENT];     /** �ͻ����б� */
    Uint16 syn_index;                     /**syn ���͵�λ��ƫ��  */
    Uint16 announce_index;                /**announce ���͵�λ��ƫ��  */
}UnicastClient;

typedef struct
{
    Uint8 currentUtcOffset;     //��ǰutc ƫ����
    Uint8 priority1;            //ʱ�����ȼ�1 
    Uint8 priority2;            //ʱ�����ȼ�2
    
    PortIdentity portIdentity;  //ʱ��ID
    
    ClockQuality clockQuality;  // ʱ������
    Uint8 grandmastIdentity[8];
    Uint8 setpRemoved;          //·������
    Uint8 timeSource;           //ʱ��Դ
}AnnounceParam;


typedef struct
{
    Uint32 serverIp;            /** ����������ip */
    char  serverMac[6];         /** ����������mac */
    Uint8 valid;                /**  ip �Ƿ���Ч*/
    Uint8 recv_sigsyn_flag;     /** 1: �յ�  0: δ�յ� */
    Uint8 recv_sigann_flag;     /** 1: �յ�  0: δ�յ� */
    Uint8 recv_sigdela_flag;    /** 1: �յ�  0: δ�յ� */
    Uint8 sig_alarm;            /** �澯1�� ���澯0 */

    Uint8 send_delayreq_enable;  /**1,���Է��ͣ�0�����ܷ���  */
    Uint8 finishT1T2_flag;      /**  1 :��� 0 : �����*/
    
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
    ServerList serverList[MAX_SERVER];   /** ������ip��ַ����mac��ַ�б� */
    Uint16 index;                       /** �����б�λ�� */
    Uint16 delay_index;                 /** ���ڿ��� delayreq���� */  
    Uint8 validServerNum;              /** ��Ч�ķ�����ip���� */               
}UnicastMultiServer;

typedef struct 
{
    Uint8 line_type;                /**��·���ͣ�ֱ��:0������������:1 */
    Uint8 ctl_phase_type;           /**�����������0:΢����1:���ٵ���  */

    short over2sec_counter;         /**����2s������  */
    short over1us_counter;          /**����1us������  */
    short low1us_counter;           /**С��1us������  */
    short over5us_cnt;              /**΢���жϳ���5us������  */
    short low5us_cnt;               /**΢���ж�С��5us ������  */
    short in_fastmodify_cnt;        /**���ڿ첶״̬����  */
    
    Slonglong64 mean_path;          /**��·��ʱ  */
    Slonglong64 time_offset;        /**ʱ��ƫ��  */ 

    Slonglong64 time_offset_buf[10];/** ʱ��ƫ���buffer  */
    Slonglong64 phase_adjsut;       /**��λ����ֵ  */
    Slonglong64 averag_tmp;         /**ƽ����·ʱ��  */


    short cnt_threshold;          /**�������ޣ�10 x logsynInterval */
    short ratediff_cnt;           /**���ڲ���ratediff ���� */

    Uint8 diff_index;                   /**Ƶ��������  */
    Slonglong64 diff_buf[5];            /**Ƶƫ����  */
    Slonglong64 calman_diff_buf[5];     /**���������˲��󻺴�  */
    
    TimeInternal lastT1;                /**��һ��T1 ֵ  */
    TimeInternal lastT2;                /** ��һ��T2 ֵ */

    Slonglong64 RateDiff;               /**RateDiff ����ֵ  */
    Slonglong64 OverMaxRateCnt;         /**����rateDiff���޼���  */
    
}PhyControl;

typedef struct 
{
    Uint8 portEnable;              /** 1:enable,  0: disable */
    Uint8 clockType;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** ��� */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */
    Uint8 profile;                /**  profile ����*/
    
    Uint8 versionNumber;          /** �汾 */
    Uint32 UnicastDuration;       /** ����Э�̻��ƣ��ϻ�ʱ�� */
    LOG_DEBUG debugLevel;             /**debug ��Ϣ   */

    /**vlan ��ر���  */
    Uint8 vlanEnable;             /** 0:disable, 1:enable */
    Uint8 vlanPriority;           /** ���ȼ�0~7 */
    Uint8 vlanId;                 /** ID 0~255 */
    Uint8 vlanCfi;                /** 0:��׼Mac��1:�Ǳ�׼Mac , Ĭ��Ϊ0*/
    

    /**��Ź��˿���  */
    Uint8 domainFilterSwitch;            /** ��Ź��˿���0:�أ�1:�� */
    
    /** announce ��Ϣ��ʱ�ӵ�ʱ�򸳸�ֵ*/
    /** ��ʱ�ӵ�ʱ���ȡ�����ʱ��ֵ */
    TimePropertiesDS timePropertiesDS;
    UInteger16 stepsRemoved;


    Uint8 initDp83640Sucess;    /**��ʼ��83640 ���1:���   0: δ���  */
    
    /**�����ʱ��  */
	PortIdentity parentPortIdentity;
	ClockIdentity grandmasterIdentity;
	ClockQuality grandmasterClockQuality;
	UInteger8 grandmasterPriority1;
	UInteger8 grandmasterPriority2;

    /**�����ʱ�ӣ�����BMC ����  */
    ForeignMasterRecord BestMasterRecord;
    Uint8 InitBmcflag;     
    Uint32 BestMasterIp;
    Uint8 BestMasterMac[6];


    /** head �еķ���Ƶ�� */
    Integer8 logSyncInterval;
    Integer8 logAnnounceInterval;
    Integer8 logMinPdelayReqInterval;
    Integer8 logMinDelayReqInterval;

    
    /**�������ڻ���  */
    NetInfor netEnviroment;

    /**����ProtIdentity  */
    PortIdentity portIdentity;  

    /**���ͽ��ջ��� */
    NetComm  netComm;


    /**������������  */
    Uint16 synSendInterval;
    Uint16 announceSendInterval;
    Uint16 delayreqInterval;
    Uint16 signalSendInterval;


    /**���͵�sequneceId  */
    Uint16 sentSyncSequenceId;              
    Uint16 sentAnnounceSequenceId;
    Uint16 sentDelayReqSequenceId;
    Uint16 sentPDelayReqSequenceId;
    Uint16 sentSignalSequenceId;

    /**�յ���sequenceId  */
    Uint16 recvSynReqSequenceId;
    Uint16 recvDelayReqSequenceId;
    Uint16 recvPDelayReqSequenceId;


    /**���յ���Ϣ��־  */
    Uint8 recvSynFlag;
    Uint8 recvAnnounceFlag;
    Uint8 recvDelayRespFlag;

    /** ����  */
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

    //Uint8 finishT3T4;                       /**���T3 T4����  */
    Uint8 finishPtp;                        /**���һ�������ռ������ڴ��͸�UI  */

    TimeInternal CurentTime;                /**ʵʱʱ��  */
    Uint16 oneSecondTime;                   /**���ڷ����߳���1s����  */
    Uint8  netAccessSpeed;                  /** ��������0:10 Base-T,   1: 100 Base-T */

    UnicastMultiServer unicastMultiServer;  /**��������������ƣ���ʱ��ʹ��  */
    UnicastClient      unicastClient;       /** �ͻ��˿��ƣ���ʱ��ʹ�� */

    
    char msgObuf[PACKET_SIZE];              /**���ͻ���  */
    char msgIbuf[PACKET_SIZE];              /** ���ջ��� */

    
    PtpdCounters counters;                  /**������  */


    PhyControl phyControl;                  /**83640 ����  */

    char ptpFileName[40];                   /**PTP �����ļ������ַ  */
    char netWorkFileName[40];               /**���������ļ������ַ  */
    char netRouteFileName[40];
    int routeTable;
    
    char *pFpgaAddress;
    
    Uint8 outBlockFlag;                 /**1:���������0��������  */
    
}PtpClock,*PPtpClock;




typedef struct 
{    
    char portStatus;  /**�˿ڴ򿪻��ǹر�  */
    Uint32 ip;          /** ����IP ��ַ */
    Uint32 gwip;		/** ��������*/
    Uint32 mask;		/** �������������ַ*/
    Uint8  mac[6];		/**����MAC ��ַ */

    
}__attribute__((packed))NetWork_Command;




typedef struct 
{
    Uint8 portEnable;              /** 1:enable,  0: disable */
    Uint8 portStatus;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** ��� */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */

    Uint8 grandmasterPriority1;   /** ���ȼ� */
	Uint8 grandmasterPriority2;
    Uint8 profile;                /**  profile ����*/

    Uint8 loalPriority;         /**�������ȼ�  */
    Uint8 announceRecvTimeOut;  /**announce���ݳ�ʱʱ��  */
    
    char logSyncInterval;                /** syn����Ƶ�� */
    char logAnnounceInterval;           
    char logMinDelayReqInterval;
    
    UInteger16 currentUtcOffset;  

    /**vlan ��ر���  */
    Uint8 vlanEnable;             /** 0:disable, 1:enable */
    Uint8 vlanPriority;           /** ���ȼ�0~7 */
    Uint8 vlanId;                 /** ID 0~255 */
    Uint8 vlanCfi;                /** 0:��׼Mac��1:�Ǳ�׼Mac , Ĭ��Ϊ0*/


    Uint32 UnicastDuration;       /** ����Э�̻��ƣ��ϻ�ʱ�� */
    
    Uint8 validServerNum;         /** ������Ч���������� */
    Uint32 serverIp[10];          /** ����������ip */ /** ����ip����mac��ַ */

}__attribute__((packed))PtpParam_Command0;


/**����ptp����  */
typedef struct 
{    
    Uint8 portEnable;              /** 1:enable,  0: disable */
    Uint8 portStatus;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** ��� */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */
}PtpParam_Command1;


/**profile  */
typedef struct 
{   
    Uint8 profile;                /**  profile ����*/
    Uint8 loalPriority;           /**�������ȼ�  */ 

}PtpParam_Command2;


/**��Ҫ������ʱ����ز���  */
typedef struct 
{
    Uint8 grandmasterPriority1;       /** ���ȼ� */
	Uint8 grandmasterPriority2;
    char logSyncInterval;            /** syn����Ƶ�� */
    char logAnnounceInterval;           
    Uint8 announceRecvTimeOut;      /**announce���ݳ�ʱʱ��  */
    Uint8 preserve;
    UInteger16 currentUtcOffset;
    
}PtpParam_Command3;




/**����vlan����  */
typedef struct 
{
    /**vlan ��ر���  */
    Uint8 vlanEnable;             /** 0:disable, 1:enable */
    Uint8 vlanPriority;           /** ���ȼ�0~7 */
    Uint8 vlanId;                 /** ID 0~255 */
    Uint8 vlanCfi;                /** 0:��׼Mac��1:�Ǳ�׼Mac , Ĭ��Ϊ0*/

}PtpParam_Command4;


/**��Ҫ���ڴ�ʱ����ز���  */
typedef struct 
{
    Uint8 UniNegotiationEnable;       /** 0:disable , 1: enable */
    Uint8 logMinDelayReqInterval;
    Uint8 validServerNum;             /** ������Ч���������� */
    Uint8 serverIndex;                /**������ip��ַ��ip�����������������λ��  */
    Uint32 serverIp;                  /** ����ip����mac��ַ */
    Uint32 UnicastDuration;           /** ����Э�̻��ƣ��ϻ�ʱ�� */

}PtpParam_Command5;



typedef struct 
{
    Uint8 portEnable[8];              /** 1:enable,  0: disable */
    Uint8 portStatus[8];              /** 0:Client    1:Server,    2: passive */

    Uint8 broadCastMode;              /** 1:enable,  0: disable */
    Uint8 multiCastMode;              /** 1:enable,  0: disable */
    Uint8 symmetricMode;              /** 1:enable,  0: disable */
    
    Uint8 Md5Enable;
    //MD5_KEY md5Key[10];            /** �ֲ�md��Կ�б� */

    

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
    Uint8 keyIndex;            /**ntp md5 �޸�����  */
    //MD5_KEY md5Key;            /** �ֲ�md��Կ�б� */
}NtpParam_Command3;


/**��������ʱ�ӵȼ�  */
typedef struct 
{
    Enumeration8 timeSource;
    UInteger8 clockClass;
    Enumeration8 clockAccuracy;

}ClockParam_Command0;



typedef struct 
{
    Uint8 grandmasterPriority1;   /** ���ȼ� */
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


/**����  */
typedef struct 
{
    char alarmByte_1;
    char alarmByte_2;
    char alarmByte_3;
    char alarmByte_4;
}Alarm_Command;


/**�汾  */
typedef struct 
{
    char CpuVersion[4];
    char FpgaVersion[4];
    char PcbVersion[4];
}Version_Command;



typedef struct
{
    Uint8 slot_1_RcuAlarm;          /**1�Ų�λrcu�澯1���澯��0�����澯  */
    Uint8 slot_2_RcuAlarm;          /**2�Ų�λrcu�澯1���澯��0�����澯  */
    Uint8 slot_1_Master_Backup;     /**1�Ų�λ������Ϣ��0,���� 1������  */
    Uint8 slot_2_Master_Backup;     /**2�Ų�λ������Ϣ��0,���� 1������  */
    Uint8 slot_1_100Alarm;          /**1�Ų�λ100m �첨�澯 ��1���澯��0�����澯*/
    Uint8 slot_2_100Alarm;          /**2�Ų�λ100m �첨�澯��1���澯��0�����澯*/
    Uint8 slot_1_1ppsAlarm;         /**1�Ų�λ1pps�첨�澯��1���澯��0�����澯  */
    Uint8 slot_2_1ppsAlarm;         /**2�Ų�λ1pps�첨�澯��1���澯��0�����澯 */
    Uint8 slot_1_TodAlarm;          /**1�Ų�λtod�첨�澯��1���澯��0�����澯  */
    Uint8 slot_2_TodAlarm;          /**2�Ų�λtod�첨�澯��1���澯��0�����澯  */
    Uint8 master100Alarm;           /**����100m �澯  */
    Uint8 master1ppsTodAlarm;       /**����1pps �澯  */
    Uint8 twoMasterAlarm;           /**��·�������ø澯  */
    Uint8 twoBackupAlarm;           /**��·���Ǳ��ø澯  */
    Uint8 slot_1_TodCrcAlarm;       /**1�Ų�λtod crc�澯  1���澯��0�����澯*/
    Uint8 slot_2_TodCrcAlarm;       /**2�Ų�λtod crc�澯  1���澯��0�����澯*/
    
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
    Uint8 leapSec;          /**����ƫ��  */
    Uint8 leapFlag;         /**������  */
    Uint8 timeSource;       /**ʱ��Դ  */
    Uint8 freqSource;       /**Ƶ��Դ  */
    Uint8 oscType;          /**������  */
    Uint8 oscState;         /**��״̬  */
    
    Uint8 clockClass;       /**ʱ�ӵȼ�  */
    Uint8 ptpClockAccuracy;    /**PTPʱ�Ӿ���  */
    Uint8 dclsClockAccuracy;    /**Dclsʱ�Ӿ���  */
    Uint8 ntpClockAccuracy;    /**Ntpʱ�Ӿ���  */
    Uint8 ppsTodClockAccuracy;    /**ppsTodʱ�Ӿ���  */
    
    Uint8 preserve;      /**����  */
}BackBusData;


typedef struct 
{
    int comfd;                  /**ͨ�Ŵ����ļ�������  */
    int slot_num;               /**��λ��  */
    int dest_num;               /** Ŀ�Ĳ�λ */

    int dev_fd;
    Version_Command version;    /**�汾��Ϣ  */
    PadAlarm padAlarm;          /**���̸澯��Ϣ  */
    Uint8 diskAlarm;            /** ���̸澯 */
    Uint8 alarmOutput;
    
    //Uint32 interval_time;              /**��¼����ʱ��  */

    BackBusData backBusParam;
   
    Uint8 pps_routine_flag;
    Uint8 powerOn;
    
    PtpClock *pPtpClock;
    SfpCtl   sfpCtl;      /**sfp ����  */
    
    Alarm_Command alarmCommand;
    
}UartDevice;



typedef struct 
{
    Uint8 portEnable;              /** 1:enable,  0: disable */
    Uint8 clockType;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** ��� */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */

    /**vlan ��ر���  */
    Uint8 vlanEnable;             /** 0:disable, 1:enable */
    Uint8 vlanPriority;           /** ���ȼ�0~7 */
    Uint8 vlanId;                 /** ID 0~255 */
    Uint8 vlanCfi;                /** 0:��׼Mac��1:�Ǳ�׼Mac , Ĭ��Ϊ0*/

    Uint8 validServerNum;              /** ��Ч�ķ�����ip���� */
    Uint32 UnicastDuration;       /** ����Э�̻��ƣ��ϻ�ʱ�� */


    /**�˹�����ķ���Ƶ�� */
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

    /**�����ʱ��  */
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
