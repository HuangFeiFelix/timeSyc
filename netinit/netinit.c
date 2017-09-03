#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/sockios.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <net/if.h>
#include <net/route.h>

#include <string.h>
#include <ifaddrs.h>
#include <ctype.h>


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


char *ctlEthConfig = "/mnt/network_ctl.cfg";
char *ptpEthConfig = "/mnt/network_ptp.cfg";
char *ntpEthConfig = "/mnt/network_ntp.cfg";

#define TRUE    1
#define FALSE   0
#define MAC_ADDRESS_LEN 6
#define PATH_PROCNET_ROUTE		"/proc/net/route"
#define IFACE_NAME_LENGTH        IF_NAMESIZE

struct NetInfor
{
    char ifaceName[5];  /** 所在网口号 */
    
    Uint32 ip;          /** 本机IP 地址 */
    Uint32 gwip;		/** 本机网关*/
    Uint32 mask;		/** 本机子网掩码地址*/
    Uint8  mac[6];		/**本机MAC 地址 */
};


/**********************************************************************
 * Function:      SetMacAddress
 * Description:   设置mac地址
 * Input:         pInterface ：网络接口名字 如：eth0，pMacAddress：mac
地址首地址
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char SetMacAddress(char *pInterface,char *pMacAddress)
{
    struct ifreq ifr;
    struct sockaddr *hwaddr;
    unsigned char *hwptr;
    int sockfd;
    int i;

	/* 判断接口输入是否正确 */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(pMacAddress == NULL)
    {
        printf("input mac address error\n");
        return FALSE;
    }
    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
    
     /*读出socket的信息*/
	if (0 > ioctl(sockfd, SIOCGIFFLAGS, &ifr))
	{
		printf("close the ethx:unknown interface\r\n");
	}
	ifr.ifr_flags &= ~IFF_UP;
    /*将信息写到socket中，从而关闭eth0*/
	if (0 > ioctl(sockfd, SIOCSIFFLAGS, &ifr)) 
	{
		printf("Can't close the ethx \r\n");
        return FALSE;
	}

	/*---------------------------------------------------------------------*/
	/* set the interface eth0's  MAC address                               */
	/*---------------------------------------------------------------------*/
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
	hwaddr=(struct sockaddr *)&ifr.ifr_hwaddr;
    
    /*ARPHRD_ETHER＝1，标识接收的ARP为以太网格式*/
	hwaddr->sa_family = ARPHRD_ETHER;
	hwptr = hwaddr->sa_data;

	for(i=0;i<6;i++)
		*hwptr++ = (unsigned char)pMacAddress[i];
		
	if (0 > ioctl(sockfd, SIOCSIFHWADDR, &ifr))
	{
		printf("Can't set the MAC address\r\n");
	}

    /*---------------------------------------------------------------------*/
    /* open the interface eth0                                             */
    /*---------------------------------------------------------------------*/	
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
    if (0 > ioctl(sockfd,SIOCGIFFLAGS, &ifr)) 
    {
    	printf("open the ethx:unknown interface\r\n");
    }
    ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
    if (0 > ioctl(sockfd, SIOCSIFFLAGS, &ifr)) 
    {
    	printf("Can't open the ethx\r\n");
    }
    
    close(sockfd);

    return TRUE;
}


