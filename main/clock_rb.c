/********************************************************************************************
*                           ��Ȩ����(C) 2015, 
*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       clock_rb.c
*    ��������:       �ӿ�ģ�飬������ӣ�״̬�����ƣ��ռ�����ֵ
*    ����:           Felix
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2017-7-29
*    �����б�:
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
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-7-29
                     ����: Felix
                     �޸�����: �´����ļ�


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
     filter_buf[i] = filter_buf[i + 1];    // �����������ƣ���λ�Ե�
     filter_sum += filter_buf[i];
  } 
   return (int)(filter_sum / 10);
}



long int Calman_FilterMeanPath(long int val,char clean)
{
    
    /**�������˲��㷨��ر��� */
    static float Km;    		 /*kalmanϵ��*/
    static float Rm=0.1;       
    static float Qm=0.0001;    		 /*������������*/
    static float Xk;		/*״̬����*/
    static float Xk_1=0;    /*ǰһ״̬����*/
    static float Pk;        /*״̬������������Э����*/
    static float Pk_1=1;    /*ǰһ״̬������������Э����*/

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
  * @brief  ������
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
    
    send_buf[i++] = '$';/**֡ͷ  */
    send_buf[i++] = 'B';/**֡ͷ  */
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
    
    send_buf[i++] = 0x0d;/**֡β  */
    send_buf[i++] = 0x0a;/**֡β  */

    //DebugPrintf(send_buf,20);
    
    AddData_ToSendList(g_RootData,ENUM_GPS,send_buf,i);    /*���÷��ͺ���*/

}


void SetRbClockWorkMode()
{

    char send_buf[20] = {0};
    short i = 0;
    
    send_buf[i++] = '!';/**֡ͷ  */
    send_buf[i++] = 'M';/**֡ͷ  */
    send_buf[i++] = 'D';/**֡ͷ  */
    send_buf[i++] = 0x0d;/**֡ͷ  */
    send_buf[i++] = 0x0a;/**֡ͷ  */
    
    AddData_ToSendList(g_RootData,ENUM_RB,send_buf,i);    /*���÷��ͺ���*/

}



void RequestRbClockStatus()
{
    char send_buf[20] = {0};
    short i = 0;
    
    send_buf[i++] = '!';/**֡ͷ  */
    send_buf[i++] = 0x5e;/**֡ͷ  */
    send_buf[i++] = 0x0d;/**֡ͷ  */
    send_buf[i++] = 0x0a;/**֡ͷ  */

    AddData_ToSendList(g_RootData,ENUM_RB,send_buf,i);   /*���÷��ͺ���*/

}


void send_absolute_clock_data(int data)
{
    short i = 0;
    int center=0;
    char send_buf[20] = {0};
    char cData[10] = {0};


    center= data;
    send_buf[i++] = '!';/**֡ͷ  */
    send_buf[i++] = 'F';/**֡ͷ  */
    send_buf[i++] = 'A';/**֡ͷ  */

    sprintf(cData,"%d",center);

    memcpy(send_buf+i,cData,strlen(cData));
    i += strlen(cData);

    send_buf[i++] = 0x0d;/**֡ͷ  */
    send_buf[i++] = 0x0a;/**֡ͷ  */
        
    AddData_ToSendList(g_RootData,ENUM_RB,send_buf,i);    /*���÷��ͺ���*/
  
}



  
  /**
  * @brief  ͨ�����ڷ������ݵ����
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
   send_buf[i++] = '!';/**֡ͷ  */
   send_buf[i++] = 'F';/**֡ͷ  */
   send_buf[i++] = 'A';/**֡ͷ  */

   sprintf(cData,"%d",p_clock_info->center);
   
   memcpy(send_buf+i,cData,strlen(cData));
   i += strlen(cData);

   send_buf[i++] = 0x0d;/**֡ͷ  */
   send_buf[i++] = 0x0a;/**֡ͷ  */

    p_clock_info->time_now =time(NULL);
    
   /*���÷��ͺ���*/
   AddData_ToSendList(g_RootData,ENUM_RB,send_buf,i);

}


  
  /**
  * @brief  GPS�������ݳ�ʼ��
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
  * @brief  PH�����ռ�
  * @param  struct clock_info *   ��ʱ��(E1�ռ�ʱ��Ϊ0)������ֵ(10ns)��
  * @retval None
  */
