/*------------------------------------------------------------------------
 Module:		syscfg.h
 Author:		solar
 Project:		
 Creation Date: 2008-9-16
 Description:	
------------------------------------------------------------------------*/

#ifndef _SYSCFG_H
#define _SYSCFG_H

/*------------------------------------------------------------------------
 设备类型定义

 四字节组成: BACK-EXT-FUN-RSV 
------------------------------------------------------------------------*/
/*自生产板卡类型最高字四位D31-D28必须大于0,否则当维护软件生成dwName
  时会与sys.h中Cfg Attr定义冲突*/
#define  BACK_TYPE_IPACS       0x10000000   /*Ida*/

/*------------------------------------------------------------------------
IO扩展模式
------------------------------------------------------------------------*/
#define  IO_TYPE_MASTER      0x00000001   /*MASTER,可采集扩展IO*/
#define  IO_TYPE_LOCAL       0x00000002   /*仅本地IO功能*/
#define  IO_TYPE_EXTIO       0x00000003   /*仅扩展IO自身功能*/

/*------------------------------------------------------------------------
CPU
------------------------------------------------------------------------*/
#define  CPU_STM32F4         0x00000001   
#define  CPU_SAM9X25         0x00000002
#define  CPU_HUAWEI          0x00000003
#define  CPU_ZYNQ            0x00000004
/*------------------------------------------------------------------------
OS
------------------------------------------------------------------------*/
#define  OS_LINUX           0x00000001 
/*------------------------------------------------------------------------
通信定制
------------------------------------------------------------------------*/
#define  COMM_SP_ALL         0x00000001   /*全应用*/
#define  COMM_SP_NET         0x00000002   /*以太网应用为主*/
#define  COMM_SP_GPRS        0x00000003   /*GPRS应用为主*/

/*------------------------------------------------------------------------
设备扩展(补丁)
------------------------------------------------------------------------*/
#define  DEV_SP_NONE	           0xFFFFFFFF
#define  DEV_SP_NONE_CN            "无"
#define  DEV_SP_NONE_EN            ""

#define  DEV_SP_MASTER	           1
#define  DEV_SP_MASTER_CN          "主板"
#define  DEV_SP_MASTER_EN          "Master"

#define  DEV_SP_FTU	               2
#define  DEV_SP_FTU_CN             "FTU"
#define  DEV_SP_FTU_EN             "FTU"

#define  DEV_SP_FTU_GPRS	       21
#define  DEV_SP_FTU_GPRS_CN        "FTU_GPRS"
#define  DEV_SP_FTU_GPRS_EN        "FTU_GPRS"

#define  DEV_SP_FTU_FA             22
#define  DEV_SP_FTU_FA_CN          "FTU_FA"
#define  DEV_SP_FTU_FA_EN          "FTU_FA"

#define  DEV_SP_FTU_RGFA           23
#define  DEV_SP_FTU_RGFA_CN        "FTU_RGFA"
#define  DEV_SP_FTU_RGFA_EN        "FTU_RGFA"

#define  DEV_SP_FTU_SHFA           24
#define  DEV_SP_FTU_SHFA_CN        "FTU_SHFA"
#define  DEV_SP_FTU_SHFA_EN        "FTU_SHFA"


#define  DEV_SP_WDG	               3
#define  DEV_SP_WDG_CN             "WDG"
#define  DEV_SP_WDG_EN             "WDG"

#define  DEV_SP_WDG_FA	           32
#define  DEV_SP_WDG_FA_CN          "WDG_FA"
#define  DEV_SP_WDG_FA_EN          "WDG_FA"

#define  DEV_SP_WDG_RGFA	       33
#define  DEV_SP_WDG_RGFA_CN        "WDG_RGFA"
#define  DEV_SP_WDG_RGFA_EN        "WDG_RGFA"

#define  DEV_SP_DTU 	           4
#define  DEV_SP_DTU_CN             "DTU"
#define  DEV_SP_DTU_EN             "DTU"

#define  DEV_SP_DTU_FA             42
#define  DEV_SP_DTU_FA_CN          "DTU_FA"
#define  DEV_SP_DTU_FA_EN          "DTU_FA"

#define  DEV_SP_DTU_RGFA           43
#define  DEV_SP_DTU_RGFA_CN        "DTU_RGFA"
#define  DEV_SP_DTU_RGFA_EN        "DTU_RGFA"

#define  DEV_SP_DTU_SHFA           44
#define  DEV_SP_DTU_SHFA_CN        "DTU_SHFA"
#define  DEV_SP_DTU_SHFA_EN        "DTU_SHFA"

#define  DEV_SP_DTU_FADIFF         45
#define  DEV_SP_DTU_FADIFF_CN      "DTU_FADIFF"
#define  DEV_SP_DTU_FADIFF_EN      "DTU_FADIFF"

#define  DEV_SP_TTU	               5
#define  DEV_SP_TTU_CN             "TTU"
#define  DEV_SP_TTU_EN             "TTU"

#define  DEV_SP_CTRL	           6
#define  DEV_SP_CTRL_CN            "CTRL"
#define  DEV_SP_CTRL_EN            "CTRL"

