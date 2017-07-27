/********************************************************************************************
*                           版权所有(C) 2015, 
*                                 版权所有
*********************************************************************************************
*    文 件 名:       comm_api.c
*    功能描述:       串口处理模块
*    作者:           HuangFei
*    版本号:         v1.0
*    编译器:         arm-linux-gcc
*    生成日期:       2015-5-11
*    函数列表:
                     add_uart
                     open_port
                     set_com_config
*********************************************************************************************
*    备注:            
*    修改历史:
                   1.日期: 2015-5-11
                     作者: HuangFei
                     修改内容: 新创建文件


*********************************************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include "common.h"
#include "comm_api.h"



/*打开串口函数*/
int open_port(int com_port)
{
	int fd;

	char *dev[] = {"/dev/ttyPS1", "/dev/ttyUL1", "/dev/ttyUL2",  "/dev/ttyUL3","/dev/ttyPS5"};
	if ((com_port < 0) || (com_port > MAX_COM_NUM))
	{
		return -1;
	}
	fd = open(dev[com_port], O_RDWR|O_NOCTTY|O_NDELAY);
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

int set_com_config(int fd,int baud_rate, int data_bits, char parity, int stop_bits)
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
        case 57600:
        {
            speed = B57600;
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

void add_uart_lcd(struct uart_buf *pUart, unsigned char data)
{
    static unsigned char last_data = 0;

	if ((data == 0xEE) && (pUart->flags == 0x00))
	{
		pUart->flags = 0x01;
		pUart->cur = 0;
	}
    else if (data == 0xFF && last_data == 0xFF)
	{
		pUart->flags = 0x10;
		pUart->buf[pUart->cur++] = data;
		pUart->buf[pUart->cur++] = '\0';
  	}
    
	if (pUart->flags == 0x01)
	{
		pUart->buf[pUart->cur++] = data;
        //printf("cur %x\n",pUart->buf[pUart->cur-1]);
    }

    last_data = data;

}


void add_uart_ui(struct uart_buf *pUart, char data)
{
    static char last_data = 0;

	if ((data == '$') && (pUart->flags == 0x00))
	{
		pUart->flags = 0x01;
		pUart->cur = 0;
	}
    else if (data == 0x0a && last_data == 0x0d)
	{
		pUart->flags = 0x10;
		pUart->buf[pUart->cur++] = data;
		pUart->buf[pUart->cur++] = '\0';
  	}
    
	if (pUart->flags == 0x01)
	{
		pUart->buf[pUart->cur++] = data;
        //printf("cur %x\n",pUart->buf[pUart->cur-1]);
    }

    last_data = data;
    //printf("%x----%d---\n",data,pUart->cur);

}

void add_uart_rb(struct uart_buf *pUart, char data)
{
    static char last_data = 0;

    if (data == 0x0a && last_data == 0x0d)
	{
		pUart->flags = 0x10;
		pUart->buf[pUart->cur++] = data;
		pUart->buf[pUart->cur++] = '\0';
        //printf("cur1 %x \n",pUart->buf[pUart->cur-1]);
  	}
	else if (pUart->flags == 0x00)
	{
		pUart->buf[pUart->cur++] = data;
        //printf("cur %x \n",pUart->buf[pUart->cur-1]);
    }

    last_data = data;
    //printf("%x-------\n",data);

}


void add_uart_ppstod(struct uart_buf *pUart, char data)
{
    static char last_data = 0;

	if (pUart->flags == 0x01 && data != 'C')
	{
        //printf("cur %x\n",pUart->buf[pUart->cur]);
		pUart->buf[pUart->cur++] = data;
        
    }
    else if(pUart->flags == 0x01 && data == 'C')
    {
		pUart->flags = 0x10;
		//pUart->buf[pUart->cur++] = data;
		pUart->buf[pUart->cur] = '\0';

    }
    
	if (data == 'M' && last_data == 'C')
	{
		pUart->flags = 0x01;
		pUart->cur = 0;
	}
    

    last_data = data;
    //printf("%x-------\n",data);

}

void init_dev_head(struct dev_head *new_head)
{
	int i = 0;
	new_head->count = 0;
	FD_ZERO(&new_head->fd_set);
	FD_ZERO(&new_head->tmp_set);
	new_head->max_fd = 0;
	pthread_rwlock_init(&new_head->dev_rwlock, NULL);
	for (i = 0; i < 2; i++)
		sem_init(&new_head->sem[i], 0, 0);
	INIT_LIST_HEAD(&new_head->list_head);
}

void init_device(struct device *new_dev, int type, int dev_id)
{
	new_dev->dev_id = dev_id;
	new_dev->type = type;
}

int del_dev(struct device *p_dev, struct dev_head *dev_head)
{
	int i = 0;
	struct data_list *p_data_list = NULL, *tmp_data_list = NULL;
	del_list(&p_dev->list_head, &dev_head->dev_rwlock);   //chenjiayi
	for (i = 0; i < 3; i++)
	{
		list_for_each_entry_safe(p_data_list,tmp_data_list,&p_dev->data_head[i].list_head,list_head)
			del_data(p_data_list, &p_dev->data_head[i], p_dev);  //
		pthread_rwlock_destroy(p_dev->data_head[i].list_rwlock);
		free(p_dev->data_head[i].list_rwlock);
	}
	//free(p_dev);
	return TRUE;
}

int add_dev_list(struct device *p_dev, struct dev_head *dev_head)
{
	pthread_rwlock_wrlock(&dev_head->dev_rwlock);
	list_add(&p_dev->list_head, &dev_head->list_head);
	pthread_rwlock_unlock(&dev_head->dev_rwlock);
	return TRUE;
}

int open_udp(struct device *p_dev, struct dev_head *dev_head)
{
    int fd = -1;
    char opt = 1;;
    struct sockaddr_in sockaddr;
    struct net_attr *p_net_attr = &p_dev->net_attr;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
            perror("socket error\n");
            return FALSE;
    }
    memset(&sockaddr,0,sizeof(sockaddr));
    
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(p_dev->net_attr.sin_port);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt,sizeof(opt));
    //setsockopt(fd,SOL_SOCKET,SO_BROADCAST,(const char*)&opt,sizeof(opt));

    if (0 > bind(fd, (struct sockaddr *)&sockaddr, sizeof(struct sockaddr_in)))
    {
        printf("UDP socket bind udp error!\n");
        close(fd);
        return FALSE;
    
    }

    p_dev->fd = fd;

    return TRUE;
}


