/********************************************************************************************
*                           ��Ȩ����(C) 2015, ���ſ�ѧ���������о���
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       phy_adjust.c
*    ��������:       ��ʱ�Ӹ�����ʱ�ӣ�ͬ��83640 �㷨
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-3-9
*    �����б�:
                     AdjustFrequency
                     AdjustPhase
                     AdustDp83640SynchronizeMaster
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-3-9
                     ����: HuangFei
                     �޸�����: �´����ļ�


*********************************************************************************************/
#include "fpga_time.h"
#include "phy_adjust.h"
#include "arith.h"

#define DIRECT_CONNECT      1
#define SWITCH_CONNECT      0
#define SLOW_MODIFY         1
#define FAST_MODIFY         0

#define THRESHOLD_US        1000
#define THRESHOLD_1MS       1000000

#define MAX_TIME_OFFSET     2000
#define MAX_MODIIFY         96
#define MIN_MODIFY          16
#define NS1_STEP_FREQUENCY  34.35974

/**Ƶƫ��������10s����һ��  */
#define FREQUENCY_SUPER_INTERVAL 25
#define FREQUENCY_LONG_INTERVAL  10
#define FREQUENCY_SHORT_INTERVAL 7


typedef struct{     
    Slonglong64 setOffset;          
    Slonglong64 ActualOffset;     
    Slonglong64 SumError;          
    Slonglong64 err;          
    Slonglong64 PreErr;          
    Slonglong64 LastErr;           
    float Kp,Ki,Kd;
}PidControl;

PidControl g_Pid;

float Km;             /*kalmanϵ��*/
float Rm=0.1;       
float Qm=0.0001;          /*������������*/
float Xk;        /*״̬����*/
float Xk_1=0;    /*ǰһ״̬����*/
float Pk;        /*״̬������������Э����*/
float Pk_1=1;    /*ǰһ״̬������������Э����*/

short freq_modify_ratio = FREQUENCY_SHORT_INTERVAL;


void AdjustFrequency(PtpClock *pPtpClock,TimeInternal timeOffset_adj);

static short over_cnt = 0;

void TimeOffsetModifyOverMaxRecord(Slonglong64 time_adj)
{
    if(abs(time_adj)>= MAX_MODIIFY)
        over_cnt++;
}
void Reset_OverCnt()
{
    over_cnt =0;

}

void Freq_adapt(PhyControl *pPhyControl)
{
    if(pPhyControl->phase_adjsut>0)
    {
        if(pPhyControl->RateDiff>0)
            pPhyControl->RateDiff -= MAX_MODIIFY;
        else if(pPhyControl->RateDiff <0)
            pPhyControl->RateDiff -= MAX_MODIIFY;
        
        printf("freq adapt modify freq Max %lld \n",pPhyControl->RateDiff);
    }
    else if(pPhyControl->phase_adjsut<0)
    {
        if(pPhyControl->RateDiff>0)
            pPhyControl->RateDiff += MAX_MODIIFY;
        else if(pPhyControl->RateDiff <0)
            pPhyControl->RateDiff += MAX_MODIIFY;
        
        printf("freq adapt modify freq Max %lld \n",pPhyControl->RateDiff);
    }

}

void ResetSmothBuf(PhyControl *pPhyControl)
{
    g_Pid.SumError = 0;
    memset(pPhyControl->time_offset_buf,0,sizeof(pPhyControl->time_offset_buf));
}

/********************************************************************************************
* ��������:    SmoothFilterArith
* ��������:    �޷���������ƽ���˲���
* �������:    PtpClock *pPtpClock      
               PhyControl *pPhyControl  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-3-18
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Slonglong64 SmoothFilterArith(PtpClock *pPtpClock,PhyControl *pPhyControl)
{
  int i;
  Slonglong64 filter_sum = 0;
  if(abs(abs(pPhyControl->time_offset)- abs(pPhyControl->time_offset_buf[9]))> MAX_TIME_OFFSET)
  {
      pPhyControl->time_offset = pPhyControl->time_offset_buf[9];

  }
  
  for(i=0;i<9;i++)
  {
    pPhyControl->time_offset_buf[i] = pPhyControl->time_offset_buf[i + 1]; // �����������ƣ���λ�Ե�
  }
  pPhyControl->time_offset_buf[9] = pPhyControl->time_offset;

  for(i=0;i<10;i++)
  {
        filter_sum += pPhyControl->time_offset_buf[i];
        //printf("filter_sum %d\n",filter_sum);
  }
  filter_sum = filter_sum /10;

  return filter_sum;
}



void ResetKalman_Fileter()
{
    Xk_1 = 0;
    Pk_1=1;
}


Slonglong64 Kalman_Filter(Slonglong64 val)
{
    
    /**�������˲��㷨��ر��� */

	Xk=Xk_1;
	Pk=Pk_1+Qm;

	Km=Pk/(Pk+Rm);
		
	Xk_1=Xk+Km*(val-Xk);
		
	Pk_1=(1-Km)*Pk;

	val=Xk_1;
    
	return val;
}

