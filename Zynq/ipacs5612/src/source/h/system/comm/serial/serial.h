/*------------------------------------------------------------------------
 Module:		serial.h
 Author:		solar
 Project:		
 Creation Date: 2008-10-29
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/

#ifndef _SERIAL_H
#define _SERIAL_H

#define SERIAL_BUF_NUM  1024  /*必须是2的幂*/

/*物理通道相关*/
typedef struct  {			
	int 	no;			    /*端口号*/
	DWORD   addr;			/*物理地址*/
	int	    int_no;			/*中断号*/
	DWORD   int_mask;       

	BYTE	ctrl_int;		/*中断控制*/
	BYTE	ctrl_modem;		/*Modem控制*/
	BYTE    cmd_cache;      
	BYTE	pad1;
	
    DWORD	baudRate;
}VSerPhyChan;

typedef struct  {
	DWORD   firstportaddr;      /*板卡的第一个端口地址*/
	DWORD	boardint_no;		/*板卡中断号*/
	DWORD   boardint_mask;
	BOOL	intsetok;	
}VSerChip;

int SerialInit (void);
int SerialRead (int no, BYTE* pbuf, int buflen, DWORD flags);
int SerialWrite (int no, const BYTE* pbuf, int buflen, DWORD flags);
void SerialCfgStrSet(int no, const char *cfgstr);
void SerialCfgStr2Value(const char *cfgstr, int *baudrate, int *databit, int *stopbit, char *parity);
void ScanSerial(void);
#endif

