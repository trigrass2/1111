/*------------------------------------------------------------------------
 Module:		bsp.h
 Author:		helen
 Project:		
 Creation Date: 2014-09-15
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/

#ifndef _BSP_H
#define _BSP_H

#include "syscfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mytypes.h"
#include "spi.h"
#include "i2c.h"

#define REG8(addr)  		*((volatile unsigned char *) (addr))
#define REG16(addr) 		*((volatile unsigned short *) (addr))
#define REG32(addr) 		*((volatile unsigned long *) (addr))

#define	sysInByte(port)        REG8(port)
#define sysInWord(port)        REG16(port)
#define sysInLong(port)        REG32(port)
#define sysOutByte(port, data) REG8(port) = data
#define sysOutWord(port, data) REG16(port) = data
#define sysOutLong(port, data) REG32(port) = data


//后续放 共享内存地址 2019年12月19日 11:34:10
#define BSP_ESRAM_TIMER		  (BSP_ESRAM_ADDR+0x00)		     /*软时钟存放*/
#define BSP_ESRAM_EXTYX       (BSP_ESRAM_ADDR+0x0B)          /*8路扩充遥信存放*/
#define BSP_ESRAM_YK          (BSP_ESRAM_ADDR+0x0C)          /*32路YK输出存放*/
#define BSP_ESRAM_YX          (BSP_ESRAM_ADDR+0x2C)          /*64路YX状态存放*/
#define BSP_ESRAM_MISC        (BSP_ESRAM_ADDR+0x3C)          /*MISC*/
#define BSP_ESRAM_PLUSE       (BSP_ESRAM_ADDR+0x4C)          /*64个脉冲值*/
#define BSP_ESRAM_ENERGY      (BSP_ESRAM_PLUSE+64*sizeof(DWORD))
#define BSP_ESRAM_FREE        (BSP_ESRAM_ENERGY+64*sizeof(DWORD))

#define SYS_PATH              "/mnt/emmc_data"
#define SYS_PATH_ROOT         "/mnt/emmc_data/tffsa"
#define SYS_PATH_ROOT2        "/mnt/emmc_data/tffsb"

//AT EEPROM 64K
#define NVRAM_DEV_ID                 0x00
#define NVRAM_BS                     0x20
#define NVRAM_BOOT_CNT               0x3F
#define NVRAM_AD_IP_MAC              0x40
#define NVRAM_AD_IPEXT_MAC           0xB0
#define NVRAM_AD_DOYB                0x100
#define NVRAM_AD_CELL                0x108
#define NVRAM_AD_GPRS                0x300
#define NVRAM_AD_RESET               0x450
#define NVRAM_PROG_CODE              0x4A0
#define NVRAM_AD_DD                  0x500
   
#define NVRAM_DEV_ID_LEN        24
#define NVRAM_AD_GAIN                (2*1024)
#define NVRAM_AD_AIDC_GAIN           (NVRAM_AD_GAIN-0x100)

#if (DEV_SP  ==  DEV_SP_TTU)||(DEV_SP == DEV_SP_DTU_IU)
#define NVRAM_AD_ATT_GAIN            (NVRAM_AD_GAIN+4*1024)
#define NVRAM_PowerOff                8*1024
#define NVRAM_MonOverU               (NVRAM_PowerOff+0x100)
#define NVRAM_MonBelowU              (NVRAM_MonOverU+0x100)
#define NVRAM_MonPassU               (NVRAM_MonBelowU+0x100)
#define NVRAM_AD_DD_JFPG             (NVRAM_MonPassU+0x100)
#define NVRAM_ATT_GAIN               (NVRAM_AD_DD_JFPG+2*1024)
#endif

#define NVRAM_DEV_TYPE_FLAG                  (7*1024) //保存装置出厂型号设置标志
#define NVRAM_DEV_TYPE                 (NVRAM_DEV_TYPE_FLAG + 1)  //保存装置出厂型号，方便给别人贴牌预留长度PARACHAR_MAXLEN
#define NVRAM_DEV_MFR_FLAG       (NVRAM_DEV_TYPE + Old_MAXLEN) //保存装置终端制造商gb2312设置标志
#define NVRAM_DEV_MFR                   (NVRAM_DEV_MFR_FLAG+1) //保存装置厂商gb2312代码 方便给别人贴牌预留长度PARACHAR_MAXLEN
#define NVRAM_DEV_SV_FLAG          (NVRAM_DEV_MFR + Old_MAXLEN) //保存装置的软件版本号标志
#define NVRAM_DEV_SV               (NVRAM_DEV_SV_FLAG + 1)        //保存装置的软件版本号
#define NVRAM_DEV_PUBYB                (NVRAM_DEV_SV+Old_MAXLEN)
#define NVRAM_DEV_MFR_UTF8_FLAG                   (NVRAM_DEV_PUBYB + 4) //保存装置厂商代码utf8标志
#define NVRAM_DEV_MFR_UTF8                   (NVRAM_DEV_MFR_UTF8_FLAG+1) //保存装置厂商代码utf-8 方便给别人贴牌预留长度PARACHAR_MAXLEN
#define NVRAm_DEV_PUBYB_MULTI                (NVRAM_DEV_MFR_UTF8+PARACHAR_MAXLEN)//多回线压板 预留长度15*4

