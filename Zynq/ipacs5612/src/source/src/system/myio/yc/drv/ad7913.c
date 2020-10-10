/*------------------------------------------------------------------------
 Module:		aidc.c
 Author:		helen
 Project:		
 Creation Date: 
 Description:	
------------------------------------------------------------------------*/
#include "syscfg.h"

#ifdef INCLUDE_AD7913

#include "myio.h"
#include "sys.h"
#include "bsp.h"
#include "ad7913.h"                             

/*60v->0.4v, 0.5v->5320000*/
#define AIDC_AD7913_60V    4256000
#define AIDC_AD7913_ZEOR   0x50000
#define AIDC_AD7913_VOL    0
#define AIDC_AD7913_TEMP   1
#define AIDC_AD7913_GAIN   2

#define AI_DC_NUM   2
#define AI_AD7913_VWV_LEN  (AI_DC_NUM*3)

int AiDc_Val[AI_DC_NUM];
int YcDc_Val[AI_DC_NUM+1];

BOOL g_ad7913_err = FALSE;

#ifdef INCLUDE_AD7913_TEMP
int AiDc_TmpOS;
int AiDc_Select;
int AiDc_Temp;
#endif

#if (TYPE_CPU == CPU_ZYNQ)
#define ADSPI   1
#endif

VYcGain AiDc_gain[AI_DC_NUM];

int ad7913Burst(int SPIx, BYTE phy_no, BYTE addr, int len)
{
     int ret=-1;
     int i,val=0;
     BYTE send[AI_AD7913_VWV_LEN+1] = {0};
	 BYTE recv[AI_AD7913_VWV_LEN+1] = {0};
	 

	 if (len > AI_AD7913_VWV_LEN) return -1;
	 
	 send[0] = (addr << 3) | AD7913_READ;

#if 1
	 ret = SPITrans_BAddr(SPIx, 0, send, recv, len+1);
#else
	 ret = SPIRead_BAddr(send[0], recv, len);
#endif

	 if (ret < 0) return -1;

	 for (i=0; i<AI_DC_NUM; i++)
	 {
	     val = (recv[i*3+1] << 16)|(recv[i*3+2] << 8)|recv[i*3+3];

		 if (recv[i*3+1] & 0x80)
		 {
		    val &= ~0x800000;
			val -=  0x7FFFFF;
		 }
#ifdef INCLUDE_AD7913_TEMP
		 if ((AiDc_Select == AIDC_AD7913_TEMP)&&(i == 1))
		 	AiDc_Temp = val;
		 else
#endif
		    AiDc_Val[i] = val;
	 }

	 return 0;
}

int ad7913Read(int SPIx, BYTE phy_no, BYTE addr, int len)
{
     int ret=-1;
     int val=0;
     BYTE send[4] = {0};
	 BYTE recv[4] = {0};
	 

	 if (len > 3) return -1;
	 
	 send[0] = (addr << 3) | AD7913_READ;

#if 1
	 ret = SPITrans_BAddr(SPIx, 0, send, recv, len+1);
#else
	 ret = SPIRead_BAddr(send[0], recv, len);
#endif

	 if (ret < 0) return -1;

	 if (len == 1)
	 	val = recv[1];

	 return val;
}

int ad7913Write(int SPIx, BYTE phy_no, BYTE addr, BYTE value)
{
     BYTE send[2] = {0};
	 BYTE recv[2] = {0};

	 send[0] = (addr << 3) | AD7913_WRITE;
	 send[1] = value;

#if 1
	 SPITrans_BAddr(SPIx, 0, send, recv, 2);
#else
	 SPISend_BAddr(send, 2);
#endif

	 return OK;
}