Slonglong64 MinValue_Filter(Slonglong64 *pBuf,int len)
{
    int i;
    Slong32 ret;
    Slong32 val;

    val = abs(pBuf[0]);
    
    for(i=0;i<len;i++)
    {
        if(val > abs(pBuf[i]))
        {
            val = abs(pBuf[i]);
            ret = pBuf[i];
        }
    }

    return ret;
}


void Init_Pid()
{
    g_Pid.Kp = 0.3;
    //g_Pid.Ki = 0.005;

    g_Pid.Ki = 0.008;
    //g_Pid.Ki = 0.;
    //g_Pid.Kd = 0.;
    g_Pid.Kd = 0.2;
    g_Pid.setOffset = 0;    /**Ŀ��ֵ  */

}

Slonglong64 Pid_Control(PidControl *pPid,Slonglong64 ActualOffset)
{
    Slonglong64 dError,Error;     
    Slonglong64 ControlVal;   
    
    Error =  ActualOffset -pPid->setOffset;
    
    pPid->SumError += Error;    
    pPid->PreErr = Error;    
    dError = pPid->PreErr - pPid->LastErr;    
    pPid->LastErr = Error; 

    printf("errr = %lld,sumError = %lld,derror = %lld \n",Error,pPid->SumError,dError);
    ControlVal = pPid->Kp * Error;    
    ControlVal += (pPid->Ki*pPid->SumError);    
    ControlVal += (pPid->Kd*dError);    
    return ControlVal;

}

void GetCnt_threshold(PtpClock *pPtpClock)
{
    float interval = pow(2,pPtpClock->logSyncInterval);
    interval = interval * freq_modify_ratio;
    if(interval < 1.0)
        interval = 1.0;
    pPtpClock->phyControl.cnt_threshold = (short)interval;
    //printf("=====%d",pPtpClock->phyControl.cnt_threshold);
}

