/*
 * @Author: zhongwei
 * @Date: 2019/11/27 13:58:43
 * @Description: 内存存储
 * @File: plt_ram.c
 *
*/

#include "plt_include.h"
#include "plt_ram.h"
#include "xil_mmu.h"



/**
 * @Description:
 *             .ld文件中定义的不开cache的共享内存地址空间
 * @author zhongwei (2019/11/27 18:02:20)
 * 
*/
/*
  DDR内存空间分配
    DDR总空间 512M，即0x20000000
    其中：
      0x00000000 ~ 0x38000000 : Linux内存 896M
      0x38000000 ~ 0x3F000000 : 裸核内存  112M
      0x3F000000 ~ 0x40000000 : 共享内存和FPGA使用的内存 16M
*/
//extern  int DDR_START;                            //DDR起始地址
//extern  int DDR_SIZE;                         //DDR大小
extern  int _CORE1_DDR_START_ADDR;              //CORE1使用的DDR内存空间起始地址
extern  int _CORE1_DDR_LENGTH;                  //CORE1使用的DDR内存空间长度
extern  int _CORE1_DDR_SHART_START_ADDR;        //共享内存起始地址
extern  int _CORE1_DDR_SHARE_LENGTH;            //共享内存长度



#define RAM_MMU_BLOCK_SIZE  0x100000U   //共享内存Block的大小

/**
 * @Function: PowerOn_Init_MMU
 * @Description: 上电初始化MMU 
 * @author zhongwei (2019/11/28 9:06:33)
 * 
 * @param void 
 * 
 * @return SECTION_PLT_CODE void 
 *
 * MMU的配置见文档
 * 《UG585 3.2.5》
 *  translation_table.s
 * 《DDI0388G_cortex_a9_ARM Technical Reference Manual_r3p0_trm 第6节》
 * 《DDI0406C_C_arm_architecture_reference_manual B3.5.1/D15.6.3》
 *
 * 几个典型MMU参数的定义：
 *  NORM_NONCACHE       0x11DE2         S=b1 TEX=b001 AP=b11, Domain=b1111, C=b0, B=b0      Full Access/Outer and Inner non-cacheable
 *  STRONG_ORDERED      0xC02           S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b0         Full Access/Strongly-ordered
 *  DEVICE_MEMORY       0xC06           S=b0 TEX=b000 AP=b11, Domain=b0, C=b0, B=b1         Full Access/shareable device
 *  RESERVED            0x0             S=b0 TEX=b000 AP=b00, Domain=b0, C=b0, B=b0         No access
 *  NORM_WB_CACHE       0x15DE6         S=b1 TEX=b101 AP=b11, Domain=b1111, C=b0, B=b1      Full Access/Write-back, write-allocate
 *  NORM_WT_CACHE       0x16DEA         S=b1 TEX=b110 AP=b11, Domain=b1111, C=b1, B=b0      Full Access/Write-through, no write-allocate
 *  EXECUTE_NEVER       0x11            XN=b1                                               Excute Never
 *
 *  其他：
 *  APX = b1 AP=b10，Read Only，测试不成功，不能设置只读内存
 *
 *  可以见plt_hd_mmu.h文件中相关说明，该文件从altera EDS16.1 hwlib中复制过来
*/
SECTION_PLT_CODE void PowerOn_Init_MMU(void)
{
    //0x00000000 ~ 0x38000000 : Linux内存 896M 禁止访问
    {
        int32 ddr_start_addr = 0;
        int32 core1_ddr_start_addr = (int32)&_CORE1_DDR_START_ADDR;
        uint32 core0_ddr_len = core1_ddr_start_addr - ddr_start_addr;
        int32 mmu_block_cnt = core0_ddr_len / RAM_MMU_BLOCK_SIZE;
        int i;

        PRINTF("[MMU]Core0 Memory = 0x%x len=0x%x\n\r", ddr_start_addr, core0_ddr_len);

        //必须是1M的整数倍
        ASSERT_L1((ddr_start_addr % RAM_MMU_BLOCK_SIZE)== 0);
        ASSERT_L1((core0_ddr_len % RAM_MMU_BLOCK_SIZE)== 0);

        //初始化Core0内存的MMU，Core1应该禁止访问
        for (i = 0; i < mmu_block_cnt; i++)
        {
            INTPTR ptr = ddr_start_addr + (RAM_MMU_BLOCK_SIZE * i);
            Xil_SetTlbAttributes(ptr, RESERVED);
            //PRINTF("[MMU]Disable Memory at %08x - %08x\n\r", ptr, ptr + RAM_MMU_BLOCK_SIZE);
        }
    }

    //0x3F000000 ~ 0x40000000 : 共享内存和FPGA使用的内存 16M 关闭cache
    {
        int32 shared_addr = (int32)&_CORE1_DDR_SHART_START_ADDR;
        uint32 shared_len = (int32)&_CORE1_DDR_SHARE_LENGTH;
        int32 mmu_block_cnt = shared_len / RAM_MMU_BLOCK_SIZE;
        int i;

        PRINTF("[MMU]Shared Memory = 0x%x len=0x%x\n\r", shared_addr, shared_len);

        //必须是1M的整数倍
        ASSERT_L1((shared_addr % RAM_MMU_BLOCK_SIZE)== 0);
        ASSERT_L1((shared_len % RAM_MMU_BLOCK_SIZE)== 0);

        //初始化共享内存的MMU，关闭共享内存的cache
        for (i = 0; i < mmu_block_cnt; i++)
        {
            INTPTR ptr = shared_addr + (RAM_MMU_BLOCK_SIZE * i);
            Xil_SetTlbAttributes(ptr, NORM_NONCACHE);
            //PRINTF("[MMU]Disable cache at %08x - %08x\n\r", ptr, ptr + RAM_MMU_BLOCK_SIZE);
        }
    }
}

