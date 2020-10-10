//////////////////////////////////////////////////
///Export_BuildInEnum.h_
///version=0.10
///2020-04-07 16:17:22
//////////////////////////////////////////////////

#ifndef EXPORT_BUILDIN_ENUM_H
#define EXPORT_BUILDIN_ENUM_H

//预定义
enum{
    SETVAL_NONE                                      = 0    ,   //无
    SETVAL_CHINESE                                   = 0    ,   //中文
    SETVAL_ENGLISH                                   = 1    ,   //英文
    SETVAL_RUSSIAN                                   = 2    ,   //俄文
    SETVAL_PARITY_NO                                 = 0    ,   //无校验
    SETVAL_PARITY_ODD                                = 1    ,   //奇校验
    SETVAL_PARITY_EVEN                               = 2    ,   //偶校验
    SETVAL_ON                                        = 1    ,   //投入
    SETVAL_OFF                                       = 0    ,   //退出
    SETVAL_CT_1A                                     = 1    ,   //1A
    SETVAL_CT_5A                                     = 5    ,   //5A
    SETVAL_PROTOCOL_NONE                             = 0    ,   //无通讯
    SETVAL_PROTOCOL_IEC61850                         = 1    ,   //61850
    SETVAL_PROTOCOL_TCP103                           = 2    ,   //TCP103
    SETVAL_PROTOCOL_IEC103                           = 3    ,   //标准103
    SETVAL_PROTOCOL_WISCOM103                        = 4    ,   //金智103
    SETVAL_PROTOCOL_MODBUS                           = 5    ,   //MODBUS
    SETVAL_PROTOCOL_DEBUG                            = 6    ,   //DEBUG
    SETVAL_PROTOCOL_DBJ                              = 7    ,   //DBJ
    SETVAL_PROTOCOL_MONITER                          = 10   ,   //监视规约
    SETVAL_PROTOCOL_PRINTER                          = 20   ,   //打印
    SETVAL_PROTOCOL_TEST                             = 30   ,   //测试规约
    SETVAL_PROTOCOL_SERIAL_LCD                       = 31   ,   //串口液晶规约
    SETVAL_GPS_NONE                                  = 0    ,   //无校时
    SETVAL_GPS_IRIGB                                 = 2    ,   //IRIB-B
    SETVAL_GPS_SPULSE                                = 3    ,   //秒脉冲
    SETVAL_GPS_MPULSE                                = 4    ,   //分脉冲
    SETVAL_GPS_SNTP                                  = 1    ,   //SNTP
    SETVAL_SERIAL_SINGLE                             = 1    ,   //单串口
    SETVAL_SERIAL_DUAL                               = 2    ,   //双串口
    SETVAL_SOESEND_SINGLE                            = 0    ,   //双网单发
    SETVAL_SOESEND_BOTH                              = 1    ,   //双网双发
    SETVAL_DISABLE                                   = 0    ,   //禁止
    SETVAL_DEFAULT                                   = 1    ,   //默认
    SETVAL_ENCRYPTION                                = 2    ,   //加密
};

//工程配置
enum{
    EVALUE_CUSTOMER_OTHER                            = 0,      //其他
    EVALUE_CUSTOMER_GW                               = 1,      //国网
    EVALUE_CUSTOMER_NW                               = 2,      //南网
    EVALUE_CUSTOMER_PWRPLT                           = 3,      //电厂
    EVALUE_CUSTOMER_ENT                              = 4,      //工企
    EVALUE_SERIES_IPACS_5700                         = 1,      //iPACS-5700保护
    EVALUE_SERIES_IPACS_5770                         = 2,      //iPACS-5770测控
    EVALUE_SERIES_IPACS_5900                         = 3,      //iPACS-5900高压
    EVALUE_SERIES_MFC                                = 4,      //MFC
    EVALUE_SERIES_WDZ                                = 5,      //WDZ
    EVALUE_CPU_MONO                                  = 0,      //单CPU架构
    EVALUE_CPU_DUAL_QD                               = 1,      //双CPU架构-启动CPU
    EVALUE_CPU_DUAL_BH                               = 2,      //双CPU架构-保护CPU
    EVALUE_ADSRC_AD                                  = 0,      //AD
    EVALUE_ADSRC_SV                                  = 1,      //SV
    EVALUE_ADSRC_AD_AND_SV                           = 2,      //AD&SV
    EVALUE_OFF                                       = 0,      //不支持
    EVALUE_ON                                        = 1,      //支持
    EVALUE_SAMPLE_24                                 = 24,     //24点
    EVALUE_SAMPLE_48                                 = 48,     //48点
    EVALUE_SAMPLE_80                                 = 80,     //80点
    EVALUE_AD7606                                    = 1,      //AD7606
    EVALUE_AD7616                                    = 2,      //AD7616
    EVALUE_NOT_CUSTOM                                = 0,      //否
    EVALUE_IS_CUSTOM                                 = 1,      //是
};

