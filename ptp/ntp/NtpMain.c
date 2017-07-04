/******************************************************************************
*	Copyright (c) 2011, by The Fifth Research Institute of Telecommunications Technology
*				All rights reserved
*******************************************************************************
*	File name:	 NtpMain.c
*	Purpose:	     
*	Author:		 Felix(HuangFei)
*	Version:	  	 v01.00
*	Compiler:	 arm-linux-gcc
*	Created on:	 2014-9-10
*******************************************************************************
*	Note:
*	Revision history:  
*                       Created by hf    2014-9-10
******************************************************************************/

#include "NtpMain.h"
#include "comm_fun.h"
#include "PingManager.h"
#include "NetWorkConfig.h"
#include "Md5Manager.h"
#include "ClientIpFilter.h"
#include "NtpdConfigOperation.h"
#include "common.h"
#include "uart_main.h"

typedef void* (*StateMachin_Manger)(void);


StateMachin_Manger Ntp_State_Machine = Ntp_Manger_State;


/**********************************************************************
 * Function:      Ntp_Manger_State
 * Description:   主管理界面，所有管理命令的总入口
 * Input:         void
 * Return:        void*
 * Others:        
**********************************************************************/
void *Ntp_Manger_State(void)
{
        Uint8 my_choice;
        
        printf("\r\n===================\r\n");
		printf("NTP MANAGEMENT\r\n");                 
		printf("===================\r\n");
		
		printf("1: Server Mode Configuration\r\n");  
		printf("2: MD5 Configuration\r\n");                     
		printf("3: Client IP Filter\r\n");                                 
		//printf("4: Ping Test\r\n");
		printf("D: Display Ntp Configurations\r\n");
		printf("F: Factory Recovery\r\n");
        printf("S: Save Ntp Changes\r\n");
        printf("R: Return\r\n");
        
		printf("===================\r\n");
		printf("Select ?\r\n");
        
        my_choice = Get_SerialChar();

        switch (my_choice)
        {

		    case '1':                                                         
			    return (void *)Mode_Config_State;
                
            case '2': 
               Get_Md5_ctl();
               return (void *)Md5_Config_State;
               
            case '3':
                return (void *)Ip_Filter_State();
		
#if 0
		    case '4':
                return (void *)Ping_Manager_State;
			    break;
#endif
               
	        case 'D':
            case 'd':
                Display_All_Config();
                break;

            case 'F':
            case 'f':
                Factory_Manange();
                break;
                
            case 'S':
            case 's':
                Ntpd_ServerManage();
                return (void *)Ntp_Manger_State;

            case 'R':
            case 'r':
                return NULL;
            default:
                printf("\aInvalid Selection!");
                break;
        }

        return(void*)Ntp_Manger_State;
}


void Ntp_Manage_Configuration(void)
{
    while(1)
    {
        Ntp_State_Machine = (StateMachin_Manger)(*Ntp_State_Machine)();
        if(Ntp_State_Machine == NULL)
        {
            Ntp_State_Machine = Ntp_Manger_State;
            break;

        }
    }
}

