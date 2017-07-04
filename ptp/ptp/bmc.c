/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       bmc.c
*    功能描述:       最佳主时钟选择算法
*    作者:           黄飞
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-19
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-19
                     作者: 黄飞
                     修改内容: 新创建文件


*********************************************************************************************/

#include "bmc.h"
#include "comm_fun.h"
#include "msg.h"


/********************************************************************************************
* 函数名称:    CopyAnnounceToParam
* 功能描述:    拷贝收到的Announce 到结构体存储
* 输入参数:    MsgHeader *header      
               MsgAnnounce *announce  
               PtpClock *ptpClock     
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void CopyAnnounceToParam(MsgHeader *header, MsgAnnounce *announce, PtpClock *ptpClock)
{
#if 0
	Boolean previousLeap59 = FALSE, previousLeap61 = FALSE;
	Integer16 previousUtcOffset = 0;


	if (ptpClock->clockType == PTP_SLAVE || ptpClock->clockType ==PTP_PASSIVE) {
		previousLeap59 = ptpClock->timePropertiesDS.leap59;
		previousLeap61 = ptpClock->timePropertiesDS.leap61;
		previousUtcOffset = ptpClock->timePropertiesDS.currentUtcOffset;
	}
#endif
	/* Current DS */
	ptpClock->stepsRemoved = announce->stepsRemoved + 1;

	/* Parent DS */
	copyClockIdentity(ptpClock->parentPortIdentity.clockIdentity,
	       header->sourcePortIdentity.clockIdentity);
	ptpClock->parentPortIdentity.portNumber = 
		header->sourcePortIdentity.portNumber;
	copyClockIdentity(ptpClock->grandmasterIdentity,
			announce->grandmasterIdentity);
	ptpClock->grandmasterClockQuality.clockAccuracy = 
		announce->grandmasterClockQuality.clockAccuracy;
	ptpClock->grandmasterClockQuality.clockClass = 
		announce->grandmasterClockQuality.clockClass;
	ptpClock->grandmasterClockQuality.offsetScaledLogVariance = 
		announce->grandmasterClockQuality.offsetScaledLogVariance;
	ptpClock->grandmasterPriority1 = announce->grandmasterPriority1;
	ptpClock->grandmasterPriority2 = announce->grandmasterPriority2;

	ptpClock->logAnnounceInterval = header->logMessageInterval;

	/* Timeproperties DS */
	ptpClock->timePropertiesDS.currentUtcOffset = announce->currentUtcOffset;

    /* "Valid" is bit 2 in second octet of flagfield */
    ptpClock->timePropertiesDS.currentUtcOffsetValid = IS_SET(header->flagField1, UTCV);


	/* only set leap state in slave mode */
	if (ptpClock->clockType == PTP_SLAVE)
    {
		ptpClock->timePropertiesDS.leap59 = IS_SET(header->flagField1, LI59);
		ptpClock->timePropertiesDS.leap61 = IS_SET(header->flagField1, LI61);


        ptpClock->timePropertiesDS.timeTraceable = IS_SET(header->flagField1, TTRA);
        ptpClock->timePropertiesDS.frequencyTraceable = IS_SET(header->flagField1, FTRA);
        ptpClock->timePropertiesDS.ptpTimescale = IS_SET(header->flagField1, PTPT);
        ptpClock->timePropertiesDS.timeSource = announce->timeSource;
   	}
}

/********************************************************************************************
* 函数名称:    bmcDataSetComparison
* 功能描述:    BMC比较算法，按照1588协议
* 输入参数:    MsgHeader *headerA      
               MsgAnnounce *announceA  
               MsgHeader *headerB      
               MsgAnnounce *announceB  
               PtpClock *ptpClock      
* 输出参数:    无
* 返回值:      static
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
Integer8 bmcDataSetComparison(MsgHeader *headerA,MsgAnnounce *announceA,
		     MsgHeader *headerB,MsgAnnounce *announceB,
		     PtpClock *ptpClock)
{
	short comp = 0;
	/*Identity comparison*/
	comp = memcmp(announceA->grandmasterIdentity,announceB->grandmasterIdentity,CLOCK_IDENTITY_LENGTH);

	if (comp!=0)
	{
        	/* Compare GM priority1 */
    	if (announceA->grandmasterPriority1 < announceB->grandmasterPriority1)
    		return -1;
    	if (announceA->grandmasterPriority1 > announceB->grandmasterPriority1)
    		return 1;

    	/* Compare GM class */
    	if (announceA->grandmasterClockQuality.clockClass <
    			announceB->grandmasterClockQuality.clockClass)
    		return -1;
    	if (announceA->grandmasterClockQuality.clockClass >
    			announceB->grandmasterClockQuality.clockClass)
    		return 1;
    	
    	/* Compare GM accuracy */
    	if (announceA->grandmasterClockQuality.clockAccuracy <
    			announceB->grandmasterClockQuality.clockAccuracy)
    		return -1;
    	if (announceA->grandmasterClockQuality.clockAccuracy >
    			announceB->grandmasterClockQuality.clockAccuracy)
    		return 1;

    	/* Compare GM offsetScaledLogVariance */
    	if (announceA->grandmasterClockQuality.offsetScaledLogVariance <
    			announceB->grandmasterClockQuality.offsetScaledLogVariance)
    		return -1;
    	if (announceA->grandmasterClockQuality.offsetScaledLogVariance >
    			announceB->grandmasterClockQuality.offsetScaledLogVariance)
    		return 1;
    	
    	/* Compare GM priority2 */
    	if (announceA->grandmasterPriority2 < announceB->grandmasterPriority2)
    		return -1;
    	if (announceA->grandmasterPriority2 > announceB->grandmasterPriority2)
    		return 1;

    	/* Compare GM identity */
    	if (comp < 0)
    		return -1;
    	else if (comp > 0)
    		return 1;
    	return 0;
    }

	  /* Algorithm part2 Fig 28 */
	if (announceA->stepsRemoved > announceB->stepsRemoved+1)
		return 1;
	if (announceA->stepsRemoved+1 < announceB->stepsRemoved)
		return -1;

	/* A within 1 of B */

	if (announceA->stepsRemoved > announceB->stepsRemoved) {
		comp = memcmp(headerA->sourcePortIdentity.clockIdentity,ptpClock->parentPortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH);
		if(comp < 0)
			return -1;
		if(comp > 0)
			return 1;
		PLOG("Sender=Receiver : Error -0\n");
		return 0;
	}

	if (announceA->stepsRemoved < announceB->stepsRemoved) {
		comp = memcmp(headerB->sourcePortIdentity.clockIdentity,ptpClock->parentPortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH);

		if(comp < 0)
			return -1;
		if(comp > 0)
			return 1;
		PLOG("Sender=Receiver : Error -1\n");
		return 0;
	}
	/*  steps removed A = steps removed B */
	comp = memcmp(headerA->sourcePortIdentity.clockIdentity,headerB->sourcePortIdentity.clockIdentity,CLOCK_IDENTITY_LENGTH);

	if (comp<0) {
		return -1;
	}

	if (comp>0) {
		return 1;
	}

	/* identity A = identity B */

	if (headerA->sourcePortIdentity.portNumber < headerB->sourcePortIdentity.portNumber)
		return -1;
	if (headerA->sourcePortIdentity.portNumber > headerB->sourcePortIdentity.portNumber)
		return 1;

	PLOG("Sender=Receiver!!\n");
	return 0;

}

