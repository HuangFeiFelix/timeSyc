/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       ptp_proto.c
*    功能描述:       消息处理，线程模块入口，打印显示函数
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-30
*    函数列表:
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
*    备注:            
*    修改历史:
                   1.日期: 2015-1-30
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/

#include "ptp_proto.h"
#include "arith.h"
#include "msg.h"
#include "unicast_process.h"
#include "bmc.h"
#include "fpga_time.h"
#include "comm_fun.h"

/********************************************************************************************
* 函数名称:    integer64_display
* 功能描述:    打印显示64未时间
* 输入参数:    const Integer64 * bigint  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void integer64_display(const Integer64 * bigint)
{
	printf("Integer 64 : \n");
	printf("LSB : %u\n", bigint->lsb);
	printf("MSB : %d\n", bigint->msb);
}


/********************************************************************************************
* 函数名称:    uInteger48_display
* 功能描述:    打印显示48位时间
* 输入参数:    const UInteger48 * bigint  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void uInteger48_display(const UInteger48 * bigint)
{
	printf("Integer 48 : \n");
	printf("LSB : %u\n", bigint->lsb);
	printf("MSB : %u\n", bigint->msb);
}


/********************************************************************************************
* 函数名称:    timeInternal_display
* 功能描述:    打印TimerInterval
* 输入参数:    const TimeInternal * timeInternal  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void timeInternal_display(const TimeInternal * timeInternal)
{
	printf("seconds : %d \n", timeInternal->seconds);
	printf("nanoseconds %d \n", timeInternal->nanoseconds);
}


/********************************************************************************************
* 函数名称:    timestamp_display
* 功能描述:    打印Timestamp
* 输入参数:    const Timestamp * timestamp  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void timestamp_display(const Timestamp * timestamp)
{
	uInteger48_display(&timestamp->secondsField);
	printf("nanoseconds %u \n", timestamp->nanosecondsField);
}


/********************************************************************************************
* 函数名称:    clockIdentity_display
* 功能描述:    时钟Identity 打印显示
* 输入参数:    const ClockIdentity clockIdentity  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    portIdentity_display
* 功能描述:    PortIndentity 打印显示
* 输入参数:    const PortIdentity * portIdentity  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void portIdentity_display(const PortIdentity * portIdentity)
{
	clockIdentity_display(portIdentity->clockIdentity);
	printf("port number : %d \n", portIdentity->portNumber);
}

/********************************************************************************************
* 函数名称:    clockQuality_display
* 功能描述:    时钟质量打印显示
* 输入参数:    const ClockQuality * clockQuality  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void clockQuality_display(const ClockQuality * clockQuality)
{
	printf("clockClass : %d \n", clockQuality->clockClass);
	printf("clockAccuracy : %d \n", clockQuality->clockAccuracy);
	printf("offsetScaledLogVariance : %d \n", clockQuality->offsetScaledLogVariance);
}


/********************************************************************************************
* 函数名称:    iFaceName_display
* 功能描述:    网口名打印显示
* 输入参数:    const Octet * iFaceName  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    unicast_display
* 功能描述:    单播地址打印显示
* 输入参数:    const Octet * unicast  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    msgHeader_display
* 功能描述:    打印消息Head 头
* 输入参数:    const MsgHeader * header  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    msgSync_display
* 功能描述:    打印syn时间消息
* 输入参数:    const MsgSync * sync  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void msgSync_display(const MsgSync * sync)
{
	printf("Message Sync : \n");
	timestamp_display(&sync->originTimestamp);
	printf("\n");
}

/********************************************************************************************
* 函数名称:    msgAnnounce_display
* 功能描述:    打印收到的Announce消息
* 输入参数:    const MsgAnnounce * announce  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    msgFollowUp_display
* 功能描述:    打印收到的FollowUp时间消息
* 输入参数:    const MsgFollowUp * follow  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void msgFollowUp_display(const MsgFollowUp * follow)
{
	timestamp_display(&follow->preciseOriginTimestamp);
}


/********************************************************************************************
* 函数名称:    msgDelayReq_display
* 功能描述:    打印收到的DelayReq时间消息
* 输入参数:    const MsgDelayReq * req  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void msgDelayReq_display(const MsgDelayReq * req)
{
	timestamp_display(&req->originTimestamp);
}


/********************************************************************************************
* 函数名称:    msgDelayResp_display
* 功能描述:    打印收到的DelayResp时间消息
* 输入参数:    const MsgDelayResp * resp  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void msgDelayResp_display(const MsgDelayResp * resp)
{
	timestamp_display(&resp->receiveTimestamp);
	portIdentity_display(&resp->requestingPortIdentity);
}

/********************************************************************************************
* 函数名称:    msgPDelayReq_display
* 功能描述:    打印PdelayReq
* 输入参数:    const MsgPDelayReq * preq  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void msgPDelayReq_display(const MsgPDelayReq * preq)
{
	timestamp_display(&preq->originTimestamp);
}


/********************************************************************************************
* 函数名称:    msgPDelayResp_display
* 功能描述:    打印PdelayResp 消息
* 输入参数:    const MsgPDelayResp * presp  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void msgPDelayResp_display(const MsgPDelayResp * presp)
{
	timestamp_display(&presp->requestReceiptTimestamp);
	portIdentity_display(&presp->requestingPortIdentity);
}


/********************************************************************************************
* 函数名称:    msgPDelayRespFollowUp_display
* 功能描述:    打印PdelayRespFollowUp消息
* 输入参数:    const MsgPDelayRespFollowUp * prespfollow  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void msgPDelayRespFollowUp_display(const MsgPDelayRespFollowUp * prespfollow)
{
	timestamp_display(&prespfollow->responseOriginTimestamp);
	portIdentity_display(&prespfollow->requestingPortIdentity);
}

/********************************************************************************************
* 函数名称:    isFromBestMaster
* 功能描述:    判断是否是最佳主时钟
* 输入参数:    const PtpClock *ptpClock  
               const MsgHeader* header   
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    isFromMySelfMaster
* 功能描述:    判断是否是来自自己发的包如果是返回真，不是返回假
* 输入参数:    const PtpClock *ptpClock  
               const MsgHeader* header   
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

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
            /**  计算delaymeanPath: ( (t4-t3)+ (t2-t1) - syncorrection -delayrespcorrection) /2 */ 
            subTime(&T2_T1,&T2,&T1);
            //subTime(&T4_T3,&T4,&T3);
            addTime(pMeanPathDelay,&pPtpClock->SMDelay,&T2_T1);
            subTime(pMeanPathDelay,pMeanPathDelay,&synCorrection);
            subTime(pMeanPathDelay,pMeanPathDelay,&delayrespCorrection);
            
            div2Time(pMeanPathDelay);
            
             /**计算TimeOffset  T2-T1 - delaymeanPath - synCorrection */

            subTime(&T2_T1,&T2,&T1);
            subTime(pTimeOffset,&T2_T1,pMeanPathDelay);
            subTime(pTimeOffset,pTimeOffset,&synCorrection);
            
        }
        else if(pPtpClock->stepType == TWO_STEP)
        {
            /**  计算delaymeanPath: ( (t4-t3)+ (t2-t1) - syncorrection -followupcorrection-delayrespcorrection) /2 */ 
            subTime(&T2_T1,&T2,&T1);
            //subTime(&T4_T3,&T4,&T3);
            addTime(pMeanPathDelay,&pPtpClock->SMDelay,&T2_T1);
            
            subTime(pMeanPathDelay,pMeanPathDelay,&synCorrection);
            subTime(pMeanPathDelay,pMeanPathDelay,&followCorrection);
            subTime(pMeanPathDelay,pMeanPathDelay,&delayrespCorrection);
            
            div2Time(pMeanPathDelay);
            
             /**计算TimeOffset  T2-T1 - delaymeanPath - synCorrection -followupcorrection*/
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
            /**计算meanpath ( t6-t3 - pdelayrespCorrection) /2*/
            //subTime(&T6_T3,&T6,&T3);
            subTime(pMeanPathDelay,&pPtpClock->SMDelay,&pdelayrespCorrection);
            div2Time(pMeanPathDelay);
            
            /**计算TimeOffset  T2-T1 - delaymeanPath - synCorrection */
            subTime(&T2_T1,&T2,&T1);
            subTime(pTimeOffset,&T2_T1,pMeanPathDelay);
            subTime(pTimeOffset,pTimeOffset,&synCorrection);

        }
        else if(pPtpClock->stepType == TWO_STEP)
        {
            /**计算meanpath ( t6-t3 - (T5-T4)-pdelayrespCorrection - pdelayrespfollowupCorrection) /2*/
            //subTime(&T6_T3,&T6,&T3);
            //subTime(&T5_T4,&pPtpClock->T5,&pPtpClock->T4);
            
            subTime(pMeanPathDelay,&pPtpClock->SMDelay,&pPtpClock->SMDelay2);
            subTime(pMeanPathDelay,pMeanPathDelay,&pdelayrespCorrection);
            subTime(pMeanPathDelay,pMeanPathDelay,&pdelayrespfollowupCorrection);
            div2Time(pMeanPathDelay);
            
           /**计算TimeOffset  T2-T1 - delaymeanPath - synCorrection -followupcorrection*/
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
    
    /**获得T1  */
    if(pPtpClock->stepType == ONE_STEP)
    {
        pPtpClock->T1.seconds = pMsgSyn->originTimestamp.secondsField.lsb;
        pPtpClock->T1.nanoseconds = pMsgSyn->originTimestamp.nanosecondsField;
        integer64_to_internalTime(pMsgHeader->correctionField,&pPtpClock->SynCorrection);
    }

    /**获得T2  */
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
* 函数名称:    HandleRequestSignal_SynMessage
* 功能描述:    处理收到的RequestSignal消息，申请syn消息，服务器有处理能力，则返回Grant，如
               果忙不过来，则回复Cancel
               
* 输入参数:    PtpClock *pPtpClock       
               MsgHeader *pMsgHeader     
               SignalReq *pReqestSignal  
               void *addr                
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    HandleRequestSignal_AnnounceMessage
* 功能描述:    处理收到的RequestSignal消息，申请announce 消息，服务器有处理能力，则返回Grant，如
               果忙不过来，则回复Cancel
               
* 输入参数:    PtpClock *pPtpClock       
               MsgHeader *pMsgHeader     
               SignalReq *pReqestSignal  
               void *addr                
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    HandleRequestSignal_DelayRespMessage
* 功能描述:    处理收到的RequestSignal消息，申请delayresp消息，如果与本机
                            时延模式不匹配，则回复Cancel消息
                            
* 输入参数:    PtpClock *pPtpClock       
               MsgHeader *pMsgHeader     
               SignalReq *pReqestSignal  
               void *addr                
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    HandleRequestSignal_PdelayRespMessage
* 功能描述:    处理Request 的Signal消息，申请Pdelayresp消息，如果与本机时延模式不匹配，则回复delayca-
               ncel
* 输入参数:    PtpClock *pPtpClock       
               MsgHeader *pMsgHeader     
               SignalReq *pReqestSignal  
               void *addr                
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    ProcessRequestSignalMessage
* 功能描述:    处理收到的RequestSignal消息
* 输入参数:    PtpClock *pPtpClock    
               char *buf              
               MsgHeader *pMsgHeader  
               void *addr             
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    ProcessGrantSignalMessage
* 功能描述:    处理收到的GrantSignal消息，三类消息的GrantSignal
* 输入参数:    PtpClock *pPtpClock    
               char *buf              
               MsgHeader *pMsgHeader  
               void *addr             
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

                /**获得服务器发包频率  */
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
* 函数名称:    ProcessCancelSignalMessage
* 功能描述:    处理收到的signal Cancel消息，根据不同的时钟状态，回复ack
               cancel消息
* 输入参数:    PtpClock *pPtpClock    
               char *buf              
               MsgHeader *pMsgHeader  
               void *addr             
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
    /**主时钟  */
    if(pPtpClock->clockType == PTP_MASTER)
    {
        /**检测收到的地址是否在客户列表里，不在则丢掉  */
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
    else if(pPtpClock->clockType == PTP_SLAVE)/**从时钟  */
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
* 函数名称:    ProcessAckCancelSignalMessage
* 功能描述:    处理收到的Ackloledeg signal消息
* 输入参数:    PtpClock *pPtpClock    
               char *buf              
               MsgHeader *pMsgHeader  
               void *addr             
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
* 函数名称:    ProcessSignalMessage
* 功能描述:    处理收到的signal消息，这是单播协商使用的signal消息，如果在多播模式
               下则立即返回，收到signal消息后，拆分signal消息，分类处理syn announce delayresp 
               的signal消息
               
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

    /** 收到自己发的消息，直接返回 */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;
    
    /** 如果在多播模式下，则直接返回 */
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
            /**主模式收到grant 直接返回 */
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
            /**从模式收到到request 直接返回  */
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
* 函数名称:    ProcessSynMessage
* 功能描述:    处理收到的syn消息，如果是最佳主时钟，则矫正从时钟时间
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

    /** 收到自己发的消息，直接返回 */
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
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
        {
            GetUniMessageInterval(pPtpClock,&pPtpClock->unicastMultiServer,addr);
            HandleSynMessage(pPtpClock,pMsgHeader,pMsgSyn);
        }
        
        Caculate_SynMsgTime(pPtpClock,pMsgHeader,pMsgSyn,addr);
    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {       
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
        {
            HandleSynMessage(pPtpClock,pMsgHeader,pMsgSyn);
        }

    }
    
}


/********************************************************************************************
* 函数名称:    ProcessAnnounceMessage
* 功能描述:    处理收到的Announce，解析Annnounce消息包，根据BMC算法参数选出最佳主
               时钟
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

    /** 收到自己发的消息，直接返回 */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;
    
    /**是否在同一个域里，不在则返回  */
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


    /**单播模式下  */
    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        
        /** 查看是否utcOffset 有效 */
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

    /**多播模式下  */
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        /** 查看是否utcOffset 有效 */
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

    if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
    {
        pPtpClock->logAnnounceInterval = pMsgHeader->logMessageInterval;
        pPtpClock->recvAnnounceFlag = TRUE;
    }

}

