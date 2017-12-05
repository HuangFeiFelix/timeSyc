#ifndef _LOGER_H
#define _LOGER_H
 
#include "common.h"


extern FILE *logp;

extern LOG_DEBUG debug_level;

extern void logMessage(LOG_DEBUG priority, const char * format, ...);
extern void logWarnMessage(LOG_DEBUG priority, const char * format, ...);
extern void logFileMessage(const char * format, ...);

extern void logerinit(FILE **fp);

#define MY_SLOG(level,format, ...) \
{                       \
    if(logp==NULL)      \
        logerinit(&logp);   \
    fprintf(logp,"| L=%d | %s %s | %s | %4.4dL | %s: ",level,__DATE__,__TIME__,__FILE__,__LINE__,__func__);      \
    fprintf(logp,format,##__VA_ARGS__)  ;   \
    fprintf(logp,"\n"); \
    fflush(logp);           \
}    \


#define MY_WLOG(level,format, ...) \
{                       \
    fprintf(stdout,"| L=%d | %s %s | %s | %4.4dL | %s: ",level,__DATE__,__TIME__,__FILE__,__LINE__,__func__);      \
    fprintf(stdout,format,##__VA_ARGS__)  ;   \
    fprintf(stdout,"\n"); \
    fflush(stdout);           \
}                               \


#define PLOG(format, ...)         logMessage(debug_level,format, ##__VA_ARGS__)
#define WLOG(format, ...)         logWarnMessage(debug_level,format, ##__VA_ARGS__)
#define SLOG(format, ...)         MY_SLOG(LEVEL_STORE,format, ##__VA_ARGS__)


#endif
