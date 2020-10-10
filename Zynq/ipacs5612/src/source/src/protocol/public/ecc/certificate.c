

#include "certificate.h"
#include "string.h"
#include <ctype.h>
#include "file.h"
#include "fileext.h"

#ifdef  ECC_MODE_OLDCHIP
#include "sgc1126a.h"
#endif


#define CertificateFileName  "Certificate%01d.cer"
#define CertificateStartStr  "-----BEGIN CERTIFICATE-----"
#define CertificateEndStr  "-----END CERTIFICATE-----"

// BASE64解码
#define BAD 0xFF
 static const unsigned char base64val[] = {
     BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
     BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD,
     BAD,BAD,BAD,BAD, BAD,BAD,BAD,BAD, BAD,BAD,BAD, 62, BAD,BAD,BAD, 63,
      52, 53, 54, 55, 56, 57, 58, 59, 60, 61,BAD,BAD, BAD,BAD,BAD,BAD,
     BAD, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,BAD, BAD,BAD,BAD,BAD,
     BAD, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
      41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,BAD, BAD,BAD,BAD,BAD
 };
 
 BOOL myisascii(char c)
 {
		if(c>0&&c<127)
			return TRUE;
		else
			return FALSE;
 }
 
 #define DECODE64(c) (myisascii(c) ? base64val[c] : BAD)

//解析二进制 报文
//数据结构类型
#define ASN_SEQUENCE 0x30
#define ASN_SET 0x31
#define ASN_OID 0x06
#define ASN_UTCTIME 0x17
#define ASN_GENTIME 0x18
#define ASN_VERSON 0xa0
#define ASN_INT 0x02
#define ASN_NULL 0x05
#define ASN_BITSTRING 0x03
#define ASN_EXTENSIONS 0xa3

//定义OID
#define IS_OID_email 0x01
#define IS_OID_orgnization 0x02
#define IS_OID_country 0x03
#define IS_OID_province 0x04
#define IS_OID_department 0x05
#define IS_OID_city 0x06
#define IS_OID_user 0x07
#define IS_OID_authKID 0x08
#define IS_OID_subKID 0x09
#define IS_OID_subaltname 0x10
#define IS_OID_keyUsage 0x11
#define IS_OID_basicCon 0x12
#define IS_OID_extKeyUsage 0x13
#define IS_OID_CRLDP 0x14



//Global Var
tCertificateData *pCertificateData;


/******************************************************************************
 *  功能：
 *      实现BASE64解码
 *  参数：
 *      out:    输出数据指针
         in:      输入数据指针
 *  返回值：
 *      返回解码后输入数据长度
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20150706
 * *****************************************************************************/
int from64tobits(BYTE *out, BYTE *in)
{
		int nRet = 0;
		BYTE *pStart = NULL;
		BYTE *pEnd = NULL;
		BYTE *pBuf;
		BYTE *temp;
		int index = 0;
		int len = 0;
	  register unsigned char digit1, digit2, digit3, digit4;
    if(!in)
        return nRet;
    
    //去除多余的头和问说明字符
    temp = (BYTE*)strstr((char*)in,CertificateStartStr);

    if(NULL != temp)
    {//去除头部
    	pStart = (BYTE*)in;
			pStart = pStart + strlen(CertificateStartStr) + 1;//开始位置，换行位后面一个字节
    }
    temp=(BYTE*)strstr((char*)in,CertificateEndStr);

    if(NULL != temp)
    {//去除头部
    	pEnd = (BYTE*)(temp - 1);//换行符位置
    	*pEnd = '\0';
    }

    //去除各个换行符
    
    pBuf = (BYTE*)in;
    temp = pStart;
    while(*temp)
    {
        if(0x0D !=*temp &&0x0A !=*temp)
        {
            pBuf[index++] = *temp++;
        }
        else
        {
            temp++;//跳过0X0D   0X0A
        }
    }
    pBuf[index++] = '\0';
    in = pBuf;
    
    //进行BASE64解码
    if (in[0] == '+' && in[1] == ' ')
        in += 2;
    if (*in == '\r')
        return nRet;

    do 
    {
        digit1 = in[0];
        if (DECODE64(digit1) == BAD)
            return nRet;
        digit2 = in[1];
        if (DECODE64(digit2) == BAD)
            return nRet;
        digit3 = in[2];
        if (digit3 != '=' && DECODE64(digit3) == BAD)
            return nRet;
        digit4 = in[3];
        if (digit4 != '=' && DECODE64(digit4) == BAD)
            return nRet;
        in += 4;
        *out++ = (DECODE64(digit1) << 2) | (DECODE64(digit2) >> 4);
        ++len;
        if (digit3 != '=')
        {
            *out++ = ((DECODE64(digit2) << 4) & 0xf0) | (DECODE64(digit3) >> 2);
            ++len;
            if (digit4 != '=')
            {
                *out++ = ((DECODE64(digit3) << 6) & 0xc0) | DECODE64(digit4);
                ++len;
            }
        }
    }while (*in && *in != '\r' && digit4 != '=');
    *out='\0';

    
    nRet = len;
    return nRet;
}

