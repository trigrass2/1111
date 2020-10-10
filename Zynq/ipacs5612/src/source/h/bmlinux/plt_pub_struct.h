/*
 * @Author: zhongwei
 * @Date: 2019/11/27 13:38:19
 * @Description: 公共数据结构定义（用于裸核和Linux）
 * @File: plt_pub_struct.h
 *
*/

#ifndef SRC_INCLUDE_PLT_PUB_STRUCT_H_
#define SRC_INCLUDE_PLT_PUB_STRUCT_H_

#include "plt_exception.h"

/************************************************************************/
//  平台版本定义
//历史
/*
    2019-12-15 1.00.0001
*/ 
/************************************************************************/
//平台版本
#ifdef _DEBUG
#define VERSION_DEBUG       "b"
#else
#define VERSION_DEBUG       ""
#endif

//装置类型标志
#ifndef VERSION_FLAG_T
#   define VERSION_FLAG_T ""
#endif

#define VERSION_FLAG            VERSION_FLAG_T

/*
版本前两个段为版本号：如1.20
最后第三段四位数字为全局编号，并且递加，如1000,1001,....9999
*/
#define VERSION_NUM     "1.00.0001"

//最终组成装置里显示的平台版本字符串
#define VERSION_PLATFORM    VERSION_NUM "" VERSION_FLAG "" VERSION_DEBUG


/************************************************************************/
//      公共数据结构定义 - 开始
//
/************************************************************************/


#ifndef _TSTRING_RESOURCE_
#define _TSTRING_RESOURCE_

//支持的语言
enum
{
#ifdef  _LANGUAGE_CN_ENABLE
    LANGUAGE_CN,            // 中文
#endif  //  _LANGUAGE_CN_ENABLE

#ifdef  _LANGUAGE_EN_ENABLE
    LANGUAGE_EN,            // 英文
#endif  //  _LANGUAGE_EN_ENABLE

#ifdef  _LANGUAGE_RU_ENABLE
    LANGUAGE_RU,            // 俄文
#endif  //  _LANGUAGE_RU_ENABLE

    LANGUAGE_COUNT      // 支持的语言数量
};

//字符串资源数据结构
typedef struct tagSTRING_RESOURCE {
    const char *str[LANGUAGE_COUNT];
}TSTRING_RESOURCE;

#endif // _TSTRING_RESOURCE_

//定值类型的定义
enum E_VALUETYPE
{
    VALUETYPE_NUMBER    = 1,         //数值型
    VALUETYPE_ENUM                  //枚举型
};

//枚举型数据结构 - 值与字符串对应结构
typedef struct {
    TiVal       nValue;                             //值
    TID         nNameStrID;                         //对应的名称
}TVALUETYPE_ENUM_VALUE;

//枚举型数据结构
typedef struct {
    uint16                  nItemCount;             //值项个数
    const TVALUETYPE_ENUM_VALUE *pEnumItems;       //指向项目的数组
}TVALUETYPE_ENUM;

//数字型数据结构
typedef struct {
    TVAL        nMax;                   //最大值
    TVAL        nMin;                   //最小值
    uint16      nStep;                  //步长
    uint16      nDecimal;               //小数位数
    uint16      nIntger;                //整数位数
}TVALUETYPE_NUMBER;

//测点配置，对应EPIDE测点配置对话框
typedef struct {
    int16 point_type;                               //测点类型
    TID nNameStrID;                                 //测点组标题字符串资源ID
    TGrpNo nGrpNo[MAX_GRP_COUNT_PER_DATE_GROUP];           //组号，每种类型最多支持4组，-1表示无定义
    BOOL bUseCidName;                               //该组测点是否使用CID中描述显示
    BOOL bEn103;                                    //103组使能
    BOOL bEn61850;                                  //61850组使能
    int16 value_type;                               //数值类型，对应VAR_REGISTER_TYPE
    BOOL bTestQuality;                              //检修品质
    BOOL bCommQuality;                              //通断品质
    BOOL bOverQuality;                              //越限品质
}TPOINT_TYPE_CONFIG;

