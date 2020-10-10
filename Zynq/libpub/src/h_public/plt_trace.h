/*
 * @Author: zhongwei
 * @Date: 2019-11-19 09:57:31
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-21 10:56:31
 * @Description: TRACE/ASSERT/LOGMSG功能实现
 * @FilePath: \ZynqBM\src\include\plt_trace.h
 */
#ifndef SRC_INCLUDE_PLT_TRACE_H_
#define SRC_INCLUDE_PLT_TRACE_H_

//#include "xil_printf.h"

void x_assert(const char *, const char *, int);
void x_assert_bool(const char * msg, const BOOL * pBool, const char * filename, int line);
void x_trace(const char *, int, const char *, ...);
void x_printf(const char * format, ...);
void x_printf_with_log(const char * format, ...);

void x_logmsg(const char * format, int a1, int a2, int a3, int a4, int a5, int a6);
void init_logmsg(void);         //初始化
void logmsg_tsk_handle(void);   //任务中调用，打印输出logmsg信息

#if (TRACE_ENABLE==1)
#   define TRACE(format,...) (x_trace(__FILE__, __LINE__, format, ## __VA_ARGS__))
#else
#   define TRACE(...)   ((void)0)
#endif  //TRACE_ENABLE 

#if (ASSERT_ENABLE==1)
#   define ASSERT(e) ((e) ? (void)0 : x_assert(#e, __FILE__, __LINE__))
#   define ASSERT_BOOL(b) ((((b)==PLT_TRUE) || ((b)==PLT_FALSE)) ? (void)0 : x_assert_bool("[B]"#b, (const BOOL *)&(b), __FILE__, __LINE__))
#else
#   define ASSERT(...)  ((void)0)
#   define ASSERT_BOOL(...)  ((void)0)
#endif  //ASSERT_ENABLE 

#define VERIFY(e) ((e) ? (void)0 : x_assert(#e, __FILE__, __LINE__))

//异步打印到控制台
#if (LOGMSG_ENABLE==1)
#   define LOGMSG       x_logmsg
#else
#   define LOGMSG(...)  ((void)0)
#endif  //LOGMSG_ENABLE 
        
//阻塞式打印到控制台
#if (PRINTF_ENABLE==1)
#   define PRINTF(format, ...) x_printf(format, ## __VA_ARGS__)
#else
#   define PRINTF(...)  ((void)0)
#endif  //PRINTF_ENABLE 

//阻塞式打印到控制台
#if defined(_MANG_UNIT_)
#   define PRINTF_WITH_LOG(format, ...) x_printf_with_log(format, ## __VA_ARGS__)
#else
#   define PRINTF_WITH_LOG PRINTF
#endif  //PRINTF_ENABLE 

extern BOOL TRACE_L1_ENABLE;
extern BOOL TRACE_L2_ENABLE;
extern BOOL TRACE_L3_ENABLE;
extern BOOL TRACE_L4_ENABLE;

extern BOOL ASSERT_L1_ENABLE;
extern BOOL ASSERT_L2_ENABLE;
extern BOOL ASSERT_L3_ENABLE;
extern BOOL ASSERT_L4_ENABLE;

extern BOOL LOGMSG_L1_ENABLE;
extern BOOL LOGMSG_L2_ENABLE;
extern BOOL LOGMSG_L3_ENABLE;
extern BOOL LOGMSG_L4_ENABLE;

extern BOOL PRINTF_L1_ENABLE;
extern BOOL PRINTF_L2_ENABLE;
extern BOOL PRINTF_L3_ENABLE;
extern BOOL PRINTF_L4_ENABLE;

extern BOOL bShellTrace;       //是否shell trace

/**
 * syslog的.log文件位置
 */
#ifdef _MANG_UNIT_

extern int g_syslog_high;       //高优先级消息
extern int g_syslog_low;        //低优先级消息
extern int g_syslog_bm;         //bm消息
                                //
extern int g_syslog_printf;     //默认printf的log pri   LOG_INFO | APP_LOG_LOW;

#endif  //_MANG_UNIT_

/*************** TRACE 分级 ********************/
#define TRACE_L1(format,...)    if(IsTrue(TRACE_L1_ENABLE)){TRACE(format,## __VA_ARGS__);}
#define TRACE_L2(format,...)    if(IsTrue(TRACE_L2_ENABLE)){TRACE(format,## __VA_ARGS__);}
#define TRACE_L3(format,...)    if(IsTrue(TRACE_L3_ENABLE)){TRACE(format,## __VA_ARGS__);}
#define TRACE_L4(format,...)    if(IsTrue(TRACE_L4_ENABLE)){TRACE(format,## __VA_ARGS__);}

