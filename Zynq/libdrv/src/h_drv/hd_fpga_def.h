/*
 * @Author: zhongwei
 * @Date: 2020/3/30 10:12:41
 * @Description: FPGA地址定义
 * @File: hd_fpga_def.h
 * 
 * 参见《FPGA通讯接口规约》文档
 * FPGA基地址 0x40000000
 *  
 * 0x0000     
 * ...          - 模拟量采样 
 * 0x0200 
 * ... 
 * 0x0400 
 * ...          - HSR报文接收
 * 0x0800 
 * ...          - HSR报文发送
 * 0x0C00 
 * ... 
 * 0x1000 
 * ...          - SV报文接收（0）
 * 0x1100 
 * ... 
 * ... ...
 * ... 
 * 0x2F00 
 * ...          - SV报文接收（31）
 * 0x3000 
 * ...          - SV报文发送
 * 0x3800 
 * ... 
 * 0x3C00 
 * ...          - 采样值报文上送（0）
 * 0x3E00 
 * ...          - 采样值报文上送（1）
 * 0x4000 
 * ...          - AXI_HP通信控制器(MMS报文)
 * 0x4100 
 * ...          - AXI_HP通信控制器(采样值报文/Goose报文/节点间报文)
 * 0x4200 
 * ... 
 * 0x4800 
 * ...          - Goose报文接收
 * 0x4C00 
 * ... 
 * 0x8000 
 * ...          - 通用寄存器
 * 0x8400 
*/

#ifndef SRC_H_BAREMETAL_PLT_HD_FPGA_DEF_H_
#define SRC_H_BAREMETAL_PLT_HD_FPGA_DEF_H_

// todo define FPGA MACRO FPGA基地址
#define FPGA_BASE                       0x40000000

#define FPGA_HP_MSG_LITTLE_ENDIAN   //FPGA采用小端模式

/*
   Analog Sampling region 模拟量采样 
模拟量采样模块控制2组ADC采样，每组控制1片ADC芯片共16路模拟量通道，作为2个间隔(AD-00、AD-01)信息上送。 
ADC芯片采用AD7616，该芯片是16位16通道非同步ADC采样芯片，量程正偏最大值32767(+10V)，量程负偏最大值-32768(-10V) ， 
负数采用2进制补码表示。内部采样值报文上送ADC采样数据时直接将16位ADC采样值进行有符号数扩展为32位有符号数上送，不进行系数变换。 
 
模拟量采样模块地址分配(0x000~0x1FF)
0x000       :   滤波延时寄存器
0x010~0x06F :   AD-00采样校准寄存器
0x070~0x0CF :   AD-01采样校准寄存器
*/
#define FPGA_ANALOG_SAMPLING_OFFSET 0x0000
#define FPGA_ANALOG_SAMPLING_FILTER_DELAY_REG           (FPGA_BASE + FPGA_ANALOG_SAMPLING_OFFSET + 0)       //滤波延时寄存器
#define FPGA_ANALOG_SAMPLING_AD_CALIBRATION_REG         (FPGA_BASE + FPGA_ANALOG_SAMPLING_OFFSET + 0x10)    //AD 采样校准寄存器 4字节一个，共32个（AD0和AD1各16个）
#define FPGA_ANALOG_SAMPLING_AD_CALIBRATION_INFO_LEN    0x60
/* FPGA Analog sampling filter delay reg Definitions and Masks 用于配置FPGA_ANALOG_SAMPLING_FILTER_DELAY_REG */
#define ANALOG_SAMPLING_FILTER_DELAY_SHIFT      0U
#define ANALOG_SAMPLING_FILTER_DELAY_MASK       0x000003FFU             //BIT0-BIT9 RW ADC采样前级滤波回路延时(单位:1us)
/* FPGA Analog sampling Calibration reg Definitions and Masks  用于配置FPGA_ANALOG_SAMPLING_AD_CALIBRATION_REG */
#define ANALOG_SAMPLE_ZERO_DRIFT_CTRL_SHIFT     0U 
#define ANALOG_SAMPLE_ZERO_DRIFT_CTRL_MASK      0x0000FFFFU             //BIT0-BIT15 RW 采样通道15去零漂数字量(负数采用二进制补码表示)
#define ANALOG_SAMPLE_PHASE_ANGLE_SHIFT         16U
#define ANALOG_SAMPLE_PHASE_ANGLE_MASK          0x00FF0000U             //BIT16-BIT23 RW 采样通道15调相角(角度单位:分，负数采用二进制补码表示)