int open_net(struct device *p_dev, struct dev_head *dev_head)
{
	int fd = -1,opt = 1;
	struct sockaddr_in net_sockaddr;
	struct net_attr *p_net_attr = &p_dev->net_attr;
    
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
            perror("socket error\n");
            return FALSE;
    }
    
    bzero(&(net_sockaddr.sin_zero), 8);
    net_sockaddr.sin_family = AF_INET;
    net_sockaddr.sin_port = htons(p_dev->net_attr.sin_port);
    net_sockaddr.sin_addr.s_addr = INADDR_ANY;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (bind(fd, (struct sockaddr *) &net_sockaddr, sizeof(struct sockaddr))    == -1)
    {
        perror("bind error\n");
        close(fd);
        return FALSE;
    }

    if (listen(fd, MAX_LINK) == -1)
    {
        perror("listen error\n");
        close(fd);
        return FALSE;
    }
    p_dev->fd = fd;
    printf("fd=%d\n",fd);
    
    //close(fd);
    return TRUE;

}


int open_com(struct device *p_dev, struct dev_head *dev_head)
{

	int fd = -1;
	struct com_attr *p_com_attr = &p_dev->com_attr;

	if ((fd = open_port(p_com_attr->com_port)) < 0)
	{
		perror("open_port error\n");
		return FALSE;
	}
	if (set_com_config(fd, p_com_attr->baud_rate, STOP_BITS, PARITY, STOP_BITS)
			< 0) {
		perror("set_com_config error\n");
		return FALSE;
	}
	p_dev->fd = fd;
	return TRUE;
}

int open_int(struct device *p_dev, struct dev_head *dev_head)
{
	int fd = -1;
	fd = open(p_dev->int_attr.path, O_RDWR);
	if (fd < 0) {
		perror("open_int error\n");
		return FALSE;
	}
	p_dev->fd = fd;

    
	return TRUE;
}

