/********************************************************************************************
*                           ��Ȩ����(C) 2015, ���ſ�ѧ���������о���
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       ptp_proto.c
*    ��������:       ��Ϣ�����߳�ģ����ڣ���ӡ��ʾ����
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-1-30
*    �����б�:
                     clockIdentity_display
                     clockQuality_display
                     iFaceName_display
                     integer64_display
                     isFromBestMaster
                     isFromMySelfMaster
                     MasterMode_Recv
                     MasterMode_Send
                     msgAnnounce_display
                     msgDelayReq_display
                     msgDelayResp_display
                     msgFollowUp_display
                     msgHeader_display
                     msgPDelayReq_display
                     msgPDelayRespFollowUp_display
                     msgPDelayResp_display
                     msgSync_display
                     portIdentity_display
                     ProcessAnnounceMessage
                     ProcessDelayReqMessage
                     ProcessDelayRespMessage
                     ProcessFollowUpMessage
                     ProcessIeee802_Msg
                     ProcessPdelayReqMessage
                     ProcessPdelayRespFollowUpMessage
                     ProcessPdelayRespMessage
                     ProcessSignalMessage
                     ProcessSynMessage
                     Process_Msg
                     SlaveMode_Recv
                     SlaveMode_Send
                     timeInternal_display
                     timestamp_display
                     uInteger48_display
                     unicast_display
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-1-30
                     ����: HuangFei
                     �޸�����: �´����ļ�


*********************************************************************************************/

#include "ptp_proto.h"
#include "arith.h"
#include "msg.h"
#include "unicast_process.h"
#include "bmc.h"
#include "fpga_time.h"
#include "comm_fun.h"

/********************************************************************************************
* ��������:    integer64_display
* ��������:    ��ӡ��ʾ64δʱ��
* �������:    const Integer64 * bigint  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void integer64_display(const Integer64 * bigint)
{
	printf("Integer 64 : \n");
	printf("LSB : %u\n", bigint->lsb);
	printf("MSB : %d\n", bigint->msb);
}


/********************************************************************************************
* ��������:    uInteger48_display
* ��������:    ��ӡ��ʾ48λʱ��
* �������:    const UInteger48 * bigint  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void uInteger48_display(const UInteger48 * bigint)
{
	printf("Integer 48 : \n");
	printf("LSB : %u\n", bigint->lsb);
	printf("MSB : %u\n", bigint->msb);
}


/********************************************************************************************
* ��������:    timeInternal_display
* ��������:    ��ӡTimerInterval
* �������:    const TimeInternal * timeInternal  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void timeInternal_display(const TimeInternal * timeInternal)
{
	printf("seconds : %d \n", timeInternal->seconds);
	printf("nanoseconds %d \n", timeInternal->nanoseconds);
}


/********************************************************************************************
* ��������:    timestamp_display
* ��������:    ��ӡTimestamp
* �������:    const Timestamp * timestamp  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void timestamp_display(const Timestamp * timestamp)
{
	uInteger48_display(&timestamp->secondsField);
	printf("nanoseconds %u \n", timestamp->nanosecondsField);
}


/********************************************************************************************
* ��������:    clockIdentity_display
* ��������:    ʱ��Identity ��ӡ��ʾ
* �������:    const ClockIdentity clockIdentity  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void clockIdentity_display(const ClockIdentity clockIdentity)
{
	printf(
	    "ClockIdentity : %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n",
	    clockIdentity[0], clockIdentity[1], clockIdentity[2],
	    clockIdentity[3], clockIdentity[4], clockIdentity[5],
	    clockIdentity[6], clockIdentity[7]
	);
}

/********************************************************************************************
* ��������:    portIdentity_display
* ��������:    PortIndentity ��ӡ��ʾ
* �������:    const PortIdentity * portIdentity  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void portIdentity_display(const PortIdentity * portIdentity)
{
	clockIdentity_display(portIdentity->clockIdentity);
	printf("port number : %d \n", portIdentity->portNumber);
}

/********************************************************************************************
* ��������:    clockQuality_display
* ��������:    ʱ��������ӡ��ʾ
* �������:    const ClockQuality * clockQuality  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void clockQuality_display(const ClockQuality * clockQuality)
{
	printf("clockClass : %d \n", clockQuality->clockClass);
	printf("clockAccuracy : %d \n", clockQuality->clockAccuracy);
	printf("offsetScaledLogVariance : %d \n", clockQuality->offsetScaledLogVariance);
}


/********************************************************************************************
* ��������:    iFaceName_display
* ��������:    ��������ӡ��ʾ
* �������:    const Octet * iFaceName  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void iFaceName_display(const Octet * iFaceName)
{

	int i;

	printf("iFaceName : ");

	for (i = 0; i < IFACE_NAME_LENGTH; i++) {
		printf("%c", iFaceName[i]);
	}
	printf("\n");

}


/********************************************************************************************
* ��������:    unicast_display
* ��������:    ������ַ��ӡ��ʾ
* �������:    const Octet * unicast  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void unicast_display(const Octet * unicast)
{

	int i;

	printf("Unicast adress : ");

	for (i = 0; i < NET_ADDRESS_LENGTH; i++) {
		printf("%c", unicast[i]);
	}
	printf("\n");

}





/********************************************************************************************
* ��������:    msgHeader_display
* ��������:    ��ӡ��ϢHead ͷ
* �������:    const MsgHeader * header  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgHeader_display(const MsgHeader * header)
{
	printf("Message header : \n");
	printf("\n");
	printf("transportSpecific : %d\n", header->transportSpecific);
	printf("messageType : %d\n", header->messageType);
	printf("versionPTP : %d\n", header->versionPTP);
	printf("messageLength : %d\n", header->messageLength);
	printf("domainNumber : %d\n", header->domainNumber);
	printf("FlagField %02hhx:%02hhx\n", header->flagField0, header->flagField1);
	printf("CorrectionField : \n");
	integer64_display(&header->correctionField);
	printf("SourcePortIdentity : \n");
	portIdentity_display(&header->sourcePortIdentity);
	printf("sequenceId : %d\n", header->sequenceId);
	printf("controlField : %d\n", header->controlField);
	printf("logMessageInterval : %d\n", header->logMessageInterval);
	printf("\n");
}

/********************************************************************************************
* ��������:    msgSync_display
* ��������:    ��ӡsynʱ����Ϣ
* �������:    const MsgSync * sync  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgSync_display(const MsgSync * sync)
{
	printf("Message Sync : \n");
	timestamp_display(&sync->originTimestamp);
	printf("\n");
}

/********************************************************************************************
* ��������:    msgAnnounce_display
* ��������:    ��ӡ�յ���Announce��Ϣ
* �������:    const MsgAnnounce * announce  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgAnnounce_display(const MsgAnnounce * announce)
{
	printf("Announce Message : \n");
	printf("\n");
	printf("originTimestamp : \n");
	printf("secondField  : \n");
	timestamp_display(&announce->originTimestamp);
	printf("currentUtcOffset : %d \n", announce->currentUtcOffset);
	printf("grandMasterPriority1 : %d \n", announce->grandmasterPriority1);
	printf("grandMasterClockQuality : \n");
	clockQuality_display(&announce->grandmasterClockQuality);
	printf("grandMasterPriority2 : %d \n", announce->grandmasterPriority2);
	printf("grandMasterIdentity : \n");
	clockIdentity_display(announce->grandmasterIdentity);
	printf("stepsRemoved : %d \n", announce->stepsRemoved);
	printf("timeSource : %d \n", announce->timeSource);
	printf("\n");
}

/********************************************************************************************
* ��������:    msgFollowUp_display
* ��������:    ��ӡ�յ���FollowUpʱ����Ϣ
* �������:    const MsgFollowUp * follow  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgFollowUp_display(const MsgFollowUp * follow)
{
	timestamp_display(&follow->preciseOriginTimestamp);
}


/********************************************************************************************
* ��������:    msgDelayReq_display
* ��������:    ��ӡ�յ���DelayReqʱ����Ϣ
* �������:    const MsgDelayReq * req  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgDelayReq_display(const MsgDelayReq * req)
{
	timestamp_display(&req->originTimestamp);
}


/********************************************************************************************
* ��������:    msgDelayResp_display
* ��������:    ��ӡ�յ���DelayRespʱ����Ϣ
* �������:    const MsgDelayResp * resp  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgDelayResp_display(const MsgDelayResp * resp)
{
	timestamp_display(&resp->receiveTimestamp);
	portIdentity_display(&resp->requestingPortIdentity);
}

/********************************************************************************************
* ��������:    msgPDelayReq_display
* ��������:    ��ӡPdelayReq
* �������:    const MsgPDelayReq * preq  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgPDelayReq_display(const MsgPDelayReq * preq)
{
	timestamp_display(&preq->originTimestamp);
}


/********************************************************************************************
* ��������:    msgPDelayResp_display
* ��������:    ��ӡPdelayResp ��Ϣ
* �������:    const MsgPDelayResp * presp  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgPDelayResp_display(const MsgPDelayResp * presp)
{
	timestamp_display(&presp->requestReceiptTimestamp);
	portIdentity_display(&presp->requestingPortIdentity);
}


/********************************************************************************************
* ��������:    msgPDelayRespFollowUp_display
* ��������:    ��ӡPdelayRespFollowUp��Ϣ
* �������:    const MsgPDelayRespFollowUp * prespfollow  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void msgPDelayRespFollowUp_display(const MsgPDelayRespFollowUp * prespfollow)
{
	timestamp_display(&prespfollow->responseOriginTimestamp);
	portIdentity_display(&prespfollow->requestingPortIdentity);
}

/********************************************************************************************
* ��������:    isFromBestMaster
* ��������:    �ж��Ƿ��������ʱ��
* �������:    const PtpClock *ptpClock  
               const MsgHeader* header   
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Boolean isFromBestMaster(const PtpClock *ptpClock, const MsgHeader* header)
{

	return(!memcmp(
		ptpClock->parentPortIdentity.clockIdentity,
		header->sourcePortIdentity.clockIdentity,
		CLOCK_IDENTITY_LENGTH)	&& 
		(ptpClock->parentPortIdentity.portNumber ==
		 header->sourcePortIdentity.portNumber));
}

/********************************************************************************************
* ��������:    isFromMySelfMaster
* ��������:    �ж��Ƿ��������Լ����İ�����Ƿ����棬���Ƿ��ؼ�
* �������:    const PtpClock *ptpClock  
               const MsgHeader* header   
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Boolean isFromMySelfMaster(const PtpClock *ptpClock, const MsgHeader* header)
{

	return(!memcmp(
		ptpClock->portIdentity.clockIdentity,
		header->sourcePortIdentity.clockIdentity,
		CLOCK_IDENTITY_LENGTH)	&& 
		(ptpClock->portIdentity.portNumber ==
		 header->sourcePortIdentity.portNumber));

}

Boolean isFromSameDomain(const PtpClock *pPtpClock, const MsgHeader* header)
{
    if(pPtpClock->domainNumber == header->domainNumber)
        return TRUE;
    else
        return FALSE;
}


void GetUniMessageInterval(PtpClock *pPtpClock,UnicastMultiServer *pServer_unicast,void *addr)
{
    int nIndex;

    nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
    
    if(nIndex<0)
        return;
    
    pPtpClock->logSyncInterval = pServer_unicast->serverList[nIndex].logInterMessagePeriod;


}

void CalCulate_Delay_TimeOffsetDelayPath(PtpClock *pPtpClock
                                                ,TimeInternal T1
                                                ,TimeInternal T2
                                                ,TimeInternal T3
                                                ,TimeInternal T4
                                                ,TimeInternal synCorrection
                                                ,TimeInternal followCorrection
                                                ,TimeInternal delayrespCorrection
                                                ,TimeInternal *pMeanPathDelay
                                                ,TimeInternal *pTimeOffset)
{
    TimeInternal T2_T1;
    TimeInternal T4_T3;
    
    if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
    {
        if(pPtpClock->stepType == ONE_STEP)
        {
            /**  ����delaymeanPath: ( (t4-t3)+ (t2-t1) - syncorrection -delayrespcorrection) /2 */ 
            subTime(&T2_T1,&T2,&T1);
            //subTime(&T4_T3,&T4,&T3);
            addTime(pMeanPathDelay,&pPtpClock->SMDelay,&T2_T1);
            subTime(pMeanPathDelay,pMeanPathDelay,&synCorrection);
            subTime(pMeanPathDelay,pMeanPathDelay,&delayrespCorrection);
            
            div2Time(pMeanPathDelay);
            
             /**����TimeOffset  T2-T1 - delaymeanPath - synCorrection */

            subTime(&T2_T1,&T2,&T1);
            subTime(pTimeOffset,&T2_T1,pMeanPathDelay);
            subTime(pTimeOffset,pTimeOffset,&synCorrection);
            
        }
        else if(pPtpClock->stepType == TWO_STEP)
        {
            /**  ����delaymeanPath: ( (t4-t3)+ (t2-t1) - syncorrection -followupcorrection-delayrespcorrection) /2 */ 
            subTime(&T2_T1,&T2,&T1);
            //subTime(&T4_T3,&T4,&T3);
            addTime(pMeanPathDelay,&pPtpClock->SMDelay,&T2_T1);
            
            subTime(pMeanPathDelay,pMeanPathDelay,&synCorrection);
            subTime(pMeanPathDelay,pMeanPathDelay,&followCorrection);
            subTime(pMeanPathDelay,pMeanPathDelay,&delayrespCorrection);
            
            div2Time(pMeanPathDelay);
            
             /**����TimeOffset  T2-T1 - delaymeanPath - synCorrection -followupcorrection*/
            subTime(&T2_T1,&T2,&T1);
            subTime(pTimeOffset,&T2_T1,pMeanPathDelay);
            subTime(pTimeOffset,pTimeOffset,&synCorrection);
            subTime(pTimeOffset,pTimeOffset,&followCorrection);
            
        }