/* 
   General config region 通用寄存器
   通用寄存器用于某些全局信号的控制或反应某些状态信息
通用寄存器地址分配(0x00~0x3FF)
0x000       :   版本寄存器
0x004       :   控制寄存器
0x010       :   外部对时寄存器0
0x014       :   外部对时寄存器1
0x018       :   外部对时寄存器2
0x020       :   1PPSx1总线寄存器
0x024       :   1PPSx2总线寄存器
0x028       :   1PPSx3总线寄存器
0x030       :   背板高速总线寄存器
0x070~0x0FF :   网口地址寄存器
0x100       :   定时器寄存器0
0x104       :   定时器寄存器1
*/
#define FPGA_GEN_CONFIG_OFFSET                  0x8000
#define FPGA_VERSION_REG                        (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0)            //版本寄存器 BIT0-15 RO 程序版本号，BIT16-31 RO 规约版本号
#define FPGA_CTRL_REG                           (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x4)          //控制寄存器 详见《FPGA通讯规约》文档
#define FPGA_GPS_TIME_0_REG                     (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x10)         //外部对时寄存器0
#define FPGA_GPS_TIME_1_REG                     (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x14)         //外部对时寄存器1
#define FPGA_GPS_TIME_2_REG                     (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x18)         //外部对时寄存器2
#define FPGA_1PPS_x1_REG                        (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x20)         //1PPSx1寄存器
#define FPGA_1PPS_x2_REG                        (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x24)         //1PPSx2寄存器
#define FPGA_1PPS_x3_REG                        (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x28)         //1PPSx3寄存器
#define FPGA_BACKPLANE_BUS_STATUS_REG           (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x30)         //背板高速总线寄存器 详见《FPGA通讯规约》文档
#define FPGA_MAC_REPLACE_ENABLE_REG             (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x70)         //网口地址寄存器 BIT0-15 网口MAC地址替换使能(发送端口)
#define FPGA_ETH_STATUS_0_REG                   (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x74)         //网口状态监视寄存器0
#define FPGA_ETH_STATUS_1_REG                   (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x78)         //网口状态监视寄存器1
#define FPGA_ETH_MAC_ADD_REG                    (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x80)         //网口MAC地址寄存器 每个网口占8字节，共16个网口
#define FPGA_TIMER_PERIOD_REG                   (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x100)        //定时器寄存器(BIT00-20 设定定时器周期 单位: ns，取值范围595000~1390000 由该定时器可产生1PPSx3总线触发信号)
#define FPGA_TIMER_COUNTER_REG                  (FPGA_BASE + FPGA_GEN_CONFIG_OFFSET + 0x104)        //定时器计数值(BIT00-20 设定定时器周期 单位: ns，取值范围595000~1390000 由该定时器可产生1PPSx3总线触发信号)

#define FPGA_ETH_PORT_COUNT                     16      //定义16个网口

