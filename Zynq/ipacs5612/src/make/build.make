#==================================================
#	工程：		build.make
#				makefile头文件
#==================================================

# 平台类型
ARCH ?=arm#默认arm平台

ifeq ("$(ARCH)", "arm")
	ARM=1
	export ARM
endif


ifdef ARM
	CC = arm-linux-gnueabihf-gcc
	CPP = arm-linux-gnueabihf-g++
	AR = arm-linux-gnueabihf-ar
else
	CC = gcc
	CPP = g++
	AR = ar
endif

#---------路径设置-------------

#编译工作目录 arm/*
PREVDIR ?= $(shell cd ..;pwd)

INSTALL_LIB_PATH =$(PREVDIR)/exe/lib
INSTALL_BIN_PATH =$(PREVDIR)/exe/bin

#头文件目录
INCLUDE_PATH = $(PREVDIR)/source/h
XML_CFG_PATH = $(PREVDIR)/common/xml
COMMON_PATH	 = $(PREVDIR)/platform/Src/Common
INSTALL_PATH = $(PREVDIR)/exe
OS_PATH = $(PREVDIR)/make/os

HEARDPATH = -I $(INCLUDE_PATH)
INCLUDEDIR =$(HEARDPATH) \
			$(HEARDPATH)/protocol/gb104  \
			$(HEARDPATH)/protocol/  \
			$(HEARDPATH)/protocol/public  \
			$(HEARDPATH)/system \
			$(HEARDPATH)/system/bsp \
			$(HEARDPATH)/system/os  \
			$(HEARDPATH)/system/common \
			$(HEARDPATH)/system/comm  \
			$(HEARDPATH)/system/myio  \
			$(HEARDPATH)/system/myio/yc  \
			$(HEARDPATH)/system/myio/yc/drv\
			$(HEARDPATH)/system/myio/yx \
			$(HEARDPATH)/system/myio/yk \
			$(HEARDPATH)/system/db  \
			$(HEARDPATH)/system/drv  \
			$(HEARDPATH)/system/comm/lan  \
			$(HEARDPATH)/system/comm/serial \
			$(HEARDPATH)/system/comm/serial/uart \
			$(HEARDPATH)/system/comm/serial \
			$(HEARDPATH)/system/comm/serial/uart\
			$(HEARDPATH)/system/myio/pb \
			$(HEARDPATH)/protocol/down485 \
			$(HEARDPATH)/protocol/maint  \
			$(HEARDPATH)/product/pb  \
			$(HEARDPATH)/bmlinux   \
			$(HEARDPATH)/product/mmi   \
			$(HEARDPATH)/product/mmi/extmmi  \
			$(HEARDPATH)/product/protect 
			
			
#编译选择

#宏定义define
ARRER_JS_DEF = -D ARRER_JS  -D _MANG_UNIT_
DEFINED_OPTION = $(ARRER_JS_DEF)
