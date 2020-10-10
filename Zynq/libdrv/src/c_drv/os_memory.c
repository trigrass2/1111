#include "plt_inc_c.h"
#include "os_memory.h"
#include "os_cpu_a.h"
#include "xil_types.h"

#define MAX_MEMORG_BLOCK    4   //支持的内存区块数量（4个）

typedef struct  
{
    unsigned int nOrgBegin;     //原始位置
    unsigned int nBegin;        //开始位置
    size_t nLength;             //长度
}TMEMORY_BLOCK;

typedef struct  
{
    TMEMORY_BLOCK MemBlock[MAX_MEMORG_BLOCK];   //内存区块
    int nBlockCount;

    size_t nTotalMemory;                //总内存
    size_t nTotalAlloced;               //已分配内存
}TMEMORY_MANAGER;
 
//dlmalloc初始内存大小
#define DLMALLOC_INIT_MEM_SIZE                  0x400000        //4MB

SECTION_PLT_DATA static TMEMORY_MANAGER mem_Manager;

SECTION_DYN_MEMORY static unsigned char _DynMemory[DLMALLOC_INIT_MEM_SIZE];

SECTION_PLT_CODE size_t mem_GetAllocedMemory(void)
{
    return mem_Manager.nTotalAlloced;
}

SECTION_PLT_CODE size_t mem_GetTotalMemory(void)
{
    return mem_Manager.nTotalMemory;
}

SECTION_PLT_CODE void mem_Initial(void) //初始化
{
    E_MEMSET(&mem_Manager, 0, sizeof(mem_Manager));
    E_MEMSET(_DynMemory, 0, sizeof(_DynMemory));

    mem_AddMemBlock((unsigned int)_DynMemory, sizeof(_DynMemory));
}

/**
 * @Function: mem_AddMemBlock
 * @Description: 添加可用动态内存块 
 * @author zhongwei (2019/11/26 11:31:45)
 * 
 * @param nBegin 内存起始地址
 * @param nLength 内存长度，字节
 * 
 * @return SECTION_PLT_CODE unsigned short 
 * 
*/
SECTION_PLT_CODE unsigned short mem_AddMemBlock(unsigned int nBegin, size_t nLength)
{
    int i;
    unsigned char bExt= 0;

    //拼接内存
    for (i=0;i<mem_Manager.nBlockCount;i++)
    {
        //地址连续，并且类型一致
        if((mem_Manager.MemBlock[i].nBegin + mem_Manager.MemBlock[i].nLength) == nBegin)
        {
            mem_Manager.MemBlock[i].nLength += nLength;     //原有区块向后延伸
            bExt = 1;
            break;
        }
        else if((nBegin + nLength) == mem_Manager.MemBlock[i].nBegin)
        {
            mem_Manager.MemBlock[i].nBegin = nBegin;
            mem_Manager.MemBlock[i].nLength += nLength;     //原有区块向前延伸
            mem_Manager.MemBlock[i].nOrgBegin = nBegin;
            bExt = 1;
            break;
        }
    }
    
    if(bExt==0) //不是延长区块，则新增一个区块
    {
        if(mem_Manager.nBlockCount >= MAX_MEMORG_BLOCK)
        {
            return 0;
        }
        
        mem_Manager.MemBlock[mem_Manager.nBlockCount].nBegin = nBegin;
        mem_Manager.MemBlock[mem_Manager.nBlockCount].nLength = nLength;
        mem_Manager.MemBlock[mem_Manager.nBlockCount].nOrgBegin = nBegin;
        mem_Manager.nBlockCount++;
    }
    
    mem_Manager.nTotalMemory += nLength;

    return 1;

}

/**
 * @Function: mem_Malloc
 * @Description: 分配内存，分配出的内存总是8字节对齐的
 * @author zhongwei (2019/11/26 11:33:54)
 *  
 * @param nSize 分配内存长度
 * 
 * @return void* 
 * 
*/
SECTION_PLT_CODE void * mem_Malloc(size_t nSize)
{
    CPU_SR_ALLOC();

    unsigned int nBegin = 0;
    int i;
    size_t nNeedLen = 8 + nSize;        //预留8字节用于字节对其

    for (i = 0; i < mem_Manager.nBlockCount; i++)
    {
        CPU_CRITICAL_ENTER();
        if (mem_Manager.MemBlock[i].nLength >= nNeedLen)
        {
            nBegin = mem_Manager.MemBlock[i].nBegin;
            mem_Manager.MemBlock[i].nBegin += nNeedLen;
            mem_Manager.MemBlock[i].nLength -= nNeedLen;

            mem_Manager.nTotalMemory += nNeedLen;
        }
        CPU_CRITICAL_EXIT();

        if (nBegin != 0) //分配到内存
        {
            break;
        }
    }

    if(nBegin == 0)
    {
        return NULL;
    }
    else
    {
        mem_Manager.nTotalAlloced += nNeedLen;

        // 重新处理字节对其 总是4字节对其
        if((nBegin % 8) != 0)
        {
            nBegin = nBegin + (8 - (nBegin % 8));
        }

        return (void *)nBegin;
    }
}

//打印输出动态内存分配信息
SECTION_PLT_CODE void Print_Mem_Malloc(void)
{
    int i;
    PRINTF("Total : %d\n\r", mem_GetTotalMemory());
    PRINTF("Alloced : %d\n\r", mem_GetAllocedMemory());
    for (i = 0; i < mem_Manager.nBlockCount; i++)
    {
        PRINTF("Block[0x%08x] ( 0x%08x - %d)\n\r",mem_Manager.MemBlock[i].nOrgBegin, mem_Manager.MemBlock[i].nBegin, mem_Manager.MemBlock[i].nLength);
    }
}

//打印输出MMU表
SECTION_PLT_CODE void Print_MMUTable(void)
{
    uint32 sec;
    const uint32 nSectorSize = 0x100000U;     //每个MMU的空间大小, 1M字节
    const uint32 nSectorCnt = 0x1000;         //4096
    uint32 last_mmu_val = 0xffffffff;
    extern u32 MMUTable;
    const u32 *ptr = &MMUTable;
    uint8 bN = 0;

    PRINTF("MMU Table - addr = 0x%08x, count=%d\n\r", (uint32)ptr, nSectorCnt);

    for (sec = 0; sec < nSectorCnt; sec++)
    {
        uint32 nMMU = ptr[sec] & 0xFFFFFU;
        if (nMMU != last_mmu_val)
        {
            PRINTF("  %08x : %08x\n\r", (sec * nSectorSize), nMMU);
            bN = 1;
            last_mmu_val = nMMU;
        }
        else if (bN)
        {
            PRINTF("  ...\n\r");
            bN = FALSE;
        }
    }

}

