#include "lcd_driver.h"
#include "main.h"

struct LcdTxCtl
{
    Uint8 rx_cmd_buffer[CMD_MAX_SIZE];//发送指令缓存
    int index;
    
}m_lcd_tx;


#if 0
#define TX_8(P1) SEND_DATA((P1)&0xFF)  //发送单个字节
#define TX_8N(P,N) SendNU8((Uint8 *)P,N)  //发送N个字节
#define TX_16(P1) TX_8((P1)>>8);TX_8(P1)  //发送16位整数
#define TX_16N(P,N) SendNU16((Uint16 *)P,N)  //发送N个16位整数
#define TX_32(P1) TX_16((P1)>>16);TX_16((P1)&0xFFFF)  //发送32位整数
#endif

void TX_8(Uint8 val)
{
    val &= 0xFF;
    m_lcd_tx.rx_cmd_buffer[m_lcd_tx.index++] = val;

}
void TX_8N(Uint8 *buf,Uint16 len)
{
	Uint16 i;
	for (i=0;i<len;i++)
	{
		TX_8(buf[i]);
	}

}

void TX_16(Uint16 val)
{
    TX_8(val>>8);
    TX_8(val);
}
void TX_16N(Uint16 *buf,Uint16 nDataLen)
{
    Uint16 i = 0;
    for (;i<nDataLen;++i)
    {
        TX_16(buf[i]);
    }

}
void TX_32(Uint32 val)
{
    TX_16((val)>>16);
    TX_16((val)&0xFFFF);
}


void BEGIN_CMD()
{
    memset(&m_lcd_tx,0,sizeof(m_lcd_tx));
    m_lcd_tx.rx_cmd_buffer[m_lcd_tx.index++] = 0xEE;
}

void END_CMD()
{
    m_lcd_tx.rx_cmd_buffer[m_lcd_tx.index++] = 0xFF;
    m_lcd_tx.rx_cmd_buffer[m_lcd_tx.index++] = 0xFC;
    m_lcd_tx.rx_cmd_buffer[m_lcd_tx.index++] = 0xFF;
    m_lcd_tx.rx_cmd_buffer[m_lcd_tx.index++] = 0xFF;

    AddData_ToSendList(g_RootData,ENUM_LCD,m_lcd_tx.rx_cmd_buffer,m_lcd_tx.index);    /*调用发送函数*/
    m_lcd_tx.index = 0;

}


void DelayMS(unsigned int n) 
{
	int i,j;  
	for(i = n;i>0;i--)
		for(j=1000;j>0;j--) ; 
}

void SendStrings(Uint8 *str)
{
	while(*str)
	{
		TX_8(*str);
		str++;
	}
}

void SendNU8(Uint8 *pData,Uint16 nDataLen)
{
	Uint16 i = 0;
	for (;i<nDataLen;++i)
	{
		TX_8(pData[i]);
	}
}

void SendNU16(Uint16 *pData,Uint16 nDataLen)
{
	Uint16 i = 0;
	for (;i<nDataLen;++i)
	{
		TX_16(pData[i]);
	}
}

void SetHandShake()
{
	BEGIN_CMD();
	TX_8(0x00);
	END_CMD();
}

void SetFcolor(Uint16 color)
{
	BEGIN_CMD();
	TX_8(0x41);
	TX_16(color);
	END_CMD();
}

void SetBcolor(Uint16 color)
{
	BEGIN_CMD();
	TX_8(0x42);
	TX_16(color);
	END_CMD();
}

void ColorPicker(Uint8 mode, Uint16 x,Uint16 y)
{
	BEGIN_CMD();
	TX_8(0xA3);
	TX_8(mode);
	TX_16(x);
	TX_16(y);
	END_CMD();
}

void GUI_CleanScreen()
{
	BEGIN_CMD();
	TX_8(0x01);
	END_CMD();
}

