/********************************************************************************************
*                           版权所有(C) 2015, 
*                                 版权所有
*********************************************************************************************
*    文 件 名:       clock_rb.h
*    功能描述:       钟控模块，控制铷钟，状态机控制，收集鉴相值
*    作者:           Felix
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2017-7-29
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-7-29
                     作者: Felix
                     修改内容: 新创建文件


*********************************************************************************************/

#ifndef _CLOCK_RB_H_
#define _CLOCK_RB_H_

#include "common.h"


#define RUN_TIME 30


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
#define GpsFastG1 20  //二20   30X 1015 
#define GpsLockG1 10   //二6   6 1014
#define GpsLockG2 4   //二4
#define GpsLockG3 1024//10//
#define XoAdjust_exp_8 800000.0		//800000.0
#define XoAdjust_exp_9 50000.0	   // 50000.0
#define GpsFastConstraint 10000
#define GpsLockConstraint 550 //二350.0		//1016g
#define GpsAccConstraint 60000



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
    /**输入信号 */
    Uint8 alarmBd1pps;      /**BD接收机1PPS检波告警*/
    Uint8 alarmPtp;
    Uint8 alarmVcxo100M;     /**vcxo 100M告警 */
    Uint8 alarmRb10M;      /**铷钟10 M 告警 */
    Uint8 alarmXo10M;       /**XO 10M告警*/
    Uint8 vcxoLock;
    Uint8 alarmDisk;
    Uint8 alarmSatellite;  /**卫星信号丢失  1告警，0 不告警*/
    
};

struct collect_data
{

   char uPdOverFlow;  /*连续N次溢出 调相0.4S*/
   char Over_100us;   /*连续N次大于100us,提前结果本次采集*/
   int  ph_max;             /*最大值*/
   int  ph_min;             /*最小值*/

   int ph_number;          /*每组记数个数  */
   int queue_number;       /*组数          */

   int phase_offset;      /**本次相位偏差  */
   int  add_ph;             /*累计鉴相值*/ 
   int  ph_number_counter;  /*当前鉴相数据总个数*/
   int  queue_number_counter;  /*当前鉴相数据队列个数*/

   int  phase_array[MAX] ; /*鉴相数据收集数组*/

   int  lAvgPhase;       /*平均鉴相数据*/
   int  set_lAvgPhase;   /*预想的平均鉴相数据*/

   char  getdata_flag;      /* 采集到一组数据*/
   
   char  finish_one_flag;   /**获得一次数据  */
   char start_flag;

};

struct clock_info
{
   /**用于  钟状态机*/
   char      workStatus;  /*工作状态:free 、fast 、lock、hold*/
   char      ref_type;          /*参考输入：0:卫星1pps ，1: ptp）*/
   char      clock_mode;        /*时钟模式0，内部，1，外部*/
   int     lPhasePrevious; /*上一次平均鉴相数据*/

   char      modify_flag;
   char      modify_cnt;

   char      bInitialClear;  /*记录上一次，是否有调相操作0:无调相  1：有调相*/
   char      adjflag;       /*是否调整标志*/

   int     lDetDdsAdj;     /*计算调节值*/ 
   int     IDetPhase;		 /*偏差值*/
   int     PrelDetPhase;	 /*上一次的偏差值*/
   int     PrelDetDdsAdj;	 //上一次晶体钟调整值
   
   int     lAccPhaseAll;	 /*偏差差累加值*/
   int     OffsetAll;	     /*偏差差累加值*/

   int       center;          /**add by hf , 调整值  */

   char      lockCounter;     /*锁定次数*/
   char      unlockCounter;   /*未锁定次数*/
   char      syn_enable;      /** 设置一次相位对齐 */
   char      ref_change_flag; /**1:参考源改变，0，参考源不变  */


#if 0
   struct collect_data phase_test_2k;  /*100Hz数据收集*/
   struct collect_data data_100Hz;
   struct collect_data phase_test_1Hz;
#endif
   struct collect_data data_1Hz;   /*1HZ数据收集*/
   /*计时相关*/
   unsigned int fast_times; /*快捕用时长度*/
   unsigned int lock_times; /*跟踪用时长度*/
   unsigned int hold_times; /*保持用时长度*/

    
    /**参考源相关  */
    unsigned int run_times;  /*系统运行时间*/

    unsigned int core_time;  /*核心时间(CPU维护该时间，写该时间进行输出)*/

    time_t time_now;            /**上一次调整铷钟的时间  */
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