int open_dev(struct device *p_dev, struct dev_head *dev_head)
{
	int i = 0;
	pthread_rwlock_t *dev_rwlock[3];

	if (p_dev->open_dev == NULL )
	{
		switch (p_dev->type)
		{
		case TCP_DEVICE:
			p_dev->open_dev = open_net;
			break;
		case COMM_DEVICE:
			p_dev->open_dev = open_com;
			break;
		case INIT_DEVICE:
			p_dev->open_dev = open_int;
			break;
        case UDP_DEVICE:
            p_dev->open_dev = open_udp;
            break;
		default:
			return FALSE;
			break;
		}
	}
	p_dev->open_dev(p_dev, dev_head);

	for (i = 0; i < 3; i++)
	{

		dev_rwlock[i] = (pthread_rwlock_t *) malloc(sizeof(pthread_rwlock_t));
		pthread_rwlock_init(dev_rwlock[i], NULL );
		init_data_head(&p_dev->data_head[i], i + 1, dev_rwlock[i]);
	}

	for (i = 0;i < 2;i ++)
		p_dev->p_sem[i] = &dev_head->sem[i];

    p_dev->data_head[0].type = 0;/**接收为0  */
    p_dev->data_head[1].type = 1;/**发送为1  */

    
	add_dev_list(p_dev, dev_head);
	return TRUE;
}

int add_fd(struct device *p_dev, struct dev_head *dev_head)
{
	FD_SET(p_dev->fd, &dev_head->fd_set);
	if (p_dev->fd > dev_head->max_fd)
		dev_head->max_fd = p_dev->fd;
	return TRUE;
}


int recv_udp_data(struct device *p_dev, struct dev_head *dev_head)
{
    char *pBuf = p_dev->net_attr.buf;
    int fd = p_dev->fd, len = 0;

    
    if(FD_ISSET(fd,&dev_head->tmp_set) > 0)
    {
        if((len = recvfrom(fd,pBuf,BUFFER_SIZE,0,(struct sockaddr*)&p_dev->dest_addr,&p_dev->sock_len)) > 0)
        {
            printf("recv udp data\n");
            add_recv(pBuf,fd,len,p_dev);
        }
    }

    return TRUE;

}

int recv_net_data(struct device *p_dev, struct dev_head *dev_head)
{
	char *p_buf = p_dev->net_attr.buf;
	int fd = p_dev->fd, client_fd = -1, len = 0;
	struct sockaddr_in client_sockaddr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	struct data_list *p_data_list = NULL, *tmp_data_list = NULL;
	
	if (FD_ISSET(fd,&dev_head->tmp_set) > 0)
	{
		printf("new client connect\n");
		if ((client_fd = accept(fd, (struct sockaddr *) &client_sockaddr,
				&addr_len)) == -1)
		{
			perror("accept error\n");
			return FALSE ;
		}
		add_net(client_fd, &client_sockaddr, p_dev);
		FD_SET(client_fd, &dev_head->fd_set);
		if (client_fd > dev_head->max_fd)
			dev_head->max_fd = client_fd;
	} else {
		list_for_each_entry_safe(p_data_list,tmp_data_list,&p_dev->data_head[2].list_head,list_head)
		{
			client_fd = p_data_list->net_lev.fd;
			if (FD_ISSET(client_fd,&dev_head->tmp_set) > 0)
			{
				if ((len = read(client_fd, p_buf, BUFFER_SIZE)) > 0)
				{
					p_buf[len++] = '\0';   //chenjiayi
					add_recv(p_buf, client_fd,len, p_dev);
				} else
				{
					close(client_fd);
					FD_CLR(client_fd, &dev_head->fd_set);
					del_data(p_data_list, &p_dev->data_head[2], p_dev);
				}
			}
		}
	}
	return TRUE;
}

int recv_ui_com_data(struct device *p_dev, struct dev_head *dev_head)
{
	char buf,*p_buf = p_dev->com_attr.uart_buf.buf;
	int fd = p_dev->fd, len = 0;
	if (FD_ISSET(fd,&dev_head->tmp_set) > 0)
	{
		if ((len = read(fd, &buf, UART_LEN)) > 0)

			add_uart_ui(&p_dev->com_attr.uart_buf, buf);
	}
	if (p_dev->com_attr.uart_buf.flags == 0x10)
	{

		add_recv(p_buf, fd,p_dev->com_attr.uart_buf.cur, p_dev);
		p_dev->com_attr.uart_buf.flags = 0x00;
	}
	return TRUE;
}

