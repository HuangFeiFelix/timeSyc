/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 Md5Manager.c
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

#include "Md5Manager.h"
#include "NetWorkConfig.h"
#include "ClientIpFilter.h"

extern void *Ntp_Manger_State(void);

static MD5_CONTROL g_md5_ctl; /* MD5 密钥控制结构体 */
static MD5_KEY g_md5_key[10];/* 局部md密钥列表 */

/* 出厂 MD5 密钥 */
MD5_KEY factory_key[10]={ 
                          {0,0,""},
                          {1,16,"0123456789abcdef"},
						  {1,16,"123456789abcdef0"},
						  {1,16,"23456789abcdef01"},
						  {1,16,"3456789abcdef012"},
						  {1,16,"okdidflg_+|-=;'d"},
						  {1,16,"wk3k4kflv,f;ro&"},
						  {1,16,">dfwertrtsdf90*&"},
						  {1,10,"*&^%$45678"},
						 };


/**********************************************************************
 * Function:      Read_Md5FromKeyFile
 * Description:   读取MD5密钥，保存到密钥列表中
 * Input:          pKey：密钥列表地址
 * Return:        成功1，出错-1；
 * Others:        
**********************************************************************/
Uint8 Read_Md5FromKeyFile(MD5_KEY *pKey)
{
    Uint16 i;
    Uint8 Data[50];
    Uint8 num;
    Uint8 type;
    Uint8 key_str[20];
    FILE *md5_file_fd = fopen(NTP_MD5_FILE,"r");
    if(md5_file_fd == NULL)
    {
        printf("can not find md5 file\n");
        return -1;
    }
    
    i = 1;
    memset(Data,0,sizeof(Data));
    
    while(fgets(Data,sizeof(Data),md5_file_fd))
    {
        memset(key_str,0,sizeof(key_str));

        sscanf(Data,"%c %c %s",&num,&type,key_str);
        memcpy(g_Ntp_Enviroment.current_key[i].key,key_str,20);
        g_Ntp_Enviroment.current_key[i].key_length = strlen(key_str);
        g_Ntp_Enviroment.current_key[i].key_valid = TRUE;
        
        memset(Data,0,sizeof(Data));
        i++;
    }
    
     
    fclose(md5_file_fd);
}

/**********************************************************************
 * Function:      Write_Md5ToKeyFile
 * Description:   将密钥内容写入keys文件中
 * Input:          pKey：密钥列表地址,iCount：列表数量
 * Return:        成功1，错误-1
 * Others:        
**********************************************************************/
Uint8 Write_Md5ToKeyFile(MD5_KEY *pKey,Uint16 iCount)
{
    Uint16 i;
    Uint8 Data[50];
    
    FILE *md5_file_fd = fopen(NTP_MD5_FILE,"w+");
    if(md5_file_fd == NULL)
    {
        printf("can not find md5 file\n");
        return -1;
    }

    for(i = 1;i <= iCount; i++)
    {
        memset(Data,0,sizeof(Data));
        if(pKey[i].key_length == 0)
        {
            continue;
        }

        sprintf(Data,"%d %c %s\n",i,'M',pKey[i].key);

        fputs(Data,md5_file_fd);
    }
    fflush(md5_file_fd);
    fclose(md5_file_fd);
    return TRUE;
}

/**********************************************************************
 * Function:      Get_Md5_ctl
 * Description:   将系统使用密钥内容拷贝到临时密钥配置结构体中
 * Input:          void
 * Return:        void
 * Others:        
**********************************************************************/
void Get_Md5_ctl(void)
{
    g_md5_ctl.config_md5_flag = g_Ntp_Enviroment.md5_flag;
    g_md5_ctl.broadcast_current_keyno = g_Ntp_Parameter.broadcast_key_num;
    g_md5_ctl.multicast_current_keyno = g_Ntp_Parameter.multicast_key_num;
    memmove(g_md5_key,g_Ntp_Enviroment.current_key,sizeof(g_md5_key));
}

