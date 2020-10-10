/*
融合sgc1126a及sc1161Y 加密代码   cjl 2017年5月19日10:39:58
*/

#include "syscfg.h"
#include "bsp.h"
#include "spi.h"
#include "sgc1126a.h"
#include "sys.h"

BYTE send[1000] = {0};
BYTE recv[1000] = {0};
BYTE cmdbuffer[1000] = {0};

#ifdef  ECC_MODE_CHIP
#if (TYPE_CPU == CPU_STM32F4)
#define JiaMiSPI   SPI2
#elif (TYPE_CPU == CPU_SAM9X25)
#define JiaMiSPI   SPI0
#endif


void jiamidelay_us(WORD us)
{
    int i;
    //for(i=0; i<(us*28); i++);
   // for(i=0; i<(us*84); i++);
   for(i=0; i<(us*100); i++);
}


WORD ecrProcData1(BYTE* cmd, BYTE cmdLen, BYTE* sBuf,
                     WORD sLen, BYTE* rBuf, WORD* rLen)
{
    BYTE data, resp[4];
    BYTE lrc1, lrc2;
    WORD i, dataLen;
	WORD sendcnt = 0;

    if (cmd == NULL || rBuf == NULL) 
	{
        return 0xffff;
    }

    /* 1 send data */
    /* 1.1 caculate lrc */
    lrc1 = 0;
    for (i = 0; i < cmdLen; i++)
	{
        lrc1 ^= cmd[i];
    }
    if (sBuf != NULL && sLen > 0)
	{
	    for (i = 0; i < sLen; i++)
		{
	        lrc1 ^= sBuf[i];
	    }

    }
    lrc1 = ~lrc1;
	
    /* 1.2 send sof */
  	//JiamiWriteCtrl(0x55);
  	send[0] = 0x55;
	sendcnt += 1;
    /* 1.3 send cmd and data */
    for (i = 0; i < cmdLen; i++) 
	{
        //JiamiWriteCtrl(cmd[i]);
        send[sendcnt++] = cmd[i];
    }
    for (i = 0; i < sLen; i++)
	{
       // JiamiWriteCtrl(sBuf[i]);
       send[sendcnt++] = sBuf[i];
    }

    /* 1.4 send lrc */
    //JiamiWriteCtrl(lrc1);
	send[sendcnt++] = lrc1;
 	SPITrans_BAddr_Jiami(JiaMiSPI, 0,send,recv, sendcnt,1,1);

    /* 2.1 get busy state */
    i = 0;
	data = JiamiReadReg(0xaa,1,0);

	if(data != 0x55)
	{
	    while (1) 
		{
	        data = JiamiReadReg(0xaa,0,0);

	       	if (data == 0x55)
			{
	          	 break;
	        }

	        //if over 3s return false;
			i++;
			if(i>1000)
			{
				data = JiamiReadReg(0xaa,0,1);
				return 0xFFFF;
			}
			else 
				jiamidelay_us(100);//thSleep(1);
	    }
	}

    /* 2.2 get resp */

	JiamiReadRegs(0xf8, resp, 4,0,0);
 

    dataLen = (WORD)resp[2] * 256 + resp[3];
    *rLen = dataLen;

	if(dataLen > 999)
	{
		*rLen = dataLen = 1;
		 JiamiReadRegs(0x00, rBuf, (dataLen+1),0,1);
		  return 0xffff;
	}
    /* 2.3 get data and lrc */

	//临时recv接收防止内存溢出
	JiamiReadRegs(0x00, recv, (dataLen+1),0,1);

    /* 2.4 check lrc */
    lrc2 = 0;
    lrc2 = resp[0] ^ resp[1] ^ resp[2] ^ resp[3];
    for (i = 0; i < *rLen; i++) 
	{
        lrc2 ^= recv[i];
    }
    lrc2 = ~lrc2;
		//JIAMI_CSN_L;
    if (lrc2 != recv[*rLen]) 
	{
        shellprintf("lrc err\n\r");
        return 0xffff;
    }   

	memcpy(rBuf,recv,dataLen+1);
	
    /* 3 return sw code */
    return ((WORD)resp[0] * 256 + resp[1]);
}

WORD ecrProcData(BYTE* cmd, BYTE cmdLen, BYTE* sBuf,
                     WORD sLen, BYTE* rBuf, WORD* rLen)
{
	WORD i,sw;
#ifdef INCLUDE_SEC_CHIP
    if(g_ipsecinit == 1) //IPSec口配置规约且配置有效，则为南网加密
    {
      sw = IPSecWriteCOS(cmd,cmdLen,sBuf,sLen,rBuf,rLen); 
    }
    else
#endif
    {
      memcpy(cmdbuffer,sBuf,1000);
      for(i = 0; i < 4;i++)
      {//SPI数据重发机制，3次
        memcpy(sBuf,cmdbuffer,sLen);
        sw = ecrProcData1(cmd,cmdLen,sBuf,sLen,rBuf,rLen);
        if(0x9000 == sw)
        {
          break;
        }
      }
    }
	return sw;
}


void JiamiWriteCtrl(BYTE bCmd,BOOL on,BOOL off)
{
	send[0]  = bCmd;
	 SPITrans_BAddr_Jiami(JiaMiSPI, 0,send,recv, 1,on,off);
}


void JiamiWriteReg(BYTE bAddr, BYTE bValue,BOOL on,BOOL off)
{
	send[0] = bValue;	
    SPITrans_BAddr_Jiami(JiaMiSPI, 0,send,recv, 1,on,off);
}

void JiamiWriteRegs(BYTE bAddr, BYTE *pBuffer, WORD bCount,BOOL on,BOOL off)
{		
    SPITrans_BAddr_Jiami(JiaMiSPI, 0,pBuffer, recv, bCount,on,off);
}

