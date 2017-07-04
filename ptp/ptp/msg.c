/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       msg.c
*    功能描述:       发送消息包，组装消息包，解拆消息包，组装头，解拆头等
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-30
*    函数列表:
                     copyClockIdentity
                     DbDebug
                     issueAckCancelSignaling
                     issueAnnounce
                     issueCancelSignaling
                     issueDelayReq
                     issueDelayResp
                     issueFollowup
                     issueGrantSignaling
                     issuePDelayReq
                     issuePDelayResp
                     issuePDelayRespFollowUp
                     issueRequestSignaling
                     issueSync
                     msgPackAnnounce
                     msgPackDelayReq
                     msgPackDelayResp
                     msgPackFollowUp
                     msgPackHeader
                     msgPackPDelayReq
                     msgPackPDelayResp
                     msgPackPDelayRespFollowUp
                     msgPackSignaling_AckCancel
                     msgPackSignaling_Cancel
                     msgPackSignaling_Grant
                     msgPackSignaling_Request
                     msgPackSync
                     msgUnpackAckCancelSignal
                     msgUnpackAnnounce
                     msgUnpackCancelSignal
                     msgUnpackDelayReq
                     msgUnpackDelayResp
                     msgUnpackFollowUp
                     msgUnpackGrantSignal
                     msgUnpackHeader
                     msgUnpackPDelayReq
                     msgUnpackPDelayResp
                     msgUnpackPDelayRespFollowUp
                     msgUnpackRequestSignal
                     msgUnpackSync
                     netInitIeeeAddress
                     netSetMulticastTTL
                     respectUtcOffset
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-30
                     作者: HuangFei
                     修改内容: 新创建文件

                   2.日期: 2015-2-6
                     作者: HuangFei
                     修改内容: 增加issue函数

*********************************************************************************************/

#include "msg.h"
#include "ptp.h"
#include "comm_fun.h"

#include "arith.h"
#include "unicast_process.h"
#include "fpga_time.h"



Boolean respectUtcOffset(PtpClock * ptpClock) {
	if (ptpClock->timePropertiesDS.currentUtcOffsetValid) {
		return TRUE;
	}
	return FALSE;
}


void copyClockIdentity(ClockIdentity dest, ClockIdentity src)
{
	memcpy(dest, src, CLOCK_IDENTITY_LENGTH);
}

void Input_TimeStamp(Octet * buf,Timestamp * receiveTimestamp)
{
    int i;
    for(i=0;i<=3;i++)
    {
        buf[36+i] = (receiveTimestamp->secondsField.lsb>>(3-i)*8)&0xff;
        buf[40+i] = (receiveTimestamp->nanosecondsField>>(3-i)*8)&0xff;
    }
}

void Input_Correction(Octet * buf,Integer64 *headCorrection)
{
    int i;
    for(i=0;i<=3;i++)
    {
        buf[8+i] = (headCorrection->msb>>(3-i)*8)&0xff;
        buf[12+i] = (headCorrection->lsb>>(3-i)*8)&0xff;
    }

}

void Input_Duration(Octet * buf,Uint32 Duration)
{
    int i;
    for(i=0;i<=3;i++)
    {
        buf[50+i] = (Duration>>(3-i)*8)&0xff;
    }

}
    

static Boolean netSetMulticastTTL(int sockfd, int ttl) {

	int temp = ttl;

	if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,
		       &temp, sizeof(temp)) < 0) {
	    PLOG("Failed to set socket multicast time-to-live");
	    return FALSE;
	}
	return TRUE;
}


/*Pack header message into OUT buffer of ptpClock*/
void msgPackHeader(Octet * buf, PtpClock * ptpClock)
{
	Nibble transport = 0x00;

	/* (spec annex D) */
	*(UInteger8 *) (buf + 0) = transport;
	*(UInteger4 *) (buf + 1) = ptpClock->versionNumber;
	*(UInteger8 *) (buf + 4) = ptpClock->domainNumber;
	/* clear flag field - message packing functions should populate it */
	memset((buf + 6), 0, 2);

	memset((buf + 8), 0, 8);
	copyClockIdentity((buf + 20), ptpClock->portIdentity.clockIdentity);
	*(UInteger16 *) (buf + 28) = flip16(ptpClock->portIdentity.portNumber);
	/* LogMessageInterval defaults to 0x7F, will be set to another value if needed as per table 24*/
	*(UInteger8 *) (buf + 33) = 0x7F;
}


