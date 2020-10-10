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
#define  BACK_TYPE_MASK      0xF0000000   /*设备的底板类型*/

/*自生产板卡类型最高字四位D31-D28必须大于0,否则当维护软件生成dwName
  时会与sys.h中Cfg Attr定义冲突*/

#define  BACK_TYPE_IPACS       0x10000000   /*Ida*/

#define  EXT_TYPE_MASK       0x0FFF0000   /*扩充接口板定义*/
#define  EXT_TYPE_DIO_MASK   0x000F0000   /*数字接口板定义*/
#define  EXT_TYPE_AIO_MASK   0x0FF00000   /*模拟接口板定义*/

#define  FUN_TYPE_MASK       0x0000F000   /*产品功能类型*/

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
#define  OS_RT              0x00000001    

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

/*------------------------------------------------------------------------
 用户类型定义
------------------------------------------------------------------------*/
#define  USER_DEFAULT	          0xFFFFFF
#define  USER_DEFAULT_CN          "缺省"
#define  USER_DEFAULT_EN          ""

#define  USER_MULTIMODE	          0x1
#define  USER_MULTIMODE_CN        "多模"
#define  USER_MULTIMODE_EN        "multi"

#define  USER_GUIYANG	      0x0A
#define  USER_GUIYANG_CN       "贵阳"
#define  USER_GUIYANG_EN       "guiyang"

/*------------------------------------------------------------------------
产品定制
------------------------------------------------------------------------*/
#define  DEV_SP                   DEV_SP_DTU
#define  DEV_CPU                  CPU_ZYNQ

#define  TYPE_USER		   		  USER_DEFAULT

#define INCLUDE_ETH
#define INCLUDE_TIMER
#undef INCLUDE_MY_GPS
#undef INCLUDE_AD

#define INCLUDE_EXTRTC
#undef  INCLUDE_RTC

#define INCLUDE_YX
#define INCLUDE_EXT_YX
#define INCLUDE_EXT_YC
#define INCLUDE_DYX
#define INCLUDE_YK
#define INCLUDE_YC
#undef INCLUDE_AI_FREQ
#undef INCLUDE_DD  
#undef INCLUDE_AI_HTM

#define INCLUDE_PR
#undef INCLUDE_COMTRADE  
#define INCLUDE_RECORD
#undef INCLUDE_PR_PRO

#undef INCLUDE_FRZ
#define INCLUDE_HIS
#undef INCLUDE_CELL
#undef INCLUDE_USWITCH
#undef INCLUDE_WEB_SERVER

#define COMM_SP    COMM_SP_ALL//默认以太网应用

#define INCLUDE_PCOL
#define INCLUDE_PSLAVE
#define INCLUDE_PMASTER
#undef  INCLUDE_GB104_M
#undef  INCLUDE_GB104_S
#undef  INCLUDE_GB101_M
#undef  INCLUDE_GB101_S
#define  INCLUDE_MODBUS_M

#define  INCLUDE_ECC
#define  PASSTHROUGH

#if (DEV_CPU == CPU_SAM9X25)
#define ECC_MODE_CHIP   //硬件加密
#define ECC_MODE_OLDCHIP //老加密
#else
#define ECC_MODE_CHIP
#undef ECC_MODE_OLDCHIP //老加密
#endif

#define INCLUDE_B2F         //buf to file
#define INCLUDE_B2F_EVT     //事件写文件
#define INCLUDE_B2F_SOE     //SOE写文件
#undef INCLUDE_B2F_COS      //COS写文件, 默认关闭
#define INCLUDE_B2F_F       //上述读写指针写文件



#if (DEV_SP == DEV_SP_DTU_FA)
#undef DEV_SP
#define DEV_SP     DEV_SP_DTU
#define INCLUDE_FA
#define INCLUDE_NET_GOOSE
#define INCLUDE_FA_PUB
#endif

#if (DEV_SP == DEV_SP_DTU_RGFA)
#undef DEV_SP
#define DEV_SP     DEV_SP_DTU
#define INCLUDE_FA
#define INCLUDE_FA_IDIR
#define PR_SHTQ_GZ863
#define INCLUDE_FA_PUB
#endif

