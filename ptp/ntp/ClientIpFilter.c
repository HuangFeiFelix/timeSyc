/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 ClientIpFilter.h
*	Purpose:	     
*	Author:		 Felix(HuangFei)
*	Version:	  	 v01.00
*	Compiler:	 arm-linux-gcc
*	Created on:	 2014-9-23
*******************************************************************************
*	Note:
*	Revision history:  
*                       Created by hf    2014-9-23
******************************************************************************/

#include "ClientIpFilter.h"
#include "NetWorkConfig.h"
#include "comm_fun.h"

NTP_PARAMETER g_Ntp_Parameter; /* NTP 参数控制全局结构体 */
NtpDevice g_NtpDevice;


extern void *Ntp_Manger_State(void);

/**********************************************************************
 * Function:      Filter_Mode_Contrl
 * Description:   黑白名单开启或关闭控制
 * Input:          void
 * Return:        成功1；失败0
 * Others:        
**********************************************************************/
Uint8 Filter_Mode_Contrl()
{
    Uint8 ret = 0;

    Uint8 str_len = 0;
    Uint8 input;
    
    while(1)
    {
        printf("Input mode (B/W/N) B: blacklist; W: whitelist; N: both disabled \r\n");
        printf("Or Press ENTER to return \\>\r\n");

        input = Get_SerialChar();
            
        if(input == 'B' || input == 'b')
        {
            g_Ntp_Parameter.blacklist=1;
			g_Ntp_Parameter.whitelist=0;
            printf("Successfully Enable Blacklist\n");
            ret = 1;
            break;
        }
        else if(input == 'W' || input == 'w')
        {
            g_Ntp_Parameter.blacklist=0;
			g_Ntp_Parameter.whitelist=1;
            printf("Successfully Enable Whitelist\n");
            ret = 1;
            break;
        }
        else if(input == 'N' || input == 'n')
        {
            g_Ntp_Parameter.blacklist=0;
			g_Ntp_Parameter.whitelist=0;
            printf("Successfully Disable both List \n");
            ret =1;
            break;

        }
        else if(input == 0x0a || input == 0x0d)
        {
            break;
        }
        else
        {
            printf("Invalid number, input again!\r\n");
        }
        
    }

    
    return ret;
}

/**********************************************************************
 * Function:      Display_List
 * Description:   打印黑名单或黑面但列表
 * Input:         type：类型1=黑名单，0=白名单,flag_list：有效列表,
 *				  ip_list：ip列表,mask_list：子网掩码列表
 * Return:        
 * Others:        
**********************************************************************/
void Display_List(Uint8 type,Uint8*flag_list,Uint32 *ip_list,Uint32 *mask_list)
{
    int i;
    if(type)
    {
        printf("------------------\r\n");
        printf("Blacklist:\r\n");
        printf("------------------\r\n");
    }

    else
    {
        printf("------------------\r\n");
        printf("whitelist:\r\n");
        printf("------------------\r\n");
    }

    
    for(i=0;i<16;i++)
	{
    	if(flag_list[i]==1)
    	{ 
        	printf("%d: ",(i+1));
        	printf("IP=%d.%d.%d.%d\t",(ip_list[i]&0xff),((ip_list[i]&0xff00)>>8),((ip_list[i]&0xff0000)>>16),((ip_list[i]&0xff000000)>>24));
    	      	
    	if(mask_list[i]==0xffffffff)
    	{
    	    printf("Mask=None\n");
    	}
    	else
    	    printf("Mask=%d.%d.%d.%d\n",(mask_list[i]&0xff),((mask_list[i]&0xff00)>>8),((mask_list[i]&0xff0000)>>16),((mask_list[i]&0xff000000)>>24));
    	}
	}

    printf("\r\n");
    
}

/**********************************************************************
 * Function:     Delete_List 
 * Description:   删除所有列表内容
 * Input:         flag_list：有效列表,ip_list：ip列表，mask_list：子网掩码列表
 * Return:        void
 * Others:        
**********************************************************************/
void Delete_List(Uint8*flag_list,Uint32 *ip_list,Uint32 *mask_list)
{
    int i;
    
    for(i=0;i<16;i++)
	{
    	flag_list[i]=0;
		ip_list[i]=0;
		mask_list[i]=0;
	}
    printf("List deleted!!\r\n");
}