BYTE JiamiReadReg(BYTE bAddr,BOOL on,BOOL off)
{
    send[0] = bAddr;
 	SPITrans_BAddr_Jiami(JiaMiSPI, 0, send, recv, 1,on,off);
    return recv[0];
}

void JiamiReadRegs(BYTE bAddr, BYTE *pBuffer, WORD bCount,BOOL on,BOOL off)
{	
	memset(send,0,bCount);
    send[0] = bAddr;
	
    SPITrans_BAddr_Jiami(JiaMiSPI, 0, send, pBuffer, bCount,on,off);
}

char* ecrProcSw(WORD sw)
{
    char *errStr = NULL;

    if (((sw & 0xfff0) == 0x9e20) || ((sw & 0xfff0) == 0x9e30)) {
        sw &= 0xfff0;
    }
    
    switch (sw) {
    case 0x6400 :
        errStr = "内部执行出错";
        break;
            
    case 0x6581 :
        errStr = "存储器故障";
        break;
   
    case 0x6700 :
        errStr = "Lc或Le 长度错";
        break;

    case 0x6901 :
        errStr = "离线计数器为0/时间比较错误/命令不接受，无效状态";
        break;

    case 0x6982 :
        errStr = "不满足安全状态";
        break;

    case 0x6983 :
        errStr = "PIN/KEY 已锁定";
        break;

    case 0x6984 :
        errStr = "引用数据无效(未申请随机数)";
        break;

    case 0x6985 :
        errStr = "使用条件不满足/计算时不存在临时密钥";
        break;

    case 0x6986 :
        errStr = "无当前 EF\n\r";
        break;
    
    case 0x6988 :
        errStr = "SM 数据对象不正确";
        break;
    
    case 0x698f :
        errStr = "DF 名已存在";
        break;
    
    case 0x6a80 :
        errStr = "数据域不正确";
        break;
    
    case 0x6a86 :
        errStr = "参数 P1、P2 不正确";
        break;
    
    case 0x6a88 :
        errStr = "未找到引用数据";
        break;
    
    case 0x6d00 :
        errStr = "命令不存在";
        break;
    
    case 0x6e00 :
        errStr = "命令类型错,CLA错";
        break;
    
    case 0x6f00 :
        errStr = "数据无效";
        break;
    
    case 0x9000 :
        errStr = "命令执行成功";
        break;
    
    case 0x9086 :
        errStr = "验签错误";
        break;

    case 0x9e20 :
        errStr = "文件错误";
        break;

    case 0x9e30 :
        errStr = "算法计算错误";
        break;

    case 0x9e57 :
        errStr = "认证错误";
        break;

    case 0x9E60 :
        errStr = "建立会话错误";
        break;

    case 0x9e5e :
        errStr = "CA证书错误";
        break;

    default :
        errStr = "未知错误码";
    }

    return errStr;
}

char* sc1161y_ecrProcSw(WORD sw)
{
    char *errStr = NULL;

	if ((sw & 0x63c0) == 0x63c0) 
	{
		sw &= 0x63c0;
	}
    
    switch (sw) {
    case 0x6D00 :
        errStr = "INS不支持";
        break;
            
    case 0x6E00 :
        errStr = "CLA不支持";
        break;
   
    case 0x6A86 :
        errStr = "P1P2不正确";
        break;

    case 0x6988 :
        errStr = "计算错误/MAC错误";
        break;
    case 0x9090 :
	 errStr = "认证失败";
        break;	
    case 0x9091 :
	 errStr = "密钥更新失败";
        break;	
    case 0x9092 :
	 errStr = "密钥恢复失败";
        break;	
    case 0x9093 :
	 errStr = "证书导入失败";
        break;	
    case 0x9094 :
	 errStr = "证书导出失败";
        break;	
    case 0x9095 :
	 errStr = "证书提取失败";
        break;	
    case 0x9096 :
	 errStr = "证书更新/下载数据包接收失败";
        break;
    case 0x9097 :
	 errStr = "证书远程更新/下载失败";
        break;
    default :
        errStr = "未知错误码";
    }
    return errStr;
}

/*mode 0为普通加密，sgc1126
			 1为最新的配电终端加密 sc11611Y
*/
BOOL JiamiInit(BYTE mode)
{
	BYTE recvbuf[64], i;
	BYTE chipSerCmd[8];// = {0x00, 0xb0, 0x99, 0x05, 0x00, 0x08};
	WORD recvlen, sw;
#ifdef INCLUDE_SEC_CHIP
	if((g_ipsecinit == 1) && (mode  == 0)) //IPSEC只有新的加密 
		return FALSE;
#endif	
	chipSerCmd[0] = 0;
	chipSerCmd[1] = 0xb0;
	chipSerCmd[2] = 0x99;
#ifdef INCLUDE_SEC_CHIP
	if(g_ipsecinit == 1) //IPSec有效
	{
		chipSerCmd[3] = 0x0;
	}
	else
#endif
	chipSerCmd[3] = 0x5;
	chipSerCmd[4] = 0;
	if(mode)
		chipSerCmd[5] = 0x2;
	else
		chipSerCmd[5] = 0x8;
	chipSerCmd[6] = 0;
	chipSerCmd[7] = 8;

	//spi总线为100M
	SPIInit(JiaMiSPI, SPI_BaudRatePrescaler_64, SPI_DataSize_8b);
    /* read chip serial num */
	
	if(mode)
    sw = ecrProcData(chipSerCmd, sizeof(chipSerCmd), NULL, 0, recvbuf, &recvlen); //sc1161Y
	else
		sw = ecrProcData(chipSerCmd, 6, NULL, 0, recvbuf, &recvlen); //sgc1126 or sgc1120

    if (sw == 0x9000) 
	{
        shellprintf("Init Encrypt Chip Sucessed\n\r");
        shellprintf("\n\rEncrypt Chip Serial Num: ");
        for (i = 0; i < recvlen; i++)
		{
            shellprintf("%02x ", recvbuf[i]);
    	}
		shellprintf("\n\r");
        return TRUE;
    }

	if(mode)
		shellprintf("Ecc Init Failed: %s %04x\n\r",sc1161y_ecrProcSw(sw), sw);
	else
		shellprintf("Ecc Init Failed: %s %04x\n\r", ecrProcSw(sw), sw);
	return FALSE;
}