// 61850量纲
enum{
    SIUNIT_NONE                                      = 0,          //None
    SIUNIT_EMPTY                                     = 1,          //
    SIUNIT_m                                         = 2,          //m
    SIUNIT_kg                                        = 3,          //kg
    SIUNIT_s                                         = 4,          //s
    SIUNIT_A                                         = 5,          //A
    SIUNIT_K                                         = 6,          //K
    SIUNIT_mol                                       = 7,          //mol
    SIUNIT_cd                                        = 8,          //cd
    SIUNIT_deg                                       = 9,          //deg
    SIUNIT_rad                                       = 10,         //rad
    SIUNIT_sr                                        = 11,         //sr
    SIUNIT_Gy                                        = 21,         //Gy
    SIUNIT_q                                         = 22,         //q
    SIUNIT_OC                                        = 23,         //℃
    SIUNIT_Sv                                        = 24,         //Sv
    SIUNIT_F                                         = 25,         //F
    SIUNIT_C                                         = 26,         //C
    SIUNIT_S                                         = 27,         //S
    SIUNIT_H                                         = 28,         //H
    SIUNIT_V                                         = 29,         //V
    SIUNIT_ohm                                       = 30,         //ohm
    SIUNIT_J                                         = 31,         //J
    SIUNIT_N                                         = 32,         //N
    SIUNIT_Hz                                        = 33,         //Hz
    SIUNIT_lx                                        = 34,         //lx
    SIUNIT_Lm                                        = 35,         //Lm
    SIUNIT_Wb                                        = 36,         //Wb
    SIUNIT_T                                         = 37,         //T
    SIUNIT_W                                         = 38,         //W
    SIUNIT_Pa                                        = 39,         //Pa
    SIUNIT_m2                                        = 41,         //m2
    SIUNIT_m3                                        = 42,         //m3
    SIUNIT_m_s                                       = 43,         //m/s
    SIUNIT_m_s2                                      = 44,         //m/s2
    SIUNIT_m3_s                                      = 45,         //m3/s
    SIUNIT_m_m3                                      = 46,         //m/m3
    SIUNIT_M                                         = 47,         //M
    SIUNIT_kg_m3                                     = 48,         //kg/m3
    SIUNIT_m2_s                                      = 49,         //m2/s
    SIUNIT_W_mK                                      = 50,         //W/m K
    SIUNIT_J_K                                       = 51,         //J/K
    SIUNIT_ppm                                       = 52,         //ppm
    SIUNIT_1_s                                       = 53,         //1/s
    SIUNIT_rad_s                                     = 54,         //rad/s
    SIUNIT_W_m2                                      = 55,         //W/m2
    SIUNIT_J_m2                                      = 56,         //J/m2
    SIUNIT_S_m                                       = 57,         //S/m
    SIUNIT_K_s                                       = 58,         //K/s
    SIUNIT_Pa_s                                      = 59,         //Pa/s
    SIUNIT_J_KgK                                     = 60,         //J/kg K
    SIUNIT_VA                                        = 61,         //VA
    SIUNIT_Watts                                     = 62,         //Watts
    SIUNIT_VAr                                       = 63,         //VAr
    SIUNIT_phi                                       = 64,         //phi
    SIUNIT_cos_phi                                   = 65,         //cos_phi
    SIUNIT_Vs                                        = 66,         //Vs
    SIUNIT_V2                                        = 67,         //V2
    SIUNIT_As                                        = 68,         //As
    SIUNIT_A2                                        = 69,         //A2
    SIUNIT_A2t                                       = 70,         //A2t
    SIUNIT_VAh                                       = 71,         //VAh
    SIUNIT_Wh                                        = 72,         //Wh
    SIUNIT_VArh                                      = 73,         //VArh
    SIUNIT_V_Hz                                      = 74,         //V/Hz
    SIUNIT_Hz_s                                      = 75,         //Hz/s
    SIUNIT_char                                      = 76,         //char
    SIUNIT_char_s                                    = 77,         //char/s
    SIUNIT_kgm2                                      = 78,         //kgm2
    SIUNIT_dB                                        = 79,         //dB
    SIUNIT_J_Wh                                      = 80,         //J_Wh
    SIUNIT_W_s                                       = 81,         //W/s
    SIUNIT_l_s                                       = 82,         //l/s
    SIUNIT_dBm                                       = 83          //dBm
};

