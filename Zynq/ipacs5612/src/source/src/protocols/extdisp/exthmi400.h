// exthmi400.h

#ifndef EXTHMI400_H
#define EXTHMI400_H
#include "sys.h" 
#include "../public/pslave.h"	

#define EXTHMI400_TYPE_CONNECT     0x01
#define EXTHMI400_TYPE_PRSET       0x02
#define EXTHMI400_TYPE_IP          0x03
#define EXTHMI400_TYPE_YX          0x04
#define EXTHMI400_TYPE_LIGHT       0x04
#define EXTHMI400_TYPE_LIGHTMD     0x05
#define EXTHMI400_TYPE_YKMD        0x06
#define EXTHMI400_TYPE_PROG_WRITE  0x07
#define EXTHMI400_TYPE_HBEAT       0xA0

#define EXTHMI400_CTRL_MASTER     0x01
#define EXTHMI400_CTRL_REQPOLL    0x02
#define EXTHMI400_CTRL_RANGE      0x04
#define EXTHMI400_CTRL_SECT_1     0x08
#define EXTHMI400_CTRL_SECT_E     0x10
#define EXTHMI400_CTRL_Addr       0xC0    //by lvyi,判断分合闸面板地址


#define EXTHMI400_FUN_READ        0x00
#define EXTHMI400_FUN_WRITE       0x01
#define EXTHMI400_FUN_CTRL        0x02
#define EXTHMI400_FUN_CONF        0x03
#define EXTHMI400_FUN_DATA        0x04
#define EXTHMI400_FUN_END         0x05

#define EXTHMI400_ACK_OK          0
#define EXTHMI400_ACK_ERR         1

#define EXTHMI400_CODE1	       0xEB
#define EXTHMI400_CODE2	       0x90
#define EXTHMI400_CRCLEN          2


#define EXTHMI400_CTRL_MASTER     0x01

#define EXTHMI400_MODEL_DISP   0
#define EXTHMI400_MODEL_NODISP 1
#define EXTHMI400_MODEL_LAMP   2

#define  LOBYTE(w)     ((BYTE)(w))
#define  HIBYTE(w)     ((BYTE)((WORD)(w) >> 8))

#pragma pack(1)
typedef struct{
	BYTE byCode1;	
	BYTE byCode2;
	BYTE byLen;   
	BYTE byCnt;
	BYTE byFun;
	BYTE byType;
	BYTE byCtrl; 
}VEXTHMI400FrmHead;	

#pragma pack()

#define EXTHMI400_MAXFRM_LEN        256
#define EXTHMI400_MAXFRMDATA_LEN   (EXTHMI400_MAXFRM_LEN-sizeof(VEXTHMI400FrmHead)-1)

#define EXTHMI400_HBEAT_INTVAL      4 //5s
#define EXTHMI400_TIMEOUT           30//30s

#define EXTHMI400_BUF_MAX           2048

class CHMI400 : public CPSecondary
{
public:
	CHMI400();
	BOOL Init(WORD wTaskID);

public:
	BYTE m_byCnt;
	BOOL m_bIn;
	BYTE commcunt;
	VEXTHMI400FrmHead *m_pSend; 
	VEXTHMI400FrmHead *m_pRec;
	BYTE m_disp_model;
	BYTE yk_flag;
	VDBYK    m_YKmsg;
	DWORD m_dwSendAllDataEqpNo;
	
	virtual DWORD SearchOneFrame(BYTE *Buf,WORD Len);
	void SendFrameHead(BYTE fmType, BYTE fmFun, BYTE fmCtrl);
	void SendFrameTail();		
	virtual BOOL DoYKRet(void);
	virtual int atOnceProcSSOE(WORD wEqpNo);
	virtual void DoCommSendIdle(void);
	virtual BOOL DoReceive();
	virtual void DoTimeOut(void);
	
	WORD SendLight(DWORD light);
	void SendConnect(void);
	void SendConf(BYTE byType,BYTE code,BYTE DispAddr);// by lvyi 带上地址

	void ProcConnect(void);
	void ProcYx(void);
	void ProcProg(void);
	
	void SendLightMd(BYTE DispAddr);// by lvyi 带上地址
	void ProcYkMd(void);
	void procYKMsg(void);
	
};


#endif 
