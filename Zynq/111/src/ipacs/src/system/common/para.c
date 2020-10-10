#include "syscfg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys.h"
#include "para.h"

static BYTE ParaFileTmpBuf[MAXFILELEN];
struct VFileHead *g_pParafileTmp;

WORD crccode[0x100]={
 0X0   ,  0Xc0c1,  0Xc181,  0X140,   0Xc301,  0X3c0,   0X280,   0Xc241,
 0Xc601,  0X6c0,   0X780,   0Xc741,  0X500,   0Xc5c1,  0Xc481,  0X440,
 0Xcc01,  0Xcc0,   0Xd80,   0Xcd41,  0Xf00,   0Xcfc1,  0Xce81,  0Xe40,
 0Xa00 ,  0Xcac1,  0Xcb81,  0Xb40 ,  0Xc901,  0X9c0,   0X880 ,  0Xc841,
 0Xd801,  0X18c0,  0X1980,  0Xd941,  0X1b00,  0Xdbc1,  0Xda81,  0X1a40,
 0X1e00,  0Xdec1,  0Xdf81,  0X1f40,  0Xdd01,  0X1dc0,  0X1c80,  0Xdc41,
 0X1400,  0Xd4c1,  0Xd581,  0X1540,  0Xd701,  0X17c0,  0X1680,  0Xd641,
 0Xd201,  0X12c0,  0X1380,  0Xd341,  0X1100,  0Xd1c1,  0Xd081,  0X1040,
 0Xf001,  0X30c0,  0X3180,  0Xf141,  0X3300,  0Xf3c1,  0Xf281,  0X3240,
 0X3600,  0Xf6c1,  0Xf781,  0X3740,  0Xf501,  0X35c0,  0X3480,  0Xf441,
 0X3c00,  0Xfcc1,  0Xfd81,  0X3d40,  0Xff01,  0X3fc0,  0X3e80,  0Xfe41,
 0Xfa01,  0X3ac0,  0X3b80,  0Xfb41,  0X3900,  0Xf9c1,  0Xf881,  0X3840,
 0X2800,  0Xe8c1,  0Xe981,  0X2940,  0Xeb01,  0X2bc0,  0X2a80,  0Xea41,
 0Xee01,  0X2ec0,  0X2f80,  0Xef41,  0X2d00,  0Xedc1,  0Xec81,  0X2c40,
 0Xe401,  0X24c0,  0X2580,  0Xe541,  0X2700,  0Xe7c1,  0Xe681,  0X2640,
 0X2200,  0Xe2c1,  0Xe381,  0X2340,  0Xe101,  0X21c0,  0X2080,  0Xe041,
 0Xa001,  0X60c0,  0X6180,  0Xa141,  0X6300,  0Xa3c1,  0Xa281,  0X6240,
 0X6600,  0Xa6c1,  0Xa781,  0X6740,  0Xa501,  0X65c0,  0X6480,  0Xa441,
 0X6c00,  0Xacc1,  0Xad81,  0X6d40,  0Xaf01,  0X6fc0,  0X6e80,  0Xae41,
 0Xaa01,  0X6ac0,  0X6b80,  0Xab41,  0X6900,  0Xa9c1,  0Xa881,  0X6840,
 0X7800,  0Xb8c1,  0Xb981,  0X7940,  0Xbb01,  0X7bc0,  0X7a80,  0Xba41,
 0Xbe01,  0X7ec0,  0X7f80,  0Xbf41,  0X7d00,  0Xbdc1,  0Xbc81,  0X7c40,
 0Xb401,  0X74c0,  0X7580,  0Xb541,  0X7700,  0Xb7c1,  0Xb681,  0X7640,
 0X7200,  0Xb2c1,  0Xb381,  0X7340,  0Xb101,  0X71c0,  0X7080,  0Xb041,
 0X5000,  0X90c1,  0X9181,  0X5140,  0X9301,  0X53c0,  0X5280,  0X9241,
 0X9601,  0X56c0,  0X5780,  0X9741,  0X5500,  0X95c1,  0X9481,  0X5440,
 0X9c01,  0X5cc0,  0X5d80,  0X9d41,  0X5f00,  0X9fc1,  0X9e81,  0X5e40,
 0X5a00,  0X9ac1,  0X9b81,  0X5b40,  0X9901,  0X59c0,  0X5880,  0X9841,
 0X8801,  0X48c0,  0X4980,  0X8941,  0X4b00,  0X8bc1,  0X8a81,  0X4a40,
 0X4e00,  0X8ec1,  0X8f81,  0X4f40,  0X8d01,  0X4dc0,  0X4c80,  0X8c41,
 0X4400,  0X84c1,  0X8581,  0X4540,  0X8701,  0X47c0,  0X4680,  0X8641,
 0X8201,  0X42c0,  0X4380,  0X8341,  0X4100,  0X81c1,  0X8081,  0X4040,
};