/**********************************************************************
 * Function:      Factory_Manange
 * Description:   出厂信息管理，可恢复到出厂设置
 * Input:          void
 * Return:        void
 * Others:        
**********************************************************************/
void Factory_Manange()
{
    Uint8 my_choice;
    Uint8 input_str[50];
    Uint8 command[20];
    Uint32 imask;
    Uint32 net_ip;
    Uint8 *pIndex;

    
    struct in_addr temsock;
    
    Uint16 i;
    
    while(1)
    {
        printf("Do you want to load the factory settings\r\n");
        printf("Input YES or NO or Press ENTER to return!\r\n");
        printf("NOTE: This operation will be execute immediately!!\r\n\\>");

        memset(input_str,0,sizeof(input_str));
        Get_SerialString(input_str,sizeof(input_str));
        
        if(input_str[0] == 0x0a || input_str[0] == 0x0d)
        {
            break;
        }
        
        if((strcmp(input_str,"YES")==0) || (strcmp(input_str,"yes")== 0))
    	{
    	   
            g_Ntp_Parameter.blacklist = FALSE;
            g_Ntp_Parameter.whitelist = FALSE;
            g_Ntp_Parameter.sympassive = FALSE;
            g_Ntp_Parameter.multicast = FALSE;
            g_Ntp_Parameter.broadcast = FALSE;

            g_Ntp_Parameter.multicast_key_num = 0;
            g_Ntp_Parameter.broadcast_key_num = 0;
			
			/* 默认周期64s */
            g_Ntp_Parameter.freq_b = 64;
            g_Ntp_Parameter.freq_m = 64;
            
            g_Ntp_Enviroment.md5_flag = g_Ntp_Enviroment.md5_flag & MD5_DISABLE;

            for(i=1;i<=TOTAL_KEY_NO;i++)
    		{
    			g_Ntp_Enviroment.current_key[i]=factory_key[i];
    		}

			
            inet_aton(FACTORY_IP,&temsock);
            g_Ntp_Enviroment.runconf.ip = temsock.s_addr;

            inet_aton(FACTORY_GWIP,&temsock);
            g_Ntp_Enviroment.runconf.gwip = temsock.s_addr;

            inet_aton(FACTORY_MASK,&temsock);
            g_Ntp_Enviroment.runconf.mask = temsock.s_addr;

            /*恢复出厂设置不恢复 mac  地址*/
            #if 0
            memset(command,0,sizeof(command));
            memcpy(command,FACTORY_MAC,strlen(FACTORY_MAC));
            pIndex = command;
    		for(i = 0; i < 6; i++)
    		{
    			pIndex = strtok(pIndex,".");
    			g_Ntp_Enviroment.runconf.mac[i] = strtol(pIndex,NULL,16);			
    			pIndex = NULL;
    		}	
            #endif
            
            //g_pre_gwip = 0;
            
            for(i = 0;i < 16;i++)
            {
                g_Ntp_Parameter.whitelist_flag[i] = 0;
                g_Ntp_Parameter.whitelist_ip[i] = 0;
                g_Ntp_Parameter.whitelist_mask[i] = 0;
                
                g_Ntp_Parameter.blacklist_flag[i] = 0;
                g_Ntp_Parameter.blacklist_ip[i] = 0;
                g_Ntp_Parameter.blacklist_mask[i] = 0;
                
            }                      
            printf("Load successful!\r\n");

            break;
        }
    	else if((strcmp(input_str,"NO")==0) || (strcmp(input_str,"no") == 0))
    	{

    		printf("Operation cancelled!\r\n");
            break;
    	}
    	else					
    		printf("Invalid input!\r\n\r\n");
        
    }

}

