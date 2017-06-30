/********************************************************************************************
*                           版权所有(C) 2015, 
*                                 版权所有
*********************************************************************************************
*    文 件 名:       clock_rb.c
*    功能描述:       钟控模块，控制铷钟，状态机控制，收集鉴相值
*    作者:           Felix
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2017-7-29
*    函数列表:
                     Calman_FilterMeanPath
                     collect_ph
                     e1_det_para
                     Filter
                     gps_init
                     gps_status_handle
                     gps_status_machine
                     rb_fast_handle
                     rb_lock_handle
                     RequestRbClockStatus
                     send_clock_data
                     SetRbClockWorkMode
                     xor
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-7-29
                     作者: Felix
                     修改内容: 新创建文件


*********************************************************************************************/

#include "clock_rb.h"
#include "main.h"



#define RUN_TIME 300

int Filter(int val) 
{
  int i;
  static int filter_buf[10 + 1];
  int filter_sum = 0;
   filter_buf[10] = val;
  for(i = 0; i < 10; i++)
   {
     filter_buf[i] = filter_buf[i + 1];    // 所有数据左移，低位仍掉
     filter_sum += filter_buf[i];
  } 
   return (int)(filter_sum / 10);
}



long int Calman_FilterMeanPath(long int val,char clean)
{
    
    /**卡尔曼滤波算法相关变量 */
    static float Km;    		 /*kalman系数*/
    static float Rm=0.1;       
    static float Qm=0.0001;    		 /*过程噪声方差*/
    static float Xk;		/*状态变量*/
    static float Xk_1=0;    /*前一状态变量*/
    static float Pk;        /*状态变量估计误差的协方差*/
    static float Pk_1=1;    /*前一状态变量估计误差的协方差*/

	Xk=Xk_1;
	Pk=Pk_1+Qm;

	Km=Pk/(Pk+Rm);
		
	Xk_1=Xk+Km*(val-Xk);
		
	Pk_1=(1-Km)*Pk;

	val=Xk_1;
     if(clean==0)
	{
		Km=0;
		Rm=0.1;
		Qm=0.0001;
		Xk=0;
		Xk_1=0;
		Pk_1=1;
		Pk=0;
	}
	return val;
}


  /**
  * @brief  异或计算
  * @param  None
  * @retval None
  */

char xor(char *string,char n)
{   
    char i=0,result=0;
    for(i=0;i<n;i++)
      result^=*(string+i);
     return result; 

}


void SetVcomWorkMode(char *cmd)
{
    char send_buf[20] = {0};
    short i = 0;
    char hh;
    char h1[2];
    
    send_buf[i++] = '$';/**帧头  */
    send_buf[i++] = 'B';/**帧头  */
    send_buf[i++] = 'D';
    send_buf[i++] = 'P';
    send_buf[i++] = 'M';
    send_buf[i++] = 'O';
    send_buf[i++] = ',';

    memcpy((send_buf+i),cmd,3);
    i += 3;
    send_buf[i++] = '*';
    
    hh = xor(send_buf+1,i-2);
    sprintf(h1,"%x",hh);
    send_buf[i++] = h1[0];
    send_buf[i++] = h1[1];
    
    send_buf[i++] = 0x0d;/**帧尾  */
    send_buf[i++] = 0x0a;/**帧尾  */

    //DebugPrintf(send_buf,20);
    
    AddData_ToSendList(g_RootData,ENUM_GPS,send_buf,i);    /*调用发送函数*/

}


void SetRbClockWorkMode()
{

    char send_buf[20] = {0};
    short i = 0;
    
    send_buf[i++] = '!';/**帧头  */
    send_buf[i++] = 'M';/**帧头  */
    send_buf[i++] = 'D';/**帧头  */
    send_buf[i++] = 0x0d;/**帧头  */
    send_buf[i++] = 0x0a;/**帧头  */
    
    AddData_ToSendList(g_RootData,ENUM_RB,send_buf,i);    /*调用发送函数*/

}