/* FPGA GENERAL CTRL REG Bit Definitions and Masks 用于配置 FPGA_CTRL_REG */
#define EXT_TIME_SOURCE_SHIFT                   0U
#define EXT_TIME_SOURCE_MASK                    0x00000007U //BIT0-2 RW 外部对时信号源 b010：外部光B码-正向，b011：外部光B码-反向，b100:外部电IRIG-B(正相)，b101:外部电IRIG-B(反相)
#define PPS_1X_OUT_SHIFT                        4U
#define PPS_1x_OUT_MASK                         0x00000010U //BIT4 RW 1PPSx1总线信号是否输出
#define PPS_2X_OUT_SHIFT                        5U
#define PPS_2x_OUT_MASK                         0x00000020U //BIT5 RW 1PPSx2总线信号是否输出
#define PPS_3X_OUT_SHIFT                        6U
#define PPS_3x_OUT_MASK                         0x00000040U //BIT6 RW 1PPSx3总线信号是否输出
#define SV_SYNC_TYPE_SHIFT                      8U
#define SV_SYNC_TYPE_MASK                       0x00000700U //BIT8-10 RW 采样值同步机制 b000:组网序号同步机制，b010:直连插值同步机制，b011:延时可测同步机制
#define SV_DELAY_SHIFT                          13U
#define SV_DELAY_MASK                           0x00FFE000U //BIT13-23 RW 采样值延时控制，单位1us (不大于2000us，用于IEC61850-9-2 SV报文发送延时控制)
#define BUS_ADR_SHIFT                           28U
#define BUS_ADR_MASK                            0xF0000000U //BIT28-21 RO 总线节点编号 整装置分配的唯一地址编号(0~15)
/* FPGA BACK SYS BUS STATUS REG Bit Definitions and Masks 用于配置 FPGA_BACKPLANE_BUS_STATUS_REG */
#define DATA0_CODE_ERR_SHIFT                    0U
#define DATA0_CODE_ERR_MASK                     0x00000001U //BIT0 RW 数据总线DATA0编码异常，CPU可通过往该位写1实现清0操作
#define DATA1_CODE_ERR_SHIFT                    1U
#define DATA1_CODE_ERR_MASK                     0x00000002U //BIT1 RW 数据总线DATA1编码异常，CPU可通过往该位写1实现清0操作
#define DATA2_CODE_ERR_SHIFT                    2U
#define DATA2_CODE_ERR_MASK                     0x00000004U //BIT2 RW 数据总线DATA2编码异常，CPU可通过往该位写1实现清0操作
#define DATA3_CODE_ERR_SHIFT                    3U
#define DATA3_CODE_ERR_MASK                     0x00000008U //BIT3 RW 数据总线DATA3编码异常，CPU可通过往该位写1实现清0操作
#define DATA0_LEN_OR_CRC_ERR_SHIFT              4U
#define DATA0_LEN_OR_CRC_ERR_MASK               0x00000010U //BIT4 RW 数据总线DATA0长度或校验异常，CPU可通过往该位写1实现清0操作
#define DATA1_LEN_OR_CRC_ERR_SHIFT              5U
#define DATA1_LEN_OR_CRC_ERR_MASK               0x00000020U //BIT5 RW 数据总线DATA1长度或校验异常，CPU可通过往该位写1实现清0操作
#define DATA2_LEN_OR_CRC_ERR_SHIFT              6U
#define DATA2_LEN_OR_CRC_ERR_MASK               0x00000040U //BIT6 RW 数据总线DATA2长度或校验异常，CPU可通过往该位写1实现清0操作
#define DATA3_LEN_OR_CRC_ERR_SHIFT              7U
#define DATA3_LEN_OR_CRC_ERR_MASK               0x00000080U //BIT7 RW 数据总线DATA3长度或校验异常，CPU可通过往该位写1实现清0操作
#define GET_BUS_CONTROL_FAIL_SHIFT              8U
#define GET_BUS_CONTROL_FAIL_MASK               0x00000100U //BIT8 RW 多次尝试未获取总线控制权（尝试16次仍总线仲裁失利，未获取总线控制权(总线竞争失败)，导致无法将报文发送出去，CPU可通过往该位写1实现清0操作）
#define TRANS_SEQ_ERR_SHIFT                     9U
#define TRANS_SEQ_ERR_MASK                      0x00000200U //BIT9 RW 报文传输时序错误（在报文的异步数据段检测到同步数据段信息，报文传输时序错误，CPU可通过往该位写1实现清0操作）
#define MSG_BIT_ERR_SHIFT                       10U
#define MSG_BIT_ERR_MASK                        0x00000400U //BIT10 RW 报文位错误（在报文异步数据段检测到位错误(输出显性电平收回隐性电平)，CPU可通过往该位写1实现清0操作）
#define BUF_OVERFLOW_ERR_SHIFT                  11U
#define BUF_OVERFLOW_ERR_MASK                   0x00000800U //BIT11 RW 缓存区溢出（该位置1说明数据缓存区溢出，CPU可通过往该位写1实现清0操作）

