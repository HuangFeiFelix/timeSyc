#include "lcd_driver.h"
#include "lcd_cmd_process.h"

Uint8 rx_cmd_buffer[CMD_MAX_SIZE];//ָ���

static Uint16 current_screen_id = 0;//��ǰ����ID

static Sint32 test_value = 0;//����ֵ
static Uint8 update_en = 0;//���±��

void ProcessMessage( PCTRL_MSG msg, Uint16 size )
{
	Uint8 cmd_type = msg->cmd_type;//ָ������
	Uint8 ctrl_msg = msg->ctrl_msg;   //��Ϣ������
	Uint8 control_type = msg->control_type;//�ؼ�����
	Uint16 screen_id = PTR2U16(&msg->screen_id);//����ID
	Uint16 control_id = PTR2U16(&msg->control_id);//�ؼ�ID
	Uint32 value = PTR2U32(msg->param);//��ֵ

	switch(cmd_type)
	{		
	case NOTIFY_TOUCH_PRESS://����������
	case NOTIFY_TOUCH_RELEASE://�������ɿ�
		NotifyTouchXY(rx_cmd_buffer[1],PTR2U16(rx_cmd_buffer+2),PTR2U16(rx_cmd_buffer+4));
		break;	
	case NOTIFY_WRITE_FLASH_OK://дFLASH�ɹ�
		NotifyWriteFlash(1);
		break;
	case NOTIFY_WRITE_FLASH_FAILD://дFLASHʧ��
		NotifyWriteFlash(0);
		break;
	case NOTIFY_READ_FLASH_OK://��ȡFLASH�ɹ�
		NotifyReadFlash(1,rx_cmd_buffer+2,size-6);//ȥ��֡ͷ֡β
		break;
	case NOTIFY_READ_FLASH_FAILD://��ȡFLASHʧ��
		NotifyReadFlash(0,0,0);
		break;
	case NOTIFY_READ_RTC://��ȡRTCʱ��
		NotifyReadRTC(rx_cmd_buffer[1],rx_cmd_buffer[2],rx_cmd_buffer[3],rx_cmd_buffer[4],rx_cmd_buffer[5],rx_cmd_buffer[6],rx_cmd_buffer[7]);
		break;
	case NOTIFY_CONTROL:
		{
			if(ctrl_msg==MSG_GET_CURRENT_SCREEN)//����ID�仯֪ͨ
			{
				NotifyScreen(screen_id);
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