extern unsigned char kkb[4][4];
extern unsigned char kkx[4][32];
extern unsigned char kky[4][32];
extern unsigned char pubID[16];
//产生随机数
BOOL ecrGenRandom(BYTE* randBuf, WORD ranLen)
{
    WORD recvLen, sw;
    BYTE cmd[] = {0x00,0x84, 0x00, 0x00, 0x00, 0x00};

    cmd[4] = (ranLen & 0xff00) >> 8;
    cmd[5] = ranLen & 0x00ff;
    sw = ecrProcData(cmd, sizeof(cmd), NULL, 0, randBuf, &recvLen);
    if (sw == 0x9000) 
	{
        if (recvLen != ranLen)
		{
            return FALSE;
     	}

        return TRUE;
    }

    return FALSE;
}


/**********************************
 brief check digital signature
 	     	鉴签函数

* @param   pucDataBuf   原始数据内容
* @param   pucDataLen   原始数据长度
* @param   pucSignBuf   签名内容
* @param   signLen      签名长度
* @param   pucKeyNum    公钥号
*
************************************************************/
BOOL ecrVerifySign(BYTE* pucDataBuf, BYTE pucDataLen, BYTE* pucSignBuf,
                   WORD signLen, BYTE pucKeyNum)
{
    BYTE cmd[24];
    WORD tlen, sw, recvLen;
	BYTE pucDSBuf[300], DSOutBuf[20];
    BYTE pucId[16] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                         0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

    //L1=2+L2(pucIDLen)+L3(pucDataLen)+LEN(pucSignature)
    tlen = 2 + 16 + pucDataLen + signLen;

    cmd[0] = 0x80;//CLA
    cmd[1] = 0x5a;//INS
    cmd[2] = pucKeyNum;//P1 公钥号
    cmd[3] = 0x00; //P2
    cmd[4] = tlen >> 8;//L1 
    cmd[5] = tlen & 0x00ff;
    cmd[6] = 16; //L2
    memcpy(&cmd[7], pucId, 16);
    cmd[23] = pucDataLen;//L3

    memcpy(pucDSBuf, pucDataBuf, pucDataLen);
    memcpy(&pucDSBuf[pucDataLen], pucSignBuf, signLen);
 
	sw = ecrProcData(cmd, sizeof(cmd), pucDSBuf, pucDataLen + signLen,
                     DSOutBuf, &recvLen);
    if (sw == 0x9000)
    {
        shellprintf("鉴签函数OK\n\r");
        return OK;
    }

    shellprintf("check sign: %s %04x\n\r", ecrProcSw(sw), sw);
    return ERROR;
}

/****************************************************
 *\brief Public key encryption
 *\note pucDataLen and randLen must equal 0x20

 		公钥加密函数
 ***************************************************/
BOOL ecrPucKeyEnc(BYTE* pucDataBuf, BYTE pucDataLen, BYTE* randBuf, BYTE randLen,
                  BYTE* outBuf, WORD* outLen, BYTE pucKeyNum)
{
    WORD sw, L1;
    BYTE cmd[6], P2;
    BYTE inBuf[0x40];

    L1 = pucDataLen + randLen;
    P2 = 0x00;

    memcpy(inBuf, pucDataBuf, pucDataLen);
    if (randLen > 0) 
	{
        memcpy(&inBuf[randLen], randBuf, randLen);
        P2 = 0x02;
    }
        
    cmd[0] = 0x80;
    cmd[1] = 0x4e;
    cmd[2] = pucKeyNum;//P1
    cmd[3] = P2;//P2
    cmd[4] = L1 >> 8;
    cmd[5] = L1 & 0x00ff;
    sw = ecrProcData(cmd, 6, inBuf, L1, outBuf, outLen);
    if (sw == 0x9000) 
	{
        return TRUE;
    }

    shellprintf("pucKey Enc: %s %04x\n\r", ecrProcSw(sw), sw);
    return FALSE;
}


/***************************************
 *\brief set public key
 *\note keyLen must equal 0x40
           公钥导入函数
 ***************************************/
BOOL ecrPucKeySet(BYTE* keyBuf, WORD keyLen, BYTE pucKeyNum)
{
    WORD sw, recvLen;
    BYTE cmd[6];
    BYTE recvBuf[32];

    cmd[0] = 0x80;
    cmd[1] = 0x52;
    cmd[2] = pucKeyNum;
    cmd[3] = 0x00;
    cmd[4] = keyLen >> 8;
    cmd[5] = keyLen & 0x00ff;

    sw = ecrProcData(cmd, sizeof(cmd), keyBuf, keyLen, recvBuf, &recvLen);
    if (sw == 0x9000) 
	{
        return TRUE;
    }

    shellprintf("pucKey Set: %s %04x\n\r", ecrProcSw(sw), sw);
    return FALSE;
}


/*****************************************
 *\brief symmetrical encryption

 			对称加密函数
 ****************************************/
BOOL ecrSymmEnc(BYTE* inBuf, WORD inLen, BYTE* outBuf,
                WORD* outLen, BYTE pucKey)
{
    WORD sw;
    BYTE cmd[6];

    cmd[0] = 0x00;
    cmd[1] = 0xe7;
    cmd[2] = pucKey;
    cmd[3] = 0x00;
    cmd[4] = inLen >> 8;
    cmd[5] = inLen & 0x00ff;

    sw = ecrProcData(cmd, sizeof(cmd), inBuf, inLen, outBuf, outLen);
    if (sw == 0x9000) 
	{
        return TRUE;
    }
    return FALSE;
}

