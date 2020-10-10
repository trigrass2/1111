/*
 * @Author: zhongwei
 * @Date: 2019/12/12 11:25:16
 * @Description: Linux启动core1裸核程序
 * @File: main.c
 *  
 * 调用方法： 
 *  
 * 在Linux命令行执行 
 *  
 * bmrun.elf ./zynqBM.bm 
 * 
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <byteswap.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>

#define CPU1_START_UP_REG           0xFFFFFFF0
#define CPU1_STARTADR               0x38000000      //core1加载地址
#define CPU1_CODE_LEN               0x1000000

//判断文件是否存在
int IfFileExist(const char* fileName) {
    struct stat file;
    if (stat((char*) fileName, &file) != 0) {
        return 0;
    }

    if (S_ISREG(file.st_mode)) {
        return 1;
    }

    return 0;
}

#define PRINTF_BUFFER_COUNT 128

void x_printf_with_log(const char * format, ...)
{
    char szBuf[PRINTF_BUFFER_COUNT];

    va_list args;
    va_start(args, format);
    vsnprintf(szBuf,PRINTF_BUFFER_COUNT, format, args);
    va_end(args);

    printf(szBuf);

    //syslog(LOG_INFO | APP_LOG_TMP | APP_LOG_LOW, szBuf);   //log to linux
}

#define PRINTF_WITH_LOG(format, ...) x_printf_with_log(format, ## __VA_ARGS__)

static int devmem_fd;

void *devm_map(unsigned long addr, int len) {
    off_t offset;
    void *map_base;

    if ((devmem_fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) {
        PRINTF_WITH_LOG("devm_map cannot open '/dev/mem'\n\r");
        goto err_open;
    }
    //PRINTF_WITH_LOG("/dev/mem opened.\n\r");

    /*
     * Map it
     */

    /* offset for mmap() must be page aligned */
    offset = addr & ~(sysconf(_SC_PAGE_SIZE) - 1);

    map_base = mmap(NULL, len + addr - offset, PROT_READ | PROT_WRITE,
            MAP_SHARED, devmem_fd, offset);
    if (map_base == MAP_FAILED) {
        PRINTF_WITH_LOG("devm_map mmap failed\n\r");
        goto err_mmap;
    }
    //PRINTF_WITH_LOG("Memory mapped at address %p.\n\r", map_base);

    return map_base + addr - offset;

    err_mmap: close(devmem_fd);

    err_open: return NULL;
}

void devm_unmap(void *virt_addr, int len) {
    unsigned long addr;

    if (devmem_fd == -1) {
        PRINTF_WITH_LOG("devm_unmap '/dev/mem' is closed\n\r");
        return;
    }

    /* page align */
    addr = (((unsigned long) virt_addr) & ~(sysconf(_SC_PAGE_SIZE) - 1));
    munmap((void *) addr, len + (unsigned long) virt_addr - addr);
    close(devmem_fd);
}

/* read & write a word */
uint32_t devmem_readl(unsigned int addr) {
    uint32_t val;
    void *virt_addr;

    virt_addr = devm_map(addr, 4);

    if (virt_addr == NULL) {
        PRINTF_WITH_LOG("devmem_readl 0x%x addr map failed\n\r", addr);
        return 0;
    }

    val = *(uint32_t *) virt_addr;

    devm_unmap(virt_addr, 4);

    return val;
}

void devmem_writel(unsigned int addr, uint32_t val) {
    void *virt_addr;

    virt_addr = devm_map(addr, 4);

    if (virt_addr == NULL) {
        PRINTF_WITH_LOG("devmem_writel 0x%x addr map failed\n\r", addr);
        return;
    }

    *(uint32_t *) virt_addr = val;

    devm_unmap(virt_addr, 4);
}

int LoadCpu1Image(int fd, const char * filename) {
    unsigned int *cpu1_load_ptr;
    unsigned char *buffer;

    FILE * core1_code;
    int load_length;
    int return_len;

    cpu1_load_ptr = (unsigned int *) mmap(NULL,
    CPU1_CODE_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
    CPU1_STARTADR);

    if (cpu1_load_ptr == MAP_FAILED) {
        PRINTF_WITH_LOG("mmap cpu1_load_ptr failed\n\r");
        return -1;
    } else {
        PRINTF_WITH_LOG("mmap cpu1_load_ptr success\n\r");
    }

    PRINTF_WITH_LOG("loading <%s>\n\r", filename);
    core1_code = fopen(filename, "rb");

    if (NULL == core1_code) {
        PRINTF_WITH_LOG("open <%s> failed!\n\r", filename);
        munmap(cpu1_load_ptr, CPU1_CODE_LEN);
        return -1;
    }
    fseek(core1_code, 0, SEEK_END);
    load_length = ftell(core1_code);

    if (CPU1_CODE_LEN <= load_length) {
        PRINTF_WITH_LOG("%s file len is larger than %d\n\r", filename, CPU1_CODE_LEN);
        munmap(cpu1_load_ptr, CPU1_CODE_LEN);
        return -1;
    }

    fseek(core1_code, 0, SEEK_SET);
    buffer = malloc(load_length);
    if (NULL == buffer) {
        PRINTF_WITH_LOG("malloc failed\n\r");
        munmap(cpu1_load_ptr, CPU1_CODE_LEN);
        return -1;
    }
    return_len = fread(buffer, 1, load_length, core1_code);
    if (return_len != load_length) {
        PRINTF_WITH_LOG("mmap return_len failed\n\r");
        free(buffer);
        munmap(cpu1_load_ptr, CPU1_CODE_LEN);
        return -1;
    } else {
        PRINTF_WITH_LOG("success load_length %d\n\r", return_len);
    }
    memcpy(cpu1_load_ptr, buffer, load_length);

    if (memcmp(cpu1_load_ptr, buffer, load_length)) {
        PRINTF_WITH_LOG("memcmp failed\n\r");
        free(buffer);
        munmap(cpu1_load_ptr, CPU1_CODE_LEN);
        return -1;
    }

    free(buffer);

    munmap(cpu1_load_ptr, CPU1_CODE_LEN);

    return (0);
}