// 61850量纲倍数
enum{
    SIMULTIPLE_none                                  = 0,          //无
    SIMULTIPLE_yocto                                 = -24,        //yocto
    SIMULTIPLE_zepto                                 = -21,        //zepto
    SIMULTIPLE_atto                                  = -18,        //阿
    SIMULTIPLE_femto                                 = -15,        //飞
    SIMULTIPLE_piko                                  = -12,        //皮
    SIMULTIPLE_nano                                  = -9,         //纳
    SIMULTIPLE_micro                                 = -6,         //微
    SIMULTIPLE_milli                                 = -3,         //毫
    SIMULTIPLE_centi                                 = -2,         //厘
    SIMULTIPLE_deci                                  = -1,         //分
    SIMULTIPLE_deka                                  = 1,          //十
    SIMULTIPLE_hekto                                 = 2,          //百
    SIMULTIPLE_kilo                                  = 3,          //千
    SIMULTIPLE_mega                                  = 6,          //兆
    SIMULTIPLE_giga                                  = 9,          //吉
    SIMULTIPLE_tera                                  = 12,         //太
    SIMULTIPLE_petra                                 = 15,         //拍
    SIMULTIPLE_exa                                   = 18,         //艾
    SIMULTIPLE_zetta                                 = 21,         //zetta
    SIMULTIPLE_yotta                                 = 24          //yotta
};

// 运算符定义
enum{
    DATA_OPERATOR_NONE                               = 0,          //无运算符
    DATA_OPERATOR_ADD                                = 1,          //加 +
    DATA_OPERATOR_SUBTRACE                           = 2,          //减 -
    DATA_OPERATOR_MULTIPLY                           = 3,          //乘 *
    DATA_OPERATOR_DIVIDE                             = 4,          //除 /
    DATA_OPERATOR_EQUAL                              = 10,         //等于 ==
    DATA_OPERATOR_UNEQUAL                            = 11,         //不等于 !=
    DATA_OPERATOR_GREATER                            = 12,         //大于 >
    DATA_OPERATOR_GREATER_OR_EQUAL                   = 13,         //大于等于 >=
    DATA_OPERATOR_LESS                               = 14,         //小于 <
    DATA_OPERATOR_LESS_OR_EQUAL                      = 15,         //小于等于 <=
    DATA_OPERATOR_LOGIC_AND                          = 20,         //逻辑与 &&
    DATA_OPERATOR_LOGIC_OR                           = 21,         //逻辑或 ||
    DATA_OPERATOR_LOGIC_NOT                          = 22,         //逻辑非 !
    DATA_OPERATOR_BIT_AND                            = 30,         //按位与 &
    DATA_OPERATOR_BIT_OR                             = 31,         //按位或 |
    DATA_OPERATOR_BIT_NOT                            = 32,         //按位非 ~
    DATA_OPERATOR_BIT_XOR                            = 33          //按位异或 ^
};

// 内核类型
enum{
    CORE_TYPE_BM                                     = 0,          //裸核
    CORE_TYPE_LINUX                                  = 1           //Linux核
};