WORD GetParaCrc16(const BYTE *p, int l)
{
    int i;
	WORD index, crc=0xFFFF;

	if (l < 0) return 0;

	for(i=0; i<l; i++)
	{
		index = ((crc^p[i])&0x00FF);
		crc = ((crc>>8)&0x00FF) ^ crccode[index];
	}

	return(crc);
}  

int SysParaInit(void)
{
	g_pParafileTmp = (struct VFileHead *)ParaFileTmpBuf;
	return OK;
}


int WriteParaFile(const char *filename, struct VFileHead *buf)
{
	return WriteParaBmFile(filename,(BYTE *)buf, buf->nLength);
}


void FillPQConfig(struct VPYcCfg *pDstCfg, VYcCfg *pSrcCfg, int index, int pqnum)
{
    int k;
    for (k=0; k<6; k++)
    {
        if (pSrcCfg[k].type > 0)
        {
           memcpy(pDstCfg, pSrcCfg+k, sizeof(VYcCfg));
		   pDstCfg++;
        }
    }
	pDstCfg->type = YC_TYPE_P;
	pDstCfg->arg1 = index;
	pDstCfg->arg2 = pqnum;
	pDstCfg->toZero = 0;
	pDstCfg++;
	index += pqnum;
	pDstCfg->type = YC_TYPE_Q;
	pDstCfg->arg1 = index;
	pDstCfg->arg2 = pqnum;
	pDstCfg->toZero = 0;
	pDstCfg++;
	index += pqnum;
	pDstCfg->type = YC_TYPE_S;
	pDstCfg->arg1 = index;
	pDstCfg->arg2 = index+1;
	pDstCfg->toZero = 0;
	pDstCfg++;
	pDstCfg->type = YC_TYPE_Cos;
	pDstCfg->arg1 = index;
	pDstCfg->arg2 = index+2;
	pDstCfg->toZero = 0;
	pDstCfg++;
	index += 4;
}

