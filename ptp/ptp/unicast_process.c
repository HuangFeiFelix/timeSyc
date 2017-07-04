/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       unicast_process.c
*    功能描述:       单播相关处理，包括单播协商，单播更新，单播服务器列表更新等
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-2-2
*    函数列表:
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
*    备注:            
*    修改历史:
                   1.日期: 2015-2-2
                     作者: HuangFei
                     修改内容: 新创建文件


                   2.日期: 2015-2-6
                     作者: HuangFei
                     修改内容: 增加计算所有单播多服务器calculate函数
*********************************************************************************************/

#include "unicast_process.h"
#include "comm_fun.h"
#include "net.h"
#include "ptp_io.h"

/********************************************************************************************
* 函数名称:    IsTwoStep
* 功能描述:    判断是否是双步，是双步返回1，不是则返回0
* 输入参数:    无
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-5
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Boolean IsTwoStep(MsgHeader *pMsgHeader)
{
    if(IS_SET(pMsgHeader->flagField0,1))
        return TRUE;
    else
        return FALSE;
}

/********************************************************************************************
* 函数名称:    Display_Netinfo
* 功能描述:    打印网络配置参数
* 输入参数:    netinfo *infopt  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/



/********************************************************************************************
* 函数名称:    Init_UnicastClient
* 功能描述:    初始化主模式下，单播客户端列表
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-3
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    UnicastClient_PpsCheck
* 功能描述:    主模式下，单播客户端每秒检测是否存在，如果不存在，则从链表中去掉
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-3
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    WhetherAddressInClient
* 功能描述:    查看收到消息是否在 客户端列表中
* 输入参数:    PtpClock *pPtpClock  
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-3
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
int WhetherAddressInClient(PtpClock *pPtpClock,void *addr)
{
    Uint32 ip;
    char mac[6];
    int i;
    UnicastClient* pUnicastClient = &pPtpClock->unicastClient;

    /**UDP IP 模式下  */
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
    /** 802.3 模式下  */
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
* 函数名称:    RecvMsg_UpdateUnicastClient
* 功能描述:    主模式下收到客户端，如果不在客户端里，则更新到客户端，如果在客户端
               则更新计数器
* 输入参数:    PtpClock *pPtpClock  
               void *addr           
               Uint32 duration      
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-3
               作者:     HuangFei
               修改内容: 新生成函数

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
            /** 在列表里寻找没有用的列表位置 */
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
        else/**在列表里找到，更新计数器  */
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
        else/**在列表里找到，更新计数器  */
        {
            pUnicastClient->clientList[index].client_counter = 0;
        }

        
    }

}

/********************************************************************************************
* 函数名称:    Check_ThreeMessageRecvAlarm
* 功能描述:    每隔20s检查syn delayresp annonce 包是否收到，如果没有收到，则报警
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Check_ThreeMessageRecvAlarm(PtpClock *pPtpClock)
{
    pPtpClock->counters.checkRecvAlarm_Counter20s++;

    if(pPtpClock->counters.checkRecvAlarm_Counter20s > 20)/**20s 检测一次报警  */
    {
        pPtpClock->counters.checkRecvAlarm_Counter20s = 0;

        /** SYN 告警检测 */
        if(pPtpClock->recvSynFlag == FALSE)
        {
            pPtpClock->synAlarm = TRUE;
        }
        else if(pPtpClock->recvSynFlag == TRUE)
        {
            pPtpClock->synAlarm = FALSE;
            pPtpClock->recvSynFlag = FALSE;
        }

        /** Announce 告警检测 */
        if(pPtpClock->recvAnnounceFlag == FALSE)
        {
            pPtpClock->announceAlarm = TRUE;
        }
        else if(pPtpClock->recvAnnounceFlag == TRUE)
        {
            pPtpClock->announceAlarm = FALSE;
            pPtpClock->recvAnnounceFlag = FALSE;
        }

        /** delayresp 告警检测 pDelayresp 同是这个标志*/
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
        
        /**输出告警检测  */
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

        /**如果20s都没收到announce，则下一个announce来的时候，更新best  */
        if(pPtpClock->announceAlarm == TRUE)
        {
            pPtpClock->InitBmcflag = TRUE;
            pPtpClock->BestMasterIp = 0;
            memset(pPtpClock->BestMasterMac,0,sizeof(pPtpClock->BestMasterMac));
        }
    }
}

