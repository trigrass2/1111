/*
 * @Author: zhongwei
 * @Date: 2020/3/30 17:18:19
 * @Description: FPGA报文处理
 * @File: hd_fpga_msg.c
 *
*/

#include "plt_inc_c.h"
#include "libdrv.h"


//FPGA消息统计信息
typedef struct{
    uint64 goose_msg_cnt;
    uint64 goose_msg_byte_cnt;
    uint64 sv_msg_cnt;
    uint64 sv_msg_byte_cnt;
    uint64 node_msg_cnt;
    uint64 node_msg_byte_cnt;
    uint64 else_msg_cnt;
    uint64 else_msg_byte_cnt;
}TFPGA_MSG_STATISTIC;

SECTION_PLT_DATA static TFPGA_MSG_STATISTIC _fpga_msg_statistic={0};

//FPGA消息注册接口
typedef struct{
    Func_OnRcvSV on_rcv_sv;
    Func_OnRcvSV on_rcv_sv_bak;
    Func_OnRcvGoose on_rcv_goose;
    Func_OnRcvGoose on_rcv_goose_bak;
}TFPGA_MSG_REGISTER;

SECTION_PLT_DATA static TFPGA_MSG_REGISTER _fpga_msg_register={0};

void fpga_msg_on_rcv_goose(const uint8 *msg, uint32 msgLen);
void fpga_msg_on_rcv_sv(const uint8 *msg, uint32 msgLen);

//打印FPGA统计信息
SECTION_PLT_CODE void Print_FpgaMsgStatistic(void)
{
    PRINTF("FPGA msg statistic:\r\n");
    PRINTF("  GOOSE pkg=%llu byte=%llu\r\n", _fpga_msg_statistic.goose_msg_cnt, _fpga_msg_statistic.goose_msg_byte_cnt);
    PRINTF("  SV pkg=%llu byte=%llu\r\n", _fpga_msg_statistic.sv_msg_cnt, _fpga_msg_statistic.sv_msg_byte_cnt);
    PRINTF("  NODE pkg=%llu byte=%llu\r\n", _fpga_msg_statistic.node_msg_cnt, _fpga_msg_statistic.node_msg_byte_cnt);
    PRINTF("  Others pkg=%llu byte=%llu\r\n", _fpga_msg_statistic.else_msg_cnt, _fpga_msg_statistic.else_msg_byte_cnt);
}

//回调注册
SECTION_PLT_CODE void fpgs_msg_register_OnRcvSv(Func_OnRcvSV func)
{
    ASSERT_L1(_fpga_msg_register.on_rcv_goose == NULL);
    _fpga_msg_register.on_rcv_sv = _fpga_msg_register.on_rcv_sv_bak = func;
    PRINTF_L1("fpgs_msg_register_OnRcvSv func=0x%x\n\r", func);
}

SECTION_PLT_CODE void fpgs_msg_register_OnRcvGoose(Func_OnRcvGoose func)
{   
    ASSERT_L1(_fpga_msg_register.on_rcv_goose == NULL);
    _fpga_msg_register.on_rcv_goose = _fpga_msg_register.on_rcv_goose_bak = func;
    PRINTF_L1("fpgs_msg_register_OnRcvGoose func=0x%x\n\r", func);
}

//中断中FPGA消息处理，fpga_msg_callback会在这个接口中调用
SECTION_PLT_CODE void fpga_msg_handle_inInt(const TTimeRun * pTimeRun)
{
    #define MAX_POLL_MSG_CNT 10     //每次拉取的最大包数
    int max_cnt = MAX_POLL_MSG_CNT;

    while(max_cnt > 0)
    {
        if (axi_hp_poll_msg() != 0) //0：拉取成功，其他：失败
            break;

        max_cnt--;
    }
}

/**
 * @Function: fpga_msg_callback
 * @Description: FPGA接收消息处理回调函数 
 * @author zhongwei (2020/3/30 17:20:20)
 * 
 * @param msg 消息头指针
 * @param msgLen 消息长度
 * @param msgType 消息类型 (E_AXI_HP_MSG_TYPE)
 * 
*/
SECTION_PLT_CODE void fpga_msg_callback(uint8 *msg, uint32 msgLen, uint8 msgType)
{
    ASSERT_L4(msg != NULL);
    ASSERT_L1(((int)msg % 4) == 0);  //必须4字节对齐

    switch (msgType)
    {
    case AXI_HP_MSG_TYPE_GOOSE:     //goose
        {
            _fpga_msg_statistic.goose_msg_cnt++;
            _fpga_msg_statistic.goose_msg_byte_cnt += msgLen;
            fpga_msg_on_rcv_goose(msg,msgLen);
            // send goose msg
            //axi_hp_send_msg(msg, msgLen);
        }
        break;
    case AXI_HP_MSG_TYPE_SV:        //SV、AD
        {
            _fpga_msg_statistic.sv_msg_cnt++;
            _fpga_msg_statistic.sv_msg_byte_cnt += msgLen;
            fpga_msg_on_rcv_sv(msg,msgLen);
        }
        break;
    case AXI_HP_MSG_TYPE_NODE:      //背板总线
        {
            _fpga_msg_statistic.node_msg_cnt++;
            _fpga_msg_statistic.node_msg_byte_cnt += msgLen;
        }
        break;
    default:
        {
            _fpga_msg_statistic.else_msg_cnt++;
            _fpga_msg_statistic.else_msg_byte_cnt += msgLen;
            ASSERT_L4(FALSE);
        }
    }
}


