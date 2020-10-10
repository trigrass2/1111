/*------------------------------------------------------------------------
 Module:		bsp.c
 Author:		helen
 Project:		
 Creation Date: 2014-09-15
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/

#include <string.h>
#include <stdarg.h>
#include "syscfg.h"
#include "bsp.h"
#include "os.h"
#include "sys.h"

void BSP_IO_Init(void)
{
	int id;
	
	hd_gpio_SetDirectionOut(GPIO_VESAM_CON, 0);
	hd_gpio_SetDirectionOut(GPIO_GPS_RST, 1);
	hd_gpio_SetDirectionOut(GPIO_RUN_LED, 1);
	hd_gpio_SetDirectionOut(GPIO_WARN_LED, 1);
	hd_gpio_SetDirectionOut(GPIO_COMM_LED, 1);
	hd_gpio_SetDirectionOut(GPIO_GPRS_LED, 1);
	hd_gpio_SetDirectionOut(GPIO_JC_RST, 1);
	hd_gpio_SetDirectionOut(GPIO_WIFI_RST, 1);
	//hd_gpio_SetDirectionOut(GPIO_CPLD_RST, 1);

	hd_gpio_SetDirectionIn(GPIO_KG1);
	hd_gpio_SetDirectionIn(GPIO_KG2);
	hd_gpio_SetDirectionIn(GPIO_AJ_FW);
	hd_gpio_SetDirectionIn(GPIO_JC_SIG);

	for (id = BSP_LIGHT_BGN; id<=BSP_LIGHT_END; id++)
	{
		turnLight(id , 0);
		turnLight(id , 1);
		turnLight(id , 0);
	}
	
}

static DWORD devLight = 0x0F;
static DWORD old_DevLight = 0x0F;

void turnLight(int id, BYTE on)
{
	DWORD bit = 0x01<<id;
	BYTE light;
	
	if (on)  devLight |= bit;
	else devLight &= (~bit);		
	
	if ((devLight^old_DevLight) == 0)  return;

	switch(id)
	{
	    case BSP_LIGHT_WARN_ID:	
		if (on)
			  SET_GPIO(0,GPIO_WARN_LED);
		else
		    CLEAR_GPIO(0,GPIO_WARN_LED);
		break;
	    case BSP_LIGHT_RUN_ID:		
		if (on)
		{
		    SET_GPIO(0,GPIO_RUN_LED);
			CPLD_Write(BSP_PORT_YK_CELL, 0xC3); // 
			CPLD_Write(BSP_PORT_YK_CELL, 0xC4);
		}
		else
		{
		    CLEAR_GPIO(0,GPIO_RUN_LED);
			CPLD_Write(BSP_PORT_YK_CELL, 0x43); // 
			CPLD_Write(BSP_PORT_YK_CELL, 0x44);
		}
		break; 
		case BSP_LIGHT_COMM_ID:
		if (on)
		    SET_GPIO(0,GPIO_COMM_LED);
		else
		    CLEAR_GPIO(0,GPIO_COMM_LED);
		break;
		case BSP_LIGHT_GPRS_ID:
		if (on)
		   SET_GPIO(0,GPIO_GPRS_LED);
		else
		   CLEAR_GPIO(0,GPIO_GPRS_LED);
		break;
		case BSP_LIGHT_FAIN_ID:
		case BSP_LIGHT_FACOMM_ID:
		case BSP_LIGHT_FADZ_ID:
		case BSP_LIGHT_FACON_ID:
			light = (devLight >> 4)&0x0F;
			CPLD_Write(BSP_PORT_LED1, light);
		break;
		case BSP_LIGHT_FAULT1_ID:
		case BSP_LIGHT_FAULT2_ID:
		case BSP_LIGHT_FAULT3_ID:
		case BSP_LIGHT_FAULT4_ID:
		case BSP_LIGHT_FAULT5_ID:
		case BSP_LIGHT_FAULT6_ID:
		case BSP_LIGHT_FAULT7_ID:
		case BSP_LIGHT_FAULT8_ID:
		     light = (devLight >> 8)&0xFF;
			 CPLD_Write(BSP_PORT_LED2, light);
			 break;
		case BSP_LIGHT_FAULT9_ID:
		case BSP_LIGHT_FAULT10_ID:
		case BSP_LIGHT_FAULT11_ID:
		case BSP_LIGHT_FAULT12_ID:
		case BSP_LIGHT_FAULT13_ID:
		case BSP_LIGHT_FAULT14_ID:
		case BSP_LIGHT_FAULT15_ID:
	    case BSP_LIGHT_FAULT16_ID:
			light = (devLight >> 16)&0xFF;
			CPLD_Write(BSP_PORT_LED3, light);
			break;
	}

#if defined (INCLUDE_EXT_MMI) || defined (INCLUDE_EXT_DISP)
    //mmiLight(id , on);  
#endif
	old_DevLight = devLight;

}

DWORD getLight(void)
{
	return devLight;
}

void runLight_turn(void)
{
    if (devLight & 0x01)
	{
		SET_GPIO(0,GPIO_RUN_LED);
		CPLD_Write(BSP_PORT_YK_CELL, 0xC3); //
		CPLD_Write(BSP_PORT_YK_CELL, 0xC4);
	}
	else
	{
	    CLEAR_GPIO(0,GPIO_RUN_LED);
		CPLD_Write(BSP_PORT_YK_CELL, 0x43); //
		CPLD_Write(BSP_PORT_YK_CELL, 0x44);		
	}

	devLight ^= 0x01;
}

void runlinuxled()
{
	DWORD mask,ledmask,ledval;
	int id;

	ledmask = ledval = 0;
	ReadLinuxLed(&ledmask, &ledval);

	for(id = 0; id < 32; id++)
	{
		mask = 1 << id;
		
		if(mask & ledmask)
		{
			turnLight(id,ledval & mask);
		}
	}
}