void RequestRbClockStatus()
{
    char send_buf[20] = {0};
    short i = 0;
    
    send_buf[i++] = '!';/**帧头  */
    send_buf[i++] = 0x5e;/**帧头  */
    send_buf[i++] = 0x0d;/**帧头  */
    send_buf[i++] = 0x0a;/**帧头  */

    AddData_ToSendList(g_RootData,ENUM_RB,send_buf,i);   /*调用发送函数*/

}


void send_absolute_clock_data(int data)
{
    short i = 0;
    int center=0;
    char send_buf[20] = {0};
    char cData[10] = {0};


    center= data;
    send_buf[i++] = '!';/**帧头  */
    send_buf[i++] = 'F';/**帧头  */
    send_buf[i++] = 'A';/**帧头  */

    sprintf(cData,"%d",center);

    memcpy(send_buf+i,cData,strlen(cData));
    i += strlen(cData);

    send_buf[i++] = 0x0d;/**帧头  */
    send_buf[i++] = 0x0a;/**帧头  */
        
    AddData_ToSendList(g_RootData,ENUM_RB,send_buf,i);    /*调用发送函数*/
  
}



  
  /**
  * @brief  通过串口发送数据到铷钟
  * @param  None
  * @retval None
  */
void send_clock_data(struct clock_info *p_clock_info,int data)
{
   short i = 0;
   char send_buf[20] = {0};
   char cData[10] = {0};

#if 0
   data = data / 1000;
   data  = data * 1000;
#endif
   
   printf("send_clock_data =%d center=%d\n",data,p_clock_info->center);
   p_clock_info->center = p_clock_info->center + data;
   printf("send_clock_data center=%d\n",p_clock_info->center);
   send_buf[i++] = '!';/**帧头  */
   send_buf[i++] = 'F';/**帧头  */
   send_buf[i++] = 'A';/**帧头  */

   sprintf(cData,"%d",p_clock_info->center);
   
   memcpy(send_buf+i,cData,strlen(cData));
   i += strlen(cData);

   send_buf[i++] = 0x0d;/**帧头  */
   send_buf[i++] = 0x0a;/**帧头  */

    p_clock_info->time_now =time(NULL);
    
   /*调用发送函数*/
   AddData_ToSendList(g_RootData,ENUM_RB,send_buf,i);

}


  
  /**
  * @brief  GPS队列数据初始化
  * @param  None
  * @retval None
  */
void gps_init(struct clock_info *p_clock_info)
{
   struct collect_data *p_collect_data=&p_clock_info->data_1Hz;   
   p_clock_info->bInitialClear=1;
   p_clock_info->syn_enable = 1;
   p_collect_data->ph_max=0;
   p_collect_data->ph_min=0;
   p_collect_data->ph_number=12;
   p_collect_data->queue_number=20;
   p_collect_data->add_ph=0;
   p_collect_data->ph_number_counter=0;
   p_collect_data->queue_number_counter=0;
   p_collect_data->getdata_flag=0;
   p_clock_info->lAccPhaseAll=0;

}

void m10ref_init(struct clock_info *p_clock_info)
{
   struct collect_data *p_collect_data=&p_clock_info->data_1Hz;   
   p_clock_info->bInitialClear=1;
   p_collect_data->ph_max=0;
   p_collect_data->ph_min=0;
   p_collect_data->ph_number=12;
   p_collect_data->queue_number=20;
   p_collect_data->add_ph=0;
   p_collect_data->ph_number_counter=0;
   p_collect_data->queue_number_counter=0;
   p_collect_data->getdata_flag=0;
   p_clock_info->lAccPhaseAll=0;

}

    /**
  * @brief  PH数据收集
  * @param  struct clock_info *   、时延(E1收集时延为0)、鉴相值(10ns)、
  * @retval None
  */
