#include "log.h"
#include <stdarg.h>

FILE *logp;
LOG_DEBUG debug_level = LEVEL_STORE;
static int log_count  = 0;

void logerinit(FILE **fp)
{
   char sbuf[50];
   struct tm *tm;
   time_t current_time;
   memset(sbuf,0,sizeof(sbuf));

   
   current_time = time(NULL);
   tm = gmtime(&current_time);
   
   sprintf(sbuf,"/home/%d%d%d_%02d:%02d:%02d.log",tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);

    *fp=fopen(sbuf,"w+");
    if(*fp==NULL)
        perror("fopen logfile");
}

#define MY_SLOG(level,format, ...) \
{                       \
    if(logp==NULL)      \
        logerinit(&logp);   \
    fprintf(logp,"| L=%d | %s %s | %s | %4.4dL | %s: ",level,__DATE__,__TIME__,__FILE__,__LINE__,__func__);      \
    fprintf(logp,format,##__VA_ARGS__)  ;   \
    fprintf(logp,"\n"); \
    fflush(logp);           \
}    \

void logFileMessage(const char * format, ...)
{
    if(logp==NULL)
        logerinit(&logp);
    if(debug_level == LEVEL_STORE)
    {
        va_list arg;
        va_start(arg, format);
        fprintf(logp,"%s %s | %s | %4.4dL | %s: ",__DATE__,__TIME__,__FILE__,__LINE__,__func__);
        fprintf(logp,format,arg);
        fprintf(logp,"\n");
        fflush(logp);
        va_end(arg);
        log_count++;

        if(log_count >= 10000)
        {
            log_count = 0;
            fclose(logp);
            logp = NULL;
        }
    }
    else if(debug_level == LEVEL_NULL)
    {}
    else
    {
        va_list arg;
        va_start(arg, format);
        vfprintf(stdout,format,arg);
        va_end(arg);
    }
}


void logMessage(LOG_DEBUG priority, const char * format, ...)
{
    va_list arg;
    va_start(arg, format);

    switch(priority)
    {
        case LEVEL_NULL:
            va_end(arg);
            break;
            
        case LEVEL_INFO:
            vfprintf(stdout,format,arg);
            va_end(arg);
            break;
            
        case LEVEL_WARN:
            vfprintf(stdout,format,arg);
            va_end(arg);
            break;
        case LEVEL_STORE:
            
            va_end(arg);
            
            break;
        default:
            va_end(arg);
            break;
    }

}


void logWarnMessage(LOG_DEBUG priority, const char * format, ...)
{
    va_list arg;
    va_start(arg, format);

    switch(priority)
    {
        case LEVEL_NULL:
            va_end(arg);
            break;
            
        case LEVEL_INFO:
            va_end(arg);
            break;
            
        case LEVEL_WARN:
            vfprintf(stdout,format,arg);
            va_end(arg);
            break;
        case LEVEL_STORE:
            va_end(arg);
            break;
        default:
            va_end(arg);
            break;
    }

}


