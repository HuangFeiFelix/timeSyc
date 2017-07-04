#ifndef __LCD_CMD_PROCESS_H__
#define __LCD_CMD_PROCESS_H__
#include "common.h"
#include "main.h"

#define NOTIFY_TOUCH_PRESS      0X01             //触摸屏按下通知
#define NOTIFY_TOUCH_RELEASE  0X03              //触摸屏松开通知
#define NOTIFY_WRITE_FLASH_OK  0X0C              //写FLASH成功
#define NOTIFY_WRITE_FLASH_FAILD  0X0D          //写FLASH失败
#define NOTIFY_READ_FLASH_OK  0X0B              //读FLASH成功
#define NOTIFY_READ_FLASH_FAILD  0X0F           //读FLASH失败
#define NOTIFY_MENU                        0X14  //菜单事件通知
#define NOTIFY_TIMER                       0X43  //定时器超时通知
#define NOTIFY_CONTROL                0XB1      //控件更新通知
#define NOTIFY_READ_RTC               0XF7       //读取RTC时间
#define MSG_GET_CURRENT_SCREEN 0X01              //画面ID变化通知
#define MSG_GET_DATA            0X11             //控件数据通知

#define PTR2U16(PTR) ((((Uint8 *)(PTR))[0]<<8)|((Uint8 *)(PTR))[1])  //从缓冲区取16位数据
#define PTR2U32(PTR) ((((Uint8 *)(PTR))[0]<<24)|(((Uint8 *)(PTR))[1]<<16)|(((Uint8 *)(PTR))[2]<<8)|((Uint8 *)(PTR))[3])  //从缓冲区取32位数据




enum CtrlType
{
	kCtrlUnknown=0x0,
	kCtrlButton=0x10,  //按钮
	kCtrlText,  //文本
	kCtrlProgress,  //进度条
	kCtrlSlider,    //滑动条
	kCtrlMeter,  //仪表
	kCtrlDropList, //下拉列表
	kCtrlAnimation, //动画
	kCtrlRTC, //时间显示
	kCtrlGraph, //曲线图控件
	kCtrlTable, //表格控件
	kCtrlMenu,//菜单控件
	kCtrlSelector,//选择控件
	kCtrlQRCode,//二维码
};

#pragma pack(push)
#pragma pack(1)	//按字节对齐

struct LcdCtlMsg
{
	Uint8    cmd_head;  //帧头

	Uint8    cmd_type;  //命令类型(UPDATE_CONTROL)	
	Uint8    ctrl_msg;   //CtrlMsgType-指示消息的类型
	Uint16   screen_id;  //产生消息的画面ID
	Uint16   control_id;  //产生消息的控件ID
	Uint8    control_type; //控件类型

	Uint8    param[256];//可变长度参数，最多256个字节

	Uint8  cmd_tail[4];   //帧尾
};


void ProcessLcdMessage(struct root_data *pRootData,char *buf,Uint16 len);

void NotifyScreen(Uint16 screen_id);

void NotifyTouchXY(Uint8 press,Uint16 x,Uint16 y);

void NotifyButton(Uint16 screen_id, Uint16 control_id, Uint8  state);

void NotifyText(Uint16 screen_id, Uint16 control_id, Uint8 *str);

void NotifyProgress(Uint16 screen_id, Uint16 control_id, Uint32 value);

void NotifySlider(Uint16 screen_id, Uint16 control_id, Uint32 value);

void NotifyMeter(Uint16 screen_id, Uint16 control_id, Uint32 value);

void NotifyMenu(Uint16 screen_id, Uint16 control_id, Uint8  item, Uint8  state);

void NotifySelector(Uint16 screen_id, Uint16 control_id, Uint8  item);

void NotifyTimer(Uint16 screen_id, Uint16 control_id);

void NotifyReadFlash(Uint8 status,Uint8 *_data,Uint16 length);

void NotifyWriteFlash(Uint8 status);

void NotifyReadRTC(Uint8 year,Uint8 month,Uint8 week,Uint8 day,Uint8 hour,Uint8 minute,Uint8 second);


#endif
