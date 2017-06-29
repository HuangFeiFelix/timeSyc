SUBDIRS	:= bin

SUBDIRS += libs
APPS = tsync

#LIBS	+= $(PWD)/test.a

OBJS	:= clock_rb.o
OBJS	+= main.o
OBJS	+= comm_api.o
OBJS	+= satellite_api.o
OBJS	+= log.o
OBJS	+= net.o
OBJS	+= manager.o
OBJS	+= lcd_cmd_process.o
OBJS	+= lcd_driver.o

CROSS_COMPILE := gcc
ARM_CROSS_COMPILE := arm-xilinx-linux-gnueabi-gcc
#ARM_CROSS_COMPILE := /opt/arm-2010q1/bin/arm-none-linux-gnueabi-gcc
#ARM_CROSS_COMPILE 

ARM_CROSS_COMPILE_AR := arm-linux-ar

AR	:= $(ARM_CROSS_COMPILE_AR)
CC	:= $(CROSS_COMPILE)

INCLUDE	:= $(PWD)
CFLAGS  += -I$(INCLUDE)
#CFLAGS  += -Wall -O0 -I$(INCLUDE)
LDFLAGS	:= -lpthread -lm  -lrt

all: $(OBJS) 
	$(CC) -I$(INCLUDE) *.o $(LIBS) -o $(APPS) $(LDFLAGS)

install:
	cp $(PWD)/tsync $(PWD)/../
	
clean:
	rm -f $(APPS) $(OBJS) *.o
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