/*解析数字证书格式*/
int unsure=0;
/******************************************************************************
 *  功能：
 *      判断是否为复合结构
 *  参数：
 *        buf:    OBJECT IDENTIFIER类型数据指针
 *        len:    OBJECT IDENTIFIER类型数据长度
 *  返回值：
 *      对应的字段号
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20150706
 * *****************************************************************************/
int myisstruct(unsigned int rv)
{
    if(rv & 0x20)
    {
        return 1;
    }
    return 0;
}
/******************************************************************************
 *  功能：
 *      获取结构长度
 *  参数：
 *        temp:    数据指针
 *        cont:     偏移量
 *  返回值：
 *      对应的数据类型长度
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20150707
 * *****************************************************************************/
unsigned long getlength(unsigned char *temp,int *cont)
{
	unsigned long len = 0,tmp = 0;
	if(temp[*cont] == 0x80)
	{
		unsure = 0xff;
		return 0;
	}
	else if(temp[*cont] < 0x80)
	{
		unsure = 1;
		return temp[*cont];
       }
	else
      {
		temp[*cont] -=0x81;
		unsure = 2+temp[*cont];
		if(temp[*cont]>0)
		{
			tmp=temp[*cont];
			for(;tmp>0;tmp--)
			{
			 	(*cont)++;
			  	len = len + temp[*cont];
				len = len*256;
			}
		}
		(*cont)++;
		len = len + temp[*cont];
	}
	return len;
}
/******************************************************************************
 *  功能：
 *      获取结构数据
 *  参数：
 *        buf:      读取到的数据存储内存
 *        temp:    数据指针
 *        templen:     真实读取到的数据长度
 *        cont:         数据偏移量
 *  返回值：
 *      对应的数据长度
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20150707
 * *****************************************************************************/
