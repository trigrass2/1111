/*------------------------------------------------------------------------
 $Rev: 2 $
------------------------------------------------------------------------*/

#ifndef MODBUSM_H
#define MODBUSM_H
	
#include "../public/pmaster.h"

#define MODBUS_MINFRAMELEN	  5
#define MAX_MODBUS_FRAME_SIZE	255

#define FUN_READ_COIL                   1
#define FUN_READ_DI                     2
#define FUN_READ_KEEPREG		3
#define FUN_READ_SINGLEREG              4
#define FUN_WRITE_COIL                  5
#define FUN_WRITE_SINGLEREG		6
#define FUN_WRITE_MUTIREG		16

#define REQ_YC				1
#define REQ_YX   			2
#define REQ_SOE 			3
#define REQ_SETTIME			4
#define REQ_DD				5
#define REQ_YK                          6

#define YK_SELECT_ADD                   11
#define YK_CLOSE_ADD                    12
#define YK_OPEN_ADD                     13
#define YK_CANCEL_ADD                   14
#define YK_OPEN_ACK                     15
#define YK_CLOSE_ACK                    16

#define REG_YX_START                    0x00
#define REG_YX_END                      0x50
#define REG_YX_NUM			            REG_YX_END- REG_YX_START + 1

#define REG_YC_START                    0x00
#define REG_YC_END                      0x23
#define REG_YC_NUM                      REG_YC_END - REG_YC_START + 1

#define YK_DATA_OPEN                    0xFF00
#define YK_DATA_CLOSE                   0x0000

#define REG_TIME_START                  0x032a
#define REG_TIME_END                    0x032f
#define REG_TIME_NUM                    REG_TIME_END - REG_TIME_START + 1

#define YXENABLE                        0x01
#define YCENABLE                        0x02
#define DDENABLE                        0x04
#define YKENABLE                        0x08

#define YX_BIT                          0x01
#define YX_WORD                         0x02
#define YC_WORD                         0x04
#define YC_DWORD                        0x08

#pragma pack(1)
struct VOpenModbusFrame
{
	BYTE byAddress ;
	BYTE byFunCode ;
	BYTE byData ;
};

struct VRegPara
{
	WORD wRegAddress ;
	union 
	{
	   WORD wValue ;
	   WORD wNum ; 
	}wData;
	WORD wRegNum ;
};


struct VFunPara
{
	BYTE byFunCode ;
	BYTE byRegGroup ;
	struct VRegPara  Reg[10];
};

struct VModbusPara
{
	BYTE byModbusCfg ;  /*D0:Yx,
	                             D1:Yc,
	                             D2:Dd,
	                             D3:Yk,
	                           */
	struct VFunPara YXPara ;
	struct VFunPara YCPara ;
	struct VFunPara DDPara ;
	struct VFunPara YKPara ;
};
#pragma pack()

class COPENMODBUSP : public CPPrimary
{
	public:
		VOpenModbusFrame * m_pReceive; 	//接收帧头指针
		VOpenModbusFrame * m_pSend;  		//发送帧头指针
		VModbusPara  *m_pCfg;
		BYTE *m_pbyData;
		BYTE m_byReqDataType;
		BYTE m_byNextReq;
		BYTE m_RegNum;
		BYTE m_RegNow;
		BYTE m_byYkStatus;
		BYTE m_byYCStatus;
		BYTE m_byYXStatus;
		BYTE m_byDDStatus;
		WORD YKRegAddr;
		WORD YKSetValue;
		BYTE m_SendAddress;


	public:
		COPENMODBUSP();
		BOOL Init(WORD wTaskID);
		virtual void CheckBaseCfg(void);
		virtual void SetBaseCfg(void);
    virtual void CheckCfg(void);
     
		virtual BOOL DoReceive(void);
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);

		virtual BOOL ReqUrgency(void);
		virtual BOOL ReqCyc(void);
		virtual BOOL ReqNormal(void);
		void SendFrameHead(BYTE FunCode, BYTE ShiftAddress);
		void SendFrameTail(void);

		void SendSetTime(void);
		void SendReadCmd(BYTE FunCode, WORD RegAddr, BYTE ShiftAddress, WORD Length);
		void SendWriteCmd(BYTE FunCode, WORD RegAddr, BYTE ShiftAddress, WORD Value);
		void SendReadSReg(WORD RegAddr,  WORD RegNum, BYTE ShiftAddress);
		void SendReadDI(WORD RegAddr, WORD RegNum, BYTE ShiftAddress);
		void SendReadMReg(WORD RegAddr, WORD RegNum, BYTE ShiftAddress);
		void SendWriteCoil(WORD RegAddr, WORD value, BYTE SHiftAddress);
		void SendWriteSReg(WORD RegAddr, BYTE ShiftAddress, WORD Value);
		void SendWriteMReg(WORD RegAddrStart, BYTE RegNum, BYTE ShiftAddress, WORD *Value);

		void DoYcData(void);
		void DoDdData(void);
		void DoYxData(void);
		virtual BOOL DoYKReq(void);
		void YkResult(BOOL Result);
		virtual void DoTimeOut(void);
        void DoYKData(void);
		
		virtual BOOL DoSend(void);

};

#endif