void msgPackSync(Octet * buf, Timestamp* originTimestamp, PtpClock* ptpClock)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;

	/* Two step flag - table 20: Sync and PDelayResp only */
	if (ptpClock->stepType)
		*(UInteger8 *) (buf + 6) |= PTP_TWO_STEP;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(SYNC_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(ptpClock->sentSyncSequenceId);
	*(UInteger8 *) (buf + 32) = 0x00;

	 /* Table 24 - unless it's multicast, logMessageInterval remains    0x7F */
	 if(ptpClock->modeType == IPMODE_MULTICAST)
		*(Integer8 *) (buf + 33) = ptpClock->logSyncInterval;
     else if(ptpClock->modeType == IPMODE_UNICAST)
        *(Integer8 *) (buf + 33) = 0x7F;
     
	memset((buf + 8), 0, 8);

	/* Sync message */
#if 0
	*(UInteger16 *) (buf + 34) = flip16(originTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(originTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(originTimestamp->nanosecondsField);
#endif
}

void msgPackFollowUp(Octet * buf, Timestamp *preciseOriginTimestamp, PtpClock * ptpClock, const UInteger16 sequenceId)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x08;
    
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(FOLLOW_UP_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x02;

	 /* Table 24 - unless it's multicast, logMessageInterval remains    0x7F */
	 if(ptpClock->modeType == IPMODE_MULTICAST)
		*(Integer8 *) (buf + 33) = ptpClock->logSyncInterval;
     else if(ptpClock->modeType == IPMODE_UNICAST)
        *(Integer8 *) (buf + 33) = 0x7F;

	/* Follow_up message */
    Input_TimeStamp(buf,preciseOriginTimestamp);
#if 0
	*(UInteger16 *) (buf + 34) = flip16(preciseOriginTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(preciseOriginTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(preciseOriginTimestamp->nanosecondsField);
#endif

       
}

void msgPackAnnounce(Octet * buf, PtpClock* ptpClock)
{
	UInteger16 stepsRemoved;
	
	msgPackHeader(buf, ptpClock);

	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x0B;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(ANNOUNCE_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(ptpClock->sentAnnounceSequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
	 /* Table 24 - logMessageInterval remains    Data */
	*(Integer8 *) (buf + 33) = ptpClock->logAnnounceInterval;

	/* Announce message */
	memset((buf + 34), 0, 10);
	*(Integer16 *) (buf + 44) = flip16(ptpClock->timePropertiesDS.currentUtcOffset);
	*(UInteger8 *) (buf + 47) = ptpClock->grandmasterPriority1;
	*(UInteger8 *) (buf + 48) = ptpClock->grandmasterClockQuality.clockClass;
	*(Enumeration8 *) (buf + 49) = ptpClock->grandmasterClockQuality.clockAccuracy;
	*(UInteger16 *) (buf + 50) =
		flip16(ptpClock->grandmasterClockQuality.offsetScaledLogVariance);
	*(UInteger8 *) (buf + 52) = ptpClock->grandmasterPriority2;
	copyClockIdentity((buf + 53), ptpClock->grandmasterIdentity);
	/* resolve bugs #37 and #40 - alignment errors on ARMv5 */
	stepsRemoved = flip16(ptpClock->stepsRemoved);
	memcpy(buf + 61, &stepsRemoved, sizeof(UInteger16));
	*(Enumeration8 *) (buf + 63) = ptpClock->timePropertiesDS.timeSource;

	/*
	 * TimePropertiesDS in FlagField, 2nd octet - spec 13.3.2.6 table 20
	 * Could / should have used constants here PTP_LI_61 etc, but this is clean
	 */
	*(UInteger8*) (buf + 7) = ptpClock->timePropertiesDS.leap61			<< 0;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.leap59)			<< 1;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.currentUtcOffsetValid)	<< 2;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.ptpTimescale)		<< 3;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.timeTraceable)		<< 4;
	*(UInteger8*) (buf + 7) |= (ptpClock->timePropertiesDS.frequencyTraceable)	<< 5;

}



void msgPackDelayReq(Octet *buf, Timestamp *originTimestamp, PtpClock *ptpClock)
{
	msgPackHeader(buf, ptpClock);

	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x01;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(DELAY_REQ_LENGTH);

	/* -- PTP_UNICAST flag will be set in netsend* if needed */
	*(UInteger16 *) (buf + 30) = flip16(ptpClock->sentDelayReqSequenceId);
	*(UInteger8 *) (buf + 32) = 0x01;
	/* Table 23 */
	*(Integer8 *) (buf + 33) = 0x7F;
	/* Table 24 */
	memset((buf + 8), 0, 8);

	/* delay_req message */
#if 0
	*(UInteger16 *) (buf + 34) = flip16(originTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(originTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(originTimestamp->nanosecondsField);
#endif
}


void msgPackPDelayReq(Octet * buf, Timestamp * originTimestamp, PtpClock * ptpClock)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x02;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(PDELAY_REQ_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(ptpClock->sentPDelayReqSequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
	/* Table 23 */
	*(Integer8 *) (buf + 33) = 0x7F;
	/* Table 24 */
	memset((buf + 8), 0, 8);

	/* Pdelay_req message */
    #if 0
	*(UInteger16 *) (buf + 34) = flip16(originTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(originTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(originTimestamp->nanosecondsField);
    #endif

	memset((buf + 44), 0, 10);
	/* RAZ reserved octets */
}



void msgPackDelayResp(Octet * buf, MsgHeader * header, Timestamp * receiveTimestamp, PtpClock * ptpClock)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x09;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(DELAY_RESP_LENGTH);
	*(UInteger8 *) (buf + 4) = header->domainNumber;

	/* -- PTP_UNICAST flag will be set in netsend* if needed */

	memset((buf + 8), 0, 8);

	/* Copy correctionField of delayReqMessage */
	*(Integer32 *) (buf + 8) = flip32(header->correctionField.msb);
	*(Integer32 *) (buf + 12) = flip32(header->correctionField.lsb);
    
	*(UInteger16 *) (buf + 30) = flip16(header->sequenceId);

	*(UInteger8 *) (buf + 32) = 0x03;

	 /* Table 24 - unless it's multicast, logMessageInterval remains    0x7F */
     if(ptpClock->modeType == IPMODE_MULTICAST)
		*(Integer8 *) (buf + 33) = ptpClock->logMinDelayReqInterval;
     else if(ptpClock->modeType == IPMODE_UNICAST)
        *(Integer8 *) (buf + 33) = 0x7F;

	/* delay_resp message */

    Input_TimeStamp(buf,receiveTimestamp);
#if 0
	*(UInteger16 *) (buf + 34) = flip16(receiveTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(receiveTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(receiveTimestamp->nanosecondsField);
#endif

	copyClockIdentity((buf + 44),header->sourcePortIdentity.clockIdentity);
	*(UInteger16 *) (buf + 52) = flip16(header->sourcePortIdentity.portNumber);
}


void msgPackPDelayResp(Octet * buf, MsgHeader * header, Timestamp * requestReceiptTimestamp, PtpClock * ptpClock)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x03;
	/* Two step flag - table 20: Sync and PDelayResp only */
	if (ptpClock->stepType)
		*(UInteger8 *) (buf + 6) |= PTP_TWO_STEP;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(PDELAY_RESP_LENGTH);
	*(UInteger8 *) (buf + 4) = header->domainNumber;
	memset((buf + 8), 0, 8);

	*(UInteger16 *) (buf + 30) = flip16(header->sequenceId);

	*(UInteger8 *) (buf + 32) = 0x05;
	/* Table 23 */
	*(Integer8 *) (buf + 33) = 0x7F;
	/* Table 24 */


    if(ptpClock->stepType == ONE_STEP)
    {
        TimeInternal RecvTime;
        
        /**获得接收到的correct  ,写入FPGA*/
        WriteCorrectionToFpgaCorrection(ptpClock,&header->correctionField);
        toInternalTime(&RecvTime,requestReceiptTimestamp);
        WriteT4ToFpga(ptpClock,&RecvTime);

    }
    else if(ptpClock->stepType == TWO_STEP)
    {
         /* Copy correctionField of PdelayReqMessage */
        Input_Correction(buf,&header->correctionField);
    }
    
#if 0
	*(Integer32 *) (buf + 8) = flip32(header->correctionField.msb);
	*(Integer32 *) (buf + 12) = flip32(header->correctionField.lsb);
#endif


	/* Pdelay_resp message */
    Input_TimeStamp(buf,requestReceiptTimestamp);
#if 0
	*(UInteger16 *) (buf + 34) = flip16(requestReceiptTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(requestReceiptTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(requestReceiptTimestamp->nanosecondsField);
#endif

	copyClockIdentity((buf + 44), header->sourcePortIdentity.clockIdentity);
	*(UInteger16 *) (buf + 52) = flip16(header->sourcePortIdentity.portNumber);

}

void msgPackPDelayRespFollowUp(Octet * buf, MsgHeader * header,const TimeInternal *requetReceipt,Timestamp * responseOriginTimestamp,PtpClock * ptpClock, const UInteger16 sequenceId)
{
	msgPackHeader(buf, ptpClock);
	
	/* changes in header */
	*(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
	/* RAZ messageType */
	*(char *)(buf + 0) = *(char *)(buf + 0) | 0x0A;
	/* Table 19 */
	*(UInteger16 *) (buf + 2) = flip16(PDELAY_RESP_FOLLOW_UP_LENGTH);
	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
	/* Table 23 */
	*(Integer8 *) (buf + 33) = 0x7F;
	/* Table 24 */

    if(ptpClock->stepType == TWO_STEP)
    {
        TimeInternal responseOrigin;
        TimeInternal correction;
        Integer64 headCorrection;
        
        toInternalTime(&responseOrigin,responseOriginTimestamp);
        subTime(&correction,&responseOrigin,requetReceipt);
        internalTime_to_integer64(correction,&headCorrection);
        
        /* Copy correctionField of PdelayReqMessage */
        Input_Correction(buf,&headCorrection);
        
    #if 0
    	*(Integer32 *) (buf + 8) = flip32(headCorrection.msb);
    	*(Integer32 *) (buf + 12) = flip32(headCorrection.lsb);
    #endif

    }

    
	/* Pdelay_resp_follow_up message */
    //Input_TimeStamp(buf,responseOriginTimestamp);
#if 0
	*(UInteger16 *) (buf + 34) = flip16(responseOriginTimestamp->secondsField.msb);
	*(UInteger32 *) (buf + 36) = flip32(responseOriginTimestamp->secondsField.lsb);
	*(UInteger32 *) (buf + 40) = flip32(responseOriginTimestamp->nanosecondsField);
#endif
    
	copyClockIdentity((buf + 44), header->sourcePortIdentity.clockIdentity);
	*(UInteger16 *) (buf + 52) =
		flip16(header->sourcePortIdentity.portNumber);
}



void msgPackSignaling_Request(Octet * buf,UInteger8 MessageType,PtpClock * ptpClock)
{
    msgPackHeader(buf, ptpClock);

    /* changes in header */
    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    /* RAZ messageType */
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x0C;
    
    *(UInteger16 *) (buf + 2) = flip16(SIGNAL_REQUEST_LENGTH);
    
	*(UInteger16 *) (buf + 30) = flip16(ptpClock->sentSignalSequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
    *(Integer8 *) (buf + 33) = 0x7F;

    memset((buf + 34),0xff,10);
    
    *(UInteger16 *)(buf + 44) = flip16(REQUEST_UNICAST_TRANSMISSION);
    *(UInteger16 *)(buf + 46) = flip16(SIGNAL_REQUEST_TLV_LENGTH);

    switch (MessageType)
    {
        case SIGNAL_SYN_MSG:
            
            *(UInteger8 *) (buf + 48) = SIGNAL_SYN_TYPE;
            *(Integer8 *) (buf + 49) = ptpClock->logSyncInterval;
            break;
        case SIGNAL_ANNOUNCE_MSG:
                        
            *(UInteger8 *) (buf + 48) = SIGNAL_ANNOUNCE_TYPE;
            *(Integer8 *) (buf + 49) = ptpClock->logAnnounceInterval;
            break;
        case SIGNAL_DELAYRESP_MSG:
            
            *(UInteger8 *) (buf + 48) = SIGNAL_DELAY_RESP_TYPE;
            *(Integer8 *) (buf + 49) = ptpClock->logMinDelayReqInterval;
            break;
        case SIGNAL_PDELAYRESP_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_PDELAY_RESP_TYPE;
            *(Integer8 *) (buf + 49) = ptpClock->logMinPdelayReqInterval;
            
        default:
            break;
        
    }
    Input_Duration(buf,ptpClock->UnicastDuration);
    
}

void msgPackSignaling_Grant(Octet * buf,UInteger8 MessageType,Integer8 loginterval,PtpClock * ptpClock,const UInteger16 sequenceId,const Uint32 duration)
{
     msgPackHeader(buf, ptpClock);

    /* changes in header */
    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    /* RAZ messageType */
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x0C;

    /** length */
    *(UInteger16 *) (buf + 2) = flip16(SIGNAL_GRANT_LENGTH);

	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
    *(Integer8 *) (buf + 33) = 0x7F;
    
    memset((buf + 34),0xff,10);
    
    *(UInteger16 *)(buf + 44) = flip16(GRANT_UNICAST_TRANSMISSION);
    *(UInteger16 *)(buf + 46) = flip16(SIGNAL_GRANT_TLV_LENGTH);

    switch (MessageType)
    {
        case SIGNAL_SYN_MSG:
            
            *(UInteger8 *) (buf + 48) = SIGNAL_SYN_TYPE;
            *(Integer8 *) (buf + 49) = loginterval;
            break;
            
        case SIGNAL_ANNOUNCE_MSG:
                        
            *(UInteger8 *) (buf + 48) = SIGNAL_ANNOUNCE_TYPE;
            *(Integer8 *) (buf + 49) = loginterval;
            break;
            
        case SIGNAL_DELAYRESP_MSG:
                       
            *(UInteger8 *) (buf + 48) = SIGNAL_DELAY_RESP_TYPE;
            *(Integer8 *) (buf + 49) = loginterval;
            break;
            
        case SIGNAL_PDELAYRESP_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_PDELAY_RESP_TYPE;
            *(Integer8 *) (buf + 49) = loginterval;
        default:
            break;
        
    }

    /** in this time send,0:refuse */
    Input_Duration(buf,ptpClock->UnicastDuration);
    //*(UInteger32 *) (buf + 50) = flip32(duration);
    /** update grantor */
    *(UInteger16 *) (buf + 54) = flip16(1);
}

void msgPackSignaling_Cancel(Octet * buf,UInteger8 MessageType,PtpClock * ptpClock,const UInteger16 sequenceId)
{
    msgPackHeader(buf, ptpClock);

    /* changes in header */
    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    /* RAZ messageType */
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x0C;

    /** length */
    *(UInteger16 *) (buf + 2) = flip16(SIGNAL_CANCEL_LENGTH);

	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
    *(Integer8 *) (buf + 33) = 0x7F;
    
    memset((buf + 34),0xff,10);
    
    *(UInteger16 *)(buf + 44) = flip16(CANCEL_UNICAST_TRANSMISSION);
    *(UInteger16 *)(buf + 46) = flip16(SIGNAL_CANCELL_TLV_LENGTH);

    switch (MessageType)
    {
        case SIGNAL_SYN_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_SYN_TYPE;
            break;
            
        case SIGNAL_ANNOUNCE_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_ANNOUNCE_TYPE;
            break;
            
        case SIGNAL_DELAYRESP_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_DELAY_RESP_TYPE;
            break;
            
        case SIGNAL_PDELAYRESP_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_PDELAY_RESP_TYPE;
        default:
            break;
        
    }

}

void msgPackSignaling_AckCancel(Octet * buf,UInteger8 MessageType,PtpClock * ptpClock,const UInteger16 sequenceId)
{
    msgPackHeader(buf, ptpClock);

    /* changes in header */
    *(char *)(buf + 0) = *(char *)(buf + 0) & 0xF0;
    /* RAZ messageType */
    *(char *)(buf + 0) = *(char *)(buf + 0) | 0x0C;

    /** length */
    *(UInteger16 *) (buf + 2) = flip16(SIGNAL_ACKNOWLEDGE_CANCEL_LENGTH);

	*(UInteger16 *) (buf + 30) = flip16(sequenceId);
	*(UInteger8 *) (buf + 32) = 0x05;
    *(Integer8 *) (buf + 33) = 0x7F;
    
    memset((buf + 34),0xff,10);
    
    *(UInteger16 *)(buf + 44) = flip16(ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION);
    *(UInteger16 *)(buf + 46) = flip16(SIGNAL_ACKNOWLEDGE_CANCEL_TLV_LENGTH);

    switch (MessageType)
    {
        case SIGNAL_SYN_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_SYN_TYPE;
            break;
            
        case SIGNAL_ANNOUNCE_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_ANNOUNCE_TYPE;
            break;
            
        case SIGNAL_DELAYRESP_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_DELAY_RESP_TYPE;
            break;
            
        case SIGNAL_PDELAYRESP_MSG:
            *(UInteger8 *) (buf + 48) = SIGNAL_PDELAY_RESP_TYPE;
        default:
            break;
        
    }

}


void msgUnpackHeader(Octet * buf, MsgHeader * header)
{
	header->transportSpecific = (*(Nibble *) (buf + 0)) >> 4;
	header->messageType = (*(Enumeration4 *) (buf + 0)) & 0x0F;
	header->versionPTP = (*(UInteger4 *) (buf + 1)) & 0x0F;
	/* force reserved bit to zero if not */
	header->messageLength = flip16(*(UInteger16 *) (buf + 2));
	header->domainNumber = (*(UInteger8 *) (buf + 4));
    header->reserved1 = (*(UInteger8 *) (buf + 5));
    
	header->flagField0 = (*(Octet *) (buf + 6));
	header->flagField1 = (*(Octet *) (buf + 7));
    
	memcpy(&header->correctionField.msb, (buf + 8), 4);
	memcpy(&header->correctionField.lsb, (buf + 12), 4);
    
	header->correctionField.msb = flip32(header->correctionField.msb);
	header->correctionField.lsb = flip32(header->correctionField.lsb);

    memcpy(&header->reserved2, (buf + 16), 4);
    header->reserved2 = flip32(header->reserved2);
    
	copyClockIdentity(header->sourcePortIdentity.clockIdentity, (buf + 20));
	header->sourcePortIdentity.portNumber = flip16(*(UInteger16 *) (buf + 28));
	header->sequenceId = flip16(*(UInteger16 *) (buf + 30));
	header->controlField = (*(UInteger8 *) (buf + 32));
	header->logMessageInterval = (*(Integer8 *) (buf + 33));

}

void msgUnpackRequestSignal(Octet * buf, SignalReq* signal)
{
    signal->messageType = (*(UInteger8*)(buf + 48))&0xF0;
    signal->logInterMessagePeriod = (*(UInteger8*)(buf + 49));

    memcpy(&signal->durationField,(buf + 50),4);
    signal->durationField = flip32(signal->durationField);

}

void msgUnpackGrantSignal(Octet * buf, SignalGrant* signal)
{
    signal->messageType = (*(UInteger8*)(buf + 48))&0xF0;
    signal->logInterMessagePeriod = (*(UInteger8*)(buf + 49));

    memcpy(&signal->durationField,(buf + 50),4);
    signal->durationField = flip32(signal->durationField);

    signal->reserved = (*(UInteger8*)(buf + 54));
    signal->R = (*(UInteger8*)(buf + 55));

}

void msgUnpackCancelSignal(Octet * buf, SignalCancel* signal)
{
    signal->messageType = (*(UInteger8*)(buf + 48))&0xF0;
    signal->reserved = (*(UInteger8*)(buf + 49));


}

void msgUnpackAckCancelSignal(Octet * buf, SignalAckCancel* signal)
{
    signal->messageType = (*(UInteger8*)(buf + 48))&0xF0;
    signal->reserved = (*(UInteger8*)(buf + 49));
}

void msgUnpackSync(Octet * buf, MsgSync * sync)
{
	sync->originTimestamp.secondsField.msb = flip16(*(UInteger16 *) (buf + 34));
    
    memcpy(&sync->originTimestamp.secondsField.lsb, (buf + 36), 4);
    memcpy(&sync->originTimestamp.nanosecondsField, (buf + 40), 4);
    
	sync->originTimestamp.secondsField.lsb = flip32(sync->originTimestamp.secondsField.lsb);
	sync->originTimestamp.nanosecondsField = flip32(sync->originTimestamp.nanosecondsField);
}

void msgUnpackFollowUp(Octet * buf, MsgFollowUp * follow)
{
	follow->preciseOriginTimestamp.secondsField.msb = flip16(*(UInteger16 *) (buf + 34));

    memcpy(&follow->preciseOriginTimestamp.secondsField.lsb, (buf + 36), 4);
    memcpy(&follow->preciseOriginTimestamp.nanosecondsField, (buf + 40), 4);
    
	follow->preciseOriginTimestamp.secondsField.lsb = flip32(follow->preciseOriginTimestamp.secondsField.lsb);
	follow->preciseOriginTimestamp.nanosecondsField = flip32(follow->preciseOriginTimestamp.nanosecondsField);

}

void msgUnpackAnnounce(Octet * buf, MsgAnnounce * announce)
{
	UInteger16 stepsRemoved;
	
	announce->originTimestamp.secondsField.msb =
		flip16(*(UInteger16 *) (buf + 34));
	announce->originTimestamp.secondsField.lsb =
		flip32(*(UInteger32 *) (buf + 36));
	announce->originTimestamp.nanosecondsField =
		flip32(*(UInteger32 *) (buf + 40));
	announce->currentUtcOffset = flip16(*(UInteger16 *) (buf + 44));
	announce->grandmasterPriority1 = *(UInteger8 *) (buf + 47);
	announce->grandmasterClockQuality.clockClass =
		*(UInteger8 *) (buf + 48);
	announce->grandmasterClockQuality.clockAccuracy =
		*(Enumeration8 *) (buf + 49);
	announce->grandmasterClockQuality.offsetScaledLogVariance =
		flip16(*(UInteger16 *) (buf + 50));
	announce->grandmasterPriority2 = *(UInteger8 *) (buf + 52);
	copyClockIdentity(announce->grandmasterIdentity, (buf + 53));
	/* resolve bugs #37 and #40 - alignment errors on ARMv5 */
	memcpy(&stepsRemoved, buf + 61, sizeof(UInteger16));
	announce->stepsRemoved = flip16(stepsRemoved);
	announce->timeSource = *(Enumeration8 *) (buf + 63);

}

void msgUnpackDelayReq(Octet * buf, MsgDelayReq * delayreq)
{
	delayreq->originTimestamp.secondsField.msb = flip16(*(UInteger16 *) (buf + 34));

    memcpy(&delayreq->originTimestamp.secondsField.lsb, (buf + 36), 4);
    memcpy(&delayreq->originTimestamp.nanosecondsField, (buf + 40), 4);
    
	delayreq->originTimestamp.secondsField.lsb = flip32(delayreq->originTimestamp.secondsField.lsb);
	delayreq->originTimestamp.nanosecondsField = flip32(delayreq->originTimestamp.nanosecondsField);

}

void msgUnpackPDelayReq(Octet * buf, MsgPDelayReq * pdelayreq)
{
	pdelayreq->originTimestamp.secondsField.msb = flip16(*(UInteger16 *) (buf + 34));

    memcpy(&pdelayreq->originTimestamp.secondsField.lsb, (buf + 36), 4);
    memcpy(&pdelayreq->originTimestamp.nanosecondsField, (buf + 40), 4);
    
	pdelayreq->originTimestamp.secondsField.lsb = flip32(pdelayreq->originTimestamp.secondsField.lsb);
	pdelayreq->originTimestamp.nanosecondsField = flip32(pdelayreq->originTimestamp.nanosecondsField);

}


void msgUnpackDelayResp(Octet * buf, MsgDelayResp * resp)
{
	resp->receiveTimestamp.secondsField.msb = flip16(*(UInteger16 *) (buf + 34));

    memcpy(&resp->receiveTimestamp.secondsField.lsb, (buf + 36), 4);
    memcpy(&resp->receiveTimestamp.nanosecondsField, (buf + 40), 4);
    
	resp->receiveTimestamp.secondsField.lsb = flip32(resp->receiveTimestamp.secondsField.lsb);
	resp->receiveTimestamp.nanosecondsField = flip32(resp->receiveTimestamp.nanosecondsField);

	copyClockIdentity(resp->requestingPortIdentity.clockIdentity,(buf + 44));
	resp->requestingPortIdentity.portNumber = flip16(*(UInteger16 *) (buf + 52));

}

void msgUnpackPDelayResp(Octet * buf, MsgPDelayResp * presp)
{
	presp->requestReceiptTimestamp.secondsField.msb = flip16(*(UInteger16 *) (buf + 34));

    memcpy(&presp->requestReceiptTimestamp.secondsField.lsb, (buf + 36), 4);
    memcpy(&presp->requestReceiptTimestamp.nanosecondsField, (buf + 40), 4);
    
	presp->requestReceiptTimestamp.secondsField.lsb = flip32(presp->requestReceiptTimestamp.secondsField.lsb);
	presp->requestReceiptTimestamp.nanosecondsField = flip32(presp->requestReceiptTimestamp.nanosecondsField);

	copyClockIdentity(presp->requestingPortIdentity.clockIdentity,(buf + 44));
	presp->requestingPortIdentity.portNumber = flip16(*(UInteger16 *) (buf + 52));

}


void msgUnpackPDelayRespFollowUp(Octet * buf, MsgPDelayRespFollowUp * prespfollow)
{
	prespfollow->responseOriginTimestamp.secondsField.msb =	flip16(*(UInteger16 *) (buf + 34));

    memcpy(&prespfollow->responseOriginTimestamp.secondsField.lsb, (buf + 36), 4);
    memcpy(&prespfollow->responseOriginTimestamp.nanosecondsField, (buf + 40), 4);
    
	prespfollow->responseOriginTimestamp.secondsField.lsb = flip32(prespfollow->responseOriginTimestamp.secondsField.lsb);
	prespfollow->responseOriginTimestamp.nanosecondsField = flip32(prespfollow->responseOriginTimestamp.nanosecondsField);

   
	copyClockIdentity(prespfollow->requestingPortIdentity.clockIdentity,(buf + 44));
	prespfollow->requestingPortIdentity.portNumber = flip16(*(UInteger16 *) (buf + 52));

}



void netInitIeeeAddress(struct sockaddr_ll *pIeee_addr,Uint8* pMacAddress,PtpClock *pPtpClock,Integer32 sockId)
{
    struct ifreq ifr;
    /* 初始化 ieee802.3 sock */
    
    memset(&ifr, 0, sizeof(ifr));
    memcpy(ifr.ifr_name, pPtpClock->netEnviroment.ifaceName, sizeof(ifr.ifr_name)-1);
    if(ioctl(sockId,SIOCGIFINDEX,&ifr)<0)
    {
        PLOG("get sll_ifindex error\n");
    }
    memset(pIeee_addr,0,sizeof(struct sockaddr_ll));
    
    pIeee_addr->sll_family=AF_PACKET;    
    pIeee_addr->sll_protocol=htons(ETH_P_ALL);
    pIeee_addr->sll_ifindex=ifr.ifr_ifindex;
    
    if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        pIeee_addr->sll_pkttype = PACKET_MULTICAST; 
        memcpy(pIeee_addr->sll_addr,pMacAddress,6);
    }
    else
    {
        pIeee_addr->sll_pkttype = PACKET_OTHERHOST;
        memcpy(pIeee_addr->sll_addr,pMacAddress,6);
    }
    pIeee_addr->sll_hatype = ARPHRD_ETHER;         
    pIeee_addr->sll_halen = ETH_ALEN; 

}


void issueAckCancelSignaling(PtpClock *pPtpClock,Uint8 signalType,MsgHeader *pMsgHeader,void *pAddr)
{
    ssize_t ret;
    int iOfset;
    
    struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    
    /**按照最大的长度申请  */
    Uint8 ether_buf[IEEE802_HEAD_LEN + SIGNAL_ACKNOWLEDGE_CANCEL_LENGTH];
    
    memset(ether_buf,0,sizeof(ether_buf));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PTP_GENERAL_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;
	
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));

    /**组装  */
    switch (signalType)
    {
        case SIGNAL_SYN_MSG:
            msgPackSignaling_AckCancel(pPtpClock->msgObuf,SIGNAL_SYN_MSG,pPtpClock,pMsgHeader->sequenceId);
            break; 
        case SIGNAL_ANNOUNCE_MSG:
            msgPackSignaling_AckCancel(pPtpClock->msgObuf,SIGNAL_ANNOUNCE_MSG,pPtpClock,pMsgHeader->sequenceId);
            break;
        case SIGNAL_DELAYRESP_MSG:
            msgPackSignaling_AckCancel(pPtpClock->msgObuf,SIGNAL_DELAYRESP_MSG,pPtpClock,pMsgHeader->sequenceId);
            break;
        case SIGNAL_PDELAYRESP_MSG:
            msgPackSignaling_AckCancel(pPtpClock->msgObuf,SIGNAL_PDELAYRESP_MSG,pPtpClock,pMsgHeader->sequenceId);
            break;
            
        case SIGNAL_MSG_NULL:
            return;
            
        default:
            return;
    }

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            memcpy(pNetComm->UnicastMac,(Uint8 *)pAddr,6);
            
            memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
            iOfset += sizeof(pNetComm->UnicastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
                        
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,SIGNAL_ACKNOWLEDGE_CANCEL_LENGTH);
            iOfset += SIGNAL_ACKNOWLEDGE_CANCEL_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending Signal AckCancel message Type\n");
            }
            else
            {
                LOG(pPtpClock->debugLevel,"%s ==>send  Signal AckCancel Msg Success!!\n",pPtpClock->netEnviroment.ifaceName);
            }
        
        }

    }
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            
            pNetComm->unicastAddr = *(Uint32 *)pAddr;
            
            addr.sin_addr.s_addr = pNetComm->unicastAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, SIGNAL_ACKNOWLEDGE_CANCEL_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending Signal AckCancel message Type\n");
            }
            else
            {
                LOG(pPtpClock->debugLevel,"%s ==>send  Signal AckCancel Msg Success!!\n",pPtpClock->netEnviroment.ifaceName);
            }

            
        }        
    }
}


void issueCancelSignaling(PtpClock *pPtpClock,Uint8 signalType,MsgHeader *pMsgHeader,SignalReq *pSignalReq,void *pAddr)
{
    ssize_t ret;
    int iOfset;
    
    struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    
    /**按照最大的长度申请  */
    Uint8 ether_buf[IEEE802_HEAD_LEN + SIGNAL_CANCEL_LENGTH];
    
    memset(ether_buf,0,sizeof(ether_buf));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PTP_GENERAL_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;
	
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));

    /**组装  */
    switch (signalType)
    {
        case SIGNAL_SYN_MSG:
            msgPackSignaling_Cancel(pPtpClock->msgObuf,SIGNAL_SYN_MSG,pPtpClock,pMsgHeader->sequenceId);
            break; 
        case SIGNAL_ANNOUNCE_MSG:
            msgPackSignaling_Cancel(pPtpClock->msgObuf,SIGNAL_ANNOUNCE_MSG,pPtpClock,pMsgHeader->sequenceId);
            break;
        case SIGNAL_DELAYRESP_MSG:
            msgPackSignaling_Cancel(pPtpClock->msgObuf,SIGNAL_DELAYRESP_MSG,pPtpClock,pMsgHeader->sequenceId);
            break;
        case SIGNAL_PDELAYRESP_MSG:
            msgPackSignaling_Cancel(pPtpClock->msgObuf,SIGNAL_PDELAYRESP_MSG,pPtpClock,pMsgHeader->sequenceId);
            break;
            
        case SIGNAL_MSG_NULL:
            return;
            
        default:
            return;
    }

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            memcpy(pNetComm->UnicastMac,(Uint8 *)pAddr,6);
            
            memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
            iOfset += sizeof(pNetComm->UnicastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
                        
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,SIGNAL_CANCEL_LENGTH);
            iOfset += SIGNAL_CANCEL_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending Signal Cancel message Type: %d\n",signalType);
            }
            else
            {
                LOG(pPtpClock->debugLevel,"%s ==>send  Signal Cancel Msg Type :%d To %x Success!!\n",pPtpClock->netEnviroment.ifaceName,signalType,pNetComm->UnicastMac[0]);
            }
        
        }

    }
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            
            pNetComm->unicastAddr = *(Uint32 *)pAddr;
            
            addr.sin_addr.s_addr = pNetComm->unicastAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, SIGNAL_CANCEL_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending Signal Cancel message Type: %d\n",signalType);
            }
            else
            {
                LOG(pPtpClock->debugLevel,"%s ==>send  Signal Cancel Msg Type :%d  To %d Success!!\n",pPtpClock->netEnviroment.ifaceName,signalType,pNetComm->unicastAddr>>24);
            }

            
        }        
    }
}


