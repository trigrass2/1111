/*
 * @Author: zhongwei
 * @Date: 2019/11/26 15:35:44
 * @Description: 测试功能
 * @File: plt_test.c
 *
*/
#include "plt_include.h"
#include <math.h>
#include "xil_mmu.h"

#define TEST_ENABLE

#ifdef TEST_ENABLE

void test_fp32_cycle()
{
    fp32 f_result=2.1;
    fp32 f_a = 1.1;
    int i=0;
    uint32 c_begin = hd_get_cycles();
    uint32 c_end;

    while ( i < 2000)
    {
        i++;
        f_result = f_a * f_result;
    }

    c_end = hd_get_cycles();

    PRINTF("test_fp32_cycle=%d result=%f \n\r",(c_end-c_begin), f_result);
}

void test_fp64_cycle()
{
    fp64 f_result=2.1;
    fp64 f_a = 1.1;
    int i=0;
    uint32 c_begin = hd_get_cycles();
    uint32 c_end;

    while ( i < 2000)
    {
        i++;
        f_result = f_a * f_result;
    }

    c_end = hd_get_cycles();

    PRINTF("test_fp64_cycle=%d result=%f \n\r",(c_end-c_begin), (fp32)f_result);
}

void test_int32_cycle()
{
    int32 f_result=2;
    int32 f_a = 3;
    int i=0;
    uint32 c_begin = hd_get_cycles();
    uint32 c_end;

    while ( i < 2000)
    {
        i++;
        f_result = f_a * f_result;
    }

    c_end = hd_get_cycles();

    PRINTF("test_int32_cycle=%d result=%d \n\r",(c_end-c_begin), f_result);
}

void test_int64_cycle()
{
    int64 f_result=2;
    int64 f_a = 3;
    int i=0;
    uint32 c_begin = hd_get_cycles();
    uint32 c_end;

    while ( i < 2000)
    {
        i++;
        f_result = f_a * f_result;
    }

    c_end = hd_get_cycles();

    PRINTF("test_int64_cycle=%d result=%d \n\r",(c_end-c_begin), (int32)f_result);
}

void test_sqrt_cycle()
{
    fp64 f_result=2.1;
    fp64 f_a = 1.1;
    int i=0;
    uint32 c_begin = hd_get_cycles();
    uint32 c_end;

    while ( i < 2000)
    {
        i++;
        f_result = sqrt(f_a+1.0);
    }

    c_end = hd_get_cycles();

    PRINTF("test_sqrt_cycle=%d result=%f \n\r",(c_end-c_begin), (fp32)f_result);
}

void test_mmu()
{
    //常识读无效内存空间
    volatile uint32 * ptr = (volatile uint32 *)0x100000;    //1M位置
    uint32 nocache_attrib = 0;
    //NORM_NONCACHE = 0x11DE2  S=b1 TEX=b001 AP=b11, Domain=b1111, C=b0, B=b0
    nocache_attrib |= ALT_MMU_TTB1_TYPE_SET(0x02);  //type总是2
    nocache_attrib |= ALT_MMU_TTB1_SECTION_B_SET(0x00);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_C_SET(0x00);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_TEX_SET(0x01);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_AP_SET(ALT_MMU_AP_FULL_ACCESS);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_DOMAIN_SET(ALT_MMU_DAP_MANAGER);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_S_SET(ALT_MMU_TTB_S_SHAREABLE);

    Xil_SetTlbAttributes((uint32)ptr, nocache_attrib);

    PRINTF("NORM_WB_CACHE = %08x\n\r",nocache_attrib);
    PRINTF("read test, ptr = 0x%x, val = 0x%x\n\r", (uint32)ptr, *ptr);
    *ptr = 0x11111111;
    PRINTF("write test, ptr = 0x%x, val = 0x%x\n\r", (uint32)ptr, *ptr);
    *ptr = 0;

    /*
        Read Only配置失败，不能配置readonly
    */
    //READ ONLY = 0x11DE2  S=b1 TEX=b001 AP=b11, Domain=b1111, C=b0, B=b0
    nocache_attrib = 0;
    nocache_attrib |= ALT_MMU_TTB1_TYPE_SET(0x02);  //type总是2
    nocache_attrib |= ALT_MMU_TTB1_SECTION_B_SET(0x00);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_C_SET(0x00);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_TEX_SET(0/*0x01*/);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_AP_SET(ALT_MMU_DAP_NO_ACCESS /*ALT_MMU_AP_READ_ONLY*/);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_DOMAIN_SET(ALT_MMU_DAP_NO_ACCESS /*ALT_MMU_DAP_CLIENT*/);
    nocache_attrib |= ALT_MMU_TTB1_SECTION_S_SET(ALT_MMU_TTB_S_NON_SHAREABLE);

    Xil_SetTlbAttributes((uint32)ptr, nocache_attrib);

    PRINTF("READ_ONLY = %08x\n\r",nocache_attrib);
    PRINTF("read test, ptr = 0x%x, val = 0x%x\n\r", (uint32)ptr, (uint32)(*ptr));
    *ptr = 0x11111111;
    PRINTF("write test, ptr = 0x%x, val = 0x%x\n\r", (uint32)ptr, (uint32)(*ptr));
}

