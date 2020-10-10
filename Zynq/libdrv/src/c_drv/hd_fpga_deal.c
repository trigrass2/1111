/*
 * @Author: zhongwei
 * @Date: 2020/3/31 17:44:09
 * @Description: FPGA处理接口
 * @File: hd_fpga_deal.c
 *
*/

#include "plt_inc_c.h"
#include "libdrv.h"

/************************************************************************/
//  FPGA通用寄存器
//
/************************************************************************/

//打印FPGA通用寄存器信息
SECTION_PLT_CODE void Print_fpga_common_cfg(void)
{
    uint32 version = axi_get_fpga_version();
    PRINTF("FPGA Version = 0x%x\n\r", version);
    //控制寄存器
    {
        uint32 ctrl = axi_get_fpga_ctrl();
        //外部对时信号源
        {
            uint8 gps = (ctrl & EXT_TIME_SOURCE_MASK) >> EXT_TIME_SOURCE_SHIFT;
            const char * szGps = "N/A";
            switch (gps)
            {
            case 2:
                szGps = "optical positive IRIG-B";
                break;
            case 3:
                szGps = "optical negtive IRIG-B";
                break;
            case 4:
                szGps = "electrical positive IRIG-B";
                break;
            case 5:
                szGps = "electrical negtive IRIG-B";
                break;
            }
            PRINTF("External GPS : %s\n\r", szGps);
        }
        //1pps
        PRINTF("1PPSx1 : %d\n\r", (ctrl & PPS_1x_OUT_MASK) >> PPS_1X_OUT_SHIFT);
        PRINTF("1PPSx2 : %d\n\r", (ctrl & PPS_2x_OUT_MASK) >> PPS_2X_OUT_SHIFT);
        PRINTF("1PPSx3 : %d\n\r", (ctrl & PPS_3x_OUT_MASK) >> PPS_3X_OUT_SHIFT);
        //采样值同步机制
        {
            const char * szSync = "N/A";
            uint8 sv_sync = (ctrl & SV_SYNC_TYPE_MASK) >> SV_SYNC_TYPE_SHIFT;
            switch (sv_sync)
            {
            case 0:
                szSync = "networking";
                break;
            case 2:
                szSync = "direct";
                break;
            case 3:
                szSync = "delay measurable";
                break;
            }

            PRINTF(" SV Sync Type : %s\n\r", szSync);
        }
        //采样值延时控制
        PRINTF(" SV delay us : %d\n\r", (ctrl & SV_DELAY_MASK) >> SV_DELAY_SHIFT);
        //总线节点编号
        PRINTF(" BUS No : %d\n\r", (ctrl & BUS_ADR_MASK) >> BUS_ADR_SHIFT);

        //采样周期
        PRINTF(" Sample period : %d\n\r", axi_hp_get_timer_period());
    }
}

//打印FPGA 外部对时信息 外部对时寄存器
SECTION_PLT_CODE void Print_fpga_gps_info(void)
{
    uint32 data[3];
    axi_hp_get_gps_time(data);

    TFPGA_GPS_TIME gps_time;
    fpga_gps_time_decode(data, &gps_time);
    uint8 state;
    uint32 ns;
    fpga_gps_time_ns_decode(data[2], &state, &ns);

    uint8 pps1_state, pps2_state, pps3_state;
    uint32 pps1_ns, pps2_ns;
    axi_hp_get_1pps_x1(&pps1_state, &pps1_ns);
    axi_hp_get_1pps_x2(&pps2_state, &pps2_ns);
    pps3_state = axi_hp_get_1pps_x3();

    uint32 timer_period = axi_hp_get_timer_period();
    uint32 timer_counter = axi_hp_get_timer_counter();

    //打印输出 IRIG-B
    {
        c64_t ch;
        PRINTF("External GPS Info : state=%d 0x%08x %08x %08x\n\r",state, data[0], data[1], data[2]);
        TFullDateTime fdt = IRIG_to_FullDateTime(gps_time.year, gps_time.day, gps_time.hour, gps_time.minute, gps_time.second);
        PRINTF(" Time : %s\n\r", str_GetFullDateTimeWithoutMsString(&fdt, ch));
        PRINTF(" ns : %u\n\r", ns);
        PRINTF(" leap_second : forecast=%d flag=%d\n\r", gps_time.leap_second_forecast, gps_time.leap_second_flag);
        PRINTF(" summer : forecast=%d flag=%d\n\r", gps_time.summer_forecast, gps_time.summer_flag);
        PRINTF(" offset : %s%d.%d\n\r", gps_time.t_offset_sign ? "-" : "+", gps_time.t_offset, gps_time.t_offset_half?5:0);
        PRINTF(" quality : 0x%x\n\r", gps_time.quality);
    }

    //打印1pps信息
    {
        PRINTF("1PPSx1 : state=%d ns=%u\n\r", pps1_state, pps1_ns);
        PRINTF("1PPSx2 : state=%d ns=%u\n\r", pps2_state, pps2_ns);
        PRINTF("1PPSx3 : state=%d\n\r", pps3_state);
        PRINTF("Sample Timer : period=%d counter=%d\n\r", timer_period, timer_counter);
    }
}

