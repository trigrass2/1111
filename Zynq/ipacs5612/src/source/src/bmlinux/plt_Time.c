/*
 * @Author: zhongwei
 * @Date: 2019-11-19 13:39:11
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 17:38:12
 * @Description: 时间处理
 * @FilePath: \ZynqBM\src\include\plt_Time.c
 */
#include "plt_inc_c.h"
#include "plt_Time.h"

#ifdef _PROT_UNIT_
//#include "plt_bm_time.h"
#endif

// One-based array of days in year at month start
SECTION_PLT_DATA
static const int16 _MonthDays[13] =
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

//00~68每年的第一天0:0:0与2000年比所过的秒数(为加快从秒计算出年的时间定义的数组)
/*
      超过2068年后的秒数已经不够存放到int32类型中，会造成数据越界
*/
//SECTION_PLT_DATA
//static const int32 _YearSeconds[] = {
//        0, 31622400, 63158400, 94694400, 126230400,                 //2000 ~ 2004
//        157852800, 189388800, 220924800, 252460800, 284083200,      //2005 ~ 2009
//        315619200, 347155200, 378691200, 410313600, 441849600,      //2010 ~ 2014
//        473385600, 504921600, 536544000, 568080000, 599616000,      //2015 ~ 2019
//        631152000, 662774400, 694310400, 725846400, 757382400,      //2020 ~ 2024
//        789004800, 820540800, 852076800, 883612800, 915235200,      //2025 ~ 2029
//        946771200, 978307200, 1009843200, 1041465600, 1073001600,   //2030 ~ 2034
//        1104537600, 1136073600, 1167696000, 1199232000, 1230768000, //2035 ~ 2039
//        1262304000, 1293926400, 1325462400, 1356998400, 1388534400, //2040 ~ 2044
//        1420156800, 1451692800, 1483228800, 1514764800, 1546387200, //2045 ~ 2049
//        1577923200, 1609459200, 1640995200, 1672617600, 1704153600, //2050 ~ 2054
//        1735689600, 1767225600, 1798848000, 1830384000, 1861920000, //2055 ~ 2059
//        1893456000, 1925078400, 1956614400, 1988150400, 2019686400, //2060 ~ 2064
//        2051308800, 2082844800, 2114380800, 2145916800,             //2065 ~ 2068
//};

//判断是否是闰年
/*
闰年有两种情况，设年份为year，则：

（1）当year是400的整倍数时为闰年，条件表示为：

year%400= =0

（2）当year是4的整倍数，但不是100的整倍数时为闰年，条件为：

    year%4= =0 && year%100 != 0
*/
#define IsLeapYear(nYear) ((((nYear) & 3) == 0) && (((nYear) % 100) != 0 || ((nYear) % 400) == 0))

//计算某年某月有多少天
#define GetDaysInMonth(nYear, nMon) (_MonthDays[nMon] - _MonthDays[(nMon)-1] + (((nMon) == 2 && IsLeapYear(nYear)) ? 1 : 0))

//时间增加一秒
void Time_AddSecond(TFullDateTime * pNow)
{
    uint16 nDaysInMonth;
    pNow->s++;
    if(pNow->s >= 60)
    {
        pNow->s = 0;
        pNow->min++;
        if(pNow->min >= 60)
        {
            pNow->min = 0;
            pNow->h++;
            if(pNow->h >= 24)
            {
                pNow->h = 0;
                pNow->d++;
                
                //计算当前月有多少天
                nDaysInMonth =GetDaysInMonth(pNow->y, pNow->mon);
                
                if(pNow->d > nDaysInMonth)  //超过一个月
                {
                    if(pNow->mon==12)
                    {
                        //下一年
                        pNow->d = 1;
                        pNow->mon = 1;
                        pNow->y++;
                    }
                    else
                    {
                        pNow->d = 1;
                        pNow->mon++;
                    }
                }
            }
        }
    }       
}

///////////////////////////////////////////////////////////////
//	函 数 名 : _SecondsFromTime
//	函数功能 : 计算dt所对应时间离2000.1.1有多少秒
//	处理过程 :
//	备    注 :
//	作    者 : 仲伟
//	时    间 : 2006年1月6日
//	返 回 值 : int32
//	参数说明 : TDateTime dt
///////////////////////////////////////////////////////////////
SECTION_PLT_CODE
static int32 _SecondsFromTime(const TFullDateTime *dt)
{
    //计算从2000.1.1到现在有多少天, 2000.1.1为1
    int32 nDate = 365L*(dt->y-2000) + (dt->y-2000)/4 - (dt->y-2000)/100 + (dt->y-2000)/400 +
        _MonthDays[dt->mon-1] + dt->d;
    
    //  If leap year and it's before March, subtract 1:
    if (dt->mon <= 2 && IsLeapYear(dt->y))
        --nDate;

    //计算多少秒
    return (nDate*86400L+3600L*dt->h+60L*dt->min+dt->s);
}

