#ifndef __PLT_HD_FPGA_H_
#define __PLT_HD_FPGA_H_

#include "hd_fpga_def.h"
#include "hd_fpga_msg.h"
#include "hd_fpga_deal.h"

enum {
    SAMPLING_RATE_24SPC = 0x00,
    SAMPLING_RATE_48SPC = 0x01,
    SAMPLING_RATE_80SPC = 0x02,
    SAMPLING_RATE_120SPC = 0x03
};

enum {
    SAMPLING_SRC_TYPE_AD = 0x00,
    SAMPLING_SRC_TYPE_SV = 0x01,
    SAMPLING_SRC_TYPE_RATED_DELAY = 0x3
};

enum {
    SAMPLING_SRC_AD_OR_SV_0 = 0x00,
    SAMPLING_SRC_AD_OR_SV_1 = 0x01
};

//用于配置SV报文控制寄存器 FPGA_SV_RECEIVE_CONFIG_REG 的数据结构
typedef struct
{
    uint8   sv_rcv_enable;      // b0:disbale b1:enable
    uint8   sv_rcv_net_port;    // sv receive net port 0~15 物理光口，一搬配置3~8
    uint8   sv_rcv_rated_delay_enable;  //SV接收报文额定延时是否有效 0:无效 1:有效 （默认第一通道为额定延时通道）
    uint32  sv_rcv_phase_angle;         //SV接收报文调相角(角度单位:分，负数采用二进制补码表示)
}TSV_RECEIVE_CONTROL;

//用于配置SV接收报文过滤寄存器的数据结构 FPGA_SV_FILTER_CONFIGx_REG
typedef struct
{
    uint32  APPID;            //SV接收报文APPID
    uint8   des_mac_addr[6];  //SV报文目的mac地址
    uint32  svid_crc32;       //SV接收报文svID字符串CRC-32生成校验码(通过fpga_calc_crc32计算出的crc32)
    uint32  confRev;          //SV接收报文confRev
    uint32  chan_cnt;         //SV接收报文每个ASDU采样通道数目
}TSV_RECEIVE_FILTER;

//用于配置SV发送控制寄存器 FPGA_SV_SEND_CONFIG_REG 寄存器的数据结构
typedef struct
{
    uint8   sv_send_enable_config_update;   //写1表示需要更新配置
    uint32  sv_send_port_bitwise;           //BIT0-12发送使能 每位对应一个外部网口，共0~12网口(目前可用3~8)
    uint8   sv_send_force_sync;             //强制同步 0:否 1是
    uint8   sv_send_force_test;             //强制测试 0:否 1是
    uint32  sv_send_chan_cnt;               //通道数目(1~49)
    uint32  sv_send_msg_len;                //发送报文长度 报文长度(1~1023字节)
}TSV_SEND_CONTROL;

//用于配置报文寻址寄存器 FPGA_SV_SEND_ADDRESSING_REG 的数据结构
typedef struct
{
    uint32  sv_send_smpCntValue_pos;    //smpSynch value首字节序号
    uint32  sv_send_smpSynchValue_pos;  //channel0 value首字节序号
    uint32  sv_send_channel0Value_pos;  //smpCnt value首字节序号
}TSV_SEND_ADDRESSING_REG;

//配置SV通道配置寄存器 FPGA_SV_SEND_CHAN_CONFIG_REG 的数据结构
typedef struct
{
    uint8   sv_src_enable;  // 数据源使能 0:禁止 1:使能
    uint8   sv_src_type;    // 数据源类型 b00:AD b01:SV b11:额定延时
    uint8   sv_src_choose;  // 数据源选择 b00:AD-00/SV-00 b01:AD-01/SV-01 ......
    uint32  sv_src_chan;    // 数据源通道 00-48
    uint8   sv_src_scale;   // 数据源缩放 0:原值 1:放大
    uint8   sv_src_factor;  // 数据源系数(有符号数，如果”数据源缩放”标志为1，则系数需放大32倍)
}TSV_CHAN_CONFIG_REG;

