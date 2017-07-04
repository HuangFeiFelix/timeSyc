#ifndef _LIST_DATA_H_
#define _LIST_DATA_H_

#include "common.h"
#include "list.h"


typedef enum {
    PRIORITY_BEFORE = 0,/**���ȼ����  */
    PRIORITY_AFTER,    /** ���ȼ���� */
}EnumDataPriority;

/** �������� */
struct Uart_List_Head{
    int count;                                          /** ��ǰ���ݸ��� */
    pthread_rwlock_t list_rwlock;                       /** ������д�� */
    struct list_head list_head_uart;                     /** ���ݽ������� */
};


/** �������ݽṹ�� */
struct Uart_Data{
    char                *data;                                         /** ���� */
    int                 len;                                         /** ���ݵĳ���*/
    short               send_cnt;                                   /**���ʹ��������ڳ���3��ɾ��  */
    time_t              recv_time,deal_time;                         /** ��������ʱ�� */
    struct list_head    list_head;
};



extern void delete_data(struct Uart_Data *p_netrecv_data,struct Uart_List_Head *p_netlist_head);
extern void *add_send_data_after(char *data,short len,struct Uart_List_Head *p_netlist_head);
extern void *add_send_data_before(char *data,short len,struct Uart_List_Head *p_netlist_head);

extern void *add_recv_data_after(char *data,short len,struct Uart_List_Head *p_netlist_head);
extern void *add_recv_data_before(char *data,short len,struct Uart_List_Head *p_netlist_head);
#endif