int FillYcConfig(struct VPYcCfg *pYcCfg)
{
    int i,j,pq;
	short ua,ub,uc,ia,ib,ic,s,ainum;
	BYTE type,fdnum,pnum;
	WORD pindex;
	
	VDefAiCfg *pDefAiCfg;
	VYcCfg pFdYcCfg[6];
	struct VPYcCfg *pTempCfg;
	struct VPYcCfg *pPYcCfg=(struct VPYcCfg*)((BYTE*)g_pParafileTmp+MAXFILELEN-10240);
#if (DEV_SP == DEV_SP_TTU)
	short uphz,iphz;
	WORD ThdIndex;
#endif
	
    ua = ub = uc = -1;
	ia = ib = ic = -1;
	s = -1;
	fdnum = 0;
	pindex = 0;
	ainum = 0;
	pTempCfg = pYcCfg;
	for (i=0; i<BSP_AC_BOARD; i++)
	{
	    type = Get_Ai_Type(i);
		if (type == 0xFF)
		    break;
		pDefAiCfg = GetDefAiCfg(type);
		if (pDefAiCfg == NULL)
			continue;

		GetDefPYcCfg(type, pDefAiCfg->ycnum, pPYcCfg);

        memset(pFdYcCfg, -1, sizeof(VYcCfg)*6);
		pnum =0;
        pq = 0;
		for (j=0; j<pDefAiCfg->tvyccfgnum; j++)
		{
		     if (pPYcCfg[j].type == YC_TYPE_Ua)
			 {
			     if ((fdnum > 0)&&(pnum > 0))
			     {
			        FillPQConfig(pTempCfg, pFdYcCfg, pindex, pnum);
					pindex += pnum*2 + 4;
					pTempCfg += pnum*2 + 4;
					pq = 1;
					memset(pFdYcCfg, -1, sizeof(VYcCfg)*6);	
					pnum = 0;
			     }
				 ua = pindex;
		     }
			 if (pPYcCfg[j].type == YC_TYPE_Ub)
			 	ub = pindex;
			 if (pPYcCfg[j].type == YC_TYPE_Uc)
			 	uc = pindex;
			 if (pPYcCfg[j].type == YC_TYPE_Ia)
			 {
			    if (pq == 0)
			    {
				    if ((fdnum > 0)&&(pnum > 0))
				    {
				         FillPQConfig(pTempCfg, pFdYcCfg, pindex, pnum);
					     pindex += pnum*2 + 4;
						 pTempCfg += pnum*2 + 4;
						 memset(pFdYcCfg, -1, sizeof(VYcCfg)*6);	
						 pnum =0;
				    }
			    }
				ia = pindex;
				fdnum++;
				pq = 0;
			 }
			 if ((pPYcCfg[j].type == YC_TYPE_Ia)&&(ua > -1))
			 {
			    pFdYcCfg[0].type = YC_TYPE_Pa;
				pFdYcCfg[0].arg1 = ua;
				pFdYcCfg[0].arg2 = pindex;
				pFdYcCfg[0].toZero = 0;
				pFdYcCfg[3].type = YC_TYPE_Qa;
				pFdYcCfg[3].arg1 = ua;
				pFdYcCfg[3].arg2 = pindex;
				pFdYcCfg[3].toZero = 0;
				pnum++;
			 }
			 if ((pPYcCfg[j].type == YC_TYPE_Ib)&&(ub > -1))
			 {
			    pFdYcCfg[1].type = YC_TYPE_Pb;
				pFdYcCfg[1].arg1 = ub;
				pFdYcCfg[1].arg2 = pindex;
				pFdYcCfg[1].toZero = 0;
				pFdYcCfg[4].type = YC_TYPE_Qb;
				pFdYcCfg[4].arg1 = ub;
				pFdYcCfg[4].arg2 = pindex;
				pFdYcCfg[4].toZero = 0;
				pnum++;
				ib = pindex; 
			 }
			 if ((pPYcCfg[j].type == YC_TYPE_Ic)&&(uc > -1))
			 {
			    pFdYcCfg[2].type = YC_TYPE_Pc;
				pFdYcCfg[2].arg1 = uc;
				pFdYcCfg[2].arg2 = pindex;
				pFdYcCfg[2].toZero = 0;
				pFdYcCfg[5].type = YC_TYPE_Qc;
				pFdYcCfg[5].arg1 = uc;
				pFdYcCfg[5].arg2 = pindex;
				pFdYcCfg[5].toZero = 0;
				pnum++;
				ic = pindex;
			 }
			 
			 memcpy(pTempCfg, pPYcCfg+j, sizeof(struct VPYcCfg));
			 pTempCfg->arg1 = pTempCfg->arg1 + ainum;

			 if (pTempCfg->type == YC_TYPE_SI0)
			 	pTempCfg->arg1 = ia;
			 if (pTempCfg->type == YC_TYPE_SIb)
		 	{
		 		pTempCfg->arg1 = ia;
				pTempCfg->arg2 = ic;
		 	}
			 if (pTempCfg->type == YC_TYPE_SIc)
		 	{
		 		pTempCfg->arg1 = ia;
				pTempCfg->arg2 = ib;
		 	}
			 if (pTempCfg->type == YC_TYPE_SU0)
			 	pTempCfg->arg1 = ua;
			 if (pTempCfg->type == YC_TYPE_Uab)
			 {
			    pTempCfg->arg1 = ua;
				pTempCfg->arg2 = ub;
			 }
			 if (pTempCfg->type == YC_TYPE_Ubc)
			 {
			    pTempCfg->arg1 = ub;
				pTempCfg->arg2 = uc;
			 }
			 if (pTempCfg->type == YC_TYPE_Uca)
			 {
			    pTempCfg->arg1 = uc;
				pTempCfg->arg2 = ua;
			 }
			 pTempCfg++;
			 pindex++;
		}
		ainum += pDefAiCfg->ainum;
		if ((fdnum > 0)&&(pnum > 0))
		{
		    FillPQConfig(pTempCfg, pFdYcCfg, pindex, pnum);
			pindex += pnum*2 + 4;
			pTempCfg += pnum*2 + 4;	

			s = pindex-2;
		}
	}
	if (pindex > 0)
	{
		pTempCfg->type = YC_TYPE_SFreq;
		pTempCfg->arg1 = 0;
		pTempCfg->arg2 = 0;
	    pindex++;
	}
#if (TYPE_USER  == USER_FUJIAN)
		pTempCfg++;
		pTempCfg->type = YC_TYPE_SIGNAL;
		pTempCfg->arg1 = 0;
		pTempCfg->arg2 = 0;
		pindex++;
#endif
#if(DEV_SP == DEV_SP_DTU_IU)
	pTempCfg++;
     pTempCfg->type = YC_TYPE_TEMETER;
	pTempCfg->arg1 = 0;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

    pTempCfg->type = YC_TYPE_AttUa;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttUb;
    pTempCfg->arg1 = 1;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttUc;
    pTempCfg->arg1 = 2;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttU0;
    pTempCfg->arg1 = 3;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttIa;
    pTempCfg->arg1 = 4;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttIb;
    pTempCfg->arg1 = 5;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttIc;
    pTempCfg->arg1 = 6;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttI0;
    pTempCfg->arg1 = 7;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttPa;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttPb;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttPc;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttP;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;    
    pTempCfg->type = YC_TYPE_AttQa;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttQb;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttQc;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;
    pTempCfg->type = YC_TYPE_AttQ;
    pTempCfg->arg1 = 0;
    pTempCfg->arg2 = 0;
    pindex++;
    pTempCfg++;    
#endif
#if(DEV_SP == DEV_SP_TTU)
	pTempCfg++;
     pTempCfg->type = YC_TYPE_TEMETER;
	pTempCfg->arg1 = 0;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ua;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ub;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = uc;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ia;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ib;
	pindex++;
	pTempCfg++;
	pTempCfg->type = YC_TYPE_Angle;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = ic;
	pindex++;
	pTempCfg++;


	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = YC_TYPE_Har2 + i -1;
		pTempCfg->type = type;
		pTempCfg->arg1 = ua;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ua;
	pindex++;
	pTempCfg++;

	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = YC_TYPE_Har2 + i -1;
		pTempCfg->type = type;
		pTempCfg->arg1 = ub;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ub;
	pindex++;
	pTempCfg++;

	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = YC_TYPE_Har2 + i -1;
		pTempCfg->type = type;
		pTempCfg->arg1 = uc;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
	
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = uc;
	pindex++;
	pTempCfg++;

	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = YC_TYPE_Har2 + i -1;
		pTempCfg->type = type;
		pTempCfg->arg1 = ia;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ia;
	pindex++;
	pTempCfg++;

	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = YC_TYPE_Har2 + i -1;
		pTempCfg->type = type;
		pTempCfg->arg1 = ib;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ib;
	pindex++;
	pTempCfg++;


	for( i = 0 ; i < 13 ; i++)
	{
		if(i == 0)
			type = YC_TYPE_Har1;
		else
			type = YC_TYPE_Har2 + i -1;
		pTempCfg->type = type;
		pTempCfg->arg1 = ic;
		pTempCfg->arg2 = 0;
		if(i==1)
		{
			ThdIndex = pindex;
		}
		pindex++;
		pTempCfg++;
		
	}
	pTempCfg->type = YC_TYPE_Thd;
	pTempCfg->arg1 = ThdIndex;
	pTempCfg->arg2 = ic;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_UPhZ;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 3;
	uphz = pindex;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_UPhF;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_UPh0;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_IPhZ;
	pTempCfg->arg1 = ia;
	pTempCfg->arg2 = 3;
	iphz = pindex;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_IPhF;
	pTempCfg->arg1 = ia;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_IPh0;
	pTempCfg->arg1 = ia;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_Uunb;
	pTempCfg->arg1 = uphz;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;
	
	pTempCfg->type = YC_TYPE_Iunb;
	pTempCfg->arg1 = iphz;
	pTempCfg->arg2 = 3;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Upp;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Upp;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Upp;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Lrate;
	pTempCfg->arg1 = s;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udot;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udot;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udot;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udut;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udut;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Udut;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umqr;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umqr;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umqr;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umot;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umot;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umot;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umut;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umut;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Umut;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Uavg;
	pTempCfg->arg1 = ua;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Uavg;
	pTempCfg->arg1 = ub;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

	pTempCfg->type = YC_TYPE_Uavg;
	pTempCfg->arg1 = uc;
	pTempCfg->arg2 = 0;
	pindex++;
	pTempCfg++;

#endif
	return pindex;

}