/**********************************************************************
 * Function:      SetIpAddress
 * Description:   设置ip地址
 * Input:         pInterface ：网络接口名字 如：eth0，IpAddress：ip地址的
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char SetIpAddress(char *pInterface,unsigned int IpAddress)
{

    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

    
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(IpAddress == 0)
    {
        printf("input ip address error\n");
        return FALSE;
    }
 
    sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
	sinaddr = (struct sockaddr_in *)&ifr.ifr_addr;
	sinaddr->sin_family = AF_INET;
	sinaddr->sin_addr.s_addr = IpAddress;
	if (0 > ioctl(sockfd, SIOCSIFADDR, &ifr)) 
	{
		printf("Can't set ip address\r\n");
	}

    close(sockfd);

    return TRUE;
}

/**********************************************************************
 * Function:      SetMaskAddress
 * Description:   设置子网掩码
 * Input:         pInterface ：网络接口名字 如：eth0，IpMask：子网掩码
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char SetMaskAddress(char *pInterface,unsigned int IpMask)
{
    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

	/* 判断接口输入是否正确 */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(IpMask == 0)
    {
        printf("input mask address error\n");
        return FALSE;
    }

    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, pInterface, sizeof(ifr.ifr_name)-1);
	sinaddr = (struct sockaddr_in *)&ifr.ifr_netmask;
	sinaddr->sin_family = AF_INET;
	sinaddr->sin_addr.s_addr = IpMask;
	if (0 > ioctl(sockfd, SIOCSIFNETMASK, &ifr)) 
	{
		printf("Can't set subnet netmask\r\n");
	}

    close(sockfd);

    return TRUE;
}

/**********************************************************************
 * Function:      DelGateWay
 * Description:   删除指定的网关
 * Input:         pInterface ：网络接口名字 如：eth0，DelGateWay：网关
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char DelGateWay(char *pInterface,unsigned int DelGateWay)
{

    struct sockaddr_in *sinaddr;
    struct rtentry rt;
    struct sockaddr rtsockaddr;
    int sockfd;

    /* 判断接口输入是否正确 */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(DelGateWay == 0)
    {
        printf("input GateWay error!\n");
        return FALSE;
    }
    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}


	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	memset((char *)&rtsockaddr,0,sizeof(struct sockaddr));
	sinaddr = (struct sockaddr_in *)&rtsockaddr;
	sinaddr->sin_family = AF_INET;
    
    /*INADDR_ANY就是指定地址为0.0.0.0的地址,这个地址事实上表示不确定地址,
或“所有地址”、“任意地址”*/
	sinaddr->sin_addr.s_addr = INADDR_ANY;	
    
    /*set the destination address to '0.0.0.0'*/
	rt.rt_dst = rtsockaddr;

    /*set the netmask to '0.0.0.0'*/
	rt.rt_genmask = rtsockaddr;			

	/* Fill in the other fields. */
	rt.rt_flags = (RTF_UP|RTF_GATEWAY);
	rt.rt_metric = 1;
	rt.rt_dev = pInterface;

	/*delete the current default gateway*/
	sinaddr->sin_addr.s_addr = DelGateWay;
	rt.rt_gateway = rtsockaddr;

    
	if (0 > ioctl(sockfd, SIOCDELRT, &rt)) 
	{
		printf("Can't delete the current gateway\r\n");
	}

    close(sockfd);

    return TRUE;
}

/**********************************************************************
 * Function:      AddGateWay
 * Description:   增加指定的网关
 * Input:         pInterface ：网络接口名字 如：eth0，NewGateWay：网关
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char AddGateWay(char *pInterface,unsigned int NewGateWay)
{
    struct sockaddr_in *sinaddr;
    struct rtentry rt;
    struct sockaddr rtsockaddr;
    int sockfd;

    /* 判断接口输入是否正确 */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(NewGateWay == 0)
    {
        printf("input GateWay error!\n");
        return FALSE;
    }
    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}


	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	memset((char *)&rtsockaddr,0,sizeof(struct sockaddr));
	sinaddr = (struct sockaddr_in *)&rtsockaddr;
	sinaddr->sin_family = AF_INET;
    
    /*INADDR_ANY就是指定地址为0.0.0.0的地址,这个地址事实上表示不确定地址,
或“所有地址”、“任意地址”*/
	sinaddr->sin_addr.s_addr = INADDR_ANY;	
    
    /*set the destination address to '0.0.0.0'*/
	rt.rt_dst = rtsockaddr;

    /*set the netmask to '0.0.0.0'*/
	rt.rt_genmask = rtsockaddr;			

	/* Fill in the other fields. */
	rt.rt_flags = (RTF_UP|RTF_GATEWAY);
	rt.rt_metric = 1;
	rt.rt_dev = pInterface;

	/*set the new default gateway*/
	sinaddr->sin_addr.s_addr = NewGateWay;
	rt.rt_gateway =rtsockaddr;
	    
	if (0 > ioctl(sockfd, SIOCADDRT, &rt))
	{
		printf("Can't set the new default gateway\r\n");
	}

    close(sockfd);

    return TRUE;

}

