#include "syscfg.h"

#ifdef INCLUDE_ECC
#include "os.h"
#include "ecc.h"
#include "encrypt.h"
#include "eccsm2_256.h"

#include "certificate.h"
#include "file.h"
#ifdef  ECC_MODE_OLDCHIP
#include "sgc1126a.h"
#endif

BYTE ChkSum(BYTE *buf, WORD len);
int txdf3(BYTE* buf,BYTE *txd);
unsigned char kkb[4][4];
unsigned char kkx[4][32];
unsigned char kky[4][32];
unsigned char pubID[16] = {0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1};
BYTE F3_flag;
void sm2init()
{
	FILE *fp;
	BYTE i;
	char path[60];
	static BOOL bInitFlag = FALSE;

#ifdef  ECC_MODE_OLDCHIP
	BYTE puckey[64]  = {0};
#endif

	if(bInitFlag)
	{
		return;
	}
	//
#ifdef  ECC_MODE_OLDCHIP
	
	if (!JiamiInit(0))
	{
		shellprintf("Init encrypt chip failed...\n\r");
		return;
	}
#endif

	//testSetKey();
	//return ;
	memset(kkb, 0, 16);
	memset(kkx, 0, 128);
	memset(kky, 0, 128);
	GetMyPath(path, "pubkey1.key");
	fp = fopen(path, "r+b");
	if(fp)
	{
		fread(kkb[0],1,4,fp);
		fread(kkx[0],1,32,fp);
		fread(kky[0],1,32,fp);
		fclose(fp);

#ifdef  ECC_MODE_OLDCHIP
		memcpy(puckey, kkx[0], 32);
		memcpy(&puckey[32], kky[0], 32);
		if (FALSE == ecrPucKeySet(puckey, 64, 0x81))
			return ;
#endif
	}
	GetMyPath(path, "pubkey2.key");
	fp = fopen(path, "r+b");
	if(fp)
	{
		fread(kkb[1],1,4,fp);
		fread(kkx[1],1,32,fp);
		fread(kky[1],1,32,fp);
		fclose(fp);

#ifdef  ECC_MODE_OLDCHIP
		memcpy(puckey, kkx[1], 32);
		memcpy(&puckey[32], kky[1], 32);
		if(FALSE == ecrPucKeySet(puckey, 64, 0x82))
			return ;			
#endif
	}
	GetMyPath(path, "pubkey3.key");
	fp = fopen(path, "r+b");
	if(fp)
	{
		fread(kkb[2],1,4,fp);
		fread(kkx[2],1,32,fp);
		fread(kky[2],1,32,fp);
		fclose(fp);

#ifdef  ECC_MODE_OLDCHIP
		memcpy(puckey, kkx[2], 32);
		memcpy(&puckey[32], kky[2], 32);
		if(FALSE == ecrPucKeySet(puckey, 64, 0x83))
			return;
#endif
	}
	GetMyPath(path, "pubkey4.key");
	fp = fopen(path, "r+b");
	if(fp)
	{
		fread(kkb[3],1,4,fp);
		fread(kkx[3],1,32,fp);
		fread(kky[3],1,32,fp);
		fclose(fp);

#ifdef  ECC_MODE_OLDCHIP
		memcpy(puckey, kkx[3], 32);
		memcpy(&puckey[32], kky[3], 32);
		if (FALSE == ecrPucKeySet(puckey, 64, 0x84))
		return;
#endif
	}
	GetMyPath(path, "pubID.key");
	fp = fopen(path, "r+b");
	if(fp)
	{
		fread(pubID,1,16,fp);
		fclose(fp);
	}
	else
	{
		for(i=0;i<16;i++)
		pubID[i]=1;
	}
	bInitFlag = true;
}
/*********************************************
*返回值说明:
*	FRAME_ERR	:	
*		不是需要解密、验签，正常处理
*	FRAME_OK	:	
*		遥控报文，并有加密，解密成功;
*		低两个字节为整帧的数据长度
*	FRAME_LESS	:
*		遥控报文，并有加密，但格式不对。
*		
*	SSZ:20160906
*
**********************************************/
DWORD SearcheccOneFrame(BYTE *Buf, WORD Len,BYTE linkaddlen,BYTE flag)
{
	//DWORD dw;
	DWORD dwRet = FRAME_ERR;
	DWORD off=0;
	DWORD off2=0;
	BYTE tt[300];
	BYTE *pdata;
	BYTE cs;
	DWORD outlen=100;
#ifdef ECC_MODE_OLDCHIP
	BYTE cksum = 0;
#else
	DWORD tmp;
#endif

	//dw = testEnc();
	
	switch(Buf[0])
	{
		case 0x68:
		{
			if(flag==1)
			{//101
				if(Len<15) 
					break;
				if(Buf[1]!=Buf[2]) 
					break;
				if((Buf[1]+6)> Len)
					break;
				if(Buf[3]!=0x68) 
					break;
				if((Buf[5+linkaddlen]!=0x2d)&&(Buf[5+linkaddlen]!=0x2e)) 
				{
					break;
				}
				if(Len<Buf[1]+6 +4)//保证读到两个0x16
					return FRAME_LESS;

				pdata = Buf+Buf[1]+6;
				off2 = pdata[1]+6+Buf[1]+6;
				
			}
			else
			{//104
				if(Len<15) 
					break;
				if((Buf[5+linkaddlen]!=0x2d)&&(Buf[5+linkaddlen]!=0x2e)) 
				{
					break;
				}
				pdata = Buf+Buf[1]+2;
				off2 = pdata[1]+6+Buf[1]+2;
				
			}
			if(pdata[0]!=0x16 || pdata[3]!=0x16 )
			{
				off = pdata - Buf;
				return FRAME_OK|off;
			}
			if(Len<off2)//读取全部数据
				return FRAME_LESS;
			if(pdata[5+pdata[1]]!=0x16)
			{
				off = pdata - Buf;
				return FRAME_OK|off;
			}
			if(pdata[2]!=64)
			{
				off = pdata - Buf;
				return FRAME_OK|off;
			}
			
			if(pdata[1]<pdata[2])
			{
				off = pdata - Buf;
				return FRAME_OK|off;
			}
			Buf=pdata;
			if(Buf[1]<Buf[2])
				break;

#ifdef ECC_MODE_OLDCHIP
			cksum = Buf[Buf[1] + 4];/**指向校验和 */
			if (!ecrSymmDec((BYTE*)(Buf + 4), (unsigned short)(Buf[1] - Buf[2]), tt,
			(uint16_t*)&outlen, cksum))
			return FRAME_ERR | off;
#else
			tmp=0xffffff00+Buf[Buf[1]+4];
			encrypt_epri((BYTE*)(Buf+4),(unsigned int)(Buf[1]-Buf[2]),tt,(unsigned int *)&outlen,(unsigned int)tmp,(int)(Buf[Buf[1]+4]%3));
#endif
			
			memcpy(Buf+4,tt,Buf[1]-Buf[2]);
			off= off2;
			dwRet = FRAME_OK;
			break;
		}
		case 0x16:
		{
			if(Len<4) 
				return FRAME_LESS;
			if(Buf[3]!=0x16)
				break;
			//if(Buf[2]!=0x40) break;
			if(Len<Buf[1]+6) 
				return FRAME_LESS;
			//crc

#ifdef ECC_MODE_OLDCHIP
			cksum = Buf[Buf[1] + 4];/**指向校验和 */
			if (!ecrSymmDec((BYTE*)(Buf + 4), (unsigned short)(Buf[1] - Buf[2]), tt,
			(uint16_t*)&outlen, cksum))
			return FRAME_ERR | off;
#else
			tmp=0xffffff00|Buf[Buf[1]+4];
			encrypt_epri((BYTE*)(Buf+4),(unsigned int)(Buf[1]-Buf[2]),tt,(unsigned int *)&outlen,(unsigned int)tmp,(int)(Buf[Buf[1]+4]%3));
#endif

			
			//encrypt_epri((BYTE*)(Buf+4),(unsigned int)(Buf[1]-Buf[2]),tt,(unsigned int *)&outlen,(unsigned int)tmp,(int)(Buf[Buf[1]+4]%3));
			memcpy(Buf+4,tt,Buf[1]-Buf[2]);
			pdata=Buf;
			cs=ChkSum(pdata+4,pdata[1]);
			if(cs!=pdata[pdata[1]+4]) return FRAME_ERR|Buf[1];
			dwRet = FRAME_OK;
			off = Buf[1]+6;
			break;
		}
		default:
		{
			off = 0;
			break;
		}
	}
	dwRet |= off;
	return dwRet;
}
BYTE ChkSum(BYTE *buf, WORD len)
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
//
int sm2ver(BYTE *pdata,BYTE *ptxd,int addr,int off,WORD wJiamiType)
{
	ECCrefPublicKey pucPublicKey;  
	ECCSignature pucSignature;
	int mm;
	int dd;
	int tmp1,tmp2;
	struct VCalClock tm;
	BYTE *buf=pdata;
	int nCertNo = 0;//证书序号
	
#ifdef ECC_MODE_OLDCHIP
	unsigned char pucSignBuf[64];
	unsigned short signLen;
#endif

	int idLen =buf[1]-buf[2]+4;	
	pucPublicKey.bits=0x00000100;
	F3_flag=0;
	if(pdata[0]==0x68)
	{
		buf=pdata+pdata[1]+off;
		idLen =buf[1]-buf[2]+4+pdata[1]+off;
	}
	if((buf[0]!=0x16)||(buf[3]!=0x16))
	{
		shellprintf("未加密\r\n");
		return -1;
	}
	//地址
	dd=buf[9];
	dd<<=8;
	dd|=buf[8];
	dd<<=8;;
	dd|=buf[7];
	dd<<=8;
	dd|=buf[6];
	if(dd != addr)
	{
		shellprintf("通讯地址错误%d,装置地址%d\r\n",dd,addr);
		return -1;
	}
	//
	GetSysClock(&tm, CALCLOCK);
	tmp1=(tm.dwMinute*60)+(tm.wMSecond/1000);
	tmp2=buf[13];
	tmp2<<=8;
	tmp2|=buf[12];
	tmp2<<=8;
	tmp2|=buf[11];
	tmp2<<=8;
	tmp2|=buf[10];
	mm=tmp1-tmp2;
	if(mm<0) mm=-mm;
	if(mm>60)
	{
		mm-=3600*8;
		if((mm<-60)||(mm>60))
		{
			//logMsg("时间差%ds\r\n",mm, 0, 0, 0, 0, 0, 0);
			shellprintf("时间差>60s\r\n");
			return -1;
		}
	}
	
	memcpy(pucSignature.r,buf+buf[1]-buf[2]+4,32);
	memcpy(pucSignature.s,buf+buf[1]-buf[2]+4+32,32);
	//验签
	if(1 == wJiamiType)
	{
		memcpy((BYTE*)&pucPublicKey.bits,kkb[(buf[4]&3)],4);
		memcpy(pucPublicKey.x,kkx[(buf[4]&3)],32);
		memcpy(pucPublicKey.y,kky[(buf[4]&3)],32);
		
#ifdef ECC_MODE_OLDCHIP
		signLen = 64;
		memcpy(pucSignBuf, buf + buf[1] - buf[2] + 4, signLen);
		mm = ecrVerifySign(pdata, idLen, pucSignBuf, signLen, 0x81 + (buf[4] & 3));
#else 
		mm=	SM2_Verify(pdata,idLen,pubID,16,&pucPublicKey,&pucSignature);
#endif
	}
	else
	{
		nCertNo = buf[4]&0x3;
		//判断证书时间
		if(pCertificateData->AbsTBegin[nCertNo].dwMinute > tm.dwMinute)
		{
			if(pCertificateData->AbsTBegin[nCertNo].dwMinute - tm.dwMinute > 1)
				return -1;
		}

		if(pCertificateData->AbsTEnd[nCertNo].dwMinute < tm.dwMinute)
		{
			if(tm.dwMinute - pCertificateData->AbsTBegin[nCertNo].dwMinute >  1)
				return -1;
		}
#ifdef ECC_MODE_OLDCHIP
		signLen = 64;
		memcpy(pucSignBuf, buf + buf[1] - buf[2] + 4, signLen);
		mm = ecrVerifySign(pdata, idLen, pucSignBuf, signLen, 0x81 + (buf[4] & 3));
#else
		//鉴签
		mm = SM2_Verify(pdata,idLen,pubID,16,&pCertificateData->PublicKey[nCertNo],&pucSignature);
#endif
	}
	if(mm!=0) 
	{
		shellprintf("验签错误\r\n");

		return -1;
	}

	//
	if(1 == wJiamiType)
	{
		mm=0;
		if(buf[0]==0x16)
		{
			switch((buf[4]>>2)&0xf)
			{
				case 4:
					memcpy(kkx[(buf[5]&3)],buf+22,32);

					memcpy(kky[(buf[5]&3)],buf+54,32);
					F3_flag=(buf[5]&3)+1;
				case 3:
					mm=txdf3(pdata,ptxd);
					break;
				case 1:
					break;
				default:
					break;
			}
		}
	}
	return mm;
}
void wr_F3()
{
	FILE *fp;
	char path[60];
	char path1[100];
#ifdef  ECC_MODE_OLDCHIP
	BYTE puckey[64]  = {0};
	BYTE keyNum = 0;
#endif
	if(F3_flag==0) return;
	F3_flag--;
	sprintf(path,"pubkey%d.key",(F3_flag&3)+1);
	GetMyPath(path1, path);
	fp = fopen(path1, "wb");
	if(fp)
	{
		fwrite(kkb[(F3_flag&3)],1,4,fp);
		fwrite(kkx[(F3_flag&3)],1,32,fp);
		fwrite(kky[(F3_flag&3)],1,32,fp);
		fclose(fp);	

#ifdef  ECC_MODE_OLDCHIP
		keyNum = F3_flag&3;
		memcpy(puckey, kkx[keyNum], 32);
		memcpy(&puckey[32], kky[keyNum], 32);
		if (FALSE == ecrPucKeySet(puckey, 64, 0x81+keyNum))
			return ;
#endif
	}
	F3_flag=0;
}
int txdf3(BYTE* buf,BYTE *txd)
{
	int len=0;
	BYTE tt[200];
	DWORD tmp, outlen;
	struct VCalClock tm;
#ifdef  ECC_MODE_OLDCHIP	
   	WORD	encOutLen;
#endif

	ECCrefPublicKey pucPublicKey;  
	memcpy((BYTE*)&(pucPublicKey.bits),kkb[(buf[5]&3)],4);
	memcpy(pucPublicKey.x,kkx[(buf[5]&3)],32);		
	memcpy(pucPublicKey.y,kky[(buf[5]&3)],32);
	txd[len++]=0x16;
	txd[len++]=0x8e;
	txd[len++]=0x80;
	txd[len++]=0x16;
	memcpy(txd+len,buf+4,6);
	len+=6;
	GetSysClock(&tm, CALCLOCK);
	tmp=(tm.dwMinute*60)+(tm.wMSecond/1000);
	txd[len++]=tmp;
	tmp>>=8;
	txd[len++]=tmp;
	tmp>>=8;
	txd[len++]=tmp;
	tmp>>=8;
	txd[len++]=tmp;
	txd[len++]=1;
	txd[len++]=0;
	txd[len++]=0;
	txd[len++]=0;

#ifdef  ECC_MODE_OLDCHIP	
	memset(buf+18,0,32);
	if (!ecrPucKeyEnc(buf + 14, 32, buf+14,32, &txd[len], &encOutLen, (buf[5] & 3) + 0x81))
	{
		return 0;
	}
	if (encOutLen != 128) 
	{
		shellprintf("enc data's len = %d is not right\n\r", encOutLen);
		return 0;
	}
#else
	memset(buf+18,0,32);		
	if(SM2_Encrypt(buf+14,32,&pucPublicKey,buf+14,32,(ECCCipher *)(txd+len))!=0)
		return 0;
#endif

	len+=128;
	txd[len++]=ChkSum(txd+4,txd[1]);
	txd[len++]=0x16;
	tmp=0xffffff00|txd[txd[1]+4];


#ifdef  ECC_MODE_OLDCHIP
	ecrSymmEnc((BYTE*)(txd + 4), (WORD)(txd[1] - txd[2]), tt, (WORD*)&outlen, tmp);
#else
	encrypt_epri((BYTE*)(txd+4),(unsigned int)(txd[1]-txd[2]),tt,(unsigned int *)&outlen,(unsigned int)tmp,(int)(txd[txd[1]+4]%3));
#endif
	memcpy(txd+4,tt,txd[1]-txd[2]);

	//ssz:20161011
	txd[2] = 0;
	
	return len;
}

#endif