void collect_phase(struct collect_data *p_collect_data,int delay,float ph)
{
    float ph_temp=0;
    int temp=0;
    float sum=0;
    /*延时补偿*/
    
    ph_temp=ph;
    p_collect_data->phase_offset = ph;
    //printf("collect_phase=%f, count=%d\n",ph,p_collect_data->ph_number_counter);
    p_collect_data->add_ph+=ph_temp;
   
    if(p_collect_data->ph_number_counter==0)  /*计算最值*/
    {
        p_collect_data->ph_max=ph_temp;
        p_collect_data->ph_min=ph_temp;
    } 
    else if(p_collect_data->ph_max<ph_temp)
    {
        p_collect_data->ph_max=ph_temp;
    }
    else if(p_collect_data->ph_min>ph_temp)
    {
        p_collect_data->ph_min=ph_temp;
    }
    p_collect_data->ph_number_counter++;

    if(p_collect_data->ph_number_counter>=p_collect_data->ph_number-1)   /*记数满一组*/
    { 
      p_collect_data->add_ph=p_collect_data->add_ph-p_collect_data->ph_max-p_collect_data->ph_min;  /*减最大最小*/
      p_collect_data->phase_array[p_collect_data->queue_number_counter]= p_collect_data->add_ph;
	  p_collect_data->queue_number_counter++;
      
	  printf("p_collect_data->add_ph=%f   \r\n",p_collect_data->add_ph);
      p_collect_data->add_ph=0;
	  p_collect_data->ph_number_counter=0;   /*记数清0*/
      
      if(p_collect_data->queue_number_counter>= p_collect_data->queue_number)  /*队列接收一组数据*/
      {
         for(temp=0;temp<p_collect_data->queue_number_counter;temp++)
		 {
             sum+=p_collect_data->phase_array[temp];
	     }
         p_collect_data->lAvgPhase=sum/(p_collect_data->ph_number-2);      /*计算平均相位值*/
         p_collect_data->queue_number_counter=0;/*记数清0*/
		 p_collect_data->getdata_flag=1;
		 printf("ph_number=%d  queue=%d  sum = %f  type= %d  ph=%f \r\n",p_collect_data->ph_number,p_collect_data->queue_number,sum,delay,ph);

      } 
    }

}
  /**
  * @brief  1Hz fast 处理
  * @param  None
  * @retval None
  */
void m10_fast_handle(struct clock_info *p_clock_info)
{   
    float everySecOffset;
        
    p_clock_info->IDetPhase= p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious;
    p_clock_info->OffsetAll += p_clock_info->IDetPhase;
    
	printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f,OffsetAll =%f,bInitiatlClear = %d \r\n"
        ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious
        ,p_clock_info->IDetPhase,p_clock_info->OffsetAll,p_clock_info->bInitialClear);


    /**相位调整  */
    if(p_clock_info->bInitialClear ==  1)
    {
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;
        p_clock_info->lDetDdsAdj=0;
        p_clock_info->bInitialClear = 0;
        p_clock_info->OffsetAll = 0;
        p_clock_info->lAccPhaseAll = 0;
        p_clock_info->IDetPhase=0;
    }
    else
    {
        
        if(p_clock_info->IDetPhase > -10.0 && p_clock_info->IDetPhase < 10.0)
        {
            if(p_clock_info->lockCounter<12)
                p_clock_info->lockCounter += 2;   
            
            p_clock_info->lDetDdsAdj = 0;
        }
        else if((p_clock_info->IDetPhase > -100.0 && p_clock_info->IDetPhase < -10.0)
            || (p_clock_info->IDetPhase > 10.0 && p_clock_info->IDetPhase < 10.0))
        {

            /** 0.125 */
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 8;
            
            /**计算10s累计偏差  */
            everySecOffset = p_clock_info->IDetPhase / 100;
            
            /**i值为0.01 比例  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /100);

            
            printf("everySecOffset1 = %f\r\n",everySecOffset);

            /**得到频偏，乘以比例  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;

            if(p_clock_info->lockCounter<12)
                p_clock_info->lockCounter++;       
        }
        else
        {

            /** 0.25 */
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 4;

            /**i值为0.5 比例  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /2);

            
            everySecOffset = p_clock_info->IDetPhase / 100;
            
            printf("everySecOffset2 = %f\r\n",everySecOffset);

            p_clock_info->lDetDdsAdj = everySecOffset * 1000;
            
            if(p_clock_info->lockCounter>1)
                p_clock_info->lockCounter--; 

        }

        
        p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
        /** 该cpt 小铷钟的调整方式是，整治往右，负值往左 */
        send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);

        
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

    }
  
}