/********************************************************************************************
* 函数名称:    CalculateConnectedClient
* 功能描述:    计算当前有效客户端数量
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-3
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    Display_PtpStatusEvery10s
* 功能描述:    10s打印一次当前参数
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-3
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Display_PtpStatusEvery10s(PtpClock *pPtpClock)
{
    int i;

    pPtpClock->counters.displayStauts_Counter10s++;
    
    if(pPtpClock->counters.displayStauts_Counter10s > 35)/**35s 打印一次  */
    {
        pPtpClock->counters.displayStauts_Counter10s = 0;
        
        if(pPtpClock->clockType == PTP_MASTER)
            printf("\n=========== PTP MASTER STATUS =============\n");
        else if(pPtpClock->clockType == PTP_SLAVE)
        {
            printf("\n=========== PTP SLAVE STATUS =============\n");
            
            /**报警信息  */
            printf("Syn Alarm : %d, Annoucne Alarm : %d, DelayResp Alarm : %d, OutPutAlarm : %d\n"
                ,pPtpClock->synAlarm
                ,pPtpClock->announceAlarm
                ,pPtpClock->delayRespAlarm
                ,pPtpClock->notAvailableAlarm);

        }
        
        Display_NetInfo(&pPtpClock->netEnviroment);
        /**如果是主时钟，有效客户端数量  */
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
        /**如果是从时钟，打印选出的最佳主时钟  */
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
* 函数名称:    GetClientUnicast_SynAddress
* 功能描述:    主时钟时，发送syn获取客户端列表里的发送地址
* 输入参数:    PtpClock *pPtpClock  
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    GetClientUnicast_AnnounceAddress
* 功能描述:    主时钟时，发送announce获取客户端列表里的发送地址
* 输入参数:    PtpClock *pPtpClock  
                             void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    Refresh_Syn_UnicastClient
* 功能描述:    主时钟时，发送syn序列累加，并置偏移量
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Refresh_Syn_UnicastClient(PtpClock *pPtpClock)
{
    UnicastClient *pUnicastClient =  &pPtpClock->unicastClient;
    pUnicastClient->syn_index = 0;

    pPtpClock->sentSyncSequenceId++;
}

/********************************************************************************************
* 函数名称:    Refresh_Announce_UnicastClient
* 功能描述:    主时钟时，发送announce 序列累加，并置偏移量
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Refresh_Announce_UnicastClient(PtpClock *pPtpClock)
{
    UnicastClient *pUnicastClient =  &pPtpClock->unicastClient;
    pUnicastClient->announce_index = 0;

    pPtpClock->sentAnnounceSequenceId++;
}



/********************************************************************************************
* 函数名称:    whether_InServUnicastList
* 功能描述:    搜索收到的地址是否在服务器列表里
* 输入参数:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
               void *addr                           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    Search_InServUnicastList
* 功能描述:    从模式下，搜索收到的地址，是否在单播多服务器列表里
                            找到则返回索引号
                            
* 输入参数:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
               void *addr                           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    SetServUnicast_AnnounceSig
* 功能描述:    收到signal announce包，置标志位
* 输入参数:    UnicastMultiServer *pServer_unicast  
               Uint16 index                         
               Uint8 val                            
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void SetServUnicast_AnnounceSig(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val)
{
    pServer_unicast->serverList[index].recv_sigann_flag = val;
}

/********************************************************************************************
* 函数名称:    SetServUnicast_SynSig
* 功能描述:    收到signal syn 包，置标志位
* 输入参数:    UnicastMultiServer *pServer_unicast  
               Uint16 index                         
               Uint8 val                            
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    SetServUnicast_DelayRespSig
* 功能描述:    收到signal delayresp 包，则置标志位
* 输入参数:    UnicastMultiServer *pServer_unicast  
               Uint16 index                         
               Uint8 val                            
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void SetServUnicast_DelayRespSig(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val)
{
    pServer_unicast->serverList[index].recv_sigdela_flag = val;
}

/********************************************************************************************
* 函数名称:    SetServUnicast_SendDelayRequstEnable
* 功能描述:    当在单播协商机制关的时候，可以直接置发送delayreq使能
* 输入参数:    UnicastMultiServer *pServer_unicast  
               Uint16 index                         
               Uint8 val                            
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void SetServUnicast_SendDelayRequstEnable(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val)
{
    pServer_unicast->serverList[index].send_delayreq_enable = val;
}

/********************************************************************************************
* 函数名称:    ReSetSendDelayServer
* 功能描述:    置发送delay偏移为0，并发送序列累加
* 输入参数:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    ResetSendSignalRequest
* 功能描述:    周期到，置发送signal消息偏移，并发送序列累加
* 输入参数:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void ResetSendSignalRequest(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast)
{
    pServer_unicast->index = 0;
    pPtpClock->sentSignalSequenceId++;
}

/********************************************************************************************
* 函数名称:    SetSendDelayServerEnable
* 功能描述:    设置delayreq发送使能
* 输入参数:    ServerUnicast *pServer_unicast  
               Uint16 delay_index              
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-28
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void SetSendDelayServerEnable(UnicastMultiServer *pServer_unicast,Uint16 delay_index)
{
    pServer_unicast->serverList[delay_index].send_delayreq_enable = TRUE;
}

/********************************************************************************************
* 函数名称:    CheckSendDelayServerEnable
* 功能描述:    检测发送delay是否使能，用于单播多服务器模式，控制delayreq发送
* 输入参数:    UnicastMultiServer *pServer_unicast  
                             Uint16 delay_index                   
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    GetSendDelayServer_Ip
* 功能描述:    获取发送delayreq的IP地址
* 输入参数:    ServerUnicast *pServer_unicast  
               Uint32 *pServerIp               
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-28
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    GetSendDelayServer_Mac
* 功能描述:    获取发送delayreq的MAC地址
* 输入参数:    ServerUnicast *pServer_unicast  
               Uint8 *mac                      
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-28
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    GetMultiServ_SendServerIpAndMsgType
* 功能描述:    得到发送信令的信令类型以及服务地址
* 输入参数:    ServerUnicast *pServer_unicast  
               Uint32 *pServerIp               
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-1-28
               作者:     HuangFei
               修改内容: 新生成函数

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

    /**如果有效服务器ip为0，立即返回  */
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

        /*满足的收到所有配置列表服务器*/
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

        /*满足的收到所有配置列表服务器*/
        return SIGNAL_MSG_NULL;

    }

}