int CreateSysConfig(char *path)
{
    int  len;
	BYTE i,j,num,fdnum,type;
	DWORD aiType, ioType;
	char pathname[MAXFILENAME];  
	VDefDiCfg *pDefDiCfg;	
	VDefDoCfg *pDefDoCfg;
	VDefAiCfg *pDefAiCfg;
	struct VPFdCfg *pPFdCfg;
	struct VPAiCfg *pPAiCfg;
	struct VPDiCfg *pPDiCfg;
	struct VPDoCfg *pPDoCfg;
	struct VPYcCfg *pPYcCfg;
	struct VPSysConfig *pPSysCfg;
	char name[2*GEN_NAME_LEN];
	
    memset(ParaFileTmpBuf, 0, sizeof(ParaFileTmpBuf));
	
	pPSysCfg=(struct VPSysConfig *)(g_pParafileTmp+1);

	pPSysCfg->dwName = BACK_TYPE_IPACS; 
	strcpy(pPSysCfg->sByName, "IPACS");
	
	pPSysCfg->dwCfg = 1<<3; //以区分新老装置
	pPSysCfg->wSYXNum = 0;
	pPSysCfg->wYKNum= 0;
	pPSysCfg->wFDNum = 0;
	pPSysCfg->wAINum= 0;
	pPSysCfg->wDDNum = 0;
	len = 0;

    aiType = ioType = 0;
	num = 0;
	for (i=0; i<BSP_AC_BOARD; i++)
	{
	    type = Get_Ai_Type(i);
	    
		aiType |= (type << (i*8));
		if (type == 0xFF) 
			break;

		num++;
		pDefAiCfg = GetDefAiCfg(type);

		if (pDefAiCfg != NULL)
		{
			pPSysCfg->wFDNum += pDefAiCfg->fdnum;
			pPSysCfg->wAINum += pDefAiCfg->ainum;

#ifdef INCLUDE_DD		
			pPSysCfg->wDDNum += pDefAiCfg->tvddcfgnum*pDefAiCfg->fdnum;
#else
			pPSysCfg->wDDNum = 0;
#endif
			pPSysCfg->wSYXNum += pDefAiCfg->tvyxcfgnum_fd*pDefAiCfg->fdnum;
			pPSysCfg->wYKNum += pDefAiCfg->tvykcfgnum_fd*pDefAiCfg->fdnum; 

		}
		else
		{
			WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "未知模拟板卡型号");
			myprintf(SYS_ID, LOG_ATTR_ERROR, "Can not find AIO_type %x default cfg!", type);
		}		
	}
	pPSysCfg->dwAIType = aiType;

	pPFdCfg = (struct VPFdCfg *)(pPSysCfg+1);
	pPAiCfg = (struct VPAiCfg *)(pPFdCfg+pPSysCfg->wFDNum);

    fdnum = 0;
	for (i=0; i<num; i++)
	{
	    type = (aiType >> (i*8))&0xFF;
		if (type == 0xFF)
			break;

		pDefAiCfg = GetDefAiCfg(type);
		if (pDefAiCfg == NULL)
			continue;
	   	GetDefPFdCfg(type, pDefAiCfg->fdnum, pPFdCfg);
		for (j=0; j<pDefAiCfg->fdnum; j++)
		{
		    sprintf(name, pPFdCfg->kgname, fdnum+j+1);
			strncpy(pPFdCfg->kgname, name, GEN_NAME_LEN-1);
			pPFdCfg->kgname[GEN_NAME_LEN-1] = '\0';
			pPFdCfg->kgid += fdnum;
			pPFdCfg->kg_stateno = (fdnum+j)*6;
			pPFdCfg->kg_ykno = fdnum+j+1;
			pPFdCfg->kg_remoteno = -1;
	        pPFdCfg ++;
		}
		fdnum += pDefAiCfg->fdnum;
	    GetDefPAiCfg(type, pDefAiCfg->ainum, pPAiCfg);
	    pPAiCfg += pDefAiCfg->ainum;	
	}
	
	len += pPSysCfg->wFDNum*sizeof(struct VPFdCfg);
	len += pPSysCfg->wAINum*sizeof(struct VPAiCfg);

    pPDiCfg = (struct VPDiCfg*)pPAiCfg;

	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = Get_Io_Type(i)&0xF;
		ioType |= (type << (i*4)); 

		if (type == 0xF)  continue;

		pDefDiCfg = GetDefDiCfg(type);
		if (pDefDiCfg != NULL)
		{
#ifdef INCLUDE_DYX
			pPSysCfg->wDYXNum += pDefDiCfg->tvdyxcfgnum;
#else
			pPSysCfg->wDYXNum = 0;	
#endif		
			pPSysCfg->wSYXNum += pDefDiCfg->tvyxcfgnum_di;	

            GetDefPDiCfg(type, pDefDiCfg->dinum, pPDiCfg);
			for (j=0; j< pDefDiCfg->dinum; j++)
				pPDiCfg[j].yxno += pPSysCfg->wDINum;

		    pPSysCfg->wDINum += pDefDiCfg->dinum;
			pPDiCfg += pDefDiCfg->dinum;

		}
	}
	len += pPSysCfg->wDINum*sizeof(struct VPDiCfg);


	pPDoCfg = (struct VPDoCfg*)pPDiCfg;
	for (i=0; i<BSP_IO_BOARD; i++)
	{
	    type = Get_Io_Type(i);
		
		if (type == 0xFF)  continue;

		pDefDoCfg = GetDefDoCfg(type);
		if (pDefDoCfg != NULL)
		{
			pPSysCfg->wDONum += pDefDoCfg->donum;
			pPSysCfg->wYTNum = 0;
			pPSysCfg->wYKNum += pDefDoCfg->tvykcfgnum_do;	

			GetDefPDoCfg(type, pDefDoCfg->donum, pPDoCfg);
			pPDoCfg += pDefDoCfg->donum;

		}		
	}
	len += pPSysCfg->wDONum*sizeof(struct VPDoCfg);


	pPYcCfg = (struct VPYcCfg*)pPDoCfg;
	pPSysCfg->wYCNum = FillYcConfig(pPYcCfg);
	pPYcCfg += pPSysCfg->wYCNum;
	len += pPSysCfg->wYCNum*sizeof(struct VPYcCfg);


	
	pDefDoCfg = GetDefDoCfg(0);
	pPSysCfg->wYKNum += pDefDoCfg->tvykcfgnum_public;
		
	type = Get_Power_Type(0);
	if (type != 0xFF)
	{
		pDefAiCfg = GetDefAiCfg(0);
		if (pDefAiCfg)
		{
			pPSysCfg->wYcNumPublic = pDefAiCfg->tvyccfgnum_public;
		  GetDefPYcCfg_Public(0, pPSysCfg->wYcNumPublic, pPYcCfg);
		  pPSysCfg->wYCNum += pPSysCfg->wYcNumPublic;
		  len += pPSysCfg->wYcNumPublic*sizeof(struct VPYcCfg);
		
		  pPSysCfg->wSYXNum += pDefAiCfg->tvyxcfgnum_public*pPSysCfg->wFDNum;
		  pDefDiCfg = GetDefDiCfg(0);
		  pPSysCfg->wSYXNum += pDefDiCfg->tvyxcfgnum_public;
		}
		
	}

	pPSysCfg->dwType = BACK_TYPE_IPACS|type;
	pPSysCfg->dwAIType = aiType;
	pPSysCfg->dwIOType = ioType;


	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(struct VPSysConfig)+len;
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;

	sprintf(pathname,"system.cfg"); 
    return(WriteParaFile(pathname,g_pParafileTmp)); 
	
}