void SetTextSpace(Uint8 x_w, Uint8 y_w)
{
	BEGIN_CMD();
	TX_8(0x43);
	TX_8(x_w);
	TX_8(y_w);
	END_CMD();
}

void SetFont_Region(Uint8 enable,Uint16 width,Uint16 height)
{
	BEGIN_CMD();
	TX_8(0x45);
	TX_8(enable);
	TX_16(width);
	TX_16(height);
	END_CMD();
}

void SetFilterColor(Uint16 fillcolor_dwon, Uint16 fillcolor_up)
{
	BEGIN_CMD();
	TX_8(0x44);
	TX_16(fillcolor_dwon);
	TX_16(fillcolor_up);
	END_CMD();
}

void DisText(Uint16 x, Uint16 y,Uint8 back,Uint8 font,Uint8 *strings )
{
	BEGIN_CMD();
	TX_8(0x20);
	TX_16(x);
	TX_16(y);
	TX_8(back);
	TX_8(font);
	SendStrings(strings);
	END_CMD();
}

void DisCursor(Uint8 enable,Uint16 x, Uint16 y,Uint8 width,Uint8 height )
{
	BEGIN_CMD();
	TX_8(0x21);
	TX_8(enable);
	TX_16(x);
	TX_16(y);
	TX_8(width);
	TX_8(height);
	END_CMD();
}

void DisFull_Image(Uint16 image_id,Uint8 masken)
{
	BEGIN_CMD();
	TX_8(0x31);
	TX_16(image_id);
	TX_8(masken);
	END_CMD();
}

void DisArea_Image(Uint16 x,Uint16 y,Uint16 image_id,Uint8 masken)
{
	BEGIN_CMD();
	TX_8(0x32);
	TX_16(x);
	TX_16(y);
	TX_16(image_id);
	TX_8(masken);
	END_CMD();
}

void DisCut_Image(Uint16 x,Uint16 y,Uint16 image_id,Uint16 image_x,Uint16 image_y,Uint16 image_l, Uint16 image_w,Uint8 masken)
{
	BEGIN_CMD();
	TX_8(0x33);
	TX_16(x);
	TX_16(y);
	TX_16(image_id);
	TX_16(image_x);
	TX_16(image_y);
	TX_16(image_l);
	TX_16(image_w);
	TX_8(masken);
	END_CMD();
}

void DisFlashImage(Uint16 x,Uint16 y,Uint16 flashimage_id,Uint8 enable,Uint8 playnum)
{
	BEGIN_CMD();
	TX_8(0x80);
	TX_16(x);
	TX_16(y);
	TX_16(flashimage_id);
	TX_8(enable);
	TX_8(playnum);
	END_CMD();
}

void GUI_Dot(Uint16 x,Uint16 y)
{
	BEGIN_CMD();
	TX_8(0x50);
	TX_16(x);
	TX_16(y);
	END_CMD();
}

void GUI_Line(Uint16 x0, Uint16 y0, Uint16 x1, Uint16 y1)
{
	BEGIN_CMD();
	TX_8(0x51);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void GUI_ConDots(Uint8 mode,Uint16 *dot,Uint16 dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x63);
	TX_8(mode);
	TX_16N(dot,dot_cnt*2);
	END_CMD();
}

void GUI_ConSpaceDots(Uint16 x,Uint16 x_space,Uint16 *dot_y,Uint16 dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x59);
	TX_16(x);
	TX_16(x_space);
	TX_16N(dot_y,dot_cnt);
	END_CMD();
}

void GUI_FcolorConOffsetDots(Uint16 x,Uint16 y,Uint16 *dot_offset,Uint16 dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x75);
	TX_16(x);
	TX_16(y);
	TX_16N(dot_offset,dot_cnt);
	END_CMD();
}

void GUI_BcolorConOffsetDots(Uint16 x,Uint16 y,Uint16 *dot_offset,Uint16 dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x76);
	TX_16(x);
	TX_16(y);
	TX_16N(dot_offset,dot_cnt);
	END_CMD();
}