/*******************************************
 *\brief symmetrical decryption
 
			 对称解密
 ********************************************/
BOOL ecrSymmDec(BYTE* inBuf, WORD inLen, BYTE* outBuf,
                WORD* outLen, BYTE pucKey)
{
    WORD sw;
    BYTE cmd[6];

    cmd[0] = 0x00;
    cmd[1] = 0xe7;
    cmd[2] = pucKey;
    cmd[3] = 0x01;
    cmd[4] = inLen >> 8;
    cmd[5] = inLen & 0x00ff;

    sw = ecrProcData(cmd, sizeof(cmd), inBuf, inLen, outBuf, outLen);
    if (sw == 0x9000) 
	{
        return TRUE;
    }
    return FALSE;
}


static BYTE F3_flag;


/***********************************************************/
/** 
* @return 
* @brief - 更新秘钥

************************************************************/

void updatePublicKey(void)
{
    FILE *fp;
    char path[60];
    char path1[100];
    if(F3_flag == 0) return;
    F3_flag--;
    sprintf(path, "pubkey%d.key", (F3_flag & 3) + 1);
    GetMyPath(path1, path);
    fp = fopen(path1, "wb");
  //  fwrite(tt, 1, 4, fp);
    fwrite(kkx[(F3_flag & 3)], 1, 32, fp);
    fwrite(kky[(F3_flag & 3)], 1, 32, fp);
    fclose(fp);
    F3_flag = 0;
}

#if 0
/***********************************************************/
/** 
* @return 
* @brief - 加密功能公钥导入
* 
************************************************************/

int encInit()
{
    FILE *fp;

#ifdef  ECC_MODE_CHIP
	BYTE puckey[64] = {0,};

    if (!JiamiInit(0)) {
        shellprintf("Init encrypt chip failed...\n\r");
        return -1;
    }
#endif

    fp = fopen("/tffsa/other/pubkey1.key", "r+b");
	if (NULL != fp) {
        fread(kkx[0], 1, 4, fp);
        fread(kkx[0], 1, 32, fp);
        fread(kky[0], 1, 32, fp);
        fclose(fp);

#ifdef  ECC_MODE_CHIP
		memcpy(puckey, kkx[0], 32);
		memcpy(&puckey[32], kky[0], 32);
		if (FALSE == ecrPucKeySet(puckey, 64, 0x81))
			return -1;
#endif
	}
	
    fp = fopen("/tffsa/other/pubkey2.key", "r+b");
	if (NULL != fp)	{	
        fread(kkx[1], 1, 4, fp);
        fread(kkx[1], 1, 32, fp);
        fread(kky[1], 1, 32, fp);
        fclose(fp);

#ifdef  ECC_MODE_CHIP
		memcpy(puckey, kkx[1], 32);
		memcpy(&puckey[32], kky[1], 32);
		if(FALSE == ecrPucKeySet(puckey, 64, 0x82))
			return -1;			
#endif
	}
	
    fp = fopen("/tffsa/other/pubkey3.key", "r+b");
	if (NULL != fp) {
        fread(kkx[2], 1, 4, fp);
        fread(kkx[2], 1, 32, fp);
        fread(kky[2], 1, 32, fp);
        fclose(fp);

#ifdef  ECC_MODE_CHIP
		memcpy(puckey, kkx[2], 32);
		memcpy(&puckey[32], kky[2], 32);
		if(FALSE == ecrPucKeySet(puckey, 64, 0x83))
			return -1;
#endif
	}

    fp = fopen("/tffsa/other/pubkey4.key", "r+b");
	if (NULL != fp)	{
        fread(kkx[3], 1, 4, fp);
        fread(kkx[3], 1, 32, fp);
        fread(kky[3], 1, 32, fp);
        fclose(fp);

#ifdef  ECC_MODE_CHIP
		memcpy(puckey, kkx[3], 32);
		memcpy(&puckey[32], kky[3], 32);
		if (FALSE == ecrPucKeySet(puckey, 64, 0x84))
			return -1;
#endif
	}
	return 0;
}
#endif
/***********************************************************/
/** 
* @return 
* @brief - 湖南加密功能公钥导入
* 
************************************************************/
	
int EndataInit(void)
{
	FILE *fp;

#ifdef  ECC_MODE_CHIP
	BYTE puckey[64];

	if (!JiamiInit(0)) {
		shellprintf("Init encrypt chip failed...\n\r");
		return -1;
	}
#endif

	fp = fopen("/tffsa/other/pubkey1.key", "r+b");
	if (NULL != fp) {
		fread(puckey, 1, 64, fp);
		fclose(fp);

#ifdef  ECC_MODE_CHIP
		if (FALSE == ecrPucKeySet(puckey, 64, 0x81))
			return -1;
#endif
	}
	
	fp = fopen("/tffsa/other/pubkey2.key", "r+b");
	if (NULL != fp) {	
		fread(puckey, 1, 64, fp);
		fclose(fp);

#ifdef  ECC_MODE_CHIP
		if(FALSE == ecrPucKeySet(puckey, 64, 0x82))
			return -1;			
#endif
	}
	
	fp = fopen("/tffsa/other/pubkey3.key", "r+b");
	if (NULL != fp) {
		fread(puckey, 1, 64, fp);
		fclose(fp);

#ifdef  ECC_MODE_CHIP
		if(FALSE == ecrPucKeySet(puckey, 64, 0x83))
			return -1;
#endif
	}

	fp = fopen("/tffsa/other/pubkey4.key", "r+b");
	if (NULL != fp) {
		fread(puckey, 1, 64, fp);
		fclose(fp);

#ifdef  ECC_MODE_CHIP
		if (FALSE == ecrPucKeySet(puckey, 64, 0x84))
			return -1;
#endif
	}
	return 0;
}

#if 0