/********************************************************************************************
* 函数名称:    Update_ServUnicastList
* 功能描述:    定期检测signal收到情况，如果收到则signal报警取消，如果未收到则sig-
               nal报警开
* 输入参数:    PtpClock *pPtpClock                  
               UnicastMultiServer *pServer_unicast  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void Update_ServUnicastList(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast)
{
    int i = 0;
    pPtpClock->counters.signalDuration_index++;

    /**老化时间到了， 更新signal 标志 */
    if(pPtpClock->UnicastDuration <= pPtpClock->counters.signalDuration_index)
    {
        pPtpClock->counters.signalDuration_index = 0;
        
        /** 每老化时间signal 报警标记 */
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


        /** 清空时间 */
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
* 函数名称:    Caculate_PdelayRespMsgTime
* 功能描述:    收到PdelayResp包的时候，存储时间T4，并值标志，该函数只用于获取时间
               ，不用于校时。
               
* 输入参数:    PtpClock *pPtpClock            
               MsgHeader *pMsgHeader          
               MsgPDelayResp *pMsgPDelayResp  
               void *addr                     
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    Caculate_FollowUpMsgTime
* 功能描述:    收到followUp包的时候，存储时间T2，该函数只用于获取时间
               ，不用于校时。
               
* 输入参数:    PtpClock *pPtpClock     
               MsgHeader *pMsgHeader   
               MsgFollowUp *pFollowUp  
               void *addr              
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    Caculate_PdelayFollowUpMsgTime
* 功能描述:    收到PdelayfollowUp消息时，存储T4 T5 ，置标志，该函数只用于获取时间
               ，不用于校时。
* 输入参数:    PtpClock *pPtpClock                            
               MsgHeader *pMsgHeader                          
               MsgPDelayRespFollowUp *pMsgPDelayRespFollowUp  
               void *addr                                     
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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