void CreateRunParaConfig(void)
{
	int i;
	char pathname[MAXFILENAME];	
	VRunParaFileCfg *pRunParaCfg;

	pRunParaCfg = (VRunParaFileCfg *)(g_pParafileTmp+1);
	memset(pRunParaCfg,0,sizeof(VRunParaFileCfg));
	
	if(g_Sys.MyCfg.wFDNum > 0)
	{
		pRunParaCfg->wPt1= g_Sys.MyCfg.pFd[0].pt;
		pRunParaCfg->wPt2= g_Sys.MyCfg.pFd[0].Un;
	}
	else
	{
		pRunParaCfg->wPt1 = 10000;
		pRunParaCfg->wPt2 = 220;
	}
	pRunParaCfg->dwDyVal= 0*1000;
	pRunParaCfg->wDyT = 600; //s
	pRunParaCfg->dwGyVal= 220*2*1000;
	#if (DEV_SP == DEV_SP_TTU)
	pRunParaCfg->wGyT = 0; //s
	#else
	pRunParaCfg->wGyT = 600; //s
	#endif
	pRunParaCfg->dwZzVal= 5*2*0.7*1000;
	pRunParaCfg->wZzT = 3600; //s
	pRunParaCfg->dwGzVal= 5*2*1000;
	pRunParaCfg->wGzT = 3600; //s
	if(g_Sys.MyCfg.wDINum > 0)
	{
		pRunParaCfg->wYxfd = g_Sys.MyCfg.pDi[0].dtime;
	}
	else
	{
		pRunParaCfg->wYxfd = 200;
	}
	if(g_Sys.MyCfg.wDONum > 0)
	{
		pRunParaCfg->wFzKeepT = g_Sys.MyCfg.pDo[0].fzontime;
		pRunParaCfg->wHzKeepT = g_Sys.MyCfg.pDo[0].hzontime;
	}
	else
	{
		pRunParaCfg->wFzKeepT = 500;
		pRunParaCfg->wHzKeepT = 500;
	}
	#ifdef INCLUDE_CELL
	if((g_Sys.MyCfg.CellCfg.dwNum > 0) && (g_Sys.MyCfg.CellCfg.pCtrl != NULL))
	{
		if(g_Sys.MyCfg.CellCfg.pCtrl->dwMode == 3)
		{
			pRunParaCfg->wDchhT = g_Sys.MyCfg.CellCfg.pCtrl->dwDayBits;
			pRunParaCfg->wDchhTime = g_Sys.MyCfg.CellCfg.pCtrl->wHour;
		}
		else
		{
			pRunParaCfg->wDchhT = 90;
			pRunParaCfg->wDchhTime = 0;
		}
	}
	else
	#endif
	{
		pRunParaCfg->wDchhT = 90;
		pRunParaCfg->wDchhTime = 0;
	}
	
	for(i=0;i < MAX_FD_NUM;i++)
	{
		if(i  < g_Sys.MyCfg.wFDNum)
		{
			pRunParaCfg->LineCfg[i].wCt1 = g_Sys.MyCfg.pFd[i].ct;
			pRunParaCfg->LineCfg[i].wCt2 = g_Sys.MyCfg.pFd[i].In;
			pRunParaCfg->LineCfg[i].wCt01 = g_Sys.MyCfg.pFd[i].ct0;
			pRunParaCfg->LineCfg[i].wCt02 = g_Sys.MyCfg.pFd[i].In0;
		}
		else
		{
			pRunParaCfg->LineCfg[i].wCt01 = 20;
			pRunParaCfg->LineCfg[i].wCt1 = 600;
			pRunParaCfg->LineCfg[i].wCt2 = 5;
			pRunParaCfg->LineCfg[i].wCt02 = 5;
		}
	}

	g_pParafileTmp->nLength = sizeof(struct VFileHead)+sizeof(VRunParaFileCfg);
	g_pParafileTmp->wVersion = CFG_FILE_VER;
	g_pParafileTmp->wAttr = CFG_ATTR_NULL;

	sprintf(pathname,"runpara.cfg");   
	WriteParaFile(pathname,g_pParafileTmp);  
}