static int encryptRespond(BYTE* buf, BYTE *txd)
{
    int len = 0;
    BYTE tt[200];
    uint16_t encOutLen;
    DWORD tmp, outlen;
    struct VCalClock tm;

    ECCrefPublicKey pucPublicKey;
    pucPublicKey.bits = 0x00000100;
    memcpy(pucPublicKey.x, kkx[(buf[5] & 3)], 32);
    memcpy(pucPublicKey.y, kky[(buf[5] & 3)], 32);
    txd[len++] = 0x16;
    txd[len++] = 0x8e;
    txd[len++] = 0x80;
    txd[len++] = 0x16;
    memcpy(txd + len, buf + 4, 6);
    len += 6;
    GetSysClock(&tm, CALCLOCK);
    tmp = (tm.dwMinute * 60) + (tm.wMSecond / 1000);
    txd[len++] = tmp;
    tmp >>= 8;
    txd[len++] = tmp;
    tmp >>= 8;
    txd[len++] = tmp;
    tmp >>= 8;
    txd[len++] = tmp;//数据域前14个字节
    txd[len++] = 1;//公钥个数
    txd[len++] = 0;//保留
    txd[len++] = 0;//保留
    txd[len++] = 0;//保留

    // 1使用主站下发的4个字节随机数进行公钥加密 
#ifndef  ECC_MODE_CHIP
    memset(buf + 18, 0, 32);//在随机数4个字节后加32个0
	if (SM2_Encrypt(buf + 14, 32, &pucPublicKey, buf + 14, 32, (ECCCipher *)(txd + len)) != 0) {
		return 0;
	}
#else
    //if (!ecrPucKeyEnc(buf + 14, 32, buf + 14, 32, &txd[len], &encOutLen, (buf[5] & 3) + 0x81)) {
    if (!ecrPucKeyEnc(buf + 14, 32, NULL, 0, &txd[len], &encOutLen, (buf[5] & 3) + 0x81)) {
        return 0;
    }
    if (encOutLen != 128) {
        shellprintf("enc data's len = %d is not right\n\r", encOutLen);
        return 0;
    }
#endif

    len += 128;
    txd[len++] = ChkSum(txd + 4, txd[1]);
    txd[len++] = 0x16;
    tmp = 0xffffff00 | txd[txd[1] + 4];

    // 2使用数据区的校验和对功能码到时间戳区域数据进行对称加密 
#ifndef  ECC_MODE_CHIP
    encrypt((BYTE*)(txd + 4), (unsigned int)(txd[1] - txd[2]), tt,
            (unsigned int *)&outlen, (unsigned int)tmp, (int)(txd[txd[1] + 4] % 3));
#else
    ecrSymmEnc((BYTE*)(txd + 4), (uint16_t)(txd[1] - txd[2]), tt, (uint16_t*)&outlen, tmp);
#endif

    memcpy(txd + 4, tt, txd[1] - txd[2]);
    return len;
}

DWORD SymmDecOneFrame(BYTE *Buf, WORD Len, BYTE linkaddlen, BYTE flag)
{
    DWORD off = 0;
    BYTE tt[300] = {0,};
    DWORD tmp;
    BYTE *pdata;
    BYTE cs;
	BYTE cksum = 0;
    DWORD outlen = 100;
    switch (Buf[0]) {
    case 0x68:
        if(flag == 1)
        {
            if(Len < 15) break;
            if(Buf[1] != Buf[2]) break;
            if(Buf[3] != 0x68) break;
            if((Buf[5 + linkaddlen] != 0x2d) && (Buf[5 + linkaddlen] != 0x2e)) break;
            if(Len < 97) return FRAME_LESS;
			
            while(off < Len - 70)
            {
                if((Buf[off] == 0x16) && (Buf[off + 3] == 0x16))
                    break;
                off++;
            }
            if(off > (Len - 70)) return FRAME_OK | off;

            pdata = Buf;
            Buf = pdata + off;
            if((Buf[0] != 0x16) || (Buf[3] != 0x16))
                return FRAME_OK | off;
            if(Buf[1] < Buf[2])
                return FRAME_OK | off;

            tmp = 0xffffff00 | Buf[Buf[1] + 4];

#ifndef ECC_MODE_CHIP
            encrypt((BYTE*)(Buf + 4), (unsigned int)(Buf[1] - Buf[2]), tt,
                    (unsigned int *)&outlen, (unsigned int)tmp, (int)(Buf[Buf[1] + 4] % 3));
#else
		    cksum = Buf[Buf[1] + 4];//指向校验和 
			if (!ecrSymmDec((BYTE*)(Buf + 4), (unsigned short)(Buf[1] - Buf[2]), tt,
                (uint16_t*)&outlen, cksum))
				return FRAME_ERR | off;
#endif
            memcpy(Buf + 4, tt, Buf[1] - Buf[2]);
            off = pdata[1] + 6 + Buf[1] + 6;
        }
        else//104 
        {
            if(Len < 15) break;
            if((Buf[5 + linkaddlen] != 0x2d) && (Buf[5 + linkaddlen] != 0x2e)) break;
            while(off < Len - 70)
            {
                if((Buf[off] == 0x16) && (Buf[off + 3] == 0x16))
                    break;
                off++;
            }
            if(off > (Len - 70)) return FRAME_OK | off;
            pdata = Buf;
            Buf = pdata + off;
            if((Buf[0] != 0x16) || (Buf[3] != 0x16))
                return FRAME_OK | off;
            if(Buf[1] < Buf[2])
                return FRAME_OK | off;

			//数据合法性检查结束 		
#ifndef ECC_MODE_CHIP
            tmp = 0xffffff00 | Buf[Buf[1] + 4];//指向校验和
            encrypt((BYTE*)(Buf + 4), (unsigned int)(Buf[1] - Buf[2]), tt,
                    (unsigned int *)&outlen, (unsigned int)tmp, (int)(Buf[Buf[1] + 4] % 3));
#else
            cksum = Buf[Buf[1] + 4];//指向校验和 
			if (!ecrSymmDec((BYTE*)(Buf + 4), (unsigned short)(Buf[1] - Buf[2]), tt,
                (uint16_t*)&outlen, cksum))
				return FRAME_ERR | off;
#endif
            memcpy(Buf + 4, tt, Buf[1] - Buf[2]);
            off = pdata[1] + 6 + Buf[1] + 6;
        }

        return FRAME_OK | off;
        break;

    case 0x16:
        if(Len < 4) return FRAME_LESS;
        if(Buf[3] != 0x16) break;
        //if(Buf[2]!=0x40) break;
        if(Len < Buf[1] + 6) return FRAME_LESS;

        tmp = 0xffffff00 | Buf[Buf[1] + 4];
#ifndef ECC_MODE_CHIP
        encrypt((BYTE*)(Buf + 4), (unsigned int)(Buf[1] - Buf[2]), tt,
                (unsigned int *)&outlen, (unsigned int)tmp, (int)(Buf[Buf[1] + 4] % 3));
#else
        cksum = Buf[Buf[1] + 4];//指向校验和 
		if (!ecrSymmDec((BYTE*)(Buf + 4), (unsigned short)(Buf[1] - Buf[2]), tt,
                        (uint16_t*)&outlen, cksum))
            return FRAME_ERR | off;
#endif
        memcpy(Buf + 4, tt, Buf[1] - Buf[2]);
        pdata = Buf;
        cs = ChkSum(pdata + 4, pdata[1]);
        if (cs != pdata[pdata[1] + 4])
            return FRAME_ERR | Buf[1];
        return FRAME_OK | Buf[1] + 6;
        break;

    }
    return FRAME_ERR | off;
}



