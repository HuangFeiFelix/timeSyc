/********************************************************************************************
*                           版权所有(C) 2015,
*                                 版权所有
*********************************************************************************************
*    文 件 名:       satellite_api.c
*    功能描述:       卫星数据处理
*    作者:           陈家毅
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-4-30
*    函数列表:
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-4-30
                     作者: 陈家毅
                     修改内容: 新创建文件


*********************************************************************************************/

/*----------------------------------------------*
*    包含头文件                                   *
*----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/timeb.h>

#include "satellite_api.h"

Uint8 g_utc_offset;

//BD_100接收数据格式
// $NVGGA,013423.00,3039.4582,N,10405.0352,E,1,9,1.55,+474.12,M,0.1*25
// $NVRMC,013423.00,A,3039.4582,N,10405.0352,E,0.035,27.500,070515,0.000,,500*3A
// $NVZDA,013423.00,07,05,2015,07,26*69


/********************************************************************************************
* 函数名称:    gga
* 功能描述:    gga数据解析处理
* 输入参数:    struct Satellite_Data *p_satellite_data  
               char *str                                
* 输出参数:    
* 返回值:      char
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-4-30
               作者:     陈家毅
               修改内容: 新生成函数

********************************************************************************************/
char Gga(struct Satellite_Data *p_satellite_data,char *str)
{
        int status;
        regex_t reg;
        char tmp_str[32];
        Uint32 val;
        int cflags = REG_EXTENDED;
        const size_t nmatch = 15;
        regmatch_t matchptr[15];
      //  printf("gga str :%s\n",str);
        const char *pattern =
         "^\\$(\\w+),(\\w+.\\w+),(\\w+.\\w+),(\\w+),(\\w+.\\w+),(\\w+),(\\w+),(\\w+),(\\w+.\\w+),(\\w+.\\w+)";
          // $NVGGA, 013423.00,  3039.4582,   N,  10405.0352,   E,    1     ,9   ,1.55      ,+474.12 ,M,0.1*25
        regcomp(&reg, pattern, cflags);
        if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
        {
            memset(p_satellite_data->position,0,sizeof(p_satellite_data->position));
            memcpy(p_satellite_data->position, str + matchptr[3].rm_so,
                    matchptr[3].rm_eo  - matchptr[3].rm_so);
            
            memset(tmp_str,0,sizeof(tmp_str));
            memcpy(tmp_str, str + matchptr[3].rm_so,matchptr[3].rm_eo - matchptr[3].rm_so);
            
            val = atof(tmp_str) * 10000;
            p_satellite_data->latitude_d = val/1000000;
            val = val%1000000;
            p_satellite_data->latitude_f = val/10000;
            val = val%10000;
            val = val*60;
            val = val/10000;
            p_satellite_data->latitude_m = val;
            
            p_satellite_data->northorsouth = *(str + matchptr[4].rm_so);

            memset(tmp_str,0,sizeof(tmp_str));
            memcpy(tmp_str, str + matchptr[5].rm_so,matchptr[5].rm_eo - matchptr[5].rm_so);

            val = atof(tmp_str) * 10000;
            p_satellite_data->longitude_d = val/1000000;
            val = val%1000000;
            p_satellite_data->longitude_f = val/10000;
            val = val%10000;
            val = val*60;
            val = val/10000;
            p_satellite_data->longitude_m = val;

            p_satellite_data->eastorwest = *(str + matchptr[6].rm_so);

            memset(tmp_str,0,sizeof(tmp_str));
            memcpy(tmp_str, str + matchptr[8].rm_so,
                    matchptr[8].rm_eo - matchptr[8].rm_so);
            p_satellite_data->satellite_position = atoi(tmp_str);  


            memset(tmp_str,0,sizeof(tmp_str));
            memcpy(tmp_str, str + matchptr[10].rm_so,matchptr[10].rm_eo - matchptr[10].rm_so);
            
            p_satellite_data->height = atof(tmp_str)*100;

            //printf("==>%d %d %d %d\n",p_satellite_data->longitude_d,p_satellite_data->longitude_f,p_satellite_data->longitude_m,p_satellite_data->height);
            regfree(&reg);
            return 1;
        }
        else
        {
            regfree(&reg);
            return 0;
        }

}