BOOL bTest[128];
int8 i8Test[54];
uint8 u8Test[32];

//void dump_regs()
//{
//    int eax = 0;
//    int ebx = 0;
//    int ecx = 0;
//    int edx = 0;
//
//    int esi = 0;
//    int edi = 0;
//    int ebp = 0;
//    int esp = 0;
//
//    int cf = 0;
//    int sf = 0;
//    int zf = 0;
//    int of = 0;
//
//    int set = 1; // -52(%ebp)
//
//    __asm__ __volatile__(
//    "movl %eax, sp"
//    );
//
//    printf("EAX = %#08x\tEBX = %#08x\tECX = %#08x\tEDX = %#08x\n",eax,ebx,ecx,edx);
//    printf("ESI = %#08x\tEDI = %#08x\tEBP = %#08x\tESP = %#08x\n",esi,edi,ebp,esp);
//    printf("CF = %d\tSF = %d\tZF = %d\tOF = %d\n",cf,sf,zf,of);
//}

//typedef fp64 (*func_fp64)(void);
//typedef int32 (*func_int32)(void);
//typedef TVAR_DATA (*func_var)(void);
//
//fp64 test_fp64(void)
//{
//    return 123.456;
//}
//
//int32 test_int32(void)
//{
//    return 12000000;
//}
//
//TVAR_DATA test_func_var(void)
//{
//    TVAR_DATA var = {VAR_DATA_TYPE_INT64, {.i_64 = 23}};
//    return var;
//}

//void test_func(void)
//{
//    func_int32 func = (func_int32)test_fp64;
//    fp64 fp = func();
//    func = (func_int32)test_func_var;
//    TVAR_DATA var = (TVAR_DATA)func();
//    c64_t ch;
//    PRINTF(str_GetVariableValueString(&fp, VARREG_TYPE_fp64));
//    PRINTF("\n\r");
//    PRINTF(str_GetVariableValueString(&var, VARREG_TYPE_TVAR_DATA));
//    PRINTF("\n\r");
//}

void test_var_data(void)
{
    TVAR_DATA var;
    var.type = VARREG_TYPE_int64;
    var.data.i_32 = 0xFFFFFFFF;

    PRINTF("0x%llx\n\r", var.data.i_64);
}

/*
STATUS axi_goose_send_msg(uint32 send_port,uint8 *msg, uint32 msgLen);

void test_goose1(void)
{
		axi_goose_send_msg(0x02, goose_send, sizeof(goose_send));
		axi_goose_send_msg(0x03, goose_send, sizeof(goose_send));
		axi_goose_send_msg(0x04, goose_send, sizeof(goose_send));
}*/

void test_sv_rcv(void)
{
    //AD通道接收配置
    axi_hp_set_sampling_msg0_interval_config(0, 0xFFFF, 0, SAMPLING_SRC_TYPE_AD, SAMPLING_SRC_AD_OR_SV_0);
    axi_hp_set_sampling_msg0_interval_config(1, 0xFFFF, 0, SAMPLING_SRC_TYPE_AD, SAMPLING_SRC_AD_OR_SV_1);
    axi_hp_set_sampling_msg0_control(SAMPLING_RATE_120SPC, 2, 0xFFFF);   
}

void my_test(void)
{
    PRINTF(">>>>>> my_test \n\r");
//    test_goose_rcv();
    test_sv_rcv();
//  test_int32_cycle();
//  test_int64_cycle();
//  test_fp32_cycle();
//  test_fp64_cycle();
//  test_sqrt_cycle();
//
//  test_mmu();

    //test_register_var();
//  test_func();

    //test_var_data();
}

#endif  //TEST_ENABLE
