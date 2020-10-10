/*
 * @Author: zhongwei
 * @Date: 2019-11-19 13:39:03
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-19 14:08:22
 * @Description: 时间处理接口
 * @FilePath: \ZynqBM\src\include\plt_Time.h
 */
#ifndef SRC_INCLUDE_PLT_TIME_H_
#define SRC_INCLUDE_PLT_TIME_H_

//libpub版本
const char * get_version_libpub(void);               //版本
TFullDateTime get_buildtime_libpub(void);   //编译时间

void Time_AddSecond(TFullDateTime * pNow);

TFullDateTime ToFullDateTime(const TDateTime * pdt);
TDateTime ToDateTime(const TFullDateTime * pdt);
 TFullDateTime IRIG_to_FullDateTime(uint8 year, uint16 day, uint8 hour, uint8 minute, uint8 second);

BOOL CheckFullTime(const TFullDateTime * pTime);
BOOL CheckTime(const TDateTime * pTime);

//微妙转毫秒整数时间
#define us_to_ms(us)    ((us)/1000)
//微妙转毫秒小数部分
#define us_to_ms_fract(us)  ((us)%1000)

//毫秒转微妙时间
#define ms_to_us(ms)    ((ms)*1000)


// YYYY-MM-DD hh:mm:ss.ms
const char * str_GetFullDateTimeString(const TFullDateTime * pDateTime, c64_t ch);

// YYYY-MM-DD hh:mm:ss.us
const char * str_GetFullDateTimeWithUsString(const TFullDateTime * pDateTime, c64_t ch);

// YYYY-MM-DD hh:mm:ss 无毫秒时间
const char * str_GetFullDateTimeWithoutMsString(const TFullDateTime * pDateTime, c64_t ch);

// YY-MM-DD hh:mm:ss.ms 年YY
const char * str_GetFullDateTimeShortYearString(const TFullDateTime * pDateTime, c64_t ch);

// YYYY-MM-DD hh:mm:ss.ms
const char * str_GetDateTimeString(const TDateTime * pDateTime, c64_t ch);

// YYYY-MM-DD hh:mm:ss.us
const char * str_GetDateTimeWithUsString(const TDateTime * pDateTime, c64_t ch);

// YYYY-MM-DD hh:mm:ss 无毫秒时间
const char * str_GetDateTimeWithoutMsString(const TDateTime * pDateTime, c64_t ch);

// YY-MM-DD hh:mm:ss.ms 年YY
const char * str_GetDateTimeShortYearString(const TDateTime * pDateTime, c64_t ch);

//解析编译时间信息
TFullDateTime ParseDateTimeString(const char * strDate, const char * strTime);

TFullDateTime From_time_t(time_t t);
time_t To_Time_t(TFullDateTime fdt);

/************************************************************************/
//  当前时间的访问接口
//
/************************************************************************/
//获取当前系统时间
TFullDateTime GetSystemFullTime(void);

TFullDateTime i_GetCurrentTime(void);
TDateTime i_GetCurrentShortTime(void);

#define Now()   i_GetCurrentTime()

#endif /* SRC_INCLUDE_PLT_TIME_H_ */