void m10_lock_handle(struct clock_info *p_clock_info)
{   

    float everySecOffset;

    p_clock_info->IDetPhase = (p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious);
    p_clock_info->OffsetAll += p_clock_info->IDetPhase;
    
	printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f,OffsetAll =%f,bInitiatlClear = %d \r\n"
        ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious
        ,p_clock_info->IDetPhase,p_clock_info->OffsetAll,p_clock_info->bInitialClear);
   

    /**相位调整  */
    if(p_clock_info->bInitialClear ==  1)
    {
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;
        p_clock_info->lDetDdsAdj=0;
        p_clock_info->bInitialClear = 0;
        p_clock_info->lAccPhaseAll=0;
	    p_clock_info->IDetPhase=0;
        p_clock_info->OffsetAll = 0;
        
    }
    else
    {
        
         if((p_clock_info->IDetPhase<-100)&&(p_clock_info->IDetPhase>100))
         {
            if(p_clock_info->unlockCounter<50)
                p_clock_info->unlockCounter++;
         }
         else
         {
            if(p_clock_info->unlockCounter>1)
                p_clock_info->unlockCounter--;
         }

         
        if((p_clock_info->IDetPhase>-10)&&(p_clock_info->IDetPhase<10))
        {
            p_clock_info->lDetDdsAdj = 0;
            
            if(p_clock_info->unlockCounter>1)
                p_clock_info->unlockCounter--;
        }
        else if((p_clock_info->IDetPhase<-100)&&(p_clock_info->IDetPhase>100))
        {
            /**减小误差  0.1*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 10;

            /**i值为0.01 比例  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll / 500);

            /**得到频偏ppm  */
            everySecOffset = p_clock_info->IDetPhase / 500;
            
            printf("everySecOffset4 = %f\r\n",everySecOffset);

            /**乘以系数  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;

            /**减半调节值  */
            //p_clock_info->lDetDdsAdj = p_clock_info->lDetDdsAdj / 4;
            
            p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
            send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);
            
            p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;



        }
        else
        {
            /**减小误差  0.2*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 5;

            /**i值为0.01 比例  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /500);

            /**得到频偏ppm  */
            everySecOffset = p_clock_info->IDetPhase / 500;
            
            printf("everySecOffset4 = %f\r\n",everySecOffset);

            /**乘以系数  */
            p_clock_info->lDetDdsAdj = everySecOffset * 10000;

            /**减半调节值  */
            //p_clock_info->lDetDdsAdj = p_clock_info->lDetDdsAdj / 2;
                        
            p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
            send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);
            
            p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

            /**如果在锁定状态下，相位偏差每次操作100ns，则调整一次相位  */
            if(abs(p_clock_info->data_1Hz.lAvgPhase)>20000)
            {
                p_clock_info->syn_enable = TRUE;
            }

        }
    }

}



