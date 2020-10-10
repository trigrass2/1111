/*
 * @Author: zhongwei
 * @Date: 2019-11-19 09:57:40
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 10:54:44
 * @Description: TRACE/ASSERT/LOGMSG功能实现
 * @FilePath: \ZynqBM\src\include\plt_trace.c
 */

#include "plt_inc_c.h"
#include "plt_trace.h"

#include <stdio.h>

#ifdef _MANG_UNIT_
#include <syslog.h>     //log for linux
#endif

//仅DEBUG模式下置TRUE定义
#ifdef _DEBUG
#define TRUE_ON_DEBUG   PLT_TRUE
#else
#define TRUE_ON_DEBUG   PLT_FALSE
#endif

SECTION_PLT_DATA
BOOL TRACE_L1_ENABLE = PLT_TRUE;
SECTION_PLT_DATA
BOOL TRACE_L2_ENABLE = PLT_TRUE;
SECTION_PLT_DATA
BOOL TRACE_L3_ENABLE = TRUE_ON_DEBUG;
SECTION_PLT_DATA
BOOL TRACE_L4_ENABLE = TRUE_ON_DEBUG;

SECTION_PLT_DATA
BOOL bShellTrace = PLT_FALSE;       //是否shell trace

SECTION_PLT_DATA
BOOL ASSERT_L1_ENABLE = PLT_TRUE;
SECTION_PLT_DATA
BOOL ASSERT_L2_ENABLE = PLT_TRUE;
SECTION_PLT_DATA
BOOL ASSERT_L3_ENABLE = TRUE_ON_DEBUG;
SECTION_PLT_DATA
BOOL ASSERT_L4_ENABLE = TRUE_ON_DEBUG;

SECTION_PLT_DATA
BOOL LOGMSG_L1_ENABLE = PLT_TRUE;
SECTION_PLT_DATA
BOOL LOGMSG_L2_ENABLE = PLT_TRUE;
SECTION_PLT_DATA
BOOL LOGMSG_L3_ENABLE = TRUE_ON_DEBUG;
SECTION_PLT_DATA
BOOL LOGMSG_L4_ENABLE = TRUE_ON_DEBUG;

SECTION_PLT_DATA
BOOL PRINTF_L1_ENABLE = PLT_TRUE;
SECTION_PLT_DATA
BOOL PRINTF_L2_ENABLE = PLT_TRUE;
SECTION_PLT_DATA
BOOL PRINTF_L3_ENABLE = TRUE_ON_DEBUG;
SECTION_PLT_DATA
BOOL PRINTF_L4_ENABLE = TRUE_ON_DEBUG;

/**
 * syslog的.log文件位置
 */
#ifdef _MANG_UNIT_

int g_syslog_high = APP_LOG_HIGH;       //高优先级消息
int g_syslog_low = APP_LOG_LOW;         //低优先级消息
int g_syslog_bm = APP_LOG_BM;           //bm消息
int g_syslog_printf = LOG_INFO | APP_LOG_LOW;

int set_syslog_pri(int pri)
{
    int old_pri = g_syslog_printf;
    g_syslog_printf = pri;
    return old_pri;
}

void restore_syslog_pri(int pri)
{
    g_syslog_printf = pri;
}


#endif  //_MANG_UNIT_

SECTION_PLT_CODE
void TraceEnable_Set(BOOL L1, BOOL L2, BOOL L3, BOOL L4)
{
    TRACE_L1_ENABLE = BoolFrom(L1);
    TRACE_L2_ENABLE = BoolFrom(L2);
    TRACE_L3_ENABLE = BoolFrom(L3);
    TRACE_L4_ENABLE = BoolFrom(L4);
}

SECTION_PLT_CODE
void AssertEnable_Set(BOOL L1, BOOL L2, BOOL L3, BOOL L4)
{
    ASSERT_L1_ENABLE = BoolFrom(L1);
    ASSERT_L2_ENABLE = BoolFrom(L2);
    ASSERT_L3_ENABLE = BoolFrom(L3);
    ASSERT_L4_ENABLE = BoolFrom(L4);
}

SECTION_PLT_CODE
void LogmsgEnable_Set(BOOL L1, BOOL L2, BOOL L3, BOOL L4)
{
    LOGMSG_L1_ENABLE = BoolFrom(L1);
    LOGMSG_L2_ENABLE = BoolFrom(L2);
    LOGMSG_L3_ENABLE = BoolFrom(L3);
    LOGMSG_L4_ENABLE = BoolFrom(L4);
}