#define NVRAM_DEV_HV_FLAG          (NVRAm_DEV_PUBYB_MULTI + 4*20) //保存装置的硬件版本号标志
#define NVRAM_DEV_HV               (NVRAM_DEV_HV_FLAG + 1)   


#define romNvRamGet(offset, buf, len)   extNvRamGet(offset, buf, len);
#define romNvRamSet(offset, buf, len)   extNvRamSet(offset, buf, len);

#define CODE_UPDATE_MAIN             0x55AA5A5A
#define CODE_UPDATE_BOOT             0xAA55A5A5
#define CODE_UPDATED                 0x55AA55AA
#define CODE_WRONG                   0x55555555

/*转换成hex格式写到文件最后，在文件结尾之前，校验和算到最后300个字符前 13+270+17*/
#define PROGVER_HEX_LEN              345
#define PROGCRC_HEX_LEN              37
#pragma pack(1)
typedef struct{
    DWORD flag;
	DWORD sectors;
	WORD crc;
}Vupdate_code;

typedef struct{
	char  flag1[4];
	char  sp[30];
	char  flag2[4];
    char  ver[30];
	char  flag3[4];
	char  user[30];
	char  flag4[4];
	DWORD codecrc;
	WORD  flag;
}Vflash_code;

typedef struct{
	DWORD addr;
	DWORD len;
	DWORD crc;
}Vsector_code;

typedef struct{
	DWORD sectors;
	Vsector_code code[20];
}Vbin_code;

typedef struct{
	DWORD sectors;
	Vsector_code code[40];
}Vbin_code_xin;

typedef struct{
	BYTE ip[4];
	BYTE mask[4];
	BYTE gate[4];
	BYTE mac[6];
}VEth_ip_mac;

typedef struct{			
	DWORD flag;
	BYTE mac1[6];
	BYTE mac2[6];
	char ip1[20];
	DWORD mask1;
	char ip2[20];
	DWORD mask2;
	char gateway1[20];
	char gateway2[20];
}Veeprom_ip_mac;
typedef struct{			
	DWORD flag;
	BYTE mac3[6];
	char ip3[20];
	DWORD mask3;
	char gateway3[20];
}Veeprom_ipext_mac;
#pragma pack()

	
/*AI拨码反了*/
#if ((DEV_SP == DEV_SP_FTU)||(DEV_SP == DEV_SP_WDG))
#define BSP_PORT_YK          0x01
#define BSP_PORT_YKFJ        0x02

BYTE readAddr(void);

#define BSP_AC_BOARD                1
#define BSP_IO_BOARD                1
#define BSP_MAX_BOARD               1
#if (DEV_SP == DEV_SP_FTU)
#define Get_Ai_Type(addr)      2
#define Get_Io_Type(addr)      3
#else
#define Get_Ai_Type(addr)      3
#define Get_Io_Type(addr)      2
#endif
#define Get_Power_Type(addr)   1
#define CPLD_Ver(void)         0


DWORD DI_INPUT(void);
DWORD GET_DI_INPUT(void);

