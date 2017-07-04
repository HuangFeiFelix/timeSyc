/********************************************************************************************
*                           ��Ȩ����(C) 2015, ���ſ�ѧ���������о���
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       unicast_process.c
*    ��������:       ������ش�����������Э�̣��������£������������б���µ�
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-2-2
*    �����б�:
                     Caculate_DelayRespMsgTime
                     Caculate_FollowUpMsgTime
                     Caculate_PdelayFollowUpMsgTime
                     Caculate_PdelayRespMsgTime
                     Caculate_SynMsgTime
                     CalculateConnectedClient
                     CheckSendDelayServerEnable
                     Check_ThreeMessageRecvAlarm
                     Display_NetInfo
                     Display_PtpStatusEvery10s
                     GetClientUnicast_AnnounceAddress
                     GetClientUnicast_SynAddress
                     GetMultiServ_SendServerAddress_And_MsgType
                     GetSendDelayServer_Ip
                     GetSendDelayServer_Mac
                     Init_UnicastClient
                     IsTwoStep
                     RecvMsg_UpdateUnicastClient
                     Refresh_Announce_UnicastClient
                     Refresh_Syn_UnicastClient
                     ReSetSendDelayServer
                     ResetSendSignalRequest
                     Search_InServUnicastList
                     SetSendDelayServerEnable
                     SetServUnicast_AnnounceSig
                     SetServUnicast_DelayRespSig
                     SetServUnicast_SendDelayRequstEnable
                     SetServUnicast_SynSig
                     UnicastClient_PpsCheck
                     Update_ServUnicastList
                     WhetherAddressInClient
                     whether_InServUnicastList
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-2-2
                     ����: HuangFei
                     �޸�����: �´����ļ�


                   2.����: 2015-2-6
                     ����: HuangFei
                     �޸�����: ���Ӽ������е����������calculate����
*********************************************************************************************/

#include "unicast_process.h"
#include "comm_fun.h"
#include "net.h"
#include "ptp_io.h"

/********************************************************************************************
* ��������:    IsTwoStep
* ��������:    �ж��Ƿ���˫������˫������1�������򷵻�0
* �������:    ��
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-5
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Boolean IsTwoStep(MsgHeader *pMsgHeader)
{
    if(IS_SET(pMsgHeader->flagField0,1))
        return TRUE;
    else
        return FALSE;
}

/********************************************************************************************
* ��������:    Display_Netinfo
* ��������:    ��ӡ�������ò���
* �������:    netinfo *infopt  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/



/********************************************************************************************
* ��������:    Init_UnicastClient
* ��������:    ��ʼ����ģʽ�£������ͻ����б�
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-3
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Init_UnicastClient(PtpClock *pPtpClock)
{
    int i;
    UnicastClient *pUnicastClient = &pPtpClock->unicastClient;
    memset(pUnicastClient,0,sizeof(UnicastClient));

    if(pPtpClock->clockType == PTP_MASTER)
    {
        pUnicastClient->syn_index = 0;
        pUnicastClient->announce_index = 0;

        for(i = 0;i < MAX_CLIENT;i++)
        {
            if(!pPtpClock->UniNegotiationEnable)
            {
                pUnicastClient->clientList[i].send_announce_enable = TRUE;
                pUnicastClient->clientList[i].send_sync_enable = TRUE;
            }
            else 
            {
                pUnicastClient->clientList[i].send_announce_enable = FALSE;
                pUnicastClient->clientList[i].send_sync_enable = FALSE;
            }
        }
    }
    
}

/********************************************************************************************
* ��������:    UnicastClient_PpsCheck
* ��������:    ��ģʽ�£������ͻ���ÿ�����Ƿ���ڣ���������ڣ����������ȥ��
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-3
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void UnicastClient_PpsCheck(PtpClock *pPtpClock)
{
    
    int i;
    UnicastClient *pUnicastClient = &pPtpClock->unicastClient;

    for(i = 0;i < MAX_CLIENT;i++)
    {
        if(pUnicastClient->clientList[i].valid)
            pUnicastClient->clientList[i].client_counter++;
    }

    for(i = 0;i < MAX_CLIENT;i++)
    {
        if(pUnicastClient->clientList[i].valid)
        {
            if(pUnicastClient->clientList[i].client_counter >= pUnicastClient->clientList[i].durationField)
            {
                PLOG("===>One Client Lost !!\n");
                pUnicastClient->clientList[i].valid = FALSE;
                pUnicastClient->clientList[i].client_counter = 0;
            }
            
        }

    }
}

/********************************************************************************************
* ��������:    WhetherAddressInClient
* ��������:    �鿴�յ���Ϣ�Ƿ��� �ͻ����б���
* �������:    PtpClock *pPtpClock  
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-3
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
int WhetherAddressInClient(PtpClock *pPtpClock,void *addr)
{
    Uint32 ip;
    char mac[6];
    int i;
    UnicastClient* pUnicastClient = &pPtpClock->unicastClient;

    /**UDP IP ģʽ��  */
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        ip = *(Uint32*)addr;

        for(i = 0;i < MAX_CLIENT;i++)
        {
            if(pUnicastClient->clientList[i].valid)
            {
                if(ip == pUnicastClient->clientList[i].client_ip)
                    return i;
            }
                
        }
        return -1;

        
    }
    /** 802.3 ģʽ��  */
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        memcpy(mac,(char*)addr,6);

        for(i = 0;i < MAX_CLIENT;i++)
        {
            if(pUnicastClient->clientList[i].valid)
            {
                if(memcmp(pUnicastClient->clientList[i].client_mac,mac,6) == 0)
                    return i;
            }
                
        }
        return -1;
    }
    
    return -1;
}

