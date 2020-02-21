/********************************************************************************************
*                           版权所有(C) 2015, *                                 版权所有
*********************************************************************************************
*    文 件 名:       net_process.c
*    功能描述:       ptp网络传输初始化，初始化e2e p2p Ieee802.3 Udp/ip 各种通信所
                     需要的套接字及地址
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-30
*    函数列表:
                     Init_CommIeeeeRecvSock
                     Init_CommIeeeRecvE2ESock
                     Init_CommIeeeRecvP2PSock
                     Init_CommUdpSock
                     Init_MultiCastMac
                     Init_NetComm
                     Init_NetEnviroment
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-30
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/
#include "net.h"
#include "net_process.h"
#include "comm_fun.h"

#define EVENSOCK_TYPE     0
#define PEVENSOCK_TYPE    1
#define GENERALSOCK_TYPE  2
#define PGENERALSOCK_TYPE 3

#define MULTICAST_TTL      8

/********************************************************************************************
* 函数名称:    Init_CommUdpSock
* 功能描述:    初始化UDP 发送相关socket
* 输入参数:    PtpClock *pPtpclock  
               Uint8 sockType       
               NetComm *pNetComm    
               char *address        
               int portNum          
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_CommUdpSock(PtpClock *pPtpclock,Uint8 sockType,NetComm *pNetComm,char *address,int portNum)
{
    struct sockaddr_in sockaddr;
    int sockfd = -1;

    /**socket 类型初始化  */
    switch(sockType)
    {
        case EVENSOCK_TYPE:
            if(pNetComm->eventSock > 0)
            {
                close(pNetComm->eventSock);
                pNetComm->eventSock = -1;
            }
            pNetComm->eventSock = socket(AF_INET, SOCK_DGRAM,0);
            sockfd = pNetComm->eventSock;
            
            break;
        case PEVENSOCK_TYPE:
            if(pNetComm->PeventSock > 0)
            {
                close(pNetComm->PeventSock);
                pNetComm->PeventSock = -1;
            }
            pNetComm->PeventSock = socket(AF_INET, SOCK_DGRAM,0);
            sockfd = pNetComm->PeventSock;
            
            break;
        case GENERALSOCK_TYPE:
            if(pNetComm->generalSock> 0)
            {
                close(pNetComm->generalSock);
                pNetComm->generalSock = -1;
            }
            pNetComm->generalSock = socket(AF_INET, SOCK_DGRAM,0);
            sockfd = pNetComm->generalSock;
            
            break;
        case PGENERALSOCK_TYPE:
            if(pNetComm->PgeneralSock> 0)
            {
                close(pNetComm->PgeneralSock);
                pNetComm->PgeneralSock = -1;
            }
            pNetComm->PgeneralSock = socket(AF_INET, SOCK_DGRAM,0);
            sockfd = pNetComm->PgeneralSock;
            
            break;
        default:
            break;
    }
    
     if(sockfd < 0)
     {
        printf("Can't creat receive socket %d \r\n",sockType);
     }
     memset(&sockaddr,0,sizeof(sockaddr));

    sockaddr.sin_family=AF_INET;
    sockaddr.sin_port = htons(portNum);
    
    if(pPtpclock->modeType == IPMODE_UNICAST)
    {
        sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else if(pPtpclock->modeType == IPMODE_MULTICAST)
    {
        sockaddr.sin_addr.s_addr = inet_addr(address);
    }

    /**设置不检测  */
    //SetSock_NoCheck (sockfd);
    
    /**绑定到网卡  */
    BindSockDevice(pPtpclock->netEnviroment.ifaceName,sockfd);


    if(pPtpclock->modeType == IPMODE_MULTICAST)
        SetSock_reUse(sockfd);

    if (0 > bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)))
	{
		printf("UDP socket bind ReceiveSock error!\n");

	}

    if(pPtpclock->modeType == IPMODE_MULTICAST)
    {
        SetMutiCastLoop(sockfd,FALSE);
        AddIpAddr_ToMultiCast(sockfd,pPtpclock->netEnviroment.ip,sockaddr.sin_addr.s_addr,MULTICAST_TTL);
    }

    
}


