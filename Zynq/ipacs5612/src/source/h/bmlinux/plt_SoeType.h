/*
 * @Author: zhongwei
 * @Date: 2019-11-19 09:22:58
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-19 13:55:56
 * @Description: SOE类型数据结构定义
 * @FilePath: \ZynqBM\src\include\plt_SoeType.h 
 *  
 * PU侧，一般只提供如下几个报告： 
 *    SOE_ACTION
 *    SOE_RSIGNAL
 *    SOE_PU_TRACE
 *    SOE_MEASURE_CHG
 *  
 *  
 * 在MU侧，会自动根据SOE_RSIGNAL中测点类型，组织出 
 *    SOE_RUNALARM
 *    SOE_DO
 *    SOE_RSIGNAL_PROT
 *    SOE_RSIGNAL_CTRL
 *    SOE_RUNALARM_PROT
 *    SOE_RUNALARM_CTRL
 *    SOE_HARDSTRAP
 * 根据SOE_ACTION，自动组织出SOE_ACTION_HV报告 
 * 操作报告 SOE_OPERATE、SOE_OPERATE_PROT、SOE_OPERATE_CTRL、SOE_OPERATE_INNER 则由MU生成
 */
#ifndef SRC_INCLUDE_PLT_SOETYPE_H_
#define SRC_INCLUDE_PLT_SOETYPE_H_

#ifndef UUID_STRUCT
#define UUID_STRUCT
typedef char uuid_c[36 + 1];            //uuid字符串 如：c8d94d02-d4dd-4227-b944-294db10762fa,长度一般为36字符
typedef char suid_c[22 + 1];            //uuid经过base64计算后的字符串，如：4/Yoi44nQ9awTvZsrdCBqQ，长度一般为22字符
#endif

//定义的SOE类型
//SOE类型
enum E_SOE_TYPE{
    //基本类型
    SOE_ACTION,             //动作报告
    SOE_RSIGNAL,            //遥信报告
    SOE_OPERATE,            //操作报告
    SOE_RUNALARM,           //运行告警报告
    SOE_WAVE,               //录波报告
    SOE_DO,                 //开出变位报告
    SOE_PU_TRACE,           //保护单元TRACE
    SOE_MU_TRACE,           //管理单元TRACE
    SOE_RSIGNAL_PROT,       //保护遥信
    SOE_RSIGNAL_CTRL,       //测控遥信
    SOE_OPERATE_PROT,       //保护操作
    SOE_OPERATE_CTRL,       //测控操作
    SOE_RUNALARM_PROT,      //保护告警
    SOE_RUNALARM_CTRL,      //测控告警
    SOE_OPERATE_INNER,      //内部操作报告（不可清除）
    SOE_VALUE_CHG,          //值变报告
    SOE_CONTROL,            //控制报告
    SOE_AUDIT,              //审计报告
    SOE_HARDSTRAP,          //硬压板变位报告
    SOE_ACTION_HV,          //高压保护动作报告

#ifdef _PROT_UNIT_
    SOE_MEASURE_CHG,        //遥测变化上送（仅保护单元程序有该SOE）
#endif 

    SOE_TYPE_COUNT,         //SOE类型总数目
};


/**
 *  TSOE_TRACE_STRUCT 定义
**/
//定义文件名的最大长度
#define MAX_TRACE_FILENAME_LEN      32
//定义TRACE描述信息的最大长度
#define MAX_TRACE_DESC_LEN          128

//TRACE信息的类型定义
enum E_TRACE_TYPE{
    TRACE_TYPE_MSG,                                 //TRACE生成的信息
    TRACE_TYPE_ERR,                                 //ASSERT生成的断言信息
    TRACE_TYPE_LOG,                                 //LOGMSG生成的信息，无需保存(仅DSP)
};

//TRACE信息
typedef struct{
#ifdef _MANG_UNIT_
    uuid_c          uid;                                        //每个SOE均有一个随机的uuid
    int8            cpu_num;                                    //由哪个CPU生成的报告
#endif
    TFullDateTime   dtTime;                                     //时间
    uint8           nType;                                      //类型，对应E_TRACE_TYPE
    char            filename[MAX_TRACE_FILENAME_LEN];           //文件名	__FILE__
    uint16          line_num;                                   //行号        __LINE__
    char            desc[MAX_TRACE_DESC_LEN];                   //描述
}TSOE_TRACE_STRUCT;

