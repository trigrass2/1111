/*------------------------------------------------------------------------
功能:      sgc1120A加密报文处理
日期:      20180417
编辑人: lvyi
版本:      V1.0
------------------------------------------------------------------------*/

#include "syscfg.h"

#ifdef ECC_MODE_CHIP
#include "os.h"
#include "cmd.h"
#include "sys.h"


#include "sgc1126a.h"
#include "SecurityProcessorHN.h"

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

BYTE MasterRandom1[8];//身份认证的主站随机数R1
BYTE MasterRandom3[8];//公钥验证的主站随机数R3
BYTE MasterRandom4[8];//密钥协商的主站随机数R4
BYTE MasterRandom6[8];//公钥更新随机数R6


BYTE PK_ID;//公钥验证的公钥索引
static BYTE PucKey[64];
static BYTE MyRandom7[8];//终端随机数R7，对称密钥更新
static BYTE MyRandom[8];//终端随机数
int MasterStaFrameIndexHN= 0;
int SecurityCheckOkHN= 0;
//
static BYTE ProccessCerBuf[1000];
static BYTE MD5SVerData[200];
static BYTE MD5asKID = 0; 
static BYTE MD5_Decrypt[17];
WORD MD5SignlenHN = 0;
WORD MD5_ERRTYPEHN = 0x9000; //验证包是否ok ,返回必须在升级结束才返回    

//
BYTE SecurityChkSum_HN(BYTE *buf, WORD len);
BOOL SecurityChipID_HN(BYTE* rBuf, WORD *rLen);
BOOL SecurityKeyVersion_HN(BYTE* rBuf, WORD *rLen);
BOOL SecurityGenRandom_HN(BYTE* rBuf, WORD *rLen);
BOOL SecurityDecrypt_HN(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityEncrypt_HN(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityUpdateKey_HN(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
void updatePublicKey_HN(void);
BOOL SecurityCheckSign_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);//公钥验证交互流程
BOOL SecurityCheckSign1_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
BOOL SecurityCheckSign2_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);//密钥协商
BOOL SecurityCheckSign3_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);//更新公钥验签
BOOL SecurityCheckSign4_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);//更新芯片公钥
BOOL SecurityCreateSign_HN(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen);
void SecurityProcessFrame_HN(BYTE *sendBuf,WORD sendLen);
void SecurityUnlink_HN(void);
//50,52,54,55,57,58,5A,60,62,63,65
BOOL SecurityProcessType50_HN(BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType52_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType55_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType58_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType60_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType63_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecurityProcessType65_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen);
BOOL SecuritySendType1F_HN(WORD sw,BYTE *sendBuf,WORD *pSendLen);
BOOL SecuritySendType_HN(BYTE type,WORD sw,BYTE *sendBuf,WORD *pSendLen);



//检测到一个不完整的帧（还未收齐）
BYTE SecurityChkSum_HN(BYTE *buf, WORD len)
{//ok
	WORD checksum, i;

	checksum = 0;
	for (i = 0; i < len; i++)
	{
		checksum = checksum + *buf;
		buf ++;
		
	}
	return LOBYTE(checksum & 0xff);
}

