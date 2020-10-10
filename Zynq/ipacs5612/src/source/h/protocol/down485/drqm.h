#ifndef DLQM_H_
#define DLQM_H_

#include "pmaster.h"
//#include "DataBase_Mqtt.h"
//#include "mqttdb.h"


#define CMD_TEST 					0X00
#define CMD_READ_STATE				0X01
#define CMD_READ_PARA				0X02
#define CMD_READ_DATA				0X03
#define CMD_READ_ACTDATA			0X04
#define CMD_BOARDCAST_SETPARA		0X10
#define CMD_BOARDCAST_CHANGEMODE	0X11
#define CMD_YK						0X12

#define CMD_CLEAR_ACTDATA			0X13
#define CMD_READ_HOSTDATA			0X20
#define CMD_READ_HOSTADDRQUE		0X21
#define CMD_READ_HOSTSTATE			0X22

#define FRAME_TYPE_BOARDCAST		0X01
#define FRAME_TYPE_SLAVE			0X02
#define FRAME_TYPE_HOST				0X03

#define DRQ_FRAME_LEN    4

typedef union FUNCODE
{
	struct
	{
		BYTE	CMD:7;
		BYTE	DIR:1;
	}__attribute__ ((packed)) bit;
	BYTE all;
}__attribute__ ((packed)) FUNCODE;	//-功能码-

typedef struct DRQFRAMEHEAD
{
	BYTE ID;
	FUNCODE FUN;
	BYTE LEN;
	BYTE DATA;
}DrqHead;

typedef struct DRQPARA
{
	WORD Uup;
	WORD Ulow;
	WORD PTin;
	WORD ActDelay;
	WORD Tup1;
	WORD X_Tup2;
	WORD Qup_ON;
	WORD Qup_OFF;
	WORD Udef;
	WORD PF;
	WORD Uha;
	WORD Iha;
	WORD REQ[4];
}DrqPara;

class CDRQM : public CPPrimary
{
	public:
		DrqHead * m_pReceive; 	//接收帧头指针
		DrqHead * m_pSend;  		//发送帧头指针
		//char *m_pCfg;
		BYTE *m_pData;
		BYTE m_ID;
		BYTE m_Rs485_id;
		DrqPara m_DrqPara;
		char *m_TempBuf;
		BYTE m_ReadFlag;
		DWORD m_DelayTime;
		BYTE *m_Modle;
		BYTE *m_State;
	public:
		CDRQM();
		BOOL Init(WORD wTaskID);   
		virtual BOOL DoReceive(void);
		virtual DWORD SearchOneFrame(BYTE *Buf, WORD Len);
		virtual BOOL ReqUrgency(void);
		virtual BOOL ReqCyc(void);
		virtual BOOL ReqNormal(void);
		virtual BOOL DoYKReq(void);
		virtual void DoTimeOut(void);
		virtual BOOL DoSend(void);
 		void SendFrameHead(BYTE FunCode, BYTE ShiftAddress);
		void SendFrameTail(void);		
		void SendTest(void);
		void SendReadState(void);
		void SendReadPara(void);
		void SendReadData(void);
		void SendReadActData(void);
		void SendSetPara(void);
		void SendChangeYKMode(BYTE mode);
		void SendYK(BYTE state);
		void SendClearActData(void);
		void SendReadHostData(void);
		void SendReadHostDataQue(void);
		void SendReadHostState(void);
		void DoTest();
		void DoYk();
		void DoReadHostData();
		void DoReadHostAddrQue();
		void DoReadHostState();
		void DoClearData();
		void DoReadActData();
		void DoReadData();
		void DoReadPara();
		void DoReadState();
		void YkResult(BOOL Result);

		int SendMqttYC(float *YC,WORD wBeginNo,WORD wNum,WORD wEqpId);
		int SendMqttSYX(BYTE *YX,WORD wBeginNo,WORD wNum,WORD wEqpId);
};

#endif