///////////////////////////////////////////////////////////////
///	\brief     : 从秒得到当前完整时间日期
///	\param int32 nS
///	\return TFullDateTime
///
///	\author    : 仲伟
///	\date      : 2006年12月31日
///	\note      :
///////////////////////////////////////////////////////////////
SECTION_PLT_CODE
static TFullDateTime _TimeFromSeconds(uint32 nS)
{
    TFullDateTime dt={2000, 1, 1, 0, 0, 0, 0};
    uint32 nYDays;      //一年多少天
    uint32 nYSecs;      //一天多少秒
    uint32 nMDays;

    while (1)
    {
        //计算本年有多少天，闰年加一天
        nYDays = 365 + (IsLeapYear(dt.y)?1:0);
        nYSecs = nYDays * 86400L;
        if(nYSecs > nS)     //大于剩余秒数
        {
            //时间在本年内
            break;
        }
        else
        {
            nS -=  nYSecs;  //时间进位到下一年
            dt.y ++;
        }
    }

    //计算日期
    nYDays = nS / 86400L;   //本年第几天
    
    for (dt.mon=1;dt.mon<=12;dt.mon++)
    {
        //计算一个月几天
        nMDays = _MonthDays[dt.mon]-_MonthDays[dt.mon-1];
        if(dt.mon == 2 && IsLeapYear(dt.y))
        {
            nMDays ++;
        }
        if(nYDays>=nMDays)
        {
            nYDays -= nMDays;
            //进入下一个月
        }
        else
        {
            //时间就在本月
            break;
        }
    }
    
    dt.d = 1+nYDays;    //日

    //计算时间
    nS %= 86400L;
    dt.h = nS / 3600L;      //时
    nS %= 3600L;
    dt.min = nS / 60;       //分
    dt.s = nS % 60;         //秒
    
    return dt;
}

///////////////////////////////////////////////////////////////
///	\brief     : 将精简的时间转换为完成的时间日期
///	\param const TDateTime * pdt
///	\return TFullDateTime
///
///	\author    : 仲伟
///	\date      : 2007年3月3日
///	\note      :
///////////////////////////////////////////////////////////////
SECTION_PLT_CODE
TFullDateTime ToFullDateTime(const TDateTime *pdt)
{
    TFullDateTime fdt = _TimeFromSeconds(pdt->s); //转换年月日，时分秒
    fdt.us = pdt->us;                             //直接复制微秒信息
    return fdt;
}

///////////////////////////////////////////////////////////////
///	\brief     : 将完整的时间日期转换为精简的时间
///	\param const TFullDateTime * pdt
///	\return TDateTime
///
///	\author    : 仲伟
///	\date      : 2007年3月3日
///	\note      :
///////////////////////////////////////////////////////////////
SECTION_PLT_CODE
TDateTime ToDateTime(const TFullDateTime *pdt)
{
    TDateTime dt;
    dt.s = _SecondsFromTime(pdt); //计算秒
    dt.us = pdt->us;

    return dt;
}

/**
 * @Function: ToMonthDay
 * @Description: B码时间转换未TFullDateTime
 * @author zhongwei (2020/4/3 14:15:53)
 * 
 * @return TFullDateTime 
 * 
*/
SECTION_PLT_CODE TFullDateTime IRIG_to_FullDateTime(uint8 year, uint16 day, uint8 hour, uint8 minute, uint8 second)
{
    TFullDateTime fdt;
    fdt.y = 2000 + year;
    fdt.mon = 0;
    fdt.d = 0;
    //day计算，1月1日对应的day=1
    if (day > 0)
    {
        --day;

        for (int i = 0; i < 11; i++)
        {
            int d1 = _MonthDays[i];
            int d2 = _MonthDays[i+1];
            if (day >= d1 && day < d2)
            {
                fdt.mon = i + 1;
                fdt.d = day - d1 + 1;
            }
            else if (i == 10)   //最后一个月
            {
                fdt.mon = 12;
                fdt.d = day - d2 + 1;
            }
        }
    }

    fdt.h = hour;
    fdt.min = minute;
    fdt.s = second;
    fdt.us = 0;

    return fdt;
}