/**********************************************************************
 * Function:      Filter_Edit_Blacklist
 * Description:   配置黑名单
 * Input:          void
 * Return:        成功1，失败0
 * Others:        
**********************************************************************/
Uint8 Filter_Edit_Blacklist()
{
    Uint8 ret = 0;
    Uint8 input;
    Uint32 num = 0;
    Uint8 i;
    Uint8 str_input[50];
    Uint8 len;
    
    while(1)
    {
        Display_List(1,g_Ntp_Parameter.blacklist_flag,g_Ntp_Parameter.blacklist_ip,g_Ntp_Parameter.blacklist_mask);
         /* 删除所有列表或者选择要编辑的列表*/
        printf("Press D to delete all lists\r\n");
        printf("Input number N to Select num or Press ENTER to retrun: (N range:1,2,3...16)\r\n\\>");

        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));


        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateNum(str_input,len,&num);

        if(ret == FALSE)
        {
            if((len == 2) && (str_input[1] == 'd' || str_input[1] == 'D'))
            {
                Delete_List(g_Ntp_Parameter.blacklist_flag,g_Ntp_Parameter.blacklist_ip,g_Ntp_Parameter.blacklist_mask);
                continue;
            }
            else
            {
                printf("Invalid number, input again! Value Must Be 1~16 !!\r\n\r\n");
                continue;

            }
        }
        else
        {
            if(num < 0 || num > 16)
            {
                printf("Invalid number, input again! Value Must Be 1~16 !!\r\n\r\n");
                continue;

            }
            else
            {
                num = num -1;
                printf("Input Success\n");
                break;

            }

        }
               
        /*输入ip地址*/
        while(1)
        {
            printf("Input blacklist IP or Press ENTER to retrun(e.g. 192.168.15.25)\r\n\\>");
            memset(str_input,0,sizeof(str_input));
            
            Get_SerialString(str_input,sizeof(str_input));

            if(str_input[0]== 0x0d || str_input[0] == 0x0a)
            {
                break;
            }
            else if(!VerifyIpAddress(str_input))
    		{
    			printf("\aInvalid address!\r\n\r\n");
    		}
            else
            {
                g_Ntp_Parameter.blacklist_flag[num]=TRUE;                              
    			g_Ntp_Parameter.blacklist_ip[num]= inet_addr(str_input);
                ret = 1;
                break;
            }
        }

        /*输入子网掩码*/
        while(1)
        {
            printf("Input Subnet Mask or input N(None) or Press ENTER to retrun \r\n(e.g. 255.255.255.0, range:255.255.0.0~255.255.255.252)\r\n\\>");
            memset(str_input,0,sizeof(str_input));
            len = Get_SerialString(str_input,sizeof(str_input));

            if(len == 1)
            {
                g_Ntp_Parameter.blacklist_mask[num] = 0xffffffff;
                ret = 1;
                break;
            }
            else if(len == 2)
            {
                if((str_input[0] == 0x4E) || (str_input[0] == 0x6E))
                {
                    g_Ntp_Parameter.blacklist_mask[num] = 0xffffffff;
                    ret = 1;
                    break;
                }
                else
                {
                    printf("\aInvalid Mask address!\r\n\r\n");
                }
            }
            else
            {
                if(!VerifyNetMastAddress(str_input))
        		{
        			printf("\aInvalid Mask address!\r\n\r\n");
        		}
                else
                {
        			g_Ntp_Parameter.blacklist_mask[num]= inet_addr(str_input);
                    ret = 1;
                    break;
                }
            }
        }
    
    }
   
    return ret;
}

