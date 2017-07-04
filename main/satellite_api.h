#ifndef __SATELLITE_API_H_
#define __SATELLITE_API_H_

#include <time.h>
#include "common.h"

struct Manual_sat_data
{
    char manual_enable;  /**�˹�����ʹ��1������0����*/
    char location_state;  /**1,����λ�ñ��֣�0�˳�λ�ñ��� */
    char relocation;     /**1,�ض�λ 0������*/
    char eastorwest;     /**E,������W������*/
    char northorsouth;   /**N,��γ��S����γ��*/
    Uint32 refSetTime;  /**���òο�Դʱ��  */
    Uint32 makeup_val;  //����ʱ�Ӳ���ֵ
    short longitude_d;
    short longitude_f;
    short longitude_m;
    short latitude_d;
    short latitude_f;
    short latitude_m;
    int   height;   

};


struct Satellite_Data
{
    struct tm tm;
    unsigned long int  time; /*ʱ��������*/
    char position[64];
    char satellite_position;/*�ο���λ��������*/
    char satellite_see;     /*�ɼ�������*/
    char time_con;          /*�����Լ���*/
    char time_enable;       /*ʱ����ñ�־:1Ϊʱ����� ��0Ϊʱ�䲻����*/
    char satellite_use; /**ʹ����������  */
    char satellite_mode[3];/*���Ƕ�λģʽ*/

    short gps_utc_leaps;    /**������  */
    char leap_enable;      /**1������ã�0 ���벻����  */
    char antenna;          /**4 ��·3��· 2 ����  */
    char leap58or60;        /** 2 �����룬1 ������ 0 �޲���*/
    
    char eastorwest;   /**E,������W������*/
    char northorsouth;   /**N,��γ��S����γ��*/


    short longitude_d;
    short longitude_f;
    short longitude_m;
    short latitude_d;
    short latitude_f;
    short latitude_m;
    int   height;   

    
    float clock_tmperature;      /**���¶ȵ�λ0.01  */
    float battery_tmperature;      /**����¶ȵ�λ0.01  */
    float device_tmperature;      /**�Ǳ��ڲ������¶ȵ�λ0.01  */
    
    Uint16 batteryMah;
    Uint8 batteryPercenty;
    
    
    struct Manual_sat_data manual_sat_data;
};

char Gga(struct Satellite_Data *p_satellite_data,char *str);
char Zda(struct Satellite_Data *p_satellite_data,char *str);
char Rmc(struct Satellite_Data *p_satellite_data,char *str);
char Gsv(struct Satellite_Data *p_satellite_data,char *str);

char BdGpt(struct Satellite_Data *p_satellite_data,char *str);
char BdLps(struct Satellite_Data *p_satellite_data,char *str);
char BdVer(struct Satellite_Data *p_satellite_data,char *str);



char SatelliteHandle(struct Satellite_Data *p_satellite_data,char *str);





#endif

