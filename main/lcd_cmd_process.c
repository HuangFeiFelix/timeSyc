#include "lcd_driver.h"
#include "lcd_cmd_process.h"
#include "clock_rb.h"

static Uint16 current_screen_id = 0;//当前画面ID
static Sint32 test_value = 0;//测试值
static Uint8 update_en = 0;//更新标记




/*! 
 *  \brief  画面切换通知
 *  \details  当前画面改变时(或调用GetScreen)，执行此函数
 *  \param screen_id 当前画面ID
 */
void NotifyScreen(Uint16 screen_id)
{
	//TODO: 添加用户代码
	current_screen_id = screen_id;//在工程配置中开启画面切换通知，记录当前画面ID

	if(current_screen_id==4)//温度曲线
	{
		Uint16 i = 0;
		Uint8 dat[100] = {0};

		//生成方波
		for (i=0;i<100;++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//添加数据到通道0

		//生成锯齿波
		for (i=0;i<100;++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//添加数据到通道1
	}
	else if(current_screen_id==9)//二维码
	{
		//二维码控件显示中文字符时，需要转换为UTF8编码，
		//通过“指令助手”，转换“广州大彩123” ，得到字符串编码如下
		Uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
}

/*! 
 *  \brief  触摸坐标事件响应
 *  \param press 1按下触摸屏，3松开触摸屏
 *  \param x x坐标
 *  \param y y坐标
 */
void NotifyTouchXY(Uint8 press,Uint16 x,Uint16 y)
{
	//TODO: 添加用户代码
}

void SetTextValueInt32(Uint16 screen_id, Uint16 control_id,Sint32 value)
{
	Uint8 buffer[12] = {0};
	sprintf(buffer,"%d",value); //把整数转换为字符串
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(Uint16 screen_id, Uint16 control_id,float value)
{
	Uint8 buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//把浮点数转换为字符串(保留一位小数)
	SetTextValue(screen_id,control_id,buffer);
}

void UpdateUI()
{
	if(current_screen_id==2)//文本设置和显示
	{
		//当前电流、温度从0到100循环显示，艺术字从0.0-99.9循环显示
		SetTextValueInt32(2,5,test_value%100);//当前电流
		SetTextValueInt32(2,6,test_value%100);//温度
		SetTextValueFloat(2,7,(test_value%1000)/10.0);//艺术字

		++test_value;
	}
	else if(current_screen_id==5)//进度条和滑块控制
	{
		SetProgressValue(5,1,test_value%100);

		++test_value;
	}
	else if(current_screen_id==6)//仪表控件
	{
		SetMeterValue(6,1,test_value%360);
		SetMeterValue(6,2,test_value%360);

		++test_value;
	}
}

/*! 
 *  \brief  按钮控件通知
 *  \details  当按钮状态改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param state 按钮状态：0弹起，1按下
 */
void NotifyButton(Uint16 screen_id, Uint16 control_id, Uint8  state)
{
	//TODO: 添加用户代码
	if(screen_id==3)//按钮、图标、动画控制
	{
		if(control_id==3)//运行按钮
		{
			if(state==0)//停止运行
			{				
				AnimationPlayFrame(3,1,1);//显示停止图标
				AnimationStop(3,2);//动画停止播放
			}
			else//开始运行
			{				
				SetControlVisiable(3,1,1);//显示图标
				SetControlVisiable(3,2,1);//显示动画

				AnimationPlayFrame(3,1,0);//显示运行图标
				AnimationStart(3,2);//动画开始播放
			}		
		}
		else if(control_id==4)//复位按钮
		{
			SetControlVisiable(3,1,0);//隐藏图标
			SetControlVisiable(3,2,0);//隐藏动画
			SetButtonValue(3,3,0);//显示开始运行
		}
	}
}

/*! 
 *  \brief  文本控件通知
 *  \details  当文本通过键盘更新(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param str 文本控件内容
 */
void NotifyText(Uint16 screen_id, Uint16 control_id, Uint8 *str)
{
	//TODO: 添加用户代码
	int32 value = 0; 

	if(screen_id==2)//画面ID2：文本设置和显示
	{
		sscanf(str,"%d",&value);//把字符串转换为整数

		if(control_id==1)//最高电压
		{
			//限定数值范围（也可以在文本控件属性中设置）
			if(value<0)
				value = 0;
			else if(value>380)
				value = 380;

			SetTextValueInt32(2,1,value);  //更新最高电压
			SetTextValueInt32(2,4,value/2);  //更新最高电压/2
		}
	}
}

/*! 
 *  \brief  进度条控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyProgress(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  滑动条控件通知
 *  \details  当滑动条改变(或调用GetControlValue)时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifySlider(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: 添加用户代码
	if(screen_id==5&&control_id==2)//滑块控制
	{
		test_value = value;

		SetProgressValue(5,1,test_value); //更新进度条数值
	}
}

/*! 
 *  \brief  仪表控件通知
 *  \details  调用GetControlValue时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param value 值
 */
void NotifyMeter(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  菜单控件通知
 *  \details  当菜单项按下或松开时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 菜单项索引
 *  \param state 按钮状态：0松开，1按下
 */
void NotifyMenu(Uint16 screen_id, Uint16 control_id, Uint8  item, Uint8  state)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  选择控件通知
 *  \details  当选择控件变化时，执行此函数
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 *  \param item 当前选项
 */
void NotifySelector(Uint16 screen_id, Uint16 control_id, Uint8  item)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  定时器超时通知处理
 *  \param screen_id 画面ID
 *  \param control_id 控件ID
 */
void NotifyTimer(Uint16 screen_id, Uint16 control_id)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  读取用户FLASH状态返回
 *  \param status 0失败，1成功
 *  \param _data 返回数据
 *  \param length 数据长度
 */
void NotifyReadFlash(Uint8 status,Uint8 *_data,Uint16 length)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  写用户FLASH状态返回
 *  \param status 0失败，1成功
 */
void NotifyWriteFlash(Uint8 status)
{
	//TODO: 添加用户代码
}

/*! 
 *  \brief  读取RTC时间，注意返回的是BCD码
 *  \param year 年（BCD）
 *  \param month 月（BCD）
 *  \param week 星期（BCD）
 *  \param day 日（BCD）
 *  \param hour 时（BCD）
 *  \param minute 分（BCD）
 *  \param second 秒（BCD）
 */
void NotifyReadRTC(Uint8 year,Uint8 month,Uint8 week,Uint8 day,Uint8 hour,Uint8 minute,Uint8 second)
{
}


void HandleWarnningState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data);
void HandleSettingState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data);
void HandleMainSreenState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data);
void HandleClockStatusState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data);

typedef void (*HandleWigetState)(struct root_data *,Uint16, Uint16,unsigned char *);
HandleWigetState HandleLcdEvent = HandleMainSreenState;

void HandleWarnningState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
    printf("HandleWarnningState\n");
    
    switch(control_id)
    {
        case 21:
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            
            HandleLcdEvent = HandleMainSreenState;
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);
            break;

        default:
            HandleLcdEvent = HandleWarnningState;
            break;
    }

}

