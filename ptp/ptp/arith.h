#ifndef _ARITH_H_
#define _ARITH_H_

#include "common.h"
#include "data_type.h"

extern void addTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y);
extern void subTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y);
extern void div2Time(TimeInternal *r);

extern void ts_to_InternalTime(const struct timespec *a,  TimeInternal * b);

extern void tv_to_InternalTime(const struct timeval *a,  TimeInternal * b);

extern void normalizeTime(TimeInternal * r);

extern void internalTime_to_integer64(TimeInternal internal, Integer64 *bigint);

extern void integer64_to_internalTime(Integer64 bigint, TimeInternal * internal);

extern void internalTime_to_longlong64(TimeInternal internal, Slonglong64 *bigint);
extern void longlong64_to_internalTime(Slonglong64 bigint,TimeInternal *internal);

extern void fromInternalTime(const TimeInternal * internal, Timestamp * external);
extern void toInternalTime(TimeInternal * internal, const Timestamp * external);
extern TimeInternal doubleToTimeInternal(const double d);

extern int isTimeInternalNegative(const TimeInternal * p);

extern int Max_Fd(int a,int b);

extern void AllYearSecondToUtc(Uint32 AllYear_second,Uint32 *year,Uint32 *day,Uint32 *hour,Uint32 *minute,Uint32 *second);

#endif