/********************************************************************************************
* 函数名称:    ProcessFollowUpMessage
* 功能描述:    处理收到的FollowUp消息
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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


    /** 收到自己发的消息，直接返回 */
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
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
        {
            HandleFollowUpMessage(pPtpClock,pMsgHeader,pMsgFollowUp);
            //msgFollowUp_display(pMsgFollowUp);

        }
        Caculate_FollowUpMsgTime(pPtpClock,pMsgHeader,pMsgFollowUp,addr);

    }
    else if(pPtpClock->modeType == IPMODE_MULTICAST)
    {
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
        {
            HandleFollowUpMessage(pPtpClock,pMsgHeader,pMsgFollowUp);
            //msgFollowUp_display(pMsgFollowUp);

        }

    }

}



/********************************************************************************************
* 函数名称:    ProcessDelayReqMessage
* 功能描述:    处理收到的DelayReq消息
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

    /** 收到自己发的消息，直接返回 */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /**是否在同一个域里，不在则返回  */
    if(!isFromSameDomain(pPtpClock,pMsgHeader)  && (pPtpClock->domainFilterSwitch == TRUE))
    {
        printf("==>Recv DelayReq Not In Same Domain !! Error !!<==\n");
        return;
    }
    
    /**得到接收到request 的时间  */
    GetFpgaRecvMessageTime(pPtpClock,pMsgHeader,&InternalTime);
        
    if(pPtpClock->debugLevel)
    {
        printf("recv DelayReqMessage !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgDelayReq_display(pMsgDelayReq);
    }


    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        /**如果单播协商关，则在这里更新客户端列表  */
        if(!pPtpClock->UniNegotiationEnable)
        {
            RecvMsg_UpdateUnicastClient(pPtpClock,addr,20);
        }
        else/**单播协商开，则检测地址是否在列表里，如果不在则不回复  */
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
* 函数名称:    ProcessPdelayReqMessage
* 功能描述:    处理收到的PdelayReq消息
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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


    /** 收到自己发的消息，直接返回 */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /**是否在同一个域里，不在则返回  */
    if(!isFromSameDomain(pPtpClock,pMsgHeader)  && (pPtpClock->domainFilterSwitch == TRUE))
    {
        printf("==>Recv PdelayReq Not In Same Domain !! Error !!<==\n");
        return;
    }
    
    /**得到接收到request 的时间  */
    GetFpgaRecvMessageTime(pPtpClock,pMsgHeader,&InternalTime);

    if(pPtpClock->debugLevel)
    {
        printf("recv PdelayReq Message !!\n");
        
        //msgHeader_display(pMsgHeader);
        //msgSync_display(pMsgSyn);
    }

    if(pPtpClock->modeType == IPMODE_UNICAST)
    {
        /**如果单播协商关，则在这里更新客户端列表  */
        if(!pPtpClock->UniNegotiationEnable)
        {
            RecvMsg_UpdateUnicastClient(pPtpClock,addr,20);
        }
        else/**单播协商开，则检测地址是否在列表里，如果不在则不回复  */
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
* 函数名称:    ProcessDelayRespMessage
* 功能描述:    处理收到的DelayResp消息
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

    /** 收到自己发的消息，直接返回 */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /** 判断是不是自己的发的Delayreq */
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
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
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
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
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
* 函数名称:    ProcessPdelayRespMessage
* 功能描述:    处理收到的PdelayResp消息
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

    /** 收到自己发的消息，直接返回 */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /** 判断是不是自己的发的pDelayreq */

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
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
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
* 函数名称:    ProcessPdelayRespFollowUpMessage
* 功能描述:    处理收到的PdelayRespFollowUp消息
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *addr           
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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


    /** 收到自己发的消息，直接返回 */
    if(isFromMySelfMaster(pPtpClock,pMsgHeader))
        return;

    /** 判断是不是自己的发的pDelayreq */

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
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
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
        if(isFromBestMaster(pPtpClock,pMsgHeader))/**是最佳主时钟  */
        {
            //msgPDelayRespFollowUp_display(pMsgPDelayRespFollowUp);
            HandlePdelayRespFollowUpMessage(pPtpClock,pMsgHeader,pMsgPDelayRespFollowUp);
        }

    }

}



