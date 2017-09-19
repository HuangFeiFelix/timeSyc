/* comm_api.h */
#ifndef	COMM_API_H
#define COMM_API_H

#include "list_ctrl.h"


int open_port(int com_port);
int set_com_config(int fd,int baud_rate, int data_bits, char parity, int stop_bits);
void add_uart(struct uart_buf *p_uart, char buf);

#endif 
