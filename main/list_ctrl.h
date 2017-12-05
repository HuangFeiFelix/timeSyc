/*
 * list_ctrl.h
 *
 *  Created on: 2014-2-23
 *      Author: administrator
 */

#ifndef LIST_CTRL_H_
#define LIST_CTRL_H_

#include <stdio.h>
#include <string.h>

#include "list.h"

#define MAX_COM_NUM			5

#define UART_BUF_SIZE 	    512
#define UART_LEN 			1

#define PROTOCOL 0
#define MAX_LINK 10

#define DATA_BITS 8
#define PARITY 'N'
#define STOP_BITS 1
#define BUFFER_SIZE 			1500


struct uart_buf {
	int cur;
	int flags;
	char buf[UART_BUF_SIZE];
};


struct net_lev{
	int fd;							//客户端文件描述符
	struct sockaddr_in dev_sockaddr;//套接字
};

struct recv_lev{
	char *data;						//数据存放地址
	int fd; 						//数据来源
	int len;						//数据长度
	time_t recv_time,deal_time;		//recv_time 数据创建时间 deal_time 数据处理时间
};

struct send_lev{
	char *data;						//数据存放地址
	int len;						//数据长度

	union {
		int fd;						//发送文件描述符
		struct list_head *fd_head;	//发送文件描述符列向量头指针
	};
	time_t add_time,send_time;		//recv_time 数据创建时间,deal_time 数据处理时间
};

struct data_list {
	union {
		struct send_lev send_lev;	//发送链表
		struct recv_lev recv_lev;	//接收链表
		struct net_lev net_lev;		//网络链表

	};
	struct list_head list_head;		//行向量链表用于链接数据
};

struct data_head {
	int type;						//类型 0：接收数据，1：发送数据，2：文件描述符
	int count;						//链表中数据数目
	pthread_rwlock_t *list_rwlock;	//链表操作读写锁
	struct list_head list_head;		//数据链表
};

struct dev_head{
	int count;						//总设备数量
	pthread_rwlock_t dev_rwlock;	//设备链表操作读写锁
	struct {						//select函数使用参数
		fd_set fd_set;				//fd_set
		fd_set tmp_set;				//tmp_set
		int max_fd;					//文件描述符最大值
		sem_t sem[2];				//［0］：数据接收，［1］：数据发送
	};
	struct list_head list_head;		//设备链表头
};

struct net_attr{
	int domain;						//通信域，套接定域描述
	int sock_type;					//套接字类型
	int protocol;					//制定和种协议，这个参数通常为0
	short int sin_family;			//地址家族，一般是AF_INET,代表TCP/IP协议族
	int sin_port;					//端口号
	int ip;				            //ip地址
	int max_link;					//最大连接数
	char buf[BUFFER_SIZE];   		//网口接收缓冲
};

struct com_attr{
	int com_port;					//串口端口号
	int baud_rate;					//波特率
	int data_bits;					//数据位
	char parity;					//奇偶校验位
	int stop_bits;					//停止位
	struct uart_buf uart_buf;		//串口接收缓冲
};

struct int_attr{
	char path[64];					//设备路径
	char buf[BUFFER_SIZE];			//数据接收缓冲
};

struct device {
	int dev_id;							//设备ID号
	int fd;								//设备打开后的文件描述符
	int type;							//设备类型 1:网口设备，2:串中设备,3:一般设备
	union {								//设备属性类型
		struct net_attr net_attr;		//网络参数
		struct com_attr com_attr;		//串口参数
		struct int_attr int_attr;		//中断设备
	};
    struct sockaddr_in  dest_addr;
    int sock_len;
    
	sem_t *p_sem[2];					//［0］：数据接收，［1］：数据发送
	struct data_head data_head[3];		//数据 ［0］：接收数据，［1］：发送数据，［2］：文件描述符
	int (*open_dev)(struct device *pdev,struct dev_head *dev_head);	//设备打开操作函数
	int (*close_dev)(struct device *pdev,struct dev_head *dev_head);//设备关闭函数
	int (*send_data)(struct device *pdev,struct dev_head *dev_head);//设备中数据发送函数
	int (*recv_data)(struct device *pdev,struct dev_head *dev_head);//设备中数据接收函数
	struct list_head list_head;			//设备链表
};