#if (DEV_SP == DEV_SP_FTU)
enum
{
	BSP_LIGHT_RUN_ID,
	BSP_LIGHT_BGN,
	BSP_LIGHT_WARN_ID=BSP_LIGHT_BGN,
	BSP_LIGHT_KG_ID,
	BSP_LIGHT_KGF_ID,
	BSP_LIGHT_COMM_ID,
	BSP_LIGHT_IYB_ID,
	BSP_LIGHT_I_ID,
	BSP_LIGHT_I0YB_ID,
	BSP_LIGHT_I0_ID,
	BSP_LIGHT_FAIN_ID,
	BSP_LIGHT_FACOMM_ID,
	BSP_LIGHT_FADZ_ID,
	BSP_LIGHT_FACON_ID,
	BSP_LIGHT_FA_ID = BSP_LIGHT_FACON_ID,
	BSP_LIGHT_END
};
#elif (DEV_SP == DEV_SP_WDG)
enum
{
	BSP_LIGHT_RUN_ID,
	BSP_LIGHT_BGN,
	BSP_LIGHT_WARN_ID=BSP_LIGHT_BGN,
	BSP_LIGHT_KG_ID,
	BSP_LIGHT_I0_ID,
	BSP_LIGHT_I_ID,
	BSP_LIGHT_BS_ID,
	BSP_LIGHT_COMM_ID,
	BSP_LIGHT_TV_ID,	
	BSP_LIGHT_ENG_ID,
	BSP_LIGHT_REMOTE_ID,
	BSP_LIGHT_LOCAL_ID,
	BSP_LIGHT_FAULT_ID,	
	BSP_LIGHT_CELL_ID,
	BSP_LIGHT_FAIN_ID,
	BSP_LIGHT_FACOMM_ID,
	BSP_LIGHT_FADZ_ID,
	BSP_LIGHT_FACON_ID,
	BSP_LIGHT_FA_ID = BSP_LIGHT_FACON_ID,
	BSP_LIGHT_END
};
#endif

void  turnLight(int id, BYTE on);
DWORD getLight(void);
void runLight_turn(void);

#endif

#if (DEV_SP == DEV_SP_DTU)

extern uint8 FPGA_CPLD_READ (DWORD addr);
extern void FPGA_CPLD_WRITE (DWORD addr,BYTE regVal);

#define BSP_ADDR_CS4         0                            //cpld扩展

#define BSP_PORT_CPLD               ((volatile unsigned char *)(BSP_ADDR_CS4+0))
#define BSP_PORT_YX_LATCH           ((volatile unsigned char *)(BSP_ADDR_CS4+0x40))
#define BSP_PORT_YX_READ            ((volatile unsigned char *)(BSP_ADDR_CS4+0x44))
#define BSP_PORT_YK                 ((volatile unsigned char *)(BSP_ADDR_CS4+0x80))
#define BSP_PORT_LED1               ((volatile unsigned char *)(BSP_ADDR_CS4+0xC0))
#define BSP_PORT_LED2               ((volatile unsigned char *)(BSP_ADDR_CS4+0xC4))
#define BSP_PORT_LED3               ((volatile unsigned char *)(BSP_ADDR_CS4+0xC8))
#define BSP_PORT_YK_CELL            ((volatile unsigned char *)(BSP_ADDR_CS4+0x94))


#define BSP_ADDR_POWER_TYPE         0x08
#define BSP_ADDR_POWER_DI           0x09
#define BSP_ADDR_IO1_TYPE           0x40
#define BSP_ADDR_IO1_READ           0x42
#define BSP_ADDR_YK1_FJ             0x41
#define BSP_ADDR_AC1_TYPE           0x50
#define BSP_ADDR_CPLD_ADD           0x8

#define BSP_AC_BOARD                3
#define BSP_IO_BOARD                7

#define BSP_RAM_YK                  0x01
#define BSP_RAM_YKFJ                0x02
#define BSP_PORT_YKD1               0x25
#define BSP_PORT_YKCELL             0x41


//读cpld操作类
myinline BYTE CPLD_Read(volatile unsigned char * addr) 
{	
	BYTE value = 0;
	
	value = FPGA_CPLD_READ((DWORD)addr);
	
	return value;
}
//写CPLD操作类
myinline void CPLD_Write(volatile unsigned char * addr,  BYTE value)
{
	FPGA_CPLD_WRITE((DWORD)addr,value);
}


/*以cpu为中心，往两边扩散，yx，yk板序号与硬件相反*/
myinline BYTE Get_Ai_Type(BYTE addr)
{
	CPLD_Write(BSP_PORT_YX_LATCH,BSP_ADDR_AC1_TYPE + addr*8);
	return (CPLD_Read(BSP_PORT_YX_READ));
}

myinline BYTE Get_Io_Type(BYTE addr)
{
	CPLD_Write(BSP_PORT_YX_LATCH,BSP_ADDR_IO1_TYPE - addr*8);
	return (CPLD_Read(BSP_PORT_YX_READ));
}

myinline BYTE Get_Power_Type(BYTE addr)
{
	BYTE YX_READ;

	CPLD_Write(BSP_PORT_YX_LATCH,BSP_ADDR_POWER_TYPE);
	YX_READ = CPLD_Read(BSP_PORT_YX_READ);
	
	return YX_READ;
}

#define  CPLD_Ver(void)  (CPLD_Read(BSP_PORT_CPLD))