void collect_phase(struct collect_data *p_collect_data,int delay,float ph)
{
    float ph_temp=0;
    int temp=0;
    float sum=0;
    /*��ʱ����*/
    
    ph_temp=ph;
    p_collect_data->phase_offset = ph;
    //printf("collect_phase=%f, count=%d\n",ph,p_collect_data->ph_number_counter);
    p_collect_data->add_ph+=ph_temp;
   
    if(p_collect_data->ph_number_counter==0)  /*������ֵ*/
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

    if(p_collect_data->ph_number_counter>=p_collect_data->ph_number-1)   /*������һ��*/
    { 
      p_collect_data->add_ph=p_collect_data->add_ph-p_collect_data->ph_max-p_collect_data->ph_min;  /*�������С*/
      p_collect_data->phase_array[p_collect_data->queue_number_counter]= p_collect_data->add_ph;
	  p_collect_data->queue_number_counter++;
      
	  printf("p_collect_data->add_ph=%f   \r\n",p_collect_data->add_ph);
      p_collect_data->add_ph=0;
	  p_collect_data->ph_number_counter=0;   /*������0*/
      
      if(p_collect_data->queue_number_counter>= p_collect_data->queue_number)  /*���н���һ������*/
      {
         for(temp=0;temp<p_collect_data->queue_number_counter;temp++)
		 {
             sum+=p_collect_data->phase_array[temp];
	     }
         p_collect_data->lAvgPhase=sum/(p_collect_data->ph_number-2);      /*����ƽ����λֵ*/
         p_collect_data->queue_number_counter=0;/*������0*/
		 p_collect_data->getdata_flag=1;
		 printf("ph_number=%d  queue=%d  sum = %f  type= %d  ph=%f \r\n",p_collect_data->ph_number,p_collect_data->queue_number,sum,delay,ph);

      } 
    }

}
  /**
  * @brief  1Hz fast ����
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


    /**��λ����  */
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
            
            /**����10s�ۼ�ƫ��  */
            everySecOffset = p_clock_info->IDetPhase / 100;
            
            /**iֵΪ0.01 ����  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /100);

            
            printf("everySecOffset1 = %f\r\n",everySecOffset);

            /**�õ�Ƶƫ�����Ա���  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;

            if(p_clock_info->lockCounter<12)
                p_clock_info->lockCounter++;       
        }
        else
        {

            /** 0.25 */
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 4;

            /**iֵΪ0.5 ����  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /2);

            
            everySecOffset = p_clock_info->IDetPhase / 100;
            
            printf("everySecOffset2 = %f\r\n",everySecOffset);

            p_clock_info->lDetDdsAdj = everySecOffset * 1000;
            
            if(p_clock_info->lockCounter>1)
                p_clock_info->lockCounter--; 

        }

        
        p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
        /** ��cpt С��ӵĵ�����ʽ�ǣ��������ң���ֵ���� */
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
   

    /**��λ����  */
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
            /**��С���  0.1*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 10;

            /**iֵΪ0.01 ����  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll / 500);

            /**�õ�Ƶƫppm  */
            everySecOffset = p_clock_info->IDetPhase / 500;
            
            printf("everySecOffset4 = %f\r\n",everySecOffset);

            /**����ϵ��  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;

            /**�������ֵ  */
            //p_clock_info->lDetDdsAdj = p_clock_info->lDetDdsAdj / 4;
            
            p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
            send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);
            
            p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;



        }
        else
        {
            /**��С���  0.2*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 5;

            /**iֵΪ0.01 ����  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /500);

            /**�õ�Ƶƫppm  */
            everySecOffset = p_clock_info->IDetPhase / 500;
            
            printf("everySecOffset4 = %f\r\n",everySecOffset);

            /**����ϵ��  */
            p_clock_info->lDetDdsAdj = everySecOffset * 10000;

            /**�������ֵ  */
            //p_clock_info->lDetDdsAdj = p_clock_info->lDetDdsAdj / 2;
                        
            p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
            send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);
            
            p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

            /**���������״̬�£���λƫ��ÿ�β���100ns�������һ����λ  */
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


    /**��λ����  */
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
            
            /**��С����  0.25*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 4;

            /**����100s�ۼ�ƫ��  */
            everySecOffset = p_clock_info->IDetPhase / 100;
            
            printf("everySecOffset1 = %f\r\n",everySecOffset);

            /**�õ�Ƶƫ�����Ա���  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;

            if(p_clock_info->lockCounter<12)
                p_clock_info->lockCounter++;       
        }
        else
        {

            /**��С����  0.5*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase;

            p_clock_info->IDetPhase += (p_clock_info->OffsetAll / 5);

             /**����100s�ۼ�ƫ��  */
            everySecOffset = p_clock_info->IDetPhase / 100;
            printf("everySecOffset2 = %f\r\n",everySecOffset);

            p_clock_info->lDetDdsAdj = everySecOffset * 1000;
            
            if(p_clock_info->lockCounter>1)
                p_clock_info->lockCounter--; 

        }

        
        p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
        /** ��cpt С��ӵĵ�����ʽ�ǣ��������ң���ֵ���� */
        send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);

        
        p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

    }
  
}

  /**
  * @brief  1Hz lock ����
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


    /**��λ����  */
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
            /**��С���  0.1 */
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 10;

            /**iֵΪ0.1 ����  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /8);

            /**�õ�Ƶƫppm  */
            everySecOffset = p_clock_info->IDetPhase / 200;
            
            printf("everySecOffset4 = %f\r\n",everySecOffset);

            /**����ϵ��  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;
            
            p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
            send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);
            
            p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

            /**���������״̬�£���λƫ��ÿ�β���100ns�������һ����λ  */
            if(abs(p_clock_info->data_1Hz.lAvgPhase)>2000)
            {
                p_clock_info->syn_enable = TRUE;
            }

        }
        else
        {
            /**��С��� 0.125*/
            p_clock_info->IDetPhase = p_clock_info->IDetPhase / 8;

            /**iֵΪ0.01 ����  */
            p_clock_info->IDetPhase += (p_clock_info->OffsetAll /10);

            /**�õ�Ƶƫppm  */
            everySecOffset = p_clock_info->IDetPhase / 200;
            
            printf("everySecOffset4 = %f\r\n",everySecOffset);

            /**����ϵ��  */
            p_clock_info->lDetDdsAdj = everySecOffset * 1000;
            
            p_clock_info->lDetDdsAdj = -p_clock_info->lDetDdsAdj;
            send_clock_data(p_clock_info,(int)p_clock_info->lDetDdsAdj);
            
            p_clock_info->lPhasePrevious = p_clock_info->data_1Hz.lAvgPhase;

            /**���������״̬�£���λƫ��ÿ�β���100ns�������һ����λ  */
            if(abs(p_clock_info->data_1Hz.lAvgPhase)>2000)
            {
                p_clock_info->syn_enable = TRUE;
            }

        }
    }
}


  /**
  * @brief  1Hz״̬������ (ÿ�����һ��,���òο�ԴΪ1PPSʱ������)
  * @param  None
  * @retval None
  */

