/*
 * @Author: zhongwei
 * @Date: 2020/3/25 14:48:03
 * @Description: libcpp对外提供的接口定义
 * @File: cpp_port.h
 *
*/

#ifndef SRC_CPP_PORT_H_
#define SRC_CPP_PORT_H_


/************************************************************************/
//  内部通讯处理接口
//
/************************************************************************/
//内部通讯初始化 需要在 PowerOn_Init_McInnerComm 后调用
void PowerOn_Initial_Inner_Comm(void);

//在通讯任务中调用
void inner_comm_handle_intask(const TTimeRun * pTimeRun);

#endif /* SRC_CPP_PORT_H_ */
