#ifndef __FPGA_TIME_H__
#define __FPGA_TIME_H__


#include "common.h"
#include "data_type.h"

extern void GetFpgaRuningTime(TimeInternal *pTime);

extern void WriteCorrectionToFpgaCorrection(PtpClock *pPtpClock,Integer64 *correction);

extern void WriteT4ToFpga(PtpClock *pPtpClock,TimeInternal *RecvTime);

extern void Control_LedRun(Boolean val);

extern void GetBackBusData(char *m_buf,char *s_buf);

extern void SetFpgaTime(Integer32 sec);

extern void SetI2cChannel(Uint8 val);
extern void SetFpgaAddressVal(short offset,Uint8 val);
extern Uint8 GetFpgaAddressVal(short offset);
extern void BlockPtpOutput(PtpClock* pPtpClock);
extern void ResumePtpOutput(PtpClock* pPtpClock);
extern void BlockNtpOutput(PtpClock* pPtpClock);
extern void ResumeNtpOutput(PtpClock* pPtpClock);
extern void CollectAlarm(UartDevice *pUartDevice);
#endif