void rb_fast_handle(struct clock_info *p_clock_info)
{   
    float everySecOffset;
        
    p_clock_info->IDetPhase= p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious;
    p_clock_info->OffsetAll += p_clock_info->IDetPhase;
    
	printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f,OffsetAll =%f,bInitiatlClear = %d \r\n"
        ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious
        ,p_clock_info->IDetPhase,p_clock_info->OffsetAll,p_clock_info->bInitialClear);


    /**相位调整  */
    if(p_clock_info->bInitialClear ==  1)
    {
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;
        p_clock_info->lDetDdsAdj=0;
        p_clock_info->bInitialClear = 0;
        p_clock_info->OffsetAll = 0;
        p_clock_info->lAccPhaseAll = 0;
	    p_clock_info->IDetPhase=0;
    }
    else
    {
        
        if(p_clock_info->IDetPhase > -10 && p_clock_info->IDetPhase < 10)
        {
            if(p_clock_info->lockCounter<12)
                p_clock_info->lockCounter += 2;   
            
            p_clock_info->lDetDdsAdj = 0;
        }
        else if((p_clock_info->IDetPhase > -50 && p_clock_info->IDetPhase < -10)
            || (p_clock_info->IDetPhase > 10 && p_clock_info->IDetPhase < 50))
        {
            
            /**减小幅度  0.25*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 4;

            /**计算100s累计偏差  */
            everySecOffset = p_clock_info->IDetPhase / 100;
            
            printf("everySecOffset1 = %f\r\n",everySecOffset);

            /**得到频偏，乘以比例  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;

            if(p_clock_info->lockCounter<12)
                p_clock_info->lockCounter++;       
        }
        else
        {

            /**减小幅度  0.5*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase;

            p_clock_info->IDetPhase += (p_clock_info->OffsetAll / 5);

             /**计算100s累计偏差  */
            everySecOffset = p_clock_info->IDetPhase / 100;
            printf("everySecOffset2 = %f\r\n",everySecOffset);

            p_clock_info->lDetDdsAdj = everySecOffset * 1000;
            
            if(p_clock_info->lockCounter>1)
                p_clock_info->lockCounter--; 

        }

        
        p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
        /** 该cpt 小铷钟的调整方式是，整治往右，负值往左 */
        send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);

        
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

    }
  
}

  /**
  * @brief  1Hz lock 处理
  * @param  None
  * @retval None
  */

void rb_lock_handle(struct clock_info *p_clock_info)
{   

    float everySecOffset;

    p_clock_info->IDetPhase = (p_clock_info->data_1Hz.lAvgPhase-p_clock_info->lPhasePrevious);
    p_clock_info->OffsetAll += p_clock_info->IDetPhase;
    
	printf("lAvgPhase= %f  lPhasePrevious= %f   IDetPhase= %f,OffsetAll =%f,bInitiatlClear = %d \r\n"
        ,p_clock_info->data_1Hz.lAvgPhase,p_clock_info->lPhasePrevious
        ,p_clock_info->IDetPhase,p_clock_info->OffsetAll,p_clock_info->bInitialClear);


    /**相位调整  */
    if(p_clock_info->bInitialClear ==  1)
    {
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;
        p_clock_info->lDetDdsAdj=0;
        p_clock_info->bInitialClear = 0;
        p_clock_info->lAccPhaseAll=0;
	    p_clock_info->IDetPhase=0;
        p_clock_info->OffsetAll = 0;
        
    }
    else
    {
        
         if((p_clock_info->IDetPhase<-100)&&(p_clock_info->IDetPhase>100))
         {
            if(p_clock_info->unlockCounter<50)
                p_clock_info->unlockCounter++;
         }
         else
         {
            if(p_clock_info->unlockCounter>1)
                p_clock_info->unlockCounter--;
         }

        if((p_clock_info->IDetPhase>-10)&&(p_clock_info->IDetPhase<10))
        {
            p_clock_info->lDetDdsAdj = 0;
            
            if(p_clock_info->unlockCounter>1)
                p_clock_info->unlockCounter--;
        }
        else if((p_clock_info->IDetPhase<-100)&&(p_clock_info->IDetPhase>100))
        {
            /**减小误差  0.1 */
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 10;

            /**i值为0.1 比例  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /8);

            /**得到频偏ppm  */
            everySecOffset = p_clock_info->IDetPhase / 200;
            
            printf("everySecOffset4 = %f\r\n",everySecOffset);

            /**乘以系数  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;
            
            p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
            send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);
            
            p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

            /**如果在锁定状态下，相位偏差每次操作100ns，则调整一次相位  */
            if(abs(p_clock_info->data_1Hz.lAvgPhase)>2000)
            {
                p_clock_info->syn_enable = TRUE;
            }

        }
        else
        {
            /**减小误差 0.125*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 8;

            /**i值为0.01 比例  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /10);

            /**得到频偏ppm  */
            everySecOffset = p_clock_info->IDetPhase / 200;
            
            printf("everySecOffset4 = %f\r\n",everySecOffset);

            /**乘以系数  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;
            
            p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
            send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);
            
            p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

            /**如果在锁定状态下，相位偏差每次操作100ns，则调整一次相位  */
            if(abs(p_clock_info->data_1Hz.lAvgPhase)>2000)
            {
                p_clock_info->syn_enable = TRUE;
            }

        }
    }
}


  /**
  * @brief  1Hz状态机处理 (每秒调用一次,配置参考源为1PPS时，调用)
  * @param  None
  * @retval None
  */

