/********************************************************************************************
*                           ��Ȩ����(C) 2015, 
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       clock_rb.h
*    ��������:       �ӿ�ģ�飬������ӣ�״̬�����ƣ��ռ�����ֵ
*    ����:           Felix
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2017-7-29
*    �����б�:
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-7-29
                     ����: Felix
                     �޸�����: �´����ļ�


*********************************************************************************************/

#ifndef _CLOCK_RB_H_
#define _CLOCK_RB_H_

#include "common.h"


#define MAX  50
#define FREE 0x0
#define FAST 0x1
#define LOCK 0x2
#define HOLD 0x3

#define REF_SATLITE 0
#define REF_10MHZ   1
#define REF_2KHz    3

#define E1FastKaSmall 30
#define E1FastKaMiddle 40  //<1exp(-9)
#define E1FastKaLarge 200  //<2exp(-8)
#define E1FastKaHuge 250   //<4.6ppm
#define Xo_FEI405_exp_2_8  500//910000   //>2* exp(-8)	 1000
#define E1LockPhaseKp 40         //test  20
#define E1LockPhaseKa0 40  //64*3/4	 48
#define E1LockPhaseKa1 12  //64*1/4	   16
//#define E1LockConstraint 350
#define Adjust_exp_8  896157  // FEI405A XO adjust 1*exp(-8)//9.9 500000-800000
//#define XoAdjust_exp_9  50000   //FEI405A XO adjust  1*exp(-9)
#define XoAdjust_exp_12 50   //FEI405A adjust 1*exp(-12)


struct clock_alarm_data
{
    /** ����ʱ�� */
    Uint8 alarmRb10M;      /**���10MHz�첨�澯  */
    Uint8 alarmVcxo25M;     /**VCXO 25MHz�첨�澯 */
    Uint8 alarmVcxo24_576M; /**VCXO24.576MHz�첨�澯  */
    Uint8 alarmVcxo25MPoe;  /**VCX0 25MHz��POE���첨�澯  */
    Uint8 alarm25MPll;      /**25MHz���໷ʧ���澯  */
    Uint8 alarm24_576MPll;  /**24.576MHz���໷ʧ���澯  */
    Uint8 alarm25MPoePll;   /*25MHz��POE�����໷ʧ���澯*  */

    /**�����ź� */
    Uint8 alarmBd1pps;      /**BD���ջ�1PPS�첨��  */
    Uint8 alarmBdTod;       /**BD���ջ����ڱ��ļ첨�澯  */
    Uint8 alarmRef10M;      /**�ο��첨�澯  */
    Uint8 ref10Mor2M;       /**1:10M    2,2M   3,�澯*/
    Uint8 alarmInputPpsTod; /**1PPS��+TOD���첨 */
    Uint8 alarmInputTod;         /**TOD�첨  */
    Uint8 alarmInputDcls;   /**DCLS�첨  */
    Uint8 alarmInput26503_2M;/**DS26503���2MHz�첨  */
    Uint8 alarmBattery;
    Uint8 alarmSatellite;  /**�����źų�����  1�澯��0 ���澯*/
};

struct collect_data
{

   char uPdOverFlow;  /*����N����� ����0.4S*/
   char Over_100us;   /*����N�δ���100us,��ǰ������βɼ�*/
   float  ph_max;             /*���ֵ*/
   float  ph_min;             /*��Сֵ*/

   int ph_number;          /*ÿ���������  */
   int queue_number;       /*����          */

   float phase_offset;      /**������λƫ��  */
   float  add_ph;             /*�ۼƼ���ֵ*/ 
   int  ph_number_counter;  /*��ǰ���������ܸ���*/
   int  queue_number_counter;  /*��ǰ�������ݶ��и���*/

   float  phase_array[MAX] ; /*���������ռ�����*/

   float  lAvgPhase;       /*ƽ����������*/
   float  set_lAvgPhase;   /*Ԥ���ƽ����������*/

   char  getdata_flag;      /* �ɼ���һ������*/
   
   char  finish_one_flag;   /**���һ������  */
   char start_flag;

};

struct clock_info
{
   /**����  ��״̬��*/
   char      workStatus;  /*����״̬:free ��fast ��lock��hold*/
   char      ref_type;          /*�ο����룺0:����1pps ��1: ptp��*/
   char      clock_mode;        /*ʱ��ģʽ0���ڲ���1���ⲿ*/
   float     lPhasePrevious; /*��һ��ƽ����������*/

   char      modify_flag;
   char      modify_cnt;

   char      bInitialClear;  /*��¼��һ�Σ��Ƿ��е������0:�޵���  1���е���*/
   char      adjflag;       /*�Ƿ������־*/

   float     lDetDdsAdj;     /*�������ֵ*/ 
   float     IDetPhase;		 /*ƫ��ֵ*/
   float     PrelDetPhase;	 /*��һ�ε�ƫ��ֵ*/
   float     PrelDetDdsAdj;	 //��һ�ξ����ӵ���ֵ
   
   float     lAccPhaseAll;	 /*ƫ����ۼ�ֵ*/
   float     OffsetAll;	     /*ƫ����ۼ�ֵ*/

   int       center;          /**add by hf , ����ֵ  */

   char      lockCounter;     /*��������*/
   char      unlockCounter;   /*δ��������*/
   char      syn_enable;      /** ����һ����λ���� */
   char      ref_change_flag; /**1:�ο�Դ�ı䣬0���ο�Դ����  */


#if 0
   struct collect_data phase_test_2k;  /*100Hz�����ռ�*/
   struct collect_data data_100Hz;
   struct collect_data phase_test_1Hz;
#endif
   struct collect_data data_1Hz;   /*1HZ�����ռ�*/
   /*��ʱ���*/
   unsigned int fast_times; /*�첶��ʱ����*/
   unsigned int lock_times; /*������ʱ����*/
   unsigned int hold_times; /*������ʱ����*/

    
    /**�ο�Դ���  */
    unsigned int run_times;  /*ϵͳ����ʱ��*/

    unsigned int core_time;  /*����ʱ��(CPUά����ʱ�䣬д��ʱ��������)*/

    time_t time_now;            /**��һ�ε�����ӵ�ʱ��  */
    struct clock_alarm_data alarmData;

   
};


extern void SetRbClockWorkMode();
extern void RequestRbClockStatus();
extern void send_absolute_clock_data(int data);
extern void gps_status_machine(struct clock_info *p_clock_info,char ref_status);

extern void gps_status_handle(struct clock_info *p_clock_info);

extern void ClockStateProcess(struct clock_info *pClockInfo);
extern void ClockHandleProcess(struct clock_info *pClockInfo);

extern void collect_phase(struct collect_data *p_collect_data,int delay,float ph);

#endif /* RB_H_ */