/********************************************************************************************
* ��������:    RecvMsg_UpdateUnicastClient
* ��������:    ��ģʽ���յ��ͻ��ˣ�������ڿͻ��������µ��ͻ��ˣ�����ڿͻ���
               ����¼�����
* �������:    PtpClock *pPtpClock  
               void *addr           
               Uint32 duration      
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-3
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void RecvMsg_UpdateUnicastClient(PtpClock *pPtpClock,void *addr,Uint32 duration)
{
    Uint32 ip;
    char mac[6];
    UnicastClient* pUnicastClient = &pPtpClock->unicastClient;
    int i;
    int index;
    
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        ip = *(Uint32*)addr;
        if(ip == 0)
        {
             printf("===>Invalid Ip In, Discard it !!\n");
             return;
        }
        index = WhetherAddressInClient(pPtpClock,addr);

        if(index < 0)
        {
            /** ���б���Ѱ��û���õ��б�λ�� */
            for(i = 0;i < MAX_CLIENT;i++)
            {
                if(!pUnicastClient->clientList[i].valid)
                {
                    pUnicastClient->clientList[i].valid = TRUE;
                    pUnicastClient->clientList[i].client_ip = ip;
                    pUnicastClient->clientList[i].durationField = duration;
                   
                    
                    printf("%d.%d.%d.%d In\n"
                        ,(pPtpClock->unicastClient.clientList[i].client_ip&0xff)
                        ,(pPtpClock->unicastClient.clientList[i].client_ip>>8&0xff)
                        ,(pPtpClock->unicastClient.clientList[i].client_ip>>16&0xff)
                        ,(pPtpClock->unicastClient.clientList[i].client_ip>>24&0xff));
                    break;
                }
            }
        }
        else/**���б����ҵ������¼�����  */
        {
            pUnicastClient->clientList[index].client_counter = 0;
        }

        
    }
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        memcpy(mac,(char*)addr,6);

        index = WhetherAddressInClient(pPtpClock,addr);
        if(index < 0)
        {
            for(i = 0;i < MAX_CLIENT;i++)
            {
                if(!pUnicastClient->clientList[i].valid)
                {
                    pUnicastClient->clientList[i].valid = TRUE;
                    memcpy(pUnicastClient->clientList[i].client_mac,mac,6);
                    pUnicastClient->clientList[i].durationField = duration;
                    break;
                }
            }
        }
        else/**���б����ҵ������¼�����  */
        {
            pUnicastClient->clientList[index].client_counter = 0;
        }

        
    }

}