///////////////////////////////////////////////////////////////
//	函 数 名 : CheckFullTime
//	函数功能 : 检查时间是否合法，这里的时间指完整时间，包括年月日时分秒
//	处理过程 :
//	备    注 :
//	作    者 : 仲伟
//	时    间 : 2005年9月3日
//	返 回 值 : uint16
//	参数说明 : TFullDateTime * pTime
//	注意：合理的年的范围是2000 ~ 2049年
///////////////////////////////////////////////////////////////
SECTION_PLT_CODE
BOOL CheckFullTime(const TFullDateTime *pTime)
{
    // Validate year and month (ignore day of week and milliseconds)
    if (pTime->y > 2049 || pTime->y<2000 || pTime->mon < 1 || pTime->mon > 12)
        return PLT_FALSE;

    // Finish validating the date
    if (pTime->d < 1 || pTime->d > GetDaysInMonth(pTime->y, pTime->mon) ||
        pTime->h > 23 || pTime->min > 59 ||
        pTime->s > 60 /*考虑闰秒*/ || pTime->us > 999999)
    {
        return PLT_FALSE;
    }

    return PLT_TRUE;
}

SECTION_PLT_CODE
BOOL CheckTime(const TDateTime *pTime)
{
    if (pTime->us > 999999)
    {
        return PLT_FALSE;
    }

    return PLT_TRUE;
}

/******************************************
 * 时间转成字符串输出格式
*******************************************/
// YYYY-MM-DD hh:mm:ss.ms
SECTION_PLT_CODE
const char *str_GetFullDateTimeString(const TFullDateTime *pDateTime, c64_t ch)
{
    E_SNPRINTF(ch,sizeof(c64_t), "%04u-%02u-%02u %02u:%02u:%02u.%03u", pDateTime->y, pDateTime->mon,
              pDateTime->d, pDateTime->h, pDateTime->min, pDateTime->s, us_to_ms(pDateTime->us));
    return ch;
}

// YYYY-MM-DD hh:mm:ss.us
SECTION_PLT_CODE
const char *str_GetFullDateTimeWithUsString(const TFullDateTime *pDateTime, c64_t ch)
{
    E_SNPRINTF(ch,sizeof(c64_t), "%04u-%02u-%02u %02u:%02u:%02u.%06u", pDateTime->y, pDateTime->mon,
              pDateTime->d, pDateTime->h, pDateTime->min, pDateTime->s, pDateTime->us);

    return ch;
}

// YYYY-MM-DD hh:mm:ss 无毫秒时间
SECTION_PLT_CODE
const char *str_GetFullDateTimeWithoutMsString(const TFullDateTime *pDateTime, c64_t ch)
{
    E_SNPRINTF(ch,sizeof(c64_t), "%04u-%02u-%02u %02u:%02u:%02u", pDateTime->y, pDateTime->mon,
              pDateTime->d, pDateTime->h, pDateTime->min, pDateTime->s);

    return ch;
}

// YY-MM-DD hh:mm:ss.ms 年YY
SECTION_PLT_CODE
const char *str_GetFullDateTimeShortYearString(const TFullDateTime *pDateTime, c64_t ch)
{
    E_SNPRINTF(ch,sizeof(c64_t), "%02u-%02u-%02u %02u:%02u:%02u.%03u", pDateTime->y % 100, pDateTime->mon,
              pDateTime->d, pDateTime->h, pDateTime->min, pDateTime->s, us_to_ms(pDateTime->us));

    return ch;
}

// YYYY-MM-DD hh:mm:ss.ms
SECTION_PLT_CODE
const char *str_GetDateTimeString(const TDateTime *pDateTime, c64_t ch)
{
    TFullDateTime fdt = ToFullDateTime(pDateTime);
    return str_GetFullDateTimeString(&fdt, ch);
}

// YYYY-MM-DD hh:mm:ss.us
SECTION_PLT_CODE
const char *str_GetDateTimeWithUsString(const TDateTime *pDateTime, c64_t ch)
{
    TFullDateTime fdt = ToFullDateTime(pDateTime);
    return str_GetFullDateTimeWithUsString(&fdt, ch);
}