/**********************************************************************
 * Function:      SetGateWay
 * Description:   设置网关，先删除已经有的，再添加新的
 * Input:         pInterface ：网络接口名字 如：eth0，DelGateWay：删除的网关，
				  NewGateWay： 新添加的网关
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char SetGateWay(char *pInterface,unsigned int DelGateWay,unsigned int NewGateWay)
{

    struct sockaddr_in *sinaddr;
    struct rtentry rt;
    struct sockaddr rtsockaddr;
    int sockfd;

    /* 判断接口输入是否正确 */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if((DelGateWay == 0) && (NewGateWay == 0))
    {
        printf("input GateWay error!\n");
        return FALSE;
    }
    
	sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
	}


	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	memset((char *)&rtsockaddr,0,sizeof(struct sockaddr));
	sinaddr = (struct sockaddr_in *)&rtsockaddr;
	sinaddr->sin_family = AF_INET;
    
    /*INADDR_ANY就是指定地址为0.0.0.0的地址,这个地址事实上表示不确定地址,
或“所有地址”、“任意地址”*/
	sinaddr->sin_addr.s_addr = INADDR_ANY;	
    
    /*set the destination address to '0.0.0.0'*/
	rt.rt_dst = rtsockaddr;

    /*set the netmask to '0.0.0.0'*/
	rt.rt_genmask = rtsockaddr;			

	/* Fill in the other fields. */
	rt.rt_flags = (RTF_UP|RTF_GATEWAY);
	rt.rt_metric = 1;
	rt.rt_dev = pInterface;

    
	/*delete the current default gateway*/
	sinaddr->sin_addr.s_addr = DelGateWay;
	rt.rt_gateway = rtsockaddr;

    if(DelGateWay != 0)
    {
        if (0 > ioctl(sockfd, SIOCDELRT, &rt)) 
        {
            printf("Can't delete the current default gateway\r\n");
        }
    }


	/*set the new default gateway*/
	sinaddr->sin_addr.s_addr = NewGateWay;
	rt.rt_gateway =rtsockaddr;
	    
	if (0 > ioctl(sockfd, SIOCADDRT, &rt))
	{
		printf("Can't set the new default gateway\r\n");
	}

    close(sockfd);

    return TRUE;
}

/**********************************************************************
 * Function:      GetMacAddress
 * Description:   获得指定网口的物理MAC地质
 * Input:         ifaceName ：网络接口名字 如：eth0，hwAddr：得到的mac地址
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char GetMacAddress (char* ifaceName, unsigned char* hwAddr)
{
    int ret;
    int sockfd;
    struct ifreq ifr;

    /* 判断接口输入是否正确 */
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
        
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
    	printf("Could not open test socket");
    	return FALSE;
    }

    memset(&ifr, 0, sizeof(ifr));

    strncpy(ifr.ifr_name, ifaceName, IFACE_NAME_LENGTH);

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        printf("failed to request hardware address for %s", ifaceName);
	    close(sockfd);
        return FALSE;
    }

    //int af = ifr.ifr_hwaddr.sa_family;
    //int af = ifr.ifr_addr.sa_family;
    
    memcpy(hwAddr, ifr.ifr_hwaddr.sa_data, MAC_ADDRESS_LEN);
   //memcpy(hwAddr, ifr.ifr_addr.sa_data, hwAddrSize);

	return TRUE;
    
}