/**********************************************************************
 * Function:      Set_Md5_ctl
 * Description:   将临时密钥配置结构体宝贝到系统密钥结构体中
 * Input:          void 
 * Return:        void
 * Others:        
**********************************************************************/
void Set_Md5_ctl(void)
{
    g_Ntp_Enviroment.md5_flag = g_md5_ctl.config_md5_flag;
    g_Ntp_Parameter.broadcast_key_num = g_md5_ctl.broadcast_current_keyno;
    g_Ntp_Parameter.multicast_key_num = g_md5_ctl.multicast_current_keyno;
    memmove(g_Ntp_Enviroment.current_key,g_md5_key,sizeof(g_md5_key));
   
}


/**********************************************************************
 * Function:      MD5_Display
 * Description:   打印密钥列表内容
 * Input:          void
 * Return:        void
 * Others:        
**********************************************************************/
void MD5_Display(void)
{
    
    printf("------------------\r\n");
	printf("MD5 CONFIGURATION\r\n");
	printf("------------------\r\n");
	printf("1: MD5 authentication ");
    if((g_md5_ctl.config_md5_flag & MD5_ENABLE) == MD5_ENABLE)
    {
        printf("[Enable]\r\n");
    }
    else
    {
        printf("[Disable]\r\n");
    }
    
    if((g_md5_ctl.multicast_current_keyno == 0) && (g_md5_ctl.broadcast_current_keyno== 0))
    {
        printf("2: Select MD5 key Number for multicast [Random] and broadcast [Random]\r\n");

    }
    else if((g_md5_ctl.multicast_current_keyno >= 1) && (g_md5_ctl.broadcast_current_keyno == 0))
    {
        printf("2: Select MD5 key Number for multicast [%d] and broadcast [Random]\r\n"
           ,g_md5_ctl.multicast_current_keyno);
    }
    else if((g_md5_ctl.multicast_current_keyno == 0) && (g_md5_ctl.broadcast_current_keyno >= 1))
    {
        printf("2: Select MD5 key Number for multicast [Random] and broadcast [%d]\r\n"
           ,g_md5_ctl.broadcast_current_keyno);
    }
    else
    {
        printf("2: Select MD5 key Number for multicast [%d] and broadcast [%d]\r\n"
           ,g_md5_ctl.multicast_current_keyno
           ,g_md5_ctl.broadcast_current_keyno);
    }
    
    printf("3: Edit MD5 key list\r\n");
    //printf("D: Delete MD5 keys\n");
	printf("F: Factory recovery for MD5 key list(8 keys in all)\r\n");
	printf("R: Return\r\n");
	printf("------------------\r\n");
	printf("Select ?\r\n");
}