#define FPGA_TIMER_MASK                         0x001FFFFFU

/* 
   SV msg receive region SV报文接收
SV报文接收模块用于点对点或组网方式下接收合并单元上送的4000Hz采样率的IEC61850-9-2采样值报文， 
并根据配置对报文进行过滤和采样值同步处理。

SV报文接收模块地址分配(0x00~0xFF)
0x00        :   报文控制寄存器
0x04        :   报文过滤寄存器0
0x08        :   报文过滤寄存器1
0x0C        :   报文过滤寄存器2
0x10        :   报文过滤寄存器3
...
*/
#define FPGA_SV_RECEIVE_OFFSET                  0x1000
#define FPGA_SV_RECEIVE_CONFIG_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x00)     //报文控制寄存器
#define FPGA_SV_FILTER_CONFIG0_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x04)     //报文过滤寄存器0
#define FPGA_SV_FILTER_CONFIG1_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x08)     //报文过滤寄存器1
#define FPGA_SV_FILTER_CONFIG2_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x0C)     //报文过滤寄存器2
#define FPGA_SV_FILTER_CONFIG3_REG              (FPGA_BASE + FPGA_SV_RECEIVE_OFFSET + 0x10)     //报文过滤寄存器3
#define FPGA_SV_RECEIVE_REGION_LEN              0x14        //每个过滤模块配置占用0x14
#define FPGA_SV_RECEIVE_REGION_MAX_CNT          6           //SV过滤模块组数量，非物理光口（0~5）

/* SV msg receive config REG Bit Definitions and Masks 用来配置FPGA_SV_RECEIVE_CONFIG_REG */
#define SV_RECEIVE_PHASE_ANGLE_SHIFT            0U
#define SV_RECEIVE_PHASE_ANGLE_MASK             0x00001FFFU //BIT0-12 RW SV接收报文调相角(角度单位:分，负数采用二进制补码表示)
#define SV_RECEIVE_RATED_DELAY_VALID_SHIFT      21U
#define SV_RECEIVE_RATED_DELAY_VALID_MASK       0x00200000U //BIT21 RW SV接收报文额定延时是否有效 0:无效 1:有效 （默认第一通道为额定延时通道）
#define SV_RECEIVE_NET_PORT_SHIFT               27U
#define SV_RECEIVE_NET_PORT_MASK                0x78000000U //BIT27-30 RW SV接收网口(0~15) 物理光口 配置3~8
#define SV_RECEIVE_ENABLE_SHIFT                 31U
#define SV_RECEIVE_ENABLE_MASK                  0x80000000U //BIT31 RW SV接收使能
/* SV msg receive fitler0 REG Bit Definitions and Masks 用于配置 FPGA_SV_FILTER_CONFIG0_REG*/
#define SV_RECEIVE_DES_MAC_BYTE6_SHIFT          0U
#define SV_RECEIVE_DES_MAC_BYTE6_MASK           0x000000FFU //BIT0-7 RW SV接收报文目的MAC地址第6字节数据(Byte6)
#define SV_RECEIVE_DES_MAC_BYTE5_SHIFT          8U
#define SV_RECEIVE_DES_MAC_BYTE5_MASK           0x0000FF00U //BIT8-15 RW SV接收报文目的MAC地址第5字节数据(Byte5)
#define SV_RECEIVE_APPID_SHIFT                  16U
#define SV_RECEIVE_APPID_MASK                   0xFFFF0000U //BIT16-31 RW SV接收报文APPID
/* SV msg receive fitler1 REG Bit Definitions and Masks 用于配置 FPGA_SV_FILTER_CONFIG1_REG*/
#define SV_RECEIVE_SVID_CRC32_SHIFT             0U
#define SV_RECEIVE_SVID_CRC32_MASK              0xFFFFFFFFU //BIT0-31 RW SV接收报文svID字符串CRC-32生成校验码
/* SV msg receive fitler2 REG Bit Definitions and Masks 用于配置 FPGA_SV_FILTER_CONFIG2_REG*/
#define SV_RECEIVE_CONFREV_SHIFT                0U
#define SV_RECEIVE_CONFREV_MASK                 0xFFFFFFFFU //BIT0-31 RW SV接收报文confRev
/* SV msg receive fitler3 REG Bit Definitions and Masks 用于配置 FPGA_SV_FILTER_CONFIG3_REG*/
#define SV_RECEIVE_ASDU_CHAN_CNT_SHIFT          0U
#define SV_RECEIVE_ASDU_CHAN_CNT_MASK           0x0000003FU //BIT0-5 SV接收报文每个ASDU采样通道数目