/********************************************************************************************
* ��������:    Check_ThreeMessageRecvAlarm
* ��������:    ÿ��20s���syn delayresp annonce ���Ƿ��յ������û���յ����򱨾�
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Check_ThreeMessageRecvAlarm(PtpClock *pPtpClock)
{
    pPtpClock->counters.checkRecvAlarm_Counter20s++;

    if(pPtpClock->counters.checkRecvAlarm_Counter20s > 20)/**20s ���һ�α���  */
    {
        pPtpClock->counters.checkRecvAlarm_Counter20s = 0;

        /** SYN �澯��� */
        if(pPtpClock->recvSynFlag == FALSE)
        {
            pPtpClock->synAlarm = TRUE;
        }
        else if(pPtpClock->recvSynFlag == TRUE)
        {
            pPtpClock->synAlarm = FALSE;
            pPtpClock->recvSynFlag = FALSE;
        }

        /** Announce �澯��� */
        if(pPtpClock->recvAnnounceFlag == FALSE)
        {
            pPtpClock->announceAlarm = TRUE;
        }
        else if(pPtpClock->recvAnnounceFlag == TRUE)
        {
            pPtpClock->announceAlarm = FALSE;
            pPtpClock->recvAnnounceFlag = FALSE;
        }

        /** delayresp �澯��� pDelayresp ͬ�������־*/
        if(pPtpClock->recvDelayRespFlag == FALSE)
        {
            pPtpClock->delayRespAlarm = TRUE;
        }
        else if(pPtpClock->recvDelayRespFlag == TRUE)
        {
            pPtpClock->delayRespAlarm = FALSE;
            pPtpClock->recvDelayRespFlag = FALSE;
        }

        if(pPtpClock->grandmasterClockQuality.clockClass == 52)
            pPtpClock->InclockClassAlarm = TRUE;
        else
            pPtpClock->InclockClassAlarm = FALSE;
        
        /**����澯���  */
        if(pPtpClock->delayRespAlarm == TRUE
            || pPtpClock->announceAlarm == TRUE
            || pPtpClock->synAlarm == TRUE
            || pPtpClock->grandmasterClockQuality.clockClass == 52)
        {
            pPtpClock->notAvailableAlarm = TRUE;
        }
        else
        {
            pPtpClock->notAvailableAlarm = FALSE;
        }

        /**���20s��û�յ�announce������һ��announce����ʱ�򣬸���best  */
        if(pPtpClock->announceAlarm == TRUE)
        {
            pPtpClock->InitBmcflag = TRUE;
            pPtpClock->BestMasterIp = 0;
            memset(pPtpClock->BestMasterMac,0,sizeof(pPtpClock->BestMasterMac));
        }
    }
}

/********************************************************************************************
* ��������:    CalculateConnectedClient
* ��������:    ���㵱ǰ��Ч�ͻ�������
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-3
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
int CalculateConnectedClient(PtpClock *pPtpClock)
{
    int i;
    int num = 0;

    for(i = 0;i< MAX_CLIENT;i++)
    {
        if(pPtpClock->unicastClient.clientList[i].valid)
            num++;
    }

    return num;
}

/********************************************************************************************
* ��������:    Display_PtpStatusEvery10s
* ��������:    10s��ӡһ�ε�ǰ����
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-3
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Display_PtpStatusEvery10s(PtpClock *pPtpClock)
{
    int i;

    pPtpClock->counters.displayStauts_Counter10s++;
    
    if(pPtpClock->counters.displayStauts_Counter10s > 35)/**35s ��ӡһ��  */
    {
        pPtpClock->counters.displayStauts_Counter10s = 0;
        
        if(pPtpClock->clockType == PTP_MASTER)
            printf("\n=========== PTP MASTER STATUS =============\n");
        else if(pPtpClock->clockType == PTP_SLAVE)
        {
            printf("\n=========== PTP SLAVE STATUS =============\n");
            
            /**������Ϣ  */
            printf("Syn Alarm : %d, Annoucne Alarm : %d, DelayResp Alarm : %d, OutPutAlarm : %d\n"
                ,pPtpClock->synAlarm
                ,pPtpClock->announceAlarm
                ,pPtpClock->delayRespAlarm
                ,pPtpClock->notAvailableAlarm);

        }
        
        Display_NetInfo(&pPtpClock->netEnviroment);
        /**�������ʱ�ӣ���Ч�ͻ�������  */
        if(pPtpClock->clockType == PTP_MASTER)
        {
            printf("Client Connecting Number [%d]\n",CalculateConnectedClient(pPtpClock));

            if(pPtpClock->protoType == PROTO_IEEE802)
            {
                for(i = 0;i < MAX_CLIENT;i++)
                {
                    if(pPtpClock->unicastClient.clientList[i].valid)
                    {                       
                        printf("%d:  %x:%x:%x:%x:%x:%x\n"
                            ,i
                            ,(pPtpClock->unicastClient.clientList[i].client_mac[0])
                            ,(pPtpClock->unicastClient.clientList[i].client_mac[1])
                            ,(pPtpClock->unicastClient.clientList[i].client_mac[2])
                            ,(pPtpClock->unicastClient.clientList[i].client_mac[3])
                            ,(pPtpClock->unicastClient.clientList[i].client_mac[4])
                            ,(pPtpClock->unicastClient.clientList[i].client_mac[5]));
                    }
                }

            }
            else if(pPtpClock->protoType == PROTO_UDP_IP)
            {
                for(i = 0;i < MAX_CLIENT;i++)
                {
                    if(pPtpClock->unicastClient.clientList[i].valid)
                    {
                        printf("%d:  %d.%d.%d.%d\n"
                            ,i
                            ,(pPtpClock->unicastClient.clientList[i].client_ip&0xff)
                            ,(pPtpClock->unicastClient.clientList[i].client_ip>>8&0xff)
                            ,(pPtpClock->unicastClient.clientList[i].client_ip>>16&0xff)
                            ,(pPtpClock->unicastClient.clientList[i].client_ip>>24&0xff));
                    }
                }

            }

        }
        /**����Ǵ�ʱ�ӣ���ӡѡ���������ʱ��  */
        if(pPtpClock->clockType == PTP_SLAVE)
        {
            if(pPtpClock->protoType == PROTO_IEEE802)
            {
                printf("BEST MASTER MAC: %x:%x:%x:%x:%x:%x\n"
                    ,pPtpClock->BestMasterMac[0]
                    ,pPtpClock->BestMasterMac[1]
                    ,pPtpClock->BestMasterMac[2]
                    ,pPtpClock->BestMasterMac[3]
                    ,pPtpClock->BestMasterMac[4]
                    ,pPtpClock->BestMasterMac[5]);
            }
            else if(pPtpClock->protoType == PROTO_UDP_IP)
            {
                printf("BEST MASTER Ip: %d:%d:%d:%d\n"
                    ,pPtpClock->BestMasterIp&0xff
                    ,(pPtpClock->BestMasterIp>>8)&0xff
                    ,(pPtpClock->BestMasterIp>>16)&0xff
                    ,(pPtpClock->BestMasterIp>>24)&0xff);

            }
        }

        printf("\n==========================================\n");
    }

}