void AdjustPhase(PtpClock *pPtpClock)
{
    short i;
    Slonglong64 tOffsetTmp;
    TimeInternal adjust;

    PhyControl *pPhyControl = &pPtpClock->phyControl;
    internalTime_to_longlong64(pPtpClock->TimeOffset,&pPhyControl->time_offset);


    if(abs(pPhyControl->time_offset) > THRESHOLD_US)
    {
        pPhyControl->low5us_cnt = 0;
        pPhyControl->over5us_cnt++;
    }
    else
    {
        pPhyControl->over5us_cnt = 0;
        pPhyControl->low5us_cnt++;

    }

    /**����20�ν��п첶��΢������  */
    if(pPhyControl->over5us_cnt >= 20)
    {
        pPhyControl->ctl_phase_type = FAST_MODIFY;
        pPhyControl->over5us_cnt = 0;
    }
    if(pPhyControl->low5us_cnt >= 20)
    {
        pPhyControl->ctl_phase_type = SLOW_MODIFY;
        pPhyControl->low5us_cnt = 0;
    }

    if(pPhyControl->ctl_phase_type == FAST_MODIFY)
    {
        if(llabs(pPhyControl->time_offset) > THRESHOLD_US)
        {
            //Dp83640AdjustTime(pPtpClock->TimeOffset);
            //GetDp83640RuningTime(&pPtpClock->CurentTime);
            //TriggerSetAndStart(pPtpClock->CurentTime.seconds+1);
            printf("==========>SET time !!\n");
            ResetSmothBuf(pPhyControl);
        }
        else
        {
            //Dp83640AdjustTime(pPtpClock->TimeOffset);
            
        }
        /**����״ִֻ̬��3��  */
        pPhyControl->in_fastmodify_cnt++;
        if(pPhyControl->in_fastmodify_cnt >= 3)
        {
            pPhyControl->ctl_phase_type = SLOW_MODIFY;
            pPhyControl->in_fastmodify_cnt = 0;
        }
    }
    else if(pPhyControl->ctl_phase_type == SLOW_MODIFY)
    {
#if 0
        if(pPhyControl->line_type == DIRECT_CONNECT)
        {
            if(pPhyControl->time_offset >24)
                pPhyControl->phase_adjsut = 24;
            else if((pPhyControl->time_offset <-24))
                pPhyControl->phase_adjsut = -24;
            else 
                pPhyControl->phase_adjsut = 0;

            printf("\nmean_path %lld ns ,",pPhyControl->mean_path);
            printf("time_offset %lld ns ,",pPhyControl->time_offset);
            printf("direct link taj %lld ns\n\n",-pPhyControl->phase_adjsut);
            
            longlong64_to_internalTime(pPhyControl->phase_adjsut,&adjust);
            Dp83640AdjustTime(adjust);
            AdjustFrequency(pPtpClock,adjust);
            pPhyControl->ratediff_cnt++;
            return;
        }
#endif

        tOffsetTmp = SmoothFilterArith(pPtpClock,pPhyControl);
               
        log_debug("time_offset %d ",pPhyControl->time_offset);
        log_debug("off_tmp %d ",tOffsetTmp);
        pPhyControl->averag_tmp = tOffsetTmp;
        
        pPhyControl->phase_adjsut = Pid_Control(&g_Pid,tOffsetTmp);
        
        //pPhyControl->phase_adjsut += 16;/**����  */
        log_debug("adust_tmp %d ",pPhyControl->phase_adjsut);
        
        if(abs(pPhyControl->phase_adjsut) > MAX_MODIIFY)
        {
            if(pPhyControl->phase_adjsut > 0)
                pPhyControl->phase_adjsut = MAX_MODIIFY;

            else
                pPhyControl->phase_adjsut = -MAX_MODIIFY;
        }
        else if(abs(pPhyControl->phase_adjsut) < MIN_MODIFY)
        {
            pPhyControl->phase_adjsut = 0;
            
        }
        printf("\nmean_path %lld ns ,",pPhyControl->mean_path);
        printf("time_offset %lld ns ,",pPhyControl->time_offset);
        printf("taj %lld ns\n\n",-pPhyControl->phase_adjsut);
        
        if(pPhyControl->phase_adjsut != 0)
        {
            longlong64_to_internalTime(pPhyControl->phase_adjsut,&adjust);
            //Dp83640AdjustTime(adjust);
            
            /**����΢��״̬����ʱƫ���Ϊ�̶������е�Ƶ����  */
            //AdjustFrequency(pPtpClock,adjust);
            pPhyControl->ratediff_cnt++;
        }
        else
        {
            //log_debug("rea2 adust_tmp %d\n",pPhyControl->phase_adjsut);

        }
       
    }
        
}