/* 
   SV msg send region SV报文发送
SV报文发送模块用于将模拟量采样值、点对点SV报文接收采样值经过插值同步处理后组织为 
以太网IEC61850-9-2采样值报文(最多49个采样通道)，发送给其它装置。

SV报文发送模块地址分配(0x000~0x7FF)
0x000       :   发送控制寄存器
0x010       :   报文寻址寄存器
0x040~0x2FF :   通道配置寄存器
0x300~0x7FF :   发送报文缓存区
 
*/
#define FPGA_SV_SEND_OFFSET                     0x3000
#define FPGA_SV_SEND_CONFIG_REG                 (FPGA_BASE + FPGA_SV_SEND_OFFSET + 0x00)    //发送控制寄存器
#define FPGA_SV_SEND_ADDRESSING_REG             (FPGA_BASE + FPGA_SV_SEND_OFFSET + 0x10)    //报文寻址寄存器
#define FPGA_SV_SEND_CHAN_CONFIG_REG            (FPGA_BASE + FPGA_SV_SEND_OFFSET + 0x40)    //通道配置寄存器 (48个通道，每个通道占用3个DWORD寄存器)
#define FPGA_SV_SEND_BUF_REG                    (FPGA_BASE + FPGA_SV_SEND_OFFSET + 0x300)   //发送报文缓存区 (ADDR = 0x300~0x7FF)
#define FPGA_SV_SEND_CHAN_MAX_CNT               49      //最多支持49个通道 00-48
#define FPGA_SV_SEND_CHAN_CONFIG_REG_MAX_CNT    3       //每个通道占用3个REG DWORD
#define FPGA_SV_SEND_CHAN_CONFIG_REGS_LENS      0x0C    //每个通道占用字节数 3*4 = 12 = 0x0C

/* SV send config REG Bit Definitions and Masks 用于配置 FPGA_SV_SEND_CONFIG_REG*/
#define SV_SEND_MSG_LEN_SHIFT                   0U
#define SV_SEND_MSG_LEN_MASK                    0x000003FFU //BIT0-9 RW 报文长度(1~1023字节)
#define SV_SEND_CHAN_CNT_SHIFT                  10U
#define SV_SEND_CHAN_CNT_MASK                   0x0000FC00U //BIT10-15 RW 通道数目(1~49)
#define SV_SEND_FORCE_TEST_SHIFT                16U
#define SV_SEND_FORCE_TEST_MASK                 0x00010000U //BIT16 RW 强制测试 0:否 1是
#define SV_SEND_FORCE_SYN_SHIFT                 17U
#define SV_SEND_FORCE_SYN_MASK                  0x00020000U //BIT17 RW 强制同步 0:否 1是
#define SV_SEND_NET_PORT_SHIFT                  18U
#define SV_SEND_NET_PORT_MASK                   0x7FFC0000U //BIT18-30 RW 发送使能 每位对应一个外部网口，共0~12网口(目前可用3~8)
#define SV_SEND_CONFIG_UPDATE_SHIFT             31U
#define SV_SEND_CONFIG_UPDATE_MASK              0x80000000U //BIT31 RW 配置更新
/* 
    SV_SEND_CONFIG_UPDATE_MASK 配置更新 补充说明：
0:允许对该模块所有的配置寄存器和发送报文缓存进行写入操作，待所有配置写入结束后将该标志位置1， 
  告知报文发送控制器需要更新配置。报文发送控制器会在下一个采样值报文发送前将新的配置搬移到
  配置执行区并按照新的配置组织采样值报文，且将该标志位清0。
1:禁止对该模块所有的配置寄存器和发送报文缓存进行写入操作。
注:为满足PT切换功能，允许对配置寄存器和发送报文缓存进行实时写入修改，且保证所有配置项(含修改项) 
在下一个采样值报文同时生效，设计了两个配置区，分别为配置读写访问区和配置执行区，配置读写访问区允 
许进行读、写操作，而配置执行区是实际生效的配置区，不允许读、写操作。当配置更新标志置1后读写访问 
区配置数据搬移至配置执行区，实现配置数据更新。
*/ 