/********************************************************************************************
* ��������:    GetClientUnicast_SynAddress
* ��������:    ��ʱ��ʱ������syn��ȡ�ͻ����б���ķ��͵�ַ
* �������:    PtpClock *pPtpClock  
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint8 GetClientUnicast_SynAddress(PtpClock *pPtpClock,void *addr)
{
    UnicastClient *pUnicastClient =  &pPtpClock->unicastClient;
    Uint8 validClient = CalculateConnectedClient(pPtpClock);
    
    if(pUnicastClient->syn_index> validClient)
        return FALSE;
    
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if((pUnicastClient->clientList[pUnicastClient->syn_index].valid == TRUE)
            &&(pUnicastClient->clientList[pUnicastClient->syn_index].send_sync_enable == TRUE))
        {
            
            *(Uint32*)addr = pUnicastClient->clientList[pUnicastClient->syn_index].client_ip;
            pUnicastClient->syn_index++;
            return TRUE;
        }
        else
        {
            pUnicastClient->syn_index++;
            return FALSE;
        }
    }
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if((pUnicastClient->clientList[pUnicastClient->syn_index].valid == TRUE)
            &&(pUnicastClient->clientList[pUnicastClient->syn_index].send_sync_enable == TRUE))
        {
            memcpy((char*)addr,pUnicastClient->clientList[pUnicastClient->syn_index].client_mac,6);
            pUnicastClient->syn_index++;
            return TRUE;
        }
        else
        {
            pUnicastClient->syn_index++;
            return FALSE;
        }
    }

    return FALSE;
}

/********************************************************************************************
* ��������:    GetClientUnicast_AnnounceAddress
* ��������:    ��ʱ��ʱ������announce��ȡ�ͻ����б���ķ��͵�ַ
* �������:    PtpClock *pPtpClock  
                             void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint8 GetClientUnicast_AnnounceAddress(PtpClock *pPtpClock,void *addr)
{
    UnicastClient *pUnicastClient =  &pPtpClock->unicastClient;
    Uint16 validClient = CalculateConnectedClient(pPtpClock);
    
    if(pUnicastClient->announce_index > validClient)
        return FALSE;
    
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if((pUnicastClient->clientList[pUnicastClient->announce_index].valid == TRUE)
            &&(pUnicastClient->clientList[pUnicastClient->announce_index].send_announce_enable == TRUE))
        {
            
            *(Uint32*)addr = pUnicastClient->clientList[pUnicastClient->announce_index].client_ip;
            pUnicastClient->announce_index++;
            return TRUE;
        }
        else
        {
            pUnicastClient->announce_index++;
            return FALSE;
        }
    }
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if((pUnicastClient->clientList[pUnicastClient->announce_index].valid == TRUE)
            &&(pUnicastClient->clientList[pUnicastClient->announce_index].send_announce_enable == TRUE))
        {
            memcpy((char*)addr,pUnicastClient->clientList[pUnicastClient->announce_index].client_mac,6);
            pUnicastClient->announce_index++;
            return TRUE;
        }
        else
        {
            pUnicastClient->announce_index++;
            return FALSE;
        }
    }

    return FALSE;
}



/********************************************************************************************
* ��������:    Refresh_Syn_UnicastClient
* ��������:    ��ʱ��ʱ������syn�����ۼӣ�����ƫ����
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Refresh_Syn_UnicastClient(PtpClock *pPtpClock)
{
    UnicastClient *pUnicastClient =  &pPtpClock->unicastClient;
    pUnicastClient->syn_index = 0;

    pPtpClock->sentSyncSequenceId++;
}

/********************************************************************************************
* ��������:    Refresh_Announce_UnicastClient
* ��������:    ��ʱ��ʱ������announce �����ۼӣ�����ƫ����
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Refresh_Announce_UnicastClient(PtpClock *pPtpClock)
{
    UnicastClient *pUnicastClient =  &pPtpClock->unicastClient;
    pUnicastClient->announce_index = 0;

    pPtpClock->sentAnnounceSequenceId++;
}



/********************************************************************************************
* ��������:    whether_InServUnicastList
* ��������:    �����յ��ĵ�ַ�Ƿ��ڷ������б���
* �������:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
               void *addr                           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint8 whether_InServUnicastList(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast,void *addr)
{
    int i = 0;
    Uint8 ret;
    
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        for(i = 0;i <pServer_unicast->validServerNum;i++)
        {
            if(pServer_unicast->serverList[i].serverIp == *(Uint32*)addr)
            {
                return TRUE;
            }
        }
        return FALSE;

    }
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        for(i = 0;i <pServer_unicast->validServerNum;i++)
        {
            if(memcmp(pServer_unicast->serverList[i].serverMac,(char*)addr,6) == 0)
            {
                return TRUE;
            }
        }
        
        return  FALSE;

    }
}


/********************************************************************************************
* ��������:    Search_InServUnicastList
* ��������:    ��ģʽ�£������յ��ĵ�ַ���Ƿ��ڵ�����������б���
                            �ҵ��򷵻�������
                            
* �������:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
               void *addr                           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
int Search_InServUnicastList(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast,void *addr)
{
    int i = 0;

    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        for(i = 0;i <pServer_unicast->validServerNum;i++)
        {
            if(pServer_unicast->serverList[i].serverIp == *(Uint32*)addr)
            {
                return i;
            }
        }
        
        return -1;

    }
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        for(i = 0;i <pServer_unicast->validServerNum;i++)
        {
            if(memcmp(pServer_unicast->serverList[i].serverMac,(char*)addr,6) == 0)
            {
                return i;
            }
        }
        
        return -1;

    }
}


/********************************************************************************************
* ��������:    SetServUnicast_AnnounceSig
* ��������:    �յ�signal announce�����ñ�־λ
* �������:    UnicastMultiServer *pServer_unicast  
               Uint16 index                         
               Uint8 val                            
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void SetServUnicast_AnnounceSig(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val)
{
    pServer_unicast->serverList[index].recv_sigann_flag = val;
}

/********************************************************************************************
* ��������:    SetServUnicast_SynSig
* ��������:    �յ�signal syn �����ñ�־λ
* �������:    UnicastMultiServer *pServer_unicast  
               Uint16 index                         
               Uint8 val                            
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void SetServUnicast_SynSig(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val)
{
    pServer_unicast->serverList[index].recv_sigsyn_flag = val;
}


void getServerUnicast_SynFreq(UnicastMultiServer *pServer_unicast,Uint16 index,Sint8 val)
{
    pServer_unicast->serverList[index].logInterMessagePeriod = val;

}

/********************************************************************************************
* ��������:    SetServUnicast_DelayRespSig
* ��������:    �յ�signal delayresp �������ñ�־λ
* �������:    UnicastMultiServer *pServer_unicast  
               Uint16 index                         
               Uint8 val                            
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void SetServUnicast_DelayRespSig(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val)
{
    pServer_unicast->serverList[index].recv_sigdela_flag = val;
}

/********************************************************************************************
* ��������:    SetServUnicast_SendDelayRequstEnable
* ��������:    ���ڵ���Э�̻��ƹص�ʱ�򣬿���ֱ���÷���delayreqʹ��
* �������:    UnicastMultiServer *pServer_unicast  
               Uint16 index                         
               Uint8 val                            
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void SetServUnicast_SendDelayRequstEnable(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val)
{
    pServer_unicast->serverList[index].send_delayreq_enable = val;
}

/********************************************************************************************
* ��������:    ReSetSendDelayServer
* ��������:    �÷���delayƫ��Ϊ0�������������ۼ�
* �������:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ReSetSendDelayServer(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast)
{
    pServer_unicast->delay_index = 0;
    
    if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
        pPtpClock->sentDelayReqSequenceId++;
    else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
        pPtpClock->sentPDelayReqSequenceId++;
}

/********************************************************************************************
* ��������:    ResetSendSignalRequest
* ��������:    ���ڵ����÷���signal��Ϣƫ�ƣ������������ۼ�
* �������:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ResetSendSignalRequest(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast)
{
    pServer_unicast->index = 0;
    pPtpClock->sentSignalSequenceId++;
}

/********************************************************************************************
* ��������:    SetSendDelayServerEnable
* ��������:    ����delayreq����ʹ��
* �������:    ServerUnicast *pServer_unicast  
               Uint16 delay_index              
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-1-28
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void SetSendDelayServerEnable(UnicastMultiServer *pServer_unicast,Uint16 delay_index)
{
    pServer_unicast->serverList[delay_index].send_delayreq_enable = TRUE;
}

/********************************************************************************************
* ��������:    CheckSendDelayServerEnable
* ��������:    ��ⷢ��delay�Ƿ�ʹ�ܣ����ڵ����������ģʽ������delayreq����
* �������:    UnicastMultiServer *pServer_unicast  
                             Uint16 delay_index                   
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void CheckSendDelayServerEnable(UnicastMultiServer *pServer_unicast,Uint16 delay_index)
{
    if((pServer_unicast->serverList[delay_index].sig_alarm == TRUE)
        || pServer_unicast->serverList[delay_index].finishT1T2_flag == FALSE)
    {
        pServer_unicast->serverList[delay_index].send_delayreq_enable = FALSE;
    }
    else
    {
        pServer_unicast->serverList[delay_index].send_delayreq_enable = TRUE;
        pServer_unicast->serverList[delay_index].finishT1T2_flag = FALSE;

    }
}

/********************************************************************************************
* ��������:    GetSendDelayServer_Ip
* ��������:    ��ȡ����delayreq��IP��ַ
* �������:    ServerUnicast *pServer_unicast  
               Uint32 *pServerIp               
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-1-28
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint8 GetSendDelayServer_Ip(UnicastMultiServer *pServer_unicast,Uint32 *pServerIp)
{
    if(pServer_unicast->validServerNum == 0)
        return FALSE;
    
    if(pServer_unicast->delay_index >= pServer_unicast->validServerNum)
    {
        return FALSE;
    }
    if(pServer_unicast->serverList[pServer_unicast->delay_index].send_delayreq_enable == FALSE)
    {
        pServer_unicast->delay_index++;
        return FALSE;
    }

    if(pServer_unicast->serverList[pServer_unicast->delay_index].valid == FALSE)
    {
        pServer_unicast->delay_index++;
        return FALSE;

    }

    *pServerIp = pServer_unicast->serverList[pServer_unicast->delay_index].serverIp;
    pServer_unicast->delay_index++;


    return TRUE;
}

/********************************************************************************************
* ��������:    GetSendDelayServer_Mac
* ��������:    ��ȡ����delayreq��MAC��ַ
* �������:    ServerUnicast *pServer_unicast  
               Uint8 *mac                      
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-1-28
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint8 GetSendDelayServer_Mac(UnicastMultiServer *pServer_unicast,Uint8 *mac)
{
    if(pServer_unicast->validServerNum == 0)
        return FALSE;
    
    if(pServer_unicast->delay_index >= pServer_unicast->validServerNum)
    {
        return FALSE;
    }
    if(pServer_unicast->serverList[pServer_unicast->delay_index].send_delayreq_enable == FALSE)
    {
        pServer_unicast->delay_index++;
        return FALSE;
    }

    if(pServer_unicast->serverList[pServer_unicast->delay_index].valid == FALSE)
    {
        pServer_unicast->delay_index++;
        return FALSE;

    }
    memcpy(mac,pServer_unicast->serverList[pServer_unicast->delay_index].serverMac,6);
    pServer_unicast->delay_index++;
    return TRUE;
}

void StoreT3InBestAndUnicastlist(PtpClock *pPtpClock,TimeInternal T3,void *addr)
{
    int index;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;
    
    index = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
    
    if(index < 0)
        return;

    pServer_unicast->serverList[index].T3 = T3;

    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->BestMasterIp == *(Uint32*)addr)
            pPtpClock->T3 = T3;
    }
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(memcmp(pPtpClock->BestMasterMac,(char*)addr,6) == 0)
            pPtpClock->T3 = T3;
    }
}

/********************************************************************************************
* ��������:    GetMultiServ_SendServerIpAndMsgType
* ��������:    �õ�������������������Լ������ַ
* �������:    ServerUnicast *pServer_unicast  
               Uint32 *pServerIp               
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-1-28
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint8 GetMultiServ_SendServerAddress_And_MsgType(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast,void *pAddr)
{  

    if(pAddr == NULL)
    {
        printf("input Address is Null. error !!\n");
        return SIGNAL_MSG_NULL;
    }

    if(pPtpClock->counters.sendControl_index >= 4)
    {
        pPtpClock->counters.sendControl_index = 0;
        
    }
    pPtpClock->counters.sendControl_index++;

    /**�����Ч������ipΪ0����������  */
    if(pServer_unicast->validServerNum == 0)
        return SIGNAL_MSG_NULL;

    /**udp ip  */
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pServer_unicast->index < pServer_unicast->validServerNum)
        {
            if(pServer_unicast->serverList[pServer_unicast->index].recv_sigsyn_flag == FALSE 
                && (pPtpClock->counters.sendControl_index == 1) && (pServer_unicast->serverList[pServer_unicast->index].valid == TRUE))
            {
                *(Uint32*)pAddr = pServer_unicast->serverList[pServer_unicast->index].serverIp;
                return SIGNAL_SYN_MSG;
            }

            else if(pServer_unicast->serverList[pServer_unicast->index].recv_sigann_flag == FALSE 
                && (pPtpClock->counters.sendControl_index == 2) && (pServer_unicast->serverList[pServer_unicast->index].valid == TRUE))
            {
                *(Uint32*)pAddr = pServer_unicast->serverList[pServer_unicast->index].serverIp;
                return SIGNAL_ANNOUNCE_MSG;
            }

            else if(pServer_unicast->serverList[pServer_unicast->index].recv_sigdela_flag == FALSE 
                && (pPtpClock->counters.sendControl_index == 3) && (pServer_unicast->serverList[pServer_unicast->index].valid == TRUE))
            {
                *(Uint32*)pAddr = pServer_unicast->serverList[pServer_unicast->index].serverIp;
                pServer_unicast->index++;
                return SIGNAL_DELAYRESP_MSG;
            }   
            else if(pPtpClock->counters.sendControl_index == 3)
            {
                pServer_unicast->index++;
            }
            
            return SIGNAL_MSG_NULL;
        }

        /*������յ����������б������*/
         return SIGNAL_MSG_NULL;

    }

    /**ieee802.3  */
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pServer_unicast->index < pServer_unicast->validServerNum)
        {
            if(pServer_unicast->serverList[pServer_unicast->index].recv_sigsyn_flag == FALSE 
                && (pPtpClock->counters.sendControl_index == 1) && (pServer_unicast->serverList[pServer_unicast->index].valid == TRUE))
            {
                memcpy((char*)pAddr,pServer_unicast->serverList[pServer_unicast->index].serverMac,6);
                return SIGNAL_SYN_MSG;
            }
            
            if(pServer_unicast->serverList[pServer_unicast->index].recv_sigann_flag == FALSE 
                && (pPtpClock->counters.sendControl_index == 2) && (pServer_unicast->serverList[pServer_unicast->index].valid == TRUE))
            {
                memcpy((char*)pAddr,pServer_unicast->serverList[pServer_unicast->index].serverMac,6);
                return SIGNAL_ANNOUNCE_MSG;
            }
            
            if(pServer_unicast->serverList[pServer_unicast->index].recv_sigdela_flag == FALSE 
                && (pPtpClock->counters.sendControl_index == 3) && (pServer_unicast->serverList[pServer_unicast->index].valid == TRUE))
            {
                memcpy((char*)pAddr,pServer_unicast->serverList[pServer_unicast->index].serverMac,6);
                pServer_unicast->index++;
                
                return SIGNAL_DELAYRESP_MSG;
            }
            else if(pPtpClock->counters.sendControl_index == 3)
            {
                pServer_unicast->index++;
            }

            return SIGNAL_MSG_NULL;
        }

        /*������յ����������б������*/
        return SIGNAL_MSG_NULL;

    }

}