unsigned int myreaddata(unsigned char *buf,unsigned char *temp,unsigned int templen,int *cont)
{
    unsigned  int i = 1,rv = 0;
    if((unsure == 0xff) && templen == 1)
    {
        while(1)
        {
            templen++;
            (*cont)++;
            buf[rv] = temp[*cont];
            rv++;
            if(temp[*cont])
                continue;
            if(temp[*cont] == i)
            {
                templen -= 2;
                break;
            }
            i = temp[*cont];
        }
        return templen;
    }
    while(rv<templen)
    {
        (*cont)++;
        buf[rv]= temp[*cont];
        rv++;
    }
		return templen;
}
int setcount(unsigned char rv,unsigned char *expect,int *expectnumber,int *steps,int isoid,int count,int *bak)
{
    if(expect[*expectnumber] == rv)
    {
        (*expectnumber)++;
        count++;
        return count;
    }
    if((rv == 0x17)&&((*steps)==0))
    {
        (*steps)++;
        (*expectnumber)+=2;
        return 11;
    }
    if((rv == 0x06) && (isoid == 0) && ((*steps) == 1))
    {
        (*steps)++;
        (*expectnumber)+=2;
        return 20;
    }
    if((rv == 0x06) && (isoid == 0) && ((*steps) == 2))
    {
        if(20 == count)
        {
            return 20;//公钥前有两个算法，不知道为什么
        }
        (*steps)++;
        return 31;
    }

    if(expect[*expectnumber] == 0x7f)
    {
        switch(isoid)
        {
            case IS_OID_email:
            {
                count = (*steps)?13:4;
                break;
            }
            case IS_OID_country:
            {
                count = (*steps)?14:5;
                break;
            }
            case IS_OID_province:
            {
                count = (*steps)?15:6;
                break;
            }
            case IS_OID_orgnization:
            {
                count = (*steps)?16:7;
                break;
            }
            case IS_OID_city:
            {
                count = (*steps)?17:8;
                break;
            }
            case IS_OID_user:
            {
                count = (*steps)?18:9;
                break;
            }
            case IS_OID_department:
            {
                count = (*steps)?19:10;
                break;

            }
            case IS_OID_authKID:
            {
                count = 24;
                break;
            }
            case IS_OID_subKID:
            {
                count = 25;
                break;
            }
            case IS_OID_subaltname:
            {
                count = 27;
                break;
            }
            case IS_OID_keyUsage:
            {
                count = 26;
                break;
            }
            case IS_OID_basicCon:
            {
                count = 28;
                break;
            }
            case IS_OID_extKeyUsage:
            {
                count = 29;
                break;
            }
            case IS_OID_CRLDP:
            {
                count = 30;
                break;
            }
            case 0:
                return (*bak);
            default:
                break;
        }
        (*bak) = count;
        return 0;
    }
    return 0;
}
/******************************************************************************
 *  功能：
 *      根据OBJECT IDENTIFIER类型获取对应的字段号
 *  参数：
 *        buf:    OBJECT IDENTIFIER类型数据指针
 *        len:    OBJECT IDENTIFIER类型数据长度
 *  返回值：
 *      对应的字段号
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20150706
 * *****************************************************************************/
int mycomp(unsigned char *buf,int len)
{
	unsigned char OID_email[9] ={0x2a,0x86,0x48,0x86,0xf7,0x0d,0x01,0x09,0x01};
	//ssz add
	//unsigned char OID_paraPQG[7] ={0x2a,0x86,0x48,0xCE,0x3D,0x02,0x01};//应该是p、q、g
	//unsigned char OID_PublicKey[8] ={0x2a,0x81,0x1C,0xCF,0x55,0x01,0x82,0x2D};//是公钥
	//
	unsigned char OID_orgnization[3] ={0x55,0x04,0x0a};
	unsigned char OID_country[3] ={0x55,0x04,0x06};
	unsigned char OID_province[3] ={0x55,0x04,0x08};
	unsigned char OID_department[3] ={0x55,0x04,0x0b};
	unsigned char OID_city[3] ={0x55,0x04,0x07};
	unsigned char OID_user[3] ={0x55,0x04,0x03};
	unsigned char OID_authKID[3] ={0x55,0x1d,0x23};
	unsigned char OID_subKID[3] ={0x55,0x1d,0x0e};
	unsigned char OID_subaltname[3] ={0x55,0x1d,0x11};
	unsigned char OID_keyUsage[3] ={0x55,0x1d,0x0f};
	unsigned char OID_basicCon[3] ={0x55,0x1d,0x13};
	unsigned char OID_extKeyUsage[3] ={0x55,0x1d,0x25};
	unsigned char OID_CRLDP[3] ={0x55,0x1d,0x1f};


	if(!memcmp(buf,OID_extKeyUsage,len))
	{
			return IS_OID_extKeyUsage;
	}
	if(!memcmp(buf,OID_CRLDP,len))
	{
			return IS_OID_CRLDP;
	}
	if(!memcmp(buf,OID_basicCon,len))
	{
			return IS_OID_basicCon;
	}
	if(!memcmp(buf,OID_subaltname,len))
	{
			return IS_OID_subaltname;
	}
	if(!memcmp(buf,OID_authKID,len))
	{
			return IS_OID_authKID;
	}
	if(!memcmp(buf,OID_keyUsage,len))
	{
			return IS_OID_keyUsage;
	}
	if(!memcmp(buf,OID_subKID,len))
	{
			return IS_OID_subKID;
	}
	if(!memcmp(buf,OID_email,len))
	{
			return IS_OID_email;
	}
	if(!memcmp(buf,OID_city,len))
	{
			return IS_OID_city;
	}
	if(!memcmp(buf,OID_orgnization,len))
	{
			return IS_OID_orgnization;
	}
	if(!memcmp(buf,OID_country,len))
	{
			return IS_OID_country;
	}
	if(!memcmp(buf,OID_province,len))
	{
			return IS_OID_province;
	}
	if(!memcmp(buf,OID_department,len))
	{
			return IS_OID_department;
	}
	if(!memcmp(buf,OID_user,len))
	{
			return IS_OID_user;
	}
	return 0;
}
/******************************************************************************
 *  功能：
 *      根据字段号写对应的数据
 *  参数：
 *        buf:           数据指针
 *        len:           数据长度
 *        count:        字段号
 *  返回值：
 *      对应的字段号
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20150706
 * *****************************************************************************/
