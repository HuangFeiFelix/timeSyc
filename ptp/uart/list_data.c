#include "list_data.h"

/********************************************************************************************
* 函数名称:    delete_data
* 功能描述:    从链表中删除数据
* 输入参数:    struct netRecv_Data *p_netrecv_data  
               struct netList_Head *p_netlist_head  
* 输出参数:    无
* 返回值:      
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-5-8
               作者:     孙旭
               修改内容: 新生成函数

********************************************************************************************/

void delete_data(struct Uart_Data *p_netrecv_data,struct Uart_List_Head *p_netlist_head)
{
    pthread_rwlock_wrlock(&p_netlist_head->list_rwlock);
    list_del(&p_netrecv_data->list_head);
    p_netlist_head->count--;
    pthread_rwlock_unlock(&p_netlist_head->list_rwlock);
    free(p_netrecv_data->data);
    free(p_netrecv_data);
}

/********************************************************************************************
* 函数名称:    add_data
* 功能描述:    添加数据到链表中
* 输入参数:    char *data                          
               struct netList_Head*p_netlist_head  
* 输出参数:    无
* 返回值:      void
* 备注:        是否可重入
* 修改历史:
             1.日期:     2015-5-8
               作者:     孙旭
               修改内容: 新生成函数

********************************************************************************************/
void *add_send_data_after(char *data,short len,struct Uart_List_Head *p_netlist_head)
{
    struct Uart_Data *p_data;
    struct Uart_Data *pData,*pTmp;
    int i;
    
    p_data = (struct Uart_Data *)malloc(sizeof(struct Uart_Data));
    if(p_data == NULL)
        return NULL;

    bzero(p_data,sizeof(struct Uart_Data));

    p_data->data = (char *)malloc(len);
    if(p_data == NULL)
        return NULL;
    
    memcpy(p_data->data,data,len);
    p_data->len = len;
    
    #if 0
        for(i=0;i<p_data->len;i++)
            printf("%x ",p_data->data[i]);
        printf("\n");
    #endif
    
#if 1
        if(p_netlist_head->count >= 10)
        {
            list_for_each_entry_safe(pData,pTmp,&p_netlist_head->list_head_uart,list_head)
            {
                delete_data(pData,p_netlist_head);
                break;
            }
        }
#endif

    
    pthread_rwlock_wrlock(&p_netlist_head->list_rwlock);
    p_netlist_head->count++;
    list_add_tail(&p_data->list_head,&p_netlist_head->list_head_uart);
    pthread_rwlock_unlock(&p_netlist_head->list_rwlock);
    
    //printf("%d  %s\n",p_netlist_head->count,p_data->data);
    return p_data;	
}

void *add_send_data_before(char *data,short len,struct Uart_List_Head *p_netlist_head)
{
    struct Uart_Data *p_data;
    p_data = (struct Uart_Data *)malloc(sizeof(struct Uart_Data));
    if(p_data == NULL)
        return NULL;

    bzero(p_data,sizeof(struct Uart_Data));

    p_data->data = (char *)malloc(len);
    if(p_data == NULL)
        return NULL;

    memcpy(p_data->data,data,len);
    p_data->len = len;
    
    pthread_rwlock_wrlock(&p_netlist_head->list_rwlock);
    p_netlist_head->count++;
    list_add(&p_data->list_head,&p_netlist_head->list_head_uart);
    pthread_rwlock_unlock(&p_netlist_head->list_rwlock);
    
    //printf("%d  %s\n",p_netlist_head->count,p_data->data);
    return p_data;	
}


void *add_recv_data_after(char *data,short len,struct Uart_List_Head *p_netlist_head)
{
    struct Uart_Data *p_data;
    struct Uart_Data *pData,*pTmp;
    int i;
    
    p_data = (struct Uart_Data *)malloc(sizeof(struct Uart_Data));
    if(p_data == NULL)
        return NULL;

    bzero(p_data,sizeof(struct Uart_Data));

    p_data->data = (char *)malloc(len);
    if(p_data == NULL)
        return NULL;
    
    memcpy(p_data->data,data,len);
    p_data->len = len;
    
    #if 0
        for(i=0;i<p_data->len;i++)
            printf("%x ",p_data->data[i]);
        printf("\n");
    #endif
    
    pthread_rwlock_wrlock(&p_netlist_head->list_rwlock);
    p_netlist_head->count++;
    list_add_tail(&p_data->list_head,&p_netlist_head->list_head_uart);
    pthread_rwlock_unlock(&p_netlist_head->list_rwlock);

    //printf("%d  %s\n",p_netlist_head->count,p_data->data);
    return p_data;	
}

void *add_recv_data_before(char *data,short len,struct Uart_List_Head *p_netlist_head)
{
    struct Uart_Data *p_data;
    p_data = (struct Uart_Data *)malloc(sizeof(struct Uart_Data));
    if(p_data == NULL)
        return NULL;

    bzero(p_data,sizeof(struct Uart_Data));

    p_data->data = (char *)malloc(len);
    if(p_data == NULL)
        return NULL;

    memcpy(p_data->data,data,len);
    p_data->len = len;
    
    pthread_rwlock_wrlock(&p_netlist_head->list_rwlock);
    p_netlist_head->count++;
    list_add(&p_data->list_head,&p_netlist_head->list_head_uart);
    pthread_rwlock_unlock(&p_netlist_head->list_rwlock);
    
    //printf("%d  %s\n",p_netlist_head->count,p_data->data);
    return p_data;	
}