#if (DEV_SP == DEV_SP_DTU_SHFA)
#undef DEV_SP
#define DEV_SP     DEV_SP_DTU
#define INCLUDE_FA_SH
#define INCLUDE_FA_PUB
#endif

#if (DEV_SP == DEV_SP_DTU_FADIFF)
#undef DEV_SP
#define DEV_SP     DEV_SP_DTU
#undef INCLUDE_FA
#define INCLUDE_NET_GOOSE
#define INCLUDE_FA_PUB
#define INCLUDE_FA_DIFF
#define INCLUDE_PR_DIFF
#endif


#if(TYPE_USER == USER_MULTIMODE)
#define INCLUDE_FA_S
#define INCLUDE_FA
#define INCLUDE_MDCP
#if(DEV_SP == DEV_SP_FTU) //只有箱式FTU才支持IRIGB对时
#define IRIGB_FLAG
#endif
#define INCLUDE_FA_PUB
#endif

#if (DEV_SP == DEV_SP_DTU)
#define TYPE_CPU    DEV_CPU
#define TYPE_OS     OS_RT
#define TYPE_IO     IO_TYPE_LOCAL

#define INCLUDE_EXT_DISP
#undef INCLUDE_HMI400
#define INCLUDE_AD

#undef INCLUDE_AD7913
#undef INCLUDE_AD7913_TEMP
#undef INCLUDE_CELL

#define INCLUDE_GB104_S
#define INCLUDE_GB101_S
#define INCLUDE_GB101_M
#define INCLUDE_GB104_M

#define YC_GAIN_NEW
#define INCLUDE_JD_PR
#undef INCLUDE_GZ101_M
#undef INCLUDE_XML_MODEL
#undef INCLUDE_YK_KEEP
#undef INCLUDE_SEC_CHIP
#undef INCLUDE_YK_CZDY //操作电源
#undef PR_ZJJH
#endif

#ifndef INCLUDE_YC
#undef INCLUDE_DD    
#undef INCLUDE_PR
#undef INCLUDE_JD_PR
#undef INCLUDE_COMTRADE
#undef INCLUDE_RECORD
#undef INCLUDE_AD
#undef INCLUDE_USWITCH
#undef INCLUDE_PB_RELATE
#undef INCLUDE_HW2_PROTECT
#endif

#ifndef INCLUDE_YX
#undef INCLUDE_DYX    
#endif

#ifndef TYPE_CPU
#error "Not define TYPE_CPU"
#endif
#ifndef TYPE_OS
#error "Not define TYPE_OS"
#endif
#ifndef TYPE_IO
#error "Not define TYPE_IO"
#endif


#if (TYPE_IO == IO_TYPE_LOCAL)
#undef INCLUDE_MY_GPS
#endif

#if (COMM_SP == COMM_SP_GPRS)
#define  INCLUDE_PMASTER
#define  INCLUDE_GB101_S
#define  INCLUDE_MODBUS_M
#endif


#define INCLUDE_SYS_LOG
#define INCULDE_COS_CFG    //COS特别处理,默认关闭
#define INCLUDE_FLAG_CUR   //遥测非当前值品质描述,默认关闭

#undef _YIERCI_RH_//一二次融合宏定义
#undef _TEST_VER_
#undef NARI_FA_TEST

#undef _GUANGZHOU_TEST_FTU_

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

#ifdef INCLUDE_NET_GOOSE
#define INCLUDE_MYGOOSE_RX
#define INCLUDE_MYGOOSE_TX
#define INCLUDE_MYGOOSE_RXMODE_TASK
#endif

#ifdef INCLUDE_SYS_LOG
#define SYS_LOG_MSGLEN    128
#define SYS_LOG_BUFNUM    128
#endif

#include "mytypes.h"


#define LOG_ATTR_INFO     0
#define LOG_ATTR_WARN     1
#define LOG_ATTR_ERROR    2

#endif

