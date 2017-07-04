SUBDIRS	:= bin

SUBDIRS += libs

#LIBS	+= $(PWD)/test.a

#OBJS	:= CfgManage.o
OBJS	+= ClientIpFilter.o
#OBJS	+= CommFun.o

OBJS	+= Md5Manager.o
OBJS	+= NetWorkConfig.o
OBJS	+= NtpdConfigOperation.o
OBJS	+= NtpMain.o
OBJS	+= PingManager.o

#CROSS_COMPILE := gcc
#ARM_CROSS_COMPILE := /opt/usr/local/arm/3.4.1/bin/arm-linux-gcc-3.4.1
ARM_CROSS_COMPILE_AR := arm-linux-ar

AR	:= $(ARM_CROSS_COMPILE_AR)
CC	:= $(ARM_CROSS_COMPILE)

#CFLAGS  += -Wall -O2 -I$(INCLUDE)
LDFLAGS	:= -lpthread -lm

all: $(OBJS) 
	cp *.o $(BIN_DIR)/

install:
	cp $(PWD)/ntpserver $(PWD)/bin

clean:
	rm -f $(APPS) $(OBJS)
	
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
