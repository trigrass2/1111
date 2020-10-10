/*------------------------------------------------------------------------
 Module:       	uart.h
 Author:        solar
 Project:       
 State:         
 Creation Date:	2008-08-05
 Description:   UART驱动声明
------------------------------------------------------------------------*/

#ifndef _UART_H
#define _UART_H

/*物理通道相关*/
typedef struct  {			
	int	no;			 
							
	DWORD   addr;			/*物理地址*/
	BYTE	int_no;			/*中断号*/
	BYTE	ctrl_modem;		/*Modem控制*/
	BYTE	ctrl_int;		/*中断控制*/
    DWORD	baudRate;
    DWORD	options;
}VUartChan;

int uartInit(void);
void uart_scan(void);
#endif
