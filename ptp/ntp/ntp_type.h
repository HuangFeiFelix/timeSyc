#ifndef __NTP_TYPE_H__
#define __NTP_TYPE_H__

#include "common.h"

#define NTP_CONFIG_FILE "/mnt/ntp/ntp.conf"
#define NTP_MD5_FILE    "/mnt/ntp/keys"

#define GET_SECOND 1		//return second_from1900
#define SET_SECOND 2		//set second_from1900
#define MODIFY_TIME     23   //add by hf 2014-9-25
#define GET_US_SECOND   24   //add by hf get sys us

#define GET_ALARM 14            //check alarm
#define GET_TIMER_SECOND 15     //check timer second
#define GET_VERSION 16   	    //check cpld version
#define WRITE_LED_REG 17        //write led reg

typedef struct 
{
	Uint32 ip;          //IP address;ip ,gwip and mask is network sequence
	Uint32 gwip;		//gateway address
	Uint32 mask;		//network address mask
	Uint8  mac[6];		//MAC address
}NET_INFOR,*PNET_INFOR;


typedef struct
{
	Uint8 key_valid;		//the md5 key is valid or not:1 valid ; 2 invalid
	Uint8 key_length;		//the length of the md5 key
	Uint8 key[20];		    //the md5 key in ASCII 
	
}MD5_KEY,*PMD5_KEY;

typedef struct
{
	Uint8       net_flag;			//the flag about network parameter
	Uint8       md5_flag;			//the flag about md5 authentication   bit1--1=Enable md5;0=Disable md5 ;    bit2--0= broadcast Closed;1=setted key number;  bit3--0= multicast Closed;1=setted key number;
	MD5_KEY     current_key[10];    //the space to store md5 key structure
	NET_INFOR   runconf;         	//the network parameter structure
	
}NTP_ENVIRONMENT,*PNTP_ENVIRONMENT;



typedef struct
{
    unsigned char stratum;   //时钟源等级
    unsigned char leap;      //闰秒
    signed char precision;   //精度
    unsigned char refid[5];  //ID
}NTPD_STATUS,*PNTPD_STATUS;


typedef struct
{
	Uint32    freq_b;                   // broadcast send period   ntpd do not have
	Uint32    freq_m;                   // multicast send period   ntpd do not have
    Uint8     broadcast;                    // flag: 0 stop 1 run
    Uint8     broadcast_key_num;
	Uint8     multicast;                    // flag
    Uint8     multicast_key_num;
    Uint8     sympassive;                   // flag
	Uint8     blacklist;                    // flag
	Uint8     whitelist;                    // flag
	Uint8     blacklist_flag[16];           //黑名单标识列表
	Uint8     whitelist_flag[16];           //白名单标识列表
	Uint32    blacklist_ip[16];             //黑名单ip地址列表
	Uint32    blacklist_mask[16];	        //黑名单子网掩码列表
	Uint32    whitelist_ip[16];             // 白名单ip地址列表
	Uint32    whitelist_mask[16];	        // 白名单子网掩码列表

}NTP_PARAMETER,*PNTP_PARAMETER;

typedef struct 
{
    NTP_PARAMETER *ntpParameter;
    NTP_ENVIRONMENT *ntpEnviroment;
    NTPD_STATUS     *ntpStatus;
}NtpDevice;


#define TOTAL_KEY_NO  8
#define ENTER_CHAR 0x0a

#define	LEAP_NOWARNING	0x0	/* normal, no leap second warning */
#define	LEAP_ADDSECOND	0x1	/* last minute of day has 61 seconds */
#define	LEAP_DELSECOND	0x2	/* last minute of day has 59 seconds */
#define	LEAP_NOTINSYNC	0x3	/* overload, clock is free running */

#define STRATUM_0_PRESION -20
#define STRATUM_1_PRESION -20
#define STRATUM_2_PRESION -18
#define STRATUM_3_PRESION -15



#endif