//打印高速背板总线状态信息 FPGA_BACKPLANE_BUS_STATUS_REG
SECTION_PLT_CODE void Print_fpga_backplane_info(void)
{
    uint32 data = axi_hp_get_backplane_status();
    PRINTF("FPGA Back Plane status = 0x%x\n\r", data);
    PRINTF("  Data0 encode : %d\n\r", (data & DATA0_CODE_ERR_MASK) >> DATA0_CODE_ERR_SHIFT);
    PRINTF("  Data1 encode : %d\n\r", (data & DATA1_CODE_ERR_MASK) >> DATA1_CODE_ERR_SHIFT);
    PRINTF("  Data2 encode : %d\n\r", (data & DATA2_CODE_ERR_MASK) >> DATA2_CODE_ERR_SHIFT);
    PRINTF("  Data3 encode : %d\n\r", (data & DATA3_CODE_ERR_MASK) >> DATA3_CODE_ERR_SHIFT);
    PRINTF("  Data0 len or crc : %d\n\r", (data & DATA0_LEN_OR_CRC_ERR_MASK) >> DATA0_LEN_OR_CRC_ERR_SHIFT);
    PRINTF("  Data1 len or crc : %d\n\r", (data & DATA1_LEN_OR_CRC_ERR_MASK) >> DATA1_LEN_OR_CRC_ERR_SHIFT);
    PRINTF("  Data2 len or crc : %d\n\r", (data & DATA2_LEN_OR_CRC_ERR_MASK) >> DATA2_LEN_OR_CRC_ERR_SHIFT);
    PRINTF("  Data3 len or crc : %d\n\r", (data & DATA3_LEN_OR_CRC_ERR_MASK) >> DATA3_LEN_OR_CRC_ERR_SHIFT);
    PRINTF("  control right : %d\n\r",  (data & GET_BUS_CONTROL_FAIL_MASK) >> GET_BUS_CONTROL_FAIL_SHIFT);
    PRINTF("  timing sequence : %d\n\r",  (data & TRANS_SEQ_ERR_MASK) >> TRANS_SEQ_ERR_SHIFT);
    PRINTF("  message bit : %d\n\r",  (data & MSG_BIT_ERR_MASK) >> MSG_BIT_ERR_SHIFT);
    PRINTF("  overflow : %d\n\r",  (data & BUF_OVERFLOW_ERR_MASK) >> BUF_OVERFLOW_ERR_SHIFT);  
}

//打印FPGA网口信息
SECTION_PLT_CODE void Print_fpga_eth_info(void)
{
    uint8 mac_addr[8];
    for (int i=0;i<FPGA_ETH_PORT_COUNT;i++)
    {
        axi_hp_get_eth_mac_addr(mac_addr, i);       //读取mac地址
        uint8 status = axi_hp_get_eth_status(i);    //网口状态
        uint32 replace_en;
        axi_hp_get_mac_addr_replace_enable(&replace_en, i);
        c32_t ch;
        PRINTF("MAC[%02d] : addr=%s status=0x%x replace=%d\n\r", i, str_GetMacAddrString(mac_addr,ch), status, replace_en);
    }
}

/************************************************************************/
//  FPGA时间寄存器
//
/************************************************************************/
//IRIG-B码时间 奇校验检查
SECTION_PLT_CODE BOOL fpga_gps_time_odd_chk(uint32 data[3])
{
    return PLT_TRUE;
}