void issueGrantSignaling(PtpClock *pPtpClock,Uint8 signalType,MsgHeader *pMsgHeader,SignalReq *pSignalReq,void *pAddr)
{
    ssize_t ret;
    int iOfset;
    
    struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    
    /**按照最大的长度申请  */
    Uint8 ether_buf[IEEE802_HEAD_LEN + SIGNAL_GRANT_LENGTH];
    
    memset(ether_buf,0,sizeof(ether_buf));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PTP_GENERAL_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;
	
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));

    /**组装  */
    switch (signalType)
    {
        case SIGNAL_SYN_MSG:
            msgPackSignaling_Grant(pPtpClock->msgObuf,SIGNAL_SYN_MSG,pPtpClock->logSyncInterval,pPtpClock,pMsgHeader->sequenceId,pSignalReq->durationField);
            break; 
        case SIGNAL_ANNOUNCE_MSG:
            msgPackSignaling_Grant(pPtpClock->msgObuf,SIGNAL_ANNOUNCE_MSG,pPtpClock->logAnnounceInterval,pPtpClock,pMsgHeader->sequenceId,pSignalReq->durationField);
            break;
        case SIGNAL_DELAYRESP_MSG:
            msgPackSignaling_Grant(pPtpClock->msgObuf,SIGNAL_DELAYRESP_MSG,pPtpClock->logMinDelayReqInterval,pPtpClock,pMsgHeader->sequenceId,pSignalReq->durationField);
            break;
        case SIGNAL_PDELAYRESP_MSG:
            msgPackSignaling_Grant(pPtpClock->msgObuf,SIGNAL_PDELAYRESP_MSG,pPtpClock->logMinPdelayReqInterval,pPtpClock,pMsgHeader->sequenceId,pSignalReq->durationField);
            break;
            
        case SIGNAL_MSG_NULL:
            return;
            
        default:
            return;
    }

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            memcpy(pNetComm->UnicastMac,(Uint8 *)pAddr,6);
            
            memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
            iOfset += sizeof(pNetComm->UnicastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
                        
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,SIGNAL_GRANT_LENGTH);
            iOfset += SIGNAL_GRANT_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending Signal Grant message Type: %d\n",signalType);
            }
            else
            {
                LOG(pPtpClock->debugLevel,"%s ==>send  Signal Grant Msg Type :%d To %x Success!!\n",pPtpClock->netEnviroment.ifaceName,signalType,pNetComm->UnicastMac[0]);
            }
        
        }

    }
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            
            pNetComm->unicastAddr = *(Uint32 *)pAddr;
            
            addr.sin_addr.s_addr = pNetComm->unicastAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, SIGNAL_GRANT_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending Signal Grant message Type: %d\n",signalType);
            }
            else
            {
                LOG(pPtpClock->debugLevel,"%s ==>send  Signal Grant Msg Type :%d  To %d Success!!\n",pPtpClock->netEnviroment.ifaceName,signalType,pNetComm->unicastAddr>>24);
            }
            
        }        
    }
}


