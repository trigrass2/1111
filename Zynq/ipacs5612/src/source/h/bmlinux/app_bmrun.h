#ifndef APP_BMRUN_H_
#define APP_BMRUN_H_

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


#define PR_SET_SIZE             1024

//共享配置文件偏移
#define FILE_SYSTEM_OFFSET              0x00       
#define FILE_SYSTEM_LEN                (64*1024)   //sysem.cfg 64K
#define FILE_PROSETPUBTABLE_OFFSET     (FILE_SYSTEM_OFFSET + FILE_SYSTEM_LEN)  //64 
#define FILE_PROSETPUBTABLE_LEN        (8*1024) // ProSetPubTable.cfg 8K
#define FILE_PROSETTABLE_OFFSET        (FILE_PROSETPUBTABLE_OFFSET + FILE_PROSETPUBTABLE_LEN)   //72
#define FILE_PROSETTABLE_LEN           (8*1024)//prosettable.cfg     8k
#define FILE_FDPUBSET_OFFSET           (FILE_PROSETTABLE_OFFSET + FILE_PROSETTABLE_LEN)   //80 fdpubset   1K   最大18间隔
#define FILE_FDSET_OFFSET              (FILE_FDPUBSET_OFFSET + 18*PR_SET_SIZE)   //98 fdset      1K   最大18间隔
#define FILE_RUNPARA_OFFSET            (FILE_FDSET_OFFSET + 18*PR_SET_SIZE)      //116
#define FILE_RUNPARA_LEN               (8*1024)
#define FILE_SYSEXT_OFFSET             (FILE_RUNPARA_OFFSET + FILE_RUNPARA_LEN)      //116
#define FILE_SYSEXT_LEN                (8*1024)
#define FILE_CCD_OFFSET                (FILE_SYSEXT_OFFSET + FILE_SYSEXT_LEN)  //
#define FILE_CCD_LEN                   (64*1024) 



#define FILE_SYSTEM_FLAG              0x01       //sysem.cfg           bit0
#define FILE_RUNPARA_FLAG             0x02       //runpara.cfg         bit1
#define FILE_SYSEXT_FLAG              0x04       //systemext.cfg       bit2
#define FILE_CCD_FLAG                 0x08       //ccd文件               bit3 

#define FILE_PROSETPUBTABLE_FLAG      0x01       // ProSetPubTable.cfg 
#define FILE_FDPUBSET_FLAG            0x02       //fdpubset            

#define FILE_PROSETTABLE_FLAG         0x01       //prosettable.cfg     
#define FILE_FDSET_FLAG               0x02       //fdset               

SECTION_PLT_CODE void PowerOn_Init_SharedMemory(void);
void * hd_map_memory(uint32 addr, uint32 length, int * pfd);
STATUS hd_unmap_memory(void * map_ptr, uint32 length, int fd);

int ReadParaFile(const char *filename, BYTE *buf, int buf_len);
int bmrun(char *filename);
int WriteParaProFile(const char *filename, BYTE *buf);
int ReadCcdFile(BYTE *buf, int buf_len);


#endif /* APP_BMRUN_H_ */
