/*
 * @Author: wangzhong
 * @Date:
 * @LastEditors: wangzhong
 * @LastEditTime:
 * @Description: fpga operation
 * @FilePath: hd_fpga.c
 */

#include "plt_inc_c.h"
#include "libdrv.h"

SECTION_PLT_DATA uint32 hd_fpga_debug = 0;

#define MAX_AXI_HP_MSG_HEAD_LEN                 8
#define MAX_AXI_ETH_DEV_CNT                     9
#define MAX_AXI_HP_RX_MSG_CNT                   128
#define MAX_AXI_HP_TX_MSG_CNT                   128
#define MAX_AXI_HP_MSG_SIZE                     2048

#define ALIGN_LEN                               32

#define MAX_AD_CNT                              2
#define MAX_AD_CHANNEL_CNT                      16

struct AXI_HP_CTRL
{
    uint32  isInitialized;
    uint32  rxIdx;
    uint32  txIdx;
    uint8   *txBuf;
    uint8   *rxBuf;
    axi_hp_msg_handle callback;
};

SECTION_PLT_DATA struct AXI_HP_CTRL gAxiHpCtrl = {0};

/**
 * @Function: axiTxMsgBuf/axiRxMsgBuf
 * @Description:  数据必须放在非cache区，否则SV/Goose发送会有问题
 * @author zhongwei (2020/3/30 9:58:48)
 * 
*/
SECTION_NOCACHE_DATA uint8 axiTxMsgBuf[(MAX_AXI_HP_TX_MSG_CNT + 1) * MAX_AXI_HP_MSG_SIZE ];
SECTION_NOCACHE_DATA uint8 axiRxMsgBuf[(MAX_AXI_HP_RX_MSG_CNT + 1) * MAX_AXI_HP_MSG_SIZE ];

#define CRC32_POLYNOMIAL            0xEDB88320

//crc32位翻转 
SECTION_PLT_CODE uint32 crc32_bit_reverse(uint32 CrcVal)
{
    uint32 Result = 0;

    //BIT位高低调换
    for(int j = 32; j > 0; j--)
    {
        Result |= (CrcVal & 0x00000001) << (j - 1);
        CrcVal >>= 1;
    }

    return Result;
}

//与f_crc32算法一样，只是最后需要再进行位翻转操作
SECTION_PLT_CODE uint32 fpga_calc_crc32(uint8 *Data, uint16 Byte)
{
    uint32 crc32 = 0xFFFFFFFF;
    crc32 = crc32_calc(crc32, Data, Byte);
    return crc32_bit_reverse(crc32);
}

//XScuGic INTCInst;
//#define SW1_INT_ID 61
//#define INT_TYPE_RISING_EDGE    0x03
//#define INT_TYPE_HIGHLEVEL      0x01
//#define INT_TYPE_MASK           0x03


SECTION_PLT_CODE uint32 axi_hp_swap32(uint32 x)
{
    uint8 *s = (uint8 *)&x;
    return (uint32)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
}

/**
 * @Function: axi_hp_get_gps_time
 * @Description: //读取外部对时寄存器的值 三个寄存器
 * @author zhongwei (2020/4/2 14:55:02)
 * 
 * @param pTime 
 * 
 * @return SECTION_PLT_CODE STATUS 0:成功
 * 
*/
SECTION_PLT_CODE STATUS axi_hp_get_gps_time(uint32 data[3])
{
    data[0] = Xil_In32(FPGA_GPS_TIME_0_REG);
    data[1] = Xil_In32(FPGA_GPS_TIME_1_REG);
    data[2] = Xil_In32(FPGA_GPS_TIME_2_REG);
    return XST_SUCCESS;
}

/**
 * @Function: axi_hp_get_1pps_x1
 * @Description: 读取1PPSx1总线寄存器的值 （用于整装置秒级对时）
 * @author zhongwei (2020/4/3 11:30:11)
 * 
 * @param state 1PPSx1总线同步状态 0:异常 1:正常
 * @param ns 纳秒计数器(0~999_999_999ns)
 * 
 * @return SECTION_PLT_CODE void 
 *  
 * 1PPSx1用于整装置秒级对时，装置上电后由指定模件定期发送秒脉冲信号， 
 * 其它模件根据该秒脉冲进行秒级以下精确打时标，该秒脉冲信号不受任何外部对时信号影响，信号频率为1Hz。 
*/
SECTION_PLT_CODE void axi_hp_get_1pps_x1(uint8 * state, uint32 * ns)
{
    uint32 reg = Xil_In32(FPGA_1PPS_x1_REG);
    *state = (reg>>31) & 0x01;
    *ns = reg & 0x3FFFFFFF;
}

/**
 * @Function: axi_hp_get_1pps_x2
 * @Description:  读取1PPSx2总线寄存器的值（由外部对时源IRIG-B信号抽取）
 * @author zhongwei (2020/4/3 11:42:36)
 * 
 * @param state 1PPSx2总线同步状态 0:异常 1:正常
 * @param ns 纳秒计数器(0~999_999_999ns)
 * 
 * @return SECTION_PLT_CODE void 
 *  
 * 1PPSx2由外部对时源IRIG-B信号抽取，同步于外部对时源IRIG-B信号，配合软报文实现整装置精确对时，信号频率为1Hz。 
*/
SECTION_PLT_CODE void axi_hp_get_1pps_x2(uint8 * state, uint32 * ns)
{
    uint32 reg = Xil_In32(FPGA_1PPS_x2_REG);
    *state = (reg>>31) & 0x01;
    *ns = reg & 0x3FFFFFFF;
}

/**
 * @Function: axi_hp_get_1pps_x3
 * @Description:  读取1PPSx3总线同步状态（该信号控制采样值插值同步时刻点）
 * @author zhongwei (2020/4/3 11:44:46)
 * 
 * @return SECTION_PLT_CODE uint8 
 * 1PPSx3该信号控制采样值插值同步时刻点，用于定频或变频采样， 
 * 该同步信号经倍频处理后产生定时中断信号作为CORE1中断源IRQ_F2P[0](上升沿有效)。
*/
SECTION_PLT_CODE uint8 axi_hp_get_1pps_x3(void)
{
    uint32 reg = Xil_In32(FPGA_1PPS_x3_REG);
    return (reg>>31) & 0x01;
}

//读取背板总线状态寄存器的值
SECTION_PLT_CODE uint32 axi_hp_get_backplane_status(void)
{
    return Xil_In32(FPGA_BACKPLANE_BUS_STATUS_REG);
}

//设置FPGA定时器 
SECTION_PLT_CODE void axi_hp_set_timer_period(uint32 ns)
{
    Xil_Out32(FPGA_TIMER_PERIOD_REG, ns & FPGA_TIMER_MASK);
}

