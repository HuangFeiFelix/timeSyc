#ifndef __MAIN_H__
#define __MAIN_H__

#include "common.h"
#include "list_ctrl.h"
#include "satellite_api.h"
#include "clock_rb.h"

#define SLOT_COUNT 6
#define SLOT_TYPE_CORE 'A'
#define SLOT_TYPE_PTP  'B'
#define SLOT_TYPE_NTP  'C'

enum
{
    ENUM_SLOT_COR_ADDR = 0x00,
    ENUM_SLOT_1_ADDR,
    ENUM_SLOT_2_ADDR,
    ENUM_SLOT_3_ADDR,
    ENUM_SLOT_4_ADDR,
    ENUM_SLOT_5_ADDR,
    ENUM_SLOT_PC_ADDR = 0xff
};



struct ServerList
{
    Uint32 serverIp;            /** ����������ip */
    char  serverMac[6];         /** ����������mac */    
};


struct PtpSetCfg
{
    Uint8 clockType;               /** 0:slave    1:master,    2: passive */
    Uint8 domainNumber;            /** ��� */
    Uint8 protoType;               /**  0:IEEE802.3,   1:UDP/IP      */
    Uint8 modeType;               /** 0: multicast,     1:unicast */
    Uint8 transmitDelayType;      /** 0:p2P,      1,E2E */
    Uint8 stepType;               /**  0: one step,     1:two step*/
    Uint8 UniNegotiationEnable;   /** 0:disable , 1: enable */
    Uint8 domainFilterSwitch;            /** ��Ź��˿���0:�أ�1:�� */
    Uint32 UnicastDuration;       /** ����Э�̻��ƣ��ϻ�ʱ�� */
    Integer8 logSyncInterval;
    Integer8 logAnnounceInterval;
    Integer8 logMinPdelayReqInterval;
    Integer8 logMinDelayReqInterval;
   
    UInteger8 grandmasterPriority1;
    UInteger8 grandmasterPriority2;
    Uint8 validServerNum;              /** ��Ч�ķ�����ip���� */
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
    unsigned char stratum;   //ʱ��Դ�ȼ�
    unsigned char leap;      //����
    signed char precision;   //����
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
	Uint8     blacklist_flag[16];           //��������ʶ�б�
	Uint8     whitelist_flag[16];           //��������ʶ�б�
	Uint32    blacklist_ip[16];             //������ip��ַ�б�
	Uint32    blacklist_mask[16];	        //���������������б�
	Uint32    whitelist_ip[16];             // ������ip��ַ�б�
	Uint32    whitelist_mask[16];	        // ���������������б�

};

struct SlotList
{
    char slot_type;     /**������  */
    struct NtpSetCfg *pNtpSetCfg;
    struct PtpSetCfg *pPtpSetcfg;
    struct PtpStatusCfg *pPtpStatusCfg; 
};


struct root_data{

		struct dev_head dev_head;	//�豸��Ϣͷ
        struct device dev[6];		//ʹ���豸��Ϣ
        int dev_fd;
        
        struct SlotList slot_list[6];
        
        char Version[4];

        /*�ӿ��㷨*/
        struct clock_info clock_info;    
              
        /*��������*/
        struct Satellite_Data satellite_data;  

        FILE *file_fd;

        Uint8 flag_usuallyRoutine;
        char comm_iface[10];                 //�ӿ�����
        int  comm_port;

        
		pthread_attr_t pattr;	//�߳�����
		pthread_t p_usual;		//�ճ�������
		pthread_t p_recv;		//���ݽ����߳�
		pthread_t p_send;		//���ݷ����߳�
		pthread_t p_handle;		//���ݴ����߳�
};

extern struct root_data *g_RootData;

#endif
