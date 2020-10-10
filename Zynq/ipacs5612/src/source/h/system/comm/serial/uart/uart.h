/*------------------------------------------------------------------------
 Module:       	uart.h
 Author:        solar
 Project:       
 State:         
 Creation Date:	2008-08-05
 Description:   UART��������
------------------------------------------------------------------------*/

#ifndef _UART_H
#define _UART_H

/*����ͨ�����*/
typedef struct  {			
	int	no;			 
							
	DWORD   addr;			/*�����ַ*/
	BYTE	int_no;			/*�жϺ�*/
	BYTE	ctrl_modem;		/*Modem����*/
	BYTE	ctrl_int;		/*�жϿ���*/
    DWORD	baudRate;
    DWORD	options;
}VUartChan;

int uartInit(void);
void uart_scan(void);
#endif