int recv_lcd_com_data(struct device *p_dev, struct dev_head *dev_head)
{
	char buf;
    char *p_buf = p_dev->com_attr.uart_buf.buf;
	int fd = p_dev->fd, len = 0;
	if (FD_ISSET(fd,&dev_head->tmp_set) > 0)
	{
		if ((len = read(fd, &buf, UART_LEN)) > 0)
			add_uart_lcd(&p_dev->com_attr.uart_buf, buf);
        
	}
	if (p_dev->com_attr.uart_buf.flags == 0x10)
	{

		add_recv(p_buf, fd,p_dev->com_attr.uart_buf.cur, p_dev);
		p_dev->com_attr.uart_buf.flags = 0x00;
	}

	return TRUE;
}

int recv_ppstod_com_data(struct device *p_dev, struct dev_head *dev_head)
{
	char buf,*p_buf = p_dev->com_attr.uart_buf.buf;
	int fd = p_dev->fd, len = 0;
	if (FD_ISSET(fd,&dev_head->tmp_set) > 0)
	{
		if ((len = read(fd, &buf, UART_LEN)) > 0)

			add_uart_ppstod(&p_dev->com_attr.uart_buf, buf);
	}
	if (p_dev->com_attr.uart_buf.flags == 0x10)
	{
		add_recv(p_buf, fd,p_dev->com_attr.uart_buf.cur, p_dev);
        
		p_dev->com_attr.uart_buf.flags = 0x00;
	}
	return TRUE;
}

int recv_rb_com_data(struct device *p_dev, struct dev_head *dev_head)
{
	char buf,*p_buf = p_dev->com_attr.uart_buf.buf;
	int fd = p_dev->fd, len = 0;
	if (FD_ISSET(fd,&dev_head->tmp_set) > 0)
	{
		if ((len = read(fd, &buf, UART_LEN)) > 0)
			add_uart_rb(&p_dev->com_attr.uart_buf, buf);
	}
	if (p_dev->com_attr.uart_buf.flags == 0x10)
	{
		add_recv(p_buf, fd,p_dev->com_attr.uart_buf.cur, p_dev);
		p_dev->com_attr.uart_buf.flags = 0x00;
        p_dev->com_attr.uart_buf.cur = 0;
	}
	return TRUE;
}


int recv_int_data(struct device *p_dev, struct dev_head *dev_head)
{
	char *p_buf = p_dev->int_attr.buf;
	int fd = p_dev->fd, len = 0;

	if (FD_ISSET(fd,&dev_head->tmp_set) > 0)
	{
		if((len = read(fd,p_buf, 100)) > 0)
		{   
			add_recv(p_buf,fd,len, p_dev);
            
        }
	}
	return TRUE;
}

int recv_data(struct device *p_dev, struct dev_head *dev_head)
{

	if(p_dev->recv_data == NULL)
	{
		switch(p_dev->type)
		{
    		case TCP_DEVICE:
    			p_dev->recv_data = recv_net_data;
    			break;
    		case COMM_DEVICE:
                switch(p_dev->dev_id)
                {
                    case ENUM_BUS:
                        p_dev->recv_data = recv_ui_com_data;
                        break;
                    case ENUM_LCD:
                        p_dev->recv_data = recv_lcd_com_data;
                        break;
                    case ENUM_GPS:
                        p_dev->recv_data = recv_ui_com_data;
                        break;
                    case ENUM_RB:
                        p_dev->recv_data = recv_rb_com_data;
                        break;
                    case ENUM_PPS_TOD:
                        p_dev->recv_data = recv_ppstod_com_data;
                        break;
                    default:
                        break;
                        
                }
    			break;
    		case INIT_DEVICE:
    			p_dev->recv_data = recv_int_data;
    			break;
            case UDP_DEVICE:
                p_dev->recv_data = recv_udp_data;
                break;
    		default:
    			break;
		}
	}
	return TRUE;
}

int send_data(struct device *p_dev, struct dev_head *dev_head)
{
	return TRUE;
}

static inline int close_dev(struct device *p_dev, struct dev_head *dev_head)
{

	close(p_dev->fd);  //chen
	//del_dev(p_dev, dev_head);
	//FD_CLR(p_dev->fd, &dev_head->fd_set);  //chen
	return TRUE;
}

int add_dev(struct device *p_dev, struct dev_head *dev_head)
{
	open_dev(p_dev,dev_head);

	add_fd(p_dev,dev_head);

	recv_data(p_dev,dev_head);
	return TRUE;
}

int init_add_dev(struct device *p_dev, struct dev_head *dev_head,int type, int dev_id)
{
	init_device(p_dev,type,dev_id);

	add_dev(p_dev,dev_head);

	return TRUE;
}




