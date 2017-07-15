SUBDIRS	:= bin

SUBDIRS += libs
APPS = ptp

#LIBS	+= $(PWD)/test.a

OBJS	:= ptp.o
OBJS	+= fpga_time.o
OBJS	+= ptp_io.o
OBJS	+= ptp_proto.o
OBJS	+= msg.o
OBJS	+= arith.o
OBJS	+= comm_fun.o
OBJS	+= net.o
OBJS	+= net_process.o
OBJS	+= bmc.o
OBJS    += unicast_process.o
OBJS    += usually_process.o
OBJS	+= log.o

CROSS_COMPILE := gcc
ARM_CROSS_COMPILE := arm-xilinx-linux-gnueabi-gcc
#ARM_CROSS_COMPILE := /opt/arm-2010q1/bin/arm-none-linux-gnueabi-gcc
#ARM_CROSS_COMPILE 

ARM_CROSS_COMPILE_AR := arm-linux-ar

AR	:= $(ARM_CROSS_COMPILE_AR)
CC	:= $(ARM_CROSS_COMPILE)

INCLUDE	:= $(PWD)
CFLAGS  += -I$(INCLUDE)
#CFLAGS  += -Wall -O0 -I$(INCLUDE)
LDFLAGS	:= -lpthread -lm  -lrt

all: $(OBJS) 
	$(CC) -I$(INCLUDE) *.o $(LIBS) -o $(APPS) $(LDFLAGS)

install:
	cp $(PWD)/ptp $(PWD)/../
	
clean:
	rm -f $(APPS) $(OBJS) *.o
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