int myfill(BYTE *buf,int templen,int count,int No)
{
switch(count)
	{
	case 1://version;
	case 2://serialNumber;
	case 3://algorithm
	case 4://issuer.email
	case 5://country
	case 6://province
	case 7://issuer.orgnization
	case 8://issuer.city
	case 9://issuer.user
	case 10://issuer.department
	    break;
	case 11:
	{//notBefore
		if(pCertificateData)
		{
				if(13 == templen)//150608022222Z
				{
						char data[3] = {0};
						int index = 0;
						//年月日
						data[0] = buf[index++];
						data[1] = buf[index++];
						data[2] = '\0';
						pCertificateData->tmBegin[No].wYear = atoi(data) + 2000;//只有最后两位
						data[0] = buf[index++];
						data[1] = buf[index++];
						data[2] = '\0';
						pCertificateData->tmBegin[No].byMonth = atoi(data);
						data[0] = buf[index++];
						data[1] = buf[index++];
						data[2] = '\0';
						pCertificateData->tmBegin[No].byDay = atoi(data);
						 //时分秒
						data[0] = buf[index++];
						data[1] = buf[index++];
						data[2] = '\0';
						pCertificateData->tmBegin[No].byHour = atoi(data);
						data[0] = buf[index++];
						data[1] = buf[index++];
						data[2] = '\0';
						pCertificateData->tmBegin[No].byMinute = atoi(data);
						data[0] = buf[index++];
						data[1] = buf[index++];
						data[2] = '\0';
						pCertificateData->tmBegin[No].bySecond = atoi(data);
						
						pCertificateData->tmBegin[No].wMSecond = 0;

                                        ToCalClock(&pCertificateData->tmBegin[No],&pCertificateData->AbsTBegin[No]);
                                        pCertificateData->AbsTBegin[No].dwMinute += (60*8);//8小时
                }
		}
		break;
	}
	case 12:
	{//notAfter
		if(pCertificateData)
		{
			if(13 == templen)//150608022222Z
			{
					char data[3] = {0};
					int index = 0;
					//年月日
					data[0] = buf[index++];
					data[1] = buf[index++];
					data[2] = '\0';
					pCertificateData->tmEnd[No].wYear = atoi(data) + 2000;//只有最后两位
					data[0] = buf[index++];
					data[1] = buf[index++];
					data[2] = '\0';
					pCertificateData->tmEnd[No].byMonth = atoi(data);
					data[0] = buf[index++];
					data[1] = buf[index++];
					data[2] = '\0';
					pCertificateData->tmEnd[No].byDay = atoi(data);
					//时分秒
					data[0] = buf[index++];
					data[1] = buf[index++];
					data[2] = '\0';
					pCertificateData->tmEnd[No].byHour = atoi(data);
					data[0] = buf[index++];
					data[1] = buf[index++];
					data[2] = '\0';
					pCertificateData->tmEnd[No].byMinute = atoi(data);
					data[0] = buf[index++];
					data[1] = buf[index++];
					data[2] = '\0';
					pCertificateData->tmEnd[No].bySecond = atoi(data);
					
					pCertificateData->tmEnd[No].wMSecond = 0;

                                ToCalClock(&pCertificateData->tmEnd[No],&pCertificateData->AbsTEnd[No]);
                                pCertificateData->AbsTEnd[No].dwMinute += (60*8);//8小时
			}
		}
		break;
	}
	case 13:
		//subject.email
		break;
	case 14:
		//subject.country
		break;
	case 15:
		//subject.province
		break;
	case 16:
		//subject.orgnization
		break;
	case 17:
		//subject.city
		break;
	case 18:
		//subject.user
		break;
	case 19:
		//subject.department
		break;
	case 20:
		//algor
		break;
	case 21:
	{//public_key
		if(pCertificateData)
		{
				if(66 == templen)
				{
						BYTE *pPubKey = buf + 2;//跳过前面两个，取后面的64字节
#ifdef  ECC_MODE_OLDCHIP
						if (FALSE == ecrPucKeySet(pPubKey, 64, 0x81+ No))
							return 0;
#endif						
						memcpy(pCertificateData->PublicKey[No].x,pPubKey,32);
                                        pPubKey += 32;
						memcpy(pCertificateData->PublicKey[No].y,pPubKey,32);
						pCertificateData->bCertificateFlag[No] = TRUE;
				}
		}
		break;
	}
	/*extensions*/
	case 22:
		//extensions.issuerUID
		break;
	case 23:
		//extensions.subjectUID
		break;
	case 24:
		//extensions.authKID
		break;
	case 25:
		//extensions.subKID
		break;
	case 26:
		//extensions.keyUSG
		break;
	case 27:
		//extensions.subAltName
		break;
	case 28:
		//extensions.basicConstraints
		break;
	case 29:
		//extensions.extKUSG
		break;
	case 30:
		//extensions.CRLDP
		break;
	case 31:
		//extensions.alg
		break;
	case 32:
		//extensions.extk
		break;
	default:
		return 0;
	}
return 1;
}
/******************************************************************************
 *  功能：
 *      解析二进制数字证书
 *  参数：
 *      pdata:              输出数据指针
         datalen:           输入数据指针
         pOneKeyData:  单个KEY的数据存储指针
 *  返回值：
 *      返回解码后输入数据长度
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20150706
 * *****************************************************************************/