void issueRequestSignaling(PtpClock *pPtpClock)
{
    ssize_t ret;
    int iOfset;
    
    struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    Uint8 signalType;
    
    /**按照最大的长度申请  */
    Uint8 ether_buf[IEEE802_HEAD_LEN + SIGNAL_REQUEST_LENGTH];
    
    memset(ether_buf,0,sizeof(ether_buf));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PTP_GENERAL_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;
	
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));


    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        signalType = GetMultiServ_SendServerAddress_And_MsgType(pPtpClock,&pPtpClock->unicastMultiServer,(void*)pNetComm->UnicastMac);
    }
    else if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        signalType = GetMultiServ_SendServerAddress_And_MsgType(pPtpClock,&pPtpClock->unicastMultiServer,(void *)&pNetComm->unicastAddr);
    }

    /**组装  */
    switch (signalType)
    {
        case SIGNAL_SYN_MSG:
            msgPackSignaling_Request(pPtpClock->msgObuf,SIGNAL_SYN_MSG,pPtpClock);
            break; 
        case SIGNAL_ANNOUNCE_MSG:
            msgPackSignaling_Request(pPtpClock->msgObuf,SIGNAL_ANNOUNCE_MSG,pPtpClock);
            break;
        case SIGNAL_DELAYRESP_MSG:
            
            if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
                msgPackSignaling_Request(pPtpClock->msgObuf,SIGNAL_DELAYRESP_MSG,pPtpClock);
            else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
                msgPackSignaling_Request(pPtpClock->msgObuf,SIGNAL_PDELAYRESP_MSG,pPtpClock);
            break;
            
        case SIGNAL_MSG_NULL:
            return;
            
        default:
            return;
    }

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
            iOfset += sizeof(pNetComm->UnicastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
                        
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,SIGNAL_REQUEST_LENGTH);
            iOfset += SIGNAL_REQUEST_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending Signal Request message Type: %d\n",signalType);
            }
            else
            {
                LOG(pPtpClock->debugLevel,"%s ==>send  Signal Request Msg Type :%d To %x Success!!\n",pPtpClock->netEnviroment.ifaceName,signalType,pNetComm->UnicastMac[0]);
            }
        
        }

    }
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            addr.sin_addr.s_addr = pNetComm->unicastAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, SIGNAL_REQUEST_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending Signal Request message Type: %d\n",signalType);
            }
            else
            {
                LOG(pPtpClock->debugLevel,"%s ==>send  Signal Request Msg Type :%d  To %d Success!!\n",pPtpClock->netEnviroment.ifaceName,signalType,pNetComm->unicastAddr>>24);
            }

            
        }        
    }
}