//消息类型 msgType
enum E_AXI_HP_MSG_TYPE{
    AXI_HP_MSG_TYPE_GOOSE    = 0x03,
    AXI_HP_MSG_TYPE_SV       = 0xB1,
    AXI_HP_MSG_TYPE_NODE     = 0xB2,
};

/**
 * @Function: axi_hp_msg_handle
 * @Description:  注册报文接收回调函数
 *  
 *   msgType :  消息类型，对应E_AXI_HP_MSG_TYPE
*/
typedef void (* axi_hp_msg_handle)(uint8 *msg, uint32 msgLen, uint8 msgType);

//crc32位翻转 
uint32 crc32_bit_reverse(uint32 CrcVal);
/* crc32 calculate*/
uint32 fpga_calc_crc32(uint8 *Data, uint16 Byte);

/* swap32 */
uint32 axi_hp_swap32(uint32 x);

//读取外部对时寄存器的值 三个寄存器
STATUS axi_hp_get_gps_time(uint32 data[3]);

//读取1PPSx1总线寄存器的值 （用于整装置秒级对时）
void axi_hp_get_1pps_x1(uint8 * state, uint32 * ns);

//读取1PPSx2总线寄存器的值（由外部对时源IRIG-B信号抽取）
void axi_hp_get_1pps_x2(uint8 * state, uint32 * ns);

//读取1PPSx3总线同步状态（该信号控制采样值插值同步时刻点）
uint8 axi_hp_get_1pps_x3(void);

//读取背板总线状态寄存器的值 FPGA_BACKPLANE_BUS_STATUS_REG
uint32 axi_hp_get_backplane_status(void);


/**
 * @Function: axi_hp_get_eth_status
 * @Description: 读取网口状态 
 * @author zhongwei (2020/4/3 13:47:06)
 * 
 * @param idx 0~15
 * 
 * @return SECTION_PLT_CODE uint8 
 *  
 * BIT[0] : 0-未连接 1-连接 
 * BIT[1] : 0-半双工 1-全双工 
 * BIT[2-3] : 0-10Mbps 1-100Mbps 2-1000Mbps 
 * 
*/
uint8 axi_hp_get_eth_status(uint32 idx);

//设置FPGA定时器 
void axi_hp_set_timer_period(uint32 ns);

//读取FPGA定时器
uint32 axi_hp_get_timer_period(void);

//读取FPGA定时器的计数值
uint32 axi_hp_get_timer_counter(void);

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_set_sv_sampling_delay  设置ADC采样前级滤波回路延时，单位1us
 * @paramuint32 delay_us : <= 2000us sv send delay control
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sv_sampling_delay(uint32 delay_us);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set goose sv test config
 * @return: 0: success  other: fail
 */
STATUS axi_hp_goose_sv_test_init();

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv receive msg control 设置SV报文接收控制使能
 * @param uint32 idx: 0-5 SV过滤模块组序号，非外部物理光口
 * @param TSV_RECEIVE_CONTROL *pRcvConfig
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sv_receive_control(uint32 idx, TSV_RECEIVE_CONTROL *pRcvConfig);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv receive msg control 获取SV报文接收配置信息
 * @param uint32 idx: 0-5 SV过滤模块组序号，非外部物理光口
 * @param TSV_RECEIVE_CONTROL *pRcvConfig
 * @return: 0: success  other: fail
 */
STATUS axi_hp_get_sv_receive_control(uint32 idx, TSV_RECEIVE_CONTROL *pRcvConfig);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv receive msg filter 设置SV报文接收过滤配置
 * @param uint32 idx: 0-5 SV过滤模块组序号，非外部物理光口
 * @param TSV_RECEIVE_FILTER *pFilter_config
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sv_receive_filter(uint32 idx, TSV_RECEIVE_FILTER *pFilter_config);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv receive msg filter
 * @param uint32 idx: 0-5
 * @param TSV_RECEIVE_CONFIG *pFilter_config
 * @return: 0: success  other: fail
 */
STATUS axi_hp_get_sv_receive_filter(uint32 idx, TSV_RECEIVE_FILTER *pFilter_config);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv send control 使能SV发送配置
 * @param TSV_SEND_CONTROL *pSendCtrl
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sv_send_control(TSV_SEND_CONTROL *pSendCtrl);

