#ifndef __SATELLITE_API_H_
#define __SATELLITE_API_H_

#include <time.h>
#include "common.h"

struct Manual_sat_data
{
    char manual_enable;  /**人工设置使能1，开，0，关*/
    char location_state;  /**1,进入位置保持，0退出位置保持 */
    char relocation;     /**1,重定位 0，不变*/
    char eastorwest;     /**E,西经，W，东经*/
    char northorsouth;   /**N,北纬，S，南纬度*/
    Uint32 refSetTime;  /**设置参考源时间  */
    Uint32 makeup_val;  //馈线时延补偿值
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
    unsigned long int  time; /*时间总秒数*/
    char position[64];
    char satellite_position;/*参考定位的卫星数*/
    char satellite_see;     /*可见卫星数*/
    char time_con;          /*连续性记数*/
    char time_enable;       /*时间可用标志:1为时间可用 ，0为时间不可用*/
    char satellite_use; /**使用卫星数量  */
    char satellite_mode[3];/*卫星定位模式*/

    short gps_utc_leaps;    /**闰秒数  */
    char leap_enable;      /**1闰秒可用，0 闰秒不可用  */
    char antenna;          /**4 开路3短路 2 正常  */
    char leap58or60;        /** 2 负闰秒，1 正闰秒 0 无操作*/
    
    char eastorwest;   /**E,西经，W，东经*/
    char northorsouth;   /**N,北纬，S，南纬度*/


    short longitude_d;
    short longitude_f;
    short longitude_m;
    short latitude_d;
    short latitude_f;
    short latitude_m;
    int   height;   

    
    float clock_tmperature;      /**钟温度单位0.01  */
    float battery_tmperature;      /**电池温度单位0.01  */
    float device_tmperature;      /**仪表内部空气温度单位0.01  */
    
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