#if 1
                         
        printf("==>meanpathdelay seconds:%d nanoseconds:%d\n",pMeanPathDelay->seconds,pMeanPathDelay->nanoseconds);
              
        printf("==>TimeOffset seconds:%d nanoseconds:%d\n",pTimeOffset->seconds,pTimeOffset->nanoseconds);
                
#endif


    }
    else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
    {
        printf("CalCulate_Delay_TimeOffsetDelayPath error!!\n");
    }
}

void CalCulate_PDelay_TimeOffsetDelayPath(PtpClock *pPtpClock
                                                ,TimeInternal T1
                                                ,TimeInternal T2
                                                ,TimeInternal T3
                                                ,TimeInternal T6
                                                ,TimeInternal synCorrection
                                                ,TimeInternal followCorrection
                                                ,TimeInternal pdelayrespCorrection
                                                ,TimeInternal pdelayrespfollowupCorrection
                                                ,TimeInternal *pMeanPathDelay
                                                ,TimeInternal *pTimeOffset)
{
    TimeInternal T2_T1;
    TimeInternal T6_T3;
    TimeInternal T5_T4;
        
    if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
    {
        printf("CalCulate_PDelay_TimeOffsetDelayPath error\n");
    }
    else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
    {
        if(pPtpClock->stepType == ONE_STEP)
        {
            /**����meanpath ( t6-t3 - pdelayrespCorrection) /2*/
            //subTime(&T6_T3,&T6,&T3);
            subTime(pMeanPathDelay,&pPtpClock->SMDelay,&pdelayrespCorrection);
            div2Time(pMeanPathDelay);
            
            /**����TimeOffset  T2-T1 - delaymeanPath - synCorrection */
            subTime(&T2_T1,&T2,&T1);
            subTime(pTimeOffset,&T2_T1,pMeanPathDelay);
            subTime(pTimeOffset,pTimeOffset,&synCorrection);

        }
        else if(pPtpClock->stepType == TWO_STEP)
        {
            /**����meanpath ( t6-t3 - (T5-T4)-pdelayrespCorrection - pdelayrespfollowupCorrection) /2*/
            //subTime(&T6_T3,&T6,&T3);
            //subTime(&T5_T4,&pPtpClock->T5,&pPtpClock->T4);
            
            subTime(pMeanPathDelay,&pPtpClock->SMDelay,&pPtpClock->SMDelay2);
            subTime(pMeanPathDelay,pMeanPathDelay,&pdelayrespCorrection);
            subTime(pMeanPathDelay,pMeanPathDelay,&pdelayrespfollowupCorrection);
            div2Time(pMeanPathDelay);
            
           /**����TimeOffset  T2-T1 - delaymeanPath - synCorrection -followupcorrection*/
            subTime(&T2_T1,&T2,&T1);
            subTime(pTimeOffset,&T2_T1,pMeanPathDelay);
            subTime(pTimeOffset,pTimeOffset,&synCorrection);
            subTime(pTimeOffset,pTimeOffset,&followCorrection);           
            
        }

#if 1
                 
        printf("==>meanpathdelay seconds:%d nanoseconds:%d\n",pMeanPathDelay->seconds,pMeanPathDelay->nanoseconds);
              
        printf("==>TimeOffset seconds:%d nanoseconds:%d\n",pTimeOffset->seconds,pTimeOffset->nanoseconds);
#endif

    }
}


void HandleSynMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgSync *pMsgSyn)
{
    PLOG("==> Handle Syn (%d)\n",pMsgHeader->sequenceId);
    pPtpClock->recvSynFlag = TRUE;

    if(IsTwoStep(pMsgHeader))
    {
        if(pPtpClock->stepType == ONE_STEP)
            pPtpClock->stepType = TWO_STEP;
    }
    else
    {
        if(pPtpClock->stepType == TWO_STEP)
            pPtpClock->stepType = ONE_STEP;
    }

    if(pPtpClock->modeType == IPMODE_MULTICAST)
        pPtpClock->logSyncInterval = pMsgHeader->logMessageInterval;

    //GetCnt_threshold(pPtpClock);
    
    /**���T1  */
    if(pPtpClock->stepType == ONE_STEP)
    {
        pPtpClock->T1.seconds = pMsgSyn->originTimestamp.secondsField.lsb;
        pPtpClock->T1.nanoseconds = pMsgSyn->originTimestamp.nanosecondsField;
        integer64_to_internalTime(pMsgHeader->correctionField,&pPtpClock->SynCorrection);
    }

    /**���T2  */
    GetFpgaRecvMessageTime(pPtpClock,pMsgHeader,&pPtpClock->T2);

    if(pPtpClock->stepType == ONE_STEP)
    {
        //if(pPtpClock->finishT3T4)
        {
            if(pPtpClock->debugLevel)
            {
                #if 1
                if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
                {
                    printf("T1.sec=%d,T1.nao=%d\n",pPtpClock->T1.seconds,pPtpClock->T1.nanoseconds);
                    printf("T2.sec=%d,T2.nao=%d\n",pPtpClock->T2.seconds,pPtpClock->T2.nanoseconds);
                    printf("T3.sec=%d,T3.nao=%d\n",pPtpClock->T3.seconds,pPtpClock->T3.nanoseconds);
                    printf("T4.sec=%d,T4.nao=%d\n",pPtpClock->T4.seconds,pPtpClock->T4.nanoseconds);

                    printf("syn correction=%d\n",pPtpClock->SynCorrection.nanoseconds);
                    printf("folloUp correction=%d\n",pPtpClock->FollowUpCorrection.nanoseconds);
                    printf("delayresp correction=%d\n",pPtpClock->DelayrespCorrection.nanoseconds);

                }
                else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
                {
                    printf("T1.sec=%d,T1.nao=%d\n",pPtpClock->T1.seconds,pPtpClock->T1.nanoseconds);
                    printf("T2.sec=%d,T2.nao=%d\n",pPtpClock->T2.seconds,pPtpClock->T2.nanoseconds);
                    printf("T3.sec=%d,T3.nao=%d\n",pPtpClock->T3.seconds,pPtpClock->T3.nanoseconds);
                    printf("T6.sec=%d,T6.nao=%d\n",pPtpClock->T6.seconds,pPtpClock->T6.nanoseconds);

                    printf("syn correction=%d\n",pPtpClock->SynCorrection.nanoseconds);
                    printf("folloUp correction=%d\n",pPtpClock->FollowUpCorrection.nanoseconds);

                    if(pPtpClock->stepType == TWO_STEP)
                    {
                        printf("pdelayresp correction=%d\n",pPtpClock->PdelayrespCorrection.nanoseconds);
                        printf("pdelayrespfollowup correction=%d\n",pPtpClock->PdelayrespFollowUpCorrection.nanoseconds);
                    }
                    else if(pPtpClock->stepType == ONE_STEP)
                    {
                        printf("pdelayresp correction=%d\n",pPtpClock->PdelayrespCorrection.nanoseconds);
                    }

                }
                #endif
            }

            
            if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
                CalCulate_Delay_TimeOffsetDelayPath(pPtpClock
                                                    ,pPtpClock->T1,pPtpClock->T2,pPtpClock->T3,pPtpClock->T4
                                                    ,pPtpClock->SynCorrection,pPtpClock->FollowUpCorrection,pPtpClock->DelayrespCorrection
                                                    ,&pPtpClock->MeanPathDelay,&pPtpClock->TimeOffset);
            
            else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
                CalCulate_PDelay_TimeOffsetDelayPath(pPtpClock
                                                    ,pPtpClock->T1,pPtpClock->T2,pPtpClock->T3,pPtpClock->T6
                                                    ,pPtpClock->SynCorrection,pPtpClock->FollowUpCorrection
                                                    ,pPtpClock->PdelayrespCorrection,pPtpClock->PdelayrespFollowUpCorrection
                                                    ,&pPtpClock->MeanPathDelay,&pPtpClock->TimeOffset);

            
            //AdustDp83640SynchronizeMaster(pPtpClock,&pPtpClock->phyControl);
            
            pPtpClock->finishPtp = TRUE;
            //pPtpClock->finishT3T4 = FALSE;
        }
    }
}

void HandleFollowUpMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgFollowUp *pMsgFollowUp)
{
    PLOG("==> Handle FollowUp (%d)\n",pMsgHeader->sequenceId);
    
    if(pPtpClock->stepType == TWO_STEP)
    {
        pPtpClock->T1.seconds = pMsgFollowUp->preciseOriginTimestamp.secondsField.lsb;
        pPtpClock->T1.nanoseconds = pMsgFollowUp->preciseOriginTimestamp.nanosecondsField;
        
        integer64_to_internalTime(pMsgHeader->correctionField,&pPtpClock->FollowUpCorrection);

        //if(pPtpClock->finishT3T4)
        {
            if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
                CalCulate_Delay_TimeOffsetDelayPath(pPtpClock,pPtpClock->T1,pPtpClock->T2,pPtpClock->T3,pPtpClock->T4
                                                    ,pPtpClock->SynCorrection,pPtpClock->FollowUpCorrection,pPtpClock->DelayrespCorrection
                                                    ,&pPtpClock->MeanPathDelay,&pPtpClock->TimeOffset);
            
            else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
                CalCulate_PDelay_TimeOffsetDelayPath(pPtpClock,pPtpClock->T1,pPtpClock->T2,pPtpClock->T3,pPtpClock->T6
                                                    ,pPtpClock->SynCorrection,pPtpClock->FollowUpCorrection
                                                    ,pPtpClock->PdelayrespCorrection,pPtpClock->PdelayrespFollowUpCorrection
                                                    ,&pPtpClock->MeanPathDelay,&pPtpClock->TimeOffset);

            //AdustDp83640SynchronizeMaster(pPtpClock,&pPtpClock->phyControl);

            //pPtpClock->finishT3T4 = FALSE;
        }
    }
}

void HandleDelayRespMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgDelayResp *pMsgDelayResp)
{
    pPtpClock->recvDelayRespFlag = TRUE;
    PLOG("==> Handle DelayResp (%d)\n",pMsgHeader->sequenceId);

    pPtpClock->T4.seconds = pMsgDelayResp->receiveTimestamp.secondsField.lsb;
    pPtpClock->T4.nanoseconds = pMsgDelayResp->receiveTimestamp.nanosecondsField;
    integer64_to_internalTime(pMsgHeader->correctionField,&pPtpClock->DelayrespCorrection);

    subTime(&pPtpClock->SMDelay,&pPtpClock->T4,&pPtpClock->T3);
    //pPtpClock->finishT3T4 = TRUE;
    

}


void HandlePdelayRespMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgPDelayResp *pMsgPDelayResp)
{
    pPtpClock->recvDelayRespFlag = TRUE;
    PLOG("==> Handle PDelayResp (%d)\n",pMsgHeader->sequenceId);

    GetFpgaRecvMessageTime(pPtpClock,pMsgHeader,&pPtpClock->T6);
    
    integer64_to_internalTime(pMsgHeader->correctionField,&pPtpClock->PdelayrespCorrection);

    if(pPtpClock->stepType == ONE_STEP)
    {
        subTime(&pPtpClock->SMDelay,&pPtpClock->T6,&pPtpClock->T3);
        //pPtpClock->finishT3T4 = TRUE;
    }
        
    else if(pPtpClock->stepType == TWO_STEP)
    {
        subTime(&pPtpClock->SMDelay,&pPtpClock->T6,&pPtpClock->T3);
        pPtpClock->T4.seconds = pMsgPDelayResp->requestReceiptTimestamp.secondsField.lsb;
        pPtpClock->T4.nanoseconds = pMsgPDelayResp->requestReceiptTimestamp.nanosecondsField;

    }

}

void HandlePdelayRespFollowUpMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,MsgPDelayRespFollowUp *pMsgPDelayRespFollowUp)
{
    PLOG("==> Handle PdelayRespFollowUp (%d)\n",pMsgHeader->sequenceId);

    integer64_to_internalTime(pMsgHeader->correctionField,&pPtpClock->PdelayrespFollowUpCorrection);

    if(pPtpClock->stepType == TWO_STEP)
    {
        subTime(&pPtpClock->SMDelay2,&pPtpClock->T5,&pPtpClock->T4);
        pPtpClock->T5.seconds = pMsgPDelayRespFollowUp->responseOriginTimestamp.secondsField.lsb;
        pPtpClock->T5.nanoseconds = pMsgPDelayRespFollowUp->responseOriginTimestamp.nanosecondsField;

        //pPtpClock->finishT3T4 = TRUE;
    }
        


}