/**
 * @Function: PowerOn_Init_SharedMemory
 * @Description: 上电初始化共享内存 
 * @author zhongwei (2019/11/27 14:03:53)
 * 
 * @param void 
 *
 * 共享内存TSHARE_MEMORY_STRUCT由Linux负责初始化
 * 
*/
SECTION_PLT_CODE void PowerOn_Init_SharedMemory(void)
{
    /*
        Baremetal侧共享内存初始化
        在BM，仅初始化BM管理的内存，即TSHARED_BAREMETAL_STRUCT部分
    */
    TSHARED_BAREMETAL_STRUCT *pShareBM = GetSharedBMMemory();

    //初始化BM共享内存
    if (IsFalse(CheckSharedBaremetalMemory()))      //BM共享内存未初始化时，才处理，否则不再初始化
    {
        int i;
        PRINTF_WITH_LOG("Shared baremetal memory initial.\n\r");
        E_MEMSET(pShareBM, 0, sizeof(TSHARED_BAREMETAL_STRUCT));
        pShareBM->struct_version = VERSION_SHARED_MEMORY;           //版本
        //E_STRNCPY(pShareBM->device_uuid, get_c_DEVICE_UUID_(_CURRENT_CPU_NUM_), sizeof(pShareBM->device_uuid));
        //E_STRNCPY(pShareBM->software_uuid, get_c_SOFTWARE_UUID_(_CURRENT_CPU_NUM_), sizeof(pShareBM->software_uuid));
        //pShareBM->reset_flag = FLAG_SHARED_MEMORY_VALID;            //置复位标志

        //PowerOn_InitBMInner_to_ShareMem();      //内建数据初始化，共享内建数据到Linux侧

        //有效标志初始化
        for (i=FLAG_SHARED_COUNT-1;i>=0;i--)
        {
            pShareBM->flag_end[i] = FLAG_SHARED_MEMORY_VALID;
            pShareBM->flag_begin[i] = FLAG_SHARED_MEMORY_VALID;
        }
    }
}