#define SHELL_TRACE(format, ...) \
                            bShellTrace = PLT_TRUE;             \
                            TRACE(format,## __VA_ARGS__);       \
                            bShellTrace = PLT_FALSE;

/*************** ASSERT 分级 ********************/
/*
    由于是ASSERT，所以即使LEVEL不支持，但是依然输出ASSERT信息，以方便调试
    只是为了节省代码空间，而缩短字符串长度，不输出完整的信息，只输出异常LEVEL和行号，4级除外
*/
#define ASSERT_L1(e)        if(IsTrue(ASSERT_L1_ENABLE)){ASSERT(e);}
#define ASSERT_L2(e)        if(IsTrue(ASSERT_L2_ENABLE)){ASSERT(e);}
#define ASSERT_L3(e)        if(IsTrue(ASSERT_L3_ENABLE)){ASSERT(e);}
#define ASSERT_L4(e)        if(IsTrue(ASSERT_L4_ENABLE)){ASSERT(e);}

#define ASSERT_BOOL_L1(e)        if(IsTrue(ASSERT_L1_ENABLE)){ASSERT_BOOL(e);}
#define ASSERT_BOOL_L2(e)        if(IsTrue(ASSERT_L2_ENABLE)){ASSERT_BOOL(e);}
#define ASSERT_BOOL_L3(e)        if(IsTrue(ASSERT_L3_ENABLE)){ASSERT_BOOL(e);}
#define ASSERT_BOOL_L4(e)        if(IsTrue(ASSERT_L4_ENABLE)){ASSERT_BOOL(e);}

/*************** PRINTF 分级 ********************/
#define PRINTF_L1(format, ...)  if(IsTrue(PRINTF_L1_ENABLE)){PRINTF(format, ## __VA_ARGS__);}
#define PRINTF_L2(format, ...)  if(IsTrue(PRINTF_L2_ENABLE)){PRINTF(format, ## __VA_ARGS__);}
#define PRINTF_L3(format, ...)  if(IsTrue(PRINTF_L3_ENABLE)){PRINTF(format, ## __VA_ARGS__);}
#define PRINTF_L4(format, ...)  if(IsTrue(PRINTF_L4_ENABLE)){PRINTF(format, ## __VA_ARGS__);}

/*************** PRINTF_WHIT_LOG 分级 ********************/
#define PRINTF_WHIT_LOG_L1(format, ...)  if(IsTrue(PRINTF_L1_ENABLE)){PRINTF_WHIT_LOG(format, ## __VA_ARGS__);}
#define PRINTF_WHIT_LOG_L2(format, ...)  if(IsTrue(PRINTF_L2_ENABLE)){PRINTF_WHIT_LOG(format, ## __VA_ARGS__);}
#define PRINTF_WHIT_LOG_L3(format, ...)  if(IsTrue(PRINTF_L3_ENABLE)){PRINTF_WHIT_LOG(format, ## __VA_ARGS__);}
#define PRINTF_WHIT_LOG_L4(format, ...)  if(IsTrue(PRINTF_L4_ENABLE)){PRINTF_WHIT_LOG(format, ## __VA_ARGS__);}

/*************** LOGMSG 分级 ********************/
#define LOGMSG_L1(format, a1, a2, a3, a4, a5, a6)   if(IsTrue(LOGMSG_L1_ENABLE)){LOGMSG(format, a1, a2, a3, a4, a5, a6);}
#define LOGMSG_L2(format, a1, a2, a3, a4, a5, a6)   if(IsTrue(LOGMSG_L2_ENABLE)){LOGMSG(format, a1, a2, a3, a4, a5, a6);}
#define LOGMSG_L3(format, a1, a2, a3, a4, a5, a6)   if(IsTrue(LOGMSG_L3_ENABLE)){LOGMSG(format, a1, a2, a3, a4, a5, a6);}
#define LOGMSG_L4(format, a1, a2, a3, a4, a5, a6)   if(IsTrue(LOGMSG_L4_ENABLE)){LOGMSG(format, a1, a2, a3, a4, a5, a6);}


/************************************************************************/
//  BM调试命令打印 LOGMSG IF
//
/************************************************************************/
#define MAX_DEBUG_MESSAGE_UINT32_CNT 4
extern uint32 g_DEBUG_MESSAGE[];
#define DEBUG_MESSAGE   g_DEBUG_MESSAGE

void SetDebugMessage(int32 nDebugMessage);
void ClrDebugMessage(int32 nDebugMessage);
#define IsDebugMessage(nDebugMessage)       ((DEBUG_MESSAGE[nDebugMessage>>5]>>(nDebugMessage&0x1f)) & 0x01)

//显示的当前DEBUG_MESSAGE数据
void ShowDebugMessage(void);


#define _DEBUG_MSG(format, a1, a2, a3, a4, a5, a6)                                      \
                                    if(IsDebugMessage(DEBUG_MESSAGE_USE_PRINTF))        \
                                    {PRINTF(format, a1, a2, a3, a4, a5, a6);} else      \
                                    {LOGMSG(format, a1, a2, a3, a4, a5, a6);}

#define DEBUG_MSG(nDbg,format, a1, a2, a3, a4, a5, a6)      if(IsDebugMessage(nDbg)) {_DEBUG_MSG(format, a1, a2, a3, a4, a5, a6);}

#endif /* SRC_INCLUDE_PLT_TRACE_H_ */
