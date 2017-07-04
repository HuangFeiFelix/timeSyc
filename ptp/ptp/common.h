/********************************************************************************************
*                           ��Ȩ����(C) 2015, ���ſ�ѧ���������о���
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       common.h
*    ��������:        ����ͷ�ļ�����������ͷ�ļ��������������
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-1-30
*    �����б�:
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-1-30
                     ����: HuangFei
                     �޸�����: �´����ļ�


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
#include <math.h>

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
#include <math.h>

#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/socket.h>

#include <asm/types.h>
#include <linux/if_ether.h>
#include <netpacket/packet.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/mii.h>

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


typedef struct
{	
	Uint32 year;	
	Uint32 day;					/*����ĵڼ���*/
	Uint32 hour;
	Uint32 minute;
	Uint32 second;		
	Uint8 leap_flag;		    /*�����־��0�������룻1��������*/
	Uint8 leap_d;				/*���뷽��0�������룻1��������*/
	Uint8 timezone_flag;		/*ʱ����־��0����ʱ����1����ʱ��*/
	Uint8 timezone;				/*ʱ����*/	
	Uint8 timequality;			/*ʱ�ӵȼ�:0�����٣�1�����֣�2����������*/
}DCLSTIME,*PDCLSTIME;


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

#if 0
#define memcmp(a, b, c)         bcmp(a, b, (int)(c))
#define memmove(t, f, c)        bcopy(f, t, (int)(c))
#define memcpy(t, f, c)         bcopy(f, t, (int)(c))
#endif



#endif
