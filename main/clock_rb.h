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
    /** 本地时钟 */
    Uint8 alarmRb10M;      /**铷钟10MHz检波告警  */
    Uint8 alarmVcxo25M;     /**VCXO 25MHz检波告警 */
    Uint8 alarmVcxo24_576M; /**VCXO24.576MHz检波告警  */
    Uint8 alarmVcxo25MPoe;  /**VCX0 25MHz（POE）检波告警  */
    Uint8 alarm25MPll;      /**25MHz锁相环失锁告警  */
    Uint8 alarm24_576MPll;  /**24.576MHz锁相环失锁告警  */
    Uint8 alarm25MPoePll;   /*25MHz（POE）锁相环失锁告警*  */

    /**输入信号 */
    Uint8 alarmBd1pps;      /**BD接收机1PPS检波告  */
    Uint8 alarmBdTod;       /**BD接收机串口报文检波告警  */
    Uint8 alarmRef10M;      /**参考检波告警  */
    Uint8 ref10Mor2M;       /**1:10M    2,2M   3,告警*/
    Uint8 alarmInputPpsTod; /**1PPS（+TOD）检波 */
    Uint8 alarmInputTod;         /**TOD检波  */
    Uint8 alarmInputDcls;   /**DCLS检波  */
    Uint8 alarmInput26503_2M;/**DS26503输出2MHz检波  */
    Uint8 alarmBattery;
    Uint8 alarmSatellite;  /**卫星信号出问题  1告警，0 不告警*/
};

struct collect_data
{

   char uPdOverFlow;  /*连续N次溢出 调相0.4S*/
   char Over_100us;   /*连续N次大于100us,提前结果本次采集*/
   float  ph_max;             /*最大值*/
   float  ph_min;             /*最小值*/

   int ph_number;          /*每组记数个数  */
   int queue_number;       /*组数          */

   float phase_offset;      /**本次相位偏差  */
   float  add_ph;             /*累计鉴相值*/ 
   int  ph_number_counter;  /*当前鉴相数据总个数*/
   int  queue_number_counter;  /*当前鉴相数据队列个数*/

   float  phase_array[MAX] ; /*鉴相数据收集数组*/

   float  lAvgPhase;       /*平均鉴相数据*/
   float  set_lAvgPhase;   /*预想的平均鉴相数据*/

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
   float     lPhasePrevious; /*上一次平均鉴相数据*/

   char      modify_flag;
   char      modify_cnt;

   char      bInitialClear;  /*记录上一次，是否有调相操作0:无调相  1：有调相*/
   char      adjflag;       /*是否调整标志*/

   float     lDetDdsAdj;     /*计算调节值*/ 
   float     IDetPhase;		 /*偏差值*/
   float     PrelDetPhase;	 /*上一次的偏差值*/
   float     PrelDetDdsAdj;	 //上一次晶体钟调整值
   
   float     lAccPhaseAll;	 /*偏差差累加值*/
   float     OffsetAll;	     /*偏差差累加值*/

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

extern void collect_phase(struct collect_data *p_collect_data,int delay,float ph);

#endif /* RB_H_ */