/**********************************************************************
 * Function:      GetIpAddress
 * Description:   获得指定端口的ip地址
 * Input:         ifaceName ：网络接口名字 如：eth0，IpAddress：得到的ip地址
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char GetIpAddress(char* ifaceName,unsigned int *IpAddress)
{
    struct ifaddrs *ifaddr, *ifa;
    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

    /* 判断接口输入是否正确 */
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }

    if(getifaddrs(&ifaddr) == -1)
    {
	    printf("Could not get interface list");
        freeifaddrs(ifaddr);
        return FALSE;
    }
    
    sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
        return FALSE;
	}

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
    	if(!strcmp(ifaceName, ifa->ifa_name) && (ifa->ifa_addr->sa_family == AF_INET))
        {
    		memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name)-1);
            if (0 > ioctl(sockfd, SIOCGIFADDR, &ifr)) 
	        {
		        printf("Can't get ip address\r\n");
                return FALSE;
	        }
            sinaddr = (struct sockaddr_in *)&ifr.ifr_addr;
            *IpAddress = sinaddr->sin_addr.s_addr;
            freeifaddrs(ifaddr);
            close(sockfd);
            return TRUE;

    	}

    }

    printf("input interface no found\n");

    freeifaddrs(ifaddr);
    
    return FALSE;
}

/**********************************************************************
 * Function:      GetMaskAddress
 * Description:   获得指定端口的子网掩码
 * Input:         pInterface ：网络接口名字 如：eth0，MaskAddress
：得到的子网掩码
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char GetMaskAddress(char* ifaceName,unsigned int *MaskAddress)
{
    struct ifaddrs *ifaddr, *ifa;
    struct ifreq ifr;
    struct sockaddr_in *sinaddr;
    int sockfd;

    /* 判断接口输入是否正确 */
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }

    if(getifaddrs(&ifaddr) == -1)
    {
	    printf("Could not get interface list");
        freeifaddrs(ifaddr);
        return FALSE;
    }

    sockfd= socket(AF_INET,SOCK_DGRAM,0);
	if (sockfd<0)
	{
		printf("Can't creat socket  \r\n");
        return FALSE;
	}


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
    	if(!strcmp(ifaceName, ifa->ifa_name) && (ifa->ifa_addr->sa_family == AF_INET))
        {
    		memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, ifa->ifa_name, sizeof(ifr.ifr_name)-1);
            if (0 > ioctl(sockfd, SIOCGIFNETMASK, &ifr)) 
	        {
		        printf("Can't get ip address\r\n");
                return FALSE;
	        }
            sinaddr = (struct sockaddr_in *)&ifr.ifr_netmask;
            *MaskAddress = sinaddr->sin_addr.s_addr;
            freeifaddrs(ifaddr);
            close(sockfd);
            return TRUE;
    	}
    }

    printf("input interface no found\n");
    freeifaddrs(ifaddr);
    
    return FALSE;
}

/**********************************************************************
 * Function:      GetAddress
 * Description:   获得指定网口的sockdaddr地址
 * Input:         ifaceName ：网络接口名字 如：eth0，addr：地址
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char GetAddress(char* ifaceName,struct sockaddr* addr)
{
    struct ifaddrs *ifaddr, *ifa;

    /* 判断接口输入是否正确 */
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(
ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }

    if(getifaddrs(&ifaddr) == -1)
    {
	    printf("Could not get interface list");
        freeifaddrs(ifaddr);
        return FALSE;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {

    	if(!strcmp(ifaceName, ifa->ifa_name) && (ifa->ifa_addr->sa_family == AF_INET))
        {
    		memcpy(addr, ifa->ifa_addr, sizeof(struct sockaddr));
            freeifaddrs(ifaddr);
            return TRUE;

    	}

    }

    printf("input interface no found\n");
    freeifaddrs(ifaddr);
    return FALSE;
}


