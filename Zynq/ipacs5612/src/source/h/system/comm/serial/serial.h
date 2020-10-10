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

#define SERIAL_BUF_NUM  1024  /*������2����*/

/*����ͨ�����*/
typedef struct  {			
	int 	no;			    /*�˿ں�*/
	DWORD   addr;			/*�����ַ*/
	int	    int_no;			/*�жϺ�*/
	DWORD   int_mask;       

	BYTE	ctrl_int;		/*�жϿ���*/
	BYTE	ctrl_modem;		/*Modem����*/
	BYTE    cmd_cache;      
	BYTE	pad1;
	
    DWORD	baudRate;
}VSerPhyChan;

typedef struct  {
	DWORD   firstportaddr;      /*�忨�ĵ�һ���˿ڵ�ַ*/
	DWORD	boardint_no;		/*�忨�жϺ�*/
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