/********************************************************************************************
* 函数名称:    BmcIp
* 功能描述:    BMC入口为IP
* 输入参数:    MsgHeader *headerA      
               MsgAnnounce *announceA  
               Uint32 *ip              
               PtpClock *pPtpClock     
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void BmcIp(MsgHeader *headerA,MsgAnnounce *announceA,Uint32 *ip,PtpClock *pPtpClock)
{
    if(pPtpClock->InitBmcflag)
    {
        pPtpClock->BestMasterRecord.header = *headerA;
        pPtpClock->BestMasterRecord.announce = *announceA;
        pPtpClock->BestMasterIp = *ip;
        CopyAnnounceToParam(headerA,announceA,pPtpClock);
        pPtpClock->InitBmcflag = FALSE;
    }
    else
    {
        if(bmcDataSetComparison(headerA,announceA
            ,&pPtpClock->BestMasterRecord.header,&pPtpClock->BestMasterRecord.announce,pPtpClock) < 0)
        {
            pPtpClock->BestMasterRecord.changeCnt++;
            
            if(pPtpClock->BestMasterRecord.changeCnt >= 10)
            {
                pPtpClock->BestMasterRecord.header = *headerA;
                pPtpClock->BestMasterRecord.announce = *announceA;
                //pPtpClock->BestMasterRecord.foreignMasterAnnounceMessages
                //pPtpClock->BestMasterRecord.foreignMasterPortIdentity
                
                pPtpClock->BestMasterIp = *ip;
                CopyAnnounceToParam(headerA,announceA,pPtpClock);

                pPtpClock->BestMasterRecord.changeCnt = 0;
                
                SLOG("Best Clock Change !!\n");
            }
        }
        /**相同，同一个主钟announce  */
        else
        {
            if(pPtpClock->BestMasterRecord.changeCnt > 0)
                pPtpClock->BestMasterRecord.changeCnt = 0;
        }
        
    }
    

}

/********************************************************************************************
* 函数名称:    BmcMac
* 功能描述:    BMC入口为MAC
* 输入参数:    MsgHeader *headerA      
               MsgAnnounce *announceA  
               char *pMac              
               PtpClock *pPtpClock     
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-2-6
               作者:     HuangFei
               修改内容: 新生成函数

********************************************************************************************/
void BmcMac(MsgHeader *headerA,MsgAnnounce *announceA,char *pMac,PtpClock *pPtpClock)
{
    if(pPtpClock->InitBmcflag)
    {
        pPtpClock->BestMasterRecord.header = *headerA;
        pPtpClock->BestMasterRecord.announce = *announceA;
        memcpy(pPtpClock->BestMasterMac,pMac,6);
        CopyAnnounceToParam(headerA,announceA,pPtpClock);
        pPtpClock->InitBmcflag = FALSE;
    }
    else
    {
        if(bmcDataSetComparison(headerA,announceA
            ,&pPtpClock->BestMasterRecord.header,&pPtpClock->BestMasterRecord.announce,pPtpClock) < 0)
        {
            pPtpClock->BestMasterRecord.changeCnt++;

            if(pPtpClock->BestMasterRecord.changeCnt >= 10)
            {
                pPtpClock->BestMasterRecord.header = *headerA;
                pPtpClock->BestMasterRecord.announce = *announceA;
                //pPtpClock->BestMasterRecord.foreignMasterAnnounceMessages
                //pPtpClock->BestMasterRecord.foreignMasterPortIdentity
                
                memcpy(pPtpClock->BestMasterMac,pMac,6);
                CopyAnnounceToParam(headerA,announceA,pPtpClock);

                pPtpClock->BestMasterRecord.changeCnt = 0;
                
                SLOG("Best Clock Change !!\n");

            }
        }
        /**相同，同一个主钟announce  */
        else
        {
            if(pPtpClock->BestMasterRecord.changeCnt > 0)
                pPtpClock->BestMasterRecord.changeCnt = 0;
        }
    }


}