#define  DEV_SP_DTU_IU	           7
#define  DEV_SP_DTU_IU_CN          "DTU_IU"
#define  DEV_SP_DTU_IU_EN          "DTU_IU"

#define  DEV_SP_DTU_PU	           8
#define  DEV_SP_DTU_PU_CN          "DTU_PU"
#define  DEV_SP_DTU_PU_EN          "DTU_PU"

/*------------------------------------------------------------------------
 用户类型定义
------------------------------------------------------------------------*/
#define  USER_DEFAULT	          0xFFFFFF
#define  USER_DEFAULT_CN          "缺省"
#define  USER_DEFAULT_EN          ""

#define  USER_MULTIMODE	          0x1
#define  USER_MULTIMODE_CN        "多模"
#define  USER_MULTIMODE_EN        "multi"

/*------------------------------------------------------------------------
产品定制
------------------------------------------------------------------------*/
#define  DEV_SP                   DEV_SP_DTU  // DEV_SP_TTU        
#define  DEV_CPU                  CPU_ZYNQ // CPU_HUAWEI

#define  TYPE_USER		   		  USER_DEFAULT

#define INCLUDE_YX
#define INCLUDE_YK
#define INCLUDE_YC
#define INCLUDE_EXT_YC
#undef INCLUDE_DD  
#define INCLUDE_CELL
#define INCLUDE_USWITCH

#define INCLUDE_PR
#undef INCLUDE_COMTRADE  
#define INCLUDE_RECORD

#define COMM_SP    COMM_SP_ALL//默认以太网应用

#define INCLUDE_PCOL
#define INCLUDE_PSLAVE
#define INCLUDE_PMASTER
#define  INCLUDE_MODBUS_M
#define  INCLUDE_GB104_M
#define  INCLUDE_GB104_S
#define  INCLUDE_GB101_M
#define  INCLUDE_GB101_S


#define INCLUDE_B2F         //buf to file
#define INCLUDE_B2F_EVT     //事件写文件
#define INCLUDE_B2F_SOE     //SOE写文件
#undef INCLUDE_B2F_COS      //COS写文件, 默认关闭
#define INCLUDE_B2F_F       //上述读写指针写文件


#if (DEV_SP == DEV_SP_FTU)
#define TYPE_CPU    DEV_CPU
#define TYPE_OS     OS_LINUX
#define TYPE_IO     IO_TYPE_LOCAL

#undef INCLUDE_EXT_DISP

#define YC_GAIN_NEW
#endif


#if (DEV_SP == DEV_SP_DTU)
#define TYPE_CPU    DEV_CPU
#define TYPE_OS     OS_LINUX
#define TYPE_IO     IO_TYPE_LOCAL

#undef INCLUDE_EXT_DISP

#define INCLUDE_AD7606

#define INCLUDE_AD7913
#define INCLUDE_AD7913_TEMP
#define INCLUDE_CELL

#define YC_GAIN_NEW

#define INCLUDE_SOC
#endif


#if (DEV_SP == DEV_SP_TTU)
#define TYPE_CPU    DEV_CPU
#define TYPE_OS     OS_LINUX
#define TYPE_IO     IO_TYPE_LOCAL

#define INCLUDE_HIS_TTU
#define INCLUDE_FRZ
#define INCLUDE_DD 
//#define INCLUDE_ADE9078
//#define INCLUDE_ATT7022E

#define YC_GAIN_NEW
#define INCLUDE_PB_RELATE
#endif


#ifndef INCLUDE_YC
#undef INCLUDE_DD    
#undef INCLUDE_PR
#undef INCLUDE_JD_PR
#undef INCLUDE_COMTRADE
#undef INCLUDE_RECORD
#undef INCLUDE_AD7606
#undef INCLUDE_AD7607
#undef INCLUDE_USWITCH
#undef INCLUDE_PB_RELATE
#undef INCLUDE_HW2_PROTECT
#endif


#ifndef TYPE_CPU
#error "Not define TYPE_CPU"
#endif
#ifndef TYPE_OS
#error "Not define TYPE_OS"
#endif

#define INCLUDE_SYS_LOG
#define INCULDE_COS_CFG    //COS特别处理,默认关闭
#define INCLUDE_FLAG_CUR   //遥测非当前值品质描述,默认关闭

#undef _TEST_VER_

#ifdef _TEST_VER_
#undef INCLUDE_B2F_COS    //COS写文件, 默认关闭
#define INCULDE_COS_CFG    //COS特别处理,默认关闭
#define INCLUDE_DYX
#undef INCLUDE_DD
#define INCLUDE_FLAG_CUR   //品质描述之当前值
#endif
#define INCLUDE_COMMTEST

#ifndef INCLUDE_B2F
#undef INCLUDE_B2F_EVT    
#undef INCLUDE_B2F_SOE    
#undef INCLUDE_B2F_COS    
#undef INCLUDE_B2F_F   
#endif
#ifdef INCLUDE_SYS_LOG
#define SYS_LOG_MSGLEN    128
#define SYS_LOG_BUFNUM    128
#endif


#include "mytypes.h"
#include "task.h"

#endif