/********************************************************************************************
* ��������:    HandleRequestSignal_SynMessage
* ��������:    �����յ���RequestSignal��Ϣ������syn��Ϣ���������д����������򷵻�Grant����
               ��æ����������ظ�Cancel
               
* �������:    PtpClock *pPtpClock       
               MsgHeader *pMsgHeader     
               SignalReq *pReqestSignal  
               void *addr                
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void HandleRequestSignal_SynMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,SignalReq *pReqestSignal,void *addr)
{
    int clientNum;
    int client_index;

    clientNum = CalculateConnectedClient(pPtpClock);

    if(clientNum >= MAX_CLIENT)
    {
        issueCancelSignaling(pPtpClock,SIGNAL_SYN_MSG,pMsgHeader,pReqestSignal,addr);
    }
    else
    {
        client_index = WhetherAddressInClient(pPtpClock,addr);
        if(client_index >= 0)
        {
            pPtpClock->unicastClient.clientList[client_index].send_sync_enable = TRUE;
            issueGrantSignaling(pPtpClock,SIGNAL_SYN_MSG,pMsgHeader,pReqestSignal,addr);
        }
    }
}
/********************************************************************************************
* ��������:    HandleRequestSignal_AnnounceMessage
* ��������:    �����յ���RequestSignal��Ϣ������announce ��Ϣ���������д����������򷵻�Grant����
               ��æ����������ظ�Cancel
               
* �������:    PtpClock *pPtpClock       
               MsgHeader *pMsgHeader     
               SignalReq *pReqestSignal  
               void *addr                
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void HandleRequestSignal_AnnounceMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,SignalReq *pReqestSignal,void *addr)
{
    int clientNum;
    int client_index;

    clientNum = CalculateConnectedClient(pPtpClock);
    
    if(clientNum >= MAX_CLIENT)
    {
        issueCancelSignaling(pPtpClock,SIGNAL_ANNOUNCE_MSG,pMsgHeader,pReqestSignal,addr);
    }
    else
    {
        RecvMsg_UpdateUnicastClient(pPtpClock,addr,pReqestSignal->durationField+10);
        client_index = WhetherAddressInClient(pPtpClock,addr);
        pPtpClock->unicastClient.clientList[client_index].send_announce_enable = TRUE;
        issueGrantSignaling(pPtpClock,SIGNAL_ANNOUNCE_MSG,pMsgHeader,pReqestSignal,addr);
    }
}

/********************************************************************************************
* ��������:    HandleRequestSignal_DelayRespMessage
* ��������:    �����յ���RequestSignal��Ϣ������delayresp��Ϣ������뱾��
                            ʱ��ģʽ��ƥ�䣬��ظ�Cancel��Ϣ
                            
* �������:    PtpClock *pPtpClock       
               MsgHeader *pMsgHeader     
               SignalReq *pReqestSignal  
               void *addr                
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void HandleRequestSignal_DelayRespMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,SignalReq *pReqestSignal,void *addr)
{
    int clientNum;
    clientNum = CalculateConnectedClient(pPtpClock);
    if(clientNum >= MAX_CLIENT)
    {
        issueCancelSignaling(pPtpClock,SIGNAL_DELAYRESP_MSG,pMsgHeader,pReqestSignal,addr);
    }
    else
    {
        if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
        {
            issueGrantSignaling(pPtpClock,SIGNAL_DELAYRESP_MSG,pMsgHeader,pReqestSignal,addr);
        }
        else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
        {
            issueCancelSignaling(pPtpClock,SIGNAL_DELAYRESP_MSG,pMsgHeader,pReqestSignal,addr);
        }
    }

}

/********************************************************************************************
* ��������:    HandleRequestSignal_PdelayRespMessage
* ��������:    ����Request ��Signal��Ϣ������Pdelayresp��Ϣ������뱾��ʱ��ģʽ��ƥ�䣬��ظ�delayca-
               ncel
* �������:    PtpClock *pPtpClock       
               MsgHeader *pMsgHeader     
               SignalReq *pReqestSignal  
               void *addr                
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void HandleRequestSignal_PdelayRespMessage(PtpClock *pPtpClock,MsgHeader *pMsgHeader,SignalReq *pReqestSignal,void *addr)
{
    int clientNum;
    clientNum = CalculateConnectedClient(pPtpClock);
    if(clientNum >= MAX_CLIENT)
    {
        issueCancelSignaling(pPtpClock,SIGNAL_PDELAYRESP_MSG,pMsgHeader,pReqestSignal,addr);
    }
    else
    {
        if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
        {
            issueCancelSignaling(pPtpClock,SIGNAL_PDELAYRESP_MSG,pMsgHeader,pReqestSignal,addr);
        }
        else if(pPtpClock->transmitDelayType == DELAY_MECANISM_P2P)
        {
            issueGrantSignaling(pPtpClock,SIGNAL_PDELAYRESP_MSG,pMsgHeader,pReqestSignal,addr);
        }
    }
}


/********************************************************************************************
* ��������:    ProcessRequestSignalMessage
* ��������:    �����յ���RequestSignal��Ϣ
* �������:    PtpClock *pPtpClock    
               char *buf              
               MsgHeader *pMsgHeader  
               void *addr             
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessRequestSignalMessage(PtpClock *pPtpClock,char *buf,MsgHeader *pMsgHeader,void *addr)
{
    SignalReq MsgRequestSignal;
    
    SignalReq *pReqestSignal = &MsgRequestSignal;

    msgUnpackRequestSignal(buf,pReqestSignal);

    if(pPtpClock->debugLevel)
    {
        if(pPtpClock->protoType == PROTO_UDP_IP)
        {
            printf("==>Handle Request signal Message Ip: %d.%d.%d.%d\n"
                ,*(Uint32*)addr&0xff,(*(Uint32*)addr>>8)&0xff
                ,(*(Uint32*)addr>>16)&0xff,(*(Uint32*)addr>>24)&0xff);
        
        }
        else if(pPtpClock->protoType == PROTO_IEEE802)
        {
            printf("==>Handle Request signal Message !! Mac: %x\n"
                ,*(char*)addr);
        }
    }
    
    switch (pReqestSignal->messageType)
    {
        case SIGNAL_SYN_TYPE:
            HandleRequestSignal_SynMessage(pPtpClock,pMsgHeader,pReqestSignal,addr);
            break;
        case SIGNAL_ANNOUNCE_TYPE:
            HandleRequestSignal_AnnounceMessage(pPtpClock,pMsgHeader,pReqestSignal,addr);
            break;
        case SIGNAL_DELAY_RESP_TYPE:
            HandleRequestSignal_DelayRespMessage(pPtpClock,pMsgHeader,pReqestSignal,addr);
            break;
        case SIGNAL_PDELAY_RESP_TYPE:
            HandleRequestSignal_PdelayRespMessage(pPtpClock,pMsgHeader,pReqestSignal,addr);
            break;
        default:
            break;
    }
}

/********************************************************************************************
* ��������:    ProcessGrantSignalMessage
* ��������:    �����յ���GrantSignal��Ϣ��������Ϣ��GrantSignal
* �������:    PtpClock *pPtpClock    
               char *buf              
               MsgHeader *pMsgHeader  
               void *addr             
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessGrantSignalMessage(PtpClock *pPtpClock,char *buf,MsgHeader *pMsgHeader,void *addr)
{
    SignalGrant MsgGrantSignal;
    
    SignalGrant *pGrantSignal = &MsgGrantSignal;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;

    msgUnpackGrantSignal(buf,pGrantSignal);
    int nIndex;
    
    if(pPtpClock->debugLevel)
    {
        if(pPtpClock->protoType == PROTO_UDP_IP)
        {
            printf("==>Handle Grant signal Message Ip: %d.%d.%d.%d\n"
                ,*(Uint32*)addr&0xff,(*(Uint32*)addr>>8)&0xff
                ,(*(Uint32*)addr>>16)&0xff,(*(Uint32*)addr>>24)&0xff);
        
        }
        else if(pPtpClock->protoType == PROTO_IEEE802)
        {
            printf("==>Handle Grant signal Message !! Mac: %x\n"
                ,*(char*)addr);
        }

    }
    switch (pGrantSignal->messageType)
    {
        case SIGNAL_SYN_TYPE:

            if(whether_InServUnicastList(pPtpClock,pServer_unicast,addr))
            {
                nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
                SetServUnicast_SynSig(pServer_unicast,nIndex,TRUE);

                /**��÷���������Ƶ��  */
                getServerUnicast_SynFreq(pServer_unicast,nIndex,pGrantSignal->logInterMessagePeriod);


            }
            break;
        case SIGNAL_ANNOUNCE_TYPE:
            if(whether_InServUnicastList(pPtpClock,pServer_unicast,addr))
            {
                nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
                SetServUnicast_AnnounceSig(pServer_unicast,nIndex,TRUE);

                
            }            
            break;
        case SIGNAL_DELAY_RESP_TYPE:
            if(whether_InServUnicastList(pPtpClock,pServer_unicast,addr))
            {
                nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
                SetServUnicast_DelayRespSig(pServer_unicast,nIndex,TRUE);
            }
            break;
        case SIGNAL_PDELAY_RESP_TYPE:
            if(whether_InServUnicastList(pPtpClock,pServer_unicast,addr))
            {
                nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
                SetServUnicast_DelayRespSig(pServer_unicast,nIndex,TRUE);
            }            
            break;
        default:
            break;
    }

}

