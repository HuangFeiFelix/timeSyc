#include "list_data.h"
#include "uart_handle.h"
#include "uart_main.h"
#include "data_type.h"
#include "termios.h"



extern void Init_Thread_Attr(pthread_attr_t *pThreadAttr);

UartDevice g_UartDevice;


/*打开串口函数*/
int Open_Ttysx()
{
	int fd;

	fd = open("/dev/ttyPS1", O_RDWR|O_NOCTTY|O_NDELAY);
	if (fd < 0)
	{
		perror("open serial port");
		return(-1);
	}
	/*恢复串口为阻塞状态*/
	if (fcntl(fd, F_SETFL, 0) < 0)
	{
		perror("fcntl F_SETFL\n");
	}

	/*测试是否为终端设备*/
	if (isatty(STDIN_FILENO) == 0)
	{
		perror("standard input is not a terminal device");
	}

	return fd;
}

int Set_Ttysx_WorkMode(int fd,int baud_rate, int data_bits, char parity, int stop_bits)
{
	struct termios new_cfg,old_cfg;
	int speed;

	/*保存测试现有串口参数设置，在这里如果串口号等出错，会有相关的出错信息*/
	if  (tcgetattr(fd, &old_cfg)  !=  0)
	{
		perror("tcgetattr");
		return -1;
	}

	/*设置字符大小*/
	new_cfg = old_cfg;
	cfmakeraw(&new_cfg);
	new_cfg.c_cflag &= ~CSIZE;

  	/*设置波特率*/
  	switch (baud_rate)
  	{
  		case 2400:
		{
			speed = B2400;
		}
		break;

  		case 4800:
		{
			speed = B4800;
		}
		break;

  		case 9600:
		{
			speed = B9600;
		}
		break;

		case 19200:
		{
			speed = B19200;
		}
		break;

  		case 38400:
		{
			speed = B38400;
		}
		break;

		default:
		case 115200:
		{
			speed = B115200;
		}
		break;
  	}
	cfsetispeed(&new_cfg, speed);
	cfsetospeed(&new_cfg, speed);

	/*设置停止位*/
	switch (data_bits)
	{
		case 7:
		{
			new_cfg.c_cflag |= CS7;
		}
		break;

		default:
		case 8:
		{
			new_cfg.c_cflag |= CS8;
		}
		break;
  	}

  	/*设置奇偶校验位*/
  	switch (parity)
  	{
		default:
		case 'n':
		case 'N':
		{
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_iflag &= ~INPCK;
		}
		break;

        case 'o':
		case 'O':
        {
            new_cfg.c_cflag |= (PARODD | PARENB);
            new_cfg.c_iflag |= INPCK;
        }
        break;

		case 'e':
        case 'E':
		{
			new_cfg.c_cflag |= PARENB;
			new_cfg.c_cflag &= ~PARODD;
			new_cfg.c_iflag |= INPCK;
        }
		break;

        case 's':  /*as no parity*/
		case 'S':
        {
			new_cfg.c_cflag &= ~PARENB;
			new_cfg.c_cflag &= ~CSTOPB;
		}
		break;
	}

	/*设置停止位*/
	switch (stop_bits)
	{
		default:
		case 1:
		{
			new_cfg.c_cflag &=  ~CSTOPB;
		}
		break;

		case 2:
		{
			new_cfg.c_cflag |= CSTOPB;
		}
	}

	/*设置等待时间和最小接收字符*/
	new_cfg.c_cc[VTIME]  = 0;
	new_cfg.c_cc[VMIN] = 1;
//CHENJIAYI
	new_cfg.c_lflag &=~(ICANON | ECHO |ECHOE |ISIG);
	new_cfg.c_oflag &=~OPOST;
	/*处理未接收字符*/
	tcflush(fd, TCIFLUSH);

	/*激活新配置*/
	if((tcsetattr(fd, TCSANOW, &new_cfg)) != 0)
	{
		perror("tcsetattr");
		return -1;
	}

	return 0;
}




/********************************************************************************************
* 函数名称:    Init_UartFd
* 功能描述:    初始化单盘通信口
* 输入参数:    无
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-6-19
               作者:     Felix
               修改内容: 新生成函数

********************************************************************************************/
int Init_UartFd()
{
    int comfd  = Open_Ttysx();
    if(comfd == -1)
        printf("comfd open error!!");


    return comfd;
}

/********************************************************************************************
* 函数名称:    初始化单盘通信链表和套接字，以及填入第一个告警消息
* 功能描述:    
* 输入参数:    无
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-6-19
               作者:     Felix
               修改内容: 新生成函数

********************************************************************************************/
int Init_Uart()
{
    pthread_t Idthread_UartHandle;        
    pthread_attr_t pattr;
    int ret;

    UartDevice *pUartDevice = &g_UartDevice;


    /**初始化接收套接字  */
    pUartDevice->comfd = Init_UartFd();
    Set_Ttysx_WorkMode(pUartDevice->comfd,115200,8,'N',1);
    
    pUartDevice->pRecvListHead = (struct Uart_List_Head *)malloc(sizeof(struct Uart_List_Head));
    memset(pUartDevice->pRecvListHead,0,sizeof(struct Uart_List_Head));
    
    pUartDevice->pSendListHead = (struct Uart_List_Head *)malloc(sizeof(struct Uart_List_Head));
    memset(pUartDevice->pSendListHead,0,sizeof(struct Uart_List_Head));
    
    /** 初始化读写锁 */
    pthread_rwlock_init(&pUartDevice->pSendListHead->list_rwlock, NULL);     
    pthread_rwlock_init(&pUartDevice->pRecvListHead->list_rwlock, NULL);
    
    /**初始化链表头，发送接收链表  */
    INIT_LIST_HEAD(&pUartDevice->pSendListHead->list_head_uart);
    INIT_LIST_HEAD(&pUartDevice->pRecvListHead->list_head_uart);

    Init_Thread_Attr(&pattr);

    /**获取槽位信息  */
    pUartDevice->slot_num = 4;
    GetFpgaVersion(pUartDevice->version.FpgaVersion);

    pUartDevice->version.CpuVersion[0] = 0x00;
    pUartDevice->version.CpuVersion[1] = 0x00; 
    pUartDevice->version.CpuVersion[2] = 0x01; 
    pUartDevice->version.CpuVersion[3] = 0x00;
    
    SetCpuVersion(pUartDevice->version.CpuVersion);
    pUartDevice->version.PcbVersion[0] = 0x00;
    pUartDevice->version.PcbVersion[1] = 0x00; 
    pUartDevice->version.PcbVersion[2] = 0x01; 
    pUartDevice->version.PcbVersion[3] = 0x00; 



    /*日常处理任务*/
    ret = pthread_create(&Idthread_UartHandle,&pattr,Thread_UartDataHandle,(void *)pUartDevice);	
	if(ret == 0)
	{
		printf("UartDataSend succes!\n");
	}
    
    
    msgPackVersionTimeUp(pUartDevice);
    msgPackParamRequest(pUartDevice);

    return 0;
}
