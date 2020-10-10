/*------------------------------------------------------------------------
功能:      加密报文处理
日期:      20170331
编辑人: 桑士振
版本:      V1.0
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef ECC_MODE_CHIP
#include "os.h"
#include "cmd.h"
#include "sys.h"


#include "sgc1126a.h"
#include "SecurityProcessor.h"

typedef struct VIec104FrameSecurity
{
	BYTE byStartCode;
	BYTE byAPDULen;
	BYTE byControl1;
	BYTE byControl2;
	BYTE byControl3;
	BYTE byControl4;
	BYTE byASDU[255-6];	
}tIec104FrameSecurity;


#define FRAME_OK       0x00010000      //检测到一个完整的帧
#define FRAME_ERR      0x00020000      //检测到一个校验错误的帧
#define FRAME_LESS     0x00030000      //检测到一个不完整的帧（还未收齐）

static BYTE ProccessBuf[400];
static BYTE ProccessBuf2[1000];//解密使用
#if  0
static BYTE GateWayRandom[8];
static BYTE MasterStaRandom[8];
#endif

static BYTE IVData[16];
static BYTE MyRandom[8];
static BYTE ToolID[8];
int GateWayFrameIndex = 0;
int MasterStaFrameIndex = 0;
int YunweiFrameIndex = 0;//
int SecurityCheckOk = 0;
BYTE MaintFlag = 0;
//
int WriteCerFinish = 0;
BOOL SendCerFlag = FALSE; 
BOOL SendMaintToolCerFlag = FALSE;
BYTE SendReturnData = 0;
BYTE bySendCerFrameIndex = 0;
BYTE bySendCerFrameCount = 0;
static BYTE ProccessCerBuf[1000];
static WORD wCerDataLen;
static BYTE MD5SVerData[200];
static BYTE MD5asKID = 0; 
static BYTE MD5_Decrypt[17];
WORD MD5Signlen = 0;
WORD MD5_ERRTYPE = 0x9000; //验证包是否ok ,返回必须在升级结束才返回

//
#define FRAME_HasProccess     0x00040000      
#define SendCerDataLen 200

//
BYTE SecurityChkSum(BYTE *buf, WORD len);
BOOL SecurityChipID(BYTE* rBuf, WORD *rLen);
BOOL SecurityKeyVersion(BYTE* rBuf, WORD *rLen);
BOOL SecurityGenRandom(BYTE* rBuf, WORD *rLen);
BOOL SecurityDecrypt(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityMaintToolDecrypt(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityEncrypt(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityMaintToolEncrypt(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityCheckSign(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityCheckSign1(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityMaintToolCheckSign(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityCreateSign(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityMaintToolCreateSign(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);

BOOL SecurityUpdateKey(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen,
	BYTE byUpdateFlag);
BOOL SecurityUpdateCer(BYTE cerID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen,
	BYTE byUpdateFlag);
BOOL SecurityLoadCer(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen,
	BYTE byUpdateFlag);
BOOL SecurityGetTestCerLen(BYTE *rBuf, WORD *rLen);
BOOL SecurityGetWorkCerData(BYTE *rBuf, WORD *rLen);
BOOL SecurityGetTestCerData(WORD DataLen,BYTE *rBuf, WORD *rLen);
void SecurityProcessFrame(BYTE *sendBuf,WORD sendLen);
void SecurityUnlink(void);
BOOL SecurityProcessType20(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType22(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType30(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType32(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType34(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType36(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType38(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType3A(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType3C(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType3E(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType40(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType42(BYTE *sendBuf,WORD *pSendLen);
int SecuritySendType43(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType44(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecuritySendType45(WORD sw,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType46(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType48(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType4A(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType4B(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType4D(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);

BOOL SecurityProcessType50(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType52(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType54(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType60(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType62(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType64(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType70(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType72(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType74(BYTE *buf,BYTE *cerBuf,WORD *cerLen);
int SecuritySendType75(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType66(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType68(BYTE *buf,BYTE *cerBuf,WORD *cerLen);

BOOL SecuritySendType1F(WORD sw,BYTE *sendBuf,WORD *pSendLen);
BOOL SecuritySendType(BYTE type,WORD sw,BYTE *sendBuf,WORD *pSendLen);
BOOL SecuritySendTypeYunWei(BYTE type,WORD sw,BYTE *sendBuf,WORD *pSendLen);

//运维认证跳过标志
void SecurityMaintConfirm(BYTE flag)
{
	MaintFlag = flag;
}

//检测到一个不完整的帧（还未收齐）
BYTE SecurityChkSum(BYTE *buf, WORD len)
{
	WORD checksum, i;

	checksum = 0;
	for (i = 0; i < len; i++)
	{
		checksum = checksum + *buf;
		buf ++;
		
	}
	return LOBYTE(checksum & 0xff);
}

DWORD SecuritySearchOneFrame(BYTE *Buf, WORD Len)
{
	WORD off = 0;
	WORD frameLen;
	BYTE checksum;
	
	if(Len < 8)
		return FRAME_LESS;

	

	if(0xEB == Buf[off] &&0xEB == Buf[off+3])
	{
		frameLen = MAKEWORD(Buf[2],Buf[1]);
		 frameLen+=6;

		 if(frameLen > 1024)
		 {
		 	WriteDoEvent(NULL, 0, "SecuritySearchOneFrame len %d ",frameLen);
		 	return (FRAME_ERR | 1);
		 }
		 
		if(Len < frameLen)
			return FRAME_LESS;
		else 
		{
              if(Buf[6] == 0x46)
                return (FRAME_OK|frameLen);
			if(0xD7 == Buf[frameLen-1])
			{
				checksum = SecurityChkSum(&Buf[4],frameLen-6);
				if(Buf[frameLen-2] == checksum)
					return  FRAME_OK|frameLen;
				else
					return  FRAME_ERR|frameLen;
			}
			else
				return  FRAME_ERR|frameLen;
		}
	}
	else
	{
		off = 0;	
		while(off < Len)
		{
			if(0xEB == Buf[off])
				break;
			off++;
		}
		if(off == 0)
			off = 1;
		return FRAME_ERR|off;
	}
}
/******************************************************************
*	// protocolType:1-104、2-101
*
*	返回值Ret:
*		0、报文错误。
*		1、报文正确，是规约业务报文，需要进一步处理。
*		2、报文正确，不需要进一步处理，直接封装发送。
*		3、报文正确，不需要进一步处理，不需要发送。
*		4、报文正确，需要分帧多次发送。
*		5、报文正确，有两帧，发送，发一帧，查询一帧
******************************************************************/