/********************************************************************************************
* ��������:    ProcessCancelSignalMessage
* ��������:    �����յ���signal Cancel��Ϣ�����ݲ�ͬ��ʱ��״̬���ظ�ack
               cancel��Ϣ
* �������:    PtpClock *pPtpClock    
               char *buf              
               MsgHeader *pMsgHeader  
               void *addr             
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessCancelSignalMessage(PtpClock *pPtpClock,char *buf,MsgHeader *pMsgHeader,void *addr)
{
    SignalCancel MsgCancelSignal;
    
    SignalCancel *pCancelSignal = &MsgCancelSignal;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;

    int nIndex;

    msgUnpackCancelSignal(buf,pCancelSignal);
    
    if(pPtpClock->debugLevel)
    {
        if(pPtpClock->protoType == PROTO_UDP_IP)
        {
            printf("==>Handle Cancel signal Message Ip: %d.%d.%d.%d\n"
                ,*(Uint32*)addr&0xff,(*(Uint32*)addr>>8)&0xff
                ,(*(Uint32*)addr>>16)&0xff,(*(Uint32*)addr>>24)&0xff);
        
        }
        else if(pPtpClock->protoType == PROTO_IEEE802)
        {
            printf("==>Handle Cancel signal Message !! Mac: %x\n"
                ,*(char*)addr);
        }

    }
    /**��ʱ��  */
    if(pPtpClock->clockType == PTP_MASTER)
    {
        /**����յ��ĵ�ַ�Ƿ��ڿͻ��б�������򶪵�  */
        nIndex = WhetherAddressInClient(pPtpClock,addr);
        if(nIndex >= 0)
        {
            switch (pCancelSignal->messageType)
            {
                case SIGNAL_SYN_TYPE:
                    pPtpClock->unicastClient.clientList[nIndex].send_sync_enable = FALSE;
                    issueAckCancelSignaling(pPtpClock,SIGNAL_SYN_MSG,pMsgHeader,addr);
                    break;
                    
                case SIGNAL_ANNOUNCE_TYPE:
                    pPtpClock->unicastClient.clientList[nIndex].send_announce_enable = FALSE;
                    
                    issueAckCancelSignaling(pPtpClock,SIGNAL_ANNOUNCE_MSG,pMsgHeader,addr);
                    break;
                default:
                    break;
            }
        }
    }
    else if(pPtpClock->clockType == PTP_SLAVE)/**��ʱ��  */
    {
        switch (pCancelSignal->messageType)
        {
            case SIGNAL_SYN_TYPE:

                if(whether_InServUnicastList(pPtpClock,pServer_unicast,addr))
                {
                    nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
                    SetServUnicast_SynSig(pServer_unicast,nIndex,FALSE);
                    
                }
                
                issueAckCancelSignaling(pPtpClock,SIGNAL_SYN_MSG,pMsgHeader,addr);
                break;
            case SIGNAL_ANNOUNCE_TYPE:
                if(whether_InServUnicastList(pPtpClock,pServer_unicast,addr))
                {
                    nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
                    SetServUnicast_AnnounceSig(pServer_unicast,nIndex,FALSE);
                }            
                
                issueAckCancelSignaling(pPtpClock,SIGNAL_ANNOUNCE_MSG,pMsgHeader,addr);
                break;
            case SIGNAL_DELAY_RESP_TYPE:
                if(whether_InServUnicastList(pPtpClock,pServer_unicast,addr))
                {
                    nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
                    SetServUnicast_DelayRespSig(pServer_unicast,nIndex,FALSE);

                }
                
                issueAckCancelSignaling(pPtpClock,SIGNAL_DELAYRESP_MSG,pMsgHeader,addr);
                break;
            case SIGNAL_PDELAY_RESP_TYPE:
                if(whether_InServUnicastList(pPtpClock,pServer_unicast,addr))
                {
                    nIndex = Search_InServUnicastList(pPtpClock,pServer_unicast,addr);
                    SetServUnicast_DelayRespSig(pServer_unicast,nIndex,FALSE);
                }        
                
                issueAckCancelSignaling(pPtpClock,SIGNAL_PDELAYRESP_MSG,pMsgHeader,addr);
                break;
            default:
                break;
        }

    }
}

/********************************************************************************************
* ��������:    ProcessAckCancelSignalMessage
* ��������:    �����յ���Ackloledeg signal��Ϣ
* �������:    PtpClock *pPtpClock    
               char *buf              
               MsgHeader *pMsgHeader  
               void *addr             
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessAckCancelSignalMessage(PtpClock *pPtpClock,char *buf,MsgHeader *pMsgHeader,void *addr)
{
    SignalAckCancel MsgAckCancelSignal;
    
    SignalAckCancel *pAckCancelSignal = &MsgAckCancelSignal;
    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;

    if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        printf("==>Handle AckCancel signal Message Ip: %d.%d.%d.%d\n"
            ,*(Uint32*)addr&0xff,(*(Uint32*)addr>>8)&0xff
            ,(*(Uint32*)addr>>16)&0xff,(*(Uint32*)addr>>24)&0xff);

    }
    else if(pPtpClock->protoType == PROTO_IEEE802)
    {
        printf("==>Handle AckCancel signal Message !! Mac: %x\n"
            ,*(char*)addr);
    }



}


/********************************************************************************************
* ��������:    ProcessSignalMessage
* ��������:    �����յ���signal��Ϣ�����ǵ���Э��ʹ�õ�signal��Ϣ������ڶಥģʽ
               �����������أ��յ�signal��Ϣ�󣬲��signal��Ϣ�����ദ��syn announce delayresp 
               ��signal��Ϣ
               
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessSignalMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{
    MsgHeader OnPackMsgHeader;

    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgSignaling *pMsgSignal = (MsgSignaling *)(buf + HEADER_LENGTH);

    
    
    msgUnpackHeader(buf,&OnPackMsgHeader);
    
    if(pPtpClock->debugLevel)
    {
        //printf("Recv signal Message !!\n");        
        //msgHeader_display(pMsgHeader);
    }

    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;
    
    /** ����ڶಥģʽ�£���ֱ�ӷ��� */
    if(pPtpClock->modeType == IPMODE_MULTICAST)
        return;
    
    UInteger16 tlvType = buf[HEADER_LENGTH + 11];
    
    if(pPtpClock->clockType == PTP_MASTER)
    {
        
        switch (tlvType)
        {
            case REQUEST_UNICAST_TRANSMISSION:
                ProcessRequestSignalMessage(pPtpClock,buf,pMsgHeader,addr);
                break;
            /**��ģʽ�յ�grant ֱ�ӷ��� */
            case GRANT_UNICAST_TRANSMISSION:
                return;

            case CANCEL_UNICAST_TRANSMISSION:
                ProcessCancelSignalMessage(pPtpClock,buf,pMsgHeader,addr);
                break;

            case ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION:
                ProcessAckCancelSignalMessage(pPtpClock,buf,pMsgHeader,addr);
                break;
                
            default:
                break;
        }
    }
    else if(pPtpClock->clockType == PTP_SLAVE)
    {
        switch(tlvType)
        {
            /**��ģʽ�յ���request ֱ�ӷ���  */
            case REQUEST_UNICAST_TRANSMISSION:
                return;

            case GRANT_UNICAST_TRANSMISSION:
                ProcessGrantSignalMessage(pPtpClock,buf,pMsgHeader,addr);
                break;

            case CANCEL_UNICAST_TRANSMISSION:
                ProcessCancelSignalMessage(pPtpClock,buf,pMsgHeader,addr);
                break;

            case ACKNOWLEDGE_CANCEL_UNICAST_TRANSMISSION:
                ProcessAckCancelSignalMessage(pPtpClock,buf,pMsgHeader,addr);
                break;

            default:
                break;

        }
    }
    
}


