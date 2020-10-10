/*------------------------------------------------------------------------
 Module:		serial.c
 Author:		solar
 Project:		
 Creation Date: 2008-10-29
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/

#include "syscfg.h"
#include "comm_cfg.h"

#ifdef INCLUDE_SERIAL
#include "sys.h"
#include "uart.h"
#include "serial.h"


/*------------------------------------------------------------------------
 Procedure:     SerialInit ID:1
 Purpose:       初始化串口
 Input:          				
 Output:		0:OK; -1:ERROR.
 Errors:
------------------------------------------------------------------------*/
int SerialInit (void)
{
#ifdef INCLUDE_UART
	uartInit();
#endif

    return OK;
}

/*------------------------------------------------------------------------
 Procedure:     SerialRead ID:1
 Purpose:       读串口
 Input:          
 Output:		读入字节个数; <0: 出错 
 Errors:
------------------------------------------------------------------------*/
int SerialRead (int no, BYTE* pbuf, int buflen, DWORD flags)
{
	int i,wp;	
	VCommBuf *pfifo;	

	pfifo = &(g_CommChan[no].rx_buf);

	/*int lock*/
    wp = pfifo->wp;
	/*int inlock*/

	for(i=0; i<buflen; i++)
	{
	    if(pfifo->rp == wp) break;		
        *(pbuf+i) = *(pfifo->buf+ pfifo->rp);
		pfifo->rp = (pfifo->rp+1)&(SERIAL_BUF_NUM-1);
	}

	//g_CommChan[no].ev_send &= ~EV_RX_AVAIL;

	return i;
}

/*------------------------------------------------------------------------
 Procedure:     SerialWrite ID:1
 Purpose:       写串口
 Input:         no: 端口号; pbuf: 缓冲区
 				buflen: 指定长度; 
 Output:		读入字节个数; <0: 出错 
 Note:          
------------------------------------------------------------------------*/
int SerialWrite (int no,const BYTE* pbuf, int buflen, DWORD flags)
{
	int i; WORD wp;
	VCommBuf *pfifo;

	g_CommChan[no].tx_idle = 0;

	if(buflen == 0) return 0;

	pfifo = &(g_CommChan[no].tx_buf);

	/*g_CommChan[no].ev_send &= ~EV_TX_IDLE;
	g_CommChan[no].ev_send &= ~EV_COMM_IDLE;*/

	/*int lock*/
	for(i=0; i<buflen; i++)
	{
		wp = (pfifo->wp+1)&(SERIAL_BUF_NUM-1);
		if(pfifo->rp == wp) break;
		*(pfifo->buf+ pfifo->wp) = *(pbuf+i);
		pfifo->wp = wp;
	}
	/*int unlock*/
		
	/*打开发送中断*/
	g_CommChan[no].ctrl(no, CCC_INT_CTRL|TXINT_ON, NULL);

	/*g_CommChan[no].ev_send &= ~EV_TX_AVAIL;*/	
	
	return i;
}

void ScanSerial(void)
{
    #ifdef INCLUDE_UART
      uart_scan();
	#endif
}

void SerialCfgStrSet(int no,const char *cfgstr)
{
    char *str[2], parity, tmpstr[30];
	DWORD baudrate;
	BYTE databit, stopbit, dwparity;

	if (strcmp(cfgstr, "") == 0)  return;
	
    strcpy(tmpstr, cfgstr);
	GetStrFromMyFormatStr(tmpstr, ':', 2, &str[0], &str[1]);

	/*if (str[0] != NULL) *type = atoi(str[0]);
	else *type = 0;*/

	if (str[1] != NULL)  
	{
		baudrate = 9600;
		databit = 8;
		stopbit = 1;
		parity = 'N';

		sscanf(str[1], "%ld,%c,%c,%c", (long*)&baudrate, &databit, &stopbit, &parity);

		commCtrl(no+COMM_START_NO, CCC_CONNECT_CTRL|CONNECT_OPEN, 0);
		commCtrl(no+COMM_START_NO, CCC_BAUD_CTRL|BAUD_SET, (BYTE*)&baudrate);
		commCtrl(no+COMM_START_NO, CCC_DATA_BITS, &databit);
		commCtrl(no+COMM_START_NO, CCC_STOP_BITS, &stopbit);
		
		if ((parity == 'E') || (parity == 'e'))
		{
            dwparity = PARITY_SET_EVEN;
			commCtrl(no+COMM_START_NO, CCC_PARITY_SET, &dwparity);
		}	
		else if ((parity == 'O') || (parity == 'o'))
		{
            dwparity = PARITY_SET_ODD;
			commCtrl(no+COMM_START_NO, CCC_PARITY_SET, &dwparity);
		}	
		else
		{
            dwparity = PARITY_SET_NONE;
			commCtrl(no+COMM_START_NO, CCC_PARITY_SET, &dwparity); 
		}	
	}	
}

void SerialCfgStr2Value(const char *cfgstr, int *baudrate, int *databit, int *stopbit, char *parity)
{
    char *str[2], tmpstr[30];
	
    strcpy(tmpstr, cfgstr);
	GetStrFromMyFormatStr(tmpstr, ':', 2, &str[0], &str[1]);

    *baudrate = 0;
	*databit = 0;
	*stopbit = 0;
	*parity = 0;

	if (str[1] != NULL)  sscanf(str[1], "%d,%d,%d,%c", baudrate, databit, stopbit, parity);
}

#endif 