//24位取16位
long ad7913Get(int i)
{
    int value = 0;
	long long longs;

	if (g_ad7913_err) return 0;
	
    if (i == 0)
    {
       ad7913Burst(ADSPI, 0, AD7913_V1WV, AI_AD7913_VWV_LEN);
#ifdef INCLUDE_AD7913_TEMP
  	   if (AiDc_Select == AIDC_AD7913_VOL)
	   {
		   ad7913Write(ADSPI, 0, AD7913_CONFIG, 0x38);
		   value = ad7913Read(ADSPI, 0, AD7913_CONFIG, 1);
		   AiDc_Select = AIDC_AD7913_TEMP;
	   }
	   else if (AiDc_Select == AIDC_AD7913_TEMP)
	   {
	       ad7913Write(ADSPI, 0, AD7913_CONFIG, 0x30);
		   AiDc_Select = AIDC_AD7913_VOL;
	   }
#endif
    }

    if (i < 2)
  	{
  	    longs = (long long)(AiDc_Val[i] - AiDc_gain[i].b) * 6000;
  	    value =  longs / AiDc_gain[i].a;
    }
#ifdef INCLUDE_AD7913_TEMP
	else if (i == 2)
	{
		  longs = ((long long)(AiDc_Temp + AiDc_TmpOS))*872;
		  value = longs/100000 - 30647; 
	}
#endif
    YcDc_Val[i] = value;

	return (long)value;
	
}

void ad7913GainInit(void)
{
    int i, warn;
	DWORD addr;
	VYcGain gain_tmp;

	warn = 0;
	for (i=0; i<AI_DC_NUM; i++)
	{
        addr = NVRAM_AD_AIDC_GAIN+i*sizeof(VYcGain);

		extNvRamGet(addr, (BYTE *)&gain_tmp, sizeof(VYcGain));
		if (gain_tmp.status != 0xAA)
		{
			gain_tmp.status = 0xAA;
			gain_tmp.type = YC_TYPE_Dc;
			gain_tmp.a = AIDC_AD7913_60V;
			gain_tmp.b = AIDC_AD7913_ZEOR;	
			gain_tmp.ycNo =i;
			gain_tmp.fd = 0;
		}

		AiDc_gain[i] = gain_tmp;
	}	
	
	if (warn)
	{
	    WriteWarnEvent(NULL, SYS_ERR_GAIN, 0, "AIF增益系数错误");		
		myprintf(SYS_ID, LOG_ATTR_ERROR, "AIF Gain error!");
	}
}

#ifdef YC_GAIN_NEW
int ad7913GainValGet(VYcVal* pData)
{
    VYcVal* pbuf = pData;
	int i;

	for (i=0; i<AI_DC_NUM; i++)
	{
	    pbuf->ycNo = i;
		pbuf->type = YC_TYPE_Dc;
		pbuf->value = YcDc_Val[i];
		pbuf++;
	}
	return AI_DC_NUM;
}

int ad7913GainValSet(int num, VYcVal* pData)
{
    int i;
	DWORD addr;
    VYcVal *pbuf=pData;
	long long longs;
	

	
	#ifdef INCLUDE_AD7913_TEMP
	ad7913Write(ADSPI, 0, AD7913_CONFIG, 0x30);
	AiDc_Select = AIDC_AD7913_GAIN;
	#endif

	thSleep(20);


	ad7913Burst(ADSPI, 0, AD7913_V1WV, AI_AD7913_VWV_LEN);


	for (i=0; i<num; i++)
	{
	    if (pbuf->type != YC_TYPE_Dc)
			continue;

		if (pbuf->value == 0)
			AiDc_gain[i].b = AiDc_Val[i];
    	else
		{
		
  	       longs = (long long)(AiDc_Val[i] - AiDc_gain[i].b) * 6000;
  	       AiDc_gain[i].a =  longs / pbuf->value;
			
		   AiDc_gain[i].status = 0xAA;
			   
		}

		addr = NVRAM_AD_AIDC_GAIN+i*sizeof(VYcGain);

		extNvRamSet(addr, (BYTE *)(AiDc_gain+i), sizeof(VYcGain));  
		pbuf++;
		
	}

	#ifdef INCLUDE_AD7913_TEMP
	AiDc_Select = AIDC_AD7913_VOL;
	#endif

	return OK;

}

int ad7913GainGet(VYcGain *pdata)
{
    VYcGain *pbuf = pdata;
	int i;

	for (i=0; i<AI_DC_NUM; i++)
	{
	    pbuf->ycNo = i;
		pbuf->a = AiDc_gain[i].a;
		pbuf->b = AiDc_gain[i].b;
		pbuf->fd = AiDc_gain[i].fd;
		pbuf->type = YC_TYPE_Dc;
		pbuf->status = 2;
		pbuf++;
	}

	return AI_DC_NUM;
}