void issueSync(PtpClock *pPtpClock)
{
	ssize_t ret;
    int iOfset;
	struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    Uint8 ether_buf[IEEE802_HEAD_LEN + SYNC_LENGTH];

    memset(ether_buf,0,sizeof(ether_buf));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PTP_EVENT_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;

    
	Timestamp originTimestamp;
	TimeInternal internalTime;
    
    memset(&originTimestamp,0,sizeof(Timestamp));
    
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
    
    /** 准备发送syn */
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
    msgPackSync(pPtpClock->msgObuf,&originTimestamp,pPtpClock);

    /**  IEEE802.3 模式下*/
    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            if(!GetClientUnicast_SynAddress(pPtpClock,pNetComm->UnicastMac))
                return;
            
            LOG(pPtpClock->debugLevel,"%s ==> Issue Sync (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentSyncSequenceId);
            memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
            iOfset += sizeof(pNetComm->UnicastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;
            
            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
            
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,SYNC_LENGTH);
            iOfset += SYNC_LENGTH;

            
            netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending Syn Muticast message\n");
            }
            else
            {
                //pPtpClock->sentSyncSequenceId++;
                PLOG("send  Syn Msg Success!!\n");

            }
            
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            memcpy(ether_buf + iOfset,pNetComm->E2EMulticastMac,sizeof(pNetComm->E2EMulticastMac));
            iOfset += sizeof(pNetComm->E2EMulticastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;
            
            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }

            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,SYNC_LENGTH);
            iOfset += SYNC_LENGTH;

            LOG(pPtpClock->debugLevel,"%s ==> Issue Sync (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentSyncSequenceId);
            netInitIeeeAddress(&Ieee_addr,pNetComm->E2EMulticastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending Syn Muticast message\n");
            }
            else
            {
                if(pPtpClock->stepType == ONE_STEP)
                    pPtpClock->sentSyncSequenceId++;
                PLOG("send  Syn Msg Success!!\n");

            }

        }
        
    }
    /** UDP IP V4 模式下 */
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            if(!GetClientUnicast_SynAddress(pPtpClock,&pNetComm->unicastAddr))
                return;
            
            LOG(pPtpClock->debugLevel,"%s ==> Issue Sync (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentSyncSequenceId);

            addr.sin_addr.s_addr = pNetComm->unicastAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->eventSock, pPtpClock->msgObuf, SYNC_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending Syn Muticast message\n");
            }
            else
            {
                //pPtpClock->sentSyncSequenceId++;
                PLOG("send  Syn Msg Success!!\n");
            }
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            addr.sin_addr.s_addr = inet_addr(PTP_E2E_MULTICAST_ADDRESS);
        #if 0
            if(netPath->ttlEvent != rtOpts->ttl)
            {
                /* set socket time-to-live  */
                if (netSetMulticastTTL(netPath->eventSock,rtOpts->ttl))
                {
                    netPath->ttlEvent = rtOpts->ttl;
                }
            }
        #endif

            LOG(pPtpClock->debugLevel,"%s ==> Issue Sync (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentSyncSequenceId);

            ret = sendto(pNetComm->eventSock, pPtpClock->msgObuf, SYNC_LENGTH, 0, 
                     (struct sockaddr *)&addr, 
                     sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending Syn Muticast message\n");
            }
            else
            {
                if(pPtpClock->stepType == ONE_STEP)
                    pPtpClock->sentSyncSequenceId++;
                PLOG("send  Syn Msg Success!!\n");
            
            }

        }
    }

    if(pPtpClock->stepType == TWO_STEP)
    {
        //GetSendMessageTime(&internalTime);
        GetFpgaSendMessageTime(pPtpClock,&internalTime);
        issueFollowup(&internalTime,pPtpClock,pPtpClock->sentSyncSequenceId);
    }

}