/********************************************************************************************
* ��������:    ProcessSynMessage
* ��������:    �����յ���syn��Ϣ������������ʱ�ӣ��������ʱ��ʱ��
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessSynMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{
    
    MsgHeader OnPackMsgHeader;
    MsgSync   SynMessage;

    if(pPtpClock->clockType != PTP_SLAVE)
        return;

    msgUnpackHeader(buf,&OnPackMsgHeader);
    msgUnpackSync(buf,&SynMessage);
    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgSync *pMsgSyn = &SynMessage;

    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;
    
    if(pPtpClock->debugLevel)
    {
        printf("recv Syn Message !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgSync_display(pMsgSyn);
    }


    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            GetUniMessageInterval(pPtpClock,&pPtpClock->unicastMultiServer,addr);
            HandleSynMessage(pPtpClock,pMsgHeader,pMsgSyn);
        }
        
        Caculate_SynMsgTime(pPtpClock,pMsgHeader,pMsgSyn,addr);
    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {       
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            HandleSynMessage(pPtpClock,pMsgHeader,pMsgSyn);
        }

    }
    
}


/********************************************************************************************
* ��������:    ProcessAnnounceMessage
* ��������:    �����յ���Announce������Annnounce��Ϣ��������BMC�㷨����ѡ�������
               ʱ��
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessAnnounceMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{
    MsgHeader OnPackMsgHeader;
    MsgAnnounce Announce;

    if(pPtpClock->clockType != PTP_SLAVE)
        return;


    msgUnpackHeader(buf,&OnPackMsgHeader);
    msgUnpackAnnounce(buf,&Announce);

    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgAnnounce *pMsgAnnounce = &Announce;

    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;
    
    /**�Ƿ���ͬһ����������򷵻�  */
    if(!isFromSameDomain(pPtpClock,pMsgHeader) && (pPtpClock->domainFilterSwitch == TRUE))
    {
        PLOG("==>Recv Announce is not Same Domain !! Error !!<==\n");
        return;
    }


    
    if(pPtpClock->debugLevel)
    {
        printf("recv Announce Message !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgAnnounce_display(pMsgAnnounce);
    }


    /**����ģʽ��  */
    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        
        /** �鿴�Ƿ�utcOffset ��Ч */
         if(!IS_SET(pMsgHeader->flagField1, UTCV))
        {
            PLOG("flagField Error\n");
            //return;
        }

        if(pPtpClock->protoType == PROTO_UDP_IP)
        {
            BmcIp(pMsgHeader,pMsgAnnounce,(Uint32*)addr,pPtpClock);
        }
        else if(pPtpClock->protoType == PROTO_IEEE802)
        {
            BmcMac(pMsgHeader,pMsgAnnounce,(char*)addr,pPtpClock);
        }

        //pPtpClock->recvAnnounceFlag = TRUE;
    }

    /**�ಥģʽ��  */
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        /** �鿴�Ƿ�utcOffset ��Ч */
        if(!IS_SET(pMsgHeader->flagField1, UTCV))
        {
            PLOG("flagField Error\n");
            //return;
        }

        if(pPtpClock->protoType == PROTO_UDP_IP)
        {
            BmcIp(pMsgHeader,pMsgAnnounce,(Uint32*)addr,pPtpClock);
        }
        else if(pPtpClock->protoType == PROTO_IEEE802)
        {
            BmcMac(pMsgHeader,pMsgAnnounce,(char*)addr,pPtpClock);
        }

        //pPtpClock->recvAnnounceFlag = TRUE;
    }

    if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
    {
        pPtpClock->logAnnounceInterval = pMsgHeader->logMessageInterval;
        pPtpClock->recvAnnounceFlag = TRUE;
    }

}

/********************************************************************************************
* ��������:    ProcessFollowUpMessage
* ��������:    �����յ���FollowUp��Ϣ
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessFollowUpMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{
    MsgHeader OnPackMsgHeader;
    MsgFollowUp FollowUp;

    if(pPtpClock->clockType != PTP_SLAVE)
        return;

    msgUnpackHeader(buf,&OnPackMsgHeader);
    msgUnpackFollowUp(buf,&FollowUp);
    
    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgFollowUp *pMsgFollowUp = &FollowUp;


    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;


    if(pPtpClock->debugLevel)
    {
        printf("recv FollowUp Message !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgSync_display(pMsgSyn);
    }

    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            HandleFollowUpMessage(pPtpClock,pMsgHeader,pMsgFollowUp);
            //msgFollowUp_display(pMsgFollowUp);

        }
        Caculate_FollowUpMsgTime(pPtpClock,pMsgHeader,pMsgFollowUp,addr);

    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            HandleFollowUpMessage(pPtpClock,pMsgHeader,pMsgFollowUp);
            //msgFollowUp_display(pMsgFollowUp);

        }

    }

}



/********************************************************************************************
* ��������:    ProcessDelayReqMessage
* ��������:    �����յ���DelayReq��Ϣ
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessDelayReqMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{
    MsgHeader OnPackMsgHeader;
    MsgDelayReq DelayReq;

    TimeInternal InternalTime;

    if(pPtpClock->clockType != PTP_MASTER)
        return;

    if(pPtpClock->transmitDelayType != DELAY_MECANISM_E2E)
        return;

    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgDelayReq *pMsgDelayReq = &DelayReq;
  
    msgUnpackHeader(buf,pMsgHeader);
    msgUnpackDelayReq(buf,pMsgDelayReq);

    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /**�Ƿ���ͬһ����������򷵻�  */
    if(!isFromSameDomain(pPtpClock,pMsgHeader)  && (pPtpClock->domainFilterSwitch == TRUE))
    {
        printf("==>Recv DelayReq Not In Same Domain !! Error !!<==\n");
        return;
    }
    
    /**�õ����յ�request ��ʱ��  */
    GetFpgaRecvMessageTime(pPtpClock,pMsgHeader,&InternalTime);
        
    if(pPtpClock->debugLevel)
    {
        printf("recv DelayReqMessage !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgDelayReq_display(pMsgDelayReq);
    }


    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        /**�������Э�̹أ�����������¿ͻ����б�  */
        if(!pPtpClock->UniNegotiationEnable)
        {
            RecvMsg_UpdateUnicastClient(pPtpClock,addr,20);
        }
        else/**����Э�̿��������ַ�Ƿ����б����������򲻻ظ�  */
        {
            if(WhetherAddressInClient(pPtpClock,addr) < 0)
                return;
        }
        issueDelayResp(&InternalTime,pMsgHeader,pPtpClock,addr);
    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        issueDelayResp(&InternalTime,pMsgHeader,pPtpClock,addr);
    }

}

/********************************************************************************************
* ��������:    ProcessPdelayReqMessage
* ��������:    �����յ���PdelayReq��Ϣ
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessPdelayReqMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{
    MsgHeader OnPackMsgHeader;
    MsgPDelayReq PDelayReq;
    
    TimeInternal InternalTime;

    if(pPtpClock->clockType != PTP_MASTER)
        return;
    
    if(pPtpClock->transmitDelayType == DELAY_MECANISM_E2E)
        return;


    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgPDelayReq *pMsgPDelayReq = &PDelayReq;
  
    msgUnpackHeader(buf,pMsgHeader);
    msgUnpackPDelayReq(buf,pMsgPDelayReq);


    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /**�Ƿ���ͬһ����������򷵻�  */
    if(!isFromSameDomain(pPtpClock,pMsgHeader)  && (pPtpClock->domainFilterSwitch == TRUE))
    {
        printf("==>Recv PdelayReq Not In Same Domain !! Error !!<==\n");
        return;
    }
    
    /**�õ����յ�request ��ʱ��  */
    GetFpgaRecvMessageTime(pPtpClock,pMsgHeader,&InternalTime);

    if(pPtpClock->debugLevel)
    {
        printf("recv PdelayReq Message !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgSync_display(pMsgSyn);
    }

    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        /**�������Э�̹أ�����������¿ͻ����б�  */
        if(!pPtpClock->UniNegotiationEnable)
        {
            RecvMsg_UpdateUnicastClient(pPtpClock,addr,20);
        }
        else/**����Э�̿��������ַ�Ƿ����б����������򲻻ظ�  */
        {
            if(WhetherAddressInClient(pPtpClock,addr) < 0)
                return;
        }
        
        issuePDelayResp(&InternalTime,pMsgHeader,pPtpClock,addr);

    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        issuePDelayResp(&InternalTime,pMsgHeader,pPtpClock,addr);
    }

}


/********************************************************************************************
* ��������:    ProcessDelayRespMessage
* ��������:    �����յ���DelayResp��Ϣ
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessDelayRespMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{
    MsgHeader OnPackMsgHeader;
    MsgDelayResp DelayResp;


    if(pPtpClock->clockType != PTP_SLAVE)
        return;
    if(pPtpClock->transmitDelayType != DELAY_MECANISM_E2E)
        return;


    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgDelayResp *pMsgDelayResp = &DelayResp;

    msgUnpackHeader(buf,pMsgHeader);
    msgUnpackDelayResp(buf,pMsgDelayResp);

    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /** �ж��ǲ����Լ��ķ���Delayreq */
    if((memcmp(pPtpClock->portIdentity.clockIdentity
        ,pMsgDelayResp->requestingPortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH) != 0)
        || (pPtpClock->portIdentity.portNumber != pMsgDelayResp->requestingPortIdentity.portNumber))
        return;

    if(pPtpClock->debugLevel)
    {
        printf("recv DelayResp Message !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgDelayResp_display(pMsgDelayResp);
    }

    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            if((pMsgHeader->sequenceId) == pPtpClock->sentDelayReqSequenceId)
            {
                HandleDelayRespMessage(pPtpClock,pMsgHeader,pMsgDelayResp);
                //msgDelayResp_display(pMsgDelayResp);
            }
            else
            {
                PLOG("==> SequenceId Error DelayResp (%d)\n",pMsgHeader->sequenceId);
            }
            
        }
        else
        {

        }

    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            if((pMsgHeader->sequenceId+1) == pPtpClock->sentDelayReqSequenceId)
            {
                HandleDelayRespMessage(pPtpClock,pMsgHeader,pMsgDelayResp);
                //msgDelayResp_display(pMsgDelayResp);
            }
            else
            {
                //PLOG("==> Error DelayResp (%d)\n",pMsgHeader->sequenceId);
            }
            
        }
    }

}