/**********************************************************************
 * Function:      Dis_Edit_Md5_List_State
 * Description:   编辑md5密钥列表内容
 * Input:          void
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
Uint8  Dis_Edit_Md5_List_State()
{
    int i;
    Uint8 first_input[20];
    Uint8 second_input[20];
    Uint8 input_str[20];
    int key_len = 0;
    Uint8 ret = 0;
    Uint8 len;
    
    Uint8 input;
    
    while(1)
    {
        printf("Current MD5 list:\r\n");
        printf("NUM  MD5              Valide\r\n");
        for(i=1;i<9;i++)                            
        {
            if(g_md5_key[i].key_valid==1)
                printf("%d:   %17s YES\r\n",i,g_md5_key[i].key);
            else
                printf("%d:   ----------------- NO\r\n",i);
        }
        printf("Input the key NO.(1~8) you want to config \r\n");
    	printf("or Press ENTER to return! \r\n\\>");

        Uint8 EditNum;

        input = Get_SerialChar();
        
        if(input ==  0)
        {
            printf("Invalid Number: MUST be 1~8\r\n\r\n");
            continue;
        }

        if(input == 0x0a || input == 0x0d)
        {
            break;
        }
                
        EditNum = input - '0';
        
        if(EditNum >= 1 && EditNum <= 8)
        {
            memset(first_input,0,sizeof(first_input));
            memset(second_input,0,sizeof(second_input));

            printf("Input the key (the length should be 1~16)! \r\n\\>");
            
            Get_SerialString(first_input,sizeof(first_input));
            
            if(first_input[0] == 0x0a || first_input[0] == 0x0d)
            {
                break;
            }

            key_len = strlen(first_input);
            if(key_len > 16)																	
        	    printf("ERROR: the length of key you input is too long (must be 1~16)!\r\n\r\n");
        	else if(key_len < 1)                                                                                                         
        	    printf("ERROR: the length of key you input is too short (must be 1~16)!\r\n\r\n");
        	else
            {
                printf("Input again! \r\n\\>");
                Get_SerialString(first_input,sizeof(first_input));
                if(strcmp(first_input,second_input)==0)
                {
                    g_md5_key[EditNum].key_valid=1;
        			g_md5_key[EditNum].key_length=(Uint8)key_len;
        			memset(g_md5_key[EditNum].key, 0, 20);                              //110524
        			memcpy(g_md5_key[EditNum].key,second_input,sizeof(second_input));
        			
        			printf("Successfully config the key %d!\r\n",EditNum);

                    ret = 1;
                    break;
                    
                }
                else
                {
                    printf("Input key no equal\n");
                }
            }
        }
        else
        {
            printf("Invalid Number: MUST be 1~8\r\n\r\n");
            
        }
    }
   
    return ret;
}

/**********************************************************************
 * Function:      Md5_En_Dis_Control
 * Description:   开启密钥或者关闭密钥控制
 * Input:          void
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
Uint8 Md5_En_Dis_Control()
{
    Uint8 input[20];
    Uint8 ret = 0;

    while(1)
    {
        printf("The MD5 authentication is ");
        if((g_md5_ctl.config_md5_flag & MD5_ENABLE)==0)
        {
            printf("Disabled \r\n");
            printf("Input ENABLE to enable the MD5 authentication or Press ENTER to return!\r\n\\>");
        }
        else
        {
            printf("Enabled \r\n");
            printf("Input DISABLE to disable the MD5 authentication or Press ENTER to return!\r\n\\>");
        }

        memset(input,0,sizeof(input));
        
        Get_SerialString(input,sizeof(input));
        
        if(input[0] == 0x0a || input[0] == 0x0d)
        {
             break;
        }
        
        if(strcmp(input,"ENABLE")==0 || strcmp(input,"enable") == 0)
    	{
    		g_md5_ctl.config_md5_flag |=MD5_ENABLE;
    		printf("The MD5 authentication Enable!!\r\n");
    		ret=1;
            break;
    		
    	}
    	else if(strcmp(input,"DISABLE")==0 || strcmp(input,"disable") == 0)
    	{
    		g_md5_ctl.config_md5_flag &= ~MD5_ENABLE;
    		printf("The MD5 authentication Disable!!\r\n");
    		ret=1;
            break;
    		
    	}
    	else					
    		printf("Invalid input!\r\n\r\n");
    }
   

    return ret;
    
    
}

/**********************************************************************
 * Function:      Md5_BroadCast_Control
 * Description:   广播ntp服务密钥控制，可以是自动密钥或者指定
 * Input:          void
 * Return:        成功 1， 失败 0
 * Others:        
**********************************************************************/
Uint8 Md5_BroadCast_Control()
{
    Uint8 ret = 0;
    Uint16 keyno;
    Uint8 input;
    
    Uint8 len;
    
    if((g_md5_ctl.multicast_current_keyno == 0) && (g_md5_ctl.broadcast_current_keyno== 0))
    {
        printf("Multicast key num [Random],Broadcast key num [Random]\r\n");

    }
    else if((g_md5_ctl.multicast_current_keyno == 1) && (g_md5_ctl.broadcast_current_keyno == 0))
    {
        printf("Multicast key num [%d],Broadcast key num [Random]\r\n"
           ,g_md5_ctl.multicast_current_keyno);

    }
    else if((g_md5_ctl.multicast_current_keyno == 0) && (g_md5_ctl.broadcast_current_keyno == 1))
    {
        printf("Multicast key num [Random],Broadcast key num [%d]\r\n"
           ,g_md5_ctl.broadcast_current_keyno);
    }
    else
    {
        printf("Multicast key num [%d],Broadcast key num [%d]\r\n"
           ,g_md5_ctl.multicast_current_keyno
           ,g_md5_ctl.broadcast_current_keyno);
    }


    /*选择md5 值到广播*/
    while(1)
    {
        printf("Please input the key NO.(1-8) which you want to apply to Broadcast\r\n");
        printf("Input 0 to select Random or Press ENTER to Return!\r\n\\>");

        input = Get_SerialChar();
        if(input == 0)
        {
            printf("Invalid Number: MUST be 0~8\r\n\r\n");
            continue;
        }
        
        if(input == 0x0a || input == 0x0d)
        {
            break;
        }

        keyno = input - '0';
        if(keyno>=0 && keyno < 9)
        {
            if((keyno >0)&&(g_md5_key[keyno].key_valid!=1))
            {
                printf("ERROR:the selected key have not been configured !\r\n");
    			printf("Please config the key first or select a configured key\r\n\r\n");	
            }
        
            else
            {
                g_md5_ctl.broadcast_current_keyno = (Uint8)keyno;
    			printf("Success!\r\n");
    			ret=1;
                break;
            }
        }
        else
        {
            printf("Invalid Number: MUST be 0~8\r\n\r\n");
        }
    }

    /*选择md5 值到多播*/
    while(1)
    {
        printf("Please input the key NO.(1-8) which you want to apply to Multicast\r\n");
        printf("Input 0 to select Random or Press ENTER to Return!\r\n\\>");

        
        input = Get_SerialChar();
        if(input == 0)
        {
            printf("Invalid Number: MUST be 0~8\r\n\r\n");
            continue;
        }

        
        if(input == 0x0a || input == 0x0d)
        {
            break;
        }

        keyno = input - '0';
        if(keyno>=0 && keyno < 9)
        {
            if((keyno >0)&&(g_md5_key[keyno].key_valid!=1))
            {
                printf("ERROR:the selected key have not been configured !\r\n");
    			printf("Please config the key first or select a configured key\r\n\r\n");	
            }
        
            else
            {
                g_md5_ctl.multicast_current_keyno = (Uint8)keyno;
    			printf("Success!\r\n");
    			ret=1;
                break;
            }
        }
        else
        {
            printf("Invalid Number: MUST be 0~8\r\n\r\n");
        }
    }   
    
    return ret;
}