/********************************************************************************************
* 函数名称:    zda
* 功能描述:    ZDA数据解析
* 输入参数:    struct Satellite_Data *p_satellite_data  
               char *str                                
* 输出参数:    char
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-4-30
               作者:     陈家毅
               修改内容: 新生成函数

********************************************************************************************/
char Zda(struct Satellite_Data *p_satellite_data,char *str)
{
	int status;
	regex_t reg;
	char tmp_str[32];
	int cflags = REG_EXTENDED;
	const size_t nmatch = 15;
	regmatch_t matchptr[15];
    //// $NVZDA,013423.00,07,05,2015,07,26*69
	const char *pattern ="^\\$(\\w+),(\\w+.\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
         
        memset(tmp_str,'\0',sizeof(tmp_str));
	    memcpy(tmp_str, str + matchptr[2].rm_so, 2);
	    p_satellite_data->tm.tm_hour = atoi(tmp_str);
        memset(tmp_str,'\0',sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[2].rm_so + 2, 2);
	    p_satellite_data->tm.tm_min = atoi(tmp_str);
        memset(tmp_str,'\0',sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[2].rm_so + 4, 2);
	    p_satellite_data->tm.tm_sec = atoi(tmp_str);
        memset(tmp_str,'\0',sizeof(tmp_str));
	    memcpy(tmp_str, str + matchptr[3].rm_so,
		matchptr[3].rm_eo - matchptr[3].rm_so);
	    p_satellite_data->tm.tm_mday = atoi(tmp_str);
        memset(tmp_str,'\0',sizeof(tmp_str));
	    memcpy(tmp_str, str + matchptr[4].rm_so,
		matchptr[4].rm_eo - matchptr[4].rm_so);
	    p_satellite_data->tm.tm_mon = atoi(tmp_str)-1;
        memset(tmp_str,'\0',sizeof(tmp_str));
	    memcpy(tmp_str, str + matchptr[5].rm_so,
		matchptr[5].rm_eo - matchptr[5].rm_so);
	    p_satellite_data->tm.tm_year = atoi(tmp_str)-1900; 

        p_satellite_data->time = mktime(&p_satellite_data->tm);
        //printf("Gps all time =%d \n",p_satellite_data->time);
        
		regfree(&reg);
		return 1;
	}
	 else 
	{
		regfree(&reg);
		return 0;
	}

}
/********************************************************************************************
* 函数名称:    rmc
* 功能描述:    GSV数据解析处理
* 输入参数:    struct Satellite_Data *p_satellite_data  
               char *str                                
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-4-30
               作者:     陈家毅
               修改内容: 新生成函数

********************************************************************************************/


char Rmc(struct Satellite_Data *p_satellite_data,char *str)
{
	int status;
	regex_t reg;
	char tmp_str[32];
	int cflags = REG_EXTENDED;
	const size_t nmatch = 15;
	regmatch_t matchptr[15];
    //  printf("rmc str :%s\n",str);    //$NVRMC,013423.00,A,3039.4582,N,10405.0352,E                         ,0.035,     27.500,070515,0.000,,500*3A
    const char *pattern ="^\\$(\\w+),(\\w+.\\w+),(\\w+),(\\w+.\\w+),(\\w+),(\\w+.\\w+),(\\w+),(\\w+.\\w+),(\\w+.\\w+),(\\w+),(\\w+.\\w+),,(\\w+)";
    regcomp(&reg, pattern, cflags);       
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        memset(tmp_str,'\0',sizeof(tmp_str));
        memcpy(p_satellite_data->satellite_mode, str + matchptr[12].rm_so,
    	matchptr[12].rm_eo - matchptr[12].rm_so);
        printf("rmc:%s\n",p_satellite_data->satellite_mode);
        regfree(&reg);
    	return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

    }

}

/********************************************************************************************
* 函数名称:    gsv
* 功能描述:    GSV数据解析处理
* 输入参数:    struct Satellite_Data *p_satellite_data  
               char *str                                
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-4-30
               作者:     陈家毅
               修改内容: 新生成函数

********************************************************************************************/
char Gsv(struct Satellite_Data *p_satellite_data,char *str)
{
	int status;
	regex_t reg;
	char tmp_str[32];
	int cflags = REG_EXTENDED;
	const size_t nmatch = 15;
	regmatch_t matchptr[15];
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+)";

    //const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        memcpy(tmp_str, str + matchptr[4].rm_so,
		matchptr[4].rm_eo - matchptr[4].rm_so);
		p_satellite_data->satellite_see = atoi(tmp_str);
        
        regfree(&reg);
		return 1;
    }
    else
    {
        //p_satellite_data->satellite_see = 0;
        regfree(&reg);
        return 0;

     }

}


