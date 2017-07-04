#include "log.h"
#include <stdarg.h>

#define LOGFILEPATH "./wk.log"

FILE *logp;
LOG_DEBUG debug_level = LEVEL_WARN;

void logerinit(FILE **fp)
{        
    *fp=fopen(LOGFILEPATH,"a");
    if(*fp==NULL)
        perror("fopen logfile");
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


