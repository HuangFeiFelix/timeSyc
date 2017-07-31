#include "lcd_driver.h"
#include "lcd_cmd_process.h"
#include "clock_rb.h"

static Uint16 current_screen_id = 0;//��ǰ����ID
static Sint32 test_value = 0;//����ֵ
static Uint8 update_en = 0;//���±��




/*! 
 *  \brief  �����л�֪ͨ
 *  \details  ��ǰ����ı�ʱ(�����GetScreen)��ִ�д˺���
 *  \param screen_id ��ǰ����ID
 */
void NotifyScreen(Uint16 screen_id)
{
	//TODO: ����û�����
	current_screen_id = screen_id;//�ڹ��������п��������л�֪ͨ����¼��ǰ����ID

	if(current_screen_id==4)//�¶�����
	{
		Uint16 i = 0;
		Uint8 dat[100] = {0};

		//���ɷ���
		for (i=0;i<100;++i)
		{
			if((i%20)>=10)
				dat[i] = 200;
			else
				dat[i] = 20;
		}
		GraphChannelDataAdd(4,1,0,dat,100);//������ݵ�ͨ��0

		//���ɾ�ݲ�
		for (i=0;i<100;++i)
		{
			dat[i] = 16*(i%15);
		}
		GraphChannelDataAdd(4,1,1,dat,100);//������ݵ�ͨ��1
	}
	else if(current_screen_id==9)//��ά��
	{
		//��ά��ؼ���ʾ�����ַ�ʱ����Ҫת��ΪUTF8���룬
		//ͨ����ָ�����֡���ת�������ݴ��123�� ���õ��ַ�����������
		Uint8 dat[] = {0xE5,0xB9,0xBF,0xE5,0xB7,0x9E,0xE5,0xA4,0xA7,0xE5,0xBD,0xA9,0x31,0x32,0x33};
		SetTextValue(9,1,dat);
	}
}

/*! 
 *  \brief  ���������¼���Ӧ
 *  \param press 1���´�������3�ɿ�������
 *  \param x x����
 *  \param y y����
 */
void NotifyTouchXY(Uint8 press,Uint16 x,Uint16 y)
{
	//TODO: ����û�����
}

void SetTextValueInt32(Uint16 screen_id, Uint16 control_id,Sint32 value)
{
	Uint8 buffer[12] = {0};
	sprintf(buffer,"%d",value); //������ת��Ϊ�ַ���
	SetTextValue(screen_id,control_id,buffer);
}

void SetTextValueFloat(Uint16 screen_id, Uint16 control_id,float value)
{
	Uint8 buffer[12] = {0};
	sprintf(buffer,"%.1f",value);//�Ѹ�����ת��Ϊ�ַ���(����һλС��)
	SetTextValue(screen_id,control_id,buffer);
}

void UpdateUI()
{
	if(current_screen_id==2)//�ı����ú���ʾ
	{
		//��ǰ�������¶ȴ�0��100ѭ����ʾ�������ִ�0.0-99.9ѭ����ʾ
		SetTextValueInt32(2,5,test_value%100);//��ǰ����
		SetTextValueInt32(2,6,test_value%100);//�¶�
		SetTextValueFloat(2,7,(test_value%1000)/10.0);//������

		++test_value;
	}
	else if(current_screen_id==5)//�������ͻ������
	{
		SetProgressValue(5,1,test_value%100);

		++test_value;
	}
	else if(current_screen_id==6)//�Ǳ�ؼ�
	{
		SetMeterValue(6,1,test_value%360);
		SetMeterValue(6,2,test_value%360);

		++test_value;
	}
}

/*! 
 *  \brief  ��ť�ؼ�֪ͨ
 *  \details  ����ť״̬�ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param state ��ť״̬��0����1����
 */
void NotifyButton(Uint16 screen_id, Uint16 control_id, Uint8  state)
{
	//TODO: ����û�����
	if(screen_id==3)//��ť��ͼ�ꡢ��������
	{
		if(control_id==3)//���а�ť
		{
			if(state==0)//ֹͣ����
			{				
				AnimationPlayFrame(3,1,1);//��ʾֹͣͼ��
				AnimationStop(3,2);//����ֹͣ����
			}
			else//��ʼ����
			{				
				SetControlVisiable(3,1,1);//��ʾͼ��
				SetControlVisiable(3,2,1);//��ʾ����

				AnimationPlayFrame(3,1,0);//��ʾ����ͼ��
				AnimationStart(3,2);//������ʼ����
			}		
		}
		else if(control_id==4)//��λ��ť
		{
			SetControlVisiable(3,1,0);//����ͼ��
			SetControlVisiable(3,2,0);//���ض���
			SetButtonValue(3,3,0);//��ʾ��ʼ����
		}
	}
}

/*! 
 *  \brief  �ı��ؼ�֪ͨ
 *  \details  ���ı�ͨ�����̸���(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param str �ı��ؼ�����
 */
