/*------------------------------------------------------------------------
 Module:		bsp.h
 Author:		
 Project:		
 Creation Date: 2020Äê7ÔÂ23ÈÕ 11:30:03
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

//AT EEPROM 64K
#define NVRAM_DEV_ID                 0x00
#define NVRAM_BS                     0x20
#define NVRAM_BOOT_CNT               0x3F
#define NVRAM_AD_IP_MAC              0x40
#define NVRAM_AD_IPEXT_MAC           0xB0
#define NVRAM_AD_DOYB                0x100
#define NVRAM_AD_CELL                0x108
#define NVRAM_BS_SP                  0x2B0
#define NVRAM_AD_GPRS                0x300
#define NVRAM_AD_RESET               0x450
#define NVRAM_PROG_CODE              0x4A0
#define NVRAM_AD_DD                  0x500

#define NVRAM_DEV_ID_LEN        24
#define NVRAM_AD_GAIN                (2*1024)
#define NVRAM_AD_AIDC_GAIN           (NVRAM_AD_GAIN-0x100)
#define NVRAM_AD_WYFZ_YX             (NVRAM_AD_AIDC_GAIN-0x20)//???ftu???


#if (DEV_SP  ==  DEV_SP_TTU)||(DEV_SP == DEV_SP_DTU_IU)
#define NVRAM_AD_ATT_GAIN            (NVRAM_AD_GAIN+4*1024)
#define NVRAM_PowerOff                8*1024
#define NVRAM_MonOverU               (NVRAM_PowerOff+0x100)
#define NVRAM_MonBelowU              (NVRAM_MonOverU+0x100)
#define NVRAM_MonPassU               (NVRAM_MonBelowU+0x100)
#define NVRAM_AD_DD_JFPG             (NVRAM_MonPassU+0x100)
#define NVRAM_ATT_GAIN               (NVRAM_AD_DD_JFPG+2*1024)

#endif




#define NVRAM_YAFFS_DISK1            (7*1024)
#define NVRAM_YAFFS_DISK2            (NVRAM_YAFFS_DISK1 + 32)
#define NVRAM_YAFFS_MAG1             (NVRAM_YAFFS_DISK2 + 48)
#define NVRAM_YAFFS_MAG2             (NVRAM_YAFFS_MAG1 + 16)
#define NVRAM_DEV_TYPE_FLAG      (NVRAM_YAFFS_MAG2 + 16) //????????????
#define NVRAM_DEV_TYPE                 (NVRAM_DEV_TYPE_FLAG + 1)  //????????,???????????PARACHAR_MAXLEN
#define NVRAM_DEV_MFR_FLAG       (NVRAM_DEV_TYPE + Old_MAXLEN) //?????????gb2312????
#define NVRAM_DEV_MFR                   (NVRAM_DEV_MFR_FLAG+1) //??????gb2312?? ???????????PARACHAR_MAXLEN
#define NVRAM_DEV_SV_FLAG          (NVRAM_DEV_MFR + Old_MAXLEN) //????????????
#define NVRAM_DEV_SV               (NVRAM_DEV_SV_FLAG + 1)        //??????????
#define NVRAM_DEV_PUBYB                (NVRAM_DEV_SV+Old_MAXLEN)
#define NVRAM_DEV_MFR_UTF8_FLAG                   (NVRAM_DEV_PUBYB + 4) //????????utf8??
#define NVRAM_DEV_MFR_UTF8                   (NVRAM_DEV_MFR_UTF8_FLAG+1) //????????utf-8 ???????????PARACHAR_MAXLEN
#define NVRAm_DEV_PUBYB_MULTI                (NVRAM_DEV_MFR_UTF8+PARACHAR_MAXLEN)//????? ????15*4


#define NVRAM_CODE_SECTOR            (8*1024 - 0x100) //
#define NVRAM_CODE_SECTOR_EXT            (8*1024 - 0x200) //??????256??,?sector??20?,?????

#define BSP_NVRAM_ERRCOUNT  (NVRAM_CODE_SECTOR_EXT - 100) //BOOT ?
#define BSP_NVRAM_BOOTERR     (BSP_NVRAM_ERRCOUNT+0x04)
#define BSP_NVRAM_CELLIN      (BSP_NVRAM_BOOTERR+0x04)
#define BSP_NVRAM_FAON        (BSP_NVRAM_CELLIN+0x04)  //32fd
#define BSP_NVRAM_FADIFFON  (BSP_NVRAM_FAON+0x04)
#define BSP_NVRAM_LANERR    (BSP_NVRAM_FADIFFON+0x04)//?15???



extern uint8 FPGA_CPLD_READ (DWORD addr);
extern void FPGA_CPLD_WRITE (DWORD addr,BYTE regVal);


#define BSP_ADDR_CS4         0                            //cpld??

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

extern unsigned char BSP_ESRAM[];

#define BSP_ESRAM_ADDR	     ((DWORD)BSP_ESRAM)
#define ESRAM_TOTAL_SIZE	 4096

#define BSP_ESRAM_EXTYX       (BSP_ESRAM_ADDR+0x0B)          /*8???????*/
#define BSP_ESRAM_YK          (BSP_ESRAM_ADDR+0x0C)          /*32?YK????       ?*/
#define BSP_ESRAM_YX          (BSP_ESRAM_ADDR+0x2C)          /*64?YX????*/


#define REG8(addr)  		*((volatile unsigned char *) (addr))
#define REG16(addr) 		*((volatile unsigned short *) (addr))
#define REG32(addr) 		*((volatile unsigned long *) (addr))