SECTION_PLT_CODE
void PrintfEnable_Set(BOOL L1, BOOL L2, BOOL L3, BOOL L4)
{
    PRINTF_L1_ENABLE = BoolFrom(L1);
    PRINTF_L2_ENABLE = BoolFrom(L2);
    PRINTF_L3_ENABLE = BoolFrom(L3);
    PRINTF_L4_ENABLE = BoolFrom(L4);
}

//短文件名，不含路径的文件名
SECTION_PLT_CODE
const char * _fileshortname(const char * filename)
{
    int i;
    for (i=(int)E_STRLEN(filename)-1;i>=0;i--) {
        if(filename[i]=='/' || filename[i]=='\\' )
        {
            return &filename[i+1];
        }
    }

    return filename;
}

SECTION_PLT_DATA
static c128_t _last_assert_filename;
SECTION_PLT_DATA
static int _last_assert_line = -1;

SECTION_PLT_CODE
void x_assert(const char * msg, const char * filename, int line)
{
    if(line != _last_assert_line || E_STRCMP(filename, _last_assert_filename)!=0)       //只纪录第一次断言
    {
        TSOE_TRACE_STRUCT NewSoe;
        NewSoe.dtTime=Now();
        NewSoe.line_num=line;
        E_STRNCPY(NewSoe.filename, _fileshortname(filename), MAX_TRACE_FILENAME_LEN);
        NewSoe.nType=TRACE_TYPE_ERR;
        E_STRNCPY(NewSoe.desc, msg, MAX_TRACE_DESC_LEN);
        
        extern void soe_AddNewTraceSoe(const TSOE_TRACE_STRUCT * pSoeNew);
        soe_AddNewTraceSoe(&NewSoe);

#ifdef _MANG_UNIT_
        int old_pri = set_syslog_pri(LOG_ALERT | g_syslog_high);
#endif
        PRINTF_WITH_LOG("==>> ASSERT [%s:%d] - %s\n\r", NewSoe.filename, NewSoe.line_num, NewSoe.desc);

#ifdef _MANG_UNIT_
        restore_syslog_pri(old_pri);
#endif

        //保存本次数据
        _last_assert_line = line;
        E_STRNCPY(_last_assert_filename, filename, sizeof(_last_assert_filename));
    }

}

SECTION_PLT_CODE
void x_assert_bool(const char * msg, const BOOL * pBool, const char * filename, int line)
{
    if(line != _last_assert_line || E_STRCMP(filename, _last_assert_filename)!=0)       //只纪录第一次断言
    {
        TSOE_TRACE_STRUCT NewSoe;
        NewSoe.dtTime=Now();
        NewSoe.line_num=line;
        E_STRNCPY(NewSoe.filename, _fileshortname(filename), MAX_TRACE_FILENAME_LEN);
        NewSoe.nType=TRACE_TYPE_ERR;
        E_SNPRINTF(NewSoe.desc, MAX_TRACE_DESC_LEN, "%s [0x%x=%d]", msg, (uint32)pBool, *pBool);
        
        extern void soe_AddNewTraceSoe(const TSOE_TRACE_STRUCT * pSoeNew);
        soe_AddNewTraceSoe(&NewSoe);

#ifdef _MANG_UNIT_
        int old_pri = set_syslog_pri(LOG_ALERT | g_syslog_high);
#endif

        PRINTF_WITH_LOG("==>> ASSERT [%s:%d] - %s", NewSoe.filename, NewSoe.line_num, NewSoe.desc);

#ifdef _MANG_UNIT_
        restore_syslog_pri(old_pri);
#endif

        //保存本次数据
        _last_assert_line = line;
        E_STRNCPY(_last_assert_filename, filename, sizeof(_last_assert_filename));
    }
}

#define PRINTF_BUFFER_COUNT 512