/* SV send msg addressing config REG Bit Definitions and Masks 用于配置FPGA_SV_SEND_ADDRESSING_REG */
#define SV_SEND_CHAN0_VALUE_POS_SHIFT           0U
#define SV_SEND_CHAN0_VALUE_POS_MASK            0x000003FFU //BIT0-9 RW smpSynch value首字节序号
#define SV_SEND_SMPSYNCH_VALUE_POS_SHIFT        10U
#define SV_SEND_SMPSYNCH_VALUE_POS_MASK         0x000FFC00U //BIT10-19 RW channel0 value首字节序号
#define SV_SEND_SMPCNT_VALUE_POS_SHIFT          20U
#define SV_SEND_SMPCNT_VALUE_POS_MASK           0x3FF00000U //BIT20-29 RW smpCnt value首字节序号
/* 
    上文的 
    SV_SEND_CHAN0_VALUE_POS_MASK
    SV_SEND_SMPSYNCH_VALUE_POS_MASK
    SV_SEND_SMPCNT_VALUE_POS_MASK
    在报文发送缓冲区 中都有对应的位置，详见说明文档
*/

/* SV send channel config REG Bit Definitions and Masks */
#define SV_SEND_SRC_FACTOR_SHIFT                0U
#define SV_SEND_SRC_FACTOR_MASK                 0x0000FFFFU //BIT0-15 RW 数据源系数(有符号数，如果”数据源缩放”标志为1，则系数需放大32倍)
#define SV_SEND_SRC_SCALE_SHIFT                 16U
#define SV_SEND_SRC_SCALE_MASK                  0x00010000U //BIT16 RW 数据源缩放 0:原值 1:放大
#define SV_SEND_SRC_CHAN_SHIFT                  18U
#define SV_SEND_SRC_CHAN_MASK                   0x00FC0000U //BIT18-23 RW 数据源通道 00-48
#define SV_SEND_SRC_CHOOSE_SHIFT                24U
#define SV_SEND_SRC_CHOOSE_MASK                 0x1F000000U //BIT24-28 RW 数据源选择 00-...
/* 
     SV_SEND_SRC_CHOOSE_MASK补充说明
     对于AD：可选0/1分别标识AD0和AD1
     对于SV：可选0-5，指定接收模块的序号(0 ~ FPGA_SV_RECEIVE_REGION_MAX_CNT-1)
*/

#define SV_SEND_SRC_TYPE_SHIFT                  29U
#define SV_SEND_SRC_TYPE_MASK                   0x60000000U //BIT29 RW 数据源类型 b00:AD b01:SV b11:额定延时
#define SV_SEND_SRC_ENABLE_SHIFT                31U
#define SV_SEND_SRC_ENABLE_MASK                 0x80000000U //BIT31 RW 数据源使能 0:禁止 1:使能