void WriteFzT(WORD fztime)
{
	int i;
	for(i = 0;i < g_Sys.MyCfg.wDONum; i++)
	{
		g_Sys.MyCfg.pDo[i].fzontime = fztime;
	}
}


void WriteHzT(WORD hztime)
{
	int i;
	for(i = 0; i < g_Sys.MyCfg.wDONum; i++)
	{
		g_Sys.MyCfg.pDo[i].hzontime = hztime;
	}
}

void WriteYxFdT(WORD yxfdtime)
{
	int i;
	for(i =0 ;i < g_Sys.MyCfg.wDINum; i++)
	{
		g_Sys.MyCfg.pDi[i].dtime = yxfdtime;
	}
	WriteYxChnTime();
}

void WriteULP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<ycNum; i++)
	{
		t = pYcCfg[i].type;
		 if ((t >= YC_TYPE_Ua) && (t <= YC_TYPE_U0))
		{
		    pYcCfg[i].toZero = svalue;
			pYcCfg[i].toZero *=10;
		}
	}
#endif
}

void WriteILP(WORD svalue)
{
	#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<ycNum; i++)
	{
		t = pYcCfg[i].type;
		 if ((t >= YC_TYPE_Ia) && (t <= YC_TYPE_I0))
		{
			pYcCfg[i].toZero = svalue;
			pYcCfg[i].toZero *=10;
		}
	}
	#endif
}