//FPGA时间寄存器解码 -> TFPGA_GPS_TIME
/* 
[0] 
位#   默认 读/写   描述
31-30   0   RO  保留
29-28   0   RO  天(百位BCD码)
27-24   0   RO  天(十位BCD码)
23-20   0   RO  天(个位BCD码)
19-18   0   RO  时(十位BCD码)
17-14   0   RO  时(个位BCD码)
13-11   0   RO  分(十位BCD码)
10-07   0   RO  分(个位BCD码)
06-04   0   RO  秒(十位BCD码)
03-00   0   RO  秒(个位BCD码)
[1] 
位#   默认 读/写   描述
31-24   0   RO  保留
23      0   RO  校验位(奇校验)
22-19   0   RO  时间质量
18      0   RO  时间偏移(0.5小时)
17-14   0   RO  时间偏移(小时)
13      0   RO  时间偏移符号位
12      0   RO  夏时制标志
11      0   RO  夏时制预告
10      0   RO  闰秒标志
09      0   RO  闰秒预告
08-05   0   RO  年(十位BCD码)
04      0   RO  索引位
03-00   0   RO  年(个位BCD码)
*/ 
SECTION_PLT_CODE void fpga_gps_time_decode(uint32 data[2], TFPGA_GPS_TIME * pTime)
{
    uint32 reg = data[0];
    pTime->second = (reg & 0xF) + 10 * ((reg >> 4) & 0x7);
    pTime->minute = ((reg>> 7) & 0xF) + 10 * ((reg >> 11) & 0x7);
    pTime->hour = ((reg>> 14) & 0xF) + 10 * ((reg >> 18) & 0x3);
    pTime->day = ((reg >> 20) & 0xF) + 10 * ((reg >> 24) & 0xF) + 100 * ((reg >> 28) & 0x3);
    reg = data[1];
    pTime->year = (reg & 0xF) + 10 * ((reg >> 5) & 0xF);
    pTime->leap_second_forecast = (reg >> 9) & 0x01;
    pTime->leap_second_flag = (reg >> 10) & 0x01;
    pTime->summer_forecast = (reg >> 11) & 0x01;
    pTime->summer_flag = (reg >> 12) & 0x01;
    pTime->t_offset_sign = (reg >> 13) & 0x01;
    pTime->t_offset = (reg >> 14) & 0x0F;
    pTime->t_offset_half = (reg >> 18) & 0x01;
    pTime->quality = (reg >> 19) & 0x0F;
}

/**
 * @Function: fpga_gps_time_ns_decode
 * @Description: ns计数器解码 
 * @author zhongwei (2020/4/2 15:47:28)
 * 
 * @param data2 外部对时寄存器2 FPGA_GPS_TIME_2_REG的值
 * @param state 外部对时状态 0:异常 1:正常
 * @param ns 纳秒时间
 * 
 * @return SECTION_PLT_CODE void 
 *  
 * 
位# 默认    读/写   描述
31  0   RO  外部对时状态
0:异常
1:正常
30  0   RO  保留
29-00   0   RO  纳秒计数器(0~999_999_999ns)
*/
SECTION_PLT_CODE void fpga_gps_time_ns_decode(uint32 data2, uint8 * state, uint32 * ns)
{
    *state = (data2>>31) & 0x01;
    *ns = data2 & 0x3FFFFFFF;
}

/************************************************************************/
//  Goose接收处理
//
/************************************************************************/

/**
 * @Function: fpga_calc_goose_rcv_crc32
 * @Description: 计算GOOSE接收报文”目的MAC+APPID+GoCBRef+DatSet+GoID”数据段CRC-32生成校验码 ，用于Goose接收过滤
 * @author zhongwei (2020/3/31 17:48:27)
 * 
 * @param dest_mac 目的MAC
 * @param app_id APP ID
 * @param gocbRef 
 * @param datSet 
 * @param goID 
 * 
 * @return SECTION_PLT_CODE uint32 返回crc32的值
 * 
*/
SECTION_PLT_CODE uint32 fpga_calc_goose_rcv_crc32(const uint8 * dest_mac, 
                                                  uint16 app_id, 
                                                  const char * gocbRef,
                                                  const char * datSet,
                                                  const char * goID)
{
    uint8 ch[8];
    uint32 crc32 = 0xFFFFFFFF;
    //mac地址
    crc32 = crc32_calc(crc32, dest_mac, 6);
    //app id （高位在前）
    ch[0] = HIBYTE(app_id);
    ch[1] = LOBYTE(app_id);
    crc32 = crc32_calc(crc32, ch, 2);
    //gocbRef
    crc32 = crc32_calc(crc32, (const uint8 *)gocbRef, strlen(gocbRef));
    //datSet
    crc32 = crc32_calc(crc32, (const uint8 *)datSet, strlen(datSet));
    //goID
    crc32 = crc32_calc(crc32, (const uint8 *)goID, strlen(goID));

    crc32 = crc32_bit_reverse(crc32);

    return crc32;
}

