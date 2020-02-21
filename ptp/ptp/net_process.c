/********************************************************************************************
*                           ��Ȩ����(C) 2015, *                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       net_process.c
*    ��������:       ptp���紫���ʼ������ʼ��e2e p2p Ieee802.3 Udp/ip ����ͨ����
                     ��Ҫ���׽��ּ���ַ
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-1-30
*    �����б�:
                     Init_CommIeeeeRecvSock
                     Init_CommIeeeRecvE2ESock
                     Init_CommIeeeRecvP2PSock
                     Init_CommUdpSock
                     Init_MultiCastMac
                     Init_NetComm
                     Init_NetEnviroment
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-1-30
                     ����: HuangFei
                     �޸�����: �´����ļ�


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
* ��������:    Init_CommUdpSock
* ��������:    ��ʼ��UDP �������socket
* �������:    PtpClock *pPtpclock  
               Uint8 sockType       
               NetComm *pNetComm    
               char *address        
               int portNum          
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Init_CommUdpSock(PtpClock *pPtpclock,Uint8 sockType,NetComm *pNetComm,char *address,int portNum)
{
    struct sockaddr_in sockaddr;
    int sockfd = -1;

    /**socket ���ͳ�ʼ��  */
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

    /**���ò����  */
    //SetSock_NoCheck (sockfd);
    
    /**�󶨵�����  */
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
* ��������:    Init_MultiCastMac
* ��������:    ��ʼ���ಥMac��ַ
* �������:    NetComm *pNetComm  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

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
* ��������:    Init_CommIeeeeRecvSock
* ��������:    ��ʼ��2������Socket
* �������:    PtpClock *pPtpClock  
               Sint32 *pSockfd      
               Uint8*pMac           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

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

    /*�󶨵�����*/
    BindSockDevice(pPtpClock->netEnviroment.ifaceName,*pSockfd);

     /*�����鲥��*/
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
* ��������:    Init_NetComm
* ��������:    ��ʼ��ͨ�����������Socket�׽���
* �������:    PtpClock *pPtpclock  
               NetComm *pNetComm    
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

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
* ��������:    Init_NetEnviroment
* ��������:    ��ȡ��ǰ�������ã�����IP��mac Netmast gateway
* �������:    NetInfor *pNetInfor  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Init_NetEnviroment(NetInfor *pNetInfor)
{
    char mac[6];

    GetIpAddress(pNetInfor->ifaceName,&pNetInfor->ip);
    GetMaskAddress(pNetInfor->ifaceName,&pNetInfor->mask);
    GetMacAddress(pNetInfor->ifaceName,pNetInfor->mac);
        
    //GetGateWay(pNetInfor->ifaceName,&pNetInfor->gwip);
}