void gps_status_machine(struct clock_info *p_clock_info,char ref_status)
{   
    static char status=0;

    
    if(p_clock_info->ref_change_flag == TRUE)  /*ͬ��ο�Դ�仯������ǰ����*/
    {
      //p_clock_info->bInitialClear=1; /*���*/
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
             if(ref_status==1)   /*�ο�Դ����*/
             { /*add */ 
               p_clock_info->workStatus=FAST;  /**/
             }	

        }
      	    break;
     	case FAST:
        {
            if(ref_status==0) /*�ο�Դ�澯*/
               p_clock_info->workStatus=FREE; 
            else if((ref_status==1)&&(p_clock_info->lockCounter>=3))
			{
               p_clock_info->workStatus=LOCK;
			   p_clock_info->unlockCounter=0;
			 }
            p_clock_info->fast_times++;/*�첶��ʱ*/
        }		
       		break;
     	case LOCK:	
	     {
            if(ref_status==0) /*�ο�Դ�澯*/
               p_clock_info->workStatus= HOLD;
            else if ((ref_status==1)&&(p_clock_info->unlockCounter>=3))
			{
               p_clock_info->workStatus= FAST;
			   p_clock_info->lockCounter=0;
			}
            p_clock_info->lock_times++;/*������ʱ*/
         }
      		break;
     	case HOLD:	
        {
            p_clock_info->hold_times++;/*������ʱ*/
           if(p_clock_info->hold_times>=86400) /*����ʱ�����1��, �޲ο�Դ ������������*/
           {
                p_clock_info->workStatus= FREE;
           }else if((p_clock_info->hold_times<86400)&&(ref_status==1))
           {
                p_clock_info->workStatus=LOCK;
           }
           /*����ʱ��С��1�ڣ��ο�Դ��Ч����������*/
        }	
      		break;
            
    	default:    
			
     		break;
     }

    if(status!=p_clock_info->workStatus)    /*����״̬�仯*/
    {
		printf("workStatus change  workStatus= %d\r\n",p_clock_info->workStatus);
        switch(p_clock_info->workStatus)
        {
        case FREE:   
            break;

        case FAST:   
          gps_init(p_clock_info);
           /**�첶״̬�£�ÿ10���ռ�һ�飬�ռ�20��  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=10;
          
          p_clock_info->fast_times=0;

         break;
        case LOCK: 
          gps_init(p_clock_info);

          /**����״̬�£�û10���ռ�һ�飬�ռ�50��  */
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
        status=p_clock_info->workStatus;  /*��¼��ǰ����״̬*/
    }

}

  void m10_status_machine(struct clock_info *p_clock_info,char ref_status)
{   
    static char status=0;

    if(p_clock_info->ref_change_flag == TRUE)  /*ͬ��ο�Դ�仯������ǰ����*/
    {
      //p_clock_info->bInitialClear=1; /*���*/
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
             if(ref_status==1)   /*�ο�Դ����*/
             { /*add */ 
               p_clock_info->workStatus=FAST;  /**/
             }	

        }
      	    break;
     	case FAST:
        {
            if(ref_status==0) /*�ο�Դ�澯*/
               p_clock_info->workStatus=FREE; 
            else if((ref_status==1)&&(p_clock_info->lockCounter>=3))
			{
               p_clock_info->workStatus=LOCK;
			   p_clock_info->unlockCounter=0;
			 }
            p_clock_info->fast_times++;/*�첶��ʱ*/
        }		
       		break;
     	case LOCK:	
	     {
            if(ref_status==0) /*�ο�Դ�澯*/
               p_clock_info->workStatus= HOLD;
            else if ((ref_status==1)&&(p_clock_info->unlockCounter>=3))
			{
               p_clock_info->workStatus= FAST;
			   p_clock_info->lockCounter=0;
			}
            p_clock_info->lock_times++;/*������ʱ*/
         }
      		break;
     	case HOLD:	
        {
            p_clock_info->hold_times++;/*������ʱ*/
           if(p_clock_info->hold_times>=86400) /*����ʱ�����1��, �޲ο�Դ ������������*/
           {
                p_clock_info->workStatus= FREE;
           }else if((p_clock_info->hold_times<86400)&&(ref_status==1))
           {
                p_clock_info->workStatus=LOCK;
           }
           /*����ʱ��С��1�ڣ��ο�Դ��Ч����������*/
        }	
      		break;
            
    	default:    
     		break;
     }

    if(status!=p_clock_info->workStatus)    /*����״̬�仯*/
    {
		printf("workStatus change  workStatus= %d\r\n",p_clock_info->workStatus);
        switch(p_clock_info->workStatus)
        {
        case FREE:   
            break;

        case FAST:   
          m10ref_init(p_clock_info);
           /**�첶״̬�£�ÿ10���ռ�һ�飬�ռ�20��  */
          p_clock_info->data_1Hz.ph_number=12;
          p_clock_info->data_1Hz.queue_number=10;
          
          p_clock_info->fast_times=0;

         break;
        case LOCK: 
          m10ref_init(p_clock_info);

          /**����״̬�£�û10���ռ�һ�飬�ռ�50��  */
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
        status=p_clock_info->workStatus;  /*��¼��ǰ����״̬*/
    }

}
  /**
  * @brief  �յ�һ�����ݺ���ò�ͬ�Ĵ������� ������Ϊ�ο�ԴΪ1PPSʱ���á�
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
    /** ϵͳ�ȶ�15s����ӿ��� */
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
    /**�ο�Դ1pps��1pps��10m������澯���ȴ�4���ӣ��ȴ�С����ȶ�  */
    /*ѡ��ο�Դ 1PPS���벻�澯*/
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
    
        /**�ռ���һ������ */
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