/* 
   Goose Receive region GOOSE报文接收
   GOOSE报文接收模块用于将外部以太网口接收到的GOOSE报文经过必要的报文解析和流量抑制处理后转发给CPU进行后续处理
GOOSE报文接收模块地址分配(0x000~0x3FF)
0x000       :   控制寄存器
0x004       :   功能寄存器
0x020       :   GOOSE接收报文抑制使能寄存器0
0x024       :   GOOSE接收报文抑制使能寄存器1
0x028       :   GOOSE接收报文抑制使能寄存器2
0x040       :   GOOSE接收报文抑制状态寄存器0
0x044       :   GOOSE接收报文抑制状态寄存器1
0x048       :   GOOSE接收报文抑制状态寄存器2
0x080~37F   :   GOOSE接收报文过滤寄存器

*/
#define FPGA_GOOSE_RECEIVE_OFFSET               0x4800
#define FPGA_GOOSE_CTRL_REG                     (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x00)  //控制寄存器
#define FPGA_GOOSE_FUN_REG                      (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x04)  //功能寄存器
#define FPGA_GOOSE_STORM_CONTROL_REG            (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x20)  //GOOSE接收报文抑制使能寄存器
#define FPGA_GOOSE_STORM_CONTROL_STATUS_REG     (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x40)  //GOOSE接收报文抑制状态寄存器
#define FPGA_GOOSE_FILTER_CRC_REG               (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x80)  //GOOSE接收报文过滤寄存器
#define FPGA_GOOSE_FILTER_CTRL_REG              (FPGA_BASE + FPGA_GOOSE_RECEIVE_OFFSET + 0x84)  //GOOSE接收报文过滤控制寄存器
#define FPGA_GOOSE_RECIEVE_FILETER_CONST        96                                              //GOOSE接收报文过滤器数量

/* 
   AXI HP communication control region (SV GOOSE) AXI_HP通信控制器
AXI_HP通信控制器模块用于控制ZYNQ芯片FPGA与CPU(ARM)之间的高速通信接口AXI_HP，AXI_HP端口由FPGA侧主动 
将报文在FPGA与CPU之间进行搬移，CPU需要预先分配一块连续内存空间给FPGA作为报文搬移缓存空间。FPGA管理 
CPU内存空间采用BD模式，收发缓存独立，各支持128个缓存空间(0~127)。每块缓存空间大小为2K字节，前4字节 
用于标识报文数据长度，后续空间用于存放报文内容。

AXI_HP通信控制器模块地址分配(0x00~0xFF)
0x20            : 发送报文对应连续内存块起始地址
0x40~0x4F   : 发送报文缓存描述表
0x60            : 接收报文对应连续内存块起始地址
0x80~0x8F   : 接收报文缓存描述表

*/
#define FPGA_SV_AXI_HP_CONTROL_OFFSET           0x4100
#define FGPA_SV_AXI_HP_TX_BUF_ADDR_REG          (FPGA_BASE + FPGA_SV_AXI_HP_CONTROL_OFFSET + 0x20)  //CPU->FPGA发送报文对应连续内存块起始地址
#define FGPA_SV_AXI_HP_TX_BD_REG                (FPGA_BASE + FPGA_SV_AXI_HP_CONTROL_OFFSET + 0x40)  //CPU->FPGA发送报文缓存描述表首地址
#define FGPA_SV_AXI_HP_RX_BUF_ADDR_REG          (FPGA_BASE + FPGA_SV_AXI_HP_CONTROL_OFFSET + 0x60)  //FPGA->CPU接收报文对应连续内存块起始地址
#define FGPA_SV_AXI_HP_RX_BD_REG                (FPGA_BASE + FPGA_SV_AXI_HP_CONTROL_OFFSET + 0x80)  //FPGA->CPU接收报文缓存描述表首地址

/* 
   Sampling msg receive region 采样值报文上送
   0x3C00 采样值报文上送0
   0x3E00 采样值报文上送1
 
采样值报文上送模块用于定期上送经过采样值同步处理后的采样值数据，并与其它状态信息按照格式 
要求组合成内部采样值报文，通过背板高速总线传送给总线上指定节点CPU处理。

采样值报文上送模块地址分配(0x000~0x1FF)
0x000       :   采样控制寄存器
0x040~0x13F :   间隔配置寄存器

*/
#define FPGA_SAMPING_MSG_INTERVAL_LEN           8
#define FPGA_SAMPING_MSG_INTERVAL_CNT           32          //间隔个数
#define FPGA_SAMPING_MSG0_RECEIVE_OFFSET        0x3C00      //采样值报文上送0
#define FPGA_SAMPING_MSG0_CONTROL_REG           (FPGA_BASE + FPGA_SAMPING_MSG0_RECEIVE_OFFSET + 0x00)   //采样控制寄存器
#define FPGA_SAMPING_MSG0_INTERVAL_CONFIG0_REG  (FPGA_BASE + FPGA_SAMPING_MSG0_RECEIVE_OFFSET + 0x40)   //间隔0配置寄存器0
#define FPGA_SAMPING_MSG0_INTERVAL_CONFIG1_REG  (FPGA_BASE + FPGA_SAMPING_MSG0_RECEIVE_OFFSET + 0x44)   //间隔0配置寄存器1
/*
    间隔配置寄存器 总共0~31 共32个间隔配置寄存器，每个间隔占用2个REG DWORD
*/ 