//用于显式的提示工程配置的信息
typedef struct{
    const char * szID;                              //宏定义ID字符串
    const char * szName;                            //宏定义名称字符串
    const char * szValue;                           //宏定义的值字符串
}TPROJECT_CONFIG_STR_STRUCT;

typedef struct{
    c32_t cn;
    c32_t en;
}TINNER_STRING_RESOURCE;

#define MAX_INNER_ENUM_VALUE_COUNT                        8
#define MAX_INNER_ENUM_TYPE_COUNT                         32

typedef struct {
    TiVal       nValue;                                                   //值
    TINNER_STRING_RESOURCE resValue;                                      //名称
}TINNER_VALUETYPE_ENUM_VALUE;

//枚举型数据结构
typedef struct {
    uint16                  nItemCount;                                   //值项个数
    TINNER_VALUETYPE_ENUM_VALUE EnumItems[MAX_INNER_ENUM_VALUE_COUNT];    //指向项目的数组
}TINNER_VALUETYPE_ENUM;

#define STRING_ID           c32_t

//BM内部设定值
typedef struct{
    TID                             nID;         //序号
    STRING_ID                       szID;        //ID
    TINNER_STRING_RESOURCE          resName;     //名称字符串资源
    c32_t                           szUnit;         //单位
    TINNER_STRING_RESOURCE          resFolder;      //分组
    TVAL                            nDefaultValue;  //默认值
    int16                           nValueType;  //数值类型              E_VALUETYPE
    TVALUETYPE_NUMBER               vNumberDef;  //如果是数值类型，其定义
    TID                             nEnumDefIdx;//枚举定义的索引
    TID                             nSpecialType;  //特殊类型定义
    TVAR_DATA                       varSpecialValue;//特殊类型附加值
    c32_t                           szRange;  //定值范围字符串
    TID                             n61850Unit;  //61850量纲
    TID                             n61850Multiple;  //61850量纲倍数
    fp64                            f61850Rate;  //61850变比
    BOOL                            bRebootToActive;  //重启激活
}TINNER_BM_SET_STRUCT;

//BM内部调试数据
typedef struct{
    TID                             nID;         //序号
    STRING_ID                       szID;        //ID
    TINNER_STRING_RESOURCE          resName;     //名称字符串资源
    c32_t                           szUnit;         //单位
    TINNER_STRING_RESOURCE          resFolder;      //分组
    int8                            nFraction;  //小数位数
    TVAR_DATA                       varVarDataDef;  //值类型(默认值）
    TID                             nMeasureType;  //测值类型
    BOOL                            bSave;  //掉电保存
    uint32                          nSavePeriod;  //保存间隔(ms)
    TID                             n61850Unit;  //61850量纲
    TID                             n61850Multiple;  //61850量纲倍数
    fp64                            f61850Rate;  //61850变比
}TINNER_BM_DBGDATA_STRUCT;

#define TBM_SETTING_STRUCT TSYS_SETTING_STRUCT       //内部设定值 （从TINNER_BM_SET_STRUCT转换获得）
#define TBM_DBGDATA_STRUCT TDEBUG_DATA_STRUCT        //内部调试数据  （从TINNER_DBGDATA_STRUCT转换获得）

/************************************************************************/
//      公共数据结构定义 - 结束
/************************************************************************/


/************************************************************************/
//  遥控相关-开始
//
/************************************************************************/
//操作源
enum E_SRC_TYPE{
    SRC_TYPE_NONE = 0xFF,   //无操作源
    SRC_TYPE_HMI = 1,       //HMI操作
    SRC_TYPE_SNTP,          //SNTP操作，仅用于校时
    SRC_TYPE_61850,
    SRC_TYPE_103,
    SRC_TYPE_TCP103,
    SRC_TYPE_IEC103,
    SRC_TYPE_MODBUS,
    SRC_TYPE_DEBUG,
    SRC_TYPE_MANUAL,         //手动
};