/**
 * @Function: fpga_msg_on_rcv_goose
 * @Description: BM收到FPGA接收的Goose报文调用接口 
 * @author zhongwei (2020/3/31 17:15:23)
 * 
 * @param msg 
 * @param msgLen 
 * 
 * @return SECTION_PLT_CODE void 
 *  
 * 报文格式参见《FPGA通讯接口规约》-03 GOOSE、MMS报文(0x03) 部分
 *  
 * 其报文内容为 8字节FPGA添加的报文头数据 + Goose原始报文(从MAC开始)
 * 
*/
SECTION_PLT_CODE void fpga_msg_on_rcv_goose(const uint8 *msg, uint32 msgLen)
{
    TFRAME_GOOSE_RCV frame;
    /*
        8字节报文头，这部分内容时FPGA附加的
        包含：
           4字节 - 报文属性（ns时间和报文长度）
           1字节 - 报文类型 固定0x03
           2字节 - 外部网口 BIT00-15 分别对应网口0-15
           1字节 - 保留
         数据内容是大端模式，高位在前
    */
    //0. 4字节 报文属性
    uint32 uData = GET_UINT32_AT(0);
    uint32 pkg_len = uData & 0xFFF; //报文长度(不含报文属性的报文长度)
    uint32 ns_time = (uData >> 12) & 0xFFFFF; //该时标取自于1PPSx1纳秒计数器；单位:1024ns
    //1. 报文类型
    uint8 msgType = msg[7];     //报文类型，固定为3

    if (msgType == AXI_HP_MSG_TYPE_GOOSE && pkg_len >= 4)
    {
        frame.ns_time = ns_time;
        frame.port = GET_UINT16_AT(5);  //外部网口，按位
        frame.data = &msg[8];   //实际Goose报文内容
        frame.data_len = pkg_len-4;     //实际Goose报文长度
//      if (IsDebugMessage(DEBUG_MESSAGE_GOOSE_RCV))
//      {
//          LOGMSG("OnRcvGoose us=%u len=%u port=0x%x pkg=0x%x\n\r",
//                 (ns_time * 1024) / 1000/*转为us时间*/,
//                 pkg_len, frame.port, (int)frame.data, 0,0);
//      }

        if (_fpga_msg_register.on_rcv_goose != NULL)
        {
            _fpga_msg_register.on_rcv_goose(&frame);
        }
    }
    else
    {
        LOGMSG("fpga_msg_on_rcv_goose error, pkg_len=%d msgType=%d\n\r", pkg_len, msgType, 0, 0, 0, 0);
    }
}

//接收到SV报文
SECTION_PLT_CODE void fpga_msg_on_rcv_sv(const uint8 *msg, uint32 msgLen)
{
    TFRAME_SV_RCV frame;
    /*
        8字节报文头
        包含：
           4字节 - 报文属性
           1字节 - 报文类型
           3字节 - 保留
         数据内容是大端模式，高位在前
    */
    //4字节 - 报文属性
    uint32 uData = GET_UINT32_AT(0);
    uint32 pkg_len = uData & 0xFFF;             //报文长度
    uint8 nSendNode = (uData >> 12) & 0xF;      //发送节点编号
    uint16 nSendPort = (uData >> 16) & 0xFF;    //总线目标节点映射 BIT00-15 按位
    //1字节 - 报文类型
    uint8 msgType = msg[7];     //报文类型，固定为0xB1
    if (msgType == AXI_HP_MSG_TYPE_SV && pkg_len >= 12)
    {
        frame.len = pkg_len;
        frame.send_node = nSendNode;
        frame.send_port = nSendPort;
        frame.sample_no = GET_UINT16_AT(9);    //2字节 - 采样序号
        frame.smaple_state = msg[8];                    //1字节 - 采样状态
        frame.delay_us = GET_UINT16_AT(14);    //2字节 - 额定延时 us
        frame.bay_cnt = msg[12];                        //1字节 - 间隔数目
        int ptr = 16;
        frame.bay_state = (const uint32 *)&msg[ptr];    //间隔状态头指针
        ptr += 4 * frame.bay_cnt;
        frame.test_state = (const uint32 *)&msg[ptr];   //检修状态头指针

		ptr += 4* ((frame.bay_cnt*16 +31) >> 5);
		
        frame.valid_state = (const uint32 *)&msg[ptr];

		ptr += 4* ((frame.bay_cnt*16 +31) >> 5);
        frame.sv_data = (const uint32 *)&msg[ptr];

//      if (IsDebugMessage(DEBUG_MESSAGE_SV_RCV))
//      {
//          LOGMSG("OnRcvSV len=%d port=0x%x no=%d state=0x%x us=%d bay=%d\n\r",
//                 pkg_len, nSendPort, frame.sample_no, frame.smaple_state, frame.delay_us, frame.bay_cnt);
//      }

        if (_fpga_msg_register.on_rcv_sv != NULL)
        {
            _fpga_msg_register.on_rcv_sv(&frame);
        }
    }

}
