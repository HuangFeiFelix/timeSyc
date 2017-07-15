/********************************************************************************************
*                           ��Ȩ����(C) 2015,*                                 ��Ȩ����
*********************************************************************************************
*    �� �� ��:       comm_fun.c
*    ��������:       ���ú���
*    ����:           HuangFei
*    �汾��:         v1.0
*    ������:         arm-linux-gcc
*    ��������:       2015-1-30
*    �����б�:
                     Caculate_frequency
                     Caculate_Second
                     Display_NetInfo
                     Get_MessageInterval
                     Get_SerialChar
                     Get_SerialString
*********************************************************************************************
*    ��ע:            
*    �޸���ʷ:
                   1.����: 2015-1-30
                     ����: HuangFei
                     �޸�����: �´����ļ�

                   2.����: 2015-2-6
                     ����: HuangFei
                     �޸�����: ������ʾ����

*********************************************************************************************/


#include "comm_fun.h"
#include <sys/select.h>



/********************************************************************************************
* ��������:    Get_SerialString
* ��������:     �ӱ�׼�����ȡ1�ַ���������string����
* �������:    Uint8*string    
               Uint16 str_len  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint8 Get_SerialString(Uint8*string,Uint16 str_len)
{
    
    Uint8 input[50];
    int len = 0;

    memset(input,0,sizeof(input));
    
    len = read(0,input,50);
    if(len == 1)
    {
        string[0] = input[0];
        return len;
    }
    if(len >= str_len)
    {
        memcpy(string,input,str_len);
        return str_len;
    }
    else
    {
        memcpy(string,input,len);
        string[len-1] = '\0';

        return len;
    }
}

/********************************************************************************************
* ��������:    Get_SerialChar
* ��������:    �ӱ�׼�����ȡ1���ַ�
* �������:    ��
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint8 Get_SerialChar()
{
    Uint8 input[200];
    int len = 0;

    memset(input,0,sizeof(input));
    
    len = read(0,input,200);

    if(len == 1)
    {
        return input[0];
    }
    else if(len == 2)
    {
        if((input[1] == 0x0a) || ( input[1] == 0x0d))
        {
            return input[0];
        }
        else
        {
            return 0xff;
        }
    }
    else
    {
        return 0xff;    
    }
}


/********************************************************************************************
* ��������:    Caculate_frequency
* ��������:    ����ಥ���㲥�ķ������ڣ�����ntpd��ʽ
* �������:    Uint32 input  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint32 Caculate_frequency(Uint32 input)
{
    Uint32 i = 0;
    while(input)
    {
        input = input / 2;
        i++;
    }
    return i -1;
}


/********************************************************************************************
* ��������:    Caculate_Second
* ��������:     �õ��㲥���ಥ������������
* �������:    Uint32 input  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint32 Caculate_Second(Uint32 input)
{
    Uint16 i;
    Uint32 second =1;

    for(i = 0;i<input;i++)
        second = second * 2;

    return second;
}


/********************************************************************************************
* ��������:    Get_MessageInterval
* ��������:    ���ݷ��͵�loginterval ��õķ��ͼ���ֵ
* �������:    Uint8 Interval  
* �������:    ��
* ����ֵ:      
* ��ע:        �Ƿ������
* �޸���ʷ:
             1.����:     2015-2-6
               ����:     HuangFei
               �޸�����: �����ɺ���

********************************************************************************************/
Uint32 Get_MessageInterval(Sint8 Interval)
{
    Uint32 synRate = 0;
    
    switch (Interval)
    {
        case 4:
            synRate = 16 * 256;
            break;
        case 3:
            synRate = 8 * 256;
            break;
        case 2:
            synRate = 4 * 256;
            break;
        case 1:
            synRate = 2 * 256;
            break;
        case 0:
            synRate = 1 * 256;
            break;
        case -1:
            synRate = 256/2;
            break;
        case -2:
            synRate = 256/4;
            break;
        case -3:
            synRate = 256/8;
            break;
            
        case -4:
            synRate = 256/16;
            break;
        case -5:
            synRate = 256/32;
            break;
        case -6:
            synRate = 256/64;
            break;  
        case -7:
            synRate = 256/128;
            break;

        case -8:
            synRate = 1;
            break;

        default:
            synRate = 1 * 256;
            break;
        
    }
    return synRate;
}

Uint8 ValidtateNum(char *input,int len,Uint32 *pNum)
{
     
    int i;

    
    if((len == 1) && (input[0] == 0x0a || input[0] == 0x0d))/** ��ʾ������ǻس�,����2 */
    {
        return 2;
    }
    else if(len == 2)/** ����0-9 */ 
    {
        if((input[0] > '9') || (input[0] < '0'))
            return FALSE;
        *pNum = input[0] - '0';
    }
    else
    {
        if(input[0] == '0')
        {
            return FALSE;
        }

        for(i = 0;i < len-1;i++)
        {
            if((input[i] > '9') || (input[i] < '0'))
            {   
                return FALSE;
            }
        }
        *pNum = atoi(input);
    }
    
    return TRUE;
}


Uint8 ValidtateIntervalNum(char *input,int len,short *pNum)
{
    int i;

    /** ��ʾ������ǻس�,����2 */
    if((len == 1) && (input[0] == 0x0a || input[0] == 0x0d))
    {
        return 2;
    }
    else if(len == 2)/** ����0-9 */ 
    {
        if((input[0] > '9') || (input[0] < '0'))
            return FALSE;
        *pNum = input[0] - '0';
    }
    else
    {
        if(input[0] == '0')
            return FALSE;
        
        if(input[0] == '-')
        {
            for(i = 1;i < len-1;i++)
            {
            
                if((input[i] > '9') || (input[i] < '0'))
                {   
                    return FALSE;
                }
            }
            
            *pNum = atoi(input);

        }
        else
        {
            for(i = 0;i < len-1;i++)
            {
            
                if((input[i] > '9') || (input[i] < '0'))
                {   
                    return FALSE;
                }
            }
            *pNum = atoi(input);
        }
    }

    return TRUE;

}