void SetPowerSaving(Uint8 enable, Uint8 bl_off_level, Uint8 bl_on_level, Uint8  bl_on_time)
{
	BEGIN_CMD();
	TX_8(0x77);
	TX_8(enable);
	TX_8(bl_off_level);
	TX_8(bl_on_level);
	TX_8(bl_on_time);
	END_CMD();
}

void GUI_FcolorConDots(Uint16 *dot,Uint16 dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x68);
	TX_16N(dot,dot_cnt*2);
	END_CMD();
}

void GUI_BcolorConDots(Uint16 *dot,Uint16 dot_cnt)
{
	BEGIN_CMD();
	TX_8(0x69);
	TX_16N(dot,dot_cnt*2);
	END_CMD();
}

void GUI_Circle(Uint16 x, Uint16 y, Uint16 r)
{
	BEGIN_CMD();
	TX_8(0x52);
	TX_16(x);
	TX_16(y);
	TX_16(r);
	END_CMD();
}

void GUI_CircleFill(Uint16 x, Uint16 y, Uint16 r)
{
	BEGIN_CMD();
	TX_8(0x53);
	TX_16(x);
	TX_16(y);
	TX_16(r);
	END_CMD();
}

void GUI_Arc(Uint16 x,Uint16 y, Uint16 r,Uint16 sa, Uint16 ea)
{
	BEGIN_CMD();
	TX_8(0x67);
	TX_16(x);
	TX_16(y);
	TX_16(r);
	TX_16(sa);
	TX_16(ea);
	END_CMD();
}