/********************************************************************************************
* ��������:    ProcessPdelayRespMessage
* ��������:    �����յ���PdelayResp��Ϣ
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessPdelayRespMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{

    MsgHeader OnPackMsgHeader;
    MsgPDelayResp PDelayResp;


    if(pPtpClock->clockType != PTP_SLAVE)
        return;
    if(pPtpClock->transmitDelayType != DELAY_MECANISM_P2P)
        return;


    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgPDelayResp *pMsgPDelayResp = &PDelayResp;

    msgUnpackHeader(buf,pMsgHeader);
    msgUnpackPDelayResp(buf,pMsgPDelayResp);

    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /** �ж��ǲ����Լ��ķ���pDelayreq */

    if((memcmp(pPtpClock->portIdentity.clockIdentity
        ,pMsgPDelayResp->requestingPortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH) != 0)
        || (pPtpClock->portIdentity.portNumber != pMsgPDelayResp->requestingPortIdentity.portNumber))
        return;
    
    if(pPtpClock->debugLevel)
    {
        printf("recv PdelayResp Message !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgSync_display(pMsgSyn);
    }

    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            if((pMsgHeader->sequenceId) == pPtpClock->sentPDelayReqSequenceId)
            {
                HandlePdelayRespMessage(pPtpClock,pMsgHeader,pMsgPDelayResp);
                //msgPDelayResp_display(pMsgPDelayResp);
            }
            else
            {
                PLOG("==> SequenceId Error DelayResp (%d)\n",pMsgHeader->sequenceId);
            }

        }
        else
        {

        }


    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {

        if((pMsgHeader->sequenceId+1) == pPtpClock->sentPDelayReqSequenceId)
        {
            HandlePdelayRespMessage(pPtpClock,pMsgHeader,pMsgPDelayResp);
            //msgPDelayResp_display(pMsgPDelayResp);
        }
        else
        {
            //PLOG("==> Error DelayResp (%d)\n",pMsgHeader->sequenceId);
        }


    }

}

/********************************************************************************************
* ��������:    ProcessPdelayRespFollowUpMessage
* ��������:    �����յ���PdelayRespFollowUp��Ϣ
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessPdelayRespFollowUpMessage(PtpClock *pPtpClock,char *buf,int recv_len,void *addr)
{
    MsgHeader OnPackMsgHeader;
    MsgPDelayRespFollowUp PDelayRespFollowUp;

    
    if(pPtpClock->clockType != PTP_SLAVE)
        return;
    if(pPtpClock->transmitDelayType != DELAY_MECANISM_P2P)
        return;


    MsgHeader *pMsgHeader = &OnPackMsgHeader;
    MsgPDelayRespFollowUp *pMsgPDelayRespFollowUp = &PDelayRespFollowUp;

    msgUnpackHeader(buf,pMsgHeader);
    msgUnpackPDelayRespFollowUp(buf,pMsgPDelayRespFollowUp);


    /** �յ��Լ�������Ϣ��ֱ�ӷ��� */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /** �ж��ǲ����Լ��ķ���pDelayreq */

    if((memcmp(pPtpClock->portIdentity.clockIdentity
        ,pMsgPDelayRespFollowUp->requestingPortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH) != 0)
        || (pPtpClock->portIdentity.portNumber != pMsgPDelayRespFollowUp->requestingPortIdentity.portNumber))
        return;
    


    if(pPtpClock->debugLevel)
    {
        printf("recv PdelayRespFollowUp Message !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgSync_display(pMsgSyn);
    }

    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            //msgPDelayRespFollowUp_display(pMsgPDelayRespFollowUp);
            HandlePdelayRespFollowUpMessage(pPtpClock,pMsgHeader,pMsgPDelayRespFollowUp);
        }
        else
        {

        }
        
    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**�������ʱ��  */
        {
            //msgPDelayRespFollowUp_display(pMsgPDelayRespFollowUp);
            HandlePdelayRespFollowUpMessage(pPtpClock,pMsgHeader,pMsgPDelayRespFollowUp);
        }

    }

}



/********************************************************************************************
* ��������:    ProcessIeee802_Msg
* ��������:    ����Ieee802.3�����յ������ݰ�������ȥ��Դmac Ŀ��mac��Ȼ��ƥ��vlan
               ����������Ϣ������
* �������:    PtpClock *pPtpClock      
               char *buf                
               int recv_len             
               struct sockaddr_ll addr  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void ProcessIeee802_Msg(PtpClock *pPtpClock,char *buf,int recv_len,struct sockaddr_ll addr)
{

    Ieee802Head *pIeee802Head;
    VlanHeader *pVlanHead;
    
    char *pInBuf;
    int Udp_len;

   
    if(pPtpClock->vlanEnable == TRUE)
    {
        pVlanHead = (VlanHeader*)buf;
        
        if(pVlanHead->vlanType1 != 0x81 || pVlanHead->vlanType2 != 0x00)
            return;

        /** ���ID ��ƥ�� */
        if(pVlanHead->vlanId != pPtpClock->vlanId)
            return;
        
        PLOG("Recv vlan Message\n");

        /**�������IEEE802.3 �����򶪵�  */
        if((pVlanHead->h_proto_1 != 0x88) || (pVlanHead->h_proto_2 != 0xf7))
            return;

        pInBuf = buf + 18;
        Udp_len = recv_len - 18;
        
    }
    else if(pPtpClock->vlanEnable == FALSE)
    {
        pIeee802Head = (Ieee802Head*)buf;
        
        if((pIeee802Head->h_proto_1 != 0x88) || (pIeee802Head->h_proto_2 != 0xf7))
            return;

        pInBuf = buf + 14;
        Udp_len = recv_len - 14;
    }
    

    //PLOG("recv IEEE802.3 msg!!\n");

    Process_Msg(pPtpClock,pInBuf,Udp_len,(void*)&addr);
    
}


/********************************************************************************************
* ��������:    Process_Msg
* ��������:    ������Ϣ���������Ӳ�ͬ��2�㣬3��Э�飬���ݲ�ͬ����Ϣ���ͣ�ѡ��ͬ
               �����֧
* �������:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *pAddr          
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void Process_Msg(PtpClock *pPtpClock,char *buf,int recv_len,void *pAddr)
{
    Uint8 msg_type;
    MsgHeader *pRecvMsgHead = (MsgHeader*)buf;
    msg_type = pRecvMsgHead->transportSpecific & 0x0f;
    
    void *pMyAddr = NULL;

    if(pPtpClock->protoType == PROTO_IEEE802)
    {
        struct sockaddr_ll *addr = (struct sockaddr_ll*)pAddr;
        pMyAddr = (void*)addr->sll_addr;
    }
    else if(pPtpClock->protoType == PROTO_UDP_IP)
    {
        struct sockaddr_in *addr = (struct sockaddr_in*)pAddr;
        pMyAddr = (Uint32*)&addr->sin_addr.s_addr;
    }

    //PLOG("recv udp msg!!\n");

    
    switch(msg_type)
    {
        case TYPE_SYN:
            ProcessSynMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_FOLLOWUP:
            ProcessFollowUpMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_ANNOUNCE:
            ProcessAnnounceMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_SIGNALING:
            ProcessSignalMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_DELAY_REQ:
            ProcessDelayReqMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_DELAY_RESP:
            ProcessDelayRespMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_PDELAY_REQ:
            ProcessPdelayReqMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_PDELAY_RESP:
            ProcessPdelayRespMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_PDELAY_RESP_FOLLOWUP:
            ProcessPdelayRespFollowUpMessage(pPtpClock,buf,recv_len,(void*)pMyAddr);
            break;
        case TYPE_MANAGEMENT:
            /** ���� */
            break;
            
        default:
            break;
    }


    
}


