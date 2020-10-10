#ifndef _EXTMMI_H
#define _EXTMMI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "pcol.h"

#define EXTMMI_TYPE_CONNECT    0x01
#define EXTMMI_TYPE_PRSET      0x02
#define EXTMMI_TYPE_IP         0x03
#define EXTMMI_TYPE_DIO        0x04
#define EXTMMI_TYPE_LIGHT      0x05
#define EXTMMI_TYPE_PROG_WRITE 0x07

#define EXTMMI_TYPE_HBEAT     0xA0

#define EXTMMI_CTRL_MASTER     0x01
#define EXTMMI_CTRL_REQPOLL    0x02
#define EXTMMI_CTRL_RANGE      0x04
#define EXTMMI_CTRL_SECT_1     0x08
#define EXTMMI_CTRL_SECT_E     0x10

#define EXTMMI_FUN_READ        0x00
#define EXTMMI_FUN_WRITE       0x01
#define EXTMMI_FUN_CTRL        0x02
#define EXTMMI_FUN_CONF        0x03
#define EXTMMI_FUN_DATA        0x04
#define EXTMMI_FUN_END         0x05

#define EXTMMI_ACK_OK          0
#define EXTMMI_ACK_ERR         1

#define EXTMMI_CODE1	    0xEB
#define EXTMMI_CODE2	    0x90
#define EXTMMI_CRCLEN       2


#pragma pack(1)
typedef struct{
	BYTE byCode1;	
	BYTE byCode2;
	BYTE byLen;   
	BYTE byCnt;
	BYTE byFun;
	BYTE byType;
	BYTE byCtrl; 
}VExtMMIFrmHead;	
typedef struct
{
   BYTE I1Set[2];
   BYTE I0Set[2];
   BYTE T0Set[2];
   BYTE I2Set[2];
   BYTE T2Set;
}VMMISet;
typedef struct
{
   BYTE I2Set;
   BYTE T2Set[2];
   BYTE U0Set;
   BYTE I0Set[2];
   BYTE T0Set[2];
}VMMISet_1;
#pragma pack()

#define EXTMMI_MAXFRM_LEN        128
#define EXTMMI_MAXFRMDATA_LEN   (EXTMMI_MAXFRM_LEN-sizeof(VExtMMIFrmHead)-1)

#define EXTMMI_HBEAT_INTVAL      4 //5s
#define EXTMMI_TIMEOUT           30//30s

#define EXTMMI_BUF_MAX           2048

#define JBMMI_FILE_PATH         "/tffsb/jb/rcd"
#define JBMMI_FILE_CFG          "/tffsb/jb/rcd.cfg"
#define JBMMI_FILE_HDR          "/tffsb/jb/rcd.hdr"
#define JB_FILE_RCD_OFFSET      23
#define JB_FILE_RCD_LEN         29
#define JB_FILE_MAX             6

typedef struct
{
    WORD  freq;
	WORD  count;
    struct VCalClock time[2];
}VRcdHead;

class VPcol;
class VExtMMI: public VPcol{
	private:
	
	public: 		
		VExtMMI();

		int m_thid;
		int m_commid;

		VExtMMIFrmHead *m_pSend; 
		VExtMMIFrmHead *m_pRec;		

		BYTE m_byCnt;
		int m_nNoFrm;
		BOOL m_bIn;  //mmi∞Â‘⁄œﬂ
		BOOL m_bLight;
		BOOL m_filests;
		BOOL m_datatrans;
		BOOL m_first;
		BYTE m_bCommCnt;
		BYTE m_bBeatCnt;
		DWORD m_dwYx;
		DWORD m_dwCount;
		DWORD m_dwCommCtrlTimer;
		VMsg DispMsg;
		DWORD m_dwOffset;

		BYTE m_byBuf[EXTMMI_BUF_MAX];

		virtual DWORD SearchOneFrame(BYTE *Buf,WORD Len);

		void SendFrameHead(BYTE fmType, BYTE fmFun, BYTE fmCtrl);
		void SendFrameTail();		
		
		virtual int readComm(BYTE *buf, int len, struct timeval *tmval); 			
		virtual int writeComm(BYTE *buf, int len, DWORD dwFlag);

		virtual int ProcFrm(void);	

		void SendLight(void);
		void SendConnect(void);
		void SendHBeat(void);
		void SendConf(BYTE byType,BYTE code);

		void ProcPr(void);
		void ProcIp(void);
		void ProcConnect(void);
		void ProcHBeat(void);
		void ProcLight(void);
		void ProcYx(void);

		void DoTimeOut(void);

		void Run(DWORD events);
		void SendTest(void);
		void DoTSDataMsg();
		void ProcProg(void);

};

#ifdef __cplusplus
}
#endif

#endif