void issueFollowup(const TimeInternal *tint,PtpClock *pPtpClock, UInteger16 sequenceId)
{

	ssize_t ret;
    int iOfset;
    
	struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    Uint8 ether_buf[IEEE802_HEAD_LEN + FOLLOW_UP_LENGTH];

    
    memset(ether_buf,0,sizeof(ether_buf));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PTP_GENERAL_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;

	Timestamp preciseOriginTimestamp;
    
	fromInternalTime(tint,&preciseOriginTimestamp);
	
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
	msgPackFollowUp(pPtpClock->msgObuf,&preciseOriginTimestamp,pPtpClock,sequenceId);	

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
            iOfset += sizeof(pNetComm->UnicastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
                        
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,FOLLOW_UP_LENGTH);
            iOfset += FOLLOW_UP_LENGTH;
            
            LOG(pPtpClock->debugLevel,"%s ==> Issue Followup (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentSyncSequenceId);

            netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending followUp Muticast message\n");
            }
            else
            {
                //pPtpClock->sentSyncSequenceId++;
                PLOG("send  followUp Msg Success!!\n");
            }

            
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            memcpy(ether_buf + iOfset,pNetComm->E2EMulticastMac,sizeof(pNetComm->E2EMulticastMac));
            iOfset += sizeof(pNetComm->E2EMulticastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
            
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,FOLLOW_UP_LENGTH);
            iOfset += FOLLOW_UP_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->E2EMulticastMac,pPtpClock,pNetComm->IeeeE2ESock);

            LOG(pPtpClock->debugLevel,"%s ==> Issue Followup (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentSyncSequenceId);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending followUp Muticast message\n");
            }
            else
            {
                pPtpClock->sentSyncSequenceId++;
                PLOG("send  followUp Msg Success!!\n");
            }

            
        }
        
        
    }
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            addr.sin_addr.s_addr = pNetComm->unicastAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;

            LOG(pPtpClock->debugLevel,"%s==> Issue Followup (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentSyncSequenceId);
            
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, FOLLOW_UP_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending followUp Muticast message\n");
            }
            else
            {
                PLOG("send  followUp Msg Success!!\n");
            }

            
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            addr.sin_addr.s_addr = inet_addr(PTP_E2E_MULTICAST_ADDRESS);
        #if 0
            if(netPath->ttlEvent != rtOpts->ttl)
            {
                /* set socket time-to-live  */
                if (netSetMulticastTTL(netPath->eventSock,rtOpts->ttl))
                {
                    netPath->ttlEvent = rtOpts->ttl;
                }
            }
        #endif
            
            LOG(pPtpClock->debugLevel,"%s ==> Issue Followup (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentSyncSequenceId);
        
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, FOLLOW_UP_LENGTH, 0, 
                     (struct sockaddr *)&addr, 
                     sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending followUp Muticast message\n");
            }
            else
            {
                pPtpClock->sentSyncSequenceId++;
                PLOG("send  followUp Msg Success!!\n");
            }

        }
    }

}

void issueAnnounce (PtpClock *pPtpClock)
{
	ssize_t ret;
    int iOfset;
    
	struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    Uint8 ether_buf[IEEE802_HEAD_LEN + ANNOUNCE_LENGTH];

    
    memset(ether_buf,0,sizeof(ether_buf));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PTP_GENERAL_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;

    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
    msgPackAnnounce(pPtpClock->msgObuf,pPtpClock);

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            if(!GetClientUnicast_AnnounceAddress(pPtpClock,pNetComm->UnicastMac))
                return;
            LOG(pPtpClock->debugLevel,"%s ==> Issue Announce (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentAnnounceSequenceId);

            memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
            iOfset += sizeof(pNetComm->UnicastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
            
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,ANNOUNCE_LENGTH);
            iOfset += ANNOUNCE_LENGTH;



            netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending Announce Muticast message\n");
            }
            else
            {
                //pPtpClock->sentAnnounceSequenceId++;
                PLOG("send  Announce Msg Success!!\n");
            }
                    
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            memcpy(ether_buf + iOfset,pNetComm->E2EMulticastMac,sizeof(pNetComm->E2EMulticastMac));
            iOfset += sizeof(pNetComm->E2EMulticastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }

            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,ANNOUNCE_LENGTH);
            iOfset += ANNOUNCE_LENGTH;

            LOG(pPtpClock->debugLevel,"%s ==> Issue Announce (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentAnnounceSequenceId);

            netInitIeeeAddress(&Ieee_addr,pNetComm->E2EMulticastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending Muticast message\n");
            }
            else
            {
                pPtpClock->sentAnnounceSequenceId++;
                PLOG("send  Announce Msg Success!!\n");
            }
        }
        
        
    }
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            if(!GetClientUnicast_AnnounceAddress(pPtpClock,&pNetComm->unicastAddr))
                return;
            
            LOG(pPtpClock->debugLevel,"%s ==> Issue Announce (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentAnnounceSequenceId);
            
            addr.sin_addr.s_addr = pNetComm->unicastAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, ANNOUNCE_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending Announce Muticast message\n");
            }
            else
            {
                //pPtpClock->sentAnnounceSequenceId++;
                PLOG("send  Announce Msg Success!!\n");
            }

        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            addr.sin_addr.s_addr = inet_addr(PTP_E2E_MULTICAST_ADDRESS);
    #if 0
            if(netPath->ttlEvent != rtOpts->ttl)
            {
                /* set socket time-to-live  */
                if (netSetMulticastTTL(netPath->eventSock,rtOpts->ttl))
                {
                    netPath->ttlEvent = rtOpts->ttl;
                }
            }
    #endif
            
            LOG(pPtpClock->debugLevel,"%s ==> Issue Announce (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentAnnounceSequenceId);
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, ANNOUNCE_LENGTH, 0, 
                     (struct sockaddr *)&addr, 
                     sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending Announce Muticast message\n");
            }
            else
            {
                pPtpClock->sentAnnounceSequenceId++;
                PLOG("send  Announce Msg Success!!\n");
            
            }

        }
    }

}

void issueDelayReq(PtpClock *pPtpClock)
{
    ssize_t ret;
    int iOfset;
    struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    Uint8 ether_buf[IEEE802_HEAD_LEN + DELAY_REQ_LENGTH];
    
    memset(ether_buf,0,sizeof(ether_buf));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PTP_EVENT_PORT);
    
    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;

	Timestamp originTimestamp;
	TimeInternal internalTime;

    
    /** 从时钟不需要填写timestamp，只需要保存发送时间 */
    #if 0
	getTime(&internalTime);
	if (respectUtcOffset(pPtpClock) == TRUE) {
		internalTime.seconds += pPtpClock->timePropertiesDS.currentUtcOffset;
	}
	fromInternalTime(&internalTime,&originTimestamp);
    #endif

	// uses current sentDelayReqSequenceId
	memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
    
	msgPackDelayReq(pPtpClock->msgObuf,&originTimestamp,pPtpClock);

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            if(!GetSendDelayServer_Mac(&pPtpClock->unicastMultiServer,pNetComm->UnicastMac))
                return;

            memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
            iOfset += sizeof(pNetComm->UnicastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }

            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,DELAY_REQ_LENGTH);
            iOfset += DELAY_REQ_LENGTH;


            LOG(pPtpClock->debugLevel,"%s ==> Issue DelayReq (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentDelayReqSequenceId);

            netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending DelayReq Muticast message\n");
            }
            else
            {
                //pPtpClock->sentDelayReqSequenceId++;
                //GetSendMessageTime(&internalTime);
                GetFpgaSendMessageTime(pPtpClock,&internalTime);
                StoreT3InBestAndUnicastlist(pPtpClock,internalTime,pNetComm->UnicastMac);
                PLOG("send DelayReq Success!!\n");

            }
            
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            LOG(pPtpClock->debugLevel,"%s ==> Issue DelayReq (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentDelayReqSequenceId);
            
            memcpy(ether_buf + iOfset,pNetComm->E2EMulticastMac,sizeof(pNetComm->E2EMulticastMac));
            iOfset += sizeof(pNetComm->E2EMulticastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
                
            }
            
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,DELAY_REQ_LENGTH);
            iOfset += DELAY_REQ_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->E2EMulticastMac,pPtpClock,pNetComm->IeeeE2ESock);

            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending DelayReq Muticast message\n");
            }
            else
            {
                pPtpClock->sentDelayReqSequenceId++;
                
                /**从时钟，多播模式下，获得发送时间戳T3  */
                //GetSendMessageTime(&pPtpClock->T3);
                GetFpgaSendMessageTime(pPtpClock,&pPtpClock->T3);
                PLOG("send DelayReq Success!!\n");
            }
            
        }
        
    }
    /** UDP IP V4 模式下 */
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            if(!GetSendDelayServer_Ip(&pPtpClock->unicastMultiServer,&pNetComm->unicastAddr))
                return;

            LOG(pPtpClock->debugLevel,"%s ==> Issue DelayReq (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentDelayReqSequenceId);

            addr.sin_addr.s_addr = pNetComm->unicastAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->eventSock, pPtpClock->msgObuf, DELAY_REQ_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending DelayReq Muticast message\n");
            }
            else
            {
                //pPtpClock->sentDelayReqSequenceId++;
                //GetSendMessageTime(&internalTime);
                GetFpgaSendMessageTime(pPtpClock,&internalTime);
                StoreT3InBestAndUnicastlist(pPtpClock,internalTime,&pNetComm->unicastAddr);
                PLOG("send  DelayReq Success!!\n");

            }

        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            LOG(pPtpClock->debugLevel,"%s ==> Issue DelayReq (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentDelayReqSequenceId);
            
            addr.sin_addr.s_addr = inet_addr(PTP_E2E_MULTICAST_ADDRESS);
#if 0
            if(netPath->ttlEvent != rtOpts->ttl)
            {
                /* set socket time-to-live  */
                if (netSetMulticastTTL(netPath->eventSock,rtOpts->ttl))
                {
                    netPath->ttlEvent = rtOpts->ttl;
                }
            }
#endif
            
            ret = sendto(pNetComm->eventSock, pPtpClock->msgObuf, DELAY_REQ_LENGTH, 0, 
                     (struct sockaddr *)&addr, 
                     sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending DelayReq Muticast message\n");
            }
            else
            {
                pPtpClock->sentDelayReqSequenceId++;

                /**从时钟，多播模式下，获得发送时间戳T3  */
                //GetSendMessageTime(&pPtpClock->T3);
                GetFpgaSendMessageTime(pPtpClock,&pPtpClock->T3);
                PLOG("send DelayReq Success!!\n");
            }


        }
    }

}