/***********************************************************/
/** 
* @param   pdata   
* @param   ptxd   
* @param   addr   
* @param   off   固定为2,为104报文0x68和len自身占用的字节数目
*


************************************************************/
int encVerify(BYTE *pdata, BYTE *ptxd, int addr, int off)
{
#ifndef ECC_MODE_CHIP
    ECCrefPublicKey pucPublicKey;
    ECCSignature pucSignature;
    unsigned char pucID[16] = {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                               0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
#else
    unsigned char pucSignBuf[64];
    unsigned char puckey[64] = {0,};
    unsigned short signLen;
    unsigned char pucKeyNum;
#endif
    int mm;
    int dd;
    int tmp1, tmp2;
    struct VCalClock tm;
    BYTE *buf = pdata;
    int idLen = buf[1] - buf[2] + 4;
    BYTE tt[4] = {0x0, 0, 1, 0};
    
    F3_flag = 0;
    if (pdata[0] == 0x68)
	{
    	/** buf偏移到AH开头 */
        buf = pdata + pdata[1]/** APDU长度*/ + off;
        idLen = buf[1] - buf[2] + 4 + pdata[1] + off;
    }

	/** 子站地址解密后检查 */
    dd = buf[9];
    dd <<= 8;
    dd |= buf[8];
    dd <<= 8;;
    dd |= buf[7];
    dd <<= 8;
    dd |= buf[6];
    if(dd != addr)
        return -1;

#ifndef ECC_MODE_CHIP
    pucPublicKey.bits = 0x00000100;
    memcpy(pucPublicKey.x, kkx[(buf[4] & 3)], 32);
    memcpy(pucPublicKey.y, kky[(buf[4] & 3)], 32);
    memcpy(pucSignature.r, buf + buf[1] - buf[2] + 4, 32);
    memcpy(pucSignature.s, buf + buf[1] - buf[2] + 4 + 32, 32);
    mm = SM2_Verify(pdata,idLen,pucID,16,&pucPublicKey,&pucSignature);
    if(mm != 0) return -1;
#else
    signLen = 64;
    memcpy(pucSignBuf, buf + buf[1] - buf[2] + 4, signLen);
    mm = ecrVerifySign(pdata, idLen, pucSignBuf, signLen, 0x81 + (buf[4] & 3));
    if (mm == 0) return -1;

#endif

    mm = 0;
    if (buf[0] == 0x16) {
#if 0
        /* 时间戳有效性判断 */
        GetSysClock(&tm, CALCLOCK);
        tmp1 = (tm.dwMinute * 60) + (tm.wMSecond / 1000);
        tmp2 = buf[13];
        tmp2 <<= 8;
        tmp2 |= buf[12];
        tmp2 <<= 8;
        tmp2 |= buf[11];
        tmp2 <<= 8;
        tmp2 |= buf[10];
        mm = tmp1 - tmp2;
        if (mm < 0) mm = -mm;
        if (mm > 60) {
            mm -= 3600 * 8;
            if ((mm < -60) || (mm > 60)) {
                shellprintf("加密时间失去有效性\n\r");
                return -1;
            }
        }
#endif
        switch ((buf[4] >> 2) & 0xf) {
        case 4://update public key
            memcpy(kkx[(buf[5] & 3)], buf + 22, 32);
            memcpy(kky[(buf[5] & 3)], buf + 54, 32);
            F3_flag = (buf[5] & 3) + 1;

#ifdef  ECC_MODE_CHIP
            memcpy(puckey, kkx[(buf[5] & 3)], 32);
		    memcpy(&puckey[32], kkx[(buf[5] & 3)], 32);
		    if (!ecrPucKeySet(puckey, 64, buf[5] & 3 + 0x81))
			    return -1;
#endif
        case 3://check public key
            mm = encryptRespond(pdata, ptxd);
            break;
        case 1:

            return 0;
            break;
        default:
            return 0;
        }
    }
    return mm;
}



/**
 * 加密测试代码
 */
BYTE Enc104Data[96] = {
                        0x68, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x01,
                        0x06, 0x00, 0x01, 0x00, 0x01, 0x60, 0x00, 0x81,
                        0x16, 0x4a, 0x40, 0x16, 0xe5, 0xff, 0xfe, 0xff,
                        0xe1, 0xff, 0x51, 0x88, 0x7a, 0xaa, 0xaa, 0x15,
                        0xe2, 0x10, 0xfa, 0x00, 0xf7, 0xb7, 0xfc, 0x44,
                        0xc5, 0xd1, 0xcb, 0x5a, 0x74, 0x01, 0xd2, 0x28,
                        0x75, 0x80, 0x20, 0xbd, 0xb8, 0xe5, 0x9e, 0xbe,
                        0xd3, 0x57, 0xb4, 0xdb, 0x38, 0x9e, 0x67, 0xdc,
                        0x66, 0xde, 0x52, 0xad, 0x38, 0xdb, 0xe1, 0x83,
                        0xc5, 0x5d, 0xe7, 0x1a, 0xc2, 0x70, 0xeb, 0x85,
                        0x78, 0x8a, 0xe0, 0xc7, 0x1d, 0x7e, 0xb3, 0x45,
                        0x58, 0x5d, 0xef, 0x5b, 0xb1, 0xfc, 0xe1, 0x16
                       };


BYTE Enc101Data1[98] = {0x68 ,0x0C ,0x0C ,0x68 ,0x73 ,0xEF ,0x09 ,0x2E ,0x01 ,
						0x06 ,0x00 ,0xEF ,0x09 ,0x01 ,0x60 ,0x02 ,0xFB ,0x16 ,
						0x16 ,0x4A ,0x40 ,0x16 ,0x71 ,0xFF ,0x10 ,0xF6 ,0x75 ,
						0xFF ,0x4F ,0x8D ,0x4C ,0xA8 ,0xD2 ,0x27 ,0xEA ,0xEE ,
						0x29 ,0xB1 ,0xDF ,0x38 ,0x11 ,0x2C ,0x1A ,0x6E ,0x05 ,
						0x03 ,0x3B ,0xB6 ,0x99 ,0xD8 ,0x5F ,0x91 ,0x86 ,0xCA ,
						0xD8 ,0xD1 ,0xA3 ,0x6D ,0x2A ,0xB7 ,0x45 ,0x01 ,0x7D ,
						0x1F ,0x40 ,0xBE ,0x3E ,0xBD ,0xD7 ,0xD4 ,0xA8 ,0x49 ,
						0x74 ,0x5C ,0x66 ,0x0C ,0x96 ,0xA8 ,0x97 ,0x0A ,0x7B ,
						0xA6 ,0x54 ,0x35 ,0x71 ,0x47 ,0x63 ,0xE6 ,0xF6 ,0x1B ,
						0xF0 ,0x70 ,0xC1 ,0xE9 ,0x25 ,0x79 ,0x75 ,0x16
                        };

BYTE Enc104Data2[96] = {
                         0x68, 0x0e, 0x06, 0x00, 0x10, 0x00, 0x2d, 0x01,
                         0x06, 0x00, 0x01, 0x00, 0x01, 0x60, 0x00, 0x00,
                         0x16, 0x4a, 0x40, 0x16, 0x8e, 0xff, 0xfe, 0xff,
                         0x8a, 0xff, 0x3f, 0x88, 0x11, 0xaa, 0x8c, 0x23,
                         0xbb, 0x07, 0x7c, 0x83, 0x50, 0x5b, 0xdd, 0x67,
                         0xda, 0x4a, 0x0d, 0xd6, 0x2c, 0x4d, 0x70, 0x3e,
                         0x1c, 0xe5, 0xae, 0x8c, 0xaa, 0xae, 0xf5, 0x1a,
                         0x45, 0xb1, 0x19, 0x5b, 0xf4, 0xdf, 0x94, 0x8e,
                         0xf0, 0x0f, 0x5c, 0xab, 0xe2, 0x05, 0xf4, 0x4c,
                         0xa7, 0x4b, 0xc0, 0x35, 0x2b, 0x5d, 0x64, 0x0d,
                         0xf8, 0x69, 0x63, 0x9b, 0xa5, 0x2a, 0x9c, 0x11,
                         0x1a, 0xb5, 0x02, 0x8b, 0xdc, 0xb0, 0x8a, 0x16
                        };
BYTE publicKey[64] = {
                      0x4d, 0x2e, 0x96, 0x2e, 0xf4, 0x5a, 0x3d, 0x42,
                      0xed, 0xa4, 0x23, 0x58, 0x61, 0xf9, 0x7d, 0x22,
                      0x95, 0xc2, 0x10, 0x73, 0xf3, 0x52, 0xa9, 0xb4,
                      0xf2, 0xaf, 0x3a, 0x56, 0xf4, 0xa8, 0x29, 0x11,
                      0x20, 0x93, 0xb7, 0x64, 0xd4, 0xc1, 0xe9, 0x37,
                      0x5c, 0x2d, 0x82, 0xcf, 0x94, 0xbf, 0x59, 0x94,
                      0xf8, 0xae, 0x9f, 0x37, 0x18, 0xcd, 0xed, 0x46,
                      0x9a, 0xc5, 0x95, 0xd5, 0xcc, 0x42, 0x86, 0x32
                     };

BYTE byKeyUpdate[100] ={0,};

BYTE tmpEncData[100], tmpEncData1[100], tmpEncData2[100];//存放对称解密过后的数据

void testSetKey()
{
	if (!ecrPucKeySet(publicKey, 64, 0x81))
	{
		shellprintf("导入公钥失败\n\r");
		return;
	}
}
//104加密遥控报文测试1
DWORD testEnc(void)
{
    if (!ecrPucKeySet(publicKey, 64, 0x81))
	{
        shellprintf("导入公钥失败\n\r");
        return -1;
    }

    memcpy(tmpEncData, Enc104Data, 96);  
     
    SymmDecOneFrame(tmpEncData,96,1,2);
    return (encVerify(tmpEncData, byKeyUpdate, 1, 2));
}

DWORD testEnc1(void)
{

    memcpy(tmpEncData1, Enc101Data1, 98);  
    SymmDecOneFrame(tmpEncData1,98,2,1);
    return (encVerify(tmpEncData1, byKeyUpdate, 0x09EF, 6));
}


//104加密遥控报文测试2
DWORD testEnc2(void)
{
    if (!ecrPucKeySet(publicKey, 64, 0x81))
	{
        shellprintf("导入公钥失败\n\r");
        return -1;
    }

    memcpy(tmpEncData2, Enc104Data2, 96);

    SymmDecOneFrame(tmpEncData2,96,1,2);
    return (encVerify(tmpEncData2, byKeyUpdate, 1, 2));
}

//直接公钥导入+鉴签测试
DWORD testEnc3(void)
{
    char mm;
    static BYTE puckey[64] = {
                       0x27, 0x30, 0x8B, 0x1E, 0x5F, 0x45, 0xCE, 0xC1,
                       0x67, 0x08, 0x63, 0x43, 0xBB, 0x63, 0x33, 0x82,
                       0x11, 0x2A, 0x52, 0x8D, 0x30, 0x28, 0xE0, 0xB0,
                       0x33, 0xFD, 0x0A, 0x56, 0xF9, 0x3A, 0xDA, 0x6F,                       
                       0x51, 0x15, 0x70, 0x72, 0x3A, 0x76, 0xE1, 0x96,
                       0x0A, 0xA4, 0x94, 0x76, 0x9A, 0xB1, 0x24, 0x6B,                       
                       0xB1, 0x27, 0x64, 0xD6, 0xF3, 0xEE, 0x35, 0xF7,
                       0x5D, 0x8B, 0x78, 0x5A, 0x9A, 0xBA, 0x9F, 0x2C
                      };
    BYTE pucData[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    BYTE pucSign[64] = {
                        0xE3, 0xBA, 0xEB, 0xF5, 0xC5, 0x94, 0x6E, 0xA8,
                        0xFE, 0x9A, 0x64, 0x6E, 0x26, 0x0E, 0xF5, 0x36,
                        0x36, 0xCA, 0x0C, 0x2D, 0x8E, 0xCD, 0x3C, 0x39,
                        0x27, 0xF4, 0x5A, 0xF1, 0x5A, 0xE7, 0x53, 0xFA,
                        0x1D, 0x52, 0xD6, 0x1E, 0x05, 0xB4, 0xF9, 0x7C,
                        0x46, 0xC4, 0x45, 0x13, 0x2D, 0x53, 0x88, 0x09,
                        0x5D, 0xC5, 0xF6, 0x59, 0x59, 0x6A, 0xCE, 0x32,
                        0xDB, 0x52, 0x04, 0x84, 0x2D, 0x62, 0x3D, 0x1D
                       };

    if (!ecrPucKeySet(puckey, 64, 0x81))
	{
        shellprintf("导入公钥失败\n\r");
        return -1;
    }

    mm = ecrVerifySign(pucData, 8, pucSign, 64, 0x81);
    if (mm == 0) return -1;
    shellprintf("鉴签成功\n\r");
    
}

//公钥检查验证, 测试之前请关闭时间戳检查代码
DWORD testEnc4(void)
{
    static BYTE puckey[64] = {
                              0x1d, 0x6b, 0x22, 0xdd, 0x6b, 0x85, 0x8a, 0xa6,
                              0x09, 0x0d, 0xb4, 0x59, 0x27, 0xc8, 0xd6, 0xbf,
                              0xe7, 0x4c, 0x00, 0x85, 0x43, 0x50, 0xba, 0xb0,
                              0xd1, 0x15, 0x5d, 0xa8, 0x41, 0x29, 0x07, 0xc4,
                              0xef, 0x14, 0xb6, 0x68, 0x7d, 0xb8, 0xda, 0xf6,
                              0xcb, 0xac, 0x64, 0xaf, 0x27, 0xc9, 0x30, 0x9c,
                              0x00, 0x8c, 0x69, 0xc5, 0x1f, 0x63, 0xbd, 0x3c,
                              0xb8, 0x1e, 0x61, 0x5d, 0x2d, 0x57, 0xef, 0xdf
                             };

    static BYTE encData[84] = {
                      0x16, 0x4E, 0x40, 0x16, 0x26, 0xDA, 0x01, 0x00,
                      0xDA, 0xDA, 0x96, 0xCF, 0xE8, 0x88, 0x11, 0xF1,
                      0xB1, 0x91, 0x80, 0x4A, 0x22, 0xA8, 0xFD, 0x34,
                      0x9D, 0x31, 0x2F, 0xA7, 0xBA, 0x7F, 0xD1, 0xAF,
                      0xB1, 0xC8, 0x31, 0x8F, 0xC6, 0x8F, 0xF1, 0xAD,
                      0x32, 0x30, 0x43, 0xE8, 0x62, 0x7F, 0x1F, 0x77,
                      0x20, 0x75, 0xCE, 0xA4, 0x11, 0x4E, 0xD9, 0xC8,
                      0x36, 0x2D, 0x69, 0x10, 0x32, 0x90, 0x80, 0x38,
                      0xBA, 0xAD, 0x99, 0x5B, 0xBF, 0xBC, 0xAC, 0x28,
                      0xDE, 0x72, 0xA1, 0x72, 0xA5, 0x1B, 0x00, 0x06,
                      0x6A, 0x8A, 0x25, 0x16
                     };

    if (!ecrPucKeySet(puckey, 64, 0x82))
	{
        shellprintf("导入公钥失败\n\r");
        return -1;
    }

    SymmDecOneFrame(encData, 84, 1, 2);
    return (encVerify(encData, byKeyUpdate, 1, 2));
}
#endif

#endif