void gps_status_machine(struct clock_info *p_clock_info,char ref_status)
{   
    static char status=0;

    
    if(p_clock_info->ref_change_flag == TRUE)  /*同类参考源变化丢弃当前数据*/
    {
      //p_clock_info->bInitialClear=1; /*标记*/
      p_clock_info->syn_enable = 1;
      p_clock_info->data_1Hz.add_ph=0;
      p_clock_info->data_1Hz.ph_number_counter=0;
      p_clock_info->data_1Hz.queue_number_counter=0;
      p_clock_info->ref_change_flag = FALSE;
    }


	//printf("workStatus= %d \r\n",p_clock_info->workStatus);
    switch(p_clock_info->workStatus)
    {
		case FREE:
        {
             if(ref_status==1)   /*参考源正常*/
             { /*add */ 
               p_clock_info->workStatus=FAST;  /**/
             }	

        }
      	    break;
     	case FAST:
        {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus=FREE; 
            else if((ref_status==1)&&(p_clock_info->lockCounter>=3))
			{
               p_clock_info->workStatus=LOCK;
			   p_clock_info->unlockCounter=0;
			 }
            p_clock_info->fast_times++;/*快捕用时*/
        }		
       		break;
     	case LOCK:	
	     {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus= HOLD;
            else if ((ref_status==1)&&(p_clock_info->unlockCounter>=3))
			{
               p_clock_info->workStatus= FAST;
			   p_clock_info->lockCounter=0;
			}
            p_clock_info->lock_times++;/*锁定用时*/
         }
      		break;
     	case HOLD:	
        {
            p_clock_info->hold_times++;/*锁定用时*/
           if(p_clock_info->hold_times>=86400) /*保持时间大于1天, 无参考源 进入自由运行*/
           {
                p_clock_info->workStatus= FREE;
           }else if((p_clock_info->hold_times<86400)&&(ref_status==1))
           {
                p_clock_info->workStatus=LOCK;
           }
           /*保持时间小于1于，参考源有效，进入锁定*/
        }	
      		break;
            
    	default:    
			
     		break;
     }

    if(status!=p_clock_info->workStatus)    /*工作状态变化*/
    {
		printf("workStatus change  workStatus= %d\r\n",p_clock_info->workStatus);
        switch(p_clock_info->workStatus)
        {
        case FREE:   
            break;

        case FAST:   
          gps_init(p_clock_info);
           /**快捕状态下，每10个收集一组，收集20组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=10;
          
          p_clock_info->fast_times=0;

         break;
        case LOCK: 
          gps_init(p_clock_info);

          /**锁定状态下，没10个收集一组，收集50组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=20;	 //50  20h
          p_clock_info->lock_times=0;


         break;
        case HOLD:   
            p_clock_info->hold_times=0;
            break;
        default :
            break;
        
        }
        status=p_clock_info->workStatus;  /*记录当前工作状态*/
    }

}

  void m10_status_machine(struct clock_info *p_clock_info,char ref_status)
{   
    static char status=0;

    if(p_clock_info->ref_change_flag == TRUE)  /*同类参考源变化丢弃当前数据*/
    {
      //p_clock_info->bInitialClear=1; /*标记*/
      p_clock_info->syn_enable = 1;
      p_clock_info->data_1Hz.add_ph=0;
      p_clock_info->data_1Hz.ph_number_counter=0;
      p_clock_info->data_1Hz.queue_number_counter=0;
      p_clock_info->ref_change_flag = FALSE;
    }


	//printf("workStatus= %d \r\n",p_clock_info->workStatus);
    switch(p_clock_info->workStatus)
    {
		case FREE:
        {
             if(ref_status==1)   /*参考源正常*/
             { /*add */ 
               p_clock_info->workStatus=FAST;  /**/
             }	

        }
      	    break;
     	case FAST:
        {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus=FREE; 
            else if((ref_status==1)&&(p_clock_info->lockCounter>=3))
			{
               p_clock_info->workStatus=LOCK;
			   p_clock_info->unlockCounter=0;
			 }
            p_clock_info->fast_times++;/*快捕用时*/
        }		
       		break;
     	case LOCK:	
	     {
            if(ref_status==0) /*参考源告警*/
               p_clock_info->workStatus= HOLD;
            else if ((ref_status==1)&&(p_clock_info->unlockCounter>=3))
			{
               p_clock_info->workStatus= FAST;
			   p_clock_info->lockCounter=0;
			}
            p_clock_info->lock_times++;/*锁定用时*/
         }
      		break;
     	case HOLD:	
        {
            p_clock_info->hold_times++;/*锁定用时*/
           if(p_clock_info->hold_times>=86400) /*保持时间大于1天, 无参考源 进入自由运行*/
           {
                p_clock_info->workStatus= FREE;
           }else if((p_clock_info->hold_times<86400)&&(ref_status==1))
           {
                p_clock_info->workStatus=LOCK;
           }
           /*保持时间小于1于，参考源有效，进入锁定*/
        }	
      		break;
            
    	default:    
     		break;
     }

    if(status!=p_clock_info->workStatus)    /*工作状态变化*/
    {
		printf("workStatus change  workStatus= %d\r\n",p_clock_info->workStatus);
        switch(p_clock_info->workStatus)
        {
        case FREE:   
            break;

        case FAST:   
          m10ref_init(p_clock_info);
           /**快捕状态下，每10个收集一组，收集20组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=10;
          
          p_clock_info->fast_times=0;

         break;
        case LOCK: 
          m10ref_init(p_clock_info);

          /**锁定状态下，没10个收集一组，收集50组  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=50;	 //50  20h
          p_clock_info->lock_times=0;


         break;
        case HOLD:   
            p_clock_info->hold_times=0;
            break;
        default :
            break;
        
        }
        status=p_clock_info->workStatus;  /*记录当前工作状态*/
    }

}
  /**
  * @brief  收到一组数据后调用不同的处理函数 ，配置为参考源为1PPS时调用。
  * @param  None
  * @retval None
  */

void gps_status_handle(struct clock_info *p_clock_info)
{   
  	switch(p_clock_info->workStatus)
    { 
		case FREE:
        	printf("free_handle\r\n");	
      	    break;
     	case FAST:
            printf("fast_handle\r\n");
            rb_fast_handle(p_clock_info);		
       		break;
     	case LOCK:	
            printf("lock_handle\r\n");
            rb_lock_handle(p_clock_info);	
      		break;
     	case HOLD:
            printf("hold_handle\r\n");		
      		break;           
    	default:    
			
     		break;
     }

}


void m10_status_handle(struct clock_info *p_clock_info)
{   
  	switch(p_clock_info->workStatus)
    { 
		case FREE:
        	printf("free_handle\r\n");	
      	    break;
     	case FAST:
            printf("fast_handle\r\n");
            m10_fast_handle(p_clock_info);		
       		break;
     	case LOCK:	
            printf("lock_handle\r\n");
            m10_lock_handle(p_clock_info);	
      		break;
     	case HOLD:
            printf("hold_handle\r\n");		
      		break;           
    	default:    
			
     		break;
     }

}

void ClockStateProcess(struct clock_info *pClockInfo)
{
    struct clock_alarm_data *pAlarmData = &pClockInfo->alarmData;
    Uint8 ref_status;
    static short tmp_ref = 0;

#if 0
    if(pClockInfo->run_times == 5);
        Fpga_WriteByte(0xa3,0x01);
    /** 系统稳定15s后，铷钟开电 */
    if(pClockInfo->run_times == 15)
        SetRbPowerUp();
#endif
    if(pClockInfo->run_times == 20)   
        SetRbClockWorkMode();
    if(pClockInfo->run_times == 25)
        RequestRbClockStatus();
        
#if 0

        SetVcomWorkMode("GGA");
        SetVcomWorkMode("TXT");
        SetVcomWorkMode("ZDA");
        SetVcomWorkMode("GPT");
        SetVcomWorkMode("GSV");
        SetVcomWorkMode("GSA");
        //SetVcomWorkMode("LPS");
        SetVcomWorkMode("LLM");
#endif


    printf("runtime=%d ref_type=%d workStatus= %d\r\n",pClockInfo->run_times,pClockInfo->ref_type,pClockInfo->workStatus);
    /**参考源1pps，1pps和10m无收入告警，等待4分钟，等待小铷钟稳定  */
    /*选择参考源 1PPS输入不告警*/
    if((pClockInfo->ref_type == REF_10MHZ)&&(pAlarmData->alarmRef10M==FALSE)&&(pClockInfo->run_times==RUN_TIME))
    {
         pClockInfo->lockCounter=0;
         send_clock_data(pClockInfo,-1250000);
        
         pClockInfo->data_1Hz.add_ph=0;
         pClockInfo->data_1Hz.ph_number_counter=0;
         pClockInfo->data_1Hz.queue_number_counter=0;
         pClockInfo->data_1Hz.start_flag = 1;
         pClockInfo->syn_enable = TRUE;
         
         printf("--------ref10Mhz _1Hz_set center\r\n");

    }
    else if((pClockInfo->ref_type == REF_SATLITE)&&(pAlarmData->alarmBd1pps==FALSE)&&(pClockInfo->run_times==RUN_TIME))
    {
        
        pClockInfo->lockCounter=0;
        send_clock_data(pClockInfo,-1250000);
        
        pClockInfo->data_1Hz.add_ph=0;
        pClockInfo->data_1Hz.ph_number_counter=0;
        pClockInfo->data_1Hz.queue_number_counter=0;
        pClockInfo->data_1Hz.start_flag = 1;
        pClockInfo->syn_enable = TRUE;
               
        printf("---------ref 1hz 1Hz_set center\r\n");

    }
    if(tmp_ref != pClockInfo->ref_type)
    {
        pClockInfo->ref_change_flag = TRUE;
        tmp_ref = pClockInfo->ref_type;
    }

    if(pClockInfo->ref_type == REF_10MHZ)
    {
        if(pAlarmData->alarmRef10M == FALSE)
            ref_status = 1;   
        else 
            ref_status = 0;
        
        m10_status_machine(pClockInfo,ref_status);
    }
    else if(pClockInfo->ref_type == REF_SATLITE)
    {
        if((pAlarmData->alarmBd1pps == TRUE) || (pAlarmData->alarmSatellite == TRUE))
            ref_status = 0;   
        else 
            ref_status = 1;
        
        gps_status_machine(pClockInfo,ref_status);

    }
    
    pClockInfo->run_times++;
}

void ClockHandleProcess(struct clock_info *pClockInfo)
{
    struct clock_alarm_data *pAlarmData = &pClockInfo->alarmData;
    
    if((pClockInfo->data_1Hz.getdata_flag==1)
        &&(pAlarmData->alarmBd1pps == FALSE)
        &&(pClockInfo->ref_type==REF_SATLITE)&&(pClockInfo->run_times>RUN_TIME)) 
    {
         pClockInfo->data_1Hz.getdata_flag=0;
    
        /**收集够一组数据 */
        gps_status_handle(pClockInfo);
           
    }
    else if((pClockInfo->data_1Hz.getdata_flag==1)
        &&(pAlarmData->alarmRef10M ==FALSE)
        &&(pClockInfo->ref_type==REF_10MHZ)&&(pClockInfo->run_times>RUN_TIME))
    {
        pClockInfo->data_1Hz.getdata_flag=0;
        
        m10_status_handle(pClockInfo);
    }
}

