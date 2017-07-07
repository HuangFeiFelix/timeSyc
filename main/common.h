/********************************************************************************************
*                           版权所有(C) 2015, 
*                                 版权所有
*********************************************************************************************
*    文 件 名:       common.h
*    功能描述:        公用头文件，定义引用头文件，定义变量类型
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-30
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-30
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/


#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <ctype.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <signal.h>
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <net/route.h>

#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#include <asm/types.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>


#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <ctype.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <signal.h>
#include <time.h>

#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <netinet/in.h>
#include <net/if_arp.h>
#include <net/route.h>

#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#include <asm/types.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/mii.h>
#include <math.h>

//#pragma pack(2)

typedef char            Int8;
typedef short           Int16;
typedef int	            Int32;
typedef long		    Long32;
typedef float		    Float32;
typedef double          Double64;

typedef signed char 	Sint8;
typedef signed short	Sint16;
typedef signed int		Sint32;
typedef signed long		Slong32;
typedef unsigned char	Uint8;
typedef unsigned short	Uint16;
typedef unsigned int	Uint32;
typedef unsigned long	Ulong32;

typedef signed long long   Slonglong64;
typedef unsigned long long Ulonglong64;



typedef enum {FALSE=0, TRUE} Boolean;

typedef unsigned char Octet;
typedef char    Integer8;
typedef short   Integer16;
typedef int     Integer32;
typedef unsigned char  UInteger8;
typedef unsigned short UInteger16;
typedef unsigned int   UInteger32;
typedef unsigned short Enumeration16;

typedef unsigned char Enumeration8;
typedef unsigned char Enumeration4;
typedef unsigned char Enumeration4Upper;
typedef unsigned char Enumeration4Lower;
typedef unsigned char UInteger4;
typedef unsigned char UInteger4Upper;
typedef unsigned char UInteger4Lower;
typedef unsigned char Nibble;
typedef unsigned char NibbleUpper;
typedef unsigned char NibbleLower;


typedef unsigned int u_int32;
typedef int int32;
typedef unsigned char u_char;
typedef signed char s_char;
typedef unsigned long u_long;
typedef signed long s_long;
typedef unsigned short u_short;
typedef signed short s_short;


/**
* \brief Implementation specific of UInteger48 type
 */
typedef struct {
    uint16_t msb;
    uint32_t lsb;
} UInteger48;

/**
* \brief Implementation specific of Integer64 type
 */
typedef struct {
	uint32_t lsb;
	int32_t msb;
} Integer64;


struct NetParam
{
    char ifaceName[5];  /** 所在网口号 */
    
    Uint32 ip;          /** 本机IP 地址 */
    Uint32 gwip;		/** 本机网关*/
    Uint32 mask;		/** 本机子网掩码地址*/
    Uint8  mac[6];		/**本机MAC 地址 */
};

typedef struct {
    Integer32 seconds;
    Integer32 nanoseconds;
}TimeInternal;


#define flip16(x) htons(x)
#define flip32(x) htonl(x)

#define IS_SET(data, bitpos) \
	((data & ( 0x1 << bitpos )) == (0x1 << bitpos))

typedef enum
{
    LEVEL_NULL = 0,
    LEVEL_INFO,
    LEVEL_WARN,
    LEVEL_STORE,
}LOG_DEBUG;

enum
{
    ENUM_BUS =0,
    ENUM_LCD,
    ENUM_GPS,
    ENUM_RB,
    ENUM_PC_CTL,
    ENUM_PC_DISCOVER,
    ENUM_PPS_TOD,
    ENUM_LOCAL_DEV,
    ENUM_IPC_NTP,
    ENUM_IPC_PTP
};

#define TCP_DEVICE  0
#define COMM_DEVICE 1
#define INIT_DEVICE 2
#define UDP_DEVICE  3


#define SetBit(n, m)		(n |= 1 << m )
#define ClrBit(n, m)		(n &= ~(1 << m) )


#if 0
#define memcmp(a, b, c)         bcmp(a, b, (int)(c))
#define memmove(t, f, c)        bcopy(f, t, (int)(c))
#define memcpy(t, f, c)         bcopy(f, t, (int)(c))
#endif



#endif