//同期方式
enum E_RC_SYNC_TYPE{
    SYNC_TYPE_NONE = 0,     //不允许
    SYNC_TYPE_SYNC,         //选择同期工作方式
    SYNC_TYPE_NO_VOLT,      //选择检无压工作方式
    SYNC_TYPE_NO_CHECK,     //选择不检工作方式
    SYNC_TYPE_VOLT_AND_SYNC,//先检无压后检同期
};

// 控制指令类型
enum E_CONTROL_CMD_TYPE{
    /** 遥控 **/
    CONTROL_CMD_TRIP            = 1,            //跳
    CONTROL_CMD_CLOSE           = 2,            //合
    /** 同期 **/
    CONTROL_CMD_UP              = 11,           //升
    CONTROL_CMD_DOWN            = 12,           //降
    CONTROL_CMD_STOP            = 13,           //停
    /** 直控 **/
    CONTROL_CMD_DIR_EXE         = 21,           //直控操作
    /** 遥调 **/
    CONTROL_CMD_ADJUST          = 31,           //4~20mA遥调
};

//遥控 选择/执行/取消
enum E_CONTROL_ORDER{
    OP_SEL = 0,                 //选择
    OP_EXE = 1,                 //执行
    OP_CANCEL = 2,              //取消
    FJT_STOP = 7,               //停
    FJT_LOWER = 8,              //降
    FJT_HIGHER = 9,             //升
    FJT_RESERVED = 10,          //预留
    OP_NONE = 11,               //无
    OP_FAIL = 12,               //操作失败
    OP_DIR = 13,                //直控，目前仅用于APC
};

/************************************************************************/
//  遥控相关-结束
//
/************************************************************************/

/************************************************************************/
//      Linux和BM共享内存定义 - 开始
//      共享内存起始地址在.ld文件中定义，TSHARE_MEMORY_STRUCT = 0x3F000000
//历史
//  2019-12-12 版本 100 (VERSION_SHARED_MEMORY)
//
/************************************************************************/


#define FLAG_SHARED_COUNT               4               //有效标志个数
#define FLAG_SHARED_MEMORY_VALID        0xA55A00FFU     //共享内存有效标志
#define VERSION_SHARED_MEMORY           100             //共享内存数据结构版本
#define SHARED_RESERVE_UINT32_COUNT     32              //预留内存长度，uint32
#define FLAG_SHARED_RESET_VALID         0xFF11AA55U     //复位有效标志


//裸核打印输出到Linux的循环缓冲区长度
#define MAX_PRINT_TO_LINUX_BUFFER_LEN   (16 * 1024)     //16K

/**
 * @Function: TPRINT_TO_LINUX_STRUCT
 * @Description: 打印输出缓存 - 缓存输出到Linux侧 
 * @author zhongwei (2019/11/27 13:39:30)
 * 
*/
typedef struct {
    int32 nBufferLen;           //缓冲区长度
    int32 nBegin;               //头数据
    int32 nEnd;                 //尾数据
    char  buf[MAX_PRINT_TO_LINUX_BUFFER_LEN];   //缓冲区
}TPRINT_TO_LINUX_STRUCT;

/*
   核间通讯使用的报文缓冲区
*/
#define MC_INNER_COMM_PKG_LEN           1024            //核间通讯每帧报文长度
#define MC_INNER_COMM_PKG_BUF_NUM       64              //核间通讯缓冲报文数

/**
 * @Function: TMC_INNER_COMM_PKG
 * @Description: 核间通讯报文结构 
 * @author zhongwei (2019/11/28 14:36:24)
 * 
*/
typedef struct {
    int32 len;                                          //长度
    uint8 data[MC_INNER_COMM_PKG_LEN];                  //报文内容
}TMC_INNER_COMM_PKG;

typedef struct {
    int32 version;                                      //规约版本
    uint32 nPkgBufLen;
    int32 nBegin;
    int32 nEnd;
    TMC_INNER_COMM_PKG pkgBuf[MC_INNER_COMM_PKG_BUF_NUM];
}TMC_INNER_COMM_STRUCT;

