/********************************************************************************************
*                           ��Ȩ����(C) 2015,*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       msg.h
*    ��������:       ������Ϣ������װ��Ϣ���������Ϣ������װͷ�����ͷ��
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

#ifndef _MSG_H_
#define _MSG_H_

#include "common.h"
#include "data_type.h"


extern void copyClockIdentity(ClockIdentity dest, ClockIdentity src);

extern void msgUnpackHeader(Octet * buf, MsgHeader * header);
extern void msgUnpackSync(Octet * buf, MsgSync * sync);
extern void msgUnpackFollowUp(Octet * buf, MsgFollowUp * follow);
extern void msgUnpackAnnounce(Octet * buf, MsgAnnounce * announce);
extern void msgUnpackDelayReq(Octet * buf, MsgDelayReq * delayreq);
extern void msgUnpackPDelayReq(Octet * buf, MsgPDelayReq * pdelayreq);
extern void msgUnpackDelayResp(Octet * buf, MsgDelayResp * resp);
extern void msgUnpackPDelayResp(Octet * buf, MsgPDelayResp * presp);
extern void msgUnpackPDelayRespFollowUp(Octet * buf, MsgPDelayRespFollowUp * prespfollow);

extern void msgUnpackRequestSignal(Octet * buf, SignalReq* signal);
extern void msgUnpackGrantSignal(Octet * buf, SignalGrant* signal);
extern void msgUnpackCancelSignal(Octet * buf, SignalCancel* signal);
extern void msgUnpackAckCancelSignal(Octet * buf, SignalAckCancel* signal);


extern void issueGrantSignaling(PtpClock *pPtpClock,Uint8 signalType,MsgHeader *pMsgHeader,SignalReq *pSignalReq,void *addr);
extern void issueCancelSignaling(PtpClock *pPtpClock,Uint8 signalType,MsgHeader *pMsgHeader,SignalReq *pSignalReq,void *pAddr);
extern void issueRequestSignaling(PtpClock *pPtpClock);
extern void issueAckCancelSignaling(PtpClock *pPtpClock,Uint8 signalType,MsgHeader *pMsgHeader,void *pAddr);

extern void issueSync(PtpClock *pPtpClock);
extern void issueAnnounce (PtpClock *pPtpClock);
extern void issueFollowup(const TimeInternal *tint,PtpClock *pPtpClock, UInteger16 sequenceId);
extern void issueDelayReq(PtpClock *pPtpClock);
extern void issuePDelayReq(PtpClock *pPtpClock);
extern void issueDelayResp(const TimeInternal *tint,MsgHeader *header,PtpClock *ptpClock,void *addr);
extern void issuePDelayResp(const TimeInternal *tint,MsgHeader *header,PtpClock *pPtpClock,void *addr);
extern void issuePDelayRespFollowUp(const TimeInternal *requetReceipt,const TimeInternal *responseOrigin,MsgHeader *header,PtpClock *pPtpClock, UInteger16 sequenceId,void *pAddr);

extern void msgPackSync(Octet * buf, Timestamp* originTimestamp, PtpClock* ptpClock);
extern void msgPackHeader(Octet * buf, PtpClock * ptpClock);
extern void msgPackDelayResp(Octet * buf, MsgHeader * header, Timestamp * receiveTimestamp, PtpClock * ptpClock);
extern void msgPackPDelayResp(Octet * buf, MsgHeader * header, Timestamp * requestReceiptTimestamp, PtpClock * ptpClock);

extern void msgPackPDelayRespFollowUp(Octet * buf, MsgHeader * header,const TimeInternal *requetReceipt,Timestamp * responseOriginTimestamp,PtpClock * ptpClock, const UInteger16 sequenceId);


extern void msgPackSignaling_Request(Octet * buf,UInteger8 MessageType,PtpClock * ptpClock);
extern void msgPackSignaling_Grant(Octet * buf,UInteger8 MessageType,Integer8 loginterval,PtpClock * ptpClock,const UInteger16 sequenceId,const Uint32 duration);
extern void msgPackSignaling_Cancel(Octet * buf,UInteger8 MessageType,PtpClock * ptpClock,const UInteger16 sequenceId);
extern void msgPackSignaling_AckCancel(Octet * buf,UInteger8 MessageType,PtpClock * ptpClock,const UInteger16 sequenceId);


#endif