/**********************************************************************
 * Function:      Filter_Edit_Whitlist
 * Description:   配置白名单
 * Input:          void
 * Return:        成功1，失败0
 * Others:        
**********************************************************************/
Uint8 Filter_Edit_Whitlist()
{
    Uint8 ret = 0;
    Uint8 input;
    Uint32 num = 0;
    Uint8 i;
    Uint8 str_input[50];
    Uint8 len;

    
    while(1)
    {
        
        Display_List(0,g_Ntp_Parameter.whitelist_flag,g_Ntp_Parameter.whitelist_ip,g_Ntp_Parameter.whitelist_mask);
        /* 删除所有列表或者选择要编辑的列表*/
        printf("Press D to delete all lists\r\n");
        printf("Input number N to Select num or Press ENTER to retrun: (N range:1,2,3...16)\r\n\\>");

        
        memset(str_input,0,sizeof(str_input));
        len = Get_SerialString(str_input,sizeof(str_input));

        if(str_input[0] == 0x0a || str_input[0] == 0x0d)
        {
            break;
        }

        ret = ValidtateNum(str_input,len,&num);

        if(ret == FALSE)
        {
            if((len == 2) && (str_input[1] == 'd' || str_input[1] == 'D'))
            {
                Delete_List(g_Ntp_Parameter.whitelist_flag,g_Ntp_Parameter.whitelist_ip,g_Ntp_Parameter.whitelist_mask);
                continue;
            }
            else
            {
                printf("Invalid number, input again! Value Must Be 1~16 !!\r\n\r\n");
                continue;

            }
        }
        else
        {
            if(num < 0 || num > 16)
            {
                printf("Invalid number, input again! Value Must Be 1~16 !!\r\n\r\n");
                continue;

            }
            else
            {
                num = num -1;
                printf("Input Success\n");
                break;

            }

        }
       
        /*输入ip地址*/
        while(1)
        {
            printf("Input whitelist IP or Press ENTER to return(e.g. 192.168.15.25)\r\n\\>");
            memset(str_input,0,sizeof(str_input));
            
            Get_SerialString(str_input,sizeof(str_input));
    
            if(str_input[0]== 0x0d || str_input[0] == 0x0a)
            {
                break;
            }
            else if(!VerifyIpAddress(str_input))
            {
                printf("\aInvalid address!\r\n\r\n");
            }
            else
            {
                g_Ntp_Parameter.whitelist_flag[num]=TRUE;                              
                g_Ntp_Parameter.whitelist_ip[num]= inet_addr(str_input);
                ret = 1;
                break;
            }
        }
    
        /*输入子网掩码*/
        while(1)
        {
            printf("Input Subnet Mask or input N(None) or type ENTER to return \r\n(e.g. 255.255.255.0, range:255.255.0.0~255.255.255.252)\r\n\\>");
            memset(str_input,0,sizeof(str_input));
            
            len = Get_SerialString(str_input,sizeof(str_input));

            if(len == 1)
            {
                g_Ntp_Parameter.whitelist_mask[num] = 0xffffffff;
                ret = 1;
                break;
            }
            else if(len == 2)
            {
                if((str_input[0] == 0x4E) || (str_input[0] == 0x6E))
                {
                    g_Ntp_Parameter.whitelist_mask[num] = 0xffffffff;
                    ret = 1;
                    break;
                }
                else
                {
                    printf("\aInvalid Mask address!\r\n\r\n");
                }
            }
            else
            {
                if(!VerifyNetMastAddress(str_input))
                {
                    printf("\aInvalid Mask address!\r\n\r\n");
                }
                else
                {
                    g_Ntp_Parameter.whitelist_mask[num]= inet_addr(str_input);
                    ret = 1;
                    break;
                }
            }
        }
    
    }
    
    return ret;

}


/**********************************************************************
 * Function:      Ip_Filter_State
 * Description:   白名单、黑名单配置界面
 * Input:          void
 * Return:        void
 * Others:        
**********************************************************************/
void *Ip_Filter_State(void)
{
    Uint8 my_choice;
	Uint8 ret =0;
    
    printf("\r\n------------------\r\n");
    printf("IP FILTER\r\n");
    printf("------------------\r\n");
    printf("1: Mode Select: ");
    if((g_Ntp_Parameter.blacklist==1) && (g_Ntp_Parameter.whitelist== 0))                                     //110506
        printf("Blacklist enabled!!\r\n");
    else if((g_Ntp_Parameter.blacklist==0) && (g_Ntp_Parameter.whitelist== 1))
        printf("Whitelist enabled!!\r\n");
    else if((g_Ntp_Parameter.blacklist==0) && (g_Ntp_Parameter.whitelist== 0))
        printf("Both lists disabled!!\r\n");
    else 
        printf("Mode Error !!\r\n");
            
    printf("2: Edit Blacklist\r\n");
    printf("3: Edit Whitelist\r\n");
    printf("R: Return\r\n");
	printf("------------------\r\n");
	printf("Select ?\r\n");

    my_choice = Get_SerialChar();
    switch (my_choice)
    {
        case '1':
            ret = Filter_Mode_Contrl();
		    break;
		    
	    case '2':
            ret = Filter_Edit_Blacklist();
		    break;
		case '3':
            ret = Filter_Edit_Whitlist();
            break;
             
        case 'R':
        case 'r':
            return (void *)Ntp_Manger_State;
            break;
        default:
            printf("\aInvalid Selection!");
            break;
    }

    return (void *)Ip_Filter_State;
}

