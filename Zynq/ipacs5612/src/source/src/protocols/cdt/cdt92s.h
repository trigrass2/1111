/*------------------------------------------------------------------------
 Module:			cdts.h
 Author:			
 Project:        	
 State:				
 Creation Date:		
 Description:   	从站CDT92规约头文件
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef _CDT92S_H
#define _CDT92S_H

#include "cdtslave.h"

#define CODE_YK_SELECT		0x61  
#define CODE_YK_EXE			0xc2  
#define CODE_YK_CANCEL		0xB3  
#define CODE_YT_SELECT		0xF4  
#define CODE_YT_EXE			0x85  
#define CODE_YT_CANCEL		0x26  
#define CODE_TIMESET		0x7A

#define FUN_YK_SELECT		0xe0
#define FUN_YK_EXE			0xe2
#define FUN_YK_CANCEL		0xe3
#define FUN_YT_SELECT		0xe4
#define FUN_YT_EXE			0xe6
#define FUN_YT_CANCEL		0xe7

#define SEND_YC_A			0x61
#define SEND_YC_B  			0xc2
#define SEND_YC_C			0xb3
#define SEND_YX				0xf4
#define SEND_DD				0x85
#define SEND_SOE			0x26
#define SEND_YKRETURN		0x9e

#define CONTROL_BYTE		0x71

#define MAXSOENUM		8
#define SENDLISTLEN		1000

#define YK_SELECT		1
#define YK_EXE			2
#define YK_CANCEL		3

#define YES		1
#define NO		0

struct VSoeFrame{
	BYTE Code1;
	BYTE MSecondLo;
	BYTE MSecondHi;
	BYTE Second;
	BYTE Minute;
	BYTE Crc1;

	BYTE Code2;
	BYTE Hour;
	BYTE Day;
	BYTE YXNoLo;
	BYTE YXNoHi;
	BYTE Crc2;
};


class CSCDT92:public CCDTSec
{
	public:
		WORD SendSoeCount;
		VSoeFrame SoeFrame;
		BYTE SOEBuf[12*MAXSOENUM];
		//BYTE m_bySoeSendMode;
		WORD SOEBufSendLen;
		char SendList[SENDLISTLEN];
		
	public:
    	CSCDT92();
    	BOOL Init(WORD wTaskID);
		void InitSendList(void);
		virtual WORD GetInsertInfoLen(void);
		virtual BYTE GetInsertFrame(BYTE LastFrameType);	
		virtual BOOL DoReceive(void);
		BOOL ReceiveYKSelect(void);
		BOOL ReceiveYKExe(void);
		BOOL ReceiveYKCancel(void);
		BOOL ReceiveYTSelect(void);
		BOOL ReceiveYTExe(void);
		BOOL ReceiveYTCancel(void);
		BOOL DoYKYT(BYTE Command);
		BOOL ReceiveSetTime(void);
		//virtual BOOL DoSend(void);
		BOOL SendYC_A(void);
		BOOL SendYC_B(void);
		BOOL SendYC_C(void);
		BOOL SendYC(BYTE m_bySendFrameType);
		WORD GetCdt92YC(WORD AINo);
		BOOL SendYX(void);
		BOOL SendDD(void);
		BOOL SendSoe(void);
		BOOL SendInsertInfo(void);
		BOOL SendCos(void);
		BOOL SendYKReturn(void);
		WORD EncodeSoe(void);
		WORD GetCosNum(WORD EqpID);
				
		void OnWriteCommEvent(void);
		virtual BOOL DoYKRet(void);	
		virtual void DoCommSendIdle();
};




#endif