/**
 * @Author: wangzhong
 * @Date:
 * @description: get sv send control
 * @param TSV_SEND_CONTROL *pSendCtrl
 * @return: 0: success  other: fail
 */
STATUS axi_hp_get_sv_send_control(TSV_SEND_CONTROL *pSendCtrl);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv send addressing reg 设置报文寻址寄存器配置
 * @param TSV_SEND_ADDRESSING_REG *pAddressingReg
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sv_send_addressing_reg(TSV_SEND_ADDRESSING_REG *pAddressingReg);

/**
 * @Author: wangzhong
 * @Date:
 * @description: get sv send addressing reg
 * @param TSV_SEND_ADDRESSING_REG *pAddressingReg
 * @return: 0: success  other: fail
 */
STATUS axi_hp_get_sv_send_addressing_reg(TSV_SEND_ADDRESSING_REG *pAddressingReg);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv send channel config reg 设置SV发送通道配置 配置FPGA_SV_SEND_CHAN_CONFIG_REG寄存器
 * @param TSV_CHAN_CONFIG_REG *pChanConfig
 * @return: 0: success  other: fail 
 *  
 *  chan_idx : 0-48 支持49个发送通道
 *  config_reg_idx : 0~2 第几个配置寄存器，每个通道占用3个DWORD寄存器
 */
STATUS axi_hp_set_sv_send_channel_config(uint32 chan_idx, uint32 config_reg_idx, TSV_CHAN_CONFIG_REG *pChanConfig);

/**
 * @Author: wangzhong
 * @Date:
 * @description: get sv send channel config reg
 * @param TSV_CHAN_CONFIG_REG *pChanConfig
 * @return: 0: success  other: fail
 */
STATUS axi_hp_get_sv_send_channel_config(uint32 chan_idx, uint32 config_reg_idx, TSV_CHAN_CONFIG_REG *pChanConfig);

/**
 * @Author: wangzhong
 * @Date:
 * @description: copy send sv msg to fpga send buffer
 * @param TSV_CHAN_CONFIG_REG *pChanConfig
 * @return: 0: success  other: fail
 */
STATUS axi_hp_sv_cp_msg_to_sendbuf(uint8 *msg, uint32 msg_len);

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_set_eth_mac_addr 设置网口MAC地址
 * @param uint8 *pMacStr: input mac address 5 bytes array
 * @param uint32 idx : eth idx 15~0
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_eth_mac_addr(uint8 *pMacStr, uint32 idx);

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_set_mac_addr_replace_enable 配置FPGA_MAC_REPLACE_ENABLE_REG寄存器
 * @param uint32 enable: 1-enable 0-disable
 * @param uint32 idx : eth idx 15~0
 * @return: 0: success  other: fail 
 *  
 * enable : BIT0-15 网口MAC地址替换使能(发送端口) 
 *  
 */
STATUS axi_hp_set_mac_addr_replace_enable(uint32 enable, uint32 idx);

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_get_mac_addr_replace_enable
 * @param uint32 *enable: output 1-enable 0-disable
 * @param uint32 idx : eth idx 15~0
 * @return: 0: success  other: fail
 */
STATUS axi_hp_get_mac_addr_replace_enable(uint32 *enable, uint32 idx);

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_get_eth_mac_addr
 * @param uint8 *pMacStr: output mac address 5 bytes array
 * @param uint32 idx : eth idx 15~0
 * @return: 0: success  other: fail
 */
STATUS axi_hp_get_eth_mac_addr(uint8 *pMacStr, uint32 idx);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set goose storm threshold 配置Goose接收报文抑制控制寄存器 FPGA_GOOSE_CTRL_REG
 * @param uint32 threshold1_bytes_per_us : bytes / 1us
 * @param uint32 threshold2_packs_per_100us : packets / 100us
 * @return: 0: success  other: fail 
 *  
 *  
 *  threshold1_bytes_per_us    GOOSE风暴抑制门槛1 (总流量) (单位:1us)，默认为1byte/1us
 *  threshold2_packs_per_100us GOOSE风暴抑制门槛2 (总流量) (单位:100us)，默认为1packet/100us
 */