/********************************************************************************************
* 函数名称:    ProcessIeee802_Msg
* 功能描述:    处理Ieee802.3两层收到的数据包，首先去掉源mac 目的mac，然后匹配vlan
               ，最后调用消息处理函数
* 输入参数:    PtpClock *pPtpClock      
               char *buf                
               int recv_len             
               struct sockaddr_ll addr  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

        /** 如果ID 不匹配 */
        if(pVlanHead->vlanId != pPtpClock->vlanId)
            return;
        
        PLOG("Recv vlan Message\n");

        /**如果不是IEEE802.3 数据则丢掉  */
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
* 函数名称:    Process_Msg
* 功能描述:    处理消息函数，更加不同的2层，3层协议，根据不同的消息类型，选择不同
               处理分支
* 输入参数:    PtpClock *pPtpClock  
               char *buf            
               int recv_len         
               void *pAddr          
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
            /** 保留 */
            break;
            
        default:
            break;
    }


    
}


/********************************************************************************************
* 函数名称:    MasterMode_Recv
* 功能描述:    接收处理线程，主模式下，所有接收处理入口
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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

  
    //多路复用，时间阻塞为1秒
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
* 函数名称:    SlaveMode_Recv
* 功能描述:    接收处理线程，从模式下，所有接收处理入口
* 输入参数:    PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

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
    
        
   /**  多路复用，时间阻塞为1秒 */
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
* 函数名称:    MasterMode_Send
* 功能描述:    发送线程中，主端模式时，运行的入口
* 输入参数:    Uint32 timerIndex    
               PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void MasterMode_Send(Uint32 timerIndex,PtpClock *pPtpClock)
{

    /**告警检测  */
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

    
    /** 多播模式按照interval 轮询发送 */
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
        /** 单播模式按照interval 间隔 更新 */
        if(timerIndex%pPtpClock->synSendInterval == 0)
        {
            Refresh_Syn_UnicastClient(pPtpClock);
        }
        if((timerIndex%pPtpClock->announceSendInterval) == 0)
        {
            Refresh_Announce_UnicastClient(pPtpClock);
        }

        /** 单播模式按照10ms间隔发送 */
        if(timerIndex%2 == 0)
        {
            issueSync(pPtpClock);
            issueAnnounce(pPtpClock);
        }

    }    

}