SECTION_PLT_DATA int _goose_rcv_filter_idx = 0;     //当前空闲的Goose接收过滤器的序号，最大FPGA_GOOSE_RECIEVE_FILETER_CONST(96)个

/**
 * @Function: fpga_add_one_goose_rcv
 * @Description: 添加一个goose接收过滤器 
 * @author zhongwei (2020/4/1 9:03:51)
 * 
 * @param dest_mac 
 * @param app_id 
 * @param gocbRef 
 * @param datSet 
 * @param goID 
 * @param in_port_bitwise GOOSE接收报文源端口映射 BIT0-15 外部网口0-15(当前用3~8)
 * 
 * @return SECTION_PLT_CODE STATUS 
 * 
*/
SECTION_PLT_CODE STATUS fpga_add_one_goose_rcv(const uint8 * dest_mac, 
                                                  uint16 app_id, 
                                                  const char * gocbRef,
                                                  const char * datSet,
                                                  const char * goID,
                                                  uint32 in_port_bitwise)
{
    if (_goose_rcv_filter_idx >= FPGA_GOOSE_RECIEVE_FILETER_CONST)
    {
        c32_t ch1, ch2;
        PRINTF_L1("fpga_add_one_goose_rcv fail, goose_rcv_filter overflow, dest_mac=%s appid=%s gocbRef=%s datSet=%s goID=%s in_port_bitwise=0x%x.\r\n",
                                str_GetMacAddrString(dest_mac, ch1),
                                str_GetAppIdString(app_id, ch2),
                                gocbRef,datSet,goID,in_port_bitwise);
        return XST_FAILURE;
    }

    uint32 crc32 = fpga_calc_goose_rcv_crc32(dest_mac, app_id, gocbRef, datSet, goID);
    if (axi_hp_set_goose_filter(_goose_rcv_filter_idx, crc32, in_port_bitwise) != XST_SUCCESS)
    {
        return XST_FAILURE;
    }
    {
    c32_t ch1, ch2;
	PRINTF("fpga_add_one_goose_rcv idx=%d crc=0x%x port=0x%x mac=%s appid=%s gocbRef=%s datSet=%s goID=%s\r\n",
                              _goose_rcv_filter_idx, crc32, in_port_bitwise,
                              str_GetMacAddrString(dest_mac, ch1),
                              str_GetAppIdString(app_id, ch2),
                              gocbRef,datSet,goID);
    }

//  if (IsDebugMessage(DEBUG_MESSAGE_GOOSE_RCV))
//  {
//      c32_t ch1, ch2;
//      PRINTF("fpga_add_one_goose_rcv idx=%d crc=0x%x port=0x%x mac=%s appid=%s gocbRef=%s datSet=%s goID=%s\r\n",
//                              _goose_rcv_filter_idx, crc32, in_port_bitwise,
//                              str_GetMacAddrString(dest_mac, ch1),
//                              str_GetAppIdString(app_id, ch2),
//                              gocbRef,datSet,goID,in_port_bitwise);
//  }

    _goose_rcv_filter_idx++;

    return XST_SUCCESS;
}