static inline int add_list(struct list_head *new, struct list_head *head,pthread_rwlock_t *list_rwlock) {
	if(new == NULL)
		return FALSE;
	pthread_rwlock_wrlock(list_rwlock);
	list_add(new,head);
	pthread_rwlock_unlock(list_rwlock);
	return TRUE;
}

static inline int add_tail_list(struct list_head *new, struct list_head *head,pthread_rwlock_t *list_rwlock) {
	if(new == NULL)
		return FALSE;
	pthread_rwlock_wrlock(list_rwlock);
	list_add_tail(new,head);
	pthread_rwlock_unlock(list_rwlock);
	return TRUE;
}

static inline int del_list(struct list_head *entry,pthread_rwlock_t *list_rwlock) {
	pthread_rwlock_wrlock(list_rwlock);
	list_del(entry);
	pthread_rwlock_unlock(list_rwlock);
	return TRUE;
}

static inline int inc_data_count(struct data_head *data_head) {
	pthread_rwlock_wrlock(data_head->list_rwlock);
	data_head->count++;
	pthread_rwlock_unlock(data_head->list_rwlock);
	return TRUE;
}

static inline int dec_data_count(struct data_head *data_head) {
	pthread_rwlock_wrlock(data_head->list_rwlock);
	data_head->count--;
	pthread_rwlock_unlock(data_head->list_rwlock);
	return TRUE;
}

static inline int malloc_new(void **p_data,int len) {
	void * new_data;
	new_data = (void *)malloc(len);
	if(new_data == NULL)
		return FALSE;
	bzero(new_data,len);
	*p_data = new_data;
	return TRUE;
}

static inline int m_data_head(struct data_head **data_head) {
	struct data_head * new_head;
	new_head = (struct data_head *)malloc(sizeof(struct data_head));
	if(new_head == NULL)
		return FALSE;
	bzero(new_head,sizeof(struct data_head));
	*data_head = new_head;
	return TRUE;
}

static inline int m_data_list(struct data_list **data_list) {
	struct data_list * new_list;
	new_list = (struct data_list *)malloc(sizeof(struct data_list));
	if(new_list == NULL)
		return FALSE;
	bzero(new_list,sizeof(struct data_head));
	*data_list = new_list;
	return TRUE;
}

static inline void init_data_head(struct data_head *new_head,int type,pthread_rwlock_t *list_rwlock) {
	new_head->type = type;
	new_head->list_rwlock = list_rwlock;
	INIT_LIST_HEAD(&new_head->list_head);
}

static inline struct data_list *init_data_list(struct data_list **data_list) {
	struct data_list *new_list;
	if(!m_data_list(&new_list))
		return NULL;
	bzero(new_list,sizeof(struct data_list));
	INIT_LIST_HEAD(&new_list->list_head);
	*data_list = new_list;
	return new_list;
}

static inline struct data_list *find_list(int fd,struct data_head *data_head) {
	struct data_list *p_data_list;
	list_for_each_entry(p_data_list,&data_head->list_head,list_head) {
		if (p_data_list->net_lev.fd == fd)
			return p_data_list;
	}
	return NULL;
}

static inline struct device *find_dev(int dev_id,struct dev_head *dev_head) {
	struct device *p_dev;
	list_for_each_entry(p_dev,&dev_head->list_head,list_head) {
		if (p_dev->dev_id == dev_id)
			return p_dev;
	}
	return NULL;
}

static inline void *malloc_data(char **list_data,char *src_data,int len) {
	*list_data = (char *)malloc(len*sizeof(char));
	if (*list_data == NULL)
		return NULL;
    memset(*list_data,0,len);
	memcpy(*list_data,src_data,len);
	return *list_data;
}

