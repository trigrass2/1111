/*
 * @Author: zhongwei
 * @Date: 2019-11-18 16:18:05
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 09:22:18
 * @Description: 基本数据类型定义
 * @FilePath: \ZynqBM\src\include\plt_BaseType.h
 */
#ifndef SRC_INCLUDE_PLT_BASETYPE_H_
#define SRC_INCLUDE_PLT_BASETYPE_H_

/* 实际在Zynq下测试结果如下
sizeof(short) = 2
sizeof(long) = 4
sizeof(int) = 4
sizeof(long long) = 8
sizeof(float) = 4
sizeof(double) = 8
sizeof(long double) = 8
*/

//基本数据类型
#ifndef _PLT_DATA_BASETYPE_

    /*整数*/
    typedef unsigned char   uint8;          //无符号8位整型变量
    typedef signed char     int8;           //有符号8位整型变量
    typedef unsigned short  uint16;         //无符号16位整型变量
    typedef signed   short  int16;          //有符号16位整型变量
    typedef unsigned int    uint32;         //无符号32位整型变量
    typedef signed   int    int32;          //有符号32位整型变量
    /*浮点数*/
    typedef float           fp32;           //单精度浮点数（32位长度）
    /*布尔型*/
    typedef int32           BOOL;           //布尔型的量,32位整数
    
    /* 64位数据类型 */
    typedef double      fp64;           //双精度浮点数（40位长度）
    typedef long long       int64;          //有符号64位整型变量	   
    typedef unsigned long long  uint64;     //无符号64位整型变量

    /* 字符串类型 */
    typedef char c32_t[32 + 1];
    typedef char c64_t[64 + 1];
    typedef char c128_t[128 + 1];
    typedef char c256_t[256 + 1];
    typedef char c512_t[512 + 1];
#endif

#define fpd   fp64          //默认浮点类型

//定义ID的数据类型0~32767
typedef int32 TID;
//定义GIN号数据类型
typedef uint16 TGin;
//103组号
typedef int16 TGrpNo;
//103条目号
typedef int16 TItemNo;
//FUN号
typedef int16 TFun; 
//INF号
typedef int16 TInf;

//定义值类型
typedef int32 TiVal;    //整形值
typedef fpd TfVal;     //浮点值     fp64的计算也很快，所以建议使用fp64
typedef int32 TVAL;     

typedef int32 STATUS;   //状态返回

typedef char * char_p;  //字符串指针
typedef const char * const_char_p;

typedef int32 * SAMPLE_DATA;    //定义采样值 

typedef void * PTR;

//TRUE  FALSE
#ifndef FALSE
#   define FALSE 0
#endif //FALSE

#ifndef TRUE
#   define TRUE 1
#endif //TRUE

#ifndef  SUCCESS
#   define SUCCESS  1     //成功 
#endif

#ifndef  FAILURE
#   define FAILURE  0     //失败
#endif


/*
    BM侧和Linux侧定义的PLT_TRUE和PLT_FALSE是不一样的
    BM侧为了可靠性考虑，使用0/0xff
    Linux则使用通用的0/1
*/
//BM侧使用的TRUE和FALSE
#define BM_TRUE         0xff
#define BM_FALSE        0

#ifdef _PROT_UNIT_
    //定义平台使用的TRUE，为了防误
    #define PLT_TRUE    BM_TRUE
    //定义平台使用的FALSE
    #define PLT_FALSE   BM_FALSE
#else
    //定义平台使用的TRUE，为了防误
    #define PLT_TRUE    TRUE
    //定义平台使用的FALSE
    #define PLT_FALSE   FALSE
#endif


//! NULL
#ifndef NULL
#ifdef  __cplusplus
#define NULL   0    ///< 0
#else
#define  NULL   ((void *)0) ///< (void *)0
#endif
#endif

// 位信息定义
enum{
    D0  =0x01,
    D1  =0x02,
    D2  =0x04,
    D3  =0x08,
    D4  =0x10,
    D5  =0x20,
    D6  =0x40,
    D7  =0x80
};

#ifndef DI_ON

#define DI_ON       1
#define DI_OFF      0

#define DO_ON       1
#define DO_OFF      0

#endif  //DI_ON

#define LNK_NONE    -1

//品质类型定义
typedef int8 TQuality;

#define QUALITY_NONE    -1  //无效

#define QUALITY_OK      0   //通断品质正常
#define QUALITY_ERR     1   //通断品质异常

#define QUALITY_NORMAL  0   //无检修品质/无越限品质
#define QUALITY_TEST    1   //有检修品质

#define QUALITY_OVER    1   //有越限品质