char BdGpt(struct Satellite_Data *p_satellite_data,char *str)
{
    int status;
    regex_t reg;
    char tmp_str[32];
    int cflags = REG_EXTENDED;
    const size_t nmatch = 15;
    regmatch_t matchptr[15];
    short leaps;
    
    /** $BDGPT,<1>,<2>, <3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>*HH<CR><LF>*/
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        /**闰秒数  */
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[7].rm_so,
		matchptr[7].rm_eo - matchptr[7].rm_so);

        leaps = atoi(tmp_str);
		p_satellite_data->gps_utc_leaps = atoi(tmp_str);
        p_satellite_data->gps_utc_leaps = (leaps/10) *16 + (leaps%10);
        g_utc_offset = p_satellite_data->gps_utc_leaps;
        
        /**闰秒可用  */
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[8].rm_so,
		matchptr[8].rm_eo - matchptr[8].rm_so);

        if(tmp_str[1]&0x40)
            p_satellite_data->leap_enable = TRUE;
        else
            p_satellite_data->leap_enable = FALSE;

        if((tmp_str[1]&0x07) == 0x04)
            p_satellite_data->time_enable = 0;
        else if((tmp_str[1]&0x07) == 0x05)
            p_satellite_data->time_enable = 1;

        /**天线状态  */
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[9].rm_so,
		matchptr[9].rm_eo - matchptr[9].rm_so);
		p_satellite_data->antenna = atoi(tmp_str);

        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[11].rm_so,
		matchptr[11].rm_eo - matchptr[11].rm_so);
        p_satellite_data->satellite_use = atoi(tmp_str);
        
        regfree(&reg);
		return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

     }

}


char BdLps(struct Satellite_Data *p_satellite_data,char *str)
{
    int status;
    regex_t reg;
    char tmp_str[32];
    int cflags = REG_EXTENDED;
    const size_t nmatch = 15;
    regmatch_t matchptr[15];

    /** $BDLPS,<1>,<2>, <3>,<4>,<5>*HH<CR><LF>*/
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);

    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        memset(tmp_str,0,sizeof(tmp_str));
        memcpy(tmp_str, str + matchptr[2].rm_so,
        matchptr[2].rm_eo - matchptr[2].rm_so);
        p_satellite_data->leap58or60= atoi(tmp_str);
        
        regfree(&reg);
		return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

    }

}

char BdVer(struct Satellite_Data *p_satellite_data,char *str)
{
    int status;
    regex_t reg;
    char tmp_str[32];
    int cflags = REG_EXTENDED;
    const size_t nmatch = 15;
    regmatch_t matchptr[15];

    /** $BDGPT,<1>,<2>, <3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>*HH<CR><LF>*/
    const char *pattern ="^\\$(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+),(\\w+)";
    regcomp(&reg, pattern, cflags);
    if ((status = regexec(&reg, str, nmatch, matchptr, 0)) == 0)
	{
        
        regfree(&reg);
		return 1;
    }
    else
    {
        regfree(&reg);
        return 0;

    }

}

char BdGsa(struct Satellite_Data *p_satellite_data,char *str)
{

}

/********************************************************************************************
* 函数名称:    satellite_handle
* 功能描述:    vcom接收数据处理
* 输入参数:    struct Satellite_Data *p_satellite_data  
               char *str                                
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-4-30
               作者:     陈家毅
               修改内容: 新生成函数

********************************************************************************************/
char SatelliteHandle(struct Satellite_Data *p_satellite_data,char *str)
{
    char *p;
    char temp[256];
    char ret;
    
    memcpy(temp,str,strlen(str));

    if(NULL != strstr(temp,"$GPZDA"))
    {
        /**获得时间  */
        ret = Zda(p_satellite_data,str); 
        if(!ret)
            p_satellite_data->time++;

    }
    else if(NULL != strstr(temp,"$GPGGA"))
    {
        /**获得  位置*/
        Gga(p_satellite_data,str);    
    }
    else if(NULL != strstr(temp,"$GPRMC"))
    {
        /**卫星运行模式  */
        Rmc(p_satellite_data,str);   
    }
    else if(NULL != strstr(temp,"$GPGSV"))
    {
        Gsv(p_satellite_data,str);
    }
    else if(NULL != strstr(temp,"$BDGPT"))
    {
        /**闰秒数，闰秒使能，天线状态  */
        BdGpt(p_satellite_data,str);
       
    }
    else if(NULL != strstr(temp,"$BDLPS"))
    {
        BdLps(p_satellite_data,str);
       
    }
    else if(NULL != strstr(temp,"$BDVER"))
    {
        BdVer(p_satellite_data,str);
    }
    else if(NULL != strstr(temp,"$GPTXT"))
    {
        
    }
    else if(NULL != strstr(temp,"$GPGSA"))
    {
        BdGsa(p_satellite_data,str);
    }
    else
    {
        //printf("-----------------\n");

    }

}

void Display_SatelliteData(struct Satellite_Data *p_satellite_data)
{
    printf("satellite_position=%d satellite_see=%d gps_utc_leaps=%d time_enable=%d antenna=%d\n",
        p_satellite_data->satellite_position,p_satellite_data->satellite_see
        ,p_satellite_data->gps_utc_leaps,p_satellite_data->time_enable,p_satellite_data->antenna);
}