void NotifyText(Uint16 screen_id, Uint16 control_id, Uint8 *str)
{
	//TODO: ����û�����
	int32 value = 0; 

	if(screen_id==2)//����ID2���ı����ú���ʾ
	{
		sscanf(str,"%d",&value);//���ַ���ת��Ϊ����

		if(control_id==1)//��ߵ�ѹ
		{
			//�޶���ֵ��Χ��Ҳ�������ı��ؼ����������ã�
			if(value<0)
				value = 0;
			else if(value>380)
				value = 380;

			SetTextValueInt32(2,1,value);  //������ߵ�ѹ
			SetTextValueInt32(2,4,value/2);  //������ߵ�ѹ/2
		}
	}
}

/*! 
 *  \brief  �������ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyProgress(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  �������ؼ�֪ͨ
 *  \details  ���������ı�(�����GetControlValue)ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifySlider(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: ����û�����
	if(screen_id==5&&control_id==2)//�������
	{
		test_value = value;

		SetProgressValue(5,1,test_value); //���½�������ֵ
	}
}

/*! 
 *  \brief  �Ǳ�ؼ�֪ͨ
 *  \details  ����GetControlValueʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param value ֵ
 */
void NotifyMeter(Uint16 screen_id, Uint16 control_id, Uint32 value)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  �˵��ؼ�֪ͨ
 *  \details  ���˵���»��ɿ�ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item �˵�������
 *  \param state ��ť״̬��0�ɿ���1����
 */
void NotifyMenu(Uint16 screen_id, Uint16 control_id, Uint8  item, Uint8  state)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ѡ��ؼ�֪ͨ
 *  \details  ��ѡ��ؼ��仯ʱ��ִ�д˺���
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 *  \param item ��ǰѡ��
 */
void NotifySelector(Uint16 screen_id, Uint16 control_id, Uint8  item)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ʱ����ʱ֪ͨ����
 *  \param screen_id ����ID
 *  \param control_id �ؼ�ID
 */
void NotifyTimer(Uint16 screen_id, Uint16 control_id)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ȡ�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 *  \param _data ��������
 *  \param length ���ݳ���
 */
void NotifyReadFlash(Uint8 status,Uint8 *_data,Uint16 length)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  д�û�FLASH״̬����
 *  \param status 0ʧ�ܣ�1�ɹ�
 */
void NotifyWriteFlash(Uint8 status)
{
	//TODO: ����û�����
}

/*! 
 *  \brief  ��ȡRTCʱ�䣬ע�ⷵ�ص���BCD��
 *  \param year �꣨BCD��
 *  \param month �£�BCD��
 *  \param week ���ڣ�BCD��
 *  \param day �գ�BCD��
 *  \param hour ʱ��BCD��
 *  \param minute �֣�BCD��
 *  \param second �루BCD��
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
    
	Uint8 cmd_type = msg->cmd_type;             //ָ������
	Uint8 ctrl_msg = msg->ctrl_msg;             //��Ϣ������
	Uint8 control_type = msg->control_type;     //�ؼ�����
	Uint16 screen_id = PTR2U16(&msg->screen_id);//����ID
	Uint16 control_id = PTR2U16(&msg->control_id);//�ؼ�ID
	Uint32 value = PTR2U32(msg->param);             //��ֵ

    printf("%x %x %x %x %x\n",cmd_type,ctrl_msg,control_type,screen_id,value);
	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://����������
	case NOTIFY_TOUCH_RELEASE://�������ɿ�
		NotifyTouchXY(buf[1],PTR2U16(buf+2),PTR2U16(buf+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://дFLASH�ɹ�
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://дFLASHʧ��
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://��ȡFLASH�ɹ�
		NotifyReadFlash(1,buf+2,len-6);//ȥ��֡ͷ֡β
		break;
	case NOTIFY_READ_FLASH_FAILD://��ȡFLASHʧ��
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://��ȡRTCʱ��
		NotifyReadRTC(buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//����ID�仯֪ͨ
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
				case kCtrlButton: //��ť�ؼ�
					NotifyButton(screen_id,control_id,msg->param[1]);
					break;
				case kCtrlText://�ı��ؼ�
					NotifyText(screen_id,control_id,msg->param);
					break;
				case kCtrlProgress: //�������ؼ�
					NotifyProgress(screen_id,control_id,value);
					break;
				case kCtrlSlider: //�������ؼ�
					NotifySlider(screen_id,control_id,value);
					break;
				case kCtrlMeter: //�Ǳ�ؼ�
					NotifyMeter(screen_id,control_id,value);
					break;
				case kCtrlMenu://�˵��ؼ�
					NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);
					break;
				case kCtrlSelector://ѡ��ؼ�
					NotifySelector(screen_id,control_id,msg->param[0]);
					break;
				case kCtrlRTC://����ʱ�ؼ�
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