void GUI_Rectangle(Uint16 x0, Uint16 y0, Uint16 x1,Uint16 y1 )
{
	BEGIN_CMD();
	TX_8(0x54);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void GUI_RectangleFill(Uint16 x0, Uint16 y0, Uint16 x1,Uint16 y1 )
{
	BEGIN_CMD();
	TX_8(0x55);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void GUI_Ellipse(Uint16 x0, Uint16 y0, Uint16 x1,Uint16 y1 )
{
	BEGIN_CMD();
	TX_8(0x56);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void GUI_EllipseFill(Uint16 x0, Uint16 y0, Uint16 x1,Uint16 y1 )
{
	BEGIN_CMD();
	TX_8(0x57);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void SetBackLight(Uint8 light_level)
{
	BEGIN_CMD();
	TX_8(0x60);
	TX_8(light_level);
	END_CMD();
}

void SetBuzzer(Uint8 time)
{
	BEGIN_CMD();
	TX_8(0x61);
	TX_8(time);
	END_CMD();
}

void GUI_AreaInycolor(Uint16 x0, Uint16 y0, Uint16 x1,Uint16 y1 )
{
	BEGIN_CMD();
	TX_8(0x65);
	TX_16(x0);
	TX_16(y0);
	TX_16(x1);
	TX_16(y1);
	END_CMD();
}

void SetTouchPaneOption(Uint8 enbale,Uint8 beep_on,Uint8 work_mode,Uint8 press_calibration)
{
	Uint8 options = 0;

	if(enbale)
		options |= 0x01;
	if(beep_on)
		options |= 0x02;
	if(work_mode)
		options |= (work_mode<<2);
	if(press_calibration)
		options |= (press_calibration<<5);

	BEGIN_CMD();
	TX_8(0x70);
	TX_8(options);
	END_CMD();
}

void CalibrateTouchPane()
{
	BEGIN_CMD();
	TX_8(0x72);
	END_CMD();
}

void TestTouchPane()
{
	BEGIN_CMD();
	TX_8(0x73);
	END_CMD();
}

void LockDeviceConfig(void)
{
	BEGIN_CMD();
	TX_8(0x09);
	TX_8(0xDE);
	TX_8(0xED);
	TX_8(0x13);
	TX_8(0x31);
	END_CMD();
}

void UnlockDeviceConfig(void)
{
	BEGIN_CMD();
	TX_8(0x08);
	TX_8(0xA5);
	TX_8(0x5A);
	TX_8(0x5F);
	TX_8(0xF5);
	END_CMD();
}

void SetCommBps(Uint8 option)
{
	BEGIN_CMD();
	TX_8(0xA0);
	TX_8(option);
	END_CMD();
}

void WriteLayer(Uint8 layer)
{
	BEGIN_CMD();
	TX_8(0xA1);
	TX_8(layer);
	END_CMD();
}

void DisplyLayer(Uint8 layer)
{
	BEGIN_CMD();
	TX_8(0xA2);
	TX_8(layer);
	END_CMD();
}

void CopyLayer(Uint8 src_layer,Uint8 dest_layer)
{
	BEGIN_CMD();
	TX_8(0xA4);
	TX_8(src_layer);
	TX_8(dest_layer);
	END_CMD();
}

void ClearLayer(Uint8 layer)
{
	BEGIN_CMD();
	TX_8(0x05);
	TX_8(layer);
	END_CMD();
}

void GUI_DispRTC(Uint8 enable,Uint8 mode,Uint8 font,Uint16 color,Uint16 x,Uint16 y)
{
	BEGIN_CMD();
	TX_8(0x85);
	TX_8(enable);
	TX_8(mode);
	TX_8(font);
	TX_16(color);
	TX_16(x);
	TX_16(y);
	END_CMD();
}

void WriteUserFlash(Uint32 startAddress,Uint16 length,Uint8 *_data)
{
	BEGIN_CMD();
	TX_8(0x87);
	TX_32(startAddress);
	TX_8N(_data,length);
	END_CMD();
}

void ReadUserFlash(Uint32 startAddress,Uint16 length)
{
	BEGIN_CMD();
	TX_8(0x88);
	TX_32(startAddress);
	TX_16(length);
	END_CMD();
}

void GetScreen()
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x01);
	END_CMD();
}

void SetScreen(Uint16 screen_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x00);
	TX_16(screen_id);
	END_CMD();
}

void SetScreenUpdateEnable(Uint8 enable)
{
	BEGIN_CMD();
	TX_8(0xB3);
	TX_8(enable);
	END_CMD();
}

void SetControlFocus(Uint16 screen_id,Uint16 control_id,Uint8 focus)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x02);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(focus);
	END_CMD();
}

void SetControlVisiable(Uint16 screen_id,Uint16 control_id,Uint8 visible)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x03);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(visible);
	END_CMD();
}

void SetControlEnable(Uint16 screen_id,Uint16 control_id,Uint8 enable)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x04);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(enable);
	END_CMD();
}

void SetButtonValue(Uint16 screen_id,Uint16 control_id,Uint8 state)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(state);
	END_CMD();
}

void SetTextValue(Uint16 screen_id,Uint16 control_id,Uint8 *str)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	SendStrings(str);
	END_CMD();
}

void SetProgressValue(Uint16 screen_id,Uint16 control_id,Uint32 value)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(value);
	END_CMD();
}

void SetMeterValue(Uint16 screen_id,Uint16 control_id,Uint32 value)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(value);
	END_CMD();
}

void SetSliderValue(Uint16 screen_id,Uint16 control_id,Uint32 value)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(value);
	END_CMD();
}

void SetSelectorValue(Uint16 screen_id,Uint16 control_id,Uint8 item)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x10);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(item);
	END_CMD();
}

void GetControlValue(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x11);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationStart(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x20);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationStop(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x21);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationPause(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x22);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationPlayFrame(Uint16 screen_id,Uint16 control_id,Uint8 frame_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x23);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(frame_id);
	END_CMD();
}