/********************************************************************************************
* ��������:    MasterMode_Recv
* ��������:    ���մ����̣߳���ģʽ�£����н��մ������
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void MasterMode_Recv(PtpClock *pPtpClock)
{
    int ret;
    fd_set current_rdfs;
    int maxfd;
    int recv_len;
    
    struct sockaddr_ll  Ieee802_Sockaddr;
    struct sockaddr_in  UdpIpv4_Sockaddr;
    socklen_t socket_len;
    struct timeval timeout;


    memset(&UdpIpv4_Sockaddr,0,sizeof(UdpIpv4_Sockaddr));
    memset(&Ieee802_Sockaddr,0,sizeof(Ieee802_Sockaddr));
    
    current_rdfs = pPtpClock->netComm.global_rdfs;
    maxfd = pPtpClock->netComm.maxfd;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

  
    //��·���ã�ʱ������Ϊ1��
	if((ret = select(maxfd + 1,&current_rdfs,NULL,NULL,&timeout)) < 0)
    {
		perror("select fail ..!!\n");
    }
    if(ret)
    {
        
        memset(pPtpClock->msgIbuf,0,PACKET_SIZE);
        
        if(pPtpClock->protoType == PROTO_IEEE802)
        {
            socket_len = sizeof(Ieee802_Sockaddr);
            
            if(FD_ISSET(pPtpClock->netComm.IeeeE2ESock,&current_rdfs))
            {
                recv_len = recvfrom(pPtpClock->netComm.IeeeE2ESock
                    ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&Ieee802_Sockaddr,&socket_len);

                if(recv_len > 0)
                    ProcessIeee802_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,Ieee802_Sockaddr);
            }
#if 0
            else if(FD_ISSET(pPtpClock->netComm.IeeeP2PSock,&current_rdfs))
            {
                recv_len = recvfrom(pPtpClock->netComm.IeeeP2PSock
                    ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&Ieee802_Sockaddr,&socket_len);

                if(recv_len > 0)
                    ProcessIeee802_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,Ieee802_Sockaddr);
            }
#endif
            
        }
        
        else if(pPtpClock->protoType == PROTO_UDP_IP)
        {
            socket_len = sizeof(UdpIpv4_Sockaddr);
            if(pPtpClock->modeType == IPMODE_UNICAST)
            {
                if(FD_ISSET(pPtpClock->netComm.generalSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.generalSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);
                }
                
                if(FD_ISSET(pPtpClock->netComm.eventSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.eventSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);

                }
                

            }
            else if(pPtpClock->modeType == IPMODE_MULTICAST)
            {
                if(FD_ISSET(pPtpClock->netComm.generalSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.generalSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);

                }
                
                if(FD_ISSET(pPtpClock->netComm.eventSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.eventSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);


                }
                if(FD_ISSET(pPtpClock->netComm.PgeneralSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.PgeneralSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);
                }
                
                if(FD_ISSET(pPtpClock->netComm.PeventSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.PeventSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);

                }
            }
            
        }
    }
    //printf("==>WaitRecvMessage TimeOut!!\n");

}

/********************************************************************************************
* ��������:    SlaveMode_Recv
* ��������:    ���մ����̣߳���ģʽ�£����н��մ������
* �������:    PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void SlaveMode_Recv(PtpClock *pPtpClock)
{
    int ret;
    fd_set current_rdfs;
    int maxfd;
    int recv_len;
    struct timeval timeout;
    
    struct sockaddr_ll  Ieee802_Sockaddr;
    struct sockaddr_in  UdpIpv4_Sockaddr;
    socklen_t socket_len;


    current_rdfs = pPtpClock->netComm.global_rdfs;
    maxfd = pPtpClock->netComm.maxfd;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
        
   /**  ��·���ã�ʱ������Ϊ1�� */
	if((ret = select(maxfd + 1,&current_rdfs,NULL,NULL,&timeout)) < 0)
    {
		perror("select fail ..!!\n");
    }
    if(ret)
    {
        memset(pPtpClock->msgIbuf,0,PACKET_SIZE);
        
        if(pPtpClock->protoType == PROTO_IEEE802)
        {
            socket_len = sizeof(Ieee802_Sockaddr);
            if(FD_ISSET(pPtpClock->netComm.IeeeE2ESock,&current_rdfs))
            {
                recv_len = recvfrom(pPtpClock->netComm.IeeeE2ESock
                    ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&Ieee802_Sockaddr,&socket_len);
                
                if(recv_len > 0)
                ProcessIeee802_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,Ieee802_Sockaddr);
            }

        }
        else if(pPtpClock->protoType == PROTO_UDP_IP)
        {
            socket_len = sizeof(UdpIpv4_Sockaddr);
            
            if(pPtpClock->modeType == IPMODE_UNICAST)
            {
                if(FD_ISSET(pPtpClock->netComm.generalSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.generalSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);

                }
                
                if(FD_ISSET(pPtpClock->netComm.eventSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.eventSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);

                }
                

            }
            else if(pPtpClock->modeType == IPMODE_MULTICAST)
            {
                if(FD_ISSET(pPtpClock->netComm.generalSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.generalSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);

                }
                
                if(FD_ISSET(pPtpClock->netComm.eventSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.eventSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);
                }
                if(FD_ISSET(pPtpClock->netComm.PgeneralSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.PgeneralSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);

                }
                
                if(FD_ISSET(pPtpClock->netComm.PeventSock,&current_rdfs))
                {
                    recv_len = recvfrom(pPtpClock->netComm.PeventSock
                        ,pPtpClock->msgIbuf,PACKET_SIZE,0,(struct sockaddr *)&UdpIpv4_Sockaddr,&socket_len);

                    if(recv_len > 0)
                        Process_Msg(pPtpClock,pPtpClock->msgIbuf,recv_len,&UdpIpv4_Sockaddr);

                }
            }

        }

    }
}

/********************************************************************************************
* ��������:    MasterMode_Send
* ��������:    �����߳��У�����ģʽʱ�����е����
* �������:    Uint32 timerIndex    
               PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void MasterMode_Send(Uint32 timerIndex,PtpClock *pPtpClock)
{

    /**�澯���  */
    pPtpClock->oneSecondTime++;
    if(pPtpClock->oneSecondTime > TIMER_1_SECOND)
    {
        pPtpClock->oneSecondTime = 0;
        UnicastClient_PpsCheck(pPtpClock);
        //Check_ThreeMessageRecvAlarm(pPtpClock);
        Display_PtpStatusEvery10s(pPtpClock);
        //printf("==============\n");
        //printf("\033[2J");

    }

    
    /** �ಥģʽ����interval ��ѯ���� */
    if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        if(timerIndex%pPtpClock->synSendInterval == 0)
        {
            issueSync(pPtpClock);
        }

        if((timerIndex%(pPtpClock->announceSendInterval)) == 0)
        {
            issueAnnounce(pPtpClock);
        }

    }
        
    else if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        /** ����ģʽ����interval ��� ���� */
        if(timerIndex%pPtpClock->synSendInterval == 0)
        {
            Refresh_Syn_UnicastClient(pPtpClock);
        }
        if((timerIndex%pPtpClock->announceSendInterval) == 0)
        {
            Refresh_Announce_UnicastClient(pPtpClock);
        }

        /** ����ģʽ����10ms������� */
        if(timerIndex%2 == 0)
        {
            issueSync(pPtpClock);
            issueAnnounce(pPtpClock);
        }

    }    

}

/********************************************************************************************
* ��������:    SlaveMode_Send
* ��������:    �����߳��У��Ӷ�ģʽʱ�����е����
* �������:    Uint32 timerIndex    
                            PtpClock *pPtpClock  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-11
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
void SlaveMode_Send(Uint32 timerIndex,PtpClock *pPtpClock)
{

    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;

    
    /**�澯���  */
    pPtpClock->oneSecondTime++;
    if(pPtpClock->oneSecondTime > TIMER_1_SECOND)
    {
        pPtpClock->oneSecondTime = 0;
        Check_ThreeMessageRecvAlarm(pPtpClock);
        Display_PtpStatusEvery10s(pPtpClock);
    }


    /** �ಥģʽ����interval ��ѯ���� */
    if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        if(timerIndex%pPtpClock->delayreqInterval == 1)
        {
            switch(pPtpClock->transmitDelayType)
            {
                case DELAY_MECANISM_E2E:
                    issueDelayReq(pPtpClock);
                    break;
                case DELAY_MECANISM_P2P:
                    issuePDelayReq(pPtpClock);
                    break;
                default:
                    break;
            }
        }
    }
        
    else if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        /** ����ģʽ����interval ��� ���� */
        if(timerIndex%pPtpClock->delayreqInterval == 0)
        {
            ReSetSendDelayServer(pPtpClock,pServer_unicast);

        }
        /**����Э�̿�ʱ��ÿ��ִ�и���  */
        if(timerIndex%256 == 0)
        {
            if(pPtpClock->UniNegotiationEnable)
            {
                ResetSendSignalRequest(pPtpClock,pServer_unicast);
                Update_ServUnicastList(pPtpClock,pServer_unicast);
            }
        }
        
        /** ����ģʽ����10ms������� */
        if(timerIndex%2 == 0)
        {
            /** ����Э�̿� */
            if(pPtpClock->UniNegotiationEnable)
            {
                issueRequestSignaling(pPtpClock);
                
                CheckSendDelayServerEnable(pServer_unicast,pServer_unicast->delay_index);
                switch(pPtpClock->transmitDelayType)
                {
                    case DELAY_MECANISM_E2E:
                        issueDelayReq(pPtpClock);
                        break;
                    case DELAY_MECANISM_P2P:
                        issuePDelayReq(pPtpClock);
                        break;
                    default:
                        break;
                }

            }
            /**����Э�̹�  */
            else if(!pPtpClock->UniNegotiationEnable)
            {
                
                SetSendDelayServerEnable(pServer_unicast,pServer_unicast->delay_index);
                
                switch(pPtpClock->transmitDelayType)
                {
                    case DELAY_MECANISM_E2E:
                        issueDelayReq(pPtpClock);
                        break;
                    case DELAY_MECANISM_P2P:
                        issuePDelayReq(pPtpClock);
                        break;
                    default:
                        break;
                }
            }

        }

    }

}