//打印FPGA Goose接收配置
SECTION_PLT_CODE void Print_fpga_goose_rcv_cfg(void)
{
    PRINTF("FPGA Goose Recieve config:\r\n");
    {
        uint32 threshold1_bytes_per_us;
        uint32 threshold2_packs_per_100us;
        if(axi_hp_get_goose_storm_threshold(&threshold1_bytes_per_us, &threshold2_packs_per_100us) == XST_SUCCESS)
        {
            PRINTF("  threshold1_bytes_per_us = %d\n\r", threshold1_bytes_per_us);
            PRINTF("  threshold2_packs_per_100us = %d\n\r", threshold2_packs_per_100us);
        }
    }

    {
        uint32 reg = Xil_In32(FPGA_GOOSE_FUN_REG);
        PRINTF("  available filter count = %d\n\r", reg & 0xFF);
    }

    {
        PRINTF(" storm control enable = 0x%08x %08x %08x\n\r",
                Xil_In32(FPGA_GOOSE_STORM_CONTROL_REG),
                Xil_In32(FPGA_GOOSE_STORM_CONTROL_REG+4),
                Xil_In32(FPGA_GOOSE_STORM_CONTROL_REG+8));
        PRINTF(" storm control status = 0x%08x %08x %08x\n\r",
                Xil_In32(FPGA_GOOSE_STORM_CONTROL_STATUS_REG),
                Xil_In32(FPGA_GOOSE_STORM_CONTROL_STATUS_REG+4),
                Xil_In32(FPGA_GOOSE_STORM_CONTROL_STATUS_REG+8));
    }

    //打印报文过了寄存器
    {
        PRINTF(" configured filter count : %d\n\r", _goose_rcv_filter_idx); //当前配置了的过滤器个数
        for (int i=0;i<FPGA_GOOSE_RECIEVE_FILETER_CONST;i++)
        {
            uint32 crc32;
            uint32 in_port_bitwise;
            if (axi_hp_get_goose_filter(i, &crc32, &in_port_bitwise) == XST_SUCCESS)
            {
                if (in_port_bitwise != 0)   //使能
                {
                    PRINTF(" filter[%d]: crc32=0x%08x port=0x%x\n\r", i, crc32, in_port_bitwise);
                }
            }
        }
    }
}

/************************************************************************/
//  SV接收处理
//
/************************************************************************/