/**********************************************************************
 * Function:      GetGateWay
 * Description:   得到默认网关
 * Input:         pInterface ：网络接口名字 如：eth0，pGateWay：得到的网关
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char GetGateWay(char *pInterface,unsigned int *pGateWay)
{
    short i = 0;
 	char buff[1024], iface[16];
	char gate_addr[128], net_addr[128];
	char mask_addr[128];
	int num, iflags, metric, refcnt, use, mss, window, irtt;
    
 	char *sp , *bp;    
	unsigned int val=0;

    /* 判断接口输入是否正确 */
    if((pInterface == NULL)||(pInterface[0]!='e')||(pInterface[1]!='t')||(
pInterface[2]!='h'))
    {
        printf("input inteface error!\n");
        return FALSE;
    }
    if(pGateWay == NULL)
    {
        printf("input mac address error\n");
        return FALSE;
    }

    FILE *fp = fopen(PATH_PROCNET_ROUTE, "r");

    while (fgets(buff, 1023, fp))
    {
             num = sscanf(buff, "%16s %128s %128s %X %d %d %d %128s %d %d %d\n",
             iface, net_addr, gate_addr,
             &iflags, &refcnt, &use, &metric, mask_addr,
             &mss, &window, &irtt);
             if (num < 10 || !(iflags & RTF_UP))
                 continue;
             if(strncmp(iface,pInterface,4)!=0)
                 continue;
             if(strncmp(net_addr,"00000000",8)!=0)
                 continue;
             if(strncmp(mask_addr,"00000000",8)!=0)
                 continue;
             bp = (char *) &val;
             
             sp = gate_addr;
             for (i = 0; i < 4; i++) 
             {
                 *sp = toupper(*sp);
                 if ((*sp >= 'A') && (*sp <= 'F'))
                     bp[i] |= (int) (*sp - 'A') + 10;
                 else if ((*sp >= '0') && (*sp <= '9'))
                     bp[i] |= (int) (*sp - '0');
                 else
                 {
                     printf("Can't convert the gateway string to integer.\n\r"
);
                     fclose(fp);
                     return 0;
                 }
                 bp[i] <<= 4;
                 sp++;
                 *sp = toupper(*sp);
     
                 if ((*sp >= 'A') && (*sp <= 'F'))
                         bp[i] |= (int) (*sp - 'A') + 10;
                 else if ((*sp >= '0') && (*sp <= '9'))
                         bp[i] |= (int) (*sp - '0');
                 else
                 {
                     printf("Can't convert the gateway string to integer.\n");
                     fclose(fp);
                     return 0;
                 }
                 sp++;
             }
    }

    if(val == 0)
    {
        printf("Don't find default gateway = %d\n",val);
        fclose(fp);
        return FALSE;
    }
    else
    {
        *pGateWay = htonl(val);
    }


    fclose(fp);
    return TRUE;
}


