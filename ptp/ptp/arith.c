/********************************************************************************************
*                           版权所有(C) 2015, 电信科学技术第五研究所
*                                 版权所有
*********************************************************************************************
*    文 件 名:       arith.c
*    功能描述:       PTP 协议中常用算法，时间转换，时间加减
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-1-30
*    函数列表:
                     absTime
                     addTime
                     check_timestamp_is_fresh
                     check_timestamp_is_fresh2
                     clearTime
                     div2Time
                     doubleToTimeInternal
                     fromInternalTime
                     getPauseAfterMidnight
                     gtTime
                     integer64_to_internalTime
                     internalTime_to_integer64
                     isTimeInternalNegative
                     is_Time_close
                     Max_Fd
                     nano_to_Time
                     normalizeTime
                     secondsToMidnight
                     subTime
                     timeInternalToDouble
                     toInternalTime
                     ts_to_InternalTime
                     tv_to_InternalTime
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-1-30
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/

#include <limits.h>
#include "arith.h"
#include <math.h>

#include "comm_fun.h"

#define  SECONDS_1970_TO_2009  0x495C0780


void internalTime_to_integer64(TimeInternal internal, Integer64 *bigint)
{
	int64_t scaledNanoseconds;

	scaledNanoseconds = internal.seconds;
	scaledNanoseconds *= 1000000000;
	scaledNanoseconds += internal.nanoseconds;
	scaledNanoseconds <<= 16;

	bigint->msb = (scaledNanoseconds >> 32) & 0xffffffff;
	bigint->lsb = scaledNanoseconds & 0xffffffff;
}



void integer64_to_internalTime(Integer64 bigint, TimeInternal * internal)
{
	int sign;
	int64_t scaledNanoseconds;

	scaledNanoseconds = bigint.msb;
	scaledNanoseconds <<=32;
	scaledNanoseconds += bigint.lsb;
	
	/*determine sign of result big integer number*/

	if (scaledNanoseconds < 0) {
		scaledNanoseconds = -scaledNanoseconds;
		sign = -1;
	} else {
		sign = 1;
	}

	/*fractional nanoseconds are excluded (see 5.3.2)*/
	scaledNanoseconds >>= 16;
	internal->seconds = sign * (scaledNanoseconds / 1000000000);
	internal->nanoseconds = sign * (scaledNanoseconds % 1000000000);
}



void internalTime_to_longlong64(TimeInternal internal, Slonglong64 *bigint)
{
	*bigint = internal.seconds;
	*bigint *= 1000000000;
	*bigint += internal.nanoseconds;
    
}

void longlong64_to_internalTime(Slonglong64 bigint,TimeInternal *internal)
{
	int sign;
	
	/*determine sign of result big integer number*/
	if (bigint < 0) {
		bigint = -bigint;
		sign = -1;
	} else {
		sign = 1;
	}

	/*fractional nanoseconds are excluded (see 5.3.2)*/
	internal->seconds = sign * (bigint / 1000000000);
	internal->nanoseconds = sign * (bigint % 1000000000);

}



void fromInternalTime(const TimeInternal * internal, Timestamp * external)
{

	if ((internal->seconds & ~INT_MAX) || 
	    (internal->nanoseconds & ~INT_MAX)) {
		printf("Negative value canno't be converted into timestamp \n");
		return;
	} else {
		external->secondsField.lsb = internal->seconds;
		external->nanosecondsField = internal->nanoseconds;
		external->secondsField.msb = 0;
	}
}

void toInternalTime(TimeInternal * internal, const Timestamp * external)
{

	/* Program will not run after 2038... */
	if (external->secondsField.lsb < INT_MAX) {
		internal->seconds = external->secondsField.lsb;
		internal->nanoseconds = external->nanosecondsField;
	} else {
		printf("Clock servo canno't be executed : "
		    "seconds field is higher than signed integer (32bits) \n");
		return;
	}
}

void ts_to_InternalTime(const struct timespec *a,  TimeInternal * b)
{

	b->seconds = a->tv_sec;
	b->nanoseconds = a->tv_nsec;
}

void tv_to_InternalTime(const struct timeval *a,  TimeInternal * b)
{

	b->seconds = a->tv_sec;
	b->nanoseconds = a->tv_usec * 1000;
}


