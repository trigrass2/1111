/*------------------------------------------------------------------------
 Module:       	comm_cfg.h
 Author:        solar
 Project:       
 Creation Date:	2008-08-04
 Description:   通讯口配置文件
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/

#ifndef _COMM_CFG_H
#define _COMM_CFG_H

#include "syscfg.h"

#include "os.h"

#define INCLUDE_SERIAL
#define INCLUDE_UART

#define INCLUDE_NET

#define INCLUDE_COMM_FILE

#define INCLUDE_COMM_SHOW

#if (TYPE_CPU == CPU_HUAWEI) 
#define COMM_SERIAL_START_NO      THREAD_MIN_NUM
#define COMM_SERIAL_NUM           6   
#define COMM_NET_SYS_NUM          2
#endif

#if (TYPE_CPU == CPU_ZYNQ) 
#define COMM_SERIAL_START_NO      THREAD_MIN_NUM
#define COMM_SERIAL_NUM           10
#define COMM_NET_SYS_NUM          8
#endif



#define COMM_NET_START_NO         (COMM_SERIAL_START_NO+COMM_SERIAL_NUM)
#define COMM_NET_USR_NO           (COMM_NET_START_NO+COMM_NET_SYS_NUM)
#define COMM_CAN_STARTNO          (COMM_NET_START_NO-1)
#define COMM_WIFI_STARTNO         (COMM_CAN_STARTNO-1)
#define COMM_START_NO             COMM_SERIAL_START_NO

extern int  COMM_NET_NUM;

#if (COMM_SERIAL_START_NO > THREAD_MIN_NUM)
#error "COMM_SERIAL_START_NO must <= THREAD_MIN_NUM"
#endif

#endif /*_COMM_CFG_H*/