void AdjustFrequency(PtpClock *pPtpClock,TimeInternal timeOffset_adj)
{
    
    PhyControl *pPhyControl = &pPtpClock->phyControl;
    int i,j;

    Slonglong64 freqAdjust = 0;
    Slonglong64 tmp_T1 = 0;
    Slonglong64 tmp_T2 = 0;
    Slonglong64 tmp = 0;
    Slonglong64 minval = 0;
    
    TimeInternal diff_T1_tmp;
    TimeInternal diff_T2_tmp;
    TimeInternal diff_tmp;


    /** ��ʱ��  */

    if(pPhyControl->ratediff_cnt >= pPhyControl->cnt_threshold)
    {
        subTime(&diff_T1_tmp,&pPtpClock->T1,&pPhyControl->lastT1);
        subTime(&diff_T2_tmp,&pPtpClock->T2,&pPhyControl->lastT2);

        /**���ʱ����ֲ���ȷ������  */
        if(isTimeInternalNegative(&diff_T1_tmp) ||
            isTimeInternalNegative(&diff_T2_tmp))
        {
            printf("==========>TimeOffset Result error!! sec %d,nao %d\n",diff_T1_tmp.seconds,diff_T1_tmp.nanoseconds);
            return;
        }

        /**������������  */
        addTime(&diff_T2_tmp,&diff_T2_tmp,&timeOffset_adj);
        
        subTime(&diff_tmp,&diff_T2_tmp,&diff_T1_tmp);
        
        //printf("diff T2-T1:sec=%d,nao=%d ",diff_tmp.seconds,diff_tmp.nanoseconds);
        
        internalTime_to_longlong64(diff_tmp,&tmp);
        internalTime_to_longlong64(diff_T1_tmp,&tmp_T1);
        
        tmp = tmp * 1000000000;
        pPhyControl->diff_buf[pPhyControl->diff_index]= tmp / tmp_T1;

        printf("====>diff %lld\n",pPhyControl->diff_buf[pPhyControl->diff_index]);

        pPhyControl->diff_index++;

        if(pPhyControl->diff_index >= 5)
        {
            pPhyControl->diff_index = 0;
            
#if 0
            for(i=0;i<5;i++)
            {
                pPhyControl->calman_diff_buf[i] = Kalman_Filter(pPhyControl->diff_buf[i]);
            }
#endif
            for(i=0;i<5;i++)
            {
                pPhyControl->calman_diff_buf[i] = (pPhyControl->diff_buf[i]);
            }

            //ResetCalman_Fileter();

            tmp = 0;
            for(i=0;i<5;i++)
            {
                tmp += pPhyControl->calman_diff_buf[i];
                
            }

            tmp = tmp / 5;

            printf("SWITCH_CONNECT ratediff %lld ",tmp);
  
            
            {
                if(abs(tmp)>THRESHOLD_US)
                    tmp = THRESHOLD_US;
 
                 if(g_Pid.SumError > 100)
                     pPhyControl->RateDiff -= 2;
                 else if(g_Pid.SumError < -100)
                     pPhyControl->RateDiff += 2;

            }
            
            /**��һ�ε���  */
            if(pPhyControl->RateDiff == 0)
            {

                /**���ֵƵƫ��Ҫ����1us  */
                if(abs(tmp)>THRESHOLD_US)
                    tmp = THRESHOLD_US;
                
                if(pPhyControl->line_type == DIRECT_CONNECT)
                {
                    if(g_Pid.SumError > 100)
                        pPhyControl->RateDiff -= 2;
                    else if(g_Pid.SumError < -100)
                        pPhyControl->RateDiff += 2;
                }
                else
                {
                    if(g_Pid.SumError> 0)
                        tmp = -abs(tmp);
                    else
                        tmp = abs(tmp);
                    
                    pPhyControl->RateDiff = tmp;

                }
                

                freqAdjust = pPhyControl->RateDiff;
                freqAdjust = freqAdjust * NS1_STEP_FREQUENCY;
                Dp83640AdjustFrequency(freqAdjust);
                ResetSmothBuf(pPhyControl);
                
            }
            else/**���������ۼ�ƫ��  */
            {
                /** 1������ */
                if(abs(tmp) < 5)
                {
                    pPhyControl->OverMaxRateCnt = 0;

                    if(g_Pid.SumError>32)
                        pPhyControl->RateDiff -= 1;
                    else if(g_Pid.SumError <-32)
                        pPhyControl->RateDiff += 1;
                    else
                    {
                        log_debug("Frequency no Modify \n");
                        
                    }

                    log_debug("pPhyControl->RateDiff %d ",pPhyControl->RateDiff);
                                        
                    freqAdjust = pPhyControl->RateDiff;
                    freqAdjust = freqAdjust * NS1_STEP_FREQUENCY;
                    Dp83640AdjustFrequency(freqAdjust);
                    
                    freq_modify_ratio = FREQUENCY_SUPER_INTERVAL;
                              
                    
                }
                else if(abs(tmp) < 32 && abs(tmp) >= 5)
                {
                    pPhyControl->OverMaxRateCnt = 0;
                    
                    if(g_Pid.SumError>0)
                        pPhyControl->RateDiff -= 2;
                    else if(g_Pid.SumError <0)
                        pPhyControl->RateDiff += 2;
                    
                    log_debug("pPhyControl->RateDiff %d ",pPhyControl->RateDiff);
                    
                    freqAdjust = pPhyControl->RateDiff;
                    freqAdjust = freqAdjust * NS1_STEP_FREQUENCY;
                    Dp83640AdjustFrequency(freqAdjust);
                    freq_modify_ratio = FREQUENCY_LONG_INTERVAL;
                }
                /**����������ֵ�������������ޣ����������  */
                else if(abs(tmp) >MAX_MODIIFY)
                {
                    if(g_Pid.SumError > 0)
                        pPhyControl->RateDiff -= (MAX_MODIIFY/2);
                    else if(g_Pid.SumError < 0)
                        pPhyControl->RateDiff += (MAX_MODIIFY/2);
                    

                    log_debug("pPhyControl->RateDiff %d ",pPhyControl->RateDiff);
                    
                    freqAdjust = pPhyControl->RateDiff;
                    freqAdjust = freqAdjust * NS1_STEP_FREQUENCY;
                    Dp83640AdjustFrequency(freqAdjust);

                    freq_modify_ratio = FREQUENCY_LONG_INTERVAL;


                    pPhyControl->OverMaxRateCnt++;
                    if(pPhyControl->OverMaxRateCnt >= 3)
                    {
                        pPhyControl->RateDiff = 0;
                        pPhyControl->OverMaxRateCnt = 0;
                        log_debug("Big modify \n");
                        
                    }

                }
                else/** ��У��Χ�ڣ������΢��������Ϊ4*/
                {
                    pPhyControl->OverMaxRateCnt = 0;
                                        
                    if(g_Pid.SumError>0)
                        pPhyControl->RateDiff -= 4;
                    else if(g_Pid.SumError<0)
                        pPhyControl->RateDiff += 4;

                    log_debug("pPhyControl->RateDiff %d ",pPhyControl->RateDiff);
                    
                    freqAdjust = pPhyControl->RateDiff;
                    freqAdjust = freqAdjust * NS1_STEP_FREQUENCY;
                    Dp83640AdjustFrequency(freqAdjust);
                    freq_modify_ratio = FREQUENCY_LONG_INTERVAL;
                }
                
            }

        }

        pPhyControl->ratediff_cnt = 0;
    }
    
    /**��ʱ10 ��ȡһ��ʱ��  */
    if(pPhyControl->ratediff_cnt == 0)
    {
        pPhyControl->lastT1 = pPtpClock->T1;
        pPhyControl->lastT2 = pPtpClock->T2;
    }

   
}