/**********************************************************************
 * Function:      Md5_FactoryReset_Control
 * Description:   恢复密钥出厂设置
 * Input:          void
 * Return:        成功 1， 失败 0
 * Others:        
**********************************************************************/
Uint8 Md5_FactoryReset_Control()
{
    Uint8 ret = 0;
    Uint8 input[20];
    int i;

    while(1)
    {
        printf("Load the factory MD5 keys(YES or NO)? \r\n");
    	printf("Or Press ENTER to return\r\n");
    	printf("NOTE: This operation will be executed immediately!!\r\n\\>");
        
        memset(input,0,sizeof(input));
        
        Get_SerialString(input,sizeof(input));
        if(input[0] == 0x0a || input[0] == 0x0d)
        {
            break;
        }
        
        if((strcmp(input,"YES")==0) || (strcmp(input,"yes") == 0))
    	{
    		for(i=1;i<=TOTAL_KEY_NO;i++)
    		{
    			g_md5_key[i]=factory_key[i];
    			g_Ntp_Enviroment.current_key[i]=factory_key[i];

    		}
    		
    		printf("load successfully!\r\n");
            ret = 1;
    		break;
    	}
    	else if(strcmp(input,"NO")==0 ||(strcmp(input,"no") == 0) )
    	{
    		
    		printf("Operation cancelled!\r\n");
            break;

    	}
    	else					
    		printf("Invalid input!\r\n\r\n");
        
        
    }

    return ret;
    
}

/**********************************************************************
 * Function:      Md5_Config_State
 * Description:   MD5管理界面
 * Input:          void
 * Return:        void*
 * Others:        
**********************************************************************/
void * Md5_Config_State(void)
{

    Uint8 my_choice;
	Uint8 ret =0;

    MD5_Display();

    my_choice = Get_SerialChar();
    switch (my_choice)
    {
        case '1':
            ret = Md5_En_Dis_Control();
		    break;
		    
	    case '2':
            ret = Md5_BroadCast_Control();
		    break;
		case '3':
            ret = Dis_Edit_Md5_List_State();
            break;
	    case 'F':
        case 'f':
            ret = Md5_FactoryReset_Control();
		    break;
        //case 'D':
        //case 'd':
            //Md5_Delet_OneKey();
            //break;
             
        case 'R':
        case 'r':
             Set_Md5_ctl();
            return (void *)Ntp_Manger_State;
            break;
        default:
            printf("\aInvalid Selection!\r\n");
            break;
    }

    
    return (void *)Md5_Config_State;
}