/********************************************************************************************
* ��������:    Update_ServUnicastList
* ��������:    ���ڼ��signal�յ����������յ���signal����ȡ�������δ�յ���sig-
               nal������
* �������:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Update_ServUnicastList(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast)
{
    int i = 0;
    pPtpClock->counters.signalDuration_index++;

    /**�ϻ�ʱ�䵽�ˣ� ����signal ��־ */
    if(pPtpClock->UnicastDuration <= pPtpClock->counters.signalDuration_index)
    {
        pPtpClock->counters.signalDuration_index = 0;
        
        /** ÿ�ϻ�ʱ��signal ������� */
        for(i = 0;i < MAX_SERVER;i++)
        {
            if((pServer_unicast->serverList[i].recv_sigann_flag == FALSE)
                ||(pServer_unicast->serverList[i].recv_sigsyn_flag == FALSE)
                ||(pServer_unicast->serverList[i].recv_sigdela_flag == FALSE))
            {
                pServer_unicast->serverList[i].sig_alarm = TRUE;
            }
            else
            {
                pServer_unicast->serverList[i].sig_alarm = FALSE;
            }
        
        }

        for(i = 0;i < MAX_SERVER;i++)
        {
            pServer_unicast->serverList[i].recv_sigann_flag = FALSE;
            pServer_unicast->serverList[i].recv_sigsyn_flag = FALSE;
            pServer_unicast->serverList[i].recv_sigdela_flag = FALSE;
            //pServer_unicast->serverList[i].sig_alarm = TRUE; 
        }


        /** ���ʱ�� */
        for(i = 0;i < MAX_SERVER;i++)
        {
            memset(&pServer_unicast->serverList[i].pathMeanDelay,0,sizeof(TimeInternal));
            memset(&pServer_unicast->serverList[i].timeOffset,0,sizeof(TimeInternal));
            pServer_unicast->serverList[i].finishT1T2_flag = 0;
        }
    }

}