/**
 * @Function: TMC_INNER_CONFIG_STRUCT
 * @Description: 保存BM内部调试数据结构和内部设定值结构 
 * @author zhongwei (2019/12/28 11:52:24)
 * 
*/
typedef struct{
    uint32 flag_begin;
    int32 innerBMSet_cnt;                                                           //BM内部设定值个数
    TINNER_BM_SET_STRUCT innerBMSet[MAX_POINT_TYPE_INNER_BM_SET_COUNT];             //BM内部设定值定义
    int32 innerBMDbgData_cnt;                                                       //BM内部调试数据个数
    TINNER_BM_DBGDATA_STRUCT innerBMDbgData[MAX_POINT_TYPE_INNER_BM_SET_COUNT];     //BM内部调试数据定义
    int32 innerValueEnumDef_cnt;                                                    //内部枚举定义索引个数
    TINNER_VALUETYPE_ENUM innerValueEnumDef[MAX_INNER_ENUM_TYPE_COUNT];             //内部枚举定义
    uint32 flag_end;
}TMC_INNER_CONFIG_STRUCT;

/**
 * @Function: TMC_SHARED_TIME
 * @Description: BM共享给Linux的时间信息 
 * @author zhongwei (2020/1/20 16:56:10)
 * 
*/
typedef struct{
    TFullDateTime fdt1;     //当前时间，保存两份
    TFullDateTime fdt2;
}TMC_SHARED_TIME;

/**
 * @Function: TMC_COMMAND_LINE
 * @Description: 控制另一个核的命令行指令
 * @author zhongwei (2020/2/10 14:48:55)
 * 
*/
typedef struct{
    BOOL bCmd;              //是否有命令要执行
    uint32 flag_begin;
    c128_t str_cmd;         //命令行字符串
    uint32 flag_end;
}TMC_COMMAND_LINE;


#define MAX_SOE_NUM (512)   //最大512
#define MAX_YC_NUM  (512)
#define MAX_SYX_NUM (2048)
#define MAX_DYX_NUM (128)
#define MAX_RECORD_NUM (8)
#define MAX_RECORD_DATA_LEN (64*1024)
#define MAX_EVENT_NUM (64)
#define MAX_EVENT_DATA_LEN (1024)
#define MAX_TEQP_NUM  (8)

struct VCalClock1{
  DWORD dwMinute;
  WORD wMSecond;
};

typedef struct{	
	BYTE type; //单双点 0单点 1双点
	WORD wNo;
	WORD byValue;
	struct VCalClock1 Time;
}VSHARESOEREC;	


typedef struct{
	VSHARESOEREC soe[MAX_SOE_NUM];
	WORD wp;
	WORD rp;
}TMC_SHARED_SOE;

/** 
 *  CONFIG_DATA用于上电时Linux初始化BM 
 *  数据格式为：
 *    POINT_TYPE字符串，以0字符结尾
 *    ID 字符串，以0字符结尾
 *    数据类型 - VAR_REGISTER_TYPE，2字节
 *    数据长度 - 2字节
 *    数据 - n字节（对应数据长度）
 */
#define MAX_CONIG_DATA_LEN (256*1024)   //最大128k
typedef struct{
    uint32 flag_begin;
    uint32 fileflag;
	uint32 prsetpubfileflag;
	uint32 prsetfileflag;
    uint8 data[MAX_CONIG_DATA_LEN];
    uint32 flag_end;
}TMC_CONFIG_DATA;

typedef struct{	
	WORD fd;
	WORD smpfreq;  //采样频率
	WORD smpnum;   //采样点数
	struct VCalClock1 time;
	struct VCalClock1 pretime;
	BYTE record[MAX_RECORD_DATA_LEN];
}VRECORDDATA;


typedef struct{
	VRECORDDATA record[MAX_RECORD_NUM];
	WORD wp;
	WORD rp;
}VBMSHARERECORD;

typedef struct {
	int flag;
    int32 len;                                          //长度
    uint8 data[MAX_EVENT_DATA_LEN];                  //报文内容
}VEVENTDATA;