//打印FPGA SV接收配置
SECTION_PLT_CODE void Print_fpga_sv_cfg(void)
{
    //SV接收 报文控制寄存器
    for (int idx=0;idx<FPGA_SV_RECEIVE_REGION_MAX_CNT;idx++)
    {
        TSV_RECEIVE_CONTROL svCtrl;
        axi_hp_get_sv_receive_control(idx, &svCtrl);
        if (svCtrl.sv_rcv_enable)   //使能了SV接收
        {
            c32_t ch;
            PRINTF("SVRcv[%d] net_port=%d delay_enable=%d phase=%d\n\r", idx, svCtrl.sv_rcv_net_port, svCtrl.sv_rcv_rated_delay_enable, svCtrl.sv_rcv_phase_angle);
            TSV_RECEIVE_FILTER svFilter;
            axi_hp_get_sv_receive_filter(idx, &svFilter);
            PRINTF("  APPID : %s\n\r", str_GetAppIdString(svFilter.APPID,ch));
            PRINTF("  MAC   : %s\n\r", str_GetMacAddrString(svFilter.des_mac_addr, ch));
            PRINTF("  CRC32 : 0x%08x\n\r", svFilter.svid_crc32);
            PRINTF("  confRev: %d\n\r", svFilter.confRev);
            PRINTF("  chan_cnt: 0xd\n\r", svFilter.chan_cnt);
        }
    }

    //采样值上送0 配置  
    {
        //FPGA_SAMPING_MSG0_CONTROL_REG 采样控制寄存器
        uint32 nCtrlReg = Xil_In32(FPGA_SAMPING_MSG0_CONTROL_REG);
        uint32 sample_rate = (nCtrlReg & SAMPLING_MSG_SAMPLE_RATE_MASK) >> SAMPLING_MSG_SAMPLE_RATE_SHIFT;
        uint32 interval_cnt = (nCtrlReg & SAMPLING_MSG_INTERVAL_CNT_MASK) >> SAMPLING_MSG_INTERVAL_CNT_SHIFT;
        uint32 des_adr_mapping = (nCtrlReg & SAMPLING_MSG_BUS_DES_ADR_MASK) >> SAMPLING_MSG_BUS_DES_ADR_SHIFT;
        if (interval_cnt > 0)
        {
            const char * szrate="N/A";
            if (sample_rate == SAMPLING_RATE_24SPC)
            {
                szrate = "24";
            }
            else if (sample_rate == SAMPLING_RATE_48SPC)
            {
                szrate = "48";
            }
            else if (sample_rate == SAMPLING_RATE_80SPC)
            {
                szrate = "80";
            }
			else if (sample_rate == SAMPLING_RATE_120SPC)
            {
                szrate = "120";
            }
            PRINTF("SMAPLE_MSG0 - rate=%s bay_cnt=%d dest_node=0x%x\n\r", szrate, interval_cnt, des_adr_mapping);

            //间隔配置
            for (int idx=0;idx<interval_cnt;idx++)
            {
                uint32 uData0 = Xil_In32(FPGA_SAMPING_MSG0_INTERVAL_CONFIG0_REG + idx * FPGA_SAMPING_MSG_INTERVAL_LEN);
                uint32 uData1 = Xil_In32(FPGA_SAMPING_MSG0_INTERVAL_CONFIG1_REG + idx * FPGA_SAMPING_MSG_INTERVAL_LEN);
                uint32 src1_mapping = uData1;
                uint32 src2_mapping = (uData0 & SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_MASK) >> SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_SHIFT;
                uint32 src_type = (uData0 & SAMPLING_INTERVAL_SRC_TYPE_MASK) >> SAMPLING_INTERVAL_SRC_TYPE_SHIFT;
                uint32 src_choose = (uData0 & SAMPLING_INTERVAL_SRC_CHOOSE_MASK) >> SAMPLING_INTERVAL_SRC_CHOOSE_SHIFT;
                const char * szType = "N/A";
                if (src_type == SAMPLING_SRC_TYPE_AD)
                {
                    szType="AD";
                }
                else if (src_type == SAMPLING_SRC_TYPE_SV)
                {
                    szType="SV";
                }
                PRINTF("  BAY[%d] SRC_MAP=0x%05x%08x src_type=%s src_choose=%d\n\r", idx, src2_mapping, src1_mapping, szType, src_choose);
            }
        }
    }

    //采样值上送1 配置 
    {
        //FPGA_SAMPING_MSG1_CONTROL_REG 采样控制寄存器
        uint32 nCtrlReg = Xil_In32(FPGA_SAMPING_MSG1_CONTROL_REG);
        uint32 sample_rate = (nCtrlReg & SAMPLING_MSG_SAMPLE_RATE_MASK) >> SAMPLING_MSG_SAMPLE_RATE_SHIFT;
        uint32 interval_cnt = (nCtrlReg & SAMPLING_MSG_INTERVAL_CNT_MASK) >> SAMPLING_MSG_INTERVAL_CNT_SHIFT;
        uint32 des_adr_mapping = (nCtrlReg & SAMPLING_MSG_BUS_DES_ADR_MASK) >> SAMPLING_MSG_BUS_DES_ADR_SHIFT;
        if (interval_cnt > 0)
        {
            const char * szrate="N/A";
            if (sample_rate == SAMPLING_RATE_24SPC)
            {
                szrate = "24";
            }
            else if (sample_rate == SAMPLING_RATE_48SPC)
            {
                szrate = "48";
            }
            else if (sample_rate == SAMPLING_RATE_80SPC)
            {
                szrate = "80";
            }
			else if (sample_rate == SAMPLING_RATE_120SPC)
            {
                szrate = "120";
            }
            PRINTF("SMAPLE_MSG1 - rate=%s bay_cnt=%d dest_node=0x%x\n\r", szrate, interval_cnt, des_adr_mapping);

            //间隔配置
            for (int idx=0;idx<interval_cnt;idx++)
            {
                uint32 uData0 = Xil_In32(FPGA_SAMPING_MSG1_INTERVAL_CONFIG0_REG + idx * FPGA_SAMPING_MSG_INTERVAL_LEN);
                uint32 uData1 = Xil_In32(FPGA_SAMPING_MSG1_INTERVAL_CONFIG1_REG + idx * FPGA_SAMPING_MSG_INTERVAL_LEN);
                uint32 src1_mapping = uData1;
                uint32 src2_mapping = (uData0 & SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_MASK) >> SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_SHIFT;
                uint32 src_type = (uData0 & SAMPLING_INTERVAL_SRC_TYPE_MASK) >> SAMPLING_INTERVAL_SRC_TYPE_SHIFT;
                uint32 src_choose = (uData0 & SAMPLING_INTERVAL_SRC_CHOOSE_MASK) >> SAMPLING_INTERVAL_SRC_CHOOSE_SHIFT;
                const char * szType = "N/A";
                if (src_type == SAMPLING_SRC_TYPE_AD)
                {
                    szType="AD";
                }
                else if (src_type == SAMPLING_SRC_TYPE_SV)
                {
                    szType="SV";
                }
                PRINTF("  BAY[%d] SRC_MAP=0x%05x%08x src_type=%s src_choose=%d\n\r", idx, src2_mapping, src1_mapping, szType, src_choose);
            }
        }
    }
}

