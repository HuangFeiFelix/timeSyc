#ifndef __MANAGER_H__
#define __MANAGER_H__

#include "main.h"

struct Head_Frame
{
    char h1;
    char h2;
    char saddr;
    char daddr;
    short index;
    char ctype;
    char pad_type;
    int length;
};


struct Discovery_Frame
{
    char h1;
    char h2;
    char saddr;
    char daddr;
    short index;
    char ctype;
    char end1;
    char end2;
};

struct Response_Frame
{
    int ipaddr;
    int port;
};


struct CmdNetwork
{
    int ipaddress;
    int mask;
    int gateway;
    char mac[6];
};

struct CmdSysSet
{
    char ref_type;
    char log_level;
    char reserv1;
    char reserv2;
};

struct CmdPtpAll
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
    Integer8 logMinPdelayReqInterval;
    Integer8 logMinDelayReqInterval;
   
    UInteger8 grandmasterPriority1;
    UInteger8 grandmasterPriority2;
    Uint8 validServerNum;              /** 有效的服务器ip数量 */
    Uint8 currentUtcOffset;
    struct ServerList serverList[10];

};



struct CmdNtpNormal
{

    Uint8     broadcast;                    // flag: 0 stop 1 run
	Uint8     freq_b;                   // broadcast send period   ntpd do not have
	Uint8     multicast;    
	Uint8     freq_m;                   // multicast send period   ntpd do not have
    Uint8     md5_flag;			//the flag about md5 authentication   bit1--1=Enable md5;0=Disable md5 ;    bit2--0= broadcast Closed;1=setted key number;  bit3--0= multicast Closed;1=setted key number;
    Uint8     sympassive;                   // flag
	struct Md5key     current_key[10];    //the space to store md5 key structure

};

struct CmdNtpMd5Enable
{
    Uint8 broadcast_key_num;
    Uint8 multicast_key_num;
    Uint8 md5_flag;
    Uint8 sympassive;
};

struct CmdNtpMd5Key
{
    Uint8 key_index;
    
    Uint8 multicast_key_num;
    Uint8 md5_flag;
    Uint8 sympassive;
    struct Md5key     current_key;
    
};

struct CmdNtpBlacklist
{
    Uint8     blacklist;                    // flag
    Uint8     blacklist_flag[16];           //黑名单标识列表
    Uint32    blacklist_ip[16];             //黑名单ip地址列表
    Uint32    blacklist_mask[16];           // 白名单子网掩码列表
};

struct CmdNtpWhitelist
{
    Uint8     whitelist;                    // flag
    Uint8     whitelist_flag[16];           //白名单标识列表
    Uint32    whitelist_ip[16];             //黑名单ip地址列表
    Uint32    whitelist_mask[16];           // 白名单子网掩码列表
};

extern void handle_discovery_message(struct root_data *pRootData,char *buf,int len);


extern void inssu_pps_data(struct root_data *pRootData);

#endif