//?cpld???
myinline BYTE CPLD_Read(volatile unsigned char * addr) 
{	
	BYTE value = 0;
	
	value = FPGA_CPLD_READ((DWORD)addr);
	
	return value;
}
//?CPLD???
myinline void CPLD_Write(volatile unsigned char * addr,  BYTE value)
{
	FPGA_CPLD_WRITE((DWORD)addr,value);
}

/*?cpu???,?????,yx,yk????????*/
myinline BYTE Get_Ai_Type(BYTE addr)
{
     /*REG8(BSP_PORT_YX_LATCH) = BSP_ADDR_AC1_TYPE + addr*8;
	 return (REG8(BSP_PORT_YX_READ));*/

	 
	CPLD_Write(BSP_PORT_YX_LATCH,BSP_ADDR_AC1_TYPE + addr*8);
	return (CPLD_Read(BSP_PORT_YX_READ));
	 
	//return 0x64;
	 
}

myinline BYTE Get_Io_Type(BYTE addr)
{
    /*REG8(BSP_PORT_YX_LATCH) = BSP_ADDR_IO1_TYPE - addr*8;
	return (REG8(BSP_PORT_YX_READ));*/

	
	CPLD_Write(BSP_PORT_YX_LATCH,BSP_ADDR_IO1_TYPE - addr*8);
	return (CPLD_Read(BSP_PORT_YX_READ));
	
	/*if(addr <3 )
		return 0x09;
	else
		return 0x08;*/
}

myinline BYTE Get_Power_Type(BYTE addr)
{
	BYTE YX_READ;
	
    /*REG8(BSP_PORT_YX_LATCH) = BSP_ADDR_POWER_TYPE;
	YX_READ = REG8(BSP_PORT_YX_READ);*/

	
	CPLD_Write(BSP_PORT_YX_LATCH,BSP_ADDR_POWER_TYPE);
	YX_READ = CPLD_Read(BSP_PORT_YX_READ);
	

	/*YX_READ = 0;*/
	return YX_READ;
}

myinline BYTE PhyBit_Read(DWORD ramaddr, DWORD port)
{
    BYTE in, k;
	BYTE YX_READ;
	/*if (ramaddr == BSP_RAM_YKFJ)
	{	
	    REG8(BSP_PORT_YX_LATCH) = port; 
	    YX_READ = (REG8(BSP_PORT_YX_READ));
      return YX_READ;		
	}
	else if (ramaddr == BSP_RAM_YK)
	{
	    in = port/32;
		k = port%32;
	    return (REG8(BSP_ESRAM_YK+in)>>k);
	}
	else */
	
	if (ramaddr == BSP_RAM_YKFJ)
	{	
		CPLD_Write(BSP_PORT_YX_LATCH,port);
	    
	    YX_READ = (CPLD_Read(BSP_PORT_YX_READ));
      return YX_READ;		
	}
	else if (ramaddr == BSP_RAM_YK)
	{
	    in = port/32;
		k = port%32;
	    return (REG8(BSP_ESRAM_YK+in)>>k);
	}
	else
		return 0;
}

myinline void PhyBit_Set(DWORD ramaddr, DWORD port, BYTE value)
{
    DWORD out,bit;
	
   /*if (port < 0x40)
		REG8(BSP_PORT_YK) = port|(value<<7);
    else
		REG8(BSP_PORT_YK_CELL) = port|(value<<7);
		
	out = port/32;

	bit = 0x01<<(port - 32);

	if (value)
	    REG32(BSP_ESRAM_YK + out) |= bit;
	else
		REG32(BSP_ESRAM_YK + out) &= ~bit;*/

	if (port < 0x40)
		CPLD_Write(BSP_PORT_YK,port|(value<<7));
	else
		CPLD_Write(BSP_PORT_YK_CELL, port|(value<<7));

	
	out = port/32;

	bit = 0x01<<(port - 32);

	if (value)
	    REG32(BSP_ESRAM_YK + out) |= bit;
	else
		REG32(BSP_ESRAM_YK + out) &= ~bit;

}

myinline void PhyBit_Init(DWORD ramaddr, DWORD port, BYTE value)
{
	REG8(ramaddr) = value;
	PhyBit_Set(ramaddr, port, value);
}

#define  CPLD_Ver(void)  (CPLD_Read(BSP_PORT_CPLD))

#define romNvRamGet(offset, buf, len)   extNvRamGet(offset, buf, len);
#define romNvRamSet(offset, buf, len)   extNvRamSet(offset, buf, len);


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

#define GPIO_VESAM_CON      54
#define GPIO_GPS_RST        55
#define GPIO_RUN_LED        56
#define GPIO_WARN_LED       57
#define GPIO_COMM_LED       58
#define GPIO_GPRS_LED       59
#define GPIO_JC_RST         60
#define GPIO_WIFI_RST       61
#define GPIO_KG1            62
#define GPIO_KG2            63
#define GPIO_AJ_FW          64
#define GPIO_JC_SIG         65
#define GPIO_CPLD_RST       66
#define HDLC1_SD            67
#define HDLC2_SD            68
#define HDLC3_SD            69


//??GPIO?????,??Data??
void hd_gpio_SetDirectionOut(u32 io, u32 Data);
//??GPIO???,???????
u32 hd_gpio_SetDirectionIn(u32 io);
//????
void hd_gpio_SetOut(u32 io, u32 Data);
//????
u32 hd_gpio_ReadIn(u32 io);


#define SET_GPIO(port, pin)     hd_gpio_SetOut( pin, 1)

#define CLEAR_GPIO(port, pin)    hd_gpio_SetOut( pin, 0)

/*??input??     output ?port->PIO_ODSR*/
#define READ_GPIO(port, pin)    (hd_gpio_ReadIn(pin))

void runlinuxled();

#ifdef __cplusplus
}
#endif
#endif