void normalizeTime(TimeInternal * r)
{
	r->seconds += r->nanoseconds / 1000000000;
	r->nanoseconds -= (r->nanoseconds / 1000000000) * 1000000000;

	if (r->seconds > 0 && r->nanoseconds < 0) {
		r->seconds -= 1;
		r->nanoseconds += 1000000000;
	} else if (r->seconds < 0 && r->nanoseconds > 0) {
		r->seconds += 1;
		r->nanoseconds -= 1000000000;
	}
}

void addTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y)
{
	r->seconds = x->seconds + y->seconds;
	r->nanoseconds = x->nanoseconds + y->nanoseconds;

	normalizeTime(r);
}

void subTime(TimeInternal * r, const TimeInternal * x, const TimeInternal * y)
{
	r->seconds = x->seconds - y->seconds;
	r->nanoseconds = x->nanoseconds - y->nanoseconds;

	normalizeTime(r);
}


/* TODO: this function could be simplified, as currently it is only called to halve the time */
void divTime(TimeInternal *r, int divisor)
{

	uint64_t nanoseconds;

	if (divisor <= 0)
		return;

	nanoseconds = ((uint64_t)r->seconds * 1000000000) + r->nanoseconds;
	nanoseconds /= divisor;

	r->seconds = 0;
	r->nanoseconds = nanoseconds;
	normalizeTime(r);
} 

void div2Time(TimeInternal *r)
{
	r->nanoseconds += (r->seconds % 2) * 1000000000;
	r->seconds /= 2;
	r->nanoseconds /= 2;

	normalizeTime(r);
}



/* clear an internal time value */
void clearTime(TimeInternal *r)
{
	r->seconds     = 0;
	r->nanoseconds = 0;
}


/* sets a time value to a certain nanoseconds */
void nano_to_Time(TimeInternal *x, int nano)
{
	x->seconds     = 0;
	x->nanoseconds = nano;
	normalizeTime(x);
}

/* greater than operation */
int gtTime(const TimeInternal *x, const TimeInternal *y)
{
	TimeInternal r;

	subTime(&r, x, y);
	return !isTimeInternalNegative(&r);
}

/* remove sign from variable */
void absTime(TimeInternal *r)
{
	/* Make sure signs are the same */
	normalizeTime(r);
	r->seconds       = abs(r->seconds);
	r->nanoseconds   = abs(r->nanoseconds);
}


/* if 2 time values are close enough for X nanoseconds */
int is_Time_close(const TimeInternal *x, const TimeInternal *y, int nanos)
{
	TimeInternal r1;
	TimeInternal r2;

	// first, subtract the 2 values. then call abs(),
	// then call gtTime for requested the number of nanoseconds
	subTime(&r1, x, y);
	absTime(&r1);

	nano_to_Time(&r2, nanos);
	
	return !gtTime(&r1, &r2);
}


int check_timestamp_is_fresh2(const TimeInternal * timeA, const TimeInternal * timeB)
{
	int ret;

	// maximum 1 millisecond offset	
	ret = is_Time_close(timeA, timeB, 1000000);
	printf("check_timestamp_is_fresh: %d\n ", ret);
	return ret;
}


int check_timestamp_is_fresh(const TimeInternal * timeA)
{
	TimeInternal timeB;
	getTime(&timeB);

	return check_timestamp_is_fresh2(timeA, &timeB);
}

int isTimeInternalNegative(const TimeInternal * p)
{
	return (p->seconds < 0) || (p->nanoseconds < 0);
}

float secondsToMidnight(void) 
{
	TimeInternal now;

	getTime(&now);

	float stmf = 86400 - (now.seconds % 86400);

	return (stmf - now.nanoseconds / 1E9);
}

float getPauseAfterMidnight(Integer8 announceInterval) 
{
	unsigned ai = 1U << announceInterval;

	if (LEAP_SECOND_PAUSE_PERIOD > 2 * ai)
		return ((float)LEAP_SECOND_PAUSE_PERIOD);
	else
		return (2.0f * ai);
}

double timeInternalToDouble(const TimeInternal * p)
{

	double sign = (p->seconds < 0 || p->nanoseconds < 0 ) ? -1.0 : 1.0;
	return (sign * ( abs(p->seconds) + abs(p->nanoseconds) / 1E9 ));

}

TimeInternal doubleToTimeInternal(const double d)
{

	TimeInternal t = {0, 0};

	t.seconds = trunc(d);
	t.nanoseconds = (d - (t.seconds + 0.0)) * 1E9;

	return t;

}

int Max_Fd(int a,int b)
{
    if(a > b)
        return a;
    else if(a < b)
        return b;
    else 
        return a;
}