void issuePDelayReq(PtpClock *pPtpClock)
{
        ssize_t ret;
        int iOfset;
        struct sockaddr_in addr;
        struct sockaddr_ll  Ieee_addr;
        Uint8 ether_buf[IEEE802_HEAD_LEN + PDELAY_REQ_LENGTH];
        
        memset(ether_buf,0,sizeof(ether_buf));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(PTP_EVENT_PORT);
        
        NetComm *pNetComm = &pPtpClock->netComm;
        iOfset = 0;
    
        Timestamp originTimestamp;
        TimeInternal internalTime;
    
    
        #if 0
        getTime(&internalTime);
        if (respectUtcOffset(pPtpClock) == TRUE) {
            internalTime.seconds += pPtpClock->timePropertiesDS.currentUtcOffset;
        }
        fromInternalTime(&internalTime,&originTimestamp);
        #endif
    
        //uses current sentDelayReqSequenceId
        memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
        
        msgPackPDelayReq(pPtpClock->msgObuf,&originTimestamp,pPtpClock);
    
        if(pPtpClock->protoType == PROTO_IEEE802)
        {
            if(pPtpClock->modeType == IPMODE_UNICAST)
            {
                if(!GetSendDelayServer_Mac(&pPtpClock->unicastMultiServer,pNetComm->UnicastMac))
                    return;
                
                LOG(pPtpClock->debugLevel,"%s ==> Issue PDelayReq (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentPDelayReqSequenceId);

                memcpy(ether_buf + iOfset,pNetComm->UnicastMac,sizeof(pNetComm->UnicastMac));
                iOfset += sizeof(pNetComm->UnicastMac);
                
                memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
                iOfset += 6;

                if(pPtpClock->vlanEnable)
                {
                    ether_buf[iOfset++] = 0x81;
                    ether_buf[iOfset++] = 0x00;
                    ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                    ether_buf[iOfset++] = pPtpClock->vlanId;
                }
             
                ether_buf[iOfset++] = 0x88;
                ether_buf[iOfset++] = 0xf7;
                
                memcpy(ether_buf + iOfset,pPtpClock->msgObuf,PDELAY_REQ_LENGTH);
                iOfset += PDELAY_REQ_LENGTH;
    
                netInitIeeeAddress(&Ieee_addr,pNetComm->UnicastMac,pPtpClock,pNetComm->IeeeP2PSock);
                
                ret = sendto(pNetComm->IeeeP2PSock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
                if (ret <= 0)
                {
                    PLOG("Error sending PDelayReq  message\n");
                }
                else
                {
                    //pPtpClock->sentPDelayReqSequenceId++;
                    //GetSendMessageTime(&internalTime);
                    GetFpgaSendMessageTime(pPtpClock,&internalTime);
                    StoreT3InBestAndUnicastlist(pPtpClock,internalTime,pNetComm->UnicastMac);
                    PLOG("send PDelayReq Success!!\n");
    
                }
                
            }
            if(pPtpClock->modeType == IPMODE_MULTICAST)
            {
                LOG(pPtpClock->debugLevel,"%s ==> Issue PDelayReq (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentPDelayReqSequenceId);
                
                memcpy(ether_buf + iOfset,pNetComm->P2PMulticastMac,sizeof(pNetComm->P2PMulticastMac));
                iOfset += sizeof(pNetComm->P2PMulticastMac);
                
                memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
                iOfset += 6;

                if(pPtpClock->vlanEnable)
                {
                    ether_buf[iOfset++] = 0x81;
                    ether_buf[iOfset++] = 0x00;
                    ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                    ether_buf[iOfset++] = pPtpClock->vlanId;
                }
                
                ether_buf[iOfset++] = 0x88;
                ether_buf[iOfset++] = 0xf7;
                
                memcpy(ether_buf + iOfset,pPtpClock->msgObuf,PDELAY_REQ_LENGTH);
                iOfset += PDELAY_REQ_LENGTH;
    
                netInitIeeeAddress(&Ieee_addr,pNetComm->P2PMulticastMac,pPtpClock,pNetComm->IeeeP2PSock);
                
                ret = sendto(pNetComm->IeeeP2PSock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
                if (ret <= 0)
                 {
                     PLOG("Error sending PDelayReq Muticast message\n");
                 }
                 else
                 {
                     pPtpClock->sentPDelayReqSequenceId++;

                     
                     /**从时钟，多播模式下，获得发送时间戳T3  */
                     //GetSendMessageTime(&pPtpClock->T3);
                     GetFpgaSendMessageTime(pPtpClock,&pPtpClock->T3);
                     PLOG("send PDelayReq Success!!\n");
                
                 }

                
            }
            
        }
        /** UDP IP V4 模式下 */
        if(pPtpClock->protoType == PROTO_UDP_IP)
        {
            if(pPtpClock->modeType == IPMODE_UNICAST)
            {
                if(!GetSendDelayServer_Ip(&pPtpClock->unicastMultiServer,&pNetComm->unicastAddr))
                    return;
                
                LOG(pPtpClock->debugLevel,"%s ==> Issue PDelayReq (%d)\n",pPtpClock->netEnviroment.ifaceName, pPtpClock->sentPDelayReqSequenceId);

                addr.sin_addr.s_addr = pNetComm->unicastAddr;
                *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
                ret = sendto(pNetComm->eventSock, pPtpClock->msgObuf, PDELAY_REQ_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
                if (ret <= 0)
                 {
                     PLOG("Error sending PDelayReq message\n");
                 }
                 else
                 {
                     //pPtpClock->sentPDelayReqSequenceId++;
                     //GetSendMessageTime(&internalTime);
                     GetFpgaSendMessageTime(pPtpClock,&internalTime);
                     StoreT3InBestAndUnicastlist(pPtpClock,internalTime,&pNetComm->unicastAddr);
                     PLOG("send PDelayReq Success!!\n");
                
                 }

    
            }
            if(pPtpClock->modeType == IPMODE_MULTICAST)
            {
                PLOG("==> Issue PDelayReq (%d)\n", pPtpClock->sentPDelayReqSequenceId);

                addr.sin_addr.s_addr = inet_addr(PTP_PEER_MULTICAST_ADDRESS);

                if(!netSetMulticastTTL(pNetComm->PeventSock,1))
                {
                    PLOG("set TTL error\n");
                }
         
                ret = sendto(pNetComm->PeventSock, pPtpClock->msgObuf, PDELAY_REQ_LENGTH, 0, 
                         (struct sockaddr *)&addr, 
                         sizeof(struct sockaddr_in));
                if (ret <= 0)
                 {
                     PLOG("Error sending PDelayReq Muticast message\n");
                 }
                 else
                 {
                     pPtpClock->sentPDelayReqSequenceId++;
 
                     /**从时钟，多播模式下，获得发送时间戳T3  */
                     //GetSendMessageTime(&pPtpClock->T3);
                     GetFpgaSendMessageTime(pPtpClock,&pPtpClock->T3);
                     PLOG("send PDelayReq Success!!\n");
                
                 }
    
            }
        }

}

void issueDelayResp(const TimeInternal *tint,MsgHeader *header,PtpClock *pPtpClock,void *pAddr)
{
    ssize_t ret;
    int iOfset;
    struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    Uint8 ether_buf[IEEE802_HEAD_LEN + DELAY_RESP_LENGTH];

    memset(ether_buf,0,sizeof(ether_buf));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PTP_GENERAL_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;

    /** 请求收到的时间 */
	Timestamp requestReceiptTimestamp;

	/*判断收到delayreq 是否与本地模式相同 */
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if (pPtpClock->modeType == IPMODE_UNICAST)
        {
            if((header->flagField0 & PTP_UNICAST) == PTP_UNICAST)
            {}
            else
            {
                PLOG("Mode Error\n");
                return;
            }

        }
        else if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            if((header->flagField0 & PTP_UNICAST) == PTP_UNICAST)
            {
                PLOG("Mode Error\n");
                return;
            }else
            {}

        }

    }

    /** 获取时间 */
	fromInternalTime(tint,&requestReceiptTimestamp);
    //memset(&requestReceiptTimestamp,0,sizeof(Timestamp));

    /**准备发送buffer  */
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
	msgPackDelayResp(pPtpClock->msgObuf,header,&requestReceiptTimestamp,pPtpClock);

    LOG(pPtpClock->debugLevel,"%s ==> Issue DelayResp (%d)\n",pPtpClock->netEnviroment.ifaceName, header->sequenceId);

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            
            memcpy(ether_buf + iOfset,(Uint8 *)pAddr,MAC_ADDR_LENGTH);
            iOfset +=MAC_ADDR_LENGTH;
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,MAC_ADDR_LENGTH);
            iOfset += MAC_ADDR_LENGTH;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }

            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,DELAY_RESP_LENGTH);
            iOfset += DELAY_RESP_LENGTH;

            netInitIeeeAddress(&Ieee_addr,(Uint8 *)pAddr,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending DelayResp message\n");
            }
            else
            {
                PLOG("send DelayResp Success!!\n");
            }
            
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            memcpy(ether_buf + iOfset,pNetComm->E2EMulticastMac,sizeof(pNetComm->E2EMulticastMac));
            iOfset += sizeof(pNetComm->E2EMulticastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
            
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,DELAY_RESP_LENGTH);
            iOfset += DELAY_RESP_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->E2EMulticastMac,pPtpClock,pNetComm->IeeeE2ESock);
            
            ret = sendto(pNetComm->IeeeE2ESock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
             {
                 PLOG("Error sending DelayResp Muticast message\n");
             }
             else
             {
                 PLOG("send DelayResp Success!!\n");
            
             }

        }
        
    }
    /** UDP IP V4 模式下 */
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            addr.sin_addr.s_addr = *(Uint32 *)pAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, DELAY_RESP_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
             {
                 PLOG("Error sending DelayResp message\n");
             }
             else
             {
                 PLOG("send DelayResp Success!!\n");
            
             }

        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            addr.sin_addr.s_addr = inet_addr(PTP_E2E_MULTICAST_ADDRESS);
        #if 0
            if(netPath->ttlEvent != rtOpts->ttl)
            {
                /* set socket time-to-live  */
                if (netSetMulticastTTL(netPath->eventSock,rtOpts->ttl))
                {
                    netPath->ttlEvent = rtOpts->ttl;
                }
            }
        #endif
            
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, DELAY_RESP_LENGTH, 0, 
                     (struct sockaddr *)&addr, 
                     sizeof(struct sockaddr_in));
            if (ret <= 0)
             {
                 PLOG("Error sending DelayResp Muticast message\n");
             }
             else
             {
                 PLOG("send DelayResp Success!!\n");
             }

        }
    }

}