STATUS axi_hp_set_goose_storm_threshold(uint32 threshold1_bytes_per_us, uint32 threshold2_packs_per_100us);

/**
 * @Author: wangzhong
 * @Date:
 * @description: get goose storm threshold
 * @param uint32* threshold1_bytes_per_us : bytes / 1us
 * @param uint32* threshold2_packs_per_100us : packets / 100us
 * @return: 0: success  other: fail
 */
STATUS axi_hp_get_goose_storm_threshold(uint32 *threshold1_bytes_per_us, uint32 *threshold2_packs_per_100us);


/**
 * @Author: wangzhong
 * @Date:
 * @description: enable or disable goose storm control 配置GOOSE接收报文抑制使能寄存器 FPGA_GOOSE_STORM_CONTROL_REG
 * @param uint32 filter_idx : goose filter idx 0-95
 * @param uint32 enable :  1:enable  0:disable
 * @return: 0: success  other: fail 
 *   
 *  
 */

STATUS axi_hp_enable_goose_storm_control(uint32 filter_idx, uint32 enable);


/**
 * @Author: wangzhong
 * @Date:
 * @description: get goose storm control status 获取GOOSE接收报文抑制状态寄存器 FPGA_GOOSE_STORM_CONTROL_STATUS_REG
 * @param uint32 filter_idx : goose filter idx 0-95
 * @param uint32* enable :  1:enable  0:disable
 * @return: 0: success  other: fail
 */

STATUS axi_hp_get_goose_storm_control_status(uint32 filter_idx, uint32 *enable);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set goose filter   配置Goose接收报文过滤寄存器 FPGA_GOOSE_FILTER_CRC_REG
 * @param uint32 filter_idx : idx of filter 0-95
 * @param uint32 crc32 : crc value GOOSE接收报文”目的MAC+APPID+GoCBRef+DatSet+GoID”数据段CRC-32生成校验码
 * @param uint32 in_port_bitwise : receive port one port per bit GOOSE接收报文源端口映射 BIT0-15 外部网口0-15(当前用3~8)
 * @return: 0: success  other: fail 
 *  
 *  
 *  
 */
STATUS axi_hp_set_goose_filter(uint32 filter_idx, uint32 crc32, uint32 in_port_bitwise);

//读取goose过滤控制
STATUS axi_hp_get_goose_filter(uint32 filter_idx, uint32 * crc32, uint32 * in_port_bitwise);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sampling msg0 cotrol config 设置采样值报文上送0 采样控制寄存器 FPGA_SAMPING_MSG0_CONTROL_REG
 * @param uint32 sample_rate : 0-24spc  1-48spc  2-80spc
 * @param uint32 interval_cnt : interval cnt 上送间隔数目(1~32) 用于描述上送报文订阅的间隔数目，包含AD和SV间隔总数
 * @param uint32 des_adr_mapping : bus destination address mapping [15:0] node[15:0] 总线目标节点映射 每位对应一个节点，共00-15节点
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sampling_msg0_control(uint32 sample_rate, uint32 interval_cnt, uint32 des_adr_mapping);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sampling msg1 cotrol config 设置采样值报文上送1 采样控制寄存器 FPGA_SAMPING_MSG1_CONTROL_REG
 * @param uint32 sample_rate : 0-24spc  1-48spc  2-80spc
 * @param uint32 interval_cnt : interval cnt
 * @param uint32 des_adr_mapping : bus destination address mapping [15:0] node[15:0]
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sampling_msg1_control(uint32 sample_rate, uint32 interval_cnt, uint32 des_adr_mapping);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sampling msg0 interval config 设置采样值报文上送0 间隔配置寄存器 FPGA_SAMPING_MSG0_INTERVAL_CONFIGx_REG
 * @param uint32 interval_idx : 0-31 每路支持32个间隔
 * @param uint32 src1_mapping : [31:0] channel31 - channel0   数据源通道映射1
 * @param uint32 src2_mapping : [15:0] channel48 - channel32  数据源通道映射2
 * @param uint32 src_type : 0-AD  1-SV 数据源类型 b00:AD b01:SV
 * @param uint32 src_choose : 0x00-AD00/SV00   0x01-AD01/SV01  ......  0x1F-AD31/SV31 数据源选择
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sampling_msg0_interval_config(uint32 interval_idx, uint32 src1_mapping, uint32 src2_mapping, uint32 src_type, uint32 src_choose);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sampling msg1 interval config 设置采样值报文上送1 间隔配置寄存器 FPGA_SAMPING_MSG1_INTERVAL_CONFIGx_REG
 * @param uint32 interval_idx : 0-31
 * @param uint32 src1_mapping : [31:0] channel31 - channel0
 * @param uint32 src2_mapping : [15:0] channel48 - channel32 
 * @param uint32 src_type : 0-AD  1-SV
 * @param uint32 src_choose : 0x00-AD00/SV00   0x01-AD01/SV01  ......  0x1F-AD31/SV31
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_sampling_msg1_interval_config(uint32 interval_idx, uint32 src1_mapping, uint32 src2_mapping, uint32 src_type, uint32 src_choose);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set analog sampling fiter delay (us) 设置模拟量采样 滤波延时寄存器 FPGA_ANALOG_SAMPLING_FILTER_DELAY_REG
 * @param uint32 delay_us : delays (us)
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_analog_sampling_filter_delay(uint32 delay_us);