int parseData(BYTE *pdata,int datalen,int No)
{
    unsigned char aimtag[50]={0x02,0x02,0x06,0x7f,0x17,0x17,0x7f,0x06,0x03,0x7f,0x06,0x03};
    int i=0,templength;
    int steps=0,restore = 0;
    unsigned char *buf,*temp;
    //unsigned long len[512];

    int expectnumber = 0;
    int isoid,count=0;
    int bak =0;
		int cont=0;
		BYTE Databuf[2048];// 2K
		
		
    temp = pdata;
    buf = Databuf;

    while(cont < datalen)
    {
        if(myisstruct(temp[cont]))
        {
            cont++;
            //len[i] = getlength(temp,&cont);
						getlength(temp,&cont);
            i++;
        }
				else
        {
            isoid = (temp[cont] == 0x06)?1:0;
            restore = temp[cont];
            cont++;
            templength = getlength(temp,&cont);
            templength = myreaddata(buf,temp,templength,&cont);
            if(isoid)
            {
                isoid = mycomp(buf,templength);
            }
            if(restore != 0x05)
            {
                count = setcount(restore,aimtag,&expectnumber,&steps,isoid,count,&bak);
                isoid = 0;
                if(count)
                {
                    int ret = myfill(buf, templength, count,No);
                    if(!ret)
                    {
                        //写入失败
                    }
                }
                memset(buf,0,templength);
            }
        }
        cont++;

        if(21 == count)
        break;//读取到公钥之后就不读了
    }
		return 1;
}
/******************************************************************************
 *  功能：
 *      判断文件是否存在
 *  参数：
 *      filename:文件名称(包括路径)
 *  返回值：
 *      TRUE:文件存在
 *      FALSE:文件不存在
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20150706
 * *****************************************************************************/