// YYYY-MM-DD hh:mm:ss 无毫秒时间
SECTION_PLT_CODE
const char *str_GetDateTimeWithoutMsString(const TDateTime *pDateTime, c64_t ch)
{
    TFullDateTime fdt = ToFullDateTime(pDateTime);
    return str_GetFullDateTimeWithoutMsString(&fdt, ch);
}

// YY-MM-DD hh:mm:ss.ms 年YY
SECTION_PLT_CODE
const char *str_GetDateTimeShortYearString(const TDateTime *pDateTime, c64_t ch)
{
    TFullDateTime fdt = ToFullDateTime(pDateTime);
    return str_GetFullDateTimeShortYearString(&fdt, ch);
}

/************************************************************************/
//  当前时间的访问接口
//
/************************************************************************/
SECTION_PLT_CODE TFullDateTime i_GetCurrentTime(void)
{
    extern TFullDateTime GetSystemFullTime(void);
    return GetSystemFullTime();
}

SECTION_PLT_CODE TDateTime i_GetCurrentShortTime(void)
{
    TFullDateTime fdt = i_GetCurrentTime();
    return ToDateTime(&fdt);
}

SECTION_PLT_CODE TFullDateTime ParseDateTimeString(const char * strDate, const char * strTime)
{
    /** 获取编译时间 **/
    const char * pMon[12]={
            "Jan",
            "Feb",
            "Mar",
            "Apr",
            "May",
            "Jun",
            "Jul",
            "Aug",
            "Sep",
            "Oct",
            "Nov",
            "Dec"
    };
    char chtmp[20];
    TFullDateTime dt;
    int i;
    char *t;
    
    // strDate  格式为 "mm dd yyyy"
    E_STRCPY(chtmp, strDate);
    //首先检索月字符串
    t = E_STRTOK(chtmp, " ");   
    dt.mon=0;
    for (i=0;i<12;i++) {                //月的处理比较特殊一些，需要查找字符串
        if(strcmp(t, pMon[i])==0)
        {
            dt.mon=i+1;
            break;
        }
    }
    //检索日字符串
    t = E_STRTOK(NULL, " ");
    dt.d = E_ATOI(t);

    //检索年字符串
    t = E_STRTOK(NULL, " ");
    dt.y = E_ATOI(t);

    //strTime 格式为 "hh:mm:ss"
    E_STRCPY(chtmp, strTime);
    //首先检索时字符串
    t = E_STRTOK(chtmp, ":");   
    dt.h=E_ATOI(t);
    //检索分字符串
    t = E_STRTOK(NULL, ":");    
    dt.min = E_ATOI(t);

    //检索秒字符串
    t = E_STRTOK(NULL, ":");    
    dt.s = E_ATOI(t);

    //E_SSCANF(strDate, "%s %d %d", chMon, &dt.d, &dt.y);
    //E_SSCANF(strTime, "%d:%d:%d", &dt.h, &dt.min, &dt.s);

    

    dt.us=0;

    return dt;
}

SECTION_PLT_CODE TFullDateTime From_time_t(time_t t)
{
    TFullDateTime fdt;
    struct tm *newtime;
    newtime = localtime( &t );

    fdt.y = newtime->tm_year+1900;
    fdt.mon = newtime->tm_mon+1;
    fdt.d = newtime->tm_mday;
    fdt.h = newtime->tm_hour;
    fdt.min = newtime->tm_min;
    fdt.s = newtime->tm_sec;
    fdt.us = 0;

    return fdt;
}

SECTION_PLT_CODE time_t To_Time_t(TFullDateTime fdt)
{
    struct tm ptm;
                
    ptm.tm_year  = fdt.y - 1900;
    ptm.tm_mon   = fdt.mon-1;
    ptm.tm_mday  = fdt.d;
    ptm.tm_hour  = fdt.h;
    ptm.tm_min   = fdt.min;
    ptm.tm_sec   = fdt.s;
                
    return mktime(&ptm);
}

//libpub版本
SECTION_PLT_CODE const char * get_version_libpub(void)
{
    return VERSION_PLATFORM;
}

//编译时间
SECTION_PLT_CODE TFullDateTime get_buildtime_libpub(void)
{
    return ParseDateTimeString(__DATE__, __TIME__);
}

//打印libpub信息
SECTION_PLT_CODE void Print_libpub(void)
{
    c64_t ch;
    PRINTF("Libpub Version : %s\n\r", get_version_libpub());
    TFullDateTime fdt = get_buildtime_libpub();
    PRINTF("Build Time    : %s\n\r", str_GetFullDateTimeWithoutMsString(&fdt, ch));
}