int ad7913GainSet(VYcGain *pdata)
{
    int i;
	DWORD addr;
	for (i=0; i<AI_DC_NUM; i++,pdata++)
	{
	    if (pdata->a == 0)
			continue;
	    AiDc_gain[i].a = pdata->a;
		AiDc_gain[i].b = pdata->b;

		addr = NVRAM_AD_AIDC_GAIN+i*sizeof(VYcGain);

		extNvRamSet(addr, (BYTE *)(AiDc_gain+i), sizeof(VYcGain));  
	}

	return OK;
}

#else
int ad7913GainSet(int flag)
{
    int i;
    DWORD addr;

	#ifdef INCLUDE_AD7913_TEMP
	ad7913Write(ADSPI, 0, AD7913_CONFIG, 0x30);
	AiDc_Select = AIDC_AD7913_GAIN;
	#endif

	if (flag == 0)
    {
        shellprintf("AI->DC gain zero set...");

		ad7913Burst(ADSPI, 0, AD7913_V1WV, AI_AD7913_VWV_LEN);

		AiDc_gain[0].b = AiDc_Val[0];

		AiDc_gain[1].b = AiDc_Val[1];

		shellprintf("AI->DC gain zero done");
	}
	else
	{
		ad7913Burst(ADSPI, 0, AD7913_V1WV, AI_AD7913_VWV_LEN);

	    for (i=0; i<AI_DC_NUM; i++)
		{
			AiDc_gain[i].a = (AiDc_Val[i] - AiDc_gain[i].b)/10;
			
			AiDc_gain[i].status |= YC_GAIN_BIT_VALID;
			
		    addr = NVRAM_AD_AIDC_GAIN+sizeof(VAiDcGain);

			extNvRamSet(addr, (BYTE *)(AiDc_gain), sizeof(VAiDcGain));
	    }

		shellprintf("done\n");
	}
	#ifdef INCLUDE_AD7913_TEMP
	AiDc_Select = AIDC_AD7913_VOL;
	#endif


	return OK;
}
#endif

DWORD ad7913Gain(int i, long *b)
{
    return 0;
}

void ad7913Init(void)
{
	 int val = -1;
	 DWORD count = 0;
	 char temp;

	 ad7913GainInit();

     SPIInit(ADSPI, 1200000, 8);

	 thSleep(25);

	 do
	 {
	      val = ad7913Read(ADSPI, 0, AD7913_STAUTS0, 1);
		  count++;
		  if (count > 2000)
		  {
		      g_ad7913_err = TRUE;
			  WriteWarnEvent(NULL, SYS_ERR_GAIN, 0, "AD7913 read error!");		
		      myprintf(SYS_ID, LOG_ATTR_ERROR, "AD7913 read error!");
			  return;
		  }
	 }while (val & 0x01);

	 thSleep(25);

	 ad7913Write(ADSPI, 0, AD7913_LOCK, 0x9C);          //if the registers are protected first unlock the registers by writing lock = 0x9C 
     ad7913Write(ADSPI, 0, AD7913_CONFIG, 0x40);        //software reset  
     thSleep(25);

	 val = -1;

     count = 0;
	 do
	 {
	      val = ad7913Read(ADSPI, 0, AD7913_STAUTS0, 1);

		  count++;
		  if (count > 2000)
		  {
		      g_ad7913_err = TRUE;
			  WriteWarnEvent(NULL, SYS_ERR_GAIN, 0, "AD7913 read error!");		
		      myprintf(SYS_ID, LOG_ATTR_ERROR, "AD7913 read error!");
			  return;
		  }
	 }while (val & 0x01);

	 ad7913Write(ADSPI, 0, AD7913_CONFIG, 0x30);          //select the ADC output frequency 1 kHz, 1 ms period  

	 val = ad7913Read(ADSPI, 0, AD7913_CONFIG, 1);

#ifdef INCLUDE_AD7913_TEMP
	 temp = ad7913Read(ADSPI, 0, AD7913_TEMPOS, 1);
     AiDc_TmpOS = temp<<11;
	 AiDc_Select = AIDC_AD7913_VOL;
#endif

     ad7913Write(ADSPI, 0, AD7913_EMI_CTRL, 0xFF);
    // ad7913Write(SPI3, 0, AD7913_LOCK, 0xCA);            //protect the user accessible and internal configuration registers

}
#endif