void HandleCtlEthState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
    struct clock_info *pClockInfo = &pRootData->clock_info;

    switch(control_id)
    {
        case 19:
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            
            HandleLcdEvent = HandleMainSreenState;
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);

            
            break;
        case 21:
            pRootData->lcd_sreen_id = PARAM_SETING_SCREEN_ID;
            HandleLcdEvent = HandleSettingState;
            
            if(pClockInfo->ref_type == 0)
            {
                SetButtonValue(PARAM_SETING_SCREEN_ID,0x0a,0x01);
                SetButtonValue(PARAM_SETING_SCREEN_ID,0x0ba,0x001);
            
            }
            else
            {
                SetButtonValue(PARAM_SETING_SCREEN_ID,0x0b,0x01);
                SetButtonValue(PARAM_SETING_SCREEN_ID,0x0a,0x00);
            
            }

        case 0x04:
            
            break;
        case 0x07:
            
            break;
        default:
            HandleLcdEvent = HandleCtlEthState;
            break;
    }



}
void HandlePtpEthState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{

}
void HandleNtpEthState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{

}

void lcdDisplayEnternet(struct NetInfor *net,Uint16 screen_id)
{
    struct sockaddr_in temsock;
    char data[20];

    temsock.sin_addr.s_addr = net->ip;
    SetTextValue(screen_id,4,inet_ntoa(temsock.sin_addr));
    temsock.sin_addr.s_addr = net->gwip;
    SetTextValue(screen_id,7,inet_ntoa(temsock.sin_addr));

    memset(data,0,sizeof(data));
    sprintf(data,"%02x:%02x:%02x:%02x:%02x:%02x",net->mac[0],net->mac[1],net->mac[2]
                            ,net->mac[3],net->mac[4],net->mac[5]);
    SetTextValue(screen_id,10,data);
}

