/*------------------------------------------------------------------------
 Module:		pcol.h
 Author:		solar
 Creation Date: 2008-10-20
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/
#ifndef _PCOL_H
#define _PCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"

#define PCOLCOMMBUFLEN	 1024 * 3

#define PCOL_FRAME_SHIELD   0xFFFF0000	   //������
#define PCOL_FRAME_OK	    0x00010000	   //��⵽һ��������֡
#define PCOL_FRAME_ERR	    0x00020000	   //��⵽һ��У������֡
#define PCOL_FRAME_LESS	    0x00030000	   //��⵽һ����������֡����δ���룩

BYTE Bch(BYTE *p, int l);
BYTE Lpc(BYTE SpecCode, BYTE *p, int l);
__inline BYTE Lrc(BYTE *p, int l)
{
	return Lpc(0x00,p,l);
}
WORD Crc16(WORD SpecCode, BYTE *p, int l);
WORD CheckSum(BYTE *buf, int l);

//��־λ����
#define PCOL_SYS_FLAG	    0	  //ϵͳ������־λ��
#define PCOL_SYS_DEVFLAG    0	  //ϵͳ������־λ��

#define PCOL_USER_FLAG	    32	  //�û���־λ��
#define PCOL_USER_DEVFLAG   32	  //�û���־λ�� 

//��־������ض���
#define MAX_FLAGNO	  128	//ÿ��ģ�����ı�־λ��

typedef struct 
{
	BYTE Flag[ MAX_FLAGNO/8 ]; 
}VFLAGS;


//========================================================================================================
//	��������: PSetFlag
//	�������: pFlags��m_Flags��������ַ  FlagNo����Ǻ� Status ����ǵ�״̬��ON/OFF
//	��������: ��Flags�������ڵĵ�FlagNo��λ��λ��Status
//	����ֵ��  ��
//========================================================================================================
void PSetFlag  (VFLAGS *pFlags,DWORD FlagNo);
void PClearFlag(VFLAGS *pFlags,DWORD FlagNo);
void PClearAllFlag(VFLAGS *pFlags);

//========================================================================================================
//	��������: PGetFlag
//	�������: pFlags��m_Flags��������ַ  FlagNo����Ǻ� 
//	��������: ��ȡm_Flags�������ڵĵ�FlagNo��λ�õ�״̬λ
//	����ֵ��  0: ��־λΪ��   ��0����־λ��Ϊ��
//========================================================================================================
DWORD PGetFlag(VFLAGS *pFlags,DWORD FlagNo);
BYTE ASCII2TOHEX(BYTE c1, BYTE c2);

typedef struct {
	WORD   wReadPtr;		//��ָ��
	WORD   wOldReadPtr; 	//��ָ��
	WORD   wWritePtr;		//дָ��
	BYTE   *pBuf;			//������
}VPcolCommBuf;

class VPcol{			
	private:
    public:
		VPcol();

		BOOL m_bHaveRxFrm;
		VPcolCommBuf m_Rec;
		VPcolCommBuf m_RecFrm;
		VPcolCommBuf m_Send;
		
		void NeatenCommBuf(VPcolCommBuf *pCommBuf); 		
		
		int DoReceive(struct timeval *tmval, int *flag);
		int procData(void);
		BOOL SearchFrame(void);
		void DoSend(DWORD dwFlag);

		virtual DWORD SearchOneFrame(BYTE *Buf,WORD Len);			
		virtual int ProcFrm(void);
		virtual int readComm(BYTE *buf, int len, struct timeval *tmval); 	
		virtual int writeComm(BYTE *buf, int len, DWORD dwFlag); 			
};

#ifdef __cplusplus
}
#endif

#endif
