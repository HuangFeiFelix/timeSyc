#ifndef _LIST_DATA_H_
#define _LIST_DATA_H_

#include "common.h"
#include "list.h"


typedef enum {
    PRIORITY_BEFORE = 0,/**优先级最高  */
    PRIORITY_AFTER,    /** 优先级其次 */
}EnumDataPriority;

/** 串口数据 */
struct Uart_List_Head{
    int count;                                          /** 当前数据个数 */
    pthread_rwlock_t list_rwlock;                       /** 操作读写锁 */
    struct list_head list_head_uart;                     /** 数据接收链表 */
};


/** 接收数据结构体 */
struct Uart_Data{
    char                *data;                                         /** 数据 */
    int                 len;                                         /** 数据的长度*/
    short               send_cnt;                                   /**发送次数，用于超过3次删除  */
    time_t              recv_time,deal_time;                         /** 处理数据时间 */
    struct list_head    list_head;
};



extern void delete_data(struct Uart_Data *p_netrecv_data,struct Uart_List_Head *p_netlist_head);
extern void *add_send_data_after(char *data,short len,struct Uart_List_Head *p_netlist_head);
extern void *add_send_data_before(char *data,short len,struct Uart_List_Head *p_netlist_head);

extern void *add_recv_data_after(char *data,short len,struct Uart_List_Head *p_netlist_head);
extern void *add_recv_data_before(char *data,short len,struct Uart_List_Head *p_netlist_head);
#endif