void AdustDp83640SynchronizeMaster(PtpClock *pPtpClock,PhyControl *pPhyControl)
{

    /**��·ʱ�Ӳ�����Ϊ��ֵ������Ǹ�ֵ�����쳣  */
    if(isTimeInternalNegative(&pPtpClock->MeanPathDelay))
    {
        log_debug("TimeOffset Result error!!\n");
        return;
    }

    if(abs(pPtpClock->T1.seconds-pPtpClock->CurentTime.seconds)>2)
    {
        pPhyControl->over2sec_counter++;

        if(pPhyControl->over2sec_counter >= 10)
        {
            /**����ʱ�� */
            Dp83640ClockTimeSet(pPtpClock->T1);
            
            GetDp83640RuningTime(&pPtpClock->CurentTime);
            TriggerSetAndStart(pPtpClock->CurentTime.seconds+1);
            
            printf("==>Set dp83640 Big time===>\n");
            
            pPhyControl->over2sec_counter= 0;
        }

        /**����2s ����ϸ������ֱ�ӷ���  */
        return;
    }

    pPhyControl->over2sec_counter= 0;

    internalTime_to_longlong64(pPtpClock->MeanPathDelay,&pPhyControl->mean_path);
        
        /**�����·����  */
    if(pPhyControl->mean_path > 300)
    {
        pPhyControl->low1us_counter = 0;
        pPhyControl->over1us_counter++;
        
        if(pPhyControl->over1us_counter > 10)
        {
            pPhyControl->over1us_counter = 0;
            pPhyControl->line_type = SWITCH_CONNECT;
        }

    }
    else if(pPhyControl->mean_path < 300)
    {
        pPhyControl->over1us_counter = 0;
        pPhyControl->low1us_counter++;

        if(pPhyControl->low1us_counter > 10)
        {
            pPhyControl->low1us_counter = 0;
            pPhyControl->line_type = DIRECT_CONNECT;
            printf("In DIRECT_CONNECT \n");
        }
    }
    
    /**��Ƶ����  */
    AdjustPhase(pPtpClock);

   
}