static inline struct data_list *add_net_data(int fd,struct sockaddr_in *dev_sockaddr,struct data_head *data_head) {
	struct data_list *new_list;
	struct net_lev *p_net_lev;
	if(data_head == NULL)
		return NULL;
	if(!init_data_list(&new_list))
		return NULL;
	p_net_lev = &new_list->net_lev;
	memcpy(&p_net_lev->dev_sockaddr,dev_sockaddr,sizeof(struct sockaddr_in));
	p_net_lev->fd= fd;
	add_list(&new_list->list_head,&data_head->list_head,data_head->list_rwlock);
	inc_data_count(data_head);
	return new_list;
}

static inline struct data_list *add_recv_data(char *data,int fd,int len,struct data_head *data_head)	//增加接收数据单元到行向量链表中
{
	struct data_list *new_list;
	struct recv_lev *p_recv_lev;
	if(data_head == NULL)
		return NULL;
	time_t time_now;
	if(!init_data_list(&new_list))
		return NULL;
	p_recv_lev = &new_list->recv_lev;
	//time_now = time(NULL);
	malloc_data(&p_recv_lev->data,data,len);
	//p_recv_lev->recv_time = time_now;
	p_recv_lev->fd = fd;
	p_recv_lev->len = len;
	p_recv_lev->deal_time = 0;
	add_list(&new_list->list_head,&data_head->list_head,data_head->list_rwlock);
	inc_data_count(data_head);
	return new_list;
}

static inline struct data_list *add_send_data(char *data,int len,int type,struct data_head *data_head) {
	int *p_int = NULL;
	struct list_head *p_list = NULL;
	struct data_list *new_list;
	struct send_lev *p_send_lev;
	if(data_head == NULL)
		return NULL;
	time_t time_now;
	if(!init_data_list(&new_list))
		return NULL;
	p_send_lev = &new_list->send_lev;
	//time_now = time(NULL);

	malloc_data(&p_send_lev->data,data,len);
	p_send_lev->len = len;
	//p_send_lev->add_time = time_now;
	p_send_lev->send_time = 0;

	add_list(&new_list->list_head,&data_head->list_head,data_head->list_rwlock);
	inc_data_count(data_head);
	return new_list;
}

static inline struct data_list *add_net(int fd,struct sockaddr_in *dev_sockaddr,struct device *p_dev) {
	struct data_list *p_data_list = NULL;
	p_data_list = add_net_data(fd,dev_sockaddr,&p_dev->data_head[2]);
	return p_data_list;
}

static inline struct data_list *add_recv(char *data,int fd,int len,struct device *p_dev) {
	struct data_list *p_data_list = NULL;
	p_data_list = add_recv_data(data,fd,len,&p_dev->data_head[0]);
	sem_post(p_dev->p_sem[0]);
	return p_data_list;
}

static inline struct data_list *add_send(char *data,int len,struct device *p_dev) {
	struct data_list *p_data_list = NULL;
	p_data_list = add_send_data(data,len,0,&p_dev->data_head[1]);
	sem_post(p_dev->p_sem[1]);
	return p_data_list;
}

static inline int del_data_list(struct data_list *data_list,struct data_head *data_head) {
	del_list(&data_list->list_head,data_head->list_rwlock);
	switch(data_head->type) {
	case 1:
		free(data_list->recv_lev.data);
		break;
	case 2:
		free(data_list->send_lev.data);
		break;
	case 3:
		break;
	default :
		return FALSE;
		break;
	}
	free(data_list);
	dec_data_count(data_head);
	return TRUE;
}

static inline int del_data(struct data_list *data_list,struct data_head *data_head,struct device *p_dev) {
	del_list(&data_list->list_head,data_head->list_rwlock);
	switch(data_head->type) {
	case 1: /**接收  */
		//sem_trywait(p_dev->p_sem[0]);
		free(data_list->recv_lev.data);
        free(data_list);
	    dec_data_count(data_head);
		break;
	case 2: /**发送  */
		//sem_trywait(p_dev->p_sem[1]);
		free(data_list->send_lev.data);
        free(data_list);
	    dec_data_count(data_head);
		break;
	case 3:
		break;
	default :
		return FALSE;
	}
	
	return TRUE;
}

static inline int del_data_head(struct data_head *data_head) {
	free(data_head);
	return TRUE;
}

#endif /* LIST_CTRL_H_ */
