/*
 * @Author: zhongwei
 * @Date: 2020/1/9 9:31:56
 * @Description: BM侧时间处理
 * @File: plt_bm_time.c
 *
*/

#include "plt_include.h"
#include "plt_bm_time.h"


SECTION_PLT_DATA TSystemTime g_SystemTime;

//上电时钟初始化
SECTION_PLT_CODE void PowerOn_InitClock(void)
{
    E_MEMSET(&g_SystemTime, 0, sizeof(g_SystemTime));
}

/**
 * @Function: ClockRun
 * @Description: 时钟走时处理，一般在最高优先级中断中调用 
 * @author zhongwei (2020/1/9 9:45:45)
 * 
 * @param pTimeRun 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void ClockRun(const TTimeRun * pTimeRun)
{
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();

    g_SystemTime.dt.us += pTimeRun->us;
    if (g_SystemTime.dt.us >= 1000000)
    {
        g_SystemTime.dt.us -= 1000000;
        Time_AddSecond(&g_SystemTime.dt);
    }
    CPU_CRITICAL_EXIT();

    //写时间到共享内存，时间一次写两遍，以便Linux侧取时间时能够正确操作
    SetBMSharedDateTime(g_SystemTime.dt);
}

SECTION_PLT_CODE TFullDateTime GetSystemFullTime(void)
{
    TFullDateTime fdt;
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    fdt = g_SystemTime.dt;
    CPU_CRITICAL_EXIT();
    return fdt;
}

//设置BM共享时间
SECTION_PLT_CODE void SetBMSharedDateTime(TFullDateTime dt)
{
    if (IsSharedBMMemValid())
    {
        TMC_SHARED_TIME *pSharedTime = GetSharedBMTime();
        pSharedTime->fdt1 = g_SystemTime.dt;
        pSharedTime->fdt2 = g_SystemTime.dt;
    }
}

//获取Linux共享的时间
SECTION_PLT_CODE BOOL GetLinuxSharedDateTime(TFullDateTime * pdt)
{
    if (IsSharedLinuxMemValid())
    {
        const TMC_SHARED_TIME * pLinuxTime = GetSharedLinuxTime();
        int i;
        for (i=0;i<3;i++)   //尝试三次
        {
            TFullDateTime dt1 = pLinuxTime->fdt1;
            TFullDateTime dt2 = pLinuxTime->fdt2;
            if (memcmp(&dt1, &dt2, sizeof(TFullDateTime)) == 0)
            {
                *pdt = dt1;
                return PLT_TRUE;
            }
        }
    }

    return PLT_FALSE;
}

//设置当前系统时间
SECTION_PLT_CODE BOOL SetSystemFullTime(const TFullDateTime * pdt)
{
    if(CheckFullTime(pdt))
    {
        CPU_SR_ALLOC();
        //直接修改当前系统时间
        CPU_CRITICAL_ENTER();
        g_SystemTime.dt = *pdt;
        CPU_CRITICAL_EXIT();

        return PLT_TRUE;
    }
    else
    {
        return PLT_FALSE;
    }
}

SECTION_PLT_CODE BOOL SetSystemTime2(int y, int mon, int d, int h, int min, int s, int us)
{
    TFullDateTime dt;
    dt.y = y;
    dt.mon = mon;
    dt.d = d;
    dt.h = h;
    dt.min = min;
    dt.s = s;
    dt.us = us;
    return SetSystemFullTime(&dt);
}

//设置日期 - 主要用于命令行设时间
SECTION_PLT_CODE BOOL SetDate(int y, int mon, int d)
{
    TFullDateTime dt = Now();
    dt.y = y;
    dt.mon = mon;
    dt.d = d;
    return SetSystemFullTime(&dt);
}

//设置时间 - 主要用于命令行设时间
SECTION_PLT_CODE BOOL SetTime(int h, int min, int s)
{
    TFullDateTime dt = Now();
    dt.h = h;
    dt.min = min;
    dt.s = s;
    return SetSystemFullTime(&dt);
}