int SecurityAllFrameProcess(WORD commno , BYTE *Buf, WORD Len,BYTE *sendBuf, WORD *sLen,
BYTE protocolType,BYTE linkaddlen,BYTE * type)  //cjl  type 应用类型
{
	BOOL bFlag;
	int Ret = 0;
	tSecurityFrame *pFrame = (tSecurityFrame *)Buf;
	BYTE *pMingwen;
	WORD Jiami;
	WORD RevLen,RevLen2;
	BYTE asKID;
	WORD sendLen,sendLen2;
	int offset =0;
	WORD FrameType;
	WORD FrameLen;
	WORD sw;
	WORD ErrorType = 0;
	int index;
	int i=0;
	struct VSysClock SysTime;
	struct VCalClock CalTime1,CalTime2;
	WORD wSecurityDataLen;
	int temp;

	//
	if(pFrame->Start1 != 0xEB||pFrame->Start2 != 0xEB)
	{
		return Ret;
	}

	//
	FrameType = MAKEWORD(Buf[5],Buf[4]);
	FrameLen = MAKEWORD(Buf[2],Buf[1]);
	
	if(FrameLen > (Len-6))
	{
		return Ret;
	}
	//
	
	Jiami = FrameType&0x8;
	if(Jiami)
	{
		//
		sendLen = FrameLen-2;
		if((FrameType&0xA)==0xA)
			bFlag = SecurityMaintToolDecrypt(pFrame->Data,sendLen,ProccessBuf,&RevLen);
		else
			bFlag = SecurityDecrypt(pFrame->Data,sendLen,ProccessBuf,&RevLen);
		if(!bFlag)
		{
			sw = 0x9103;
			SecuritySendType1F(sw,sendBuf+4,sLen);
			SecurityProcessFrame(sendBuf,*sLen);
			*sLen += 6;
			Ret = 2;
			return Ret;
		}
		for(i = 0; i < RevLen;i++)
			shellprintf("%02X ",ProccessBuf[i]);
		shellprintf("\n");
		pMingwen = ProccessBuf;
	}
	else
	{
		pMingwen = Buf+6;
		RevLen = FrameLen -2;
	}

	
	//
	sendLen = pMingwen[1];
	
	//check
	if(pMingwen[0]<0x20)
	{
						
		if(!SecurityCheckOk)
		{
			ErrorType = 0x9107;
			goto ERR;
		}
		if(pMingwen[0] != 0x08)
		{
		if((pMingwen[2+5+linkaddlen]==0x2d)||(pMingwen[2+5+linkaddlen] ==0x2e) ||(pMingwen[2+5+linkaddlen] == 0xCB)) 
		{
			if(Jiami == 0)
			{
				ErrorType = 0x9106;
				goto ERR;
			}
		}

		if(pMingwen[2+5+linkaddlen]== 0xCB) //不带签名的参数固化不知为何返回了0x9104,加了这句话 
		{
			if((pMingwen[0] != 0x01 ) && (pMingwen[0] != 0x03 ))
			{
				ErrorType = 0x9101;
				goto ERR;
			}
		}

		if((pMingwen[2+5+linkaddlen]==0x2d)||(pMingwen[2+5+linkaddlen] ==0x2e)) //遥控业务应用类型错误
		{
			if((pMingwen[0] != 0x05 ) && (pMingwen[0] != 0x07 ))
			{
				ErrorType = 0x9101;
				goto ERR;
			}
		}

		if(pMingwen[2+5+linkaddlen]== 0xC8)  //切换定制区 
		{
			if(pMingwen[0] != 0x01 )
			{
				ErrorType = 0x9101;
				goto ERR;
			}
		}
		}
		
		ErrorType = 0;
		if(pMingwen[0] != 0x08)
		{
			index = 2+sendLen;
			wSecurityDataLen = MAKEWORD(pMingwen[index+1], pMingwen[index]);

			if((RevLen-2-sendLen - 2) != wSecurityDataLen)
			{
				ErrorType = 0x9110;
				goto ERR;
			}
		}
		else
		{
			index = 1;
			wSecurityDataLen = MAKEWORD(pMingwen[index+1], pMingwen[index]);

			if((RevLen-3) != wSecurityDataLen)
			{
				MD5_ERRTYPE= 0x9110;
				return FALSE;
			}
		}

			
		switch(pMingwen[0])
		{
			case 0x1://签名
			{
				if(wSecurityDataLen<65)
				{
					ErrorType = 0x9110;	
					break;
				}
				if(Jiami == 0)
				{
					ErrorType = 0x9106;
					break;
				}
				break;
			}
			case 0x02:
			case 0x03:
			{
				if(wSecurityDataLen<8)
				{
					ErrorType = 0x9110;
					break;
				}
				if(Jiami == 0)
				{
					ErrorType = 0x9106;
					break;
				}
				index = 2+sendLen+2;
				for(i=0;i<6;i++)
				{
					if(pMingwen[index+i] != MyRandom[i])
						break;
				}
				if(i<6)
					ErrorType = 0x9104;	
				
				break;
			}
			/*case 0x3://随机数
			{
				if(wSecurityDataLen<71)
				{
					ErrorType = 0x9110;
					break;
				}
				if(Jiami == 0)
				{
					ErrorType = 0x9106;
					break;
				}
				index = 2+sendLen+2;
				for(i=0;i<8;i++)
				{
					if(pMingwen[index+i] != MyRandom[i])
						break;
				}
				if(i<8)
					ErrorType = 0x9104;	
				break;
			}*/
			case 0x04:
			case 0x5://时间
			{
				
				if(pMingwen[0] == 4)
				{
					if(wSecurityDataLen<6)
					{
						ErrorType = 0x9110;
						break;
					}
				}
				
				if(pMingwen[0] == 5)
				{
					if(wSecurityDataLen<71)
					{
						ErrorType = 0x9110;
						break;
					}
				}
				if(Jiami == 0)
				{
					ErrorType = 0x9106;
					break;
				}

				index = 2+sendLen+2;
				//
				SysTime.wYear = 2000+pMingwen[index];
				SysTime.byMonth = pMingwen[index+1];
				SysTime.byDay= pMingwen[index+2];
				SysTime.byHour= pMingwen[index+3];
				SysTime.byMinute= pMingwen[index+4];
				SysTime.bySecond = pMingwen[index+5];
				SysTime.wMSecond= 0;
				GetSysClock(&CalTime2, CALCLOCK);
				ToCalClock(&SysTime,&CalTime1);
				temp = CalTime2.dwMinute - CalTime1.dwMinute;
				temp = temp*60;
				temp = temp + (CalTime2.wMSecond- CalTime1.wMSecond)/1000;
				
				if(temp > 60||temp < -60)
				{
					ErrorType = 0x9105;
				}
				break;
			}
			case 0x6:
			case 0x7://随机数、时间
			case 0x08:
			{
				if(pMingwen[0] == 6)
				{
					if(wSecurityDataLen<14)
					{
						ErrorType = 0x9110;
						break;
					}
				}
				if(pMingwen[0] == 7)
				{
					if(wSecurityDataLen<79)
					{
						ErrorType = 0x9110;
						break;
					}						
				}
				
				if(pMingwen[0] == 8)
				{
					if(wSecurityDataLen<79)
					{
						MD5_ERRTYPE= 0x9110;
						return FALSE;
					}						
				}
				
				if(Jiami == 0)
				{
					if(pMingwen[0] == 8)
					{
						MD5_ERRTYPE = 0x9106;
						return FALSE;
					}
					else
					{
						ErrorType = 0x9106;
						break;
					}
				}

				if(pMingwen[0] != 0x08)
					index = 2+sendLen+2;
				else
					index = 3;
				SysTime.wYear = 2000+pMingwen[index];
				SysTime.byMonth = pMingwen[index+1];
				SysTime.byDay= pMingwen[index+2];
				SysTime.byHour= pMingwen[index+3];
				SysTime.byMinute= pMingwen[index+4];
				SysTime.bySecond = pMingwen[index+5];
				SysTime.wMSecond= 0;
				GetSysClock(&CalTime2, CALCLOCK);
				ToCalClock(&SysTime,&CalTime1);
				temp = CalTime2.dwMinute - CalTime1.dwMinute;
				temp = temp*60;
				temp = temp + (CalTime2.wMSecond- CalTime1.wMSecond)/1000;
				
				if(temp > 60||temp < -60)
				{
					if(pMingwen[0] == 8)
					{
						MD5_ERRTYPE = 0x9105;
						return FALSE;
					}
					else
					{
						ErrorType = 0x9105;
						break;
					}
				}
				//
				index += 6;
				for(i=0;i<6;i++)
				{
					if(pMingwen[index+i] != MyRandom[i])
						break;
				}
				if(i<6)
				{
					if(pMingwen[0] == 8)
					{
						MD5_ERRTYPE = 0x9104;
						return FALSE;
					}
					else
					{
						ErrorType = 0x9104;
						break;
					}
				}
			}
		}
ERR:
		if(ErrorType>0)
		{
			sw = ErrorType;
			SecuritySendType1F(sw,sendBuf+4,sLen);
			SecurityProcessFrame(sendBuf,*sLen);
			*sLen += 6;
			Ret = 2;
			return Ret;
		}
	}

	/*if(pMingwen[0] >= 0x20) //2017年5月2日14:28:23 cjl 判断非业务类型长度
	{
		sendLen = MAKEWORD(pMingwen[2],pMingwen[1]);
		if(Jiami == 0)
		{
			if((FrameLen - 5) != sendLen)
			{
				sw = 0x9110;
				SecuritySendType(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				Ret = 2;
				return Ret;
			}
		}
		
	}*/

		
	switch(pMingwen[0])
	{
		case 0x0:
		{
			memcpy(Buf,&pMingwen[2],sendLen);
			Ret = 1;
			break;
		}
		case 0x1://签名
		case 0x3://随机数
		case 0x5://时间
		case 0x7://随机数、时间
		{
			if(RevLen<67)
				return Ret;
			asKID = pMingwen[RevLen-1];
			
			sendLen = pMingwen[1];
			offset = 2;
			memcpy(ProccessBuf2,&pMingwen[offset],sendLen);
			offset += sendLen;
			sendLen2 = MAKEWORD(pMingwen[offset+1],pMingwen[offset]);
			offset += 2;
			memcpy(ProccessBuf2+sendLen,&pMingwen[offset],sendLen2-1);
			sendLen = RevLen -5;
			
			if(SecurityCheckSign1(asKID,ProccessBuf2,sendLen,ProccessCerBuf,&RevLen2))
			{
				memcpy(Buf,&pMingwen[2],pMingwen[1]);
				Ret = 1;
				break;
			}
			else
			{
				sw = 0x9102;
				SecuritySendType1F(sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				Ret = 2;
				return Ret;
			}
		}
		case 0x08:
		{
			if(RevLen<67)
				return Ret;
			
			memset(MD5_Decrypt,0,sizeof(MD5_Decrypt));
			MD5asKID = pMingwen[wSecurityDataLen +2];
			memcpy(MD5SVerData,pMingwen+3,wSecurityDataLen);
			MD5Signlen = wSecurityDataLen-1;
			MD5_Final(MD5_Decrypt);

			memcpy(ProccessCerBuf,MD5_Decrypt,16);
			memcpy(ProccessCerBuf+16,MD5SVerData,MD5Signlen);	
			if(SecurityCheckSign1(MD5asKID,ProccessCerBuf,MD5Signlen+16,ProccessBuf,&RevLen2))
			{
				MD5_ERRTYPE= 0x9000;
			}
			else
			{
				MD5_ERRTYPE = 0x9102;
			}
			return FALSE;
		}
		case 0x2:
		case 0x4:
		case 0x6:
		{
			Ret  = 1;
			break;
		}
		//网关认证
		case 0x20:
		{
			GateWayFrameIndex = 0;
			bFlag = SecurityProcessType20(pMingwen+1,sendBuf+4,sLen);
#if 0				
				bFlag = SecurityGetTestCerLen(ProccessCerBuf,sLen);
				if(!bFlag)
				return FALSE;
				Len = MAKEWORD(ProccessCerBuf[1],ProccessCerBuf[0]);
				//
				bFlag = SecurityGetTestCerData(Len,sendBuf+4,sLen);
#endif					
			if(!bFlag)
			{
				GateWayFrameIndex = 0;
				return FALSE;
			}
	
			//成功
			if(bFlag)
			{
				GateWayFrameIndex = 1;
				Ret = 2;
			}
			break;
		}
		case 0x22:
		{
			//if(GateWayFrameIndex == 1)
			{
				bFlag = SecurityProcessType22(pMingwen+1,sendBuf+4,sLen);

				//成功
				if(bFlag)
				{
					GateWayFrameIndex = 2;
					Ret = 2;
				}
				else
				{
					GateWayFrameIndex = 0;
					if(*sLen>0)
					{
						Ret = 2;
					}
				}
			}
			break;
		}
		//证书模式认证
		case 0x30:
		{
			YunweiFrameIndex = 0;
			bFlag = SecurityProcessType30(pMingwen+1,sendBuf+4,sLen);

			if(bFlag)
			{
				YunweiFrameIndex = 1;
				Ret = 5;
			}
			else
			{
					YunweiFrameIndex = 0;
					Ret = 2;
			}

			break;
		}
		//公钥模式认证
		case 0x4A:
		{
			YunweiFrameIndex = 0;
			bFlag = SecurityProcessType4A(pMingwen+1,sendBuf+4,sLen);

			if(bFlag)
			{
				YunweiFrameIndex = 1;
				Ret = 5;
			}
			else
			{
					YunweiFrameIndex = 0;
					Ret = 2;
			}

			break;
		}
		case 0x32:
		{
			bFlag = SecurityProcessType32(pMingwen+1,sendBuf+4,sLen);
	
			//成功
			if(bFlag)
				YunweiFrameIndex = 2;
			else 
				YunweiFrameIndex = 0;

			Ret = 2;
			break;
		}
		case 0x34:
		{
			if(YunweiFrameIndex != 2)
			{
				return FALSE;
			}
			bFlag = SecurityProcessType34(sendBuf+4,sLen);
			if(!bFlag)
				return FALSE;
	
			Ret = 2;
			break;
		}
		case 0x36:
		{
			if(YunweiFrameIndex != 2)
			{
				return FALSE;
			}
			
			bFlag = SecurityProcessType36(sendBuf+4,sLen);
			if(!bFlag)
				return FALSE;
	
			Ret = 2;
			break;
		}
		case 0x38:
		{
			if(YunweiFrameIndex != 2)
			{
				return FALSE;
			}
			
			bFlag = SecurityProcessType38(sendBuf+4,sLen);
			if(!bFlag)
				return FALSE;
	
			Ret = 2;
			break;
		}
		case 0x3A:
		{
			if(YunweiFrameIndex != 2)
			{
				sw = 0x9090;
				SecuritySendTypeYunWei(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			
			bFlag = SecurityProcessType3A(sendBuf+4,sLen);
			if(!bFlag)
				return FALSE;
	
			Ret = 2;
			break;
		}
		case 0x3C:
		{
			if(YunweiFrameIndex != 2)
			{
				sw = 0x9090;
				SecuritySendTypeYunWei(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}

			bFlag = SecurityProcessType3C(pMingwen+1,sendBuf+4,sLen);
	
			if(bFlag)
				Ret = 5;
			else
				Ret = 2;

			break;
		}
		case 0x3E:
		{//证书导入和导出操作不需要进行身份认证
			bFlag = SecurityProcessType3E(pMingwen+1,sendBuf+4,sLen);

			
			if(bFlag)
				Ret = 5;
			else
				Ret = 2;
			break;
		}
		case 0x40:
		{
			
			if(YunweiFrameIndex != 2)
			{
				sw = 0x9090;
				SecuritySendTypeYunWei(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			
			bFlag = SecurityProcessType40(sendBuf+4,sLen);
			if(*sLen>0)
				Ret = 2;			
			break;
		}
		case 0x42:
		{//证书导入和导出操作不需要进行身份认证
			
			bFlag = SecurityProcessType42(sendBuf+4,sLen);
			if(*sLen > 0)
				Ret = 4;
			break;
		}
		case 0x44:
		{
			if(YunweiFrameIndex != 2)
			{
				sw = 0x9094;
				SecuritySendTypeYunWei(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			
			bFlag = SecurityProcessType44(pMingwen+1,sendBuf+4,sLen);
			
			break;
		}
		case 0x46:
		{
			if(YunweiFrameIndex != 2)
			{
				sw = 0x9092;
				SecuritySendTypeYunWei(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			bFlag = SecurityProcessType46(pMingwen+1,sendBuf+4,sLen);
			Ret = 2;
			break;
		}
		case 0x48:
		{
			if(YunweiFrameIndex != 2)
			{
				sw = 0x9107;
				SecuritySendTypeYunWei(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			
			bFlag = SecurityProcessType48(pMingwen+1,sendBuf+4,sLen);

			if(bFlag)
				Ret = 2;
			break;
		}
		case 0x4B: // 将根CA公钥导入终端
		{
			bFlag = SecurityProcessType4B(pMingwen+1,sendBuf+4,sLen);
			Ret = 2;
			break;
		}
		case 0x4D:
		{
			bFlag = SecurityProcessType4D(pMingwen+1,sendBuf+4,sLen);
			Ret = 2;
			break;
		}
		//主站认证
		case 0x50:
		{
			MasterStaFrameIndex = 0;
			SecurityCheckOk = 0;
			
			//if(GateWayFrameIndex == 2)
			{
				bFlag = SecurityProcessType50(pMingwen+1,sendBuf+4,sLen);

				//成功
				if(bFlag)
				{
					MasterStaFrameIndex = 1;
					Ret = 2;
				}
				else
				{
					MasterStaFrameIndex = 0;
					SecurityCheckOk = 0;
				}
			}
			break;
		}
		case 0x52:
		{
			//if(MasterStaFrameIndex == 1)
			{
				bFlag = SecurityProcessType52(pMingwen+1,sendBuf+4,sLen);

				//成功
				if(bFlag)
				{
					MasterStaFrameIndex = 2;
					Ret = 2;
				}
				else
				{
					MasterStaFrameIndex = 0;
					SecurityCheckOk = 0;
					if(*sLen>0)
					{
						Ret = 2;
					}
				}
			}
			break;
		}
		case 0x54:
		{
			if(MasterStaFrameIndex == 2)
			{
				bFlag = SecurityProcessType54(sendBuf+4,sLen);

				//成功
				if(bFlag)
				{
					MasterStaFrameIndex = 3;
					SecurityCheckOk = 1;
					Ret = 2;
				}
				else
				{
					MasterStaFrameIndex = 0;
					SecurityCheckOk = 0;
				}
			}
			break;
		}
		//密钥
		case 0x60:
		{
			if(!SecurityCheckOk)
			{
				return FALSE;
			}
			bFlag = SecurityProcessType60(sendBuf+4,sLen);
			
			if(bFlag)
			{
				Ret = 2;
			}
			break;
		}
		case 0x62:
		{
			if(!SecurityCheckOk)
			{
				sw = 0x9091;
				SecuritySendType(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			bFlag = SecurityProcessType62(pMingwen+1,sendBuf+4,sLen);
			if(bFlag)
			{
				Ret = 2;
			}
			else
			{
				if(*sLen>0)
					{
						Ret = 2;
					}
			}
				
			break;
		}
		case 0x64:
		{
			if(!SecurityCheckOk)//密钥恢复失败 cjl
			{
				sw = 0x9092;
				SecuritySendType(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			bFlag = SecurityProcessType64(pMingwen+1,sendBuf+4,sLen);
			if(bFlag)
				Ret = 2;
			else
			{
				if(*sLen>0)
					Ret = 2;
			}
			
			break;
		}
		//主站公钥远程下发流程 0x66
		case 0x66:
		{
			if(!SecurityCheckOk)
			{
				sw = 0x9097;
				SecuritySendType(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			
			bFlag = SecurityProcessType66(pMingwen+1,sendBuf+4,sLen);
			if(bFlag)
				Ret = 2;
			else
			{
				if(*sLen>0)
					Ret = 2;
			}
			break;
		}
		//主站提取公钥0x68
		case 0x68:
		{
			bFlag = SecurityProcessType68(pMingwen+1,sendBuf+4,sLen);
			if(bFlag)
				Ret = 2;
			else
			{
				if(*sLen>0)
					Ret = 2;
			}
			break;
		}
		case 0x6A:
		{//主站返回公钥的提取结果
			break;
		}
		//终端证书
		case 0x70:
		{
			if(!SecurityCheckOk)
			{
				sw = 0x9097;
				SecuritySendType(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			
			bFlag = SecurityProcessType70(pMingwen+1,sendBuf+4,sLen);
			if(bFlag)
				Ret = 2;
			else
			{
				if(*sLen>0)
					Ret = 2;
			}
			break;
		}
		case 0x72:
		{
			
			if(!SecurityCheckOk)
			{
				sw = 0x9097;
				SecuritySendType(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			
			bFlag = SecurityProcessType72(pMingwen+1,sendBuf+4,sLen);
			if(bFlag)
			{
				Ret = 2;
			}
			else
			{
				if(*sLen>0)
					Ret = 2;
			}
			break;
		}
		case 0x74:
		{
			bFlag = SecurityProcessType74(pMingwen+1,sendBuf+4,sLen);
			if(bFlag)
				Ret = 4;
			else
				Ret = 2;
			break;
		}
		case 0x76:
		{//获取终端证书，主站接收确认帧
			break;
		}
		default:
			break;
	}
	if((2 == Ret)||(5 == Ret))
	{
		SecurityProcessFrame(sendBuf,*sLen);
		*sLen += 6;
	}
	if(1 == Ret)
	{
		*type = pMingwen[0];
#ifdef INCLUDE_COMM_SHOW	
	commBufFill(commno - COMM_START_NO, 3, pMingwen[1], Buf);
#endif		
	}
	return Ret;
}

BOOL SecuritySendMD5Frame(BYTE *sendBuf, WORD *sLen)
{
	BOOL bRet = FALSE;
	
	
	if(MD5_ERRTYPE != 0x9000)
	{
		SecuritySendType1F(MD5_ERRTYPE,sendBuf+4,sLen);
		SecurityProcessFrame(sendBuf,*sLen);
		*sLen += 6;
		bRet = FALSE;
	}
	else
	{
		bRet = TRUE;
	}
	return bRet;
	
}

void SecuritySend1FFrame(BYTE *sendBuf, WORD *sLen)
{
	WORD sw;
	sw = 0x9101; //业务应用类型错误，cjl

	SecuritySendType1F(sw,sendBuf+4,sLen);
	SecurityProcessFrame(sendBuf,*sLen);
	*sLen += 6;
	
}

//发送数据接口,返回发送长度
// protocolType:1-104、2-101
WORD SecuritySendFrame(WORD commno,BYTE *buf,WORD wLen,BYTE protocolType,BYTE appType)
{
	tIec104FrameSecurity *pSend104;

	BYTE byType;
	int index;
	WORD rLen;
	
#ifdef INCLUDE_COMM_SHOW	
	commBufFill(commno - COMM_START_NO, 4, wLen, buf);
#endif
	
	if(wLen<1)
		return 0;
	if(protocolType>2)
		return 0;
	pSend104 = (tIec104FrameSecurity *)buf;	

	memcpy(ProccessBuf+8,buf,wLen);
	ProccessBuf[7] = wLen&0xFF;
	if(1 == protocolType)
	{//104
		byType = pSend104->byControl1&0x01;
		if(0 == byType)
		{//I帧
			ProccessBuf[6] = appType;
			ProccessBuf[5] = 0x08;//
			ProccessBuf[4] = 0;
		}
		else
		{//S帧、U帧
			ProccessBuf[6] = 0;
			ProccessBuf[5] = 0x00;//
			ProccessBuf[4] = 0;
		}
	}
	else
	{//101
		if(0x68 == buf[0])
		{
			ProccessBuf[6] = appType;
			ProccessBuf[5] = 0x08;//
			ProccessBuf[4] = 0;
		}
		else
		{
			ProccessBuf[6] = 0;
			ProccessBuf[5] = 0x00;//
			ProccessBuf[4] = 0;
		}
	}

	index = wLen + 8;
	if(0 == appType)
	{
		ProccessBuf[index++] = 0;
		ProccessBuf[index++] = 0;
	}
	else if(1 == appType)
	{
		ProccessBuf[index++] = 0;
		ProccessBuf[index++] = 65;
		if(SecurityCreateSign(ProccessBuf+8, wLen, ProccessBuf+index,&rLen) == FALSE)
			shellprintf("发送01签名错误 \n");
		if(rLen != 0x48)	
			shellprintf("发送01签名长度错误 \n");
		index = index + 0x48;
		ProccessBuf[index++] = 0x06;
	}
	else
	{
		ProccessBuf[index++] = 0;
		ProccessBuf[index++] = 8;
		memcpy(ProccessBuf+index,MyRandom,8);
		index += 8;
	}
	
	if((ProccessBuf[5]&0x08) == 0x08)
	{//加密

		{
			int i;
			for(i = 0;i<(index-6);i++)
				shellprintf("%02X ",*(ProccessBuf+i+6));
			shellprintf("\n");
		}
		if(SecurityEncrypt(ProccessBuf+6,index-6,buf+6,&rLen))
		{
			buf[4] = ProccessBuf[4];
			buf[5] = 	ProccessBuf[5];
			wLen = rLen;
			wLen += 2;
		}
		else
		{//
			return 0;
		}
	}
	else
	{//不加密
		memcpy(buf,ProccessBuf,wLen+10);
		wLen +=6;// 2+1+1+2
	}
	SecurityProcessFrame(buf,wLen);
	return wLen+6;
}

BOOL Securitysuotiantest(BYTE* rBuf, WORD *rLen)
{
    	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x00;
	chipCmd[1] = 0xC0;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x00;

	sw = ecrProcData(chipCmd, 6, NULL, 0, rBuf, rLen);
	if (sw == 0x9000) 
	{
		return TRUE;
	}
    return FALSE;
}

//产生芯片序列号
BOOL SecurityChipID(BYTE* rBuf, WORD *rLen)
{
    	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x00;
	chipCmd[1] = 0xB0;
	chipCmd[2] = 0x99;
#ifdef INCLUDE_SEC_CHIP
	if(g_ipsecinit == 1) //IPSec有效
	{
		chipCmd[3] = 0x00;
	}
	else
#endif
	chipCmd[3] = 0x05;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x02;
	chipCmd[6] = 0x00;
	chipCmd[7] = 0x08;

	sw = ecrProcData(chipCmd, 8, NULL, 0, rBuf, rLen);
	if (sw == 0x9000) 
	{
		return TRUE;
	}
    return FALSE;
}


//获取秘钥版本信息
BOOL SecurityKeyVersion(BYTE* rBuf, WORD *rLen)
{
   	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x1A;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x00;

	sw = ecrProcData(chipCmd, 6, NULL, 0, rBuf, rLen);
	if (sw == 0x9000) 
	{
		return TRUE;
	}
    	return FALSE;
}

//产生随机数
BOOL SecurityGenRandom(BYTE* rBuf, WORD *rLen)
{
    WORD sw;

	BYTE chipCmd[8];
	chipCmd[0] = 0x00;
	chipCmd[1] = 0x84;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x08;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x00;

	sw = ecrProcData(chipCmd, 6, NULL, 0, rBuf, rLen);

	if (sw == 0x9000) 
	{
		return TRUE;
	}
    	return FALSE;
}


BOOL SecurityDecrypt(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;

#if 1
	// 1.1.0
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x2c;
	chipCmd[2] = 0x60;
	chipCmd[3] = 0x01;

	memcpy(ProccessBuf2, IVData,16);
	memcpy(ProccessBuf2+16, sBuf,sLen);
	sLen += 16;
#else
	// 1.0.0
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x2c;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x01;
	memcpy(ProccessBuf2, sBuf,sLen);
#endif
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,ProccessBuf2,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 解密失败,sw = %04x\n",sw);
		return FALSE;
	}
}
BOOL SecurityMaintToolDecrypt(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;

	// 1.1.0
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x2c;
	chipCmd[2] = 0x62;
	chipCmd[3] = 0x02;
	memcpy(ProccessBuf2,ToolID,8);
	memcpy(ProccessBuf2+8, MyRandom,8);
	memcpy(ProccessBuf2+16, IVData,16);
	memcpy(ProccessBuf2+32, sBuf,sLen);
	sLen += 32;

	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,ProccessBuf2,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 解密失败,sw = %04x\n",sw);
		return FALSE;
	}
}
BOOL SecurityEncrypt(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x26;
	chipCmd[2] = 0x60;
	chipCmd[3] = 0x01;

	memcpy(ProccessBuf2, IVData,16);
	memcpy(ProccessBuf2+16, sBuf,sLen);
	sLen += 16;
	
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,ProccessBuf2,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}

BOOL SecurityMaintToolEncrypt(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x26;
	chipCmd[2] = 0x62;
	chipCmd[3] = 0x02;

	memcpy(ProccessBuf2, ToolID,8);
	memcpy(ProccessBuf2+8, MyRandom,8);
	memcpy(ProccessBuf2+16, IVData,16);
	memcpy(ProccessBuf2+32, sBuf,sLen);
	sLen += 32;
	
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,ProccessBuf2,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}


BOOL SecurityCheckSign(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x18;
	chipCmd[2] = 0x00;
	chipCmd[3] = asKID;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 验签失败,sw = %04x\n",sw);
		return FALSE;
	}
}

BOOL SecurityCheckSign1(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen) // 8008
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x08;
	chipCmd[2] = 0x00;
	chipCmd[3] = asKID;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 验签失败,sw = %04x\n",sw);
		return FALSE;
	}
}

BOOL SecurityMaintToolCheckSign(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x20;
	chipCmd[2] = 0x00;
	chipCmd[3] = asKID;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 验签失败,sw = %04x\n",sw);
		return FALSE;
	}
}



BOOL SecurityCreateSign(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x16;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x80;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 签名失败,sw = %04x \n",sw);
		return FALSE;
	}
}

BOOL SecurityMaintToolCreateSign(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x0A;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x80;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 签名失败,sw = %04x \n",sw);
		return FALSE;
	}
}



BOOL SecurityUpdateKey(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen
,BYTE byUpdateFlag)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x1C;
	chipCmd[2] = byUpdateFlag;//0恢复，>为更新
	chipCmd[3] = asKID;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 秘钥操作失败,sw = %04x \n",sw);
		return FALSE;
	}
}

BOOL SecurityUpdateCer(BYTE cerID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen
,BYTE byUpdateFlag)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	if(byUpdateFlag)
	{//0恢复，>为更新
		chipCmd[1] = 0x22;
		chipCmd[2] = cerID;
	}
	else
	{
		chipCmd[1] = 0x24;
		chipCmd[2] = 0;
	}
	chipCmd[3] = 0x00;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 升级证书操作失败,sw = %04x \n",sw);
		return FALSE;
	}
}

//新增 更新公钥 Pub
BOOL SecurityUpdatePub(BYTE P1,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x2A;
	chipCmd[2] = P1;
	chipCmd[3] = 0x01;
	memcpy(ProccessBuf2, IVData,8);
	memcpy(ProccessBuf2+8, sBuf,sLen);
	sLen += 8;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,ProccessBuf2,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else
	{ 
		shellprintf("更新公钥Pub失败 \n");
		return FALSE;
	}
}

BOOL SecurityMaintToolPubCA(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0xBF;
	chipCmd[1] = 0x2E;
	chipCmd[2] = 0x01;
	chipCmd[3] = 0x01;
	memcpy(ProccessBuf2, MyRandom,8);
	memcpy(ProccessBuf2+8, sBuf,sLen);
	sLen += 8;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,ProccessBuf2,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else
	{ 
		shellprintf("导入根CA公钥失败 \n");
		return FALSE;
	}
}
//与SecurityUpdatePub一样
BOOL SecurityMaintToolPub(BYTE P1,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x2A;
	chipCmd[2] = P1;
	chipCmd[3] = 0x01;
	memcpy(ProccessBuf2, MyRandom,8);
	memcpy(ProccessBuf2+8, sBuf,sLen);
	sLen += 8;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,ProccessBuf2,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else
	{ 
		shellprintf("更新公钥Pub失败 \n");
		return FALSE;
	}
}


BOOL SecurityLoadCer(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen,
BYTE byUpdateFlag)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x24;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 下载证书操作失败,sw = %04x \n",sw);
		return FALSE;
	}
}

BOOL SecurityGetTestCerLen(BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x00;
	chipCmd[1] = 0xB0;
	chipCmd[2] = 0x81;
	chipCmd[3] = 0x00;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x02;
	chipCmd[6] = 0x00;
	chipCmd[7] = 0x02;

	sw = ecrProcData(chipCmd,8,NULL,0,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}

BOOL SecurityGetWorkCerData(BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
#ifdef INCLUDE_SEC_CHIP
    if(g_ipsecinit == 1) //IPSec口配置规约且配置有效，则为南网加密
    {
      chipCmd[1] = 0x36;
    }
    else
 #endif
	chipCmd[1] = 0x30;
	chipCmd[2] = 0x01;
	chipCmd[3] = 0x00;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x00;


	sw = ecrProcData(chipCmd,6,NULL,0,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}

BOOL SecurityGetPubData(BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x36;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x00;


	sw = ecrProcData(chipCmd,6,NULL,0,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}

BOOL SecurityGetPublicKey(BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	
#ifdef INCLUDE_SEC_CHIP
	if(g_ipsecinit == 1)
	{
		chipCmd[1] = 0x36;
	}
	else
#endif
	chipCmd[1] = 0x30;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x00;


	sw = ecrProcData(chipCmd,6,NULL,0,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}
BOOL SecurityGetTestCerData(WORD DataLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x00;
	chipCmd[1] = 0xB0;
	chipCmd[2] = 0x81;
	chipCmd[3] = 0x02;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x02;
	chipCmd[6] = (DataLen>>8)&0xFF;
	chipCmd[7] = DataLen&0xFF;

	sw = ecrProcData(chipCmd,8,NULL,0,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}

BOOL SecurityCheckMaintCer(BYTE *sendBuf,WORD sendLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[6];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x1E;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;
	chipCmd[4] = (sendLen>>8)&0xFF;
	chipCmd[5] = sendLen&0xFF;


	sw = ecrProcData(chipCmd,6,sendBuf,sendLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}
BOOL SecurityCheckMaintPub(BYTE *sendBuf,WORD sendLen,BYTE *rBuf, WORD *rLen)
{
	BYTE chipCmd[6];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x1E;
	chipCmd[2] = 0x01;
	chipCmd[3] = 0x01;
	chipCmd[4] = (sendLen>>8)&0xFF;
	chipCmd[5] = sendLen&0xFF;


	sw = ecrProcData(chipCmd,6,sendBuf,sendLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}
//业务报文，sendBuf前4个字节空出来
void SecurityProcessFrame(BYTE *sendBuf,WORD sendLen)
{
	BYTE cs;
	int index = 0;
	sendBuf[index++] = 0xEB;
	sendBuf[index++] = (sendLen>>8)&0XFF;
	sendBuf[index++] = sendLen&0xFF;
	sendBuf[index++] = 0xEB;
	
	cs = SecurityChkSum(sendBuf+4,sendLen);
	index+=sendLen;
	sendBuf[index++] = cs;// 4-1
	sendBuf[index++] = 0xD7;
}
void SecurityUnlink(void)
{
	SecurityCheckOk = 0;
	GateWayFrameIndex = 0;
	MasterStaFrameIndex = 0;
}
//buf:从应用类型之后的字节开始
BOOL SecurityProcessType20(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD Len;
	int i;
	BOOL bFlag;
	WORD sendLen;

	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 8)
		return bRet;
	
#if 0
	for(i=0;i<8;i++ )
	{
		GateWayRandom[i] = buf[i+2];
	}
#endif

	bFlag = SecurityCreateSign(buf+2,8,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type20 签名失败\n");
		return bRet;
	}
	if(sendLen != 0x48)
		return bRet;
	
	for(i=0;i<8;i++ )
	{
		MyRandom[i] = sendBuf[i+5];
	}
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x80;
	sendBuf[2] = 0x21;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x49;
	sendLen += 5;
	sendBuf[sendLen] = 0x06;
	sendLen++;
	bRet = TRUE;
	*pSendLen = sendLen;
	return bRet;
}

BOOL SecurityProcessType22(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = TRUE;
	WORD Len;
	BOOL bFlag;
	BOOL bNoSign = FALSE;
	WORD sendLen;

	*pSendLen = 0;
	
	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 0x0041)
	{
		bRet = FALSE;
		bNoSign = TRUE;
	}
	if(bRet)
		bFlag = SecurityCheckSign(5,buf+2,64,sendBuf+5,&sendLen);
	else
		bFlag = FALSE;
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x80;
	sendBuf[2] = 0x23;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	if(bNoSign)
	{
		sendBuf[5] = 0x90;
		sendBuf[6] = 0x90;//失败
	}
	else
	{
		if(bFlag)
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x00;//成功
		}
		else
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x90;//失败
		}
	}
	
	sendLen=7;
	*pSendLen = sendLen;
	if(bFlag)
		bRet = TRUE;
	else
	{
		shellprintf("Error: Type22 验签失败\n");
		bRet = FALSE;
	}
	return bRet;
}
//
BOOL SecurityProcessType30(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD Len;
	BOOL bFlag = TRUE;
	int i;
	
	static BYTE FrameIndex = 0;
	static BYTE FrameCount = 0;
	static BYTE CerId = 0;
	static WORD CerIndex = 0;
	WORD sw;
	*pSendLen = 0;
	if(!MaintFlag)	
	{
		Len = MAKEWORD(buf[1], buf[0]);

		if(1 == buf[4])
		{
			if( 7 != buf[2] ) //证书标识必须为7
				return FALSE;
			
			CerId = buf[2];
			FrameCount = buf[3];
			FrameIndex = 1;
			memcpy(&ProccessCerBuf[0],buf+5,Len-11);
			CerIndex = Len-11;
			memcpy(ToolID,buf+5+CerIndex,8);

			WriteCerFinish = 0;

			if(FrameCount ==  FrameIndex)
				WriteCerFinish = 1;
		}
		else
		{
			if( 7 != buf[2] ) //证书标识必须为7
				return FALSE;
			FrameIndex++;
			if(CerId == buf[2]&&FrameIndex == buf[4])
			{
				memcpy(&ProccessCerBuf[CerIndex],buf+5,Len-3);
				CerIndex = CerIndex + Len-11;

				if(FrameCount ==  FrameIndex)
					WriteCerFinish = 1;
			}
			else
			{
				CerId = 0;
				FrameCount = 0;
				FrameIndex = 0;
				WriteCerFinish = 1;
				bFlag = FALSE;
			}
		}

		if(bFlag)
			sw = 0x9000;
		else
			sw = 0x9090;
		SecuritySendType45(sw,sendBuf,pSendLen);

		
		if(WriteCerFinish != 1)
		{
			sw = 0x9000;
			SecuritySendType45(sw,sendBuf,pSendLen);
			return FALSE;
		}
			
		if(!bFlag)
		{
			WriteCerFinish = 0;
			return FALSE;
		}
		

		bFlag = SecurityCheckMaintCer(ProccessCerBuf,CerIndex,ProccessBuf,&Len);

		WriteCerFinish = 0;
		if(!bFlag)
		{
			sw = 0x9090;
			SecuritySendType45(sw,sendBuf,pSendLen);
			return FALSE;
		}
	}
	else
	{
		if(buf[3]!=buf[4])
		{
			sw = 0x9000;
			SecuritySendType45(sw,sendBuf,pSendLen);
			return FALSE;
		}
	}
	bFlag = SecurityGenRandom(ProccessBuf+7,&Len);
	if(!bFlag)
		return bRet;

	for(i=0;i<8;i++ )
	{
		MyRandom[i] = ProccessBuf[i+7];
		IVData[i] = ProccessBuf[i+7];
		IVData[i+8] = ProccessBuf[i+7]^0xFF;
	}
	ProccessBuf[0] = 0;
	ProccessBuf[1] = 13;
	ProccessBuf[2] = 0x00;
	ProccessBuf[3] = 0x40;
	ProccessBuf[4] = 0x31;
	ProccessBuf[5] = 0x00;
	ProccessBuf[6] = 0x08;
	
	SendReturnData = 0x31;
	return TRUE;
	
}

BOOL SecurityProcessType32(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = TRUE;
	WORD Len;
	BOOL bFlag = TRUE;
	BOOL bNoSign = FALSE;
	WORD sendLen;

	*pSendLen = 0;
	
	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 0x0040)
	{
		bRet = FALSE;
	    bNoSign = TRUE;
	}
	if(!MaintFlag)
	{
		if(bRet)
			bFlag = SecurityMaintToolCheckSign(0,buf+2,64,sendBuf+5,&sendLen);
		else
			bFlag = FALSE;
	}
	
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x33;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	if(bNoSign)
	{
		sendBuf[5] = 0x90;
		sendBuf[6] = 0x90;
	}
	else
	{
		if(bFlag)
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x00;//成功
		}
		else
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x90;//失败
		}
	}
	
	sendLen=7;
	*pSendLen = sendLen;
	if(bFlag)
		bRet = TRUE;
	else
	{
		shellprintf("Error: Type32 验签失败\n");
		bRet = FALSE;
	}
	return bRet;
}

BOOL SecurityProcessType34(BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;

	BOOL bFlag;
	WORD sendLen;

	*pSendLen = 0;

	//
	bFlag = SecurityKeyVersion(sendBuf+5,&sendLen);

	if(!bFlag)
		return bRet;

	bFlag = SecurityGenRandom(sendBuf+6,&sendLen);
	if(!bFlag)
		return bRet;
	
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x35;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x09;
	if(MaintFlag)
		sendBuf[5] = 0x01;
	sendLen=14;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;
}
BOOL SecurityProcessType36(BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD sendLen;
	//
	*pSendLen = 0;
	
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x37;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x18;

	//
	memcpy(sendBuf+5,g_Sys.InPara[SELFPARA_ID],NVRAM_DEV_ID_LEN);
	//
	
	sendLen=29;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;
}

BOOL SecurityProcessType38(BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	BOOL bFlag;
	WORD sendLen;
	//
	*pSendLen = 0;
	
	bFlag = SecurityChipID(sendBuf+5,&sendLen);

	if(!bFlag)
	{
		*pSendLen = 0;
		return FALSE;
	}
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x39;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x08;
	
	sendLen=13;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;
}

BOOL SecurityProcessType3A(BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	BOOL bFlag;
	WORD sendLen;
	//
	*pSendLen = 0;
	
	bFlag = SecurityGetPublicKey(ProccessBuf+3,pSendLen);

	if(!bFlag)
	{
		*pSendLen = 0;
		return FALSE;
	}
	sendLen = *pSendLen;
#ifdef INCLUDE_SEC_CHIP
	if(g_ipsecinit == 1)
	{
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x40;
		sendBuf[2] = 0x3B;
		sendBuf[3] = HIBYTE(sendLen);
		sendBuf[4] = LOBYTE(sendLen);
		memcpy(sendBuf+5,ProccessBuf+3,sendLen);
		sendLen=sendLen+5;
		*pSendLen = sendLen;
		bRet = TRUE;
	}
	else	
#endif
	{
		ProccessBuf[0] = 0x3B;
		ProccessBuf[1] = (sendLen>>8)& 0xFF;
		ProccessBuf[2] = (sendLen & 0xFF);
		sendLen += 3;
		if(SecurityMaintToolEncrypt(ProccessBuf,sendLen,sendBuf+2,pSendLen))
		{
			sendBuf[0] = 0x00;
			sendBuf[1] = 0x40|(1<<3);  //加密D3
			sendLen = *pSendLen;
			*pSendLen = sendLen;
			sendLen += 2;
			*pSendLen = sendLen;
			bRet = TRUE;
		}
	}
	return bRet;
}
BOOL SecurityProcessType3C(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD Len;
	BOOL bFlag = TRUE;
	
	static BYTE FrameIndex = 0;
	static BYTE FrameCount = 0;
	static WORD CerIndex = 0;
	WORD sw;
	
	Len = MAKEWORD(buf[1], buf[0]);

	if(1 == buf[3])
	{
		FrameCount = buf[2];
		FrameIndex = 1;
		memcpy(&ProccessCerBuf[0],buf+4,Len-2);
		CerIndex = Len-2;
		WriteCerFinish = 0;

		if(FrameCount ==  FrameIndex)
			WriteCerFinish = 1;
	}
	else
	{
		FrameIndex++;
		if(FrameIndex == buf[3])
		{
			memcpy(&ProccessCerBuf[CerIndex],buf+4,Len-2);
			CerIndex = CerIndex + Len-2;

			if(FrameCount ==  FrameIndex)
				WriteCerFinish = 1;
		}
		else
		{
			FrameCount = 0;
			FrameIndex = 0;
			WriteCerFinish = 1;
			bFlag = FALSE;
		}
	}

	if(bFlag)
		sw = 0x9000;
	else
		sw = 0x9097;
	SecuritySendType45(sw,sendBuf,pSendLen);
	

	if(WriteCerFinish != 1)
	{
		return FALSE;
	}

	if(!bFlag)
	{
		return FALSE;
	}
		
	//
	bFlag = SecurityMaintToolCreateSign(ProccessCerBuf,CerIndex,ProccessBuf+7,&Len);

	if(!bFlag)
	{
		shellprintf("Error: Type3C 签名失败\n");
		return bRet;
	}


	ProccessBuf[2] = 0x00;
	ProccessBuf[3] = 0x40;
	ProccessBuf[4] = 0x3D;
	ProccessBuf[5] = 0x00;
	ProccessBuf[6] = 0x40;
	Len += 5;
	
	ProccessBuf[0] = (Len>>8)&0xFF;
	ProccessBuf[1] = Len&0xFF;

	SendReturnData = 0x3D;

	bRet = TRUE;
	return bRet;
}


BOOL SecurityProcessType3E(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD Len;
	BOOL bFlag = TRUE;
	
	static BYTE FrameIndex = 0;
	static BYTE FrameCount = 0;
	static BYTE CerId = 0;
	static WORD CerIndex = 0;

	WORD sw;
	
	Len = MAKEWORD(buf[1], buf[0]);

	if(1 == buf[4])
	{
		CerId = buf[2];
		FrameCount = buf[3];
		FrameIndex = 1;
		ProccessCerBuf[0] = CerId;
		memcpy(&ProccessCerBuf[0],buf+5,Len-3);
		CerIndex = Len-3;
		WriteCerFinish = 0;

		if(FrameCount ==  FrameIndex)
			WriteCerFinish = 1;
	}
	else
	{
		FrameIndex++;
		if(CerId == buf[2]&&FrameIndex == buf[4])
		{
			memcpy(&ProccessCerBuf[CerIndex],buf+5,Len-3);
			CerIndex = CerIndex + Len-3;

			if(FrameCount ==  FrameIndex)
				WriteCerFinish = 1;
		}
		else
		{
			CerId = 0;
			FrameCount = 0;
			FrameIndex = 0;
			WriteCerFinish = 1;
			bFlag = FALSE;
		}
	}

	if(bFlag)
		sw = 0x9000;
	else
		sw = 0x9097;
	SecuritySendType45(sw,sendBuf,pSendLen);


	
	if(WriteCerFinish != 1)
	{
		return FALSE;
	}

	if(!bFlag)
	{
		WriteCerFinish = 0;
		return FALSE;
	}
	
	if(CerId<6)
		bFlag = SecurityUpdateCer(CerId,ProccessCerBuf,CerIndex,ProccessBuf,&Len,1);
	else
		bFlag = SecurityUpdateCer(CerId,ProccessCerBuf,CerIndex,ProccessBuf,&Len,0);

	WriteCerFinish = 0;


	ProccessBuf[2] = 0x00;
	ProccessBuf[3] = 0x40;
	ProccessBuf[4] = 0x3F;
	ProccessBuf[5] = 0x00;
	ProccessBuf[6] = 0x02;
	if(bFlag)
	{
		ProccessBuf[8] = 0x0;
	}
	else
	{
		ProccessBuf[8] = 0x93;
	}
	ProccessBuf[7] = 0x90;
	
	ProccessBuf[0]=0;
	ProccessBuf[1]=7;

	SendReturnData = 0x3F;

	bRet = TRUE;
	return bRet;
}

BOOL SecurityProcessType40(BYTE *sendBuf,WORD *pSendLen)
{

	BOOL bRet = FALSE;

	BOOL bFlag;
	WORD len;
	
	//
	bFlag = SecurityGetTestCerLen(ProccessCerBuf,pSendLen);
	if(bFlag)
	{
		len = MAKEWORD(ProccessCerBuf[1],ProccessCerBuf[0]);
		bFlag = SecurityGetTestCerData(len,ProccessCerBuf,pSendLen);

		if(bFlag)
		{
			bFlag = SecurityUpdateCer(0,ProccessCerBuf,len,sendBuf,pSendLen,0);
			
		}
		
	}
		
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x41;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	if(bFlag)
	{
		sendBuf[6] = 0x0;
		bRet = TRUE;
	}
	else
	{
		sendBuf[6] = 0x92;
		bRet = FALSE;
	}
	sendBuf[5] = 0x90;
	*pSendLen=7;
	
	return bRet;
}

BOOL SecurityProcessType42(BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bFlag;
	WORD sLen;
	bFlag = SecurityGetWorkCerData(ProccessCerBuf,&sLen);

	wCerDataLen = sLen;
	*pSendLen = 0;
	if(bFlag)
	{
		SendMaintToolCerFlag = TRUE; 
		bySendCerFrameIndex = 1;
		bySendCerFrameCount = (wCerDataLen-1)/SendCerDataLen +1;
		*pSendLen = wCerDataLen;
		return TRUE;
	}
	else
	{
		SendMaintToolCerFlag = FALSE; 
		bySendCerFrameIndex = 0;
		wCerDataLen =0;
		bySendCerFrameCount = 0;
		return TRUE;
	}
}

int SecuritySendType43(BYTE *sendBuf,WORD *pSendLen)
{
	int nRet = 0;//不发送处理
	BYTE *p = sendBuf;
	WORD FrameLen;
	int index = 0;
	
	if(!SendMaintToolCerFlag)
		return nRet;

	if(bySendCerFrameCount ==0||bySendCerFrameIndex>bySendCerFrameCount)
		return nRet;
	
	//发送标志
	if(bySendCerFrameIndex < bySendCerFrameCount)
		FrameLen = SendCerDataLen;
	else
		FrameLen = wCerDataLen - SendCerDataLen*(bySendCerFrameIndex-1);

	FrameLen += 3;
	
	p[index++] = 0x00;
	p[index++] = 0x40;
	p[index++] = 0x43;
	p[index++] = (FrameLen>>8)&0xFF;
	p[index++] = FrameLen&0xFF;
	p[index++] = 0x06; //终端证书标识6
	p[index++] = bySendCerFrameCount;
	p[index++] = bySendCerFrameIndex;
	memcpy(p+index,ProccessCerBuf+(bySendCerFrameIndex-1)*SendCerDataLen,FrameLen-3);
	index += (FrameLen-3);
	bySendCerFrameIndex++;

	*pSendLen = index;
	if(bySendCerFrameIndex>bySendCerFrameCount)
	{
		SendMaintToolCerFlag = FALSE;
		nRet = 2;
	}
	else
	{
		nRet = 1;
	}
	return nRet;
}
BOOL SecurityProcessType44(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	WORD sw = MAKEWORD(ProccessCerBuf[3],ProccessCerBuf[2]);

	if(0x9000 == sw)
		return TRUE;
	else
		return FALSE;
}

BOOL SecuritySendType45(WORD sw,BYTE *sendBuf,WORD *pSendLen)
{
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x45;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	sendBuf[5] = (sw>>8)&0xFF;
	sendBuf[6] = sw&0xFF;
	*pSendLen = 7;
	return TRUE;
}


BOOL SecurityProcessType46(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	WORD Len;
	BOOL bFlag;
	
	Len = MAKEWORD(buf[1], buf[0]);
#ifdef INCLUDE_SEC_CHIP
    if(g_ipsecinit == 1) //IPSec口配置规约且配置有效，则为南网加密
    {
      if(Len == 0x0100)
		bFlag = SecurityUpdateKey(0,buf+2,Len,sendBuf+5,pSendLen,2);
    }
    else
#endif
	if(Len == 0x00B9)
		bFlag = SecurityUpdateKey(0,buf+2,Len,sendBuf+5,pSendLen,2);
	else
		bFlag = FALSE;
	
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x47;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	if(bFlag)
		sendBuf[6] = 0x00;//成功
	else
		sendBuf[6] = 0x92;//失败
	sendBuf[5] = 0x90;
	
	*pSendLen=7;
	return bFlag;
}

BOOL SecurityProcessType48(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD sendLen = 0;
	
	sendLen = MAKEWORD(buf[1], buf[0]);

	ProccessBuf[0] = 0x49;
	ProccessBuf[1] = (sendLen>>8)& 0xFF;
	ProccessBuf[2] = ((sendLen) & 0xFF);
	sendLen += 3;

	if(SecurityMaintToolEncrypt(ProccessBuf,sendLen,sendBuf+2,pSendLen))
	{
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x40|(1<<3);  //加密D3
		sendLen = *pSendLen;
		*pSendLen = sendLen;
		sendLen += 2;
		*pSendLen = sendLen;
		bRet = TRUE;
	}
	else
		bRet = FALSE;

	return bRet;
}

//运维（公钥模式）认证
BOOL SecurityProcessType4A(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD Len;
	BOOL bFlag = TRUE;
	int i;
	
	WORD sw;
	*pSendLen = 0;
	
	Len = MAKEWORD(buf[1], buf[0]);
	if(Len < 9)
	{
		sw = 0x9090;
		SecuritySendType45(sw,sendBuf,pSendLen);
	}
	else
	{
		memcpy(ToolID,buf+2+Len-8,8);
		memcpy(ProccessCerBuf,ToolID,8);
		memcpy(ProccessCerBuf+8,buf+2,Len-8);
	}
	Len = Len;	
	bFlag = SecurityCheckMaintPub(ProccessCerBuf,Len,ProccessBuf,&Len);
	
	if(!bFlag)
	{
		sw = 0x9090;
		SecuritySendType45(sw,sendBuf,pSendLen);
		return FALSE;
	}

	bFlag = SecurityGenRandom(ProccessBuf+7,&Len);
	if(!bFlag)
		return bRet;

	for(i=0;i<8;i++ )
	{
		MyRandom[i] = ProccessBuf[i+7];
		IVData[i] = ProccessBuf[i+7];
		IVData[i+8] = ProccessBuf[i+7]^0xFF;
	}
	ProccessBuf[0] = 0x00;
	ProccessBuf[1] = 13;
	ProccessBuf[2] = 0x00;
	ProccessBuf[3] = 0x40;
	ProccessBuf[4] = 0x31;
	ProccessBuf[5] = 0x00;
	ProccessBuf[6] = 0x08;	
	SendReturnData = 0x31;
	return TRUE;
}

BOOL SecurityProcessType4B(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD sendLen = 0;
	
	sendLen = MAKEWORD(buf[1], buf[0]);
	

	bRet = SecurityMaintToolPubCA(buf+2,sendLen,ProccessBuf,&sendLen);
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x4C;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	if(bRet)
		sendBuf[6] = 0x00;
	else
		sendBuf[6] = 0x99;
	sendBuf[5] = 0x90;
	
	*pSendLen=7;

	return bRet;
}

BOOL SecurityProcessType4D(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD sendLen = 0;
	BYTE PubID;
	
	sendLen = MAKEWORD(buf[1], buf[0]);
	PubID = buf[2];
	PubID = 0x80 | (PubID & 0x7F);
	sendLen = sendLen - 1;
	bRet = SecurityMaintToolPub(PubID,buf+3,sendLen,ProccessBuf,&sendLen);
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = 0x4E;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	if(bRet)
		sendBuf[6] = 0x00;
	else
		sendBuf[6] = 0x99;
	sendBuf[5] = 0x90;
	
	*pSendLen=7;

	return bRet;
}

//
BOOL SecurityProcessType50(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD Len;
	int i;
	BOOL bFlag;
	WORD sendLen;
	*pSendLen = 0;

	
	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 8)
		return bRet;
	for(i=0;i<8;i++ )
	{
#if 0
		MasterStaRandom[i] = buf[i+2];
#endif
		IVData[i] = buf[i+2];
		IVData[i+8] = buf[i+2]^0xFF;
	}
	//发终端，收终端随机数+签名??
	bFlag = SecurityCreateSign(buf+2,8,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type50 签名失败\n");
		return bRet;
	}
	if(sendLen != 0x48)
		return bRet;
	
	for(i=0;i<8;i++ )
	{
		MyRandom[i] = sendBuf[i+5];
	}
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x01;
	sendBuf[2] = 0x51;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x49;
	sendLen += 5;
	sendBuf[sendLen] = 0x06;
	sendLen++;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;
}


BOOL SecurityProcessType52(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = TRUE;
	WORD Len;
	BOOL bFlag;
	WORD sendLen;
	BOOL bNoSign = FALSE;
	BYTE asKID;
	*pSendLen = 0;
	
	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 0x0041)
	{
		 bNoSign = TRUE;
		 bRet = FALSE;
	}
	asKID = buf[66];// 64+2
	//
	if(bRet)
		bFlag = SecurityCheckSign(asKID,buf+2,64,sendBuf+5,&sendLen);
	else
		bFlag = FALSE;
	
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x01;
	sendBuf[2] = 0x53;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	
	if(bNoSign)
	{
		sendBuf[5] = 0x90;
		sendBuf[6] = 0x90;
	}
	else
	{
		if(bFlag)
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x00;//成功
		}
		else
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x90;//失败
			bRet = FALSE;
		}
	}
	
	sendLen=7;
	*pSendLen = sendLen;
	if(bFlag)
		bRet = TRUE;
	else
	{
		shellprintf("Error: Type52 验签失败\n");
		bRet = FALSE;
	}
	return bRet;
}


BOOL SecurityProcessType54(BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	BOOL bFlag;
	WORD sendLen;
	//
	*pSendLen = 0;
	
	bFlag = SecurityChipID(sendBuf+5,&sendLen);

	if(!bFlag)
	{
		*pSendLen = 0;
		return FALSE;
	}
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x01;
	sendBuf[2] = 0x55;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x08;
	
	sendLen=13;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;
}



BOOL SecurityProcessType60(BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;

	BOOL bFlag;
	WORD sendLen;
	//
	bFlag = SecurityKeyVersion(sendBuf+5,&sendLen);

	if(!bFlag)
		return bRet;

	bFlag = SecurityGenRandom(sendBuf+6,&sendLen);
	if(!bFlag)
		return bRet;
	
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x01;
	sendBuf[2] = 0x61;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x09;
	
	sendLen=14;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;
}

BOOL SecurityProcessType62(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = TRUE;
	WORD Len;
	BOOL bFlag;
	BYTE askID;
	BOOL bNoSign = FALSE;
	
#ifdef INCLUDE_SEC_CHIP	
	static BYTE FrameIndex = 0;
	static BYTE FrameCount = 0;
	static WORD KeyIndex = 0;
	if(g_ipsecinit == 1)
	{
		Len = MAKEWORD(buf[1], buf[0]);
		if(1 == buf[3])
		{
			FrameCount = buf[2];
			FrameIndex = buf[3];
			memcpy(&ProccessCerBuf[0],buf+4,Len-2);
			KeyIndex = Len - 2;
			if(FrameCount == FrameIndex)
				WriteCerFinish = 1;
		}
		else
		{
			FrameIndex++;
			if(FrameIndex == buf[3])
			{
				memcpy(&ProccessCerBuf[KeyIndex],buf+4,Len-2);
				KeyIndex += Len - 2;
				if(FrameCount == FrameIndex)
					WriteCerFinish = 1;
			}
			else
			{
				FrameCount = 0;
				FrameIndex = 0;
				WriteCerFinish = 0;
				KeyIndex = 0;
				sendBuf[0] = 0x00;
				sendBuf[1] = 0x01;
				sendBuf[2] = 0x63;
				sendBuf[3] = 0x00;
				sendBuf[4] = 0x02;
				sendBuf[6] = 0x91;
				sendBuf[5] = 0x90;
				*pSendLen=7;
				return FALSE;
			}
		}
		if(WriteCerFinish != 1) 
		{
			*pSendLen = 0;
			return FALSE;
		}
		
		if(KeyIndex != 0x013D)
			bFlag = FALSE;
		else
		{
		//
		askID = ProccessCerBuf[KeyIndex-1];
		bFlag = SecurityUpdateKey(askID,ProccessCerBuf,KeyIndex-1,sendBuf+5,pSendLen,0);
		}
		
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x01;
		sendBuf[2] = 0x63;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x02;
		
		if(bFlag)
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x00;  //3   
			bRet = TRUE;
		}
		else
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x91;
			bRet = FALSE;
		}
		*pSendLen=7;
	}
	else
#endif
	{
		Len = MAKEWORD(buf[1], buf[0]);
		if(Len != 0x00F6)
		{
			bRet  = FALSE;
			bNoSign = TRUE;
		}

		//
		askID = buf[Len+2-1];
		if(bRet)
			bFlag = SecurityUpdateKey(askID,buf+2,Len-1,sendBuf+5,pSendLen,0);
		else
			bFlag = FALSE;
		
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x01;
		sendBuf[2] = 0x63;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x02;
		
		if(bNoSign)
		{
			sendBuf[5] = 0x90;
			sendBuf[6] = 0x91;//失败
		}
		else
		{
			if(bFlag)
			{
				sendBuf[5] = 0x90;
				sendBuf[6] = 0x00;  //3   
				bRet = TRUE;
			}
			else
			{
				sendBuf[5] = 0x90;
				sendBuf[6] = 0x91;
				bRet = FALSE;
			}
		}
		
		*pSendLen=7;
	}
	return bRet;
}

BOOL SecurityProcessType64(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	BOOL bRet = FALSE;
	WORD Len;
	BOOL bFlag;
	BYTE askID;
	
#ifdef INCLUDE_SEC_CHIP	
	static BYTE FrameIndex = 0;
	static BYTE FrameCount = 0;
	static WORD KeyIndex = 0;
	if(g_ipsecinit == 1)
	{
		Len = MAKEWORD(buf[1], buf[0]);
		if(1 == buf[3])
		{
			FrameCount = buf[2];
			FrameIndex = buf[3];
			memcpy(&ProccessCerBuf[0],buf+4,Len-2);
			KeyIndex = Len - 2;
			if(FrameCount == FrameIndex)
				WriteCerFinish = 1;
		}
		else
		{
			FrameIndex++;
			if(FrameIndex == buf[3])
			{
				memcpy(&ProccessCerBuf[KeyIndex],buf+4,Len-2);
				KeyIndex += Len - 2;
				if(FrameCount == FrameIndex)
					WriteCerFinish = 1;
			}
			else
			{
				FrameCount = 0;
				FrameIndex = 0;
				WriteCerFinish = 0;
				KeyIndex = 0;
				sendBuf[0] = 0x00;
				sendBuf[1] = 0x01;
				sendBuf[2] = 0x65;
				sendBuf[3] = 0x00;
				sendBuf[4] = 0x02;
				sendBuf[6] = 0x92;
				sendBuf[5] = 0x90;
				*pSendLen=7;
				return FALSE;
			}
		}
		
		if(WriteCerFinish != 1) //不是最后一帧
		{
			*pSendLen = 0;
			return FALSE;
		}
		
		if(KeyIndex != 0x013D)
			bFlag = FALSE;
		else
		{
			askID = ProccessCerBuf[KeyIndex-1];
			bFlag = SecurityUpdateKey(askID,ProccessCerBuf,KeyIndex-1,sendBuf+5,pSendLen,1);
		}
		
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x01;
		sendBuf[2] = 0x65;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x02;
		if(bFlag)
			sendBuf[6] = 0x00;//成功
		else
			sendBuf[6] = 0x92;//失败
		sendBuf[5] = 0x90;
		
		*pSendLen=7;
		bRet = TRUE;
	}
	else
#endif
	{
		Len = MAKEWORD(buf[1], buf[0]);
		if(Len != 0x00F6)
			bFlag = FALSE;
		else
		{
			askID = buf[Len+2-1];
			bFlag = SecurityUpdateKey(askID,buf+2,Len-1,sendBuf+5,pSendLen,1);
		}
		
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x01;
		sendBuf[2] = 0x65;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x02;
		if(bFlag)
			sendBuf[6] = 0x00;//成功
		else
			sendBuf[6] = 0x92;//失败
		sendBuf[5] = 0x90;
		
		*pSendLen=7;
		bRet = TRUE;
	}
	return bRet;
}

BOOL SecurityProcessType70(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	WORD Len,Len1;
	BOOL bFlag;
	BYTE askID;
	BYTE cerID;
	
	static BYTE FrameIndex = 0;
	static BYTE FrameCount = 0;
	static BYTE CerId = 0;
	static WORD CerIndex = 0;

	Len = MAKEWORD(buf[1], buf[0]);

	if(1 == buf[4])
	{
		CerId = buf[2];
		FrameCount = buf[3];
		FrameIndex = 1;
		ProccessCerBuf[0] = CerId;
		memcpy(&ProccessCerBuf[0],buf+5,Len-3);
		CerIndex = Len-3;
		WriteCerFinish = 0;

		if(FrameCount ==  FrameIndex)
			WriteCerFinish = 1;
	}
	else
	{
		FrameIndex++;
		if(CerId == buf[2]&&FrameIndex == buf[4])
		{
			memcpy(&ProccessCerBuf[CerIndex],buf+5,Len-3);
			CerIndex = CerIndex + Len-3;

			if(FrameCount ==  FrameIndex)
				WriteCerFinish = 1;
		}
		else
		{
			CerId = 0;
			FrameCount = 0;
			FrameIndex = 0;
			WriteCerFinish = 1;
			bFlag = FALSE;
		}
	}
	if(WriteCerFinish != 1)
	{
		*pSendLen = 0;
		return FALSE;
	}

	if(!bFlag)
	{
		WriteCerFinish = 0;
		
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x01;
		sendBuf[2] = 0x71;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x02;
		sendBuf[6] = 0x97;//失败
		sendBuf[5] = 0x90;
		*pSendLen=7;
		return FALSE;
	}
		
	//解密
	bFlag = SecurityDecrypt(ProccessCerBuf,CerIndex,ProccessCerBuf,&Len);
	if(bFlag)
	{
		askID = ProccessCerBuf[Len -1];
		bFlag = SecurityCheckSign1(askID,ProccessCerBuf,Len-1,ProccessBuf,&Len1);
		if(bFlag)
		{//
			cerID = ProccessCerBuf[0];
			Len = Len -2 -64 - 6;
			bFlag = SecurityUpdateCer(cerID,ProccessCerBuf+1,Len,ProccessBuf,&Len,1);
		}
		else
		{
			WriteCerFinish = 0;
			sendBuf[0] = 0x00;
			sendBuf[1] = 0x01;
			sendBuf[2] = 0x71;
			sendBuf[3] = 0x00;
			sendBuf[4] = 0x02;
			sendBuf[6] = 0x97;//失败
			sendBuf[5] = 0x90;
			*pSendLen=7;
			return FALSE;
		}
	}
	else
	{
			WriteCerFinish = 0;
			sendBuf[0] = 0x00;
			sendBuf[1] = 0x01;
			sendBuf[2] = 0x71;
			sendBuf[3] = 0x00;
			sendBuf[4] = 0x02;
			sendBuf[6] = 0x97;//失败
			sendBuf[5] = 0x90;
			*pSendLen=7;
			return FALSE;
	}
	
	if(!bFlag)
	{
		WriteCerFinish = 0;
		
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x01;
		sendBuf[2] = 0x71;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x02;
		sendBuf[6] = 0x97;//失败
		sendBuf[5] = 0x90;
		*pSendLen=7;
		return FALSE;
	}
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x01;
		sendBuf[2] = 0x71;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x02;
		sendBuf[6] = 0x00;//失败
		sendBuf[5] = 0x90;
		*pSendLen=7;
	return TRUE;
}

//
BOOL SecurityProcessType72(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	WORD Len;
	BOOL bFlag;
	
	static BYTE FrameIndex = 0;
	static BYTE FrameCount = 0;
	static BYTE CerId = 0;
	static WORD CerIndex = 0;

	Len = MAKEWORD(buf[1], buf[0]);

	if(1 == buf[4])
	{
		CerId = buf[2];
		FrameCount = buf[3];
		FrameIndex = 1;
		ProccessCerBuf[0] = CerId;
		memcpy(&ProccessCerBuf[0],buf+5,Len-3);
		CerIndex = Len-3;
		WriteCerFinish = 0;

		if(FrameCount ==  FrameIndex)
			WriteCerFinish = 2;
	}
	else
	{
		FrameIndex++;
		if(CerId == buf[2]&&FrameIndex == buf[4])
		{
			memcpy(&ProccessCerBuf[CerIndex],buf+5,Len-3);
			CerIndex = CerIndex + Len-3;

			if(FrameCount ==  FrameIndex)
				WriteCerFinish = 2;
		}
		else
		{
			CerId = 0;
			FrameCount = 0;
			FrameIndex = 0;
			WriteCerFinish = 2;
			bFlag = FALSE;
		}
	}
	if(WriteCerFinish != 2)
	{
		*pSendLen = 0;
		return FALSE;
	}

	CerId = 0;
	Len = CerIndex;
	bFlag = SecurityLoadCer(ProccessCerBuf,Len,ProccessBuf,&Len,1);

	sendBuf[0] = 0x00;
	sendBuf[1] = 0x01;
	sendBuf[2] = 0x73;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	if(bFlag)
		sendBuf[6] = 0x00;//成功
	else
		sendBuf[6] = 0x97;//失败
	sendBuf[5] = 0x90;
	*pSendLen=7;
	return TRUE;
}
//获取了证书，没有组织发送报文
BOOL SecurityProcessType74(BYTE *buf,BYTE *cerBuf,WORD *cerLen)
{
	WORD Len;
	BOOL bFlag;
#if 1
	bFlag = SecurityGetWorkCerData(ProccessCerBuf,cerLen);
#else
	bFlag = SecurityGetTestCerLen(ProccessCerBuf,sLen);
	if(!bFlag)
	return FALSE;
	Len = MAKEWORD(ProccessCerBuf[1],ProccessCerBuf[0]);
	//
	bFlag = SecurityGetTestCerData(Len,ProccessCerBuf,sLen);
	*cerLen = *sLen;
#endif
	if(bFlag)
	{
		SendCerFlag = TRUE; 
		bySendCerFrameIndex = 1;
		wCerDataLen = *cerLen;
		bySendCerFrameCount = (wCerDataLen-1)/SendCerDataLen +1;
		return TRUE;
	}
	else
	{
		SendCerFlag = FALSE; 
		bySendCerFrameIndex = 0;
		wCerDataLen =0;
		bySendCerFrameCount = 0;
		
		cerBuf[0] = 0x00;
		cerBuf[1] = 0x01;
		cerBuf[2] = 0x75;
		cerBuf[3] = 0x00;
		cerBuf[4] = 0x02;
		cerBuf[6] = 0x94;//失败
		cerBuf[5] = 0x90;
		
		Len=7;
		*cerLen = Len;
		
		return FALSE;
	}
}
//分帧发送
//nRet:0-不发送处理，1-发送数据，并后面还有。
// 2-发送数据，但是是最后一帧，后面没有了
int SecuritySendType75(BYTE *sendBuf,WORD *pSendLen)
{
	int nRet = 0;//不发送处理
	BYTE *p = sendBuf;
	WORD FrameLen;
	int index = 0;
	
	if(!SendCerFlag)
		return nRet;

	if(bySendCerFrameCount ==0||bySendCerFrameIndex>bySendCerFrameCount)
		return nRet;
	
	//发送标志
	if(bySendCerFrameIndex < bySendCerFrameCount)
		FrameLen = SendCerDataLen;
	else
		FrameLen = wCerDataLen - SendCerDataLen*(bySendCerFrameIndex-1);

	FrameLen += 3;
	
	p[index++] = 0x00;
	p[index++] = 0x00;
	p[index++] = 0x75;
	p[index++] = (FrameLen>>8)&0xFF;
	p[index++] = FrameLen&0xFF;
	p[index++] = 06;
	p[index++] = bySendCerFrameCount;
	p[index++] = bySendCerFrameIndex;
	memcpy(p+index,ProccessCerBuf+(bySendCerFrameIndex-1)*SendCerDataLen,FrameLen-3);
	index += (FrameLen-3);
	bySendCerFrameIndex++;

	*pSendLen = index;
	if(bySendCerFrameIndex>bySendCerFrameCount)
	{
		SendCerFlag = FALSE;
		nRet = 2;
	}
	else
	{
		nRet = 1;
	}
	return nRet;
}

//主站公钥远程下发流程，内容估计只有一帧，不再分帧
BOOL SecurityProcessType66(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{
	WORD Len,Len1;
	BOOL bFlag;
	//BYTE enPub1;
	BYTE asKID,PubID,P1;

	Len = MAKEWORD(buf[1], buf[0]);
	//enPub1 =  buf[2];	
	memcpy(ProccessCerBuf,buf+3,Len-1);
	//解密
	bFlag = SecurityDecrypt(ProccessCerBuf,Len-1,ProccessCerBuf,&Len);
	if(bFlag)
	{
		asKID = ProccessCerBuf[Len -1];
		bFlag = SecurityCheckSign1(asKID,ProccessCerBuf,Len-1,ProccessBuf,&Len1);
		if(bFlag)
		{//
			PubID = ProccessCerBuf[0];
			P1 = 0x80|(PubID & 0x7F);
			Len = Len -2 -64;
			bFlag = SecurityUpdatePub(P1,ProccessCerBuf+1,Len,ProccessBuf,&Len);
			if(!bFlag)
			{
				sendBuf[0] = 0x00;
				sendBuf[1] = 0x01;
				sendBuf[2] = 0x67;
				sendBuf[3] = 0x00;
				sendBuf[4] = 0x02;
				sendBuf[6] = 0x99;//失败
				sendBuf[5] = 0x90;
				*pSendLen=7;
				return FALSE;
			}
		}
		else
		{
			sendBuf[0] = 0x00;
			sendBuf[1] = 0x01;
			sendBuf[2] = 0x67;
			sendBuf[3] = 0x00;
			sendBuf[4] = 0x02;
			sendBuf[6] = 0x99;//失败
			sendBuf[5] = 0x90;
			*pSendLen=7;
			return FALSE;
		}
	}
	else
	{
			sendBuf[0] = 0x00;
			sendBuf[1] = 0x01;
			sendBuf[2] = 0x67;
			sendBuf[3] = 0x00;
			sendBuf[4] = 0x02;
			sendBuf[6] = 0x99;//失败
			sendBuf[5] = 0x90;
			*pSendLen=7;
			return FALSE;
	}
	
		sendBuf[0] = 0x00;
		sendBuf[1] = 0x01;
		sendBuf[2] = 0x67;
		sendBuf[3] = 0x00;
		sendBuf[4] = 0x02;
		sendBuf[6] = 0x00;
		sendBuf[5] = 0x90;
		*pSendLen=7;
	return TRUE;
}

//主站获取终端公钥流程
BOOL SecurityProcessType68(BYTE *buf,BYTE *cerBuf,WORD *cerLen)
{
	WORD Len;
	BOOL bFlag;

	bFlag = SecurityGetPubData(cerBuf+5,cerLen);

	if(bFlag)
	{
		//发送待加
		cerBuf[0] = 0x00;
		cerBuf[1] = 0x01;
		cerBuf[2] = 0x69;
		cerBuf[3] = HIBYTE(*cerLen);
		cerBuf[4] = LOBYTE(*cerLen);
		*cerLen = 5 + *cerLen;
		return TRUE;
	}
	else
	{
		cerBuf[0] = 0x00;
		cerBuf[1] = 0x01;
		cerBuf[2] = 0x69;
		cerBuf[3] = 0x00;
		cerBuf[4] = 0x02;
		cerBuf[6] = 0x98;//失败
		cerBuf[5] = 0x90;
		
		Len=7;
		*cerLen = Len;
		
		return FALSE;
	}
}
//

BOOL SecuritySendType1F(WORD sw,BYTE *sendBuf,WORD *pSendLen)
{
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x1F;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	sendBuf[5] = (sw>>8)&0xFF;
	sendBuf[6] = sw&0xFF;
	*pSendLen = 7;
	return TRUE;
}

BOOL SecuritySendType(BYTE type,WORD sw,BYTE *sendBuf,WORD *pSendLen)
{
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x00;
	sendBuf[2] = type;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	sendBuf[5] = (sw>>8)&0xFF;
	sendBuf[6] = sw&0xFF;
	*pSendLen = 7;
	return TRUE;
}

BOOL SecuritySendTypeYunWei(BYTE type,WORD sw,BYTE *sendBuf,WORD *pSendLen)
{
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x40;
	sendBuf[2] = type;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	sendBuf[5] = (sw>>8)&0xFF;
	sendBuf[6] = sw&0xFF;
	*pSendLen = 7;
	return TRUE;
}

int SecuritySendReturnData(BYTE No,BYTE *sendBuf,WORD *pSendLen)
{
	int nRet = 0;
	WORD len;

	len = MAKEWORD(ProccessBuf[1],ProccessBuf[0]);
	if(len<1||len>300)
		return nRet;
	
	memcpy(sendBuf,ProccessBuf+2,len);
	*pSendLen = len;

	nRet = 2;
	return nRet;
}
int SecurityPollSend(BYTE *sendBuf,WORD *pSendLen)
{
	int nRet = 0;//不发送处理
	if(SendCerFlag)
	{
		nRet = SecuritySendType75(sendBuf+4,pSendLen);
		if(nRet>0)
		{
			SecurityProcessFrame(sendBuf,*pSendLen);
			*pSendLen += 6;
		}
		return nRet;
	}
	if(SendMaintToolCerFlag)
	{
		nRet = SecuritySendType43(sendBuf+4,pSendLen);
		if(nRet>0)
		{
			SecurityProcessFrame(sendBuf,*pSendLen);
			*pSendLen += 6;
		}
		return nRet;
	}
	
	switch(SendReturnData)
	{
		case 0x31:
		case 0x3D:
		case 0x3F:
		case 0x44:
		{
			nRet = SecuritySendReturnData(SendReturnData,sendBuf+4,pSendLen);
			if(nRet>0)
			{
				SecurityProcessFrame(sendBuf,*pSendLen);
				*pSendLen += 6;
			}
			break;
		}
		default:
			break;
	}
	SendReturnData = 0;
	
	return nRet;
}



#endif

