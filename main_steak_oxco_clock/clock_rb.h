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


#define RUN_TIME 300


#define MAX  50
#define FREE 0x0
#define FAST 0x1
#define LOCK 0x2
#define HOLD 0x3

#define REF_SATLITE 0
#define REF_10MHZ   1
#define REF_2KHz    2
#define REF_PTP     3

#define RB_CENTER_VALUE  10000
#define GpsFastG1 2  //��20   30X 1015 
#define GpsLockG1 1   //��6   6 1014
#define GpsLockG2 4   //��4
#define GpsLockG3 10    //10//
#define XoAdjust_exp_8 800000.0		//800000.0
#define XoAdjust_exp_9 50000.0	   // 50000.0
#define GpsFastConstraint 10000.0
#define GpsLockConstraint 550.0 //��350.0		//1016g
#define GpsAccConstraint 60000.0



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
    /**�����ź� */
    Uint8 alarmBd1pps;      /**BD���ջ�1PPS�첨�澯*/
    Uint8 alarmPtp;
    Uint8 alarmVcxo100M;     /**vcxo 100M�澯 */
    Uint8 alarmRb10M;      /**���10 M �澯 */
    Uint8 alarmXo10M;       /**XO 10M�澯*/
    Uint8 alarmVcxoLock;
    Uint8 alarmDisk;
    Uint8 alarmSatellite;  /**�����źŶ�ʧ  1�澯��0 ���澯*/
    
};

struct collect_data
{

   char uPdOverFlow;  /*����N����� ����0.4S*/
   char Over_100us;   /*����N�δ���100us,��ǰ������βɼ�*/
   int  ph_max;             /*���ֵ*/
   int  ph_min;             /*��Сֵ*/

   int ph_number;          /*ÿ���������  */
   int queue_number;       /*����          */

   int phase_offset;      /**������λƫ��  */
   int  add_ph;             /*�ۼƼ���ֵ*/ 
   int  ph_number_counter;  /*��ǰ���������ܸ���*/
   int  queue_number_counter;  /*��ǰ�������ݶ��и���*/

   int  phase_array[MAX] ; /*���������ռ�����*/

   float  lAvgPhase;       /*ƽ����������*/
   int  set_lAvgPhase;   /*Ԥ���ƽ����������*/

   char  getdata_flag;      /* �ɼ���һ������*/
   
   char  finish_one_flag;   /**���һ������  */
   char start_flag;

};

struct clock_info
{
   /**����  ��״̬��*/
   char      workStatus;  /*����״̬:free ��fast ��lock��hold*/
   char      ref_type;          /*�ο����룺0:����1pps ��1: ptp��*/
   char      clock_mode;        /**0: ��ӣ�1:OCXO  */
   float     lPhasePrevious; /*��һ��ƽ����������*/

   char      modify_flag;
   char      modify_cnt;

   char      bInitialClear;  /*��¼��һ�Σ��Ƿ��е������0:�޵���  1���е���*/
   char      adjflag;       /*�Ƿ������־*/

   float     lDetDdsAdj;     /*�������ֵ*/ 
   float     IDetPhase;		 /*ƫ��ֵ*/
   int     PrelDetPhase;	 /*��һ�ε�ƫ��ֵ*/
   int     PrelDetDdsAdj;	 //��һ�ξ����ӵ���ֵ
   
   float     lAccPhaseAll;	 /*ƫ����ۼ�ֵ*/
   int     OffsetAll;	     /*ƫ����ۼ�ֵ*/

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
extern void Init_RbClockCenter(struct clock_info *pClock_info);

extern void collect_phase(struct collect_data *p_collect_data,int delay,int ph);

#endif /* RB_H_ */