/**
 * @Author: wangzhong
 * @Date:
 * @description: set analog sampling calibration info 设置模拟量采样 AD采样校准寄存器 FPGA_ANALOG_SAMPLING_AD_CALIBRATION_REG
 * @param uint32 ad_idx : 0~1           AD0/AD1
 * @param uint32 chan_idx : 0~15        AD通道，每片AD支持16路
 * @param uint32 phase_angle            采样通道15调相角(角度单位:分，负数采用二进制补码表示)
 * @param uint32 digital_quantity       采样通道15去零漂数字量(负数采用二进制补码表示)
 * @return: 0: success  other: fail
 */
STATUS axi_hp_set_analog_sampling_calibration(uint32 ad_idx, uint32 chan_idx, uint32 phase_angle, uint32 digital_quantity);


/**
 * @Author: wangzhong
 * @Date:
 * @description: regist axi hp msg callback 注册报文接收回调函数
 * @param axi_hp_msg_handle callback: call back function
 * @return: 0: success  other: fail
 */
STATUS regist_axi_hp_msg_callback(axi_hp_msg_handle callback);

// todo axi_hp_sys_init
/**
 * @Author: wangzhong
 * @Date:
 * @description: init axi hp AXI_HP初始化
 * @param {type}
 * @return: 0: success  other: fail
 */
STATUS axi_hp_sys_init(void);


/**
 * @Author: wangzhong
 * @Date:
 * @description: fill msg head 8byte
 * @param uint8 * buf: head buf
 * @param uint32 send_port: send port
 * @param uint32 msgLen: msg body len
 * @param uint32 msgType: type
 * @return: 0: success  other: fail
 */
STATUS axi_hp_fill_goose_msg_head(uint8 *buf, uint32 send_port, uint32 msgLen);


/**
 * @Author: wangzhong
 * @Date:
 * @description: send msg from axi hp
 * @param uint8 * msg: msg
 * @param uint32 msgLen: len
 * @return: 0: success  other: fail
 */
STATUS axi_hp_send_msg(uint8 *msg, uint32 msgLen);


STATUS axi_goose_send_msg(uint32 send_port,uint8 *msg, uint32 msgLen);

/**
 * @Author: wangzhong
 * @Date:
 * @description: poll rx msg 从FPGA拉取数据，在该接口中会回调regist_axi_hp_msg_callback注册的回调函数
 * @return: 0: success  other: fail
 */
STATUS axi_hp_poll_msg(void);

//读取FPGA版本寄存器的值
uint32 axi_get_fpga_version(void);

//读取FPGA控制寄存器的值
uint32 axi_get_fpga_ctrl(void);

#endif //__PLT_HD_FPGA_H_