/********************************************************************************************
* 函数名称:    SlaveMode_Send
* 功能描述:    发送线程中，从端模式时，运行的入口
* 输入参数:    Uint32 timerIndex    
                            PtpClock *pPtpClock  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-11
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void SlaveMode_Send(Uint32 timerIndex,PtpClock *pPtpClock)
{

    UnicastMultiServer *pServer_unicast = &pPtpClock->unicastMultiServer;

    
    /**告警检测  */
    pPtpClock->oneSecondTime++;
    if(pPtpClock->oneSecondTime > TIMER_1_SECOND)
    {
        pPtpClock->oneSecondTime = 0;
        Check_ThreeMessageRecvAlarm(pPtpClock);
        Display_PtpStatusEvery10s(pPtpClock);
    }


    /** 多播模式按照interval 轮询发送 */
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
        /** 单播模式按照interval 间隔 更新 */
        if(timerIndex%pPtpClock->delayreqInterval == 0)
        {
            ReSetSendDelayServer(pPtpClock,pServer_unicast);

        }
        /**单播协商开时，每秒执行更新  */
        if(timerIndex%256 == 0)
        {
            if(pPtpClock->UniNegotiationEnable)
            {
                ResetSendSignalRequest(pPtpClock,pServer_unicast);
                Update_ServUnicastList(pPtpClock,pServer_unicast);
            }
        }
        
        /** 单播模式按照10ms间隔发送 */
        if(timerIndex%2 == 0)
        {
            /** 单播协商开 */
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
            /**单播协商关  */
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

