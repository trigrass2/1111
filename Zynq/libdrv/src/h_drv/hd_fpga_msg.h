/*
 * @Author: zhongwei
 * @Date: 2020/3/30 17:17:53
 * @Description: FPGA报文处理接口
 * @File: hd_fpga_msg.h
 *
*/

#ifndef SRC_H_BAREMETAL_PLT_HD_FPGA_MSG_H_
#define SRC_H_BAREMETAL_PLT_HD_FPGA_MSG_H_

#ifdef FPGA_HP_MSG_LITTLE_ENDIAN
    #define GET_UINT32_AT(idx)  MAKEUINT32(msg[idx], msg[idx+1], msg[idx+2], msg[idx+3])
    #define GET_UINT16_AT(idx)  MAKEWORD(msg[idx], msg[idx+1])
#else
    #define GET_UINT32_AT(idx)  MAKEUINT32(msg[idx+3], msg[idx+2], msg[idx+1], msg[idx])
    #define GET_UINT16_AT(idx)  MAKEWORD(msg[idx+1], msg[idx])
#endif

//Goose接收报文帧结构
typedef struct
{
    uint32 ns_time;         //该时标取自于1PPSx1纳秒计数器；单位:1024ns
    uint16 port;            //外部网口，按位表示
    const uint8 * data;     //Goose原始报文
    size_t data_len;        //原始报文长度
}TFRAME_GOOSE_RCV;

//SV接收报文帧结构
typedef struct
{
    uint32 len;         //报文长度
    uint8 send_node;    //发送节点编号
    uint16 send_port;   //总线目标节点映射 BIT00-15 按位
    uint16 sample_no;   //采样序号
    uint8 smaple_state; //采样状态
    uint16 delay_us;    //采样延时
    uint8 bay_cnt;      //间隔个数
    const uint32 * bay_state;    //间隔状态头指针，每4字节一个间隔状态，
    const uint32 * test_state;   //检修状态头指针
    const uint32 * valid_state;  //有效状态头指针
    const uint32 * sv_data;      //采样数据头指针
}TFRAME_SV_RCV;

typedef void (*Func_OnRcvSV)(const TFRAME_SV_RCV* pFrame);
typedef void (*Func_OnRcvGoose)(const TFRAME_GOOSE_RCV* pFrame);

//回调注册
void fpgs_msg_register_OnRcvSv(Func_OnRcvSV func);
void fpgs_msg_register_OnRcvGoose(Func_OnRcvGoose func);

//FPGA接收消息处理回调函数 
void fpga_msg_callback(uint8 *msg, uint32 msgLen, uint8 msgType);

#endif /* SRC_H_BAREMETAL_PLT_HD_FPGA_MSG_H_ */