//日期时间，精确到毫秒，DSP中存储的时间为S和ms
/***
    s 秒：表示从2000.1.1 0:0:0开始到现在所经过的S时间数
    us 微秒：表示从当前的us时间 0~999999
***/
typedef struct{
    int32 s;    //秒时间
    uint32 us;  //微秒
}TDateTime;

//完整的时间信息
typedef struct{
    uint16 y;   /*!< 年 2006 */
    uint8 mon;  /*!< 月 */
    uint8 d;    /*!< 日 */
    uint8 h;    /*!< 时 */
    uint8 min;  /*!< 分 */
    uint8 s;    /*!< 秒 */
    uint32 us;  /*!< 微秒 */
}TFullDateTime;

//复数
typedef struct{
    TfVal Real;         //实部
    TfVal Image;        //虚部    
}TCOMPLEX;

typedef struct {
    int8 type;         //E_VAR_DATA_TYPE
    union
    {
        BOOL b;
        int32 i_32;
        int64 i_64;
        fp64 f_64;
    }data;
}TVAR_DATA;


//变量注册类型
enum VAR_REGISTER_TYPE{
    VARREG_TYPE_NONE = 0,       //无
    VARREG_TYPE_BOOL,           //BOOL
    VARREG_TYPE_int8,           //int8
    VARREG_TYPE_uint8,          //uint8
    VARREG_TYPE_int16,          //int16
    VARREG_TYPE_uint16,         //uint16
    VARREG_TYPE_int32,          //int32
    VARREG_TYPE_uint32,         //uint32
    VARREG_TYPE_int64,          //int64
    VARREG_TYPE_uint64,         //uint64
    VARREG_TYPE_fp32,           //fp32
    VARREG_TYPE_fp64,           //fp64
    VARREG_TYPE_char_p,         //char *
    VARREG_TYPE_TVAR_DATA,      //TVAR_DATA
    VARREG_TYPE_TDateTime,      //TDateTime
    VARREG_TYPE_TFullDateTime,  //TFullDateTime
    VARREG_TYPE_TCOMPLEX,       //TCOMPLEX
    VARREG_TYPE_PTR,            //指针
    VARREG_TYPE_SAMPLE_DATA,    //采样值
    VARREG_TYPE_COUNT,          //总个数

    //引用类型
    VARREG_TYPE_fpd = VARREG_TYPE_fp64,     //fpd
    VARREG_TYPE_TID = VARREG_TYPE_int32,    //TID
    VARREG_TYPE_TGin = VARREG_TYPE_uint16,  //TGin
    VARREG_TYPE_TiVal = VARREG_TYPE_int32,  //TiVal
    VARREG_TYPE_TfVal = VARREG_TYPE_fpd,    //TfVal
    VARREG_TYPE_TVAL = VARREG_TYPE_int32,   //TVAL
    VARREG_TYPE_STATUS = VARREG_TYPE_int32, //STATUS
    VARREG_TYPE_string = VARREG_TYPE_char_p,    //char *
    VARREG_TYPE_COMPONENT_SET = 0xff,       //元件设定值，需要特殊处理
};
 
//TVAR_DATA 类型定义
enum E_VAR_DATA_TYPE
{
    VAR_DATA_TYPE_NONE = VARREG_TYPE_NONE,
    VAR_DATA_TYPE_BOOL = VARREG_TYPE_BOOL,
    VAR_DATA_TYPE_INT32 = VARREG_TYPE_int32,
    VAR_DATA_TYPE_INT64 = VARREG_TYPE_int64,
    VAR_DATA_TYPE_FP64 = VARREG_TYPE_fp64,
};

typedef union{
    BOOL b;
    int8 i8;
    uint8 u8;
    int16 i16;
    uint16 u16;
    int32 i32;
    uint32 u32;
    int64 i64;
    uint64 u64;
    fp32 f32;
    fp64 f64;
    char * c_p;
    TVAR_DATA var;
    TDateTime dt;
    TFullDateTime fdt;
    TCOMPLEX cpx;
    void * v_p;
}TVAL_UNION;

//走时时间
typedef struct {
    int32 ms;               //走过的毫秒时间
    int32 us;               //走过的微妙时间
    uint32 os_tick;         //走过的系统tick（对应os_get_ticks_per_sec()）
    uint32 cycles;          //走过的CPU cycles（对应hd_get_timer_clk_freq()）
}TTimeRun;

//添加mytypeses.h
typedef unsigned char   BYTE;
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

#define OK         0
#define ERROR     -1
typedef int       STATUS;

#endif /* SRC_INCLUDE_PLT_BASETYPE_H_ */