// 测量类型
enum{
    MEAS_TYPE_DEFAULT                                = 0,          //默认
    MEAS_TYPE_PROT_U                                 = 1,          //保护相电压
    MEAS_TYPE_PROT_UL                                = 2,          //保护线电压
    MEAS_TYPE_PROT_I                                 = 3,          //保护电流
    MEAS_TYPE_PROT_P                                 = 4,          //保护有功
    MEAS_TYPE_PROT_Q                                 = 5,          //保护无功
    MEAS_TYPE_MEAS_U                                 = 16,         //测量相电压
    MEAS_TYPE_MEAS_UL                                = 32,         //测量线电压
    MEAS_TYPE_MEAS_I                                 = 48,         //测量电流
    MEAS_TYPE_MEAS_P                                 = 64,         //测量有功
    MEAS_TYPE_MEAS_Q                                 = 80,         //测量无功
    MEAS_TYPE_MEAS_ENERGY                            = 96,         //测量电能
    MEAS_TYPE_PRIMARY_U                              = 256,        //一次值相电压
    MEAS_TYPE_PRIMARY_UL                             = 512,        //一次值线电压
    MEAS_TYPE_PRIMARY_I                              = 768,        //一次值电流
    MEAS_TYPE_PRIMARY_P                              = 1024,       //一次值有功
    MEAS_TYPE_PRIMARY_Q                              = 1280,       //一次值无功
    MEAS_TYPE_PRIMARY_ENERGY                         = 1536,       //一次值电能
    MEAS_TYPE_FACT                                   = 65536,      //功率因素
    MEAS_TYPE_ANGLE                                  = 131072,     //相角
    MEAS_TYPE_FREQ                                   = 196608,     //频率
    MEAS_TYPE_TIME_S                                 = 262144,     //秒时间
    MEAS_TYPE_FAULT_PHASE                            = 327680,     //故障相别
    MEAS_TYPE_FAULT_FAN                              = 393216,     //故障FAN
    MEAS_TYPE_FAULT_NOF                              = 458752,     //故障NOF
    MEAS_TYPE_SET_GROUP                              = 524288,     //定值区号
    MEAS_TYPE_DIR                                    = 589824,     //方向(1:正向/2:反向)
    MEAS_TYPE_GEAR                                   = 655360,     //档位
    MEAS_TYPE_DEV_TEMPERATURE                        = 1048576,    //装置温度
    MEAS_TYPE_DEV_INNER_VOLT                         = 2097152,    //装置内部电压
    MEAS_TYPE_DEV_HUMIDITY                           = 3145728,    //湿度
    MEAS_TYPE_DEV_OPTICAL_PWRX_UW                    = 4194304,    //光功率值-收 uW
    MEAS_TYPE_DEV_OPTICAL_PWTX_UW                    = 5242880,    //光功率值-发 uW
    MEAS_TYPE_DEV_OPTICAL_PWRX_DBM                   = 6291456,    //光功率值-收 dbm
    MEAS_TYPE_DEV_OPTICAL_PWTX_DMB                   = 7340032,    //光功率值-发 dbm
    MEAS_TYPE_BOOL                                   = 16777216,   //BOOL型
    MEAS_TYPE_BOOL_DUAL                              = 33554432,   //双点BOOL
    MEAS_TYPE_HEX                                    = 50331648,   //16进制
    MEAS_TYPE_BIT                                    = 67108864    //位串
};

// 定值特殊类型
enum{
    VALUETYPE_SPEC_DEFAULT                           = 0,          //默认
    VALUETYPE_SPEC_CTRLWORD                          = 1,          //控制字
    VALUETYPE_SPEC_MATRIX                            = 2,          //跳闸矩阵
    VALUETYPE_SPEC_IP                                = 3,          //IP地址
    VALUETYPE_SPEC_MASK                              = 4,          //掩码
    VALUETYPE_SPEC_HEX                               = 6,          //16进制
    VALUTTYPE_SPEC_CLOMPETE_NUM                      = 9,          //补全数值
    VALUETYPE_SPEC_IP_WHITE                          = 10          //IP白名单
};

