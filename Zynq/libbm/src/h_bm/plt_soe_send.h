/*
 * @Author: zhongwei
 * @Date: 2020/3/26 14:22:51
 * @Description: SOE发送处理，发送到Linux
 * @File: plt_soe_send.h
 *
*/

#ifndef SRC_H_BAREMETAL_PLT_SOE_SEND_H_
#define SRC_H_BAREMETAL_PLT_SOE_SEND_H_

#define GetSoeSend()            (&GetSoeBufferSave()->soeSend)
#define GetSoeSendIdx(soe_type, s_port)     (GetSoeSend()->nSendIdx[soe_type][s_port])
#define SetSoeSendIdx(soe_type, s_port, idx)     (GetSoeSend()->nSendIdx[soe_type][s_port] = (idx))


//清空SOE发送 标志，所有报告类型的发送缓冲区清空
void soe_ClearSoeSendIndex(uint16 s_port);
//按SOE类型置全部已发送
void soe_SoeSendClear(uint16 s_port, int16 soe_type);
//重置SOE为未发送
void soe_ResetSoeSendIndex(uint16 s_port);
//SOE待发送缓冲区是否为空
BOOL soe_SoeSendIsEmpty(uint16 s_port, int16 soe_type);
//确认SOE发送
void soe_SoeSendAck(uint16 s_port, int16 soe_type, int32 count);

//获取待发送SOE 
const void * soe_SoeSendGet(uint16 s_port, int16 soe_type, int32 * pCount);

#endif /* SRC_H_BAREMETAL_PLT_SOE_SEND_H_ */