SECTION_PLT_CODE
void x_trace(const char * filename, int line, const char * format, ...)
{
    char szBuf[PRINTF_BUFFER_COUNT];

    TSOE_TRACE_STRUCT NewSoe;
    va_list args;
    va_start(args, format);
    E_VSNPRINTF(szBuf,PRINTF_BUFFER_COUNT, format, args);
    va_end(args);

    //生成SOE
    NewSoe.dtTime=Now();
    NewSoe.line_num=line;
    E_STRNCPY(NewSoe.filename, _fileshortname(filename), MAX_TRACE_FILENAME_LEN);
    NewSoe.nType=TRACE_TYPE_MSG;
    E_STRNCPY(NewSoe.desc, szBuf, MAX_TRACE_DESC_LEN);
    
    extern void soe_AddNewTraceSoe(const TSOE_TRACE_STRUCT * pSoeNew);
    soe_AddNewTraceSoe(&NewSoe);

    if (!IsTrue(bShellTrace))
    {
#ifdef _MANG_UNIT_
        int old_pri = set_syslog_pri(LOG_INFO | g_syslog_high);
#endif
        PRINTF_WITH_LOG("==>> TRACE [%s:%d] - %s\n\r", NewSoe.filename, NewSoe.line_num, NewSoe.desc);
#ifdef _MANG_UNIT_
        restore_syslog_pri(old_pri);
#endif
    }
}

SECTION_PLT_CODE
void x_printf(const char * format, ...)
{
    char szBuf[PRINTF_BUFFER_COUNT];
    extern void Print_To_Linux(const char * str);
    extern void print( const char *ptr);

    va_list args;
    va_start(args, format);
    E_VSNPRINTF(szBuf,PRINTF_BUFFER_COUNT, format, args);
    va_end(args);

    //输出到终端
#ifdef _PROT_UNIT_
    print(szBuf);
#else
    printf(szBuf);
#endif

#ifdef _PROT_UNIT_
    //打印输出到Linux核
    Print_To_Linux(szBuf);      //调用应用的接口
#endif  //_PROT_UNIT_
}

SECTION_PLT_CODE
void x_printf_with_log(const char * format, ...)
{
    char szBuf[PRINTF_BUFFER_COUNT];
    extern void Print_To_Linux(const char * str);
    extern void print( const char *ptr);

    va_list args;
    va_start(args, format);
    E_VSNPRINTF(szBuf,PRINTF_BUFFER_COUNT, format, args);
    va_end(args);

    //输出到终端
#ifdef _PROT_UNIT_
    print(szBuf);
#else
    printf(szBuf);
#endif

#ifdef _PROT_UNIT_
    //打印输出到Linux核
    Print_To_Linux(szBuf);      //调用应用的接口
#endif  //_PROT_UNIT_

#ifdef _MANG_UNIT_
    syslog(g_syslog_printf, szBuf);   //log to linux
#endif
}

/************************************************************************/
//  LOGMSG接口
//      LOGMSG实现类似vxWorks的logMsg功能，最多支持6个整型参数
/************************************************************************/
#define MAX_LOGMSG_BUF_CNT  128         //最多缓冲128个logmsg信息
typedef struct TLOGMSG_ITEM{
    const char * format;
    int a1;
    int a2;
    int a3;
    int a4;
    int a5;
    int a6;
}TLOGMSG_ITEM;

typedef struct TLOGMSG_STRUCT{
    int begin;                                      //起始指针
    int end;                                        //结束指针
    TLOGMSG_ITEM buf_items[MAX_LOGMSG_BUF_CNT];     //循环存储
}TLOGMSG_STRUCT;

SECTION_PLT_DATA static TLOGMSG_STRUCT _logmsg_struct;

//初始化
SECTION_PLT_CODE void init_logmsg(void)
{
    E_MEMSET(&_logmsg_struct, 0, sizeof(_logmsg_struct));
}