#define FPGA_SAMPING_MSG1_RECEIVE_OFFSET        0x3E00      //采样值报文上送1
#define FPGA_SAMPING_MSG1_CONTROL_REG           (FPGA_BASE + FPGA_SAMPING_MSG1_RECEIVE_OFFSET + 0x00)
#define FPGA_SAMPING_MSG1_INTERVAL_CONFIG0_REG  (FPGA_BASE + FPGA_SAMPING_MSG1_RECEIVE_OFFSET + 0x40)
#define FPGA_SAMPING_MSG1_INTERVAL_CONFIG1_REG  (FPGA_BASE + FPGA_SAMPING_MSG1_RECEIVE_OFFSET + 0x44)

/* FPGA sampling ctrl reg Definitions and Masks  用于配置采样控制寄存器 */
#define SAMPLING_MSG_BUS_DES_ADR_SHIFT          0U
#define SAMPLING_MSG_BUS_DES_ADR_MASK           0x0000FFFFU //BIT0-15 RW 总线目标节点映射 每位对应一个节点，共00-15节点
#define SAMPLING_MSG_INTERVAL_CNT_SHIFT         16U
#define SAMPLING_MSG_INTERVAL_CNT_MASK          0x003F0000U //BIT16-21 RW 上送间隔数目(1~32) 用于描述上送报文订阅的间隔数目，包含AD和SV间隔总数
#define SAMPLING_MSG_SAMPLE_RATE_SHIFT          22U
#define SAMPLING_MSG_SAMPLE_RATE_MASK           0x00C00000U //BIT22-23 RW 数据采样率 b00:24点 b01:48点 b10:80点
/* FPGA sampling interval config reg 0 Definitions and Masks  用于配置间隔配置寄存器0*/
#define SAMPLING_INTERVAL_SRC_CHOOSE_SHIFT      0U
#define SAMPLING_INTERVAL_SRC_CHOOSE_MASK       0x0000001FU //BIT0-4 数据源选择
/* 
   SAMPLING_INTERVAL_SRC_CHOOSE_MASK 同 SV_SEND_SRC_CHOOSE_MASK
     对于AD：可选0/1分别标识AD0和AD1
     对于SV：可选0-5，指定接收模块的序号(0 ~ FPGA_SV_RECEIVE_REGION_MAX_CNT-1)
*/

#define SAMPLING_INTERVAL_SRC_TYPE_SHIFT        8U
#define SAMPLING_INTERVAL_SRC_TYPE_MASK         0x00000300U //BIT8 RW 数据源类型 b00:AD b01:SV
#define SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_SHIFT   15U
#define SAMPLING_INTERVAL_SRC_CHAN_MAPPING2_MASK    0xFF800000U //BIT15-31 RW 数据源通道映射2(与数据源通道映射1联合使用)
/* FPGA sampling interval config reg 1 Definitions and Masks */
#define SAMPLING_INTERVAL_SRC_CHAN_MAPPING1_SHIFT   0U
#define SAMPLING_INTERVAL_SRC_CHAN_MAPPING1_MASK    0xFFFFFFFFU //BIT0-31 RW 数据源通道映射1(与数据源通道映射2联合使用)
/*
    数据源通道映射
    BIT[00]-通道00
    ...
    BIT[48]-通道48  --->  数据源通道映射2 的BIT[31]
*/ 


/*FPGA扩展CPLD */
#define FPGA_CPLD_OFFSET                  0x5000
#define FPGA_CPLD_BASE                     (FPGA_BASE + FPGA_CPLD_OFFSET)




#endif /* SRC_H_BAREMETAL_PLT_HD_FPGA_DEF_H_ */