// 操作类型
enum{
    OPERATE_TYPE_NONE                                = 0,          //无操作
    OPERATE_TYPE_SIGNAL_RESET                        = 1,          //信号复归
    OPERATE_TYPE_GOOSE_TRIP                          = 2,          //站控层Goose跳闸
    OPERATE_TYPE_SET_GROUE                           = 3,          //定值切区
    OPERATE_TYPE_PROT_SETTING                        = 4,          //保护定值修改
    OPERATE_TYPE_SYS_SETTING                         = 5,          //系统参数修改
    OPERATE_TYPE_COMM_SETTING                        = 6,          //通讯参数修改
    OPERATE_TYPE_SOFTSTRAP                           = 7,          //软压板切换
    OPERATE_TYPE_INNER_SETTING                       = 8,          //内部参数修改
    OPERATE_TYPE_PRECISION_AUTO                      = 20,         //精度系数自动修改
    OPERATE_TYPE_PRECISION_MANUAL                    = 21,         //精度系数手动修改
    OPERATE_TYPE_MODIFY_PASSWORD                     = 22,         //用户密码修改
    OPERATE_TYPE_MODIFY_USERNAME                     = 23,         //用户名修改
    OPERATE_TYPE_MODIFY_TIME                         = 24,         //修改时钟
    OPERATE_TYPE_POWERENERGY_SET                     = 25,         //电度修改
    OPERATE_TYPE_POWERENERGY_CLR                     = 26,         //电度清除
    OPERATE_TYPE_MODIFY_STR_SET                      = 30,         //修改字符串定值
    OPERATE_TYPE_RESET_FACTORY                       = 60,         //恢复出厂
    OPERATE_TYPE_UPDATE_CID                          = 61,         //CID更新(预留)
    OPERATE_TYPE_UPDATE_CCD                          = 62,         //CCD更新(预留)
    OPERATE_TYPE_UPDATE_QDCPU_CODE                   = 63,         //启动程序升级(预留)
    OPERATE_TYPE_UPDATE_BHCPU_CODE                   = 64,         //保护程序升级(预留)
    OPERATE_TYPE_UPDATE_LINUX_CODE                   = 65,         //Linux程序升级(预留)
    OPERATE_TYPE_UPDATE_FPGA_CODE                    = 66,         //FPGA程序升级(预留)
    OPERATE_TYPE_USER_LOGIN                          = 80,         //用户登陆成功
    OPERATE_TYPE_USER_LOGOUT                         = 81,         //用户退出
    OPERATE_TYPE_USER_LOGFAIL                        = 82,         //用户登陆失败
    OPERATE_TYPE_USER_LOG_LOCKED                     = 83,         //用户登陆锁定
    OPERATE_TYPE_REMOTE_CLOSE_SEL                    = 100,        //遥控合闸选择
    OPERATE_TYPE_REMOTE_CLOSE_EXE                    = 101,        //遥控合闸执行
    OPERATE_TYPE_REMOTE_CLOSE_CANCEL                 = 102,        //遥控合闸取消
    OPERATE_TYPE_REMOTE_OPEN_SEL                     = 103,        //遥控分闸选择
    OPERATE_TYPE_REMOTE_OPEN_EXE                     = 104,        //遥控分闸执行
    OPERATE_TYPE_REMOTE_OPEN_CANCEL                  = 105,        //遥控分闸取消
    OPERATE_TYPE_REMOTE_FAIL                         = 106,        //遥控失败
    OPERATE_TYPE_EARTHTRY_SEL                        = 107,        //接地试跳选择
    OPERATE_TYPE_EARTHTRY_EXE                        = 108,        //接地试跳执行
    OPERATE_TYPE_EARTHTRY_CANCEL                     = 109,        //接地试跳取消
    OPERATE_TYPE_DIRECT_CONTROL                      = 110,        //直控操作
    OPERATE_TYPE_REMOTE_UP_SEL                       = 120,        //控升选择
    OPERATE_TYPE_REMOTE_UP_EXE                       = 121,        //控升执行
    OPERATE_TYPE_REMOTE_UP_CANCEL                    = 122,        //控升取消
    OPERATE_TYPE_REMOTE_DOWN_SEL                     = 123,        //控降选择
    OPERATE_TYPE_REMOTE_DOWN_EXE                     = 124,        //控降执行
    OPERATE_TYPE_REMOTE_DOWN_CANCEL                  = 125,        //控降取消
    OPERATE_TYPE_REMOTE_STOP_SEL                     = 126,        //控停选择
    OPERATE_TYPE_REMOTE_STOP_EXE                     = 127,        //控停执行
    OPERATE_TYPE_REMOTE_STOP_CANCEL                  = 128,        //控停取消
    OPERATE_TYPE_REPORT_CLR                          = 200,        //报告清除
    OPERATE_TYPE_CLR_SRAM_JOURNAL                    = 201,        //清除61850日志
    OPERATE_TYPE_ACTIVE_BACKUP_SETGRP                = 202,        //激活备用定值区
    OPERATE_TYPE_DBG_RSIGNAL                         = 220,        //调试-遥信对点
    OPERATE_TYPE_DBG_DO_CTRL                         = 221,        //调试-出口传动
    OPERATE_TYPE_DBG_RMEASURE                        = 222,        //调试-遥测对点
    OPERATE_TYPE_DBG_MAN_RECORD                      = 223         //调试-手动启动录波
};