/************************************************************************/
//  遥信变位SOE 对应DSP数据结构SOE_TYPE
/*
1、	保护装置需支持品质位变化上传，包括通断品质与检修品质；
2、	保护装置保留原SOE值变化上传机制，该值是保护经逻辑以及防抖综合处理过的值，形成的SOE需包含品质位信息；
3、	保护装置需增加通断品质、检修品质变化SOE上传；其中通断品质与检修品质是未经延时处理的原始输入；由启动DSP直接生成；
4、	SOE结构中字段state与com_flag含义需扩充，具体如下：
高位---------------------------------------------------------------------低位
State:      Bit7       Bit6    Bit5   Bit4          Bit3   Bit2    Bit1   Bit0 
           通断品质  检修品质  双点   GOOSE状态                    值（合） 值（分）

flag:       Bit7   Bit6       Bit5      Bit4    Bit3   Bit2  Bit1  Bit0
                 通断变位   检修变位   值变位    合变位 分变位
5、	单点SOE时State中值取Bit0。
6、	双点遥信SOE的dz_index取合位遥信的dz_index。
7、 GOOSE状态 表示当前是GOOSE点，需要显示通断品质和检修品质
8、	液晶显示格式如下：
001
2015年02月05日18时:32分:32秒.011毫秒
保护远方操作硬压板
                       值  0->1
                       检修  0
                       品质  0
9、	装置如果处于检修位，信号量和模拟量仅检修品质变化，不要产生SOE报告。（因为装置的检修态是特殊处理的） 
10、目前可以生成遥信变位的包括： 
    POINT_TYPE_SIGNAL_INSIDE        ,  //内部信号
    POINT_ACTION_UNIT               ,  //动作元件
    POINT_RUN_ALARM                 ,  //运行告警
    POINT_TYPE_HARD_STRAP           ,  //硬压板/开入定义
    POINT_TYPE_VIRTUAL_SIGNAL       ,  //虚遥信
    POINT_TYPE_DUAL_SIGNAL          ,  //双点遥信
    POINT_TYPE_DI                   ,  //装置开入
    POINT_TYPE_DO                   ,  //装置开出
    POINT_TYPE_SOFT_STRAP           ,  //软压板
11、BM侧不再提供运行告警报告、硬压板变位报告，如果需要，由Linux侧根据遥信变位报告生成 
 
*/
/************************************************************************/
#define RSIGNAL_STATE_VALUE_TRIP        D0      //值（分）
#define RSIGNAL_STATE_VALUE_CLOSE       D1      //值（合）
#define RSIGNAL_STATE_GOOSE_STAT        D4      //Goose状态
#define RSIGNAL_STATE_DUAL_POINT        D5      //双点
#define RSIGNAL_STATE_TEST_QUALITY      D6      //检修品质
#define RSIGNAL_STATE_COMM_QUALITY      D7      //通断品质
#define RSIGNAL_STATE_MASK              0xE3    //掩码

#define RSIGNAL_FLAG_TRIP_CHG           D2      //分变位
#define RSIGNAL_FLAG_CLOSE_CHG          D3      //合变位
#define RSIGNAL_FLAG_VALUE_CHG          D4      //值变位
#define RSIGNAL_FLAG_TEST_CHG           D5      //检修变位
#define RSIGNAL_FLAG_COMM_CHG           D6      //通断变位
#define RSIGNAL_FLAG_MASK               0x7C    //掩码

typedef struct {
#ifdef _MANG_UNIT_
    uuid_c          uid;                                        //每个SOE均有一个随机的uuid
    int8            cpu_num;                                    //由哪个CPU生成的报告
#endif
    TFullDateTime dtTime;           //时间
    int16 nPointType;               //测点类型，对应POINT_TYPE_xxxx
    TID nPointID;                   //变位项ID
    TID nRealIOID;                  //对于物理开入/开出，还需要填充实遥信的开入/开出ID（-1为非实遥信，POINT_TYPE_DI）
    uint8 state;                    //变位状态0,1
    uint8 flag;                     //变位标志
}TSOE_RSIGNAL_STRUCT;

/************************************************************************/
//  动作报告SOE结构
/************************************************************************/
#define MMAX_SOE_VALUE_COUNT            16          //保护动作报告后所根测值的最大个数

//动作报告SOE结构
typedef struct {
    TID  nProtMeasID;            //故障测值ID
    TVAR_DATA var;               //故障测值的值
}TVALUE_IN_SOE;

typedef struct {
#ifdef _MANG_UNIT_
    uuid_c          uid;                                        //每个SOE均有一个随机的uuid
    int8            cpu_num;                                    //由哪个CPU生成的报告
#endif
    TFullDateTime dtTime;           //时间
    uint32 qtime;                   //相对时间，us微妙时间
    TID nActionID;                  //动作元件ID
    uint8 state;                    //动作还是返回0/1
    uint16 nPhase;                  //故障相别
    TVALUE_IN_SOE MeasValue[MMAX_SOE_VALUE_COUNT];  //故障值 - 对应故障信息
    uint16 nMeasValueCnt;                           //故障值个数
    uint16 fan;                     //fan号
    uint16 nof;                     //nof号
}TSOE_ACTION_STRUCT;

#ifdef _PROT_UNIT_

/************************************************************************/
//  测值变化报告，仅用于BM侧
//
/************************************************************************/

#define MAX_MEASURE_UINT8_COUNT     (MAX_POINT_COUNT_PER_TYPE / 8 + 1)
//测值变化报文
typedef struct{
    TFullDateTime dtTime;
    uint8 uMeasureBit[MAX_MEASURE_UINT8_COUNT];
}TSOE_MEASURE_CHG_STRUCT;

#endif  //_PROT_UNIT_

#endif /* SRC_INCLUDE_PLT_SOETYPE_H_ */