typedef struct {
	WORD wp;
	WORD rp;                                         //长度
    VEVENTDATA eventdata[MAX_EVENT_NUM];                  //报文内容
}VBMSHAREEVENT;

/**
 * @Function: TSHARED_BAREMETAL_STRUCT
 * @Description:  裸核的共享数据结构
 * @author zhongwei (2019/11/27 13:47:21)
 * 
*/
typedef struct {
    uint32 flag_begin[FLAG_SHARED_COUNT];
    uint32 struct_version;                              //结构版本
    c128_t device_uuid;                                 //裸核程序的装置uuid 
    c128_t software_uuid;                               //裸核程序的软件uuid

    volatile uint32 reset_flag_0;                                //BM复位标志0，每次复位都将标志置上
    volatile uint32 reset_flag_1;                                //BM复位标志1，每次复位都将标志置上
   
    /** BM共享给Linux的数据内容 - 开始 *   */
    TPRINT_TO_LINUX_STRUCT print_struct;
    TMC_INNER_COMM_STRUCT inner_comm_struct;
    TMC_SHARED_TIME shared_time;                        //BM共享给Linux的时间信息
    TMC_INNER_CONFIG_STRUCT inner_bm_config;            //BM内部配置，需要传送给Linux
    /** BM共享给Linux的数据内容 - 结束 *   */

	uint32 prsetpubflag;
	uint32 prsetflag;
	uint8 prosetpubtable[8192];
	uint8 prosettable[8192];
	uint8 prosetpub[18][1024];
	uint8 proset[18][1024];
		
	//遥测
	long Bm_dbYcValue[MAX_TEQP_NUM][MAX_YC_NUM];  //预留512个遥测
	//遥信
	BYTE Bm_SyxValue[MAX_TEQP_NUM][MAX_SYX_NUM];   //单点
	WORD Bm_DyxValue[MAX_TEQP_NUM][MAX_DYX_NUM];    //双点
	//soe
	WORD Bm_DiValue[20];    //共享DiValue
	
	DWORD dwAIType;
	DWORD dwIOType;
	
	TMC_SHARED_SOE BmSoebuf[MAX_TEQP_NUM];
	VBMSHARERECORD BmRecord;
	VBMSHAREEVENT  BmEvent;
	
	DWORD dwEmergencyflag;
	
    uint32 reserve[4096];                               //预留16k空间用于以后扩展

    uint32 flag_end[FLAG_SHARED_COUNT];
}TSHARED_BAREMETAL_STRUCT;

typedef struct {
    uint32 flag_begin[FLAG_SHARED_COUNT];
    uint32 struct_version;                              //结构版本

    /*
        用于指示BM复位进度的标识
        配置BM初始化过程
    */
    volatile uint32 wait_flag_0;
    volatile uint32 wait_flag_1;

    /** Linux共享给BM的数据内容 - 开始 *   */
    TMC_INNER_COMM_STRUCT inner_comm_struct;
    TMC_SHARED_TIME shared_time;                        //linux共享给BM的时间信息
    TMC_COMMAND_LINE command_line;                      //linux控制BM的命令行信息
    TMC_CONFIG_DATA config_data;                        //linux配置数据
    uint32 ledmask;                               //LED MASK
	uint32 ledvalue;                              //LED
    /** Linux共享给BM的数据内容 - 结束 *   */
	
    uint32 reserve[4096];                               //预留16k空间用于以后扩展

    uint32 flag_end[FLAG_SHARED_COUNT];
}TSHARED_LINUX_STRUCT;