// 测值转换关系
enum{
    MEAS_RELATION_EQU                                = 0,          //等于
    MEAS_RELATION_SQRT                               = 1,          //开方
    MEAS_RELATION_SQUARE                             = 2,          //平方
    MEAS_RELATION_ABS                                = 3           //绝对值
};

// 软压板修改提示类型
enum{
    MODIFY_PROMPT_NONE                               = 0,          //不提示
    MODIFY_PROMPT_ON                                 = 1,          //置1提示
    MODIFY_PROMPT_OFF                                = 2,          //置0提示
    MODIFY_PROMPT_MODIFY                             = 3           //修改提示
};

// 遥控对象类型
enum{
    CONTROL_TYPE_RCONTROL                            = 1,          //遥控
    CONTROL_TYPE_RREGULATION                         = 2,          //遥调
    CONTROL_TYPE_SYNCLOSE                            = 3,          //同期合闸
    CONTROL_TYPE_DIRECT_CONTROL                      = 4,          //直控
    CONTROL_TYPE_TAP_ADJUST                          = 5,          //分接头调整
    CONTROL_TYPE_SYNCLOSE_MANUAL                     = 6,          //手合同期
    CONTROL_TYPE_EARTH_TRIP                          = 10          //接地试跳
};

// 控点类型（保护/测控）
enum{
    PORM_TYPE_MEAS                                   = 0,          //测控
    PORM_TYPE_RPOT                                   = 1           //保护
};

// 调试操作命令类型
enum{
    DEBUG_CMD_TYPE_LINUX                             = 0,          //Linux命令
    DEBUG_CMD_TYPE_BM                                = 1,          //BM命令
    DEBUG_CMD_TYPE_LINUX_BM                          = 2,          //Linux和BM命令，Linux执行完后命令同时转发BM
    DEBUG_CMD_TYPE_LINUX_INOVERHAUL                  = 10,         //检修态Linux命令
    DEBUG_CMD_TYPE_BM_INOVERHAUL                     = 11,         //检修态BM命令
    DEBUG_CMD_TYPE_LINUX_BM_INOVERHAUL               = 12,         //检修态Linux和BM命令，Linux执行完后命令同时转发BM
    DEBUG_CMD_TYPE_LINUX_INTEST                      = 50,         //整机调试Linux命令
    DEBUG_CMD_TYPE_BM_INTEST                         = 51,         //整机调试BM命令
    DEBUG_CMD_TYPE_LINUX_BM_INTEST                   = 52          //整机调试Linux和BM命令，Linux执行完后命令同时转发BM
};

// 模拟量通道属性
enum{
    ANALOGCHNL_ATTRIB_OTHER                          = 0,          //其他
    ANALOGCHNL_ATTRIB_U                              = 1,          //电压
    ANALOGCHNL_ATTRIB_I                              = 2,          //电流
    ANALOGCHNL_ATTRIB_3I0                            = 3,          //3I0 零序电流
    ANALOGCHNL_ATTRIB_3U0                            = 4,          //3U0 零序电压
    ANALOGCHNL_ATTRIB_dU                             = 5,          //差压
    ANALOGCHNL_ATTRIB_dI                             = 6,          //差流
    ANALOGCHNL_ATTRIB_RTHERM                         = 7,          //温度电阻
    ANALOGCHNL_ATTRIB_mA                             = 8,          //4~20mA 直流
    ANALOGCHNL_ATTRIB_5V                             = 9,          //0~5V 直流
    ANALOGCHNL_ATTRIB_DC_I                           = 10,         //直流通道-电流
    ANALOGCHNL_ATTRIB_DC_U                           = 11          //直流通道-电压
};

// 模拟量通道相位
enum{
    ANALOGCHNL_PHASE_NONE                            = 0,          //无相位
    ANALOGCHNL_PHASE_A                               = 1,          //A相
    ANALOGCHNL_PHASE_B                               = 2,          //B相
    ANALOGCHNL_PHASE_C                               = 3,          //C相
    ANALOGCHNL_PHASE_AB                              = 4,          //AB线
    ANALOGCHNL_PHASE_BC                              = 5,          //BC线
    ANALOGCHNL_PHASE_CA                              = 6,          //CA线
    ANALOGCHNL_PHASE_POS                             = 10,         //正序
    ANALOGCHNL_PHASE_NEG                             = 11,         //负序
    ANALOGCHNL_PHASE_ZERO                            = 12          //零序
};

