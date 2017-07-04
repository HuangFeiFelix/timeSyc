/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       unicast_process.h
*    功能描述:       单播相关处理
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-2-2
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-2-2
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/


#ifndef __UNICAST_PROCESS_H__
#define __UNICAST_PROCESS_H__

#include "common.h"
#include "data_type.h"

extern Boolean IsTwoStep(MsgHeader *pMsgHeader);

extern void getServerUnicast_SynFreq(UnicastMultiServer *pServer_unicast,Uint16 index,Sint8 val);

extern void Init_UnicastClient(PtpClock *pPtpClock);
extern void UnicastClient_PpsCheck(PtpClock *pPtpClock);
extern int WhetherAddressInClient(PtpClock *pPtpClock,void *addr);
extern int CalculateConnectedClient(PtpClock *pPtpClock);
extern void Check_ThreeMessageRecvAlarm(PtpClock *pPtpClock);
extern void Display_PtpStatusEvery10s(PtpClock *pPtpClock);
extern void RecvMsg_UpdateUnicastClient(PtpClock *pPtpClock,void *addr,Uint32 duration);

extern Uint8 GetClientUnicast_SynAddress(PtpClock *pPtpClock,void *addr);
extern Uint8 GetClientUnicast_AnnounceAddress(PtpClock *pPtpClock,void *addr);

extern void Refresh_Syn_UnicastClient(PtpClock *pPtpClock);
extern void Refresh_Announce_UnicastClient(PtpClock *pPtpClock);



extern Uint8 GetSendDelayServer_Ip(UnicastMultiServer *pServer_unicast,Uint32 *pServerIp);
extern Uint8 GetSendDelayServer_Mac(UnicastMultiServer *pServer_unicast,Uint8 *mac);
extern void SetSendDelayServerEnable(UnicastMultiServer *pServer_unicast,Uint16 delay_index);

extern void ResetSendSignalRequest(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast);
extern void ReSetSendDelayServer(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast);

extern Uint8 GetMultiServ_SendServerAddress_And_MsgType(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast,void *pAddr);


extern void SetServUnicast_AnnounceSig(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val);
extern void SetServUnicast_SynSig(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val);
extern void SetServUnicast_DelayRespSig(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val);
extern void SetServUnicast_SendDelayRequstEnable(UnicastMultiServer *pServer_unicast,Uint16 index,Uint8 val);
extern void CheckSendDelayServerEnable(UnicastMultiServer *pServer_unicast,Uint16 delay_index);
extern Uint8 whether_InServUnicastList(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast,void *addr);
extern int Search_InServUnicastList(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast,void *addr);

extern void StoreT3InBestAndUnicastlist(PtpClock *pPtpClock,TimeInternal T3,void *addr);

extern void Update_ServUnicastList(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast);

extern void Caculate_SynMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgSync *pMsgSyn,void *addr);
extern void Caculate_DelayRespMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgDelayResp *pMsgDelayResp,void *addr);
extern void Caculate_PdelayRespMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgPDelayResp *pMsgPDelayResp,void *addr);
extern void Caculate_FollowUpMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgFollowUp *pFollowUp,void *addr);
extern void Caculate_PdelayFollowUpMsgTime(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgPDelayRespFollowUp *pMsgPDelayRespFollowUp,void *addr);


#endif