/**********************************************************************
 * Function:      SetSock_reUse
 * Description:   设置网络套接字重复使用
 * Input:         sockfd：网络套接字
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char SetSock_reUse(int sockfd)
{
    int reuse = 1;
    if(-1 == setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)))
    {
		printf("setsockopt fail!\n");
        return FALSE;
	}

    return TRUE;
}

/**********************************************************************
 * Function:      SetSock_NoCheck
 * Description:   设置套接字数据包不检测
 * Input:         sockfd：网络套接字
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char SetSock_NoCheck(int sockfd)
{
    int nocheck = 1;
    if(0 != setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, &nocheck, sizeof(int)))
    {
        printf("setsockopt NO_CHECK error!\n");
        return FALSE;
    }

    return TRUE;
}

/**********************************************************************
 * Function:      SetMutiCastLoop
 * Description:   设置回环多播
 * Input:         sockfd：socket套接字，val：0 关闭，1 打开
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char SetMutiCastLoop(int sockfd,int val)
{
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &val, sizeof(int))< 0)
    {
        printf("failed to set multicast LOOP value \n");
        return FALSE;
    }

    return TRUE;
}

/**********************************************************************
 * Function:      SetMutiCastTTl
 * Description:   设置多播地址组数据包的生存时间
 * Input:         sockfd：socket套接字，ttl，生存时间
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char SetMutiCastTTl(int sockfd,int ttl)
{
    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&ttl, sizeof(ttl))< 0)
    {
        printf("failed to set multicast TTL value \n");
        return FALSE;
    }
    return TRUE;
}

unsigned char BindSockDevice(char*ifaceNmae,int sockfd)
{
    struct ifreq ifreg;
    memcpy(ifreg.ifr_name, ifaceNmae,IFNAMSIZ);
    if(setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE,(char *)&ifreg, sizeof(ifreg)) < 0)
    {
        printf("bindSockDevice error\n");
        return FALSE;
    }

    return TRUE;
}

/**********************************************************************
 * Function:      AddIpAddr_ToMultiCast
 * Description:   将指定ip地址添加入多播组
 * Input:         sockfd：socket套接字，ip：ip地址，multi_ip：多播地址，ttl
：生存时间
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
unsigned char AddIpAddr_ToMultiCast(int sockfd,unsigned int ip,unsigned int multi_ip,int ttl)
{
    struct ip_mreq imr;
    
    imr.imr_multiaddr.s_addr = multi_ip;
    imr.imr_interface.s_addr = ip;

    if(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &imr.imr_interface.s_addr, sizeof(struct in_addr)) < 0)
    {
        printf("failed to set  multicast send interface\n");
        return FALSE;
    }
    
    if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr))< 0)
    {
        printf("failed to add  multicast member\n");
        return FALSE;
    }
    if(SetMutiCastTTl(sockfd,ttl) == FALSE)
    {
        return FALSE;
    }
    
    return TRUE;
}

/**********************************************************************
 * Function:      DisplayInterfaceStatus
 * Description:   显示当前接口状态信息
 * Input:         ifaceName ：网络接口名字 如：eth0
 * Return:        void
 * Others:        
**********************************************************************/
void DisplayInterfaceStatus(char* ifaceName)
{
    struct ifaddrs *ifaddr, *ifa;
    unsigned int flags;

    
    if((ifaceName == NULL)||(ifaceName[0]!='e')||(ifaceName[1]!='t')||(ifaceName[2]!='h'))
    {
        printf("input inteface error!\n");
        return;
    }
    
    if(getifaddrs(&ifaddr) == -1)
    {
    	printf("Could not get interface list");
        freeifaddrs(ifaddr);
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {

    	if(!strcmp(ifaceName, ifa->ifa_name))
        {
    	    flags = ifa->ifa_flags;
    	    freeifaddrs(ifaddr);
    	}

    }
    if(flags & IFF_UP)
    {
        printf("Interface %s is up \n",ifaceName);
    }
    if(flags & IFF_RUNNING)
    {
        printf("Interface %s is running ",ifaceName);
    }

}

void SetNetworkToEnv(struct NetInfor *infopt)
{
    SetMacAddress(infopt->ifaceName,infopt->mac);
    usleep(500000);

    if(strcmp(infopt->ifaceName,"eth2") == 0)
        AddGateWay(infopt->ifaceName,infopt->gwip);

}

void Get_Net_FormSysEvn(struct NetInfor *pNetInfor)
{
    char mac[6];
  
    GetIpAddress(pNetInfor->ifaceName,&pNetInfor->ip);
    GetMaskAddress(pNetInfor->ifaceName,&pNetInfor->mask);
    GetMacAddress(pNetInfor->ifaceName,pNetInfor->mac);
        
    GetGateWay(pNetInfor->ifaceName,&pNetInfor->gwip);
}


void SaveNetParamToFile(char *network_cfg,struct NetInfor *infopt)
{
    Uint8 line_str[200];
    struct sockaddr_in temsock;
    FILE *network_fd = fopen(network_cfg,"w+");
    
    if(network_fd == NULL)
    {
        printf("SaveNet can not find network.conf file\n");
        return;
    }
       
    memset(line_str,0,sizeof(line_str));
    
    temsock.sin_addr.s_addr = infopt->ip;
    sprintf(line_str,"%s:%s\n","IP",inet_ntoa(temsock.sin_addr));
    fputs(line_str,network_fd);
    
    memset(line_str,0,sizeof(line_str));
    temsock.sin_addr.s_addr = infopt->gwip;
    sprintf(line_str,"%s:%s\n","GATEWAY",inet_ntoa(temsock.sin_addr));
    fputs(line_str,network_fd);
    
    memset(line_str,0,sizeof(line_str));
    temsock.sin_addr.s_addr = infopt->mask;
    sprintf(line_str,"%s:%s\n","MASK",inet_ntoa(temsock.sin_addr));
    fputs(line_str,network_fd);
    
    memset(line_str,0,sizeof(line_str));
    sprintf(line_str,"%s:%02x.%02x.%02x.%02x.%02x.%02x\n","MAC",infopt->mac[0]
        ,infopt->mac[1]
        ,infopt->mac[2]
        ,infopt->mac[3]
        ,infopt->mac[4]
        ,infopt->mac[5]);
    
    fputs(line_str,network_fd);
    
    fflush(network_fd);
    fclose(network_fd);

}

int Load_NetWorkParam_FromFile(char *network_cfg,struct NetInfor *infopt)
{
    Uint8 line_str[50];
    
    Uint8 tile[10];
    Uint8 *pIndex;
    Uint8 *pStr = line_str;
    struct in_addr temsock;
    
    Uint8 *pMack;
    int i;
    
    memset(line_str,0,sizeof(line_str));
    memset(tile,0,sizeof(tile));
    
    FILE *net_work_fd = fopen(network_cfg,"r");
    if(net_work_fd == NULL)
    {
        printf("can not find network.conf file\n");
        Get_Net_FormSysEvn(infopt);
        SaveNetParamToFile(network_cfg,infopt);
        //SetRouteToEnv(network_cfg,infopt);
        return -1;
    }
    
    while(fgets(line_str,sizeof(line_str),net_work_fd))
    {
        
        pIndex = strchr(line_str,':');
        memcpy(tile,line_str,pIndex-pStr);

        pIndex++;
        
        if(strcmp("IP",tile) == 0)
        {
            inet_aton(pIndex,&temsock);
            infopt->ip = temsock.s_addr;

        }
        if(strcmp("GATEWAY",tile) == 0)
        {
            inet_aton(pIndex,&temsock);
            infopt->gwip = temsock.s_addr;

        }
        if(strcmp("MASK",tile) == 0)
        {
            inet_aton(pIndex,&temsock);
            infopt->mask = temsock.s_addr;
            
        }

        if(strcmp("MAC",tile) == 0)
        {

            pMack = pIndex;
    		for(i = 0; i < 6; i++)
    		{
    			pMack = strtok(pIndex,".");
    			infopt->mac[i] = strtol(pMack,NULL,16);			
    			pIndex = NULL;
    		}		            
        }

        memset(tile,0,sizeof(tile));
        memset(line_str,0,sizeof(line_str));
    }
    SetNetworkToEnv(infopt);
    //SetRouteToEnv(network_cfg,infopt);
    fclose(net_work_fd);
}


struct NetInfor ptp_port;
struct NetInfor ntp_port;
struct NetInfor ctl_port;


void main()
{
    strcpy(ptp_port.ifaceName,"eth0");
    strcpy(ntp_port.ifaceName,"eth1");
    strcpy(ctl_port.ifaceName,"eth2");
    Load_NetWorkParam_FromFile(ctlEthConfig,&ctl_port);
    Load_NetWorkParam_FromFile(ntpEthConfig,&ntp_port);
    Load_NetWorkParam_FromFile(ptpEthConfig,&ptp_port);


}