// 模拟量通道对象
enum{
    ANALOGCHNL_OBJ_THISSIDE                          = 0,          //本侧
    ANALOGCHNL_OBJ_OFFSIDE                           = 1,          //对侧
    ANALOGCHNL_OBJ_HIDH                              = 10,         //高压侧
    ANALOGCHNL_OBJ_MIDD                              = 11,         //中压侧
    ANALOGCHNL_OBJ_LOW                               = 12,         //低压侧
    ANALOGCHNL_OBJ_SIDE_1                            = 21,         //一侧
    ANALOGCHNL_OBJ_SIDE_2                            = 22,         //二侧
    ANALOGCHNL_OBJ_SIDE_3                            = 23,         //三侧
    ANALOGCHNL_OBJ_SIDE_4                            = 24          //四侧
};

// 模拟量通道类型
enum{
    ANALOGCHNL_TYPE_AD                               = 0,          //AD通道
    ANALOGCHNL_TYPE_SV                               = 1,          //SV通道
    ANALOGCHNL_TYPE_VR                               = 2           //虚通道
};

// 模拟量通道每周波采样点数
enum{
    ANALOGCHNL_SAMPLE_24                             = 24,         //24点
    ANALOGCHNL_SAMPLE_48                             = 48,         //48点
    ANALOGCHNL_SAMPLE_80                             = 80          //80点
};

// 精度系数类型
enum{
    PRECISION_TYPE_GAIN                              = 0,          //通道增益
    PRECISION_TYPE_OFFSET                            = 1,          //通道偏置
    PRECISION_TYPE_ANGLE                             = 2           //角度
};

// 扰动类型
enum{
    DISTURB_TYPE_ANALOG                              = 0,          //模拟量通道
    DISTURB_TYPE_DIGITAL                             = 1           //数字量通道
};

// 重要测点值类型
enum{
    IMPT_VALUE_TYPE_FALSE                            = 0,          //总是FALSE
    IMPT_VALUE_TYPE_TRUE                             = 1,          //总是TRUE
    IMPT_VALUE_TYPE_BYPOINT                          = 10,         //根据测点的值
    IMPT_VALUE_TYPE_BYPOINT_REVERSE                  = 11          //测点的值取反
};

// 发送方式
enum{
    SEND_DATA_MODE_NONE                              = 0,          //无，不需要发送
    SEND_DATA_MODE_ON_CHG                            = 1,          //变化发送
    SEND_DATA_MODE_ON_TIME                           = 2,          //定时发送(提供时间定值，单位秒，浮点)
    SEND_DATA_MODE_ON_POWERON                        = 3           //上电时发送
};

// 网口配置
enum{
    ETHERNET_PORT_MMS                                = 0,          //MMS网口
    ETHERNET_PORT_GSE_SV                             = 1,          //Goose/SV
    ETHERNET_PORT_GSE                                = 2,          //Goose网口
    ETHERNET_PORT_SV                                 = 3,          //SV网口
    ETHERNET_PORT_INNER                              = 9,          //内部通讯网口
    ETHERNET_PORT_DEBUG                              = 10          //调试网口
};

// 网络接口类型
enum{
    ETHERNET_TYPE_ELECTRICAL                         = 0,          //电口
    ETHERNET_TYPE_OPTICAL                            = 1           //光口
};

// 联锁类型
enum{
    INTERLOCK_TYPE_NONE                              = 0,          //无
    INTERLOCK_TYPE_OPEN                              = 1,          //分
    INTERLOCK_TYPE_CLOSE                             = 2           //合
};

// 开关量IO类型
enum{
    DIGITAL_IO_TYPE_P2P                              = 0,          //点对点
    DIGITAL_IO_TYPE_BUS0                             = 1,          //总线型-0地址
    DIGITAL_IO_TYPE_BUS1                             = 2,          //总线型-1地址
    DIGITAL_IO_TYPE_BUS2                             = 3,          //总线型-2地址
    DIGITAL_IO_TYPE_BUS3                             = 4,          //总线型-3地址
    DIGITAL_IO_TYPE_BUS4                             = 5,          //总线型-4地址
    DIGITAL_IO_TYPE_BUS5                             = 6,          //总线型-5地址
    DIGITAL_IO_TYPE_BUS6                             = 7,          //总线型-6地址
    DIGITAL_IO_TYPE_BUS7                             = 8           //总线型-7地址
};

#endif