/********************************************************************************************
* 函数名称:    Init_MultiCastMac
* 功能描述:    初始化多播Mac地址
* 输入参数:    NetComm *pNetComm  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_MultiCastMac(NetComm *pNetComm)
{
    pNetComm->E2EMulticastMac[0]=0x01;                      
    pNetComm->E2EMulticastMac[1]=0x1b;
    pNetComm->E2EMulticastMac[2]=0x19;
    pNetComm->E2EMulticastMac[3]=0x00;
    pNetComm->E2EMulticastMac[4]=0x00;
    pNetComm->E2EMulticastMac[5]=0x00;

    pNetComm->P2PMulticastMac[0]=0x01;                      
    pNetComm->P2PMulticastMac[1]=0x80;
    pNetComm->P2PMulticastMac[2]=0xc2;
    pNetComm->P2PMulticastMac[3]=0x00;
    pNetComm->P2PMulticastMac[4]=0x00;
    pNetComm->P2PMulticastMac[5]=0x0e;

}

/********************************************************************************************
* 函数名称:    Init_CommIeeeeRecvSock
* 功能描述:    初始化2层数据Socket
* 输入参数:    PtpClock *pPtpClock  
               Sint32 *pSockfd      
               Uint8*pMac           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Uint8 Init_CommIeeeeRecvSock(PtpClock *pPtpClock,Sint32 *pSockfd,Uint8*pMac)
{
    struct ifreq ifr;
    struct sockaddr_ll  Ieee_addr;
    struct packet_mreq emr;

    if(*pSockfd > 0)
    {
        close(*pSockfd);
        *pSockfd = -1;
    }
    *pSockfd = socket(PF_PACKET,SOCK_RAW, htons(ETH_P_ALL));
    if (*pSockfd < 0)
    {
        printf("Can't creat layer2 EVENT socket  \r\n");
        return FALSE;
    }
    
    memset(&ifr, 0, sizeof(ifr));
    memcpy(ifr.ifr_name, pPtpClock->netEnviroment.ifaceName, sizeof(ifr.ifr_name)-1);
    
    if(ioctl(*pSockfd,SIOCGIFINDEX,&ifr)<0)
    {
        PLOG("get sll_ifindex error\n");
        return FALSE;
    }
    
    memset(&Ieee_addr,0,sizeof(struct sockaddr_ll));
    
    Ieee_addr.sll_family=AF_PACKET;    
    Ieee_addr.sll_protocol=htons(ETH_P_ALL);
    Ieee_addr.sll_ifindex=ifr.ifr_ifindex;
    
    if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        Ieee_addr.sll_pkttype = PACKET_MULTICAST; 
        memcpy(Ieee_addr.sll_addr,pMac,6);
    }
    else if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        Ieee_addr.sll_pkttype = PACKET_OTHERHOST;
    }
    
    Ieee_addr.sll_hatype = ARPHRD_ETHER;         
    Ieee_addr.sll_halen = ETH_ALEN; 
    
    if (bind(*pSockfd, (struct sockaddr *) &Ieee_addr, sizeof(struct sockaddr_ll)) < 0)   
    {
        printf("Ieee socket bind ReceiveSock error!\n\n");
        return FALSE;
    }

    /*绑定到网口*/
    BindSockDevice(pPtpClock->netEnviroment.ifaceName,*pSockfd);

     /*加入组播组*/
    if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        emr.mr_ifindex = ifr.ifr_ifindex;
        emr.mr_type = PACKET_MR_MULTICAST;   
        emr.mr_alen = sizeof(pPtpClock->netComm.E2EMulticastMac); 
        memcpy(emr.mr_address,pMac,emr.mr_alen);
       
        if(setsockopt(*pSockfd,SOL_PACKET,PACKET_ADD_MEMBERSHIP,&emr,sizeof(struct packet_mreq))<0)
        {
            printf("failed to set Ieee802.3 multicast interface\n");
            return FALSE;
        }
    }
    return TRUE;
}



/********************************************************************************************
* 函数名称:    Init_NetComm
* 功能描述:    初始化通信所需的所有Socket套接字
* 输入参数:    PtpClock *pPtpclock  
               NetComm *pNetComm    
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_NetComm(PtpClock *pPtpclock,NetComm *pNetComm)
{
    if(pPtpclock->modeType == IPMODE_MULTICAST)
    {
        Init_CommUdpSock(pPtpclock,EVENSOCK_TYPE,pNetComm,PTP_E2E_MULTICAST_ADDRESS,PTP_EVENT_PORT);
        
        Init_CommUdpSock(pPtpclock,PEVENSOCK_TYPE,pNetComm,PTP_PEER_MULTICAST_ADDRESS,PTP_EVENT_PORT);
        
        Init_CommUdpSock(pPtpclock,GENERALSOCK_TYPE,pNetComm,PTP_E2E_MULTICAST_ADDRESS,PTP_GENERAL_PORT);
        
        Init_CommUdpSock(pPtpclock,PGENERALSOCK_TYPE,pNetComm,PTP_PEER_MULTICAST_ADDRESS,PTP_GENERAL_PORT);
        
    }
    else if(pPtpclock->modeType == IPMODE_UNICAST)
    {
        Init_CommUdpSock(pPtpclock,EVENSOCK_TYPE,pNetComm,PTP_E2E_MULTICAST_ADDRESS,PTP_EVENT_PORT);
        Init_CommUdpSock(pPtpclock,GENERALSOCK_TYPE,pNetComm,PTP_E2E_MULTICAST_ADDRESS,PTP_GENERAL_PORT);
    }

    Init_MultiCastMac(pNetComm);

    Init_CommIeeeeRecvSock(pPtpclock,&pNetComm->IeeeE2ESock,pNetComm->E2EMulticastMac);
    
    Init_CommIeeeeRecvSock(pPtpclock,&pNetComm->IeeeP2PSock,pNetComm->P2PMulticastMac);
}



/********************************************************************************************
* 函数名称:    Init_NetEnviroment
* 功能描述:    获取当前网络配置，包含IP，mac Netmast gateway
* 输入参数:    NetInfor *pNetInfor  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Init_NetEnviroment(NetInfor *pNetInfor)
{
    char mac[6];

    GetIpAddress(pNetInfor->ifaceName,&pNetInfor->ip);
    GetMaskAddress(pNetInfor->ifaceName,&pNetInfor->mask);
    GetMacAddress(pNetInfor->ifaceName,pNetInfor->mac);
        
    //GetGateWay(pNetInfor->ifaceName,&pNetInfor->gwip);
}

