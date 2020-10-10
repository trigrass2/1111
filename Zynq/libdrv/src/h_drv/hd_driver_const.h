/*
 * @Author: zhongwei
 * @Date: 2020/4/5 9:55:49
 * @Description: 硬件定义
 * @File: hd_driver_const.h
 *
*/

#ifndef SRC_H_DRV_HD_DRIVER_CONST_H_
#define SRC_H_DRV_HD_DRIVER_CONST_H_

/************************************************************************/
//  MIO/EMIO定义
//  主要用于定义装置开入和开出
//  开入有两种：点对点和总线型开入
//  点对点开入定义(共8个)：EMIO7-EMIO14
//  总线型开入,8个输入管脚  EMIO15-EMIO22
//      另外有４根地址线，来片选８个总线型开入具体接那个芯片EMIO3-EMIO6
//      目前板子上有４块总线型开入芯片　地址为０～３
// 
//  开出都是点对点的
//  MIO9      CPU1_STALL        CPU喂狗的脉冲信号，翻转时间小于4.5ms，否则装置闭锁
//  MIO28     CPU1_IO_BSJ       CPU主动闭锁信号，高电平有效，正常工作需输出低电平，不驱动闭锁总线
//  MIO29     CPU1_IO_BJJ       CPU主动报警信号，高电平有效，正常工作需输出低电平，不驱动报警总线
//  MIO30     CPU1_IO_QD        CPU启动信号，高电平有效，正常工作需输出低电平，不驱动启动总线
//  MIO31     BSJ_CPU1_IN       装置闭锁回读信号，检测到高电平，装置闭锁
//  MIO32     BJJ_CPU1_IN       装置报警回读信号，检测到高电平，装置报警
//  MIO33     QD_CPU1_IN        装置启动回读信号，检测到高电平，装置启动
//  MIO16-    MIO23             点对点开出1—开出8
//  MIO24-    MIO27             操作回路开出1—开出4
//  MIO47                       操作回路开出5
//  MIO52-    MIO53             操作回路开出6和开出7
//  EMIO1     开出
//  EMIO2     复归开出 
/************************************************************************/
#define EMIO_BASE           54              //在EMIO上的GPIO开始编号
/* 总线型开入定义 */
#define GPIO_IN_BUS_ADDR0   (EMIO_BASE+3)   //总线型开入地址0
#define GPIO_IN_BUS_ADDR1   (EMIO_BASE+4)   //总线型开入地址1
#define GPIO_IN_BUS_ADDR2   (EMIO_BASE+5)   //总线型开入地址2
#define GPIO_IN_BUS_ADDR3   (EMIO_BASE+6)   //总线型开入地址3
#define GPIO_IN_BUS_ADDR_CNT 4              //4根地址线
#define GPIO_IN_BUS_DATA0   (EMIO_BASE+15)  //总线型开入数据0
#define GPIO_IN_BUS_DATA1   (EMIO_BASE+16)  //总线型开入数据1
#define GPIO_IN_BUS_DATA2   (EMIO_BASE+17)  //总线型开入数据2
#define GPIO_IN_BUS_DATA3   (EMIO_BASE+18)  //总线型开入数据3
#define GPIO_IN_BUS_DATA4   (EMIO_BASE+19)  //总线型开入数据4
#define GPIO_IN_BUS_DATA5   (EMIO_BASE+20)  //总线型开入数据5
#define GPIO_IN_BUS_DATA6   (EMIO_BASE+21)  //总线型开入数据6
#define GPIO_IN_BUS_DATA7   (EMIO_BASE+22)  //总线型开入数据7
#define GPIO_IN_BUS_DATA_CNT 8              //8根数据线

#define GPIO_OUT_STALL      9               //CPU喂狗的脉冲信号，翻转时间小于4.5ms，否则装置闭锁
#define GPIO_OUT_RETURN     (EMIO_BASE+2)   //复归开出
//#define GPIO_OUT_BSJ        28
//#define GPIO_OUT_BJJ        29
//#define GPIO_OUT_QD         30
//#define GPIO_IN_BSJ         31
//#define GPIO_IN_BJJ         32
//#define GPIO_IN_QD          33



#endif /* SRC_H_DRV_HD_DRIVER_CONST_H_ */