//读取FPGA定时器
SECTION_PLT_CODE uint32 axi_hp_get_timer_period(void)
{
    return Xil_In32(FPGA_TIMER_PERIOD_REG) & FPGA_TIMER_MASK;
}

//读取FPGA定时器的计数值
SECTION_PLT_CODE uint32 axi_hp_get_timer_counter(void)
{
    return Xil_In32(FPGA_TIMER_COUNTER_REG) & FPGA_TIMER_MASK;
}

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
SECTION_PLT_CODE uint8 axi_hp_get_eth_status(uint32 idx)
{
    if (idx >= FPGA_ETH_PORT_COUNT)
    {
        return 0;
    }
    else if (idx < 8)
    {
        uint32 reg = Xil_In32(FPGA_ETH_STATUS_0_REG);
        return (reg >> (4*idx)) & 0xF;
    }
    else
    {
        uint32 reg = Xil_In32(FPGA_ETH_STATUS_1_REG);
        return (reg >> (4*(idx-8))) & 0xF;
    }
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv receive msg control
 * @param uint32 idx: 0-5
 * @param TSV_RECEIVE_CONTROL *pRcvConfig
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sv_receive_control(uint32 idx, TSV_RECEIVE_CONTROL *pRcvConfig)
{
    uint32 regVal = 0;
    uint32 reg;

    if (idx >= FPGA_SV_RECEIVE_REGION_MAX_CNT)
    {
        PRINTF_L1("axi_hp_set_sv_receive_config para err!  idx=%x \r\n", idx);
        return -1;
    }

    reg = FPGA_SV_RECEIVE_CONFIG_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;

    regVal = ((pRcvConfig->sv_rcv_enable << SV_RECEIVE_ENABLE_SHIFT) & SV_RECEIVE_ENABLE_MASK) |
            ((pRcvConfig->sv_rcv_net_port << SV_RECEIVE_NET_PORT_SHIFT) & SV_RECEIVE_NET_PORT_MASK) |
            ((pRcvConfig->sv_rcv_rated_delay_enable << SV_RECEIVE_RATED_DELAY_VALID_SHIFT) & SV_RECEIVE_RATED_DELAY_VALID_MASK) |
            ((pRcvConfig->sv_rcv_phase_angle << SV_RECEIVE_PHASE_ANGLE_SHIFT) & SV_RECEIVE_PHASE_ANGLE_MASK);

    Xil_Out32(reg, regVal);
    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_set_sv_sampling_delay
 * @paramuint32 delay_us : <= 2000us sv send delay control
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sv_sampling_delay(uint32 delay_us)
{
    uint32 regVal = 0;
    uint32 reg;

    reg = FPGA_CTRL_REG;
    regVal = Xil_In32(reg);

    regVal = regVal & (~ SV_DELAY_MASK);
    regVal = ((delay_us << SV_DELAY_SHIFT) & SV_DELAY_MASK) | regVal;
    Xil_Out32(reg, regVal);
    return 0;
}


/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv receive msg control
 * @param uint32 idx: 0-5
 * @param TSV_RECEIVE_CONTROL *pRcvConfig
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_get_sv_receive_control(uint32 idx, TSV_RECEIVE_CONTROL *pRcvConfig)
{
    uint32 regVal = 0;
    uint32 reg;

    if (idx >= FPGA_SV_RECEIVE_REGION_MAX_CNT)
    {
        PRINTF_L1("axi_hp_set_sv_receive_config para err!  idx=%x \r\n", idx);
        return -1;
    }

    reg = FPGA_SV_RECEIVE_CONFIG_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = Xil_In32(reg);

    pRcvConfig->sv_rcv_enable = (regVal & SV_RECEIVE_ENABLE_MASK) << SV_RECEIVE_ENABLE_SHIFT;
    pRcvConfig->sv_rcv_net_port = (regVal & SV_RECEIVE_NET_PORT_MASK) << SV_RECEIVE_NET_PORT_SHIFT;
    pRcvConfig->sv_rcv_rated_delay_enable = (regVal & SV_RECEIVE_RATED_DELAY_VALID_MASK) << SV_RECEIVE_RATED_DELAY_VALID_SHIFT;
    pRcvConfig->sv_rcv_phase_angle = (regVal & SV_RECEIVE_PHASE_ANGLE_MASK) << SV_RECEIVE_PHASE_ANGLE_SHIFT;

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv receive msg filter
 * @param uint32 idx: 0-5
 * @param TSV_RECEIVE_FILTER *pFilter_config
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sv_receive_filter(uint32 idx, TSV_RECEIVE_FILTER *pFilter_config)
{
    uint32 regVal = 0;
    uint32 reg;

    if ((idx >= FPGA_SV_RECEIVE_REGION_MAX_CNT) || (NULL == pFilter_config))
    {
        PRINTF_L1("axi_hp_set_sv_receive_filter para err!  idx=%x pFilter_config=%x\r\n", idx, (uint32)pFilter_config);
        return -1;
    }

    /* config0 reg */
    reg = FPGA_SV_FILTER_CONFIG0_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = ((pFilter_config->APPID << SV_RECEIVE_APPID_SHIFT) & SV_RECEIVE_APPID_MASK) |
            ((pFilter_config->des_mac_addr[4] << SV_RECEIVE_DES_MAC_BYTE5_SHIFT) & SV_RECEIVE_DES_MAC_BYTE5_MASK) |
            ((pFilter_config->des_mac_addr[5] << SV_RECEIVE_DES_MAC_BYTE6_SHIFT) & SV_RECEIVE_DES_MAC_BYTE6_MASK);
    Xil_Out32(reg, regVal);

    /* config1 reg */
    reg = FPGA_SV_FILTER_CONFIG1_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = pFilter_config->svid_crc32;
    Xil_Out32(reg, regVal);

    /* config2 reg */
    reg = FPGA_SV_FILTER_CONFIG2_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = pFilter_config->confRev;
    Xil_Out32(reg, regVal);

    /* config3 reg */
    reg = FPGA_SV_FILTER_CONFIG3_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = (pFilter_config->chan_cnt << SV_RECEIVE_ASDU_CHAN_CNT_SHIFT) & SV_RECEIVE_ASDU_CHAN_CNT_MASK;
    Xil_Out32(reg, regVal);

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv receive msg filter
 * @param uint32 idx: 0-5
 * @param TSV_RECEIVE_CONFIG *pFilter_config
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_get_sv_receive_filter(uint32 idx, TSV_RECEIVE_FILTER *pFilter_config)
{
    uint32 regVal = 0;
    uint32 reg;

    if ((idx >= FPGA_SV_RECEIVE_REGION_MAX_CNT) || (NULL == pFilter_config))
    {
        PRINTF_L1("axi_hp_get_sv_receive_filter para err!  idx=%x pFilter_config=%x\r\n", idx, (uint32)pFilter_config);
        return -1;
    }

    /* config0 reg */
    reg = FPGA_SV_FILTER_CONFIG0_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = Xil_In32(reg);
    pFilter_config->APPID = (regVal & SV_RECEIVE_APPID_MASK) << SV_RECEIVE_APPID_SHIFT;
    pFilter_config->des_mac_addr[4] = (regVal & SV_RECEIVE_DES_MAC_BYTE5_MASK) << SV_RECEIVE_DES_MAC_BYTE5_SHIFT;
    pFilter_config->des_mac_addr[5] = (regVal & SV_RECEIVE_DES_MAC_BYTE6_MASK) << SV_RECEIVE_DES_MAC_BYTE6_SHIFT;

    /* config1 reg */
    reg = FPGA_SV_FILTER_CONFIG1_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = Xil_In32(reg);
    pFilter_config->svid_crc32 = regVal;

    /* config2 reg */
    reg = FPGA_SV_FILTER_CONFIG2_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = Xil_In32(reg);
    pFilter_config->confRev = regVal;

    /* config3 reg */
    reg = FPGA_SV_FILTER_CONFIG3_REG + idx * FPGA_SV_RECEIVE_REGION_LEN;
    regVal = Xil_In32(reg);
    pFilter_config->chan_cnt = (regVal & SV_RECEIVE_ASDU_CHAN_CNT_MASK) << SV_RECEIVE_ASDU_CHAN_CNT_SHIFT;

    return 0;
}


/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv send control
 * @param TSV_SEND_CONTROL *pSendCtrl
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sv_send_control(TSV_SEND_CONTROL *pSendCtrl)
{
    uint32 regVal = 0;
    uint32 reg;

    if (NULL == pSendCtrl)
    {
        PRINTF_L1("axi_hp_set_sv_send_control para err!  pSendCtrl=%x\r\n",  (uint32)pSendCtrl);
        return -1;
    }

    reg = FPGA_SV_SEND_CONFIG_REG;
    regVal = ((pSendCtrl->sv_send_enable_config_update << SV_SEND_CONFIG_UPDATE_SHIFT) & SV_SEND_CONFIG_UPDATE_MASK) |
            ((pSendCtrl->sv_send_port_bitwise << SV_SEND_NET_PORT_SHIFT) & SV_SEND_NET_PORT_MASK) |
            ((pSendCtrl->sv_send_force_sync << SV_SEND_FORCE_SYN_SHIFT) & SV_SEND_FORCE_SYN_MASK) |
            ((pSendCtrl->sv_send_force_test << SV_SEND_FORCE_TEST_SHIFT) & SV_SEND_FORCE_TEST_MASK) |
            ((pSendCtrl->sv_send_chan_cnt << SV_SEND_CHAN_CNT_SHIFT) & SV_SEND_CHAN_CNT_MASK) |
            ((pSendCtrl->sv_send_msg_len << SV_SEND_MSG_LEN_SHIFT) & SV_SEND_MSG_LEN_MASK);
    Xil_Out32(reg, regVal);
    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: get sv send control
 * @param TSV_SEND_CONTROL *pSendCtrl
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_get_sv_send_control(TSV_SEND_CONTROL *pSendCtrl)
{
    uint32 regVal = 0;
    uint32 reg;

    if (NULL == pSendCtrl)
    {
        PRINTF_L1("axi_hp_set_sv_send_control para err!  pSendCtrl=%x\r\n", (uint32)pSendCtrl);
        return -1;
    }
    reg = FPGA_SV_SEND_CONFIG_REG;
    regVal = Xil_In32(reg);

    pSendCtrl->sv_send_enable_config_update = (regVal & SV_SEND_CONFIG_UPDATE_MASK) << SV_SEND_CONFIG_UPDATE_SHIFT;
    pSendCtrl->sv_send_port_bitwise = (regVal & SV_SEND_NET_PORT_MASK) << SV_SEND_NET_PORT_SHIFT;
    pSendCtrl->sv_send_force_sync = (regVal & SV_SEND_FORCE_SYN_MASK) << SV_SEND_FORCE_SYN_SHIFT;
    pSendCtrl->sv_send_force_test = (regVal & SV_SEND_FORCE_TEST_MASK) << SV_SEND_FORCE_TEST_SHIFT;
    pSendCtrl->sv_send_chan_cnt = (regVal & SV_SEND_CHAN_CNT_MASK) << SV_SEND_CHAN_CNT_SHIFT;
    pSendCtrl->sv_send_msg_len = (regVal & SV_SEND_MSG_LEN_MASK) << SV_SEND_MSG_LEN_SHIFT;

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv send addressing reg
 * @param TSV_SEND_ADDRESSING_REG *pAddressingReg
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sv_send_addressing_reg(TSV_SEND_ADDRESSING_REG *pAddressingReg)
{
    uint32 regVal = 0;
    uint32 reg;

    if (NULL == pAddressingReg)
    {
        PRINTF_L1("axi_hp_set_sv_send_addressing_reg para err!  pAddressingReg=%x\r\n", (uint32)pAddressingReg);
        return -1;
    }

    reg = FPGA_SV_SEND_ADDRESSING_REG;
    regVal = ((pAddressingReg->sv_send_channel0Value_pos << SV_SEND_CHAN0_VALUE_POS_SHIFT) & SV_SEND_CHAN0_VALUE_POS_MASK) |
            ((pAddressingReg->sv_send_smpSynchValue_pos << SV_SEND_SMPSYNCH_VALUE_POS_SHIFT) & SV_SEND_SMPSYNCH_VALUE_POS_MASK) |
            ((pAddressingReg->sv_send_smpCntValue_pos << SV_SEND_SMPCNT_VALUE_POS_SHIFT) & SV_SEND_SMPCNT_VALUE_POS_MASK);
    Xil_Out32(reg, regVal);
    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: get sv send addressing reg
 * @param TSV_SEND_ADDRESSING_REG *pAddressingReg
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_get_sv_send_addressing_reg(TSV_SEND_ADDRESSING_REG *pAddressingReg)
{
    uint32 regVal = 0;
    uint32 reg;

    if (NULL == pAddressingReg)
    {
        PRINTF_L1("axi_hp_get_sv_send_addressing_reg para err!  pAddressingReg=%x\r\n", (uint32)pAddressingReg);
        return -1;
    }

    reg = FPGA_SV_SEND_ADDRESSING_REG;
    regVal = Xil_In32(reg);

    pAddressingReg->sv_send_channel0Value_pos = (regVal & SV_SEND_CHAN0_VALUE_POS_MASK) << SV_SEND_CHAN0_VALUE_POS_SHIFT;
    pAddressingReg->sv_send_smpSynchValue_pos = (regVal & SV_SEND_SMPSYNCH_VALUE_POS_MASK) << SV_SEND_SMPSYNCH_VALUE_POS_SHIFT;
    pAddressingReg->sv_send_smpCntValue_pos = (regVal & SV_SEND_SMPCNT_VALUE_POS_MASK) << SV_SEND_SMPCNT_VALUE_POS_SHIFT;

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sv send channel config reg
 * @param TSV_CHAN_CONFIG_REG *pChanConfig
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sv_send_channel_config(uint32 chan_idx, uint32 config_reg_idx, TSV_CHAN_CONFIG_REG *pChanConfig)
{
    uint32 regVal = 0;
    uint32 reg;

    if ((NULL == pChanConfig) || (chan_idx >= FPGA_SV_SEND_CHAN_MAX_CNT) || (config_reg_idx >= FPGA_SV_SEND_CHAN_CONFIG_REG_MAX_CNT))
    {
        PRINTF_L1("axi_hp_set_sv_send_channel_config para err!  pChanConfig=%x chan_idx=%d config_reg_idx=%d\r\n", (uint32)pChanConfig, chan_idx, config_reg_idx);
        return -1;
    }

    reg = FPGA_SV_SEND_CHAN_CONFIG_REG + (chan_idx * FPGA_SV_SEND_CHAN_CONFIG_REGS_LENS) + (config_reg_idx * 0x04);
    regVal = ((pChanConfig->sv_src_enable << SV_SEND_SRC_ENABLE_SHIFT) & SV_SEND_SRC_ENABLE_MASK) |
            ((pChanConfig->sv_src_type << SV_SEND_SRC_TYPE_SHIFT) & SV_SEND_SRC_TYPE_MASK) |
            ((pChanConfig->sv_src_choose << SV_SEND_SRC_CHOOSE_SHIFT) & SV_SEND_SRC_CHOOSE_MASK) |
            ((pChanConfig->sv_src_chan << SV_SEND_SRC_CHAN_SHIFT) & SV_SEND_SRC_CHAN_MASK) |
            ((pChanConfig->sv_src_scale << SV_SEND_SRC_SCALE_SHIFT) & SV_SEND_SRC_SCALE_MASK) |
            ((pChanConfig->sv_src_factor << SV_SEND_SRC_FACTOR_SHIFT) & SV_SEND_SRC_FACTOR_MASK);
    Xil_Out32(reg, regVal);
    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: get sv send channel config reg
 * @param TSV_CHAN_CONFIG_REG *pChanConfig
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_get_sv_send_channel_config(uint32 chan_idx, uint32 config_reg_idx, TSV_CHAN_CONFIG_REG *pChanConfig)
{
    uint32 regVal = 0;
    uint32 reg;

    if ((NULL == pChanConfig) || (chan_idx >= FPGA_SV_SEND_CHAN_MAX_CNT) || (config_reg_idx >= FPGA_SV_SEND_CHAN_CONFIG_REG_MAX_CNT))
    {
        PRINTF_L1("axi_hp_get_sv_send_channel_config para err!  pChanConfig=%x chan_idx=%d config_reg_idx=%d\r\n", (uint32)pChanConfig, chan_idx, config_reg_idx);
        return -1;
    }

    reg = FPGA_SV_SEND_CHAN_CONFIG_REG + (chan_idx * FPGA_SV_SEND_CHAN_CONFIG_REGS_LENS) + (config_reg_idx * 0x04);
    regVal = Xil_In32(reg);

    pChanConfig->sv_src_enable = (regVal & SV_SEND_SRC_ENABLE_MASK) << SV_SEND_SRC_ENABLE_SHIFT;
    pChanConfig->sv_src_type = (regVal & SV_SEND_SRC_TYPE_MASK) << SV_SEND_SRC_TYPE_SHIFT;
    pChanConfig->sv_src_choose = (regVal & SV_SEND_SRC_CHOOSE_MASK) << SV_SEND_SRC_CHOOSE_SHIFT;
    pChanConfig->sv_src_chan = (regVal & SV_SEND_SRC_CHAN_MASK) << SV_SEND_SRC_CHAN_SHIFT;
    pChanConfig->sv_src_scale = (regVal & SV_SEND_SRC_SCALE_MASK) << SV_SEND_SRC_SCALE_SHIFT;
    pChanConfig->sv_src_factor = (regVal & SV_SEND_SRC_FACTOR_SHIFT) << SV_SEND_SRC_FACTOR_SHIFT;

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: copy send sv msg to fpga send buffer
 * @param TSV_CHAN_CONFIG_REG *pChanConfig
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_sv_cp_msg_to_sendbuf(uint8 *msg, uint32 msg_len)
{
    if ((NULL == msg) || (msg_len > 1280))
    {
        PRINTF_L1("axi_hp_sv_send_cp_msg para err!  msg=%x msg_len=%d \r\n", (uint32)msg, msg_len);
        return -1;
    }

    memcpy((uint32 *)FPGA_SV_SEND_BUF_REG, msg, msg_len);
    return 0;
}


/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_set_eth_mac_addr
 * @param uint8 *pMacStr: input mac address 5 bytes array
 * @param uint32 idx : eth idx 15~0
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_eth_mac_addr(uint8 *pMacStr, uint32 idx)
{
    uint32 regVal = 0;
    uint32 reg;

    if ((NULL == pMacStr) || (idx >= FPGA_ETH_PORT_COUNT))
    {
        PRINTF_L1("axi_hp_set_eth_mac_addr para err!  pMacStr=%x idx=%d\r\n", (uint32)pMacStr, idx);
        return -1;
    }

    reg = FPGA_ETH_MAC_ADD_REG + idx * 8;

    regVal = (pMacStr[0] << 8) + pMacStr[1];
    Xil_Out32(reg, regVal);

    reg += 4;
    regVal = (pMacStr[3] << 24) + (pMacStr[4] << 16) + (pMacStr[5] << 8) + pMacStr[6];
    Xil_Out32(reg, regVal);

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_get_eth_mac_addr
 * @param uint8 *pMacStr: output mac address 5 bytes array
 * @param uint32 idx : eth idx 15~0
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_get_eth_mac_addr(uint8 *pMacStr, uint32 idx)
{
    uint32 regVal = 0;
    uint32 reg;

    if ((NULL == pMacStr) || (idx >= FPGA_ETH_PORT_COUNT))
    {
        PRINTF_L1("axi_hp_get_eth_mac_addr para err!  pMacStr=%x idx=%d\r\n", (uint32)pMacStr, idx);
        return -1;
    }

    reg = FPGA_ETH_MAC_ADD_REG + idx * 8;
    regVal = Xil_In32(reg);

    pMacStr[0] = (regVal >> 8) & 0xFF;
    pMacStr[1] = regVal & 0xFF;

    reg += 4;
    regVal = Xil_In32(reg);
    pMacStr[2] = (regVal >> 24) & 0xFF;
    pMacStr[3] = (regVal >> 16) & 0xFF;
    pMacStr[4] = (regVal >> 8) & 0xFF;
    pMacStr[5] = (regVal) & 0xFF;

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_set_mac_addr_replace_enable
 * @param uint32 enable: 1-enable 0-disable
 * @param uint32 idx : eth idx 15~0
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_mac_addr_replace_enable(uint32 enable, uint32 idx)
{
    uint32 regVal = 0;
    uint32 reg;

    if (idx >= FPGA_ETH_PORT_COUNT)
    {
        PRINTF_L1("axi_hp_set_mac_addr_replace_enable para err!  idx=%d\r\n", idx);
        return -1;
    }

    reg = FPGA_MAC_REPLACE_ENABLE_REG;
    regVal = Xil_In32(reg);

    if (enable)
    {
        regVal |= (0x01 << idx);
    }
    else
    {
        regVal &= ~(0x01 << idx);
    }

    Xil_Out32(reg, regVal);

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: axi_hp_get_mac_addr_replace_enable
 * @param uint32 *enable: output 1-enable 0-disable
 * @param uint32 idx : eth idx 15~0
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_get_mac_addr_replace_enable(uint32 *enable, uint32 idx)
{
    uint32 regVal = 0;
    uint32 reg;

    if ((idx >= FPGA_ETH_PORT_COUNT) || (NULL == enable))
    {
        PRINTF_L1("axi_hp_set_mac_addr_replace_enable para err!  idx=%d\r\n", idx);
        return -1;
    }

    reg = FPGA_MAC_REPLACE_ENABLE_REG;
    regVal = Xil_In32(reg);

    if (regVal & (0x01 << idx))
        *enable = 1;
    else
        *enable = 0;

    return 0;
}



/**
 * @Author: wangzhong
 * @Date:
 * @description: set analog sampling fiter delay (us)
 * @param uint32 delay_us : delays (us)
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_analog_sampling_filter_delay(uint32 delay_us)
{
    uint32 regVal;

    regVal = (delay_us << ANALOG_SAMPLING_FILTER_DELAY_SHIFT) & ANALOG_SAMPLING_FILTER_DELAY_MASK;
    Xil_Out32(FPGA_ANALOG_SAMPLING_FILTER_DELAY_REG, regVal);
    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set analog sampling calibration info
 * @param uint32 ad_idx : 0~1 
 * @param uint32 chan_idx : 0~15 
 * @param uint32 phase_angle
 * @param uint32 digital_quantity
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_analog_sampling_calibration(uint32 ad_idx, uint32 chan_idx, uint32 phase_angle, uint32 digital_quantity)
{
    uint32 regVal;
    uint32 reg;

    if ((ad_idx > (MAX_AD_CNT-1)) || (chan_idx > (MAX_AD_CHANNEL_CNT - 1)))
    {   
        PRINTF_L1("axi_hp_set_analog_sampling_calibration para err!  ad_idx=%d chan_idx=%d\r\n", ad_idx, chan_idx);
        return -1;
    }

    regVal = ((phase_angle << ANALOG_SAMPLE_PHASE_ANGLE_SHIFT) &  ANALOG_SAMPLE_PHASE_ANGLE_MASK)
            | ((digital_quantity << ANALOG_SAMPLE_ZERO_DRIFT_CTRL_SHIFT) & ANALOG_SAMPLE_ZERO_DRIFT_CTRL_MASK);

    reg = FPGA_ANALOG_SAMPLING_AD_CALIBRATION_REG + ad_idx * FPGA_ANALOG_SAMPLING_AD_CALIBRATION_INFO_LEN + (chan_idx * 4);
    Xil_Out32(reg, regVal);
    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sampling msg0 cotrol config
 * @param uint32 sample_rate : 0-24spc  1-48spc  2-80spc
 * @param uint32 interval_cnt : interval cnt
 * @param uint32 des_adr_mapping : bus destination address mapping [15:0] node[15:0]
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sampling_msg0_control(uint32 sample_rate, uint32 interval_cnt, uint32 des_adr_mapping)
{
    uint32 regVal;

    regVal = ((sample_rate << SAMPLING_MSG_SAMPLE_RATE_SHIFT) & SAMPLING_MSG_SAMPLE_RATE_MASK) |
            ((interval_cnt << SAMPLING_MSG_INTERVAL_CNT_SHIFT) & SAMPLING_MSG_INTERVAL_CNT_MASK) |
            ((des_adr_mapping << SAMPLING_MSG_BUS_DES_ADR_SHIFT) & SAMPLING_MSG_BUS_DES_ADR_MASK);

    Xil_Out32(FPGA_SAMPING_MSG0_CONTROL_REG, regVal);

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sampling msg1 cotrol config
 * @param uint32 sample_rate : 0-24spc  1-48spc  2-80spc
 * @param uint32 interval_cnt : interval cnt
 * @param uint32 des_adr_mapping : bus destination address mapping [15:0] node[15:0]
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sampling_msg1_control(uint32 sample_rate, uint32 interval_cnt, uint32 des_adr_mapping)
{
    uint32 regVal;

    regVal = ((sample_rate << SAMPLING_MSG_SAMPLE_RATE_SHIFT) & SAMPLING_MSG_SAMPLE_RATE_MASK) |
            ((interval_cnt << SAMPLING_MSG_INTERVAL_CNT_SHIFT) & SAMPLING_MSG_INTERVAL_CNT_MASK) |
            ((des_adr_mapping << SAMPLING_MSG_BUS_DES_ADR_SHIFT) & SAMPLING_MSG_BUS_DES_ADR_MASK);

    Xil_Out32(FPGA_SAMPING_MSG1_CONTROL_REG, regVal);
    return 0;
}


/**
 * @Author: wangzhong
 * @Date:
 * @description: set sampling msg0 interval config
 * @param uint32 interval_idx : 0-31
 * @param uint32 src1_mapping : [31:0] channel31 - channel0
 * @param uint32 src2_mapping : [15:0] channel48 - channel32 
 * @param uint32 src_type : 0-AD  1-SV
 * @param uint32 src_choose : 0x00-AD00/SV00   0x01-AD01/SV01  ......  0x1F-AD31/SV31
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sampling_msg0_interval_config(uint32 interval_idx, uint32 src1_mapping, uint32 src2_mapping, uint32 src_type, uint32 src_choose)
{
    uint32 regVal;
    uint32 reg;

    if (interval_idx >= FPGA_SAMPING_MSG_INTERVAL_CNT)
    {
        PRINTF_L1("axi_hp_set_sampling_msg_interval_config para err!  interval_idx=%d \r\n", interval_idx);
        return -1;
    }

    reg = FPGA_SAMPING_MSG0_INTERVAL_CONFIG0_REG + interval_idx * FPGA_SAMPING_MSG_INTERVAL_LEN;
    regVal = ((src2_mapping << SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_SHIFT) & SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_MASK) |
            ((src_type << SAMPLING_INTERVAL_SRC_TYPE_SHIFT) & SAMPLING_INTERVAL_SRC_TYPE_MASK) |
            ((src_choose << SAMPLING_INTERVAL_SRC_CHOOSE_SHIFT) & SAMPLING_INTERVAL_SRC_CHOOSE_MASK);
    Xil_Out32(reg, regVal);

    reg = FPGA_SAMPING_MSG0_INTERVAL_CONFIG1_REG + interval_idx * FPGA_SAMPING_MSG_INTERVAL_LEN;
    regVal = src1_mapping;
    Xil_Out32(reg, regVal);

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set sampling msg1 interval config
 * @param uint32 interval_idx : 0-31
 * @param uint32 src1_mapping : [31:0] channel31 - channel0
 * @param uint32 src2_mapping : [15:0] channel48 - channel32 
 * @param uint32 src_type : 0-AD  1-SV
 * @param uint32 src_choose : 0x00-AD00/SV00   0x01-AD01/SV01  ......  0x1F-AD31/SV31
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_sampling_msg1_interval_config(uint32 interval_idx, uint32 src1_mapping, uint32 src2_mapping, uint32 src_type, uint32 src_choose)
{
    uint32 regVal;
    uint32 reg;

    if (interval_idx >= FPGA_SAMPING_MSG_INTERVAL_CNT)
    {
        PRINTF_L1("axi_hp_set_sampling_msg_interval_config para err!  interval_idx=%d \r\n", interval_idx);
        return -1;
    }

    reg = FPGA_SAMPING_MSG1_INTERVAL_CONFIG0_REG + interval_idx * FPGA_SAMPING_MSG_INTERVAL_LEN;
    regVal = ((src2_mapping << SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_SHIFT) & SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_MASK) |
            ((src_type << SAMPLING_INTERVAL_SRC_TYPE_SHIFT) & SAMPLING_INTERVAL_SRC_TYPE_MASK) |
            ((src_choose << SAMPLING_INTERVAL_SRC_CHOOSE_SHIFT) & SAMPLING_INTERVAL_SRC_CHOOSE_MASK);
    Xil_Out32(reg, regVal);

    reg = FPGA_SAMPING_MSG1_INTERVAL_CONFIG1_REG + interval_idx * FPGA_SAMPING_MSG_INTERVAL_LEN;
    regVal = src1_mapping;
    Xil_Out32(reg, regVal);

    return 0;
}


/**
 * @Author: wangzhong
 * @Date:
 * @description: set goose storm threshold
 * @param uint32 threshold1_bytes_per_us : bytes / 1us
 * @param uint32 threshold2_packs_per_100us : packets / 100us
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_goose_storm_threshold(uint32 threshold1_bytes_per_us, uint32 threshold2_packs_per_100us)
{
    uint32 regAddr;
    uint32 regVal = 0;

    /* threshold2_packs_per_100us bit 31:28  threshold2_packs_per_100us bit 27:22 */
    regAddr = FPGA_GOOSE_CTRL_REG;
    regVal = ((threshold1_bytes_per_us & 0xF) << 28) | ((threshold2_packs_per_100us & 0x3F) << 22);
    Xil_Out32(regAddr, regVal);

    return 0;
}


/**
 * @Author: wangzhong
 * @Date:
 * @description: get goose storm threshold
 * @param uint32* threshold1_bytes_per_us : bytes / 1us
 * @param uint32* threshold2_packs_per_100us : packets / 100us
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_get_goose_storm_threshold(uint32 *threshold1_bytes_per_us, uint32 *threshold2_packs_per_100us)
{
    uint32 regVal = 0;
    uint32 regAddr;

    if ((NULL == threshold1_bytes_per_us) || (NULL == threshold2_packs_per_100us))
        return -1;

    /* threshold2_packs_per_100us bit 31:28  threshold2_packs_per_100us bit 27:22 */
    regAddr = FPGA_GOOSE_CTRL_REG;
    regVal = Xil_In32(regAddr);

    *threshold1_bytes_per_us = (regVal >> 28) & 0xF;
    *threshold2_packs_per_100us = (regVal >> 22) & 0x3F;

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: enable or disable goose storm control
 * @param uint32 filter_idx : goose filter idx 0-95
 * @param uint32 enable :  1:enable  0:disable
 * @return: 0: success  other: fail
 */

SECTION_PLT_CODE STATUS axi_hp_enable_goose_storm_control(uint32 filter_idx, uint32 enable)
{
    uint32 regAddr;
    uint32 regVal;
    uint32 offset;
    uint32 bit;

    if (filter_idx >= FPGA_GOOSE_RECIEVE_FILETER_CONST)
        return -1;

    offset = filter_idx / 32;
    bit = filter_idx % 32;

    regAddr = FPGA_GOOSE_STORM_CONTROL_REG + offset * 4;
    regVal = Xil_In32(regAddr);

    if (enable)
        regVal |= (0x01 << bit);
    else
        regVal &= ~(0x01 << bit);

    Xil_Out32(regAddr, regVal);

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: get goose storm control status
 * @param uint32 filter_idx : goose filter idx 0-95
 * @param uint32* enable :  1:enable  0:disable
 * @return: 0: success  other: fail
 */

SECTION_PLT_CODE STATUS axi_hp_get_goose_storm_control_status(uint32 filter_idx, uint32 *enable)
{
    uint32 regAddr;
    uint32 regVal;
    uint32 offset;
    uint32 bit;

    if (filter_idx >= FPGA_GOOSE_RECIEVE_FILETER_CONST)
        return -1;

    offset = filter_idx / 32;
    bit = filter_idx % 32;

    regAddr = FPGA_GOOSE_STORM_CONTROL_STATUS_REG + offset * 4;
    regVal = Xil_In32(regAddr);

    if (regVal & (0x01 << bit))
        *enable = 1;
    else
        *enable = 0;

    return 0;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: set goose filter
 * @param uint32 filter_idx : idx of filter
 * @param uint32 crc32 : crc value
 * @param uint32 in_port_bitwise : receive port one port per bit
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_set_goose_filter(uint32 filter_idx, uint32 crc32, uint32 in_port_bitwise)
{
    uint32 regAddr;

    if (filter_idx >= FPGA_GOOSE_RECIEVE_FILETER_CONST)
        return -1;

    regAddr = FPGA_GOOSE_FILTER_CRC_REG + (filter_idx * 8);
    Xil_Out32(regAddr, crc32);

    regAddr = FPGA_GOOSE_FILTER_CTRL_REG + (filter_idx * 8);
    Xil_Out32(regAddr, ((in_port_bitwise & 0xFFFF) << 16));

    return 0;
}

//读取goose过滤控制
SECTION_PLT_CODE STATUS axi_hp_get_goose_filter(uint32 filter_idx, uint32 * crc32, uint32 * in_port_bitwise)
{
     uint32 regAddr;
     if (filter_idx >= FPGA_GOOSE_RECIEVE_FILETER_CONST)
         return XST_FAILURE;

     regAddr = FPGA_GOOSE_FILTER_CRC_REG + (filter_idx * 8);
     *crc32 = Xil_In32(regAddr);
     regAddr = FPGA_GOOSE_FILTER_CTRL_REG + (filter_idx * 8);
     *in_port_bitwise = (Xil_In32(regAddr) >> 16) & 0xFFFF;
     return XST_SUCCESS;
}

/**
 * @Author: wangzhong
 * @Date:
 * @description: regist axi hp msg callback
 * @param axi_hp_msg_handle callback: call back function
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS regist_axi_hp_msg_callback(axi_hp_msg_handle callback)
{
    ASSERT_L1(callback != NULL);
    gAxiHpCtrl.callback = callback;

    return 0;
}

// todo axi_hp_sys_init
/**
 * @Author: wangzhong
 * @Date:
 * @description: init axi hp
 * @param {type}
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_sys_init(void)
{
    uint32 regVal;
    
    if (gAxiHpCtrl.isInitialized)
        return -1;
    
    PRINTF_L1("FPGA VERSION REG %x\r\n", (uint32)Xil_In32(FPGA_VERSION_REG));
    PRINTF_L1("FPGA CTRL REG %x\r\n", (uint32)Xil_In32(FPGA_CTRL_REG));

    //alloc memory
    gAxiHpCtrl.txBuf = (uint8 *)(((uint32)axiTxMsgBuf + (ALIGN_LEN - 1)) & (~(ALIGN_LEN - 1)));
    gAxiHpCtrl.rxBuf = (uint8 *)(((uint32)axiRxMsgBuf + (ALIGN_LEN - 1)) & (~(ALIGN_LEN - 1)));

    gAxiHpCtrl.txIdx = 0;
    gAxiHpCtrl.rxIdx = 0;

    //init axi_hp fpga register
    Xil_Out32(FGPA_SV_AXI_HP_TX_BUF_ADDR_REG, (uint32)gAxiHpCtrl.txBuf);
    Xil_Out32(FGPA_SV_AXI_HP_RX_BUF_ADDR_REG, (uint32)gAxiHpCtrl.rxBuf);

    PRINTF_L1("txBuf=%x rxBuf=%x\r\n", (uint32)gAxiHpCtrl.txBuf, (uint32)gAxiHpCtrl.rxBuf);

    // enable pps output
    regVal = (uint32)Xil_In32(FPGA_CTRL_REG);
    regVal = regVal | 
            (0x01 << PPS_1X_OUT_SHIFT) |
            (0x01 << PPS_2X_OUT_SHIFT) |
            (0x01 << PPS_3X_OUT_SHIFT) ;
    Xil_Out32(FPGA_CTRL_REG, regVal);
    regVal = Xil_In32(FPGA_CTRL_REG);
            
    gAxiHpCtrl.isInitialized = 1;

    return 0;
}


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
SECTION_PLT_CODE STATUS axi_hp_fill_goose_msg_head(uint8 *buf, uint32 send_port, uint32 msgLen)
{
    if (NULL == buf)
        return -1;

#ifdef FPGA_HP_MSG_LITTLE_ENDIAN

    /* add 4 byte extra head */
    msgLen += 4;

    buf[3] = 0;
    buf[2] = 0;
    buf[1] =  (msgLen >> 8) & 0x0F;
    buf[0] =  (msgLen & 0xFF);


    buf[7] = AXI_HP_MSG_TYPE_GOOSE & 0xFF;
    buf[6] = (uint8)((send_port & 0xFF00) >> 8);
    buf[5] = (uint8)(send_port & 0x00FF);
    buf[4] = 0;
#else
    /* add 4 byte extra head */
    msgLen += 4;

    buf[0] = 0;
    buf[1] = 0;
    buf[2] =  (msgLen >> 8) & 0x0F;
    buf[3] =  (msgLen & 0xFF);


    buf[4] = AXI_HP_MSG_TYPE_GOOSE & 0xFF;
    buf[5] = (uint8)((send_port & 0xFF00) >> 8);
    buf[6] = (uint8)(send_port & 0x00FF);
    buf[7] = 0;
#endif

    return 0;
}


/**
 * @Author: wangzhong
 * @Date:
 * @description: send msg from axi hp
 * @param uint8 * msg: msg
 * @param uint32 msgLen: len
 * @return: 0: success  other: fail
 */
SECTION_PLT_CODE STATUS axi_hp_send_msg(uint8 *msg, uint32 msgLen)
{
    uint32 txBdIdx;
    uint8 *txBuf;
    uint32 offset;
    uint32 bit;
    uint32 regAddr;
    uint32 regVal;

    txBdIdx = gAxiHpCtrl.txIdx;
    txBuf = gAxiHpCtrl.txBuf + txBdIdx * MAX_AXI_HP_MSG_SIZE;

    if (!gAxiHpCtrl.isInitialized)
    {
        PRINTF_L1("axi_hp_send_msg gAxiHpCtrl is not isInitialized\r\n");
        return -1;
    }

    if (!msg)
    {
        PRINTF_L1("axi_hp_send_msg msg is NULL !\r\n");
        return -1;
    }

    // check tx is busy
    offset = 3 - txBdIdx / 32;
    bit = txBdIdx % 32;
    regAddr = FGPA_SV_AXI_HP_TX_BD_REG + offset * 4;
    regVal = Xil_In32(regAddr);

    // tx not complete
    if (regVal & (0x01 << bit))
        return -1;

    // copy msg body
    msgLen = (((msgLen + 3) >> 2) << 2);
    memcpy(txBuf, msg, msgLen);

    // flush cache
    //Xil_DCacheFlushRange((INTPTR)txBuf, msgLen);

    // write fpga reg
    regVal = 0;
    regVal |= (0x01 << bit);
    Xil_Out32(regAddr, regVal);

    // update tx idx
    gAxiHpCtrl.txIdx++;
    gAxiHpCtrl.txIdx = (gAxiHpCtrl.txIdx % MAX_AXI_HP_TX_MSG_CNT);

    return 0;
}

SECTION_PLT_CODE STATUS axi_goose_send_msg(uint32 send_port,uint8 *msg, uint32 msgLen)
{
	uint8 buf[8];
	uint32 txBdIdx;
    uint8 *txBuf;
    uint32 offset;
    uint32 bit;
    uint32 regAddr;
    uint32 regVal;
	
	if (NULL == msg)
        return -1;

    /* add 4 byte extra head */
#ifdef FPGA_HP_MSG_LITTLE_ENDIAN

    /* add 4 byte extra head */
    msgLen += 4;

    buf[3] = 0;
    buf[2] = 0;
    buf[1] =  (msgLen >> 8) & 0x0F;
    buf[0] =  (msgLen & 0xFF);


    buf[7] = AXI_HP_MSG_TYPE_GOOSE & 0xFF;
    buf[6] = (uint8)((send_port & 0xFF00) >> 8);
    buf[5] = (uint8)(send_port & 0x00FF);
    buf[4] = 0;
#else
    /* add 4 byte extra head */
    msgLen += 4;

    buf[0] = 0;
    buf[1] = 0;
    buf[2] =  (msgLen >> 8) & 0x0F;
    buf[3] =  (msgLen & 0xFF);


    buf[4] = AXI_HP_MSG_TYPE_GOOSE & 0xFF;
    buf[5] = (uint8)((send_port & 0xFF00) >> 8);
    buf[6] = (uint8)(send_port & 0x00FF);
    buf[7] = 0;
#endif

    txBdIdx = gAxiHpCtrl.txIdx;
    txBuf = gAxiHpCtrl.txBuf + txBdIdx * MAX_AXI_HP_MSG_SIZE;

    if (!gAxiHpCtrl.isInitialized)
    {
        PRINTF_L1("axi_hp_send_msg gAxiHpCtrl is not isInitialized\r\n");
        return -1;
    }

    if (!msg)
    {
        PRINTF_L1("axi_hp_send_msg msg is NULL !\r\n");
        return -1;
    }

    // check tx is busy
    offset = 3 - txBdIdx / 32;
    bit = txBdIdx % 32;
    regAddr = FGPA_SV_AXI_HP_TX_BD_REG + offset * 4;
    regVal = Xil_In32(regAddr);

    // tx not complete
    if (regVal & (0x01 << bit))
        return -1;

	
    // copy msg body
    memcpy(txBuf, buf, 8);
    memcpy(txBuf+8, msg, msgLen-4);

    // flush cache
    //Xil_DCacheFlushRange((INTPTR)txBuf, msgLen);

    // write fpga reg
    regVal = 0;
    regVal |= (0x01 << bit);
    Xil_Out32(regAddr, regVal);

    // update tx idx
    gAxiHpCtrl.txIdx++;
    gAxiHpCtrl.txIdx = (gAxiHpCtrl.txIdx % MAX_AXI_HP_TX_MSG_CNT);

    return 0;	
}

uint32 backplane_check_cnt = 0;

/**
 * @Author: wangzhong
 * @Date:
 * @description: poll rx msg
 * @return: 0: success  other: fail
 */
 uint8 hpmsg[MAX_AXI_HP_MSG_SIZE];
SECTION_PLT_CODE STATUS axi_hp_poll_msg(void)
{
    uint32 regAddr;
    uint32 regVal;
    uint32 rxBdIdx;
    uint32 msgLen;
    uint8 *rxBuf;
    uint8 msgType;
    uint8 bit;
    uint8 offset;

    rxBdIdx = gAxiHpCtrl.rxIdx;
    rxBuf = gAxiHpCtrl.rxBuf + rxBdIdx * MAX_AXI_HP_MSG_SIZE;

    if (!gAxiHpCtrl.isInitialized)
        return -1;

    // check rx has msg
    offset = 3 - rxBdIdx / 32;
    bit = rxBdIdx % 32;
    regAddr = FGPA_SV_AXI_HP_RX_BD_REG + offset * 4;
    regVal = Xil_In32(regAddr);

    // no msg
    if (!(regVal & (0x01 << bit)))
        return -1;

    // invalid cache
    //Xil_DCacheInvalidateRange((INTPTR)rxBuf, MAX_AXI_HP_MSG_SIZE);
    
#ifdef FPGA_HP_MSG_LITTLE_ENDIAN
    // get msg type
    msgType = rxBuf[7];

    // get msg len
    msgLen = ((rxBuf[1] & 0xF) << 8) + rxBuf[0];
    msgLen -= 4;
#else
    // get msg type
    msgType = rxBuf[4];

    // get msg len
    msgLen = ((rxBuf[2] & 0xF) << 8) + rxBuf[3];
    msgLen -= 4;
#endif

    if (msgLen > MAX_AXI_HP_MSG_SIZE)
    {
        PRINTF_L1("axi_hp_poll_msg msgLen Err len=%d \r\n", msgLen);
        goto DISCARD_MSG;
    }

	memcpy(hpmsg, rxBuf,msgLen + MAX_AXI_HP_MSG_HEAD_LEN +3); //访问rxBuf太慢
	rxBuf = hpmsg;
	
    // process msg
    if (gAxiHpCtrl.callback)
        gAxiHpCtrl.callback(rxBuf, msgLen + MAX_AXI_HP_MSG_HEAD_LEN, msgType);

    if (hd_fpga_debug)
    {
        uint32 *pMsg;
        uint32 i;
        uint32 len;

        PRINTF_L1("eth recv msg len=%d msgType=%x\r\n", msgLen + MAX_AXI_HP_MSG_HEAD_LEN, msgType);

        pMsg = (uint32 *)rxBuf;
        len = (msgLen + MAX_AXI_HP_MSG_HEAD_LEN) / 4;
        if ((msgLen + MAX_AXI_HP_MSG_HEAD_LEN) % 4)
            len++;

        for (i=0; i<len; i++)
        {
            PRINTF_L1("%08x", pMsg[i]);

            if ((i+1) % 4)
            {
                PRINTF_L1(" ");
            }
            else
            {
                PRINTF_L1("\r\n");
            }
        }
        PRINTF_L1("\r\n");
    }

DISCARD_MSG:
    // update RX BD
    gAxiHpCtrl.rxIdx++;
    gAxiHpCtrl.rxIdx %= MAX_AXI_HP_RX_MSG_CNT;

    regVal = 0;
    regVal |= (0x01 << bit);
    Xil_Out32(regAddr, regVal);

    return 0;
}

//读取FPGA版本寄存器的值
SECTION_PLT_CODE uint32 axi_get_fpga_version(void)
{
    return Xil_In32(FPGA_VERSION_REG);
}

//读取FPGA控制寄存器的值
SECTION_PLT_CODE uint32 axi_get_fpga_ctrl(void)
{
    return Xil_In32(FPGA_CTRL_REG);
}

SECTION_PLT_CODE uint8 FPGA_CPLD_READ (DWORD addr)
{	
	int i;
	uint8 regVal = 0;
	regVal = Xil_In32(FPGA_CPLD_BASE + addr);
	//hd_sleep_useconds(60);
	for(i = 0; i < 100; i++);
	
	regVal = Xil_In32(FPGA_CPLD_BASE + addr);
	return regVal;
}

SECTION_PLT_CODE void FPGA_CPLD_WRITE (DWORD addr,BYTE regVal)
{	
	int i;
	Xil_Out32(FPGA_CPLD_BASE + addr, regVal);
	for(i = 0; i < 50; i++);
}