void AnimationPlayPrev(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x24);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void AnimationPlayNext(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x25);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void GraphChannelAdd(Uint16 screen_id,Uint16 control_id,Uint8 channel,Uint16 color)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x30);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(channel);
	TX_16(color);
	END_CMD();
}

void GraphChannelDel(Uint16 screen_id,Uint16 control_id,Uint8 channel)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x31);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(channel);
	END_CMD();
}

void GraphChannelDataAdd(Uint16 screen_id,Uint16 control_id,Uint8 channel,Uint8 *pData,Uint16 nDataLen)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x32);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(channel);
	TX_16(nDataLen);
	TX_8N(pData,nDataLen);
	END_CMD();
}

void GraphChannelDataClear(Uint16 screen_id,Uint16 control_id,Uint8 channel)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x33);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(channel);
	END_CMD();
}

void GraphSetViewport(Uint16 screen_id,Uint16 control_id,Sint16 x_offset,Uint16 x_mul,Sint16 y_offset,Uint16 y_mul)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x34);
	TX_16(screen_id);
	TX_16(control_id);
	TX_16(x_offset);
	TX_16(x_mul);
	TX_16(y_offset);
	TX_16(y_mul);
	END_CMD();
}

void BatchBegin(Uint16 screen_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x12);
	TX_16(screen_id);
}

void BatchSetButtonValue(Uint16 control_id,Uint8 state)
{
	TX_16(control_id);
	TX_16(1);
	TX_8(state);
}

void BatchSetProgressValue(Uint16 control_id,Uint32 value)
{
	TX_16(control_id);
	TX_16(4);
	TX_32(value);
}

void BatchSetSliderValue(Uint16 control_id,Uint32 value)
{
	TX_16(control_id);
	TX_16(4);
	TX_32(value);
}

void BatchSetMeterValue(Uint16 control_id,Uint32 value)
{
	TX_16(control_id);
	TX_16(4);
	TX_32(value);
}

Uint32 GetStringLen(Uint8 *str)
{
	Uint8 *p = str;
	while(*str)
	{
		str++;
	}

	return (str-p);
} 

void BatchSetText(Uint16 control_id,Uint8 *strings)
{
	TX_16(control_id);
	TX_16(GetStringLen(strings));
	SendStrings(strings);
}

void BatchSetFrame(Uint16 control_id,Uint16 frame_id)
{
	TX_16(control_id);
	TX_16(2);
	TX_16(frame_id);
}

void BatchEnd()
{
	END_CMD();
}

void SeTimer(Uint16 screen_id,Uint16 control_id,Uint32 timeout)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x40);
	TX_16(screen_id);
	TX_16(control_id);
	TX_32(timeout);
	END_CMD();
}

void StartTimer(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x41);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void StopTimer(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x42);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void PauseTimer(Uint16 screen_id,Uint16 control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x44);
	TX_16(screen_id);
	TX_16(control_id);
	END_CMD();
}

void SetControlBackColor(Uint16 screen_id,Uint16 control_id,Uint16 color)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x18);
	TX_16(screen_id);
	TX_16(control_id);
	TX_16(color);
	END_CMD();
}

void SetControlForeColor(Uint16 screen_id,Uint16 control_id,Uint16 color)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x19);
	TX_16(screen_id);
	TX_16(control_id);
	TX_16(color);
	END_CMD();
}

void ShowPopupMenu(Uint16 screen_id,Uint16 control_id,Uint8 show,Uint16 focus_control_id)
{
	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x13);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(show);
	TX_16(focus_control_id);
	END_CMD();
}

void ShowKeyboard(Uint8 show,Uint16 x,Uint16 y,Uint8 type,Uint8 option,Uint8 max_len)
{
	BEGIN_CMD();
	TX_8(0x86);
	TX_8(show);
	TX_16(x);
	TX_16(y);
	TX_8(type);
	TX_8(option);
	TX_8(max_len);
	END_CMD();
}