void WritePLP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<ycNum; i++)
	{
		t = pYcCfg[i].type;
		 if (((t >= YC_TYPE_Pa)&&(t >= YC_TYPE_Qc))||((t >= YC_TYPE_P) && (t <= YC_TYPE_S)))
		{
		    pYcCfg[i].toZero = svalue;
			pYcCfg[i].toZero *=10;
		}
	}
#endif
}
void WriteFLP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<ycNum; i++)
	{
		t = pYcCfg[i].type;
		 if (t == YC_TYPE_Freq)
		{
		    pYcCfg[i].toZero = svalue;
			pYcCfg[i].toZero *=10;
		}
	}
#endif
}
void WriteZLLP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<ycNum; i++)
	{
		t = pYcCfg[i].type;
		 if (t == YC_TYPE_Dc)
		{
		    pYcCfg[i].toZero = svalue;
			pYcCfg[i].toZero *=10;
		}
	}
#endif
}
void WriteCosLP(WORD svalue)
{
#ifdef INCLUDE_YC
	int t,i;
	for (i=0; i<ycNum; i++)
	{
		t = pYcCfg[i].type;
		 if (t == YC_TYPE_Cos) 
		{
		    pYcCfg[i].toZero = svalue;
			pYcCfg[i].toZero *=10;
		}
	}
#endif
}