/**
 * @Function: x_logmsg
 * @Description: 仿造vxworks logMsg的函数接口 
 * @author zhongwei (2019/11/26 14:59:31)
 * 
 * @param format 格式字符串
 * @param a1 参数，最多支持6个 a1~a6
 * @param a2 
 * @param a3 
 * @param a4 
 * @param a5 
 * @param a6 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE
void x_logmsg(const char * format, int a1, int a2, int a3, int a4, int a5, int a6)
{
    if (format != NULL)
    {
#ifdef _PROT_UNIT_
//        CPU_SR_ALLOC();
#endif
        TLOGMSG_ITEM it;
        it.format = format;
        it.a1 = a1;
        it.a2 = a2;
        it.a3 = a3;
        it.a4 = a4;
        it.a5 = a5;
        it.a6 = a6;
#ifdef _PROT_UNIT_
//        CPU_CRITICAL_ENTER();
#endif
        _logmsg_struct.buf_items[_logmsg_struct.end++] = it;
        if (_logmsg_struct.end >= MAX_LOGMSG_BUF_CNT)
        {
            _logmsg_struct.end = 0;
        }
#ifdef _PROT_UNIT_
//        CPU_CRITICAL_EXIT();
#endif
    }
}

//任务中处理
SECTION_PLT_CODE void logmsg_tsk_handle(void)
{
    int per_cnt = 20;   //每次最多处理20条
    int end = _logmsg_struct.end;
    int begin = _logmsg_struct.begin;
    if (end >=0 && end < MAX_LOGMSG_BUF_CNT &&
        begin >=0 && begin < MAX_LOGMSG_BUF_CNT)
    {
        while (begin != end && per_cnt > 0)
        {
            const TLOGMSG_ITEM * pItem = &_logmsg_struct.buf_items[begin];
            if (pItem->format != NULL)
            {
                per_cnt--;
                PRINTF_WITH_LOG(pItem->format, pItem->a1, pItem->a2, pItem->a3, pItem->a4, pItem->a5, pItem->a6);
                //c128_t ch;
                //E_SNPRINTF(ch, sizeof(c128_t), pItem->format, pItem->a1, pItem->a2, pItem->a3, pItem->a4, pItem->a5, pItem->a6);
                //print(ch);
            }
            begin++;
            if (begin >= MAX_LOGMSG_BUF_CNT)
            {
                begin = 0;
            }
        }

        _logmsg_struct.begin = begin;
    }
    else    //清空
    {
        _logmsg_struct.end = _logmsg_struct.begin = 0;
    }
}

/************************************************************************/
//  BM调试命令打印 LOGMSG IF
/************************************************************************/
SECTION_PLT_DATA uint32 g_DEBUG_MESSAGE[MAX_DEBUG_MESSAGE_UINT32_CNT] = {0,0,0,0};
#define MAX_DEBUG_MESSAGE_ID (MAX_DEBUG_MESSAGE_UINT32_CNT * 32)
const char ** g_DEBUG_MESSAGE_str=NULL;

SECTION_PLT_CODE void SetDebugMessageStr(const char ** str_arr )
{
    g_DEBUG_MESSAGE_str = str_arr;
}

//显示的当前DEBUG_MESSAGE数据
SECTION_PLT_CODE void ShowDebugMessage(void)
{
    int i;
    PRINTF("DEBUG_MESSAGE = 0x%08x %08x %08x %08x\n\r", DEBUG_MESSAGE[0], DEBUG_MESSAGE[1], DEBUG_MESSAGE[2], DEBUG_MESSAGE[3]);
    if (g_DEBUG_MESSAGE_str != NULL)
    {
        for (i = 0; i < MAX_DEBUG_MESSAGE_ID; i++)
        {
            const char * str = g_DEBUG_MESSAGE_str[i];
            if(str == NULL) //最后一个总是nullptr
            {
                break;
            }
            if(IsDebugMessage(i))
            {
                PRINTF("  %4d  --  %s\n\r", i, str);
            }
        }
    }
}

SECTION_PLT_CODE void SetDebugMessage(int32 nDebugMessage)
{ 
    if(nDebugMessage == 0)
    {
        E_MEMSET(DEBUG_MESSAGE, 0, sizeof(DEBUG_MESSAGE));
    }
    else if(nDebugMessage > 0 && nDebugMessage < MAX_DEBUG_MESSAGE_ID)
    {
        DEBUG_MESSAGE[nDebugMessage/32] |= 1<<(nDebugMessage%32);       //置调试输出
    } 

    ShowDebugMessage();
}

SECTION_PLT_CODE void ClrDebugMessage(int32 nDebugMessage)
{
    if(nDebugMessage > 0 && nDebugMessage < MAX_DEBUG_MESSAGE_ID)
    {
        DEBUG_MESSAGE[nDebugMessage/32] &= ~(1<<(nDebugMessage%32));
    }
    
    ShowDebugMessage();
}

SECTION_PLT_CODE void DebugMessageHelp(void)
{
    int i;
    PRINTF("Use < SetDebugMessage/ClrDebugMessage > to change LOGMSG or PRINTF output info.\n\r");
    PRINTF("  < SetDebugMessage 0 > to clear all output info.\n\r");
    PRINTF("  Parmeters:\n\r");
    if (g_DEBUG_MESSAGE_str != NULL)
    {
        for (i = 0; i < MAX_DEBUG_MESSAGE_ID; i++)
        {
            const char * str = g_DEBUG_MESSAGE_str[i];
            if(str == NULL) //最后一个总是nullptr
            {
                break;
            }
            PRINTF("  %4d  --  %s\n\r", i, str);
        }
    }
}