/**********************************************************************
 * Function:      Stop_Ntp_Server
 * Description:   关闭ntpd服务，发送system命令关闭ntpd服务
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Stop_Ntp_Server()
{
    Uint8 command[20];
    memset(command,0,sizeof(command));
    
    sprintf(command,"%s","pkill ntpd");
    system(command);

}

/**********************************************************************
 * Function:      Start_Ntp_Server
 * Description:   开启ntpd服务，开启命令，如果debug==1 则ntp打开时开启debug信息
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Start_Ntp_Server()
{
    int ret;

    ret = system("/mnt/ntp/ntpd -c /tmp/ntp/ntp.conf");
    printf("\nNtp Server Running !!!\n\n");

   
    if((ret == 127) || (ret == -1))
    {
        printf("Start Ntp Fail !!\n");
    }

}

/**********************************************************************
 * Function:      Restart_Ntp_Server
 * Description:   重启ntp服务
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Restart_Ntp_Server()
{
   Stop_Ntp_Server();
   usleep(100);
   Start_Ntp_Server();

}

/**********************************************************************
 * Function:      Ntpd_ServerManage
 * Description:   管理ntpd服务
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Ntpd_ServerManage(void)
{

    Uint8 my_choice;
    Uint8 input_str[50];

    
    while(1)
    {
        printf("Input YES to Save all Changes or Input NO to Cancel or Press ENTER to return!\r\n");
        printf("Do You Really Want to Save all Changes ?\r\n\\>");

        memset(input_str,0,sizeof(input_str));
        Get_SerialString(input_str,sizeof(input_str));
        
        if(input_str[0] == 0x0a || input_str[0] == 0x0d)
        {

            break;
        }
        
        if((strcmp(input_str,"YES")==0) || (strcmp(input_str,"yes")== 0))
    	{
    	    Save_All_Changes();
    	    //Stop_Ntp_Server();
            printf("Save All Changes Successfully !!\r\n");
            //sleep(2);
            printf("Restart Ntp Server...\n");
            Restart_Ntp_Server();
            break;
    		
    	}
    	else if(strcmp(input_str,"NO")==0 || (strcmp(input_str,"no") == 0))
    	{
    		printf("Save all Changes Cancel !!\r\n");
            break;
    	}
    	else					
    		printf("Invalid input!\r\n\r\n");
    }   

}

/**********************************************************************
 * Function:      Manange_Broadcast
 * Description:   设置广播服务开启或者关闭，以及广播轮询时间
 * Input:          void
 * Return:        void
 * Others:        
**********************************************************************/
void Manange_Broadcast()
{
    Uint8 str_input[50];
    Uint8 len;
    Uint8 input;
    
    while(1)
    {
        printf("Broadcast ");
		if(g_Ntp_Parameter.broadcast==1)
		{
			printf("[On] Send period:%d(s)\r\n",g_Ntp_Parameter.freq_b);
		}
		if(g_Ntp_Parameter.broadcast==0)
		{	
			printf("[Off] \r\n");
		}
	    printf("1:On;2:Off;or Press ENTER to return\r\n\\>");

        input = Get_SerialChar();
        if(input == 0)
        {
            printf("Invalid number, input again!\r\n");
            continue;
        }
        if(input == 0x0a || input == 0x0d)
        {
            break;
        }
        else if(input == '1')
        {
            printf("Input send period or Press ENTER to return (e.g. 64/128/256/512/1024)\r\n\\>");
            memset(str_input,0,sizeof(str_input));
            Get_SerialString(str_input,sizeof(str_input));
            
            if((str_input[0] == 0x0a) || (str_input[0]== 0x0d))
            {
                break;
            }

            if (0==(strcmp(str_input,"64")) ||0==(strcmp(str_input,"128"))
                ||0==(strcmp(str_input,"256"))||0==(strcmp(str_input,"512"))
		        ||0==(strcmp(str_input,"1024")))
            {
                
                g_Ntp_Parameter.freq_b = atoi(str_input);
                PLOG("freq_b = %d\n",g_Ntp_Parameter.freq_b);
                g_Ntp_Parameter.broadcast = 1;
                break;
            }
            else
            {
                printf("Invalid number, input again!\r\n");
                continue;
            }
            
        }
        else if(input == '2')
        {
            g_Ntp_Parameter.broadcast=0;
            printf("Broadcast Off\r\n");
            break;
        }
        else
        {
            printf("Invalid number, input again!\r\n");
        }

    }
}

