/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef _CDT92M_H
#define _CDT92M_H

#include "cdtmaster.h"

#define CODE_YK_SELECT		0x61  
#define CODE_YK_EXE 		0xc2  
#define CODE_YK_CANCEL		0xB3  
#define CODE_YT_SELECT		0xF4  
#define CODE_YT_EXE 		0x85  
#define CODE_YT_CANCEL		0x26  
#define CODE_TIMESET		0x7A

#define FUN_YK_SELECT		0xe0
#define FUN_YK_RET			0xe1
#define FUN_YK_EXE			0xe2
#define FUN_YK_CANCEL		0xe3
#define FUN_YT_SELECT		0xe4
#define FUN_YT_RET			0xe5
#define FUN_YT_EXE			0xe6
#define FUN_YT_CANCEL		0xe7

#define REC_YC_A			0x61
#define REC_YC_B			0xc2
#define REC_YC_C			0xb3
#define REC_YX	 			0xf4
#define REC_DD 				0x85
#define REC_SOE				0x26
#define REC_YKRETURN		0x9e

#define YK_SET_CLOSE		0xcc
#define YK_SET_OPEN			0x33
#define YK_EXE				0xaa
#define YK_CANCEL			0x55
#define YK_FAIL				0xff

#define CONTROL_BYTE		0x71

#define OLDSOE_NUM 		8
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

class CCDT92P:public CCDTPri
{

	public:
		VSoeFrame 	  SoeFrame;		/*上次得到的SOE*/
		VSoeFrame	  m_OldSoe[OLDSOE_NUM];
		BYTE	m_OldSoe_wp;
		WORD	m_wYear;			/*用来补全SOE时标*/
		BYTE	m_byMonth;
		BOOL	m_bWaitSoePart2;	/*等待接收SOE第2信息字*/
	public:
		
		CCDT92P();
		BOOL Init(WORD wTaskID);
		virtual void SetBaseCfg(void);
		virtual BOOL DoReceive(void);

		BOOL ReceiveYC(void);
		BOOL ReceiveYX(void);
		BOOL ReceiveDD(void);
		int ReceiveSOE(void);				
		BOOL DoYKReturn(void);
		virtual BOOL DoYKReq(void);
		int  SendYK(void);
		int  MyYKReturn(int result);
		virtual BOOL ReqCyc(void);
		void SendSetTime(void);
};




#endif

