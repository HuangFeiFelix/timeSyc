#ifndef __SATELLITE_API_H_
#define __SATELLITE_API_H_

#include <time.h>
#include "common.h"


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