enum
{
	BSP_LIGHT_RUN_ID,
	BSP_LIGHT_BGN,
	BSP_LIGHT_WARN_ID=BSP_LIGHT_BGN,
	BSP_LIGHT_COMM_ID,
	BSP_LIGHT_GPRS_ID,
	BSP_LIGHT_SYS_ID = BSP_LIGHT_GPRS_ID,
	BSP_LIGHT_FAIN_ID,
	BSP_LIGHT_FADZ_ID,
	BSP_LIGHT_FACOMM_ID,
	BSP_LIGHT_FACON_ID,
	BSP_LIGHT_FA_ID = BSP_LIGHT_FACON_ID,
	BSP_LIGHT_FAULT1_ID,
	BSP_LIGHT_FAULT2_ID,
	BSP_LIGHT_FAULT3_ID,
	BSP_LIGHT_FAULT4_ID,
	BSP_LIGHT_FAULT5_ID,
	BSP_LIGHT_FAULT6_ID,
	BSP_LIGHT_FAULT7_ID,
	BSP_LIGHT_FAULT8_ID,
	BSP_LIGHT_FAULT9_ID,
	BSP_LIGHT_FAULT10_ID,
	BSP_LIGHT_FAULT11_ID,
	BSP_LIGHT_FAULT12_ID,
	BSP_LIGHT_FAULT13_ID,
	BSP_LIGHT_FAULT14_ID,
	BSP_LIGHT_FAULT15_ID,
	BSP_LIGHT_FAULT16_ID,
	BSP_LIGHT_END
};

void  turnLight(int id, BYTE on);
DWORD getLight(void);
void runLight_turn(void);

#endif

#if (DEV_SP == DEV_SP_TTU)
#define BSP_PORT_YK          0x01
#define BSP_PORT_YKFJ        0x02

BYTE readAddr(void);
#define BSP_PORT_YX_READ       0  //临时写 2019年12月19日 11:48:30
#define BSP_YX_LED_ADDR  0x42

#define GPRS_IN                    0
#define BOOT_MODE 		 0
#define BSP_AC_BOARD                1
#define BSP_IO_BOARD                1
#define BSP_MAX_BOARD               1

#define Get_Ai_Type(addr)      4
#define Get_Io_Type(addr)      5

#define Get_Power_Type(addr)   1
#define CPLD_Ver(void)         0

DWORD DI_INPUT(void);
DWORD GET_DI_INPUT(void);

enum
{
	BSP_LIGHT_RUN_ID,
	BSP_LIGHT_BGN,
	BSP_LIGHT_WARN_ID=BSP_LIGHT_BGN,
	BSP_LIGHT_COMM_ID,
	BSP_LIGHT_RF_ID,
	BSP_LIGHT_ZB_ID,
	BSP_LIGHT_WIFI_ID,
	BSP_LIGHT_GPRS_NET_ID,
	BSP_LIGHT_GPRS_ST_ID,
	BSP_LIGHT_YX1_ID,
	BSP_LIGHT_YX2_ID,
	BSP_LIGHT_YX3_ID,
	BSP_LIGHT_YX4_ID,
	BSP_LIGHT_YX5_ID,
	BSP_LIGHT_YX6_ID,
	BSP_LIGHT_YX7_ID,
	BSP_LIGHT_YX8_ID,
	BSP_LIGHT_END
};

void  turnLight(int id, BYTE on);
DWORD getLight(void);
void runLight_turn(void);
#endif

void myMacCheck(void);
void myMacGet(BYTE *mac1, BYTE *mac2);
void myIpSet(char *ip1, DWORD mask1, char *ip2, DWORD mask2, char *gateway1, char *gateway2);
void myIpGet(char *ip1, DWORD *mask1, char *ip2, DWORD *mask2, char *gateway1, char *gateway2);

void Slow(unsigned long  a);
void quickreboot(void);
void reboot(int flag);
void BootDogMon(int interval);

STATUS extNvRamGet(DWORD offset, BYTE *buf, int len);
STATUS extNvRamSet(DWORD offset, BYTE *buf, int len);

void RtosAssert(char *msg, int flag);

extern char g_assert_buf[128];

int GetNetAddr(int no,BYTE* szAddr);
int ESDK_GetNetMac(int no,char *szMac);
int ESDK_GetNetAddr(int no,char *szAddr);
int ESDK_SetNetInfo(int no,const char *szAddr, const char *szMask, const char *szGatway);
void sendLedIO(DWORD LedValue,DWORD* IoValue,int iolen);
int ReadIO(int no,int id,BYTE* value);
void SetLinuxLed(int id, BYTE on);

#ifdef __cplusplus
}
#endif
#endif


