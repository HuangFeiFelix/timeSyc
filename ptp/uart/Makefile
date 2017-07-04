SUBDIRS	:= bin

SUBDIRS += libs

#LIBS	+= $(PWD)/test.a


OBJS	+= uart_main.o
OBJS	+= uart_handle.o
OBJS	+= list_data.o


#CROSS_COMPILE := gcc
#ARM_CROSS_COMPILE := arm-xilinx-linux-gnueabi-gcc
#ARM_CROSS_COMPILE := /opt/arm-2010q1/bin/arm-none-linux-gnueabi-gcc
#ARM_CROSS_COMPILE :=/home/arm-gcc-3.4.5/gcc-3.4.5-glibc-2.3.6/arm-softfloat-linux-gnu/bin/arm-softfloat-linux-gnu-gcc-3.4.5
ARM_CROSS_COMPILE_AR := arm-linux-ar

AR	:= $(ARM_CROSS_COMPILE_AR)
CC	:= $(ARM_CROSS_COMPILE)

#CFLAGS  += -I$(INCLUDE)
#CFLAGS  += -Wall -O0 -I$(INCLUDE)
LDFLAGS	:= -lpthread -lm  -lrt

all: $(OBJS) 
	cp *.o $(BIN_DIR)/

install:
	cp $(PWD)/exe $(PWD)/bin

clean:
	rm -f $(APPS) $(OBJS)
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
