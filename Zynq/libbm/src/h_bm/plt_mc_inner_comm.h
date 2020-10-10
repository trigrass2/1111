/*
 * @Author: zhongwei
 * @Date: 2019/11/28 14:51:54
 * @Description: 核间通讯处理接口
 * @File: plt_mc_inner_comm.h
 *
*/
#ifndef SRC_MC_PLT_MC_INNER_COMM_H_
#define SRC_MC_PLT_MC_INNER_COMM_H_

//初始化
void PowerOn_Init_McInnerComm(void);

//清空发送缓冲区 
void mc_inner_ClearSendBuf(void);

//缓冲区是否满
BOOL mc_inner_send_buf_is_full(void);

//缓冲区是否为空
BOOL mc_inner_send_buf_is_empty(void);

//发送报文到Linux的接口
BOOL mc_inner_send_to_linux(const uint8 * data, int32 len);


//接收缓冲区是否为空
BOOL mc_inner_rcv_buf_is_empty(void);

//从Linux获取通讯数据的接口 
int32 mc_inner_rcv_from_linux(uint8 * data, int32 max_len);

//获取Linux发送到BM侧的命令行 
BOOL mc_GetLinuxCommandLine(c128_t cmd);

#endif /* SRC_MC_PLT_MC_INNER_COMM_H_ */