void issuePDelayResp(const TimeInternal *tint,MsgHeader *header,PtpClock *pPtpClock,void *pAddr)
{
    ssize_t ret;
    int iOfset;
    struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    Uint8 ether_buf[IEEE802_HEAD_LEN + PDELAY_RESP_LENGTH];

    memset(ether_buf,0,sizeof(ether_buf));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PTP_EVENT_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;

    /** 请求收到的时间 */
    Timestamp requestReceiptTimestamp;
    TimeInternal responseOriginTimeInternal;
    
	/*判断收到delayreq 是否与本地模式相同 */
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if (pPtpClock->modeType == IPMODE_UNICAST)
        {
            if((header->flagField0 & PTP_UNICAST) == PTP_UNICAST)
            {
            }
            else
            {
                PLOG("Mode Error\n");
                return;
            }
        }
        else if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            if((header->flagField0 & PTP_UNICAST) == PTP_UNICAST)
            {
                PLOG("Mode Error\n");
                return;
            }
            else
            {
            }
        
        }

    }

    /** 获取时间 */
	fromInternalTime(tint,&requestReceiptTimestamp);

    /**准备发送buffer  */
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
	msgPackPDelayResp(pPtpClock->msgObuf,header,&requestReceiptTimestamp,
			 pPtpClock);

    LOG(pPtpClock->debugLevel,"%s ==> Issue PDelayResp (%d)\n",pPtpClock->netEnviroment.ifaceName, header->sequenceId);

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            memcpy(ether_buf + iOfset,(Uint8*)pAddr,MAC_ADDR_LENGTH);
            iOfset += MAC_ADDR_LENGTH;
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,MAC_ADDR_LENGTH);
            iOfset += MAC_ADDR_LENGTH;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
            
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,PDELAY_RESP_LENGTH);
            iOfset += PDELAY_RESP_LENGTH;

            netInitIeeeAddress(&Ieee_addr,(Uint8 *)pAddr,pPtpClock,pNetComm->IeeeP2PSock);
            
            ret = sendto(pNetComm->IeeeP2PSock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending PDelayResp message\n");
            }
            else
            {
                PLOG("send PDelayResp Success!!\n");
            }
            
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            memcpy(ether_buf + iOfset,pNetComm->P2PMulticastMac,sizeof(pNetComm->P2PMulticastMac));
            iOfset += sizeof(pNetComm->P2PMulticastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;

            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
            
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,PDELAY_RESP_LENGTH);
            iOfset += PDELAY_RESP_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->P2PMulticastMac,pPtpClock,pNetComm->IeeeP2PSock);
            
            ret = sendto(pNetComm->IeeeP2PSock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
             {
                 PLOG("Error sending PDelayResp Muticast message\n");
             }
             else
             {
                 PLOG("send PDelayResp Success!!\n");
            
             }

        }
        
    }
    /** UDP IP V4 模式下 */
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            addr.sin_addr.s_addr = *(Uint32 *)pAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->eventSock, pPtpClock->msgObuf, PDELAY_RESP_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
             {
                 PLOG("Error sending PDelayResp message\n");
             }
             else
             {
                 PLOG("send PDelayResp Success!!\n");
            
             }

        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            addr.sin_addr.s_addr = inet_addr(PTP_PEER_MULTICAST_ADDRESS);
            
            if(!netSetMulticastTTL(pNetComm->PeventSock,1))
            {
                PLOG("set TTL error\n");
            }
            
            ret = sendto(pNetComm->PeventSock, pPtpClock->msgObuf, PDELAY_RESP_LENGTH, 0, 
                     (struct sockaddr *)&addr, 
                     sizeof(struct sockaddr_in));
            if (ret <= 0)
             {
                 PLOG("Error sending PDelayResp Muticast message\n");
             }
             else
             {
                 PLOG("send PDelayResp Success!!\n");
             }

        }
    }

    if(pPtpClock->stepType == TWO_STEP)
    {
        //GetSendMessageTime(&responseOriginTimeInternal);
        GetFpgaSendMessageTime(pPtpClock,&responseOriginTimeInternal);
        issuePDelayRespFollowUp(tint,&responseOriginTimeInternal,header,pPtpClock,header->sequenceId,pAddr);
    }
}


void issuePDelayRespFollowUp(const TimeInternal *requetReceipt,const TimeInternal *responseOrigin,MsgHeader *header,PtpClock *pPtpClock, UInteger16 sequenceId,void *pAddr)
{

	ssize_t ret;
    int iOfset;
    
	struct sockaddr_in addr;
    struct sockaddr_ll  Ieee_addr;
    Uint8 ether_buf[IEEE802_HEAD_LEN + PDELAY_RESP_FOLLOW_UP_LENGTH];

    
    memset(ether_buf,0,sizeof(ether_buf));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PTP_GENERAL_PORT);

    NetComm *pNetComm = &pPtpClock->netComm;
    iOfset = 0;

	Timestamp responseOriginTimestamp;
	fromInternalTime(responseOrigin,&responseOriginTimestamp);

    
    /**准备发送buffer  */
    memset(pPtpClock->msgObuf,0,sizeof(pPtpClock->msgObuf));
	msgPackPDelayRespFollowUp(pPtpClock->msgObuf,header,requetReceipt,&responseOriginTimestamp,pPtpClock,sequenceId);	

    LOG(pPtpClock->debugLevel,"%s ==> Issue PDelayRespFollowup (%d)\n",sequenceId);


    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            memcpy(ether_buf + iOfset,(Uint8 *)pAddr,MAC_ADDR_LENGTH);
            iOfset += MAC_ADDR_LENGTH;
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,MAC_ADDR_LENGTH);
            iOfset += MAC_ADDR_LENGTH;
            
            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }
            
            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,PDELAY_RESP_FOLLOW_UP_LENGTH);
            iOfset += PDELAY_RESP_FOLLOW_UP_LENGTH;

            netInitIeeeAddress(&Ieee_addr,(Uint8*)pAddr,pPtpClock,pNetComm->IeeeP2PSock);
            
            ret = sendto(pNetComm->IeeeP2PSock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending PDelayRespFollowUp message\n");
            }
            else
            {
                PLOG("send  PDelayRespFollowUp Msg Success!!\n");
            }

            
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            memcpy(ether_buf + iOfset,pNetComm->P2PMulticastMac,sizeof(pNetComm->P2PMulticastMac));
            iOfset += sizeof(pNetComm->P2PMulticastMac);
            
            memcpy(ether_buf + iOfset,pPtpClock->netEnviroment.mac,6);
            iOfset += 6;
            
            if(pPtpClock->vlanEnable)
            {
                ether_buf[iOfset++] = 0x81;
                ether_buf[iOfset++] = 0x00;
                ether_buf[iOfset++] = (pPtpClock->vlanPriority<<5) + (pPtpClock->vlanCfi<<4);
                ether_buf[iOfset++] = pPtpClock->vlanId;
            }

            ether_buf[iOfset++] = 0x88;
            ether_buf[iOfset++] = 0xf7;
            
            memcpy(ether_buf + iOfset,pPtpClock->msgObuf,PDELAY_RESP_FOLLOW_UP_LENGTH);
            iOfset += PDELAY_RESP_FOLLOW_UP_LENGTH;

            netInitIeeeAddress(&Ieee_addr,pNetComm->P2PMulticastMac,pPtpClock,pNetComm->IeeeP2PSock);
            
            ret = sendto(pNetComm->IeeeP2PSock, ether_buf, iOfset, 0,(struct sockaddr *)&Ieee_addr,sizeof(struct sockaddr_ll));
            if (ret <= 0)
            {
                PLOG("Error sending PDelayRespFollowUp Muticast message\n");
            }
            else
            {
                PLOG("send  PDelayRespFollowUp Msg Success!!\n");
            }

            
        }
        
    }
    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        if(pPtpClock->modeType == IPMODE_UNICAST)
        {
            addr.sin_addr.s_addr = *(Uint32 *)pAddr;
            *(char *)(pPtpClock->msgObuf + 6) |= PTP_UNICAST;
            ret = sendto(pNetComm->generalSock, pPtpClock->msgObuf, PDELAY_RESP_FOLLOW_UP_LENGTH, 0,(struct sockaddr *)&addr,sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending PDelayRespFollowUp message\n");
            }
            else
            {
                PLOG("send  PDelayRespFollowUp Msg Success!!\n");
            }

            
        }
        if(pPtpClock->modeType == IPMODE_MULTICAST)
        {
            addr.sin_addr.s_addr = inet_addr(PTP_PEER_MULTICAST_ADDRESS);
        #if 0
            if(netPath->ttlEvent != rtOpts->ttl)
            {
                /* set socket time-to-live  */
                if (netSetMulticastTTL(netPath->eventSock,rtOpts->ttl))
                {
                    netPath->ttlEvent = rtOpts->ttl;
                }
            }
        #endif
            
            ret = sendto(pNetComm->PgeneralSock, pPtpClock->msgObuf, PDELAY_RESP_FOLLOW_UP_LENGTH, 0, 
                     (struct sockaddr *)&addr, 
                     sizeof(struct sockaddr_in));
            if (ret <= 0)
            {
                PLOG("Error sending PDelayRespFollowUp Muticast message\n");
            }
            else
            {
                PLOG("send  PDelayRespFollowUp Msg Success!!\n");
            }

        }
    }

}