/**
 * @Function: TSHARE_MEMORY_STRUCT
 * @Description: 共享内存数据结构 （Linux和裸核）
 * @author zhongwei (2019/11/27 13:43:06)
 * 
*/
typedef struct {
    uint32 flag_begin[FLAG_SHARED_COUNT];               //头标志 4个标志
    uint32 struct_version;                              //结构版本

    uint32 reserve1[SHARED_RESERVE_UINT32_COUNT];       //预留

    TSHARED_BAREMETAL_STRUCT bm_shared;                 //BM共享给Linux

    uint32 reserve2[SHARED_RESERVE_UINT32_COUNT];       //预留

    TSHARED_LINUX_STRUCT linux_shared;                  //Linux共享给BM

    uint32 reserve3[SHARED_RESERVE_UINT32_COUNT];       //预留

    uint32 reserve4[4096];                              //预留16k空间用于以后扩展

	uint32 nvram[4096];                              //16k铁电
	
    uint32 flag_end[FLAG_SHARED_COUNT];                 //尾标志 4个标志
}TSHARE_MEMORY_STRUCT;

/*
    DDR中BM和Linux共享内存的地址空间
    必须与lscript.ld文件同步
*/
#define DDR_SHART_START_ADDR    0x3F000000              //起始地址
#define DDR_SHARE_LENGTH        0x1000000               //长度 16M

/************************************************************************/
//      Linux和BM共享内存定义 - 结束
/************************************************************************/

typedef int32           (*FUNCPTR_get_cnt)(int8 cpu_num);                                 //读测点个数
typedef const void*     (*FUNCPTR_get_def)(int8 cpu_num, TID id);                         //读测点定义结构函数
typedef const char *    (*FUNCPTR_get_id_str)(int8 cpu_num, TID id);                      //读测点ID字符串
typedef TID             (*FUNCPTR_get_name_strid)(int8 cpu_num, TID id);                  //读测点名称字符串资源ID
typedef const char *    (*FUNCPTR_get_name_str)(int8 cpu_num, TID id, c128_t ch);         //读测点名称字符串
typedef const char *    (*FUNCPTR_get_unit_str)(int8 cpu_num, TID id);                    //读测点量纲字符串
typedef TGin            (*FUNCPTR_get_gin)(int8 cpu_num, TID id);                         //读取测点的gin号
typedef BOOL            (*FUNCPTR_get_def_val_ex)(int8 cpu_num, TID id, void * pdata, int16 reg_type, int32 max_len);   //读测点的默认值
typedef BOOL            (*FUNCPTR_get_val_ex)(int8 cpu_num, TID id, void * pdata, int16 reg_type, int32 max_len);   //读测点的当前值
typedef BOOL            (*FUNCPTR_set_val_ex)(int8 cpu_num, TID id, const void * pdata, int16 reg_type, int32 max_len);   //写测点的当前值
typedef TQuality        (*FUNCPTR_get_quality)(int8 cpu_num, TID id);                     //读品质（检修、通断、越限）
typedef void            (*FUNCPTR_set_quality)(int8 cpu_num, TID id, TQuality q);         //写品质（检修、通断、越限）
typedef BOOL            (*FUNCPTR_quality_en)(int8 cpu_num, TID id);                      //品质是否使能（检修、通断、越限）
typedef void            (*FUNCPTR_get_max_min)(int8 cpu_num, TID id, int32 * pMax, int32* pMin);   //获取最大最小值(定值类，肯定是整数)
typedef int32           (*FUNCPTR_get_step)(int8 cpu_num, TID id);                  //读取步长（定值类，肯定是整数）
typedef int8            (*FUNCPTR_get_fraction)(int8 cpu_num, TID id);                    //读取测点小数位数
typedef const char *    (*FUNCPTR_get_val_ex_str)(int8 cpu_num, TID id, const void * pdata, int16 reg_type, c128_t ch);          //读测点值字符串
typedef TID             (*FUNCPTR_get_visible_id)(int8 cpu_num, TID id);                  //读测点关联隐藏显示内部信号ID(如果有的话)
typedef int32           (*FUNCPTR_get_61850_unit)(int8 cpu_num, TID id);                  //读取61850量纲
typedef int32           (*FUNCPTR_get_61850_multiple)(int8 cpu_num, TID id);              //读取61850量纲倍数
typedef fp64            (*FUNCPTR_get_61850_rate)(int8 cpu_num, TID id);                  //读取61850变比
typedef BOOL            (*FUNCPTR_porm_pt)(int8 cpu_num, TID id);                         //测点是否保护/测控测点