/**********************************************************************
 * Function:      Manange_Multicast
 * Description:   设置多播服务开启或关闭以及多播周期时间
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Manange_Multicast()
{
    Uint8 input;
    Uint8 len;
    Uint8 str_input[50];
    
    while(1)
    {
        printf("Multicast ");
        if(g_Ntp_Parameter.multicast==1)
        {
            printf("([On] Send period:%d(s)\r\n",g_Ntp_Parameter.freq_m);
        }
        if(g_Ntp_Parameter.broadcast==0)
        {   
            printf("[Off] \r\n");
        }
        printf("1:On;2:Off;Or Press ENTER to return\r\n\\>");

        input = Get_SerialChar();
        if(input == 0)
        {
            printf("Invalid number, input again!\r\n");
            continue;
        }
        
        if((input == 0x0d) || (input == 0x0a))
        {
            break;
        }
        else if(input == '1')
        {
            printf("Input send period or Press ENTER to return (e.g. 64/128/256/512/1024)\r\n\\>");
            
            memset(str_input,0,sizeof(str_input));
            Get_SerialString(str_input,sizeof(str_input));
            
            if((str_input[0] == 0x0a) || (str_input[0]== 0x0d))
            {
                break;
            }
            if (0==(strcmp(str_input,"64")) ||0==(strcmp(str_input,"128"))
                ||0==(strcmp(str_input,"256"))||0==(strcmp(str_input,"512"))
                ||0==(strcmp(str_input,"1024")))
            {
                g_Ntp_Parameter.freq_m = atoi(str_input);
                PLOG("freq_m = %d\n",g_Ntp_Parameter.freq_m);

                g_Ntp_Parameter.multicast= 1;
                break;
            }
            else
            {
                printf("Invalid number, input again!\r\n");
                continue;
            }
            
        }
        else if(input == '2')
        {
            g_Ntp_Parameter.multicast =0;
            printf("Multicast Off\r\n");
            break;
        }    
        else
        {
            printf("Invalid number, input again!\r\n");
        }
    }

}

/**********************************************************************
 * Function:      Manange_sympassive
 * Description:   对等体方式设置，开启或关闭
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Manange_sympassive()
{
    Uint8 input;
    Uint8 len;
    
    while(1)
    {
        printf("Symmetric passive ");
        if(g_Ntp_Parameter.sympassive==1)
        {
            printf("[On]\r\n");
        }
        if(g_Ntp_Parameter.sympassive==0)
        {   
            printf("[Off]\r\n");
        }
        printf("1:On;2:Off;or Press ENTER to return\r\n\\>");

        input = Get_SerialChar();
            
        if(input == 0)
        {
            printf("Invalid number, input again!\r\n");
            continue;
        }
        
        if((input == 0x0a) || (input == 0x0d))
        {
            break;
        }
        else if(input == '1')
        {
            g_Ntp_Parameter.sympassive = 1;
            printf("Symmetric passive On!\r\n");
            break;
            
        }
        else if(input == '2')
        {
            g_Ntp_Parameter.sympassive = 0;
            printf("Symmetric passive Off\r\n");
            break;
        }
        else
        {
            printf("Invalid number, input again!\r\n");
        }
    
    }

}

/**********************************************************************
 * Function:      Mode_Config_State
 * Description:   模式管理界面，可管理多播、广播、对等体
 * Input:         void
 * Return:        void*
 * Others:        
**********************************************************************/
void * Mode_Config_State(void)
{
    Uint8 my_choice;
    printf("\r\n------------------\r\n");
    printf("SERVER MODE CONFIGURATION\r\n");
    printf("------------------\r\n");

    printf("1: Broadcast ");
    if(g_Ntp_Parameter.broadcast == FALSE)
        printf("[Off]\r\n");
    else
        printf("[On] Send period:%d(s)\r\n",g_Ntp_Parameter.freq_b);
    
    printf("2: Multicast ");
    if(g_Ntp_Parameter.multicast == FALSE)
        printf("[Off]\r\n");
    else
        printf("[On] Send period:%d(s)\r\n",g_Ntp_Parameter.freq_m);

    /*
    printf("3: Symmetric passive ");
    if(g_Ntp_Parameter.sympassive == FALSE)
        printf("[Off]\r\n");
    else
        printf("[On]\r\n");
    */
    
    printf("R: Return\r\n ");

	printf("------------------\r\n");
	printf("Select ? \r\n");

    my_choice = Get_SerialChar();

    switch(my_choice)
    {

        case '1':
            Manange_Broadcast();
            break;
        case '2':
            Manange_Multicast();
            break;
        /*
        case '3':
            Manange_sympassive();
            break;
            */
        case 'R':
        case 'r':
            return (void *)Ntp_Manger_State;
        default:
            printf("\aInvalid Selection!");
            break;
    }

    return (void *)Mode_Config_State;
}



/**********************************************************************
 * Function:      Sys_Init
 * Description:   系统变量初始化，
 * Input:         void
 * Return:        void
 * Others:        
**********************************************************************/
void Sys_Init()
{

    memset(&g_Net_Infor,0,sizeof(g_Net_Infor));

    memset(&g_Ntp_Enviroment,0,sizeof(g_Ntp_Enviroment));
    memset(&g_Ntp_Parameter,0,sizeof(g_Ntp_Parameter));
    g_NtpDevice.ntpEnviroment = &g_Ntp_Enviroment;
    g_NtpDevice.ntpParameter = &g_Ntp_Parameter;
    g_NtpDevice.ntpStatus = &g_ntpd_status;

    
}

/**********************************************************************
 * Function:      main
 * Description:   程序入口，初始化系统参数，网络参数，读取config
 *				  ，开启ntp服务，运行管理软件等
 * Input:         int argc, char *argv[]
 * Return:        int 
 * Others:        
**********************************************************************/
int Init_NtpControl()
{

    Sys_Init();
    
    Load_Ntpd_Config();
    
    Start_Ntp_Server();
    
    Init_Ntpd_Socket();
    
    /*等待初始化完成*/
    sleep(1); 
    
    return 1;
}