DWORD SecuritySearchOneFrame_HN(BYTE *Buf, WORD Len)
{//ok
	WORD off = 0;
	WORD frameLen;
	BYTE checksum;
	
	if(Len < 8)
		return FRAME_LESS;

	

	if(0xEB == Buf[off] &&0xEB == Buf[off+3])
	{
		frameLen = MAKEWORD(Buf[2],Buf[1]);
		 frameLen+=6;
		if(Len < frameLen)
			return FRAME_LESS;
		else 
		{
			if(0xD7 == Buf[frameLen-1])
			{
				checksum = SecurityChkSum_HN(&Buf[4],frameLen-6);
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
******************************************************************/

int SecurityAllFrameProcess_HN(WORD commno , BYTE *Buf, WORD Len,BYTE *sendBuf, WORD *sLen,
BYTE protocolType,BYTE linkaddlen,BYTE * type)  // type 应用类型
{
	BOOL bFlag;
	int Ret = 0;
	tSecurityFrameHN*pFrame = (tSecurityFrameHN*)Buf;
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
	BYTE DeType;//不加密的apptype
	BYTE DeLen;//不加密的101，104明文长度
	//
	if(pFrame->Start1 != 0xEB||pFrame->Start2 != 0xEB)
	{
		return Ret;
	}

	//
	FrameType = MAKEWORD(Buf[5],Buf[4]);
	FrameLen = MAKEWORD(Buf[2],Buf[1]);
	
	if(FrameLen > (Len-6))
	{//FrameLess
		return Ret;
	}
	//
	
	Jiami = FrameType&0x8;
	if(Jiami)
	{
		//
		sendLen = FrameLen-2;
		//解密
		bFlag = SecurityDecrypt_HN(pFrame->Data,sendLen,ProccessBuf,&RevLen);
		if(!bFlag)
		{
			sw = 0x9103;
			SecuritySendType1F_HN(sw,sendBuf+4,sLen);
			SecurityProcessFrame_HN(sendBuf,*sLen);
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
		RevLen = FrameLen -2;//去掉2字节报文类型

		DeType = pMingwen[0];
		DeLen = pMingwen[1];
		
	}

	
	//
	sendLen = pMingwen[1];//101，104原始报文长度
	
	//check
	if(pMingwen[0]<0x20)
	{
						
		if(!SecurityCheckOkHN)
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
				if(pMingwen[0] != 0x00 )
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

			if((RevLen-2-sendLen - 2) != wSecurityDataLen)//信息安全拓展区长度
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
				MD5_ERRTYPEHN= 0x9110;
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
					if(pMingwen[2+5+linkaddlen]!= 0xD2)//写文件激活0x01不加密
					{
						ErrorType = 0x9106;
						break;
					}	
				}
				break;
			}
			case 0x02:
			case 0x03:
			{
				if(pMingwen[0] == 2)
				{
					if(wSecurityDataLen<8)
					{
						ErrorType = 0x9110;
						break;
					}
				}
				
				if(pMingwen[0] == 3)
				{
					if(wSecurityDataLen<73)
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
				for(i=0;i<6;i++)
				{
					if(pMingwen[index+i] != MyRandom[i])
						break;
				}
				if(i<6)
					ErrorType = 0x9104;	
				
				break;
			}
			case 0x04:
			case 0x05://时间
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
			case 0x06:
			case 0x07://随机数、时间
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
						MD5_ERRTYPEHN= 0x9110;
						return FALSE;
					}
					if(Jiami)
					{
						MD5_ERRTYPEHN= 0x9106;
						return FALSE;
					}
				}	
				if(Jiami == 0)
				{
					if(pMingwen[0] != 8)
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
						MD5_ERRTYPEHN= 0x9105;
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
						MD5_ERRTYPEHN= 0x9104;
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
			SecuritySendType1F_HN(sw,sendBuf+4,sLen);
			SecurityProcessFrame_HN(sendBuf,*sLen);
			*sLen += 6;
			Ret = 2;
			return Ret;
		}
	}

		
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
				
					if(RevLen<67)// 1+1+65
						return Ret;
					asKID = pMingwen[RevLen-1];//0x81-0x84
					
					sendLen = pMingwen[1];
					offset = 2;
					memcpy(ProccessBuf2,&pMingwen[offset],sendLen);//应用数据
					offset += sendLen;
					sendLen2 = MAKEWORD(pMingwen[offset+1],pMingwen[offset]);
					offset += 2;
					memcpy(ProccessBuf2+sendLen,&pMingwen[offset],sendLen2-1);//去掉asKID一个字节
					sendLen = RevLen-5;//apptype(1byte)+应用数据长度(1byte)+应用数据长度(2byte)+asKID
					
					if(SecurityCheckSign1_HN(asKID,ProccessBuf2,sendLen,ProccessCerBuf,&RevLen2))
					{
						memcpy(Buf,&pMingwen[2],pMingwen[1]);//把101，104报文拷贝出来
						Ret = 1;
						break;
					}
					else
					{
						sw = 0x9102;
						SecuritySendType1F_HN(sw,sendBuf+4,sLen);
						SecurityProcessFrame_HN(sendBuf,*sLen);
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
			MD5asKID = pMingwen[wSecurityDataLen +2];//wSecurityDataLen==0x4F
			memcpy(MD5SVerData,pMingwen+3,wSecurityDataLen);
			MD5SignlenHN= wSecurityDataLen-1;//去掉签名密钥标识
			MD5_Final(MD5_Decrypt);

			memcpy(ProccessCerBuf,MD5_Decrypt,16);
			memcpy(ProccessCerBuf+16,MD5SVerData,MD5SignlenHN);	
			if(SecurityCheckSign1_HN(MD5asKID,ProccessCerBuf,MD5SignlenHN+16,ProccessBuf,&RevLen2))
			{
				MD5_ERRTYPEHN= 0x9000;
			}
			else
			{
				MD5_ERRTYPEHN= 0x9102;
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


		case 0x50:
		{
			MasterStaFrameIndexHN= 0;
			SecurityCheckOkHN= 0;
			bFlag = SecurityProcessType50_HN(sendBuf+4,sLen);

			//成功
			if(bFlag)
			{
				MasterStaFrameIndexHN = 1;
				Ret = 2;
			}
			else
			{
				MasterStaFrameIndexHN = 0;
				SecurityCheckOkHN= 0;
			}
			break;

		}
		//主站认证
		case 0x52:
		{
			if(MasterStaFrameIndexHN == 1)
			{
			    bFlag = SecurityProcessType52_HN(pMingwen+1,sendBuf+4,sLen);

				//成功
				if(bFlag)
				{
					MasterStaFrameIndexHN = 2;
					Ret = 2;
				}
				else
				{
					MasterStaFrameIndexHN = 0;
					SecurityCheckOkHN= 0;
				}
			}
			
		  break;
		}
		case 0x54:
		{
			if(MasterStaFrameIndexHN == 2)
			{
				sw = MAKEWORD(pMingwen[4],pMingwen[3]);

				if(sw==0x9000)
				{//主站与终端的身份认证成功
					MasterStaFrameIndexHN = 3;
					SecurityCheckOkHN= 1;
					Ret = 3;
				}
				else 
				{//sw ==0x9002
					MasterStaFrameIndexHN = 0;
					SecurityCheckOkHN= 0;
					Ret = 3;
					shellprintf("Error: 主站返回身份认证失败,sw = %04x \n",sw);
				}
			}
			break;
		}
		case 0x55:
		{//公钥验证
			
			if(!SecurityCheckOkHN)
			{
				return FALSE;
			}
			bFlag = SecurityProcessType55_HN(pMingwen+1,sendBuf+4,sLen);
			
			if(bFlag)
			{
				Ret = 2;
			}
			break;
		}
		case 0x57:
		{
			if(SecurityCheckOkHN)
			{
				sw = MAKEWORD(pMingwen[4],pMingwen[3]);

				if(sw==0x9000)
				{//公钥验证成功					
					Ret = 3;
				}
				else 
				{//sw ==0x9003
					Ret = 3;
					shellprintf("Error: 主站返回公钥验证结果失败,sw = %04x \n",sw);
				}
			}
			break;
		}
		case 0x58:
		{//密钥协商
			
			if(!SecurityCheckOkHN)
			{
				return FALSE;
			}
			bFlag = SecurityProcessType58_HN(pMingwen+1,sendBuf+4,sLen);
			
			if(bFlag)
			{
				Ret = 2;
			}
			break;
		}
		case 0x5A:
		{
			if(SecurityCheckOkHN)
			{
				sw = MAKEWORD(pMingwen[4],pMingwen[3]);

				if(sw==0x9000)
				{//密钥协商成功					
					Ret = 3;
				}
				else 
				{//sw ==0x9005
					Ret = 3;
					shellprintf("Error: 主站返回密钥协商结果失败,sw = %04x \n",sw);
				}
			}
			break;
		}
		//公钥更新
		case 0x60:
		{
			if(!SecurityCheckOkHN)
			{
				return FALSE;
			}
			bFlag = SecurityProcessType60_HN(pMingwen+1,sendBuf+4,sLen);
			
			if(bFlag)
			{
				Ret = 2;
			}
			break;
		}
		case 0x62:
		{
			if(SecurityCheckOkHN)
			{
				sw = MAKEWORD(pMingwen[4],pMingwen[3]);

				if(sw==0x9000)
				{//公钥更新成功					
					Ret = 3;
					updatePublicKey_HN();//将更新后的密钥写入tffsa/other
					
				}
				else 
				{//sw ==0x9004
					Ret = 3;
					shellprintf("Error: 主站返回公钥更新结果失败,sw = %04x \n",sw);
				}
			}
			break;
		}
		case 0x63:
		{//对称密钥更新
			if(!SecurityCheckOkHN)
			{
				return FALSE;
			}
			bFlag = SecurityProcessType63_HN(pMingwen+1,sendBuf+4,sLen);
			
			if(bFlag)
			{
				Ret = 2;
			}
			break;
		}
		case 0x65:
		{//对称密钥更新
			if(!SecurityCheckOkHN)
			{
				sw = 0x9005;
				SecuritySendType_HN(pMingwen[0]+1,sw,sendBuf+4,sLen);
				SecurityProcessFrame_HN(sendBuf,*sLen);
				*sLen += 6;
				return 2;
			}
			bFlag = SecurityProcessType65_HN(pMingwen+1,sendBuf+4,sLen);
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
		
		default:
			break;
	}
	if(2 == Ret)
	{
		SecurityProcessFrame_HN(sendBuf,*sLen);
		*sLen += 6;
	}
	if(1 == Ret)
	{
	if(Jiami)
	{
			*type = pMingwen[0];
		#ifdef INCLUDE_COMM_SHOW	
		commBufFill(commno - COMM_START_NO, 3, pMingwen[1], Buf);
		#endif		
	}
	else
	{
		*type = DeType;
		#ifdef INCLUDE_COMM_SHOW	
		commBufFill(commno - COMM_START_NO, 3, DeLen, Buf);
		#endif	
	
	}
	
	}
	return Ret;
}

BOOL SecuritySendMD5Frame_HN(BYTE *sendBuf, WORD *sLen)
{
	BOOL bRet = FALSE;
	
	
	if(MD5_ERRTYPEHN!= 0x9000)
	{
		SecuritySendType1F_HN(MD5_ERRTYPEHN,sendBuf+4,sLen);
		SecurityProcessFrame_HN(sendBuf,*sLen);
		*sLen += 6;
		bRet = FALSE;
	}
	else
	{
		bRet = TRUE;
	}
	return bRet;
	
}

void SecuritySend1FFrame_HN(BYTE *sendBuf, WORD *sLen)
{//ok
	WORD sw;
	sw = 0x9101; //业务应用类型错误

	SecuritySendType1F_HN(sw,sendBuf+4,sLen);
	SecurityProcessFrame_HN(sendBuf,*sLen);
	*sLen += 6;
	
}

//发送数据接口,返回发送长度
// protocolType:1-104、2-101;  typid:Ti标识
WORD SecuritySendFrame_HN(WORD commno,BYTE *buf,WORD wLen,BYTE protocolType,BYTE appType,BYTE typid)
{//buf为101，104原始报文
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
		shellprintf("bytype %d typid = %d  typid = %d\n",byType,typid,appType);
		if(0 == byType)
		{//I帧
			if((typid==203)||(typid==45)||(typid==46))
			{//加密
				ProccessBuf[6] = appType;
				ProccessBuf[5] = 0x08;//
				ProccessBuf[4] = 0;
			}
			else if(typid==211)
			{
				if(appType==0)
				{//升级结束不加密
					ProccessBuf[6] = 0;
					ProccessBuf[5] = 0x00;//
					ProccessBuf[4] = 0;
				}
				else
				{//升级启动加密
					ProccessBuf[6] = appType;
					ProccessBuf[5] = 0x08;//
					ProccessBuf[4] = 0;
				}
			}
			else
			{//其余不加密
				ProccessBuf[6] = 0;
				ProccessBuf[5] = 0x00;//
				ProccessBuf[4] = 0;
			}
			
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
			if((typid==203)||(typid==45)||(typid==46))
			{//加密
				ProccessBuf[6] = appType;
				ProccessBuf[5] = 0x08;//
				ProccessBuf[4] = 0;
			}
			else if(typid==211)
			{
				if(appType==0)
				{//升级结束不加密
					ProccessBuf[6] = 0;
					ProccessBuf[5] = 0x00;//
					ProccessBuf[4] = 0;
				}
				else
				{//升级启动加密
					ProccessBuf[6] = appType;
					ProccessBuf[5] = 0x08;//
					ProccessBuf[4] = 0;
				}
			}
			else
			{//其余不加密
				ProccessBuf[6] = 0;
				ProccessBuf[5] = 0x00;//
				ProccessBuf[4] = 0;
			}
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
	else
	{//终端部分只用到0x00和0x02
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
		if(SecurityEncrypt_HN(ProccessBuf+6,index-6,buf+6,&rLen))
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
		memcpy(buf,ProccessBuf,wLen+10);//6+1+1+2(固定6字节)
		wLen +=6;// 2+1+1+2校验
	}
	SecurityProcessFrame_HN(buf,wLen);
	return wLen+6;
}


//产生芯片序列号
BOOL SecurityChipID_HN(BYTE* rBuf, WORD *rLen)
{//ok
    	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x00;
	chipCmd[1] = 0xB0;
	chipCmd[2] = 0x99;
	chipCmd[3] = 0x05;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x08;

	sw = ecrProcData(chipCmd, 6, NULL, 0, rBuf, rLen);
	if (sw == 0x9000) 
	{
		return TRUE;
	}
     else 
	{
		shellprintf("Error: 读取终端芯片序列号信息失败,sw = %04x\n",sw);
		return FALSE;
	}
}


//获取秘钥版本信息
BOOL SecurityKeyVersion_HN(BYTE* rBuf, WORD *rLen)
{//ok
   	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x5E;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x00;

	sw = ecrProcData(chipCmd, 6, NULL, 0, rBuf, rLen);
	if (sw == 0x9000) 
	{
		return TRUE;
	}
    else 
	{
		shellprintf("Error: 读取终端芯片秘钥版本信息失败,sw = %04x\n",sw);
		return FALSE;
	}
}

//产生随机数
BOOL SecurityGenRandom_HN(BYTE* rBuf, WORD *rLen)
{//ok
    WORD sw;

	BYTE chipCmd[8];
	chipCmd[0] = 0x00;
	chipCmd[1] = 0x84;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;
	chipCmd[4] = 0x00;
	chipCmd[5] = 0x08;

	sw = ecrProcData(chipCmd, 6, NULL, 0, rBuf, rLen);

	if (sw == 0x9000) 
	{
		return TRUE;
	}
    	return FALSE;
}


BOOL SecurityDecrypt_HN(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{//ok
	BYTE chipCmd[8];
	WORD sw;

	chipCmd[0] = 0x80;
	chipCmd[1] = 0x64;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;

	memcpy(ProccessBuf2, sBuf,sLen);

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

BOOL SecurityEncrypt_HN(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{//ok
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x62;
	chipCmd[2] = 0x00;
	chipCmd[3] = 0x00;

	memcpy(ProccessBuf2, MasterRandom4,8); 
	memcpy(ProccessBuf2+8, sBuf,sLen);
	sLen += 8;
	
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,ProccessBuf2,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
		return FALSE;
}

BOOL SecurityCheckSign_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{//公钥验证交互流程,OK
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x4E;
	chipCmd[2] = asKID;
	chipCmd[3] = 0x00;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 公钥验证失败,sw = %04x\n",sw);
		return FALSE;
	}
}

BOOL SecurityCheckSign1_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen) // 805A
{//101，104报文验签,ok
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x5A;
	chipCmd[2] = asKID|0x80;
	chipCmd[3] = 0x00;
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

BOOL SecurityCheckSign2_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{//密钥协商交互流程,OK
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x60;
	chipCmd[2] = asKID;
	chipCmd[3] = 0x01;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 密钥协商加密失败,sw = %04x\n",sw);
		return FALSE;
	}
}

BOOL SecurityCheckSign3_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{//公钥更新验签流程,OK
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x5A;
	chipCmd[2] = asKID;
	chipCmd[3] = 0x00;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 公钥更新验签失败,sw = %04x\n",sw);
		return FALSE;
	}
}

BOOL SecurityCheckSign4_HN(BYTE asKID,BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{//更新芯片公钥流程,OK
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x52;
	chipCmd[2] = asKID;
	chipCmd[3] = 0x00;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 更新芯片公钥失败,sw = %04x\n",sw);
		return FALSE;
	}
}

BOOL SecurityCreateSign_HN(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{//ok,主站认证时对随机数签名
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x80;
	chipCmd[1] = 0x5C;
	chipCmd[2] = 0x01;
	chipCmd[3] = 0x00;
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

//业务报文，sendBuf前4个字节空出来
void SecurityProcessFrame_HN(BYTE *sendBuf,WORD sendLen)
{//ok
	BYTE cs;
	int index = 0;
	sendBuf[index++] = 0xEB;
	sendBuf[index++] = (sendLen>>8)&0XFF;
	sendBuf[index++] = sendLen&0xFF;
	sendBuf[index++] = 0xEB;
	
	cs = SecurityChkSum_HN(sendBuf+4,sendLen);
	index+=sendLen;
	sendBuf[index++] = cs;// 4-1
	sendBuf[index++] = 0xD7;
}
void SecurityUnlink_HN(void)
{
	SecurityCheckOkHN = 0;
	MasterStaFrameIndexHN = 0;
}
//buf:从应用类型之后的字节开始

BOOL SecurityProcessType52_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{//ok，主站身份认证
	BOOL bRet = FALSE;
	WORD Len;
	int i;
	BOOL bFlag;
	WORD sendLen;
	WORD RandomLen;
	*pSendLen = 0;

	
	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 8)
		return bRet;
	for(i=0;i<8;i++ )
	{
		MasterRandom1[i] = buf[i+2];
	}
	//终端调用安全芯片生成终端随机数
	SecurityGenRandom_HN(MyRandom,&RandomLen);
	
	//终端调用安全芯片生成密文认证数据
	bFlag = SecurityCreateSign_HN(buf+2,8,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type52 签名失败\n");
		return bRet;
	}
		//终端调用安全芯片生成终端随机数
	//bFlag = SecurityGenRandom_HN(MyRandom,&RandomLen);
	
	if(sendLen != 0x10)
		return bRet;
	
	sendBuf[0] = 0x00;//报文类型
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x53;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x10;
	sendLen += 5;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;
}

BOOL SecurityProcessType50_HN(BYTE *sendBuf,WORD *pSendLen)
{//ok
	BOOL bRet = FALSE;
	BOOL bFlag;
	WORD sendLen;
	//
	*pSendLen = 0;
	bFlag = SecurityKeyVersion_HN(sendBuf+5,&sendLen);
	if(!bFlag)
	{
		*pSendLen = 0;
		return FALSE;
	}
	
	bFlag = SecurityChipID_HN(sendBuf+6,&sendLen);

	if(!bFlag)
	{
		*pSendLen = 0;
		return FALSE;
	}
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x51;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x09;
	
	sendLen=14;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;
}



BOOL SecurityProcessType55_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{//ok
	BOOL bRet = FALSE;
	WORD Len;
	int i;
	BOOL bFlag;
	WORD sendLen;
	BYTE FID;
	*pSendLen = 0;

	
	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 9)
		return bRet;

	PK_ID = buf[2];//主站公钥索引0x01-0x04
	FID = PK_ID|0x80;//终端密钥索引0x81-0x84
	
	for(i=0;i<8;i++ )
	{
		MasterRandom3[i] = buf[i+3];
	}
	//终端调用安全芯片生成密文认证数据
	bFlag = SecurityCheckSign_HN(FID,buf+3,8,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type55 公钥验证交互失败\n");
		return bRet;
	}
	if(sendLen != 0x68)
		return bRet;
	
	sendBuf[0] = 0x00;//报文类型
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x56;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x68;
	sendLen += 5;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;

}

BOOL SecurityProcessType58_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{//ok
	BOOL bRet = FALSE;
	WORD Len;
	int i;
	BOOL bFlag;
	WORD sendLen;
	BYTE asKID;
	*pSendLen = 0;

	
	Len = MAKEWORD(buf[1], buf[0]);//0X0049
	if(Len != 0x49)
		return bRet;

	PK_ID = buf[74];//主站公钥索引0x01-0x04
	asKID = PK_ID|0x80;//终端密钥索引0x81-0x84
	
	for(i=0;i<8;i++ )
	{
		MasterRandom4[i] = buf[i+2];//密钥协商随机数
	}
	//终端调用安全芯片生成密文认证数据
	bFlag = SecurityCheckSign2_HN(asKID,buf+2,72,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type58 密钥协商交互失败\n");
		return bRet;
	}
	if(sendLen != 0x78)
		return bRet;
	
	sendBuf[0] = 0x00;//报文类型
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x59;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x78;
	sendLen += 5;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;

}


BOOL SecurityProcessType60_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{//公钥更新,ok
	BOOL bRet = FALSE;
	WORD Len;
	int i;
	BOOL bFlag;
	WORD sendLen;
	BYTE asKID;
	*pSendLen = 0;
	
	Len = MAKEWORD(buf[1], buf[0]);//0X008A
	if(Len != 0x8A)
		return bRet;

	PK_ID = buf[2];//更新公钥索引0x01-0x04
	asKID = PK_ID|0x80;//终端密钥索引0x81-0x84
	
	for(i=0;i<8;i++ )
	{
		MasterRandom6[i] = buf[i+3];//公钥更新随机数
	}
	
	//终端调用安全芯片生成密文认证数据
	bFlag = SecurityCheckSign3_HN(asKID,buf+2,137,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type60 公钥更新验签失败\n");
		return bRet;
	}

	bFlag = SecurityCheckSign4_HN(asKID,buf+11,64,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type60 更新芯片公钥失败\n");
		return bRet;
	}
	
	memcpy(PucKey,buf+11,64);//拷贝新的公钥

	bFlag = SecurityCheckSign_HN(asKID,buf+3,8,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type60 公钥对随机数加密失败\n");
		return bRet;
	}
	
	if(sendLen != 0x68)
		return bRet;
	
	sendBuf[0] = 0x00;//报文类型
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x61;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x68;
	sendLen += 5;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;

}

BOOL SecurityProcessType63_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{//对称密钥更新，ok
	BOOL bRet = FALSE;
	WORD Len;
	BOOL bFlag;
	WORD sendLen;
	*pSendLen = 0;

	
	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 0)
		return bRet;

	//终端调用安全芯片生成终端随机数
	bFlag = SecurityGenRandom_HN(sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type63 对称密钥更新取终端随机数失败\n");
		return bRet;
	}
	memcpy(MyRandom7,sendBuf+5,8);

	if(sendLen != 8)
		return bRet;
	
	sendBuf[0] = 0x00;//报文类型
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x64;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x08;
	sendLen += 5;
	*pSendLen = sendLen;
	bRet = TRUE;
	return bRet;

}


BOOL SecurityProcessType65_HN(BYTE *buf,BYTE *sendBuf,WORD *pSendLen)
{//ok
	BOOL bRet = FALSE;
	WORD Len;
	BOOL bFlag;
	WORD sendLen;
	BYTE asKID;

	Len = MAKEWORD(buf[1], buf[0]);
	if(Len != 0x00C5)
		return bRet;
	
	asKID = buf[Len+2-1]|0x80;
	bFlag = SecurityCheckSign3_HN(asKID,buf+2,196,sendBuf+5,&sendLen);;
	if(!bFlag)
	{
		shellprintf("Error: Type65 验证密钥更新数据签名失败\n");
		return bRet;
	}

	bFlag = SecurityUpdateKey_HN(buf+2,132,sendBuf+5,&sendLen);
	if(!bFlag)
	{
		shellprintf("Error: Type60 更新对称密钥数据失败\n");
		return bRet;
	}
	
	sendBuf[0] = 0x00;
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x66;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	if(bFlag)
		sendBuf[6] = 0x00;//成功
	else
		sendBuf[6] = 0x05;//失败
	sendBuf[5] = 0x90;
	
	*pSendLen=7;
	bRet = TRUE;
	return bRet;
}

BOOL SecuritySendType1F_HN(WORD sw,BYTE *sendBuf,WORD *pSendLen)
{//ok
	sendBuf[0] = 0x00;//报文类型2字节
	sendBuf[1] = 0x00;
	sendBuf[2] = 0x1F;
	sendBuf[3] = 0x00;
	sendBuf[4] = 0x02;
	sendBuf[5] = (sw>>8)&0xFF;
	sendBuf[6] = sw&0xFF;
	*pSendLen = 7;
	return TRUE;
}

BOOL SecuritySendType_HN(BYTE type,WORD sw,BYTE *sendBuf,WORD *pSendLen)
{//对应应用类型的错误返回，ok
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
BOOL SecurityUpdateKey_HN(BYTE *sBuf, WORD sLen,BYTE *rBuf, WORD *rLen)
{//ok
	BYTE chipCmd[8];
	WORD sw;
	chipCmd[0] = 0x84;
	chipCmd[1] = 0xD4;
	chipCmd[2] = 0x01;
	chipCmd[3] = 0xFF;
	chipCmd[4] = (sLen>>8)&0xFF;
	chipCmd[5] = sLen&0xFF;

	sw = ecrProcData(chipCmd,6,sBuf,sLen,rBuf,rLen);
	if(0x9000 == sw)
		return TRUE;
	else 
	{
		shellprintf("Error: 更新对称密钥数据失败,sw = %04x \n",sw);
		return FALSE;
	}
}

void updatePublicKey_HN(void)
{
    FILE *fp;
    char path[60];
    char path1[100];
    sprintf(path, "pubkey%d.key", PK_ID);
    GetMyPath(path1, path);
    fp = fopen(path1, "wb");
    fwrite(PucKey, 1, 64, fp);
    fclose(fp);
}

#endif

