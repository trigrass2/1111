/*
 * @Author: zhongwei
 * @Date: 2020/1/9 9:31:45
 * @Description: BM侧时间处理
 * @File: plt_bm_time.h
 *
*/

#ifndef SRC_H_BAREMETAL_PLT_BM_TIME_H_
#define SRC_H_BAREMETAL_PLT_BM_TIME_H_

//系统时钟数据结构
typedef struct{
    //系统时钟
    TFullDateTime dt;           //当前时间
}TSystemTime;

extern TSystemTime g_SystemTime;

//上电时钟初始化
void PowerOn_InitClock(void);

//时钟走时处理，一般在最高优先级中断中调用 
void ClockRun(const TTimeRun * pTimeRun);

//获取当前系统时间
TFullDateTime GetSystemFullTime(void);

//设置BM共享时间
void SetBMSharedDateTime(TFullDateTime dt);

//获取Linux共享的时间
BOOL GetLinuxSharedDateTime(TFullDateTime * pdt);

BOOL SetSystemFullTime(const TFullDateTime * pdt);
BOOL SetSystemTime2(int y, int mon, int d, int h, int min, int s, int us);

//设置日期 - 主要用于命令行设时间
BOOL SetDate(int y, int mon, int d);
//设置时间 - 主要用于命令行设时间
BOOL SetTime(int h, int min, int s);

#endif /* SRC_H_BAREMETAL_PLT_BM_TIME_H_ */