void HandleSettingState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
    printf("HandleSettingState\n");


    switch(control_id)
    {
        case 21:
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            
            HandleLcdEvent = HandleMainSreenState;
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);
            break;
        case 7:
            pRootData->lcd_sreen_id = ETH_CTL_SCREEN_ID;
            lcdDisplayEnternet(&pRootData->comm_port,ETH_CTL_SCREEN_ID);
            HandleLcdEvent = HandleCtlEthState;
            break;
        case 12:
            pRootData->lcd_sreen_id = ETH_PTP_SCREEN_ID;
            lcdDisplayEnternet(&pRootData->ptp_port,ETH_PTP_SCREEN_ID);
            HandleLcdEvent = HandlePtpEthState;
            break;
        case 15:
            pRootData->lcd_sreen_id = ETH_NTP_SCREEN_ID;
            lcdDisplayEnternet(&pRootData->ntp_port,ETH_NTP_SCREEN_ID);
            HandleLcdEvent = HandleNtpEthState;
            break;
            
        default:
            HandleLcdEvent = HandleSettingState;
            break;
    }


}

void HandleClockStatusState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{
    printf("HandleClockStatusState\n");

    switch(control_id)
    {
        case 21:
            pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
            
            HandleLcdEvent = HandleMainSreenState;
            SetTextValue(MAIN_SCREEN_ID,4,pRootData->current_time);
            break;

        default:
            HandleLcdEvent = HandleClockStatusState;
            break;
    }


}


void HandleMainSreenState(struct root_data *pRootData,Uint16 screen_id, Uint16 control_id,unsigned char *data)
{

        printf("HandleMainSreenState\n");
        struct clock_info *pClockInfo = &pRootData->clock_info;
        
        switch(control_id)
        {
            case 12:
                pRootData->lcd_sreen_id = CLOCK_STATUS_SCREEN_ID;
                
                HandleLcdEvent = HandleClockStatusState;
                break;
            case 13:
                pRootData->lcd_sreen_id = PARAM_SETING_SCREEN_ID;
                
                HandleLcdEvent = HandleSettingState;
                
                if(pClockInfo->ref_type == 0)
                {
                    SetButtonValue(PARAM_SETING_SCREEN_ID,0x0a,0x01);
                    SetButtonValue(PARAM_SETING_SCREEN_ID,0x0ba,0x001);

                }
                else
                {
                    SetButtonValue(PARAM_SETING_SCREEN_ID,0x0b,0x01);
                    SetButtonValue(PARAM_SETING_SCREEN_ID,0x0a,0x00);

                }
                
                
                break;
            case 14:
                pRootData->lcd_sreen_id = WARN_SCREEN_ID;
                
                HandleLcdEvent = HandleWarnningState;
                break;
            default:
                pRootData->lcd_sreen_id = MAIN_SCREEN_ID;
                HandleLcdEvent = HandleMainSreenState;
                break;
        }
}



void ProcessLcdMessage(struct root_data *pRootData,char *buf,Uint16 len)
{
    struct LcdCtlMsg *msg = (struct LcdCtlMsg *)buf;
    
	Uint8 cmd_type = msg->cmd_type;             //指令类型
	Uint8 ctrl_msg = msg->ctrl_msg;             //消息的类型
	Uint8 control_type = msg->control_type;     //控件类型
	Uint16 screen_id = PTR2U16(&msg->screen_id);//画面ID
	Uint16 control_id = PTR2U16(&msg->control_id);//控件ID
	Uint32 value = PTR2U32(msg->param);             //数值

    printf("%x %x %x %x %x\n",cmd_type,ctrl_msg,control_type,screen_id,value);
	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://触摸屏按下
	case NOTIFY_TOUCH_RELEASE://触摸屏松开
		NotifyTouchXY(buf[1],PTR2U16(buf+2),PTR2U16(buf+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://写FLASH成功
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://写FLASH失败
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://读取FLASH成功
		NotifyReadFlash(1,buf+2,len-6);//去除帧头帧尾
		break;
	case NOTIFY_READ_FLASH_FAILD://读取FLASH失败
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://读取RTC时间
		NotifyReadRTC(buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//画面ID变化通知
			{
				NotifyScreen(screen_id);
			}
            else if(ctrl_msg == MSG_GET_DATA)
            {
                
                (*HandleLcdEvent)(pRootData,screen_id,control_id,msg->param);
            }
			else
			{
				switch(control_type)
				{
				case kCtrlButton: //按钮控件
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
				case kCtrlText://文本控件
					NotifyText(screen_id,control_id,msg->param);
					break;
				case kCtrlProgress: //进度条控件
					NotifyProgress(screen_id,control_id,value);
					break;
				case kCtrlSlider: //滑动条控件
					NotifySlider(screen_id,control_id,value);
					break;
				case kCtrlMeter: //仪表控件
					NotifyMeter(screen_id,control_id,value);
					break;
				case kCtrlMenu://菜单控件
					NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
					break;
				case kCtrlSelector://选择控件
					NotifySelector(screen_id,control_id,msg->param[0]);
					break;
				case kCtrlRTC://倒计时控件
					NotifyTimer(screen_id,control_id);
					break;
				default:
					break;
				}
			}			
		}
		break;
	default:
		break;
	}
}