void Caculate_SynMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgSync *pMsgSyn,void *addr)
{
    int index;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;
    
    index = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
    
    if(index < 0)
        return;

    
    if(!IsTwoStep(pMsgHeader))
        pServer_unicast->serverList[index].finishT1T2_flag = TRUE;
}



void Caculate_DelayRespMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgDelayResp *pMsgDelayResp,void *addr)
{
    Uint16 index;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;
    
    index = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
    
}

/********************************************************************************************
* ��������:    Caculate_PdelayRespMsgTime
* ��������:    �յ�PdelayResp����ʱ�򣬴洢ʱ��T4����ֵ��־���ú���ֻ���ڻ�ȡʱ��
               ��������Уʱ��
               
* �������:    PtpClock *pPtpClock            
               MsgHeader *pMsgHeader          
               MsgPDelayResp *pMsgPDelayResp  
               void *addr                     
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Caculate_PdelayRespMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgPDelayResp *pMsgPDelayResp,void *addr)
{
    int index;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;
    index = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);

    if(index < 0)
        return;

}

/********************************************************************************************
* ��������:    Caculate_FollowUpMsgTime
* ��������:    �յ�followUp����ʱ�򣬴洢ʱ��T2���ú���ֻ���ڻ�ȡʱ��
               ��������Уʱ��
               
* �������:    PtpClock *pPtpClock     
               MsgHeader *pMsgHeader   
               MsgFollowUp *pFollowUp  
               void *addr              
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Caculate_FollowUpMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgFollowUp *pFollowUp,void *addr)
{
    int index;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;

    index = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);

    if(index < 0)
        return;

    
    if(pPtpClock->stepType == TWO_STEP)
        pServer_unicast->serverList[index].finishT1T2_flag = TRUE;
}

/********************************************************************************************
* ��������:    Caculate_PdelayFollowUpMsgTime
* ��������:    �յ�PdelayfollowUp��Ϣʱ���洢T4 T5 ���ñ�־���ú���ֻ���ڻ�ȡʱ��
               ��������Уʱ��
* �������:    PtpClock *pPtpClock                            
               MsgHeader *pMsgHeader                          
               MsgPDelayRespFollowUp *pMsgPDelayRespFollowUp  
               void *addr                                     
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Caculate_PdelayFollowUpMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgPDelayRespFollowUp *pMsgPDelayRespFollowUp,void *addr)
{
    int index;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;
    
    index = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);

    if(index < 0)
        return;
    
    if(pPtpClock->stepType == TWO_STEP);
        

}


