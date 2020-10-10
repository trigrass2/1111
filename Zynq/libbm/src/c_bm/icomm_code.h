/*
 * @Author: zhongwei
 * @Date: 2020/3/26 9:32:12
 * @Description: 内部通讯CODE码定义
 * @File: icomm_code.h
 * 
 *  
 * 报文格式： 
 *  
 * 0 CODE 
 * 1 FLAG                 标识
 * 2 LG(L)                从CODE开始，到报文内容最后
 * 3 LG(H) 
 * 4 Invoke ID(L)         0或任意数，0表示非回复报文
 * 5 Invoke ID(H) 
 *   .. 
 *   .. 
 *   ..    报文内容 
 *   .. 
 *   ..
 *  
 *  ====================================
 *   FLAG说明(按位)
 *   7 6 5 4 3 2 1 0
 *  
 *   0 - IC_FLAG_M 主要上送标志，主动上送的invoke id应该置0
 *   1 - IC_FLAG_I Invoke ID有效标志，同时表示本报文是被动回复
 *   2 - IC_FLAG_SP 分包标志
 *   3 - IC_FLAG_SE 分包最后一包标志
 *   4 -
 *   5 -
 *   6 - 
 *   7 - 
*/

#ifndef SRC_ICOMM_ICOMM_CODE_H_
#define SRC_ICOMM_ICOMM_CODE_H_


//内部通讯版本
#define ICOMM_VERION    100

//报文最大长度
#define ICOMM_PKG_MAX_LENGTH        MC_INNER_COMM_PKG_LEN

//报文内容最大长度 根据报文格式，去除CODE、FALG等
#define ICOMM_PKG_DATA_LENGTH       (ICOMM_PKG_MAX_LENGTH - 10)

//报文缓冲包数的最大个数 预留2个
#define ICOMM_PKG_BUFFER_COUNT      (MC_INNER_COMM_PKG_BUF_NUM - 2)

//定义报文水位
/*
    例如，对于定时报文，则水位定为一半，如果超出该水位，就不再传送定时报文了
*/ 
//1/2 普通报文水位（指一些定时上送的报文）
#define ICOMM_WATER_STAGE_NORMAL    (ICOMM_PKG_BUFFER_COUNT / 2)

//3/4 主动上送报告报文的水位，需要考虑为突发报文预留水位
#define ICOMM_WATER_STAGE_REPORT    (ICOMM_PKG_BUFFER_COUNT * 3 / 4)

//4/5 召唤返回报文的水位，需要考虑为突发报文预留水位
#define ICOMM_WATER_STAGE_RETURN    (ICOMM_PKG_BUFFER_COUNT * 4 / 5)

/************************************************************************/
//  CODE码定义
//
/************************************************************************/

enum E_IC_CODE_C{
    IC_CODE_ACK                  = 0x01,         //ACK 确认
    IC_CODE_NAK                  = 0x02,         //NAK 否认
    IC_CODE_KEEP_ALIVE           = 0x03,         //保持链接报文
};

//BM -> Linux的 CODE码
enum E_IC_CODE_B_to_L{
    IC_CODE_B_POINT              = 0x10,         //上送测点信息
    IC_CODE_B_REPORT             = 0x11,         //报告（变位、动作、TRACE）

    IC_CODE_B_WAVE_BRIEF         = 0x30,         //录波简报
    IC_CODE_B_WAVE_DATA          = 0x32,         //录波数据
    IC_CODE_B_WAVE_ADDINFO       = 0x33,         //录波附加信息

    IC_CODE_B_MEMORY             = 0x40,         //内存数据
    IC_CODE_B_EXCEPTION          = 0x41,         //异常数据
    IC_CODE_B_SOFTINFO           = 0x42,         //软件信息(包括名称和值)
};

//Linux -> BM的 CODE码
enum E_IC_CODE_L_to_B{
    IC_CODE_L_SET_POINT          = 0x80,        //设置测点信息(主要是设定值)
    IC_CODE_L_CHK_POINT          = 0x81,        //测点自检（主要是设定值自检，定时下发Linux侧的设定值到BM）
    IC_CODE_L_CALL_POINT         = 0x82,        //召唤测点信息（一次召唤一组，或多个单独的测点）

    IC_CODE_L_TOTAL_CALL         = 0x90,        //总召(召唤BM全部测点当前信息)

    IC_CODE_L_CALL_WAVE_DATA     = 0xA2,        //读取录波数据

    IC_CODE_L_CALL_MEMORY        = 0xC0,        //读取内存
    IC_CODE_L_CALL_SOFTINFO      = 0xC2,        //读取软件信息
};

//FLAG标识定义
enum E_IC_FLAG{
    IC_FLAG_M       =0x01,                      //主要上送标志，主动上送的invoke id应该置0
    IC_FLAG_I       =0x02,                      //Invoke ID有效标志，同时表示本报文是被动回复
    IC_FLAG_SP      =0x04,                      //分包标志
    IC_FLAG_SE      =0x08,                      //分包最后一包标志

};

#endif /* SRC_ICOMM_ICOMM_CODE_H_ */