//测点分类
enum E_PT_CLASS{
    PT_CLASS_DEFAULT = 0,        //默认，无
    PT_CLASS_STATUS = 1,         //状态量   一般值类型是BOOL或uint8(双点)
    PT_CLASS_MEASURE = 2,        //测量量   一般值类型是TVAR_DATA
    PT_CLASS_SETTING = 3,        //设定值   一般值类型是int32/fp64
    PT_CLASS_STR_SETTING = 4,    //字符串设定值 一般值类型是char * 
    PT_CLASS_INFO = 5,           //信息     一般值类型是char *
    PT_CLASS_CONTROL = 6,        //控制类
};

//测点类型定义结构
typedef struct {
    int16 point_type;                                     //测点类型POINT_TYPE_xxx
    const char * szType;                                  //类型字符串
    void * func_get;                                      //读值
    void * func_set;                                      //写值
    void * func_get_d;                                    //读取默认值（如果有）
    int32 max_count;                                      //测点最大个数
    int16 pt_class;                                       //测点分类 PT_CLASS_xxx
    FUNCPTR_get_cnt func_get_cnt;                         //读测点个数函数指针
    FUNCPTR_get_def func_get_def;                         //读测点定义结构函数指针
    FUNCPTR_get_id_str func_get_id_str;                   //读测点ID字符串
    FUNCPTR_get_name_strid func_get_name_strid;           //读测点名称字符串资源ID函数指针
    FUNCPTR_get_name_str func_get_name_str;               //读测点名称字符串函数指针
    FUNCPTR_get_unit_str func_get_unit_str;               //读测点量纲字符串函数指针
    FUNCPTR_get_gin func_get_gin;                         //读测点GIN函数指针
    FUNCPTR_get_def_val_ex func_get_def_val_ex;           //读取测点默认值函数指针
    FUNCPTR_get_val_ex func_get_val_ex;                   //读取测点值函数指针
    FUNCPTR_set_val_ex func_set_val_ex;                   //写测点值函数指针
    FUNCPTR_get_quality func_get_q_test;                  //读品质
    FUNCPTR_set_quality func_set_q_test;                  //写品质
    FUNCPTR_quality_en  func_get_q_test_en;               
    FUNCPTR_get_quality func_get_q_comm;
    FUNCPTR_set_quality func_set_q_comm;
    FUNCPTR_quality_en  func_get_q_comm_en;
    FUNCPTR_get_quality func_get_q_over;
    FUNCPTR_set_quality func_set_q_over;
    FUNCPTR_quality_en  func_get_q_over_en;
    FUNCPTR_get_fraction func_get_fraction;               //读取测点小数位数
    FUNCPTR_get_val_ex_str func_get_val_str_ex;           //读测点值字符串函数指针
    FUNCPTR_get_visible_id func_get_visible_id;           //读测点关联隐藏显示内部信号ID(如果有的话)
    FUNCPTR_get_max_min func_get_max_min;                 //读取最大最小值（设定值）
    FUNCPTR_get_step func_get_step;                       //读步长函数指针
    FUNCPTR_get_61850_unit func_get_61850_unit;           //读取61850量纲
    FUNCPTR_get_61850_multiple func_get_61850_mulpitle;   //读取61850量纲倍速
    FUNCPTR_get_61850_rate func_get_61850_rate;           //读取61850变比
    FUNCPTR_porm_pt func_get_prot_pt;                     //保护测点
    FUNCPTR_porm_pt func_get_meas_pt;                     //测控测点
}TPOINT_TYPE_DEFINE_STRUCT;

//Linux应用初始化给BM的配置信息
typedef struct{
    const char * szPointType;       //测点类型字符串
    const char * szPointID;         //测点ID字符串
    int16 reg_type;                 //值类型
    int32 val_length;               //值长度
    TVAL_UNION val;                 //数值存储
}TCONFIG_DATA_ITEM;

#endif /* SRC_INCLUDE_PLT_PUB_STRUCT_H_ */