void StartCpu1(void) {
    PRINTF_WITH_LOG("CPU0: writing startaddress for cpu1\n\r");
    devmem_writel(CPU1_START_UP_REG, CPU1_STARTADR);
}

/* Data Memory Barrier */
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")

void CPU1_WfePre() {
    //向0xFFFFFF00起始的地址写入WFE指令
    devmem_writel(0xFFFFFF00, 0xe3e0000f);
    devmem_writel(0xFFFFFF04, 0xe3a01000);
    devmem_writel(0xFFFFFF08, 0xe5801000);
    devmem_writel(0xFFFFFF0C, 0xe320f002);
    devmem_writel(0xFFFFFF10, 0xe5902000);
    devmem_writel(0xFFFFFF14, 0xe1520001);
    devmem_writel(0xFFFFFF18, 0x0afffffb);
    devmem_writel(0xFFFFFF1C, 0xe1a0f002);

    //向0x00写入跳转代码指向0xFFFFFF00
    devmem_writel(0x00000000, 0xe3e0f0ff);

    //清0xFFFFFFF0
    devmem_writel(0xFFFFFFF0, 0x00000000);
    dmb();
}

#define XSLCR_BASEADDR      0xF8000000U

#define A9_CPU_RST_CTRL     (XSLCR_BASEADDR + 0x244)
#define A9_RST1_MASK        0x00000002
#define A9_CLKSTOP1_MASK    0x00000020
#define CPU1_CATCH          0x00000024
#define XSLCR_LOCK_ADDR     (XSLCR_BASEADDR + 0x4)
#define XSLCR_LOCK_CODE     0x0000767B

#define APP_CPU1_ADDR       CPU1_STARTADR
#define XSLCR_UNLOCK_CODE   0x0000DF0D
#define XSLCR_UNLOCK_ADDR   (XSLCR_BASEADDR + 0x00000008)

void resetCPU1(void) {


    uint32_t RegVal;

    /* * Setup cpu1 catch address with starting address of app_cpu1. The FSBL initialized the vector table at 0x00000000
     * using a boot.S that checks for cpu number and jumps to the address stored at the
     * end of the vector table in cpu0_catch and cpu1_catch entries.
     * Note: Cache has been disabled at the beginning of main(). Otherwise
     * a cache flush would have to be issued after this write
     */
    devmem_writel(CPU1_CATCH, APP_CPU1_ADDR);

    /* Unlock the slcr register access lock */
    devmem_writel(XSLCR_UNLOCK_ADDR, XSLCR_UNLOCK_CODE);
    //  the user must stop the associated clock, de-assert the reset, and then restart the clock. During a
    //   system or POR reset, hardware automatically takes care of this. Therefore, a CPU cannot run the code
    //  that applies the software reset to itself. This reset needs to be applied by the other CPU or through
    //  JTAG or PL. Assuming the user wants to reset CPU1, the user must to set the following fields in the
    //    slcr.A9_CPU_RST_CTRL (address 0xF8000244) register in the order listed:
    //  1. A9_RST1 = 1 to assert reset to CPU0
    //   2. A9_CLKSTOP1 = 1 to stop clock to CPU0
    //  3. A9_RST1 = 0 to release reset to CPU0
    //    4. A9_CLKSTOP1 = 0 to restart clock to CPU0

    /* Assert and deassert cpu1 reset and clkstop using above sequence*/
#if 1

    RegVal = devmem_readl(A9_CPU_RST_CTRL);
	printf("%08x  \r\n",RegVal);
    RegVal |= A9_RST1_MASK;
    devmem_writel(A9_CPU_RST_CTRL, RegVal);
    RegVal |= A9_CLKSTOP1_MASK;
    devmem_writel(A9_CPU_RST_CTRL, RegVal);
    RegVal &= ~A9_RST1_MASK;
    devmem_writel(A9_CPU_RST_CTRL, RegVal);
    RegVal &= ~A9_CLKSTOP1_MASK;
    devmem_writel(A9_CPU_RST_CTRL, RegVal);
#endif
    /* lock the slcr register access */
    // devmem_writel(XSLCR_LOCK_ADDR, XSLCR_LOCK_CODE);


}

int main(int argc, char *argv[]) {
    //至少一个参数
    if (argc > 1) {
        const char * szBin = argv[1];
        if (IfFileExist(szBin)) {
            int fd;
            PRINTF_WITH_LOG("Bootup core1 from <%s>...\n\r", szBin);

            fd = open("/dev/mem", O_RDWR | O_SYNC);
            if (fd < 0) {
                PRINTF_WITH_LOG("open(/dev/mem) failed (%d)\n\r", errno);
                return 0;
            }

            if (LoadCpu1Image(fd, szBin) < 0) {
                PRINTF_WITH_LOG("LoadCpu1Image failed (%d)\n\r", errno);
                return 0;
            }

            CPU1_WfePre();

            resetCPU1();

            StartCpu1();

            PRINTF_WITH_LOG("Bootup core1 from <%s> success.\n\r", szBin);

            sleep(1);

            close(fd);

            return 1;
        } else {
            PRINTF_WITH_LOG("Bin file <%s> doesn't exist!\n\r", szBin);
        }
    } else {
        PRINTF_WITH_LOG("Please call {bmrun.elf zynqBM.bm} to bootup core1.\n\r");
    }

    return 0;
}