BOOL FileIsExist1(char *filename)
{
	FILE *fp;
	if ((fp = fopen(filename, "r")) == NULL)
		return FALSE;
	else
	{
		fclose(fp);
		return TRUE;
	}
}
/******************************************************************************
 *  功能：
 *      数字证书总接口
 *  参数：
 *      无
 *  返回值：
 *      TRUE:   有数字证书，并解析成功。
 *      FALSE:  没有数字证书，或者解析失败。
 *  注意：
 *      空
 *  编辑人：
 *      桑士振
 *  日期：
 *      20160908
 * *****************************************************************************/
BOOL certificate_Init(void)
{
    static BOOL bRet = FALSE;
    int i;
    BOOL bReadRet[4];

    char fileName[60];
    int len = 0;//BASE64格式文件长度
    int lenBin = 0;//二进制文件长度
 #if 0
    unsigned int FileOffset = 0;
 #endif
    BYTE *pFileBuf_Base64;
    BYTE *pFileBuf_bin;
    //extern FIL * gpFile;
    //BOOL bFileExist = FALSE;
    static BOOL bInitFlag = FALSE;
	  FILE *fp;
    char path[60];
    

	
    if(bInitFlag)
    {
        return bRet;
    }
#ifdef  ECC_MODE_OLDCHIP
	
	if (!JiamiInit(0))
	{
		shellprintf("Init encrypt chip failed...\n\r");
		return FALSE;
	}
#endif
		
		bRet = FALSE;
    //内存分配
    len = sizeof(tCertificateData);
    pCertificateData = (tCertificateData *)calloc(1,len);
    pFileBuf_Base64 = (BYTE *)calloc(1,4096);//4//4K
    pFileBuf_bin = (BYTE *)calloc(1,2048);//4//2K
    if(!pCertificateData ||!pFileBuf_Base64||!pFileBuf_bin)
    {
        return bRet;
    }
    pCertificateData->bCertificateFlag[0] = FALSE;
    pCertificateData->bCertificateFlag[1] = FALSE;
    pCertificateData->bCertificateFlag[2] = FALSE;
    pCertificateData->bCertificateFlag[3] = FALSE;

    bReadRet[0] = FALSE;
    bReadRet[1] = FALSE;
    bReadRet[2] = FALSE;
    bReadRet[3] = FALSE;

    for(i=0; i<4; i++)
    {
        //bFileExist = FALSE;
        //文件操作
        sprintf(fileName, CertificateFileName, i); 
	 #if 0
        //if(FileIsExist(fileName))
        {
            if(f_open(gpFile, fileName, FA_OPEN_EXISTING|FA_READ)==FR_OK)
            {
                len = f_size(gpFile);
                if(len>66)//至少大于公钥长度
                {
                    f_read(gpFile, pFileBuf_Base64, len, &FileOffset);
			bFileExist = TRUE;
                    bReadRet[i] = TRUE;
                }
                f_close(gpFile);
            }
        }
				if(!bFileExist)
					continue;
	 #endif

        sprintf(fileName, CertificateFileName, i); 
        if(ERROR == FileIsExist(fileName))
            continue;
        GetMyPath(path, fileName);//other file
        fp = fopen(path, "r+b");
        fseek(fp,0,SEEK_SET);
        len = fread(pFileBuf_Base64,1,4096,fp);
        fclose(fp);
        
        if(len<1)//0或-1
            continue;
        //
        lenBin = from64tobits(pFileBuf_bin,pFileBuf_Base64);
        if(0 == lenBin)
            continue;
        //
        parseData(pFileBuf_bin,lenBin,i);
    }

		bInitFlag = TRUE;
    //free
    if(pFileBuf_Base64)
        free(pFileBuf_Base64);
    if(pFileBuf_bin)
        free(pFileBuf_bin);

    if(bReadRet[0]&&bReadRet[1]&&bReadRet[2]&&bReadRet[3])
        bRet = TRUE;
    return bRet;
}
