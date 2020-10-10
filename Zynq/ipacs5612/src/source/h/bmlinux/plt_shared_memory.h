/*
 * plt_shared_memory.h
 *
 *  Created on: 2020年2月10日
 *      Author: power
 */

#ifndef SRC_H_PUBLIC_PLT_SHARED_MEMORY_H_
#define SRC_H_PUBLIC_PLT_SHARED_MEMORY_H_

#ifdef _PROT_UNIT_
     //共享内存 BM与Linux
    extern TSHARE_MEMORY_STRUCT g_SharedMemory; 
     //获取共享内存指针
    #define GetSharedMemory()   (&g_SharedMemory)
#else
    extern TSHARE_MEMORY_STRUCT * g_pSharedMemory;

    //获取共享内存指针
    #define GetSharedMemory()   (g_pSharedMemory)
#endif  //_PROT_UNIT_

//获取BM共享内存指针
#define GetSharedBMMemory() (&GetSharedMemory()->bm_shared)

//获取Linux共享内存指针
#define GetSharedLinuxMemory()  (&GetSharedMemory()->linux_shared)

//获取Print打印输出到Linux的共享内存指针
#define GetSharedPrintMemory()  (&GetSharedBMMemory()->print_struct)

//获取BM发送的Linux的报文缓冲区指针
#define GetSharedBMInnerComm()  (&GetSharedBMMemory()->inner_comm_struct)

//获取BM共享给Linux的时间信息
#define GetSharedBMTime()       (&GetSharedBMMemory()->shared_time)

//获取BM共享给Linux的内建数据配置
#define GetSharedBMInnerConfig()  (&GetSharedBMMemory()->inner_bm_config)

//获取Linux发送的BM的报文缓冲区指针
#define GetSharedLinuxInnerComm()   (&GetSharedLinuxMemory()->inner_comm_struct)

//获取Linux共享给BM的时间信息
#define GetSharedLinuxTime()       (&GetSharedLinuxMemory()->shared_time)

//获取Linux共享给BM的命令行
#define GetSharedLinuxCommandLine()       (&GetSharedLinuxMemory()->command_line)

//获取Linux配置数据
#define GetSharedLinuxConfigData()        (&GetSharedLinuxMemory()->config_data)

#define IsSharedMemoryValid()   ((GetSharedMemory() != NULL)?PLT_TRUE:PLT_FALSE)

//BM共享内存是否有效
#define IsSharedBMMemValid()    ((GetSharedMemory() != NULL && GetSharedBMMemory()->flag_begin[0] == FLAG_SHARED_MEMORY_VALID && GetSharedBMMemory()->struct_version == VERSION_SHARED_MEMORY)?PLT_TRUE:PLT_FALSE)

//Linux共享内存是否有效
#define IsSharedLinuxMemValid() ((GetSharedMemory() != NULL && GetSharedLinuxMemory()->flag_begin[0] == FLAG_SHARED_MEMORY_VALID  && GetSharedLinuxMemory()->struct_version == VERSION_SHARED_MEMORY)?PLT_TRUE:PLT_FALSE)

//获取BM遥测
#define GetSharedBMYC(wEqpID)       (GetSharedBMMemory()->Bm_dbYcValue[wEqpID])

//获取BM单点遥信
#define GetSharedBMSYX(wEqpID)       (GetSharedBMMemory()->Bm_SyxValue[wEqpID])

//获取BM实时值
#define GetSharedDiValue()       (GetSharedBMMemory()->Bm_DiValue)

//获取BM双点遥信
#define GetSharedBMDYX(wEqpID)       (GetSharedBMMemory()->Bm_DyxValue[wEqpID])

//获取BM soe
#define GetSharedBMSoe()       (GetSharedBMMemory()->BmSoebuf)

//获取共享铁电
#define GetSharedNvRam()        (GetSharedMemory()->nvram)

//获取BmRecord                
#define GetSharedReCord()       (&(GetSharedBMMemory()->BmRecord))

//获取BmEvent            
#define GetSharedEvent()       (&(GetSharedBMMemory()->BmEvent))

//检查Baremetal共享内存
BOOL CheckSharedBaremetalMemory(void);

// Linux共享内存检查  TSHARED_LINUX_STRUCT
BOOL CheckSharedLinuxMemory(void);

//共享内存检查
BOOL CheckSharedMemory(void);

//在BM启动成功后，置复位标志 该接口提供给BM
void SetBaremetalResetFlag(int step);
//获取Baremetal复位标识，该接口提供给Linux
uint32 GetBaremetalResetFlag(int step);
void ClrBaremetalResetFlag(void);

//设置Baremetal启动时等待标识，该接口提供给Linux
void SetBaremetalWaitFlag(int step);
//获取BM等待标志，该接口提供给BM
uint32 GetBaremetalWaitFlag(int step);
//清除BM启动等待标志，接口提供给Linux
void ClrBaremetalWaitFlag(void);

//读取BM内部通讯版本
int32 GetBaremetalInnerCommVerion(void);
//读取Linux内部通讯版本
int32 GetLinuxInnerCommVerion(void);

#endif /* SRC_H_PUBLIC_PLT_SHARED_MEMORY_H_ */
