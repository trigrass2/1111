#include "syscfg.h"

#ifdef INCLUDE_PR
#include "PrLib.h"
#include "sys.h"

extern VFdCfg *pCfg;
extern VFdProtCal *pVal;
extern VAiTqVal *pTqVal;
extern VPrRunSet *pRunSet;
extern VAiProtCal aiProtCal[];
extern VPrSetPublic *pPublicSet;
extern VPrRunPublic *pRunPublic;

WORD FsxBuf1[500] =  
{
	14, 28, 42, 56, 70, 83, 97, 110, 123, 136, 
	149, 162, 175, 187, 200, 212, 225, 237, 249, 261, 
	273, 285, 296, 308, 319, 331, 342, 354, 365, 376, 
	387, 398, 409, 419, 430, 441, 451, 462, 472, 482, 
	493, 503, 513, 523, 533, 543, 553, 562, 572, 582, 
	591, 601, 610, 620, 629, 638, 647, 656, 666, 675, 
	684, 693, 701, 710, 719, 728, 736, 745, 754, 762, 
	771, 779, 787, 796, 804, 812, 820, 829, 837, 845, 
	853, 861, 869, 876, 884, 892, 900, 908, 915, 923, 
	930, 938, 946, 953, 960, 968, 975, 983, 990, 997, 
	1004, 1012, 1019, 1026, 1033, 1040, 1047, 1054, 1061, 1068, 
	1075, 1082, 1088, 1095, 1102, 1109, 1115, 1122, 1129, 1135, 
	1142, 1148, 1155, 1161, 1168, 1174, 1181, 1187, 1194, 1200, 
	1206, 1212, 1219, 1225, 1231, 1237, 1243, 1250, 1256, 1262, 
	1268, 1274, 1280, 1286, 1292, 1298, 1303, 1309, 1315, 1321, 
	1327, 1333, 1338, 1344, 1350, 1356, 1361, 1367, 1373, 1378, 
	1384, 1389, 1395, 1400, 1406, 1411, 1417, 1422, 1428, 1433, 
	1439, 1444, 1449, 1455, 1460, 1465, 1470, 1476, 1481, 1486, 
	1491, 1497, 1502, 1507, 1512, 1517, 1522, 1527, 1532, 1537, 
	1542, 1547, 1552, 1557, 1562, 1567, 1572, 1577, 1582, 1587, 
	1592, 1597, 1601, 1606, 1611, 1616, 1621, 1625, 1630, 1635, 
	1639, 1644, 1649, 1653, 1658, 1663, 1667, 1672, 1677, 1681, 
	1686, 1690, 1695, 1699, 1704, 1708, 1713, 1717, 1722, 1726, 
	1731, 1735, 1739, 1744, 1748, 1752, 1757, 1761, 1766, 1770, 
	1774, 1778, 1783, 1787, 1791, 1795, 1800, 1804, 1808, 1812, 
	1816, 1821, 1825, 1829, 1833, 1837, 1841, 1845, 1849, 1854, 
	1858, 1862, 1866, 1870, 1874, 1878, 1882, 1886, 1890, 1894, 
	1898, 1902, 1906, 1909, 1913, 1917, 1921, 1925, 1929, 1933, 
	1937, 1941, 1944, 1948, 1952, 1956, 1960, 1963, 1967, 1971, 
	1975, 1978, 1982, 1986, 1990, 1993, 1997, 2001, 2004, 2008, 
	2012, 2015, 2019, 2023, 2026, 2030, 2034, 2037, 2041, 2044, 
	2048, 2052, 2055, 2059, 2062, 2066, 2069, 2073, 2076, 2080, 
	2083, 2087, 2090, 2094, 2097, 2101, 2104, 2108, 2111, 2114, 
	2118, 2121, 2125, 2128, 2131, 2135, 2138, 2142, 2145, 2148, 
	2152, 2155, 2158, 2162, 2165, 2168, 2171, 2175, 2178, 2181, 
	2185, 2188, 2191, 2194, 2198, 2201, 2204, 2207, 2210, 2214, 
	2217, 2220, 2223, 2226, 2230, 2233, 2236, 2239, 2242, 2245, 
	2249, 2252, 2255, 2258, 2261, 2264, 2267, 2270, 2273, 2276, 
	2279, 2283, 2286, 2289, 2292, 2295, 2298, 2301, 2304, 2307, 
	2310, 2313, 2316, 2319, 2322, 2325, 2328, 2331, 2334, 2337, 
	2340, 2342, 2345, 2348, 2351, 2354, 2357, 2360, 2363, 2366, 
	2369, 2372, 2374, 2377, 2380, 2383, 2386, 2389, 2392, 2394, 
	2397, 2400, 2403, 2406, 2409, 2411, 2414, 2417, 2420, 2423, 
	2425, 2428, 2431, 2434, 2436, 2439, 2442, 2445, 2447, 2450, 
	2453, 2456, 2458, 2461, 2464, 2467, 2469, 2472, 2475, 2477, 
	2480, 2483, 2485, 2488, 2491, 2493, 2496, 2499, 2501, 2504, 
	2507, 2509, 2512, 2515, 2517, 2520, 2522, 2525, 2528, 2530, 
	2533, 2535, 2538, 2541, 2543, 2546, 2548, 2551, 2553, 2556, 
	2558, 2561, 2564, 2566, 2569, 2571, 2574, 2576, 2579, 2581, 
	2584, 2586, 2589, 2591, 2594, 2596, 2599, 2601, 2604, 2606 
};

// 反时限的常数表 (k^0.02 -1)/0.14*10000  其中 k=1.1~31 步长0.1 
WORD  	FsxBuf2[300]=
{
	136, 261, 376, 482, 582, 675, 762, 845, 923, 997, 
	1068, 1135, 1200, 1262, 1321, 1378, 1433, 1486, 1537, 1587, 
	1635, 1681, 1726, 1770, 1812, 1854, 1894, 1933, 1971, 2008, 
	2044, 2080, 2114, 2148, 2181, 2214, 2245, 2276, 2307, 2337, 
	2366, 2394, 2423, 2450, 2477, 2504, 2530, 2556, 2581, 2606, 
	2631, 2655, 2678, 2702, 2725, 2747, 2770, 2792, 2813, 2835, 
	2856, 2877, 2897, 2917, 2937, 2957, 2976, 2996, 3015, 3033, 
	3052, 3070, 3088, 3106, 3124, 3141, 3158, 3175, 3192, 3209, 
	3225, 3242, 3258, 3274, 3290, 3305, 3321, 3336, 3351, 3366, 
	3381, 3396, 3411, 3425, 3439, 3454, 3468, 3482, 3495, 3509, 
	3523, 3536, 3549, 3563, 3576, 3589, 3602, 3614, 3627, 3640, 
	3652, 3664, 3677, 3689, 3701, 3713, 3725, 3737, 3748, 3760, 
	3771, 3783, 3794, 3805, 3817, 3828, 3839, 3850, 3861, 3871, 
	3882, 3893, 3903, 3914, 3924, 3935, 3945, 3955, 3965, 3975, 
	3985, 3995, 4005, 4015, 4025, 4034, 4044, 4054, 4063, 4073, 
	4082, 4091, 4101, 4110, 4119, 4128, 4137, 4146, 4155, 4164, 
	4173, 4182, 4191, 4199, 4208, 4217, 4225, 4234, 4242, 4251, 
	4259, 4268, 4276, 4284, 4292, 4300, 4309, 4317, 4325, 4333, 
	4341, 4349, 4356, 4364, 4372, 4380, 4388, 4395, 4403, 4410, 
	4418, 4426, 4433, 4440, 4448, 4455, 4463, 4470, 4477, 4484, 
	4492, 4499, 4506, 4513, 4520, 4527, 4534, 4541, 4548, 4555, 
	4562, 4569, 4576, 4583, 4589, 4596, 4603, 4609, 4616, 4623, 
	4629, 4636, 4642, 4649, 4655, 4662, 4668, 4675, 4681, 4687, 
	4694, 4700, 4706, 4713, 4719, 4725, 4731, 4737, 4744, 4750, 
	4756, 4762, 4768, 4774, 4780, 4786, 4792, 4798, 4804, 4809, 
	4815, 4821, 4827, 4833, 4838, 4844, 4850, 4856, 4861, 4867, 
	4873, 4878, 4884, 4889, 4895, 4901, 4906, 4912, 4917, 4922, 
	4928, 4933, 4939, 4944, 4950, 4955, 4960, 4966, 4971, 4976, 
	4981, 4987, 4992, 4997, 5002, 5007, 5013, 5018, 5023, 5028, 
	5033, 5038, 5043, 5048, 5053, 5058, 5063, 5068, 5073, 5078
};

// 反时限的常数表 (k^0.02 -1)/0.14*10000  k=1~100 步长1 
WORD FsxBuf3[100]=
{
	0, 997, 1587, 2008, 2337, 2606, 2835, 3033, 3209, 3366, 
	3509, 3640, 3760, 3871, 3975, 4073, 4164, 4251, 4333, 4410, 
	4484, 4555, 4623, 4687, 4750, 4809, 4867, 4922, 4976, 5028, 
	5078, 5127, 5174, 5220, 5264, 5307, 5349, 5390, 5430, 5469, 
	5507, 5544, 5580, 5616, 5650, 5684, 5718, 5750, 5782, 5813, 
	5844, 5874, 5903, 5932, 5960, 5988, 6016, 6043, 6069, 6095, 
	6121, 6146, 6171, 6195, 6219, 6243, 6266, 6290, 6312, 6335, 
	6357, 6378, 6400, 6421, 6442, 6463, 6483, 6503, 6523, 6543, 
	6562, 6581, 6600, 6619, 6637, 6655, 6673, 6691, 6709, 6726, 
	6744, 6761, 6778, 6794, 6811, 6827, 6844, 6860, 6876, 6891
};
/////////////////////////////////////////////////////////////////////
//时间继电器子程序
////////////////////////////////////////////////////////////////////
void  InitTR(TIMERELAY *pTimeRelay,DWORD dTripThreshold,DWORD dRetThreshold)//初始化时间继电器
{
	
	pTimeRelay->dTripThreshold=dTripThreshold;
	pTimeRelay->dRetThreshold=dRetThreshold;
	pTimeRelay->boolTrip=FALSE;
	pTimeRelay->boolStart=FALSE;
	pTimeRelay->boolHold= FALSE;
	pTimeRelay->boolInputold = FALSE;
	pTimeRelay->dTimer=0;
	pTimeRelay->dTimerRetTimer=0;
	pTimeRelay->dTRTThreshold=0;
	pTimeRelay->dIntervalHold=0;
}

void  ResetTR(TIMERELAY *pTimeRelay)//复位时间继电器
{
	pTimeRelay->boolTrip=FALSE;
	pTimeRelay->boolStart=FALSE;
	pTimeRelay->dTimer=0;
	pTimeRelay->dTimerRetTimer=0;
	pTimeRelay->boolInputold = FALSE;
	pTimeRelay->boolHold=FALSE;
	pTimeRelay->dIntervalHold=0;
}

void   SetTRHold(TIMERELAY *pTimeRelay, BOOL boolInput)
{
   if (pTimeRelay->boolStart && !boolInput)
   {
       if (!pTimeRelay->boolHold)
       {
          pTimeRelay->boolHold = TRUE;
       }
   }
	else
	{
		if(pTimeRelay->boolHold)
		{
			pTimeRelay->boolHold = FALSE;
			pTimeRelay->dIntervalHold=0;
		}
	}
}

void   RunTR(TIMERELAY *pTimeRelay,BOOL boolInput)//运行时间继电器
{
    DWORD interval;
	DWORD time;

	time = Get100usCnt();

	if(!pTimeRelay->boolTrip)
	{
		if(boolInput)
		{
			if(pTimeRelay->boolStart)
			{
			    if (pTimeRelay->boolHold)
			    {
			        pTimeRelay->dTimer = time - pTimeRelay->dIntervalHold;
			    }
			    interval = time - pTimeRelay->dTimer;
			}
			else
			{
			    pTimeRelay->boolStart=TRUE;
				pTimeRelay->dTimer = time;
				interval = 0;
			}
			if(interval >= pTimeRelay->dTripThreshold)
			{
				pTimeRelay->boolTrip=TRUE;
				pTimeRelay->dTimer=time;
			}
			pTimeRelay->dTimerRetTimer=time;

			if(pTimeRelay->boolHold)
			{
				pTimeRelay->boolHold = FALSE;
				pTimeRelay->dIntervalHold=0;
			}
		}
		else
		{
			if(pTimeRelay->boolInputold)
				pTimeRelay->dIntervalHold =  pTimeRelay->dTimerRetTimer -pTimeRelay->dTimer; //保留之前的计时
				
		    interval = time - pTimeRelay->dTimerRetTimer;
			pTimeRelay->dTimer=time;
			if(interval >= pTimeRelay->dTRTThreshold)
			{
				pTimeRelay->dTimer=time;
				pTimeRelay->boolStart=FALSE;
				pTimeRelay->boolHold=FALSE;
				pTimeRelay->dIntervalHold=0;
			}
		}	
	}
	else
	{
		if(!boolInput)
		{
			if(pTimeRelay->boolStart)
				interval = time - pTimeRelay->dTimer;
			else
			{
			    pTimeRelay->boolStart=TRUE;
				pTimeRelay->dTimer = time;
				interval = 0;
			}
			if(interval >= pTimeRelay->dRetThreshold)
			{
				pTimeRelay->boolTrip=FALSE;
				pTimeRelay->boolStart=FALSE;
				pTimeRelay->dTimer=time;
			}
			pTimeRelay->dTimerRetTimer=time;
		}
		else
		{
			pTimeRelay->dTimer=time;
			pTimeRelay->dTimerRetTimer=time;
		}
		pTimeRelay->boolHold=FALSE;
		pTimeRelay->dIntervalHold=0;
	}
	pTimeRelay->boolInputold = boolInput;
}

void  SetTR_TRTT(TIMERELAY *pTimeRelay,DWORD dTRTT)//设置时间继电器延时中返回时间
{
	pTimeRelay->dTRTThreshold=dTRTT;
}

void  PreSetTime(TIMERELAY *pTimeRelay,DWORD dPreTime)//预置时间继电器
{
	pTimeRelay->dTimer=dPreTime;
	pTimeRelay->boolStart=TRUE;
}

#define INVR_INITED			0x01
#define INVR_CONFIGED		0x02
#define INVR_RUNNED			0x04
#define INVR_PICKUP			0x40
#define INVR_OPERATED		0x80
#define CURV_NORMAL         0
#define CURV_VERY           1
#define CURV_EXTREME        2


DWORD InverseNormal(DWORD dI, DWORD dSet)
{
    DWORD dResult;
	DWORD dFact;

	if(dI > 0x2000000 )
    {
       dI=dI>>7;
	   dSet=dSet>>7;
    }
	dFact = dI * 100 / dSet;
	if(dFact <= 600)
		dResult = FsxBuf1[dFact-101];
	else if (dFact <= 3100 ) 
		dResult = FsxBuf2[dFact/10 - 11];
	else if (dFact <= 10000 ) 
		dResult = FsxBuf3[dFact/100 - 1];
	return dResult;
}

DWORD InverseVery(DWORD dI, DWORD dSet)
{
    DWORD dResult;
	DWORD dFact;

	if(dI > 0x2000000 )
    {
       dI=dI>>7;
	   dSet=dSet>>7;
    }
	dFact = dI * 100 / dSet;
	dResult = (dFact-100)*1000/135;
	return dResult;
}

DWORD InverseExtreme(DWORD dI, DWORD dSet)
{
    DWORD dResult;
	DWORD dFact;

	dFact = dI*100/dSet;

	dResult = dFact*dFact - 10000;
	dResult = dResult / 100;
	dResult = dResult * 125;
	dResult = dResult / 100;
	return dResult;
}
static void INVR_Init(INV_RELAY *pRelay)
{
	pRelay->flags 	=0;
	pRelay->integral=0;
}

static void INVR_Reset(INV_RELAY *pRelay)
{
	pRelay->flags   =0;
	pRelay->integral=0;
}

static void INVR_Setup(INV_RELAY *pRelay, INVR_CONFIG *pConfig)
{
	pRelay->config =*pConfig;
}

static void INVR_Run(int phase, INV_RELAY *pRelay, DWORD dIx, BOOL *bPickup, BOOL *bOperate)
{
	DWORD	tick_now;
	DWORD	dDelta, temp, intg;

	*bPickup  =FALSE;
	*bOperate =FALSE;
	tick_now  =Get100usCnt();
	if (!(pRelay->flags & INVR_RUNNED))
	{
		pRelay->flags	|=INVR_RUNNED;
		pRelay->integral =0;
		pRelay->tick  	 =tick_now;
		return;
	}
	if(dIx <= pRelay->config._IP+pRelay->config._IP*3/100)
	{
		if( dIx < pRelay->config._IP )
		{
		    pRelay->integral =0;
		}
		pRelay->tick  	 =tick_now;
		return;
	}
	*bPickup =TRUE;
	if (pRelay->integral > pRelay->config._TINV)
	{
		pRelay->tick  	 =tick_now;
		*bOperate =TRUE;
		return;
	}

	//计算定点Ibuf=IJG*0x10000/IVI_SET
	switch(pRelay->config._CUV)
	{
	   case CURV_NORMAL:
	   	temp = InverseNormal(dIx, pRelay->config._IP);
	   	break;
	   case CURV_VERY:
	   	temp = InverseVery(dIx, pRelay->config._IP);
	   	break;
	   case CURV_EXTREME:
	   	temp = InverseExtreme(dIx, pRelay->config._IP);
	   	break;
	}

	dDelta   =tick_now-pRelay->tick;
	pRelay->tick =tick_now;
	intg =temp*dDelta;
	pRelay->integral +=intg;

	if (pRelay->integral > pRelay->config._TINV)
		*bOperate =TRUE;
	/* probe */	
}

#define MY_21_TIMER1	((TMAXTIMERELAY *)(pelm->apvUser[0]))
#define MY_21_INVR		((INVR_3PHASE *)(pelm->apvUser[1]))
#define MY_21_FLAG1	    (pelm->aulUser[0])
#define F1_21_INVMODE	0x01	
void PRLIB_21_SCAN(EP_ELEMENT *pelm)
{
	DWORD   I,idz;
    BOOL    pu[3],opt[3];
	int     i;
	INVR_CONFIG	cfg[3];

    pu[0]  = pu[1]  = pu[2]  = FALSE;
	opt[0] = opt[1] = opt[2] = FALSE;
   	if (pRunSet->bSetChg||pelm->bSetChg)
	{
	    for(i=0; i<3; i++)
		    InitTR(&(MY_21_TIMER1->tMaxTimeRelay[i]), pelm->ppioIn[_PR_21_T_PICK].ulVal, pelm->ppioIn[_PR_21_T_DROP].ulVal);
		if(pelm->ppioIn[_PR_21_INVMODE].bVal)
		{
			for(i=0; i<3; i++)
			{
				cfg[i]._IP   =pelm->ppioIn[_PR_21_IA_PICK+i].ulVal;
				cfg[i]._CUV  =pelm->ppioIn[_PR_21_CUV].ulVal;
				cfg[i]._TINV =pelm->ppioIn[_PR_21_T_PICK].ulVal*10000;
				INVR_Setup(&MY_21_INVR->p[i], &cfg[i]);
			}
		}
		pelm->bSetChg = FALSE;
	}
	if (!pelm->ppioIn[_PR_21_ENABLE].bVal)
	{
	    for(i=0; i<3; i++)
		    ResetTR(&(MY_21_TIMER1->tMaxTimeRelay[i]));
		INVR_Reset(&MY_21_INVR->p[0]);
		INVR_Reset(&MY_21_INVR->p[1]);
		INVR_Reset(&MY_21_INVR->p[2]);
		pelm->aioOut[_PR_21_PICKUP].bVal=FALSE;
		pelm->aioOut[_PR_21_OPERATE].bVal=FALSE;
		pelm->aioOut[_PR_21_A_OPT].bVal=FALSE;
		pelm->aioOut[_PR_21_B_OPT].bVal=FALSE;
		pelm->aioOut[_PR_21_C_OPT].bVal=FALSE;
		return;
	}
	/* 反时限处理 */
	if (pelm->ppioIn[_PR_21_INVMODE].bVal)
	{
		if (!(MY_21_FLAG1 & F1_21_INVMODE))
		{
			MY_21_FLAG1 |=0x01;//F1_21_INVMODE;
			INVR_Reset(&MY_21_INVR->p[0]);
			INVR_Reset(&MY_21_INVR->p[1]);
			INVR_Reset(&MY_21_INVR->p[2]);
		}

		if(pelm->ppioIn[_PR_21_I0MODE].bVal)
			INVR_Run(4, &MY_21_INVR->p[0], pVal->dI0, &pu[0], &opt[0]);
		else
        {
           for(i=0; i<3; i++)
		      INVR_Run(i, &MY_21_INVR->p[i], pVal->dI[i], &pu[i], &opt[i]);
		}
		
		pelm->aioOut[_PR_21_PICKUP].bVal = pu[0]|pu[1]|pu[2];
		
		if (pelm->ppioIn[_PR_21_BLOCKED].bVal)
		{
			pelm->aioOut[_PR_21_OPERATE].bVal=FALSE;
			pelm->aioOut[_PR_21_A_OPT].bVal=FALSE;
			pelm->aioOut[_PR_21_B_OPT].bVal=FALSE;
			pelm->aioOut[_PR_21_C_OPT].bVal=FALSE;
			return;			
		}
		if (pelm->ppioIn[_PR_21_DIR_ENABLE].bVal)
		{
			opt[0] =(BOOL)(opt[0] && pelm->ppioIn[_PR_21_A_DIR].bVal);
			opt[1] =(BOOL)(opt[1] && pelm->ppioIn[_PR_21_B_DIR].bVal);
			opt[2] =(BOOL)(opt[2] && pelm->ppioIn[_PR_21_C_DIR].bVal);
		}
		if (opt[0] || opt[1] || opt[2])
		{
		    pelm->aioOut[_PR_21_OPERATE].bVal =TRUE;
#if (TYPE_USER == USER_GUANGXI)
            pelm->aioOut[_PR_21_A_OPT].bVal	=opt[0]&&opt[1];
		    pelm->aioOut[_PR_21_B_OPT].bVal	=opt[1]&&opt[2];
		    pelm->aioOut[_PR_21_C_OPT].bVal	=opt[2]&&opt[0];
#else
			pelm->aioOut[_PR_21_A_OPT].bVal	=opt[0];
		    pelm->aioOut[_PR_21_B_OPT].bVal	=opt[1];
		    pelm->aioOut[_PR_21_C_OPT].bVal	=opt[2];
#endif
		}
		else
		{
		    pelm->aioOut[_PR_21_OPERATE].bVal =FALSE;
			pelm->aioOut[_PR_21_A_OPT].bVal =FALSE;
	        pelm->aioOut[_PR_21_B_OPT].bVal =FALSE;
	        pelm->aioOut[_PR_21_C_OPT].bVal =FALSE;
		}
		return;
	}

	/* 定时限处理 */
	if (MY_21_FLAG1 & F1_21_INVMODE)
	{
		MY_21_FLAG1 &=~F1_21_INVMODE;
		for(i=0; i<3; i++)
		    ResetTR(&(MY_21_TIMER1->tMaxTimeRelay[i]));
	}

    if(pelm->ppioIn[_PR_21_I0MODE].bVal)
    {
        I   = pVal->dI[3];
		idz = pelm->ppioIn[_PR_21_IA_PICK].ulVal;
		
#if (TYPE_USER != USER_BEIJING) // 北京序列特殊，不然不能复归
		if(pelm->aioOut[_PR_21_OPERATE].bVal)
		   idz =pelm->ppioIn[_PR_21_IA_DROP].ulVal;
#endif

	    if (I > idz)
	   	   pu[0] = TRUE;
	    else
	   	   pu[0] = FALSE;
    }
	else
	{
	    for(i=0; i<3; i++)
		{
		   I   = pVal->dI[i];
		   idz = pelm->ppioIn[_PR_21_IA_PICK+i].ulVal;
			 
#if (TYPE_USER != USER_BEIJING) // 北京序列特殊，不然不能复归
		   if (pelm->aioOut[_PR_21_A_OPT+i].bVal)
			   idz =pelm->ppioIn[_PR_21_IA_DROP+i].ulVal;
#endif

		   if(I > idz)
		   	   pu[i] = TRUE;
		   else
		   	   pu[i] = FALSE;
		}
	}

    if(pu[0]||pu[1]||pu[2])
	    pelm->aioOut[_PR_21_PICKUP].bVal =TRUE;
	else
		pelm->aioOut[_PR_21_PICKUP].bVal = FALSE;

	if (pelm->ppioIn[_PR_21_BLOCKED].bVal)
	{
	    for(i=0; i<3; i++)
		    ResetTR(&(MY_21_TIMER1->tMaxTimeRelay[i]));
		pelm->aioOut[_PR_21_OPERATE].bVal=FALSE;
		pelm->aioOut[_PR_21_A_OPT].bVal=FALSE;
		pelm->aioOut[_PR_21_B_OPT].bVal=FALSE;
		pelm->aioOut[_PR_21_C_OPT].bVal=FALSE;
		return;			
	}

	if (pelm->ppioIn[_PR_21_DIR_ENABLE].bVal)
	{
		pu[0] =(BOOL)(pu[0] && pelm->ppioIn[_PR_21_A_DIR].bVal);
		pu[1] =(BOOL)(pu[1] && pelm->ppioIn[_PR_21_B_DIR].bVal);
		pu[2] =(BOOL)(pu[2] && pelm->ppioIn[_PR_21_C_DIR].bVal);
	}

    for(i=0; i<3; i++)
    {
    	RunTR(&(MY_21_TIMER1->tMaxTimeRelay[i]), pu[i]);
    }
	
	if (MY_21_TIMER1->tMaxTimeRelay[0].boolTrip|
		MY_21_TIMER1->tMaxTimeRelay[1].boolTrip|
		MY_21_TIMER1->tMaxTimeRelay[2].boolTrip)
	{
	    pelm->aioOut[_PR_21_OPERATE].bVal =TRUE;
		pelm->aioOut[_PR_21_A_OPT].bVal =MY_21_TIMER1->tMaxTimeRelay[0].boolTrip;
	    pelm->aioOut[_PR_21_B_OPT].bVal =MY_21_TIMER1->tMaxTimeRelay[1].boolTrip;
	    pelm->aioOut[_PR_21_C_OPT].bVal =MY_21_TIMER1->tMaxTimeRelay[2].boolTrip;	
	}
	else
	{
	    pelm->aioOut[_PR_21_OPERATE].bVal =FALSE;
		pelm->aioOut[_PR_21_A_OPT].bVal =FALSE;
	    pelm->aioOut[_PR_21_B_OPT].bVal =FALSE;
	    pelm->aioOut[_PR_21_C_OPT].bVal =FALSE;	
	}
}


int PRLIB_21(EP_ELEMENT *pelm)
{
	MY_21_FLAG1  =0;
	
	pelm->apvUser[0] =(TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
	pelm->apvUser[1] =(INVR_3PHASE *)malloc(sizeof(INVR_3PHASE));
	pelm->ppioIn =(VALUE_TYPE *)malloc(_PR_21_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut =(VALUE_TYPE *)malloc(_PR_21_OUTPUT*sizeof(VALUE_TYPE));
	
	INVR_Init(MY_21_INVR->p);
	INVR_Init(MY_21_INVR->p+1);
	INVR_Init(MY_21_INVR->p+2);
				
	pelm->aioOut[_PR_21_PICKUP].bVal=FALSE;
	pelm->aioOut[_PR_21_OPERATE].bVal=FALSE;
	pelm->aioOut[_PR_21_A_OPT].bVal=FALSE;
	pelm->aioOut[_PR_21_B_OPT].bVal=FALSE;
	pelm->aioOut[_PR_21_C_OPT].bVal=FALSE;
		
	pelm->Scan_Func=PRLIB_21_SCAN;
	
	return 0;	
}

#define MY_22_TIMER1		((TIMERELAY *)(pelm->apvUser[0]))
#define MY_22_TIMER2		((TIMERELAY *)(pelm->apvUser[1]))
void PRLIB_22_SCAN(EP_ELEMENT *pelm)
{
	BOOL   bYy, bDy, bSlip;
	int i;
	DWORD idz;

    if (pRunSet->bSetChg)
	{
		InitTR(MY_22_TIMER1, pRunSet->dTDy, 0);
		InitTR(MY_22_TIMER2, pRunSet->dTYy, 0);	
	}

	bDy = TRUE;
	for(i=0; i<3; i++)
	{
	   if((pelm->ppioIn[_PR_22_UAB+i].ulVal > pRunSet->dUWy[i])&&
	   	  pRunSet->bPTU[i])
	   {
	      bDy = FALSE;
		  break;
	   }
	}
	pelm->aioOut[_PR_22_WY].bVal = bDy;
	
	if ((!pelm->ppioIn[_PR_22_ENABLE].bVal)||!(pRunSet->bPTU[0]||pRunSet->bPTU[1]||pRunSet->bPTU[2]))
	{
		ResetTR(MY_22_TIMER1);
		ResetTR(MY_22_TIMER2);
		pelm->aioOut[_PR_22_PICKUP].bVal=FALSE;
		pelm->aioOut[_PR_22_OPERATE].bVal=FALSE;
		pelm->aioOut[_PR_22_YY_OPERATE].bVal=FALSE;
		return;
	}

    bYy = TRUE;
    for(i=0; i<3; i++)
	{
	   if(((pelm->ppioIn[_PR_22_UAB+i].ulVal < pRunSet->dUDy[i])&&
	   	  pRunSet->bPTU[i]))
	   {
	      bYy = FALSE;
		  break;
	   }
    }
	RunTR(MY_22_TIMER2,bYy);
	if(MY_22_TIMER2->boolTrip&&pRunSet->bCYY)
	   pelm->aioOut[_PR_22_YY_OPERATE].bVal=TRUE;
	if(pelm->ppioIn[_PR_22_TWJ].bVal)
	   pelm->aioOut[_PR_22_YY_OPERATE].bVal=FALSE;

    bDy = FALSE;
	for(i=0; i<3; i++)
	{
	   if((pelm->ppioIn[_PR_22_UAB+i].ulVal < pRunSet->dUDy[i])&&
	   	  pRunSet->bPTU[i])
	   {
	      bDy = TRUE;
		  break;
	   }
	}
	pelm->aioOut[_PR_22_DYBS].bVal = bDy;

	bSlip = FALSE;

	for(i=0; i<3; i++)
	{
	    if((pVal->dUDelta[i] > pRunSet->dUSlip[i])&&pRunSet->bPTU[i])
		{
		   bSlip = TRUE;
		   break;
	    }
	}


    bDy = TRUE;
	for(i=0; i<3; i++)
	{
	   if(pelm->aioOut[_PR_22_OPERATE].bVal)
	   	 idz = pRunSet->dUDyRet[i];
	   else
	   	 idz = pRunSet->dUDy[i];
	   if(((pelm->ppioIn[_PR_22_UAB+i].ulVal > idz)||
	   	   (pelm->ppioIn[_PR_22_UAB+i].ulVal < pRunSet->dUxxMin[i]))&&
	   	   pRunSet->bPTU[i])
	   {
	      bDy = FALSE;
		  break;
	   }
	}

	if(bDy && pRunSet->bCYY)
		bDy = pelm->aioOut[_PR_22_YY_OPERATE].bVal;

	pelm->aioOut[_PR_22_PICKUP].bVal = bDy;

 	if(pelm->ppioIn[_PR_22_BLOCKED].bVal||bSlip)
	{
	   ResetTR(MY_22_TIMER1);
	   pelm->aioOut[_PR_22_OPERATE].bVal = FALSE;
	   return;
	}
	RunTR(MY_22_TIMER1, bDy);
	pelm->aioOut[_PR_22_OPERATE].bVal = MY_22_TIMER1->boolTrip;
}

int PRLIB_22(EP_ELEMENT *pelm)
{
	pelm->apvUser[0] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->apvUser[1] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->ppioIn =(VALUE_TYPE *)malloc(_PR_22_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut =(VALUE_TYPE *)malloc(_PR_22_OUTPUT*sizeof(VALUE_TYPE));
		
	pelm->aioOut[_PR_22_PICKUP].bVal=FALSE;
	pelm->aioOut[_PR_22_OPERATE].bVal=FALSE;
	pelm->aioOut[_PR_22_YY_OPERATE].bVal=FALSE;

	pelm->Scan_Func=PRLIB_22_SCAN;
	
	return 0;	
}

/*方向元件*/
typedef struct DIR_CTX_STRUCT
{
	COMPLEX		Iph, Upp, Upp3;
	DWORD   	*p_flag;
	COMPLEX		I0, U0;
	DWORD       U_2V, U_1V, U0_2V;
	long        p_ang1, p_ang2;
	long 		n_ang1, n_ang2;
}	DIR_CTX;

static BOOL fallinto_zone(long dAngle, long low, long up)
{
	
	if ((low < dAngle) && (dAngle < up))
		return TRUE;
	if (dAngle < 0)
		dAngle +=(360<<16);
	else
		dAngle -=(360<<16);
	if ((low < dAngle) && (dAngle < up))
		return TRUE;
	return FALSE;
}

/* 相间方向元件 */
#define DIR_F_PICKUP	0x80	/*正方向标志*/
#define DIR_F_CHECKED	0x08	/*是否进行过比相标志*/
#define DIR_F_VOLTOK	0x02	/*电压符合标志*/
#define DIR_F_CALLED	0x01	/*无压时比相计算过标志*/
static BOOL phase_dir(DIR_CTX *ctx)
{
	long    dAngle;
	DWORD	uFlags =*(ctx->p_flag);

    if (uFlags & DIR_F_CHECKED)
    { 
    	/*非初次比相判别*/
        if (F_AMP(ctx->Upp)> ctx->U_2V)
        {
        	/* 本次电压符合 */
        	uFlags &=~DIR_F_CALLED;
			if (uFlags & DIR_F_VOLTOK)
            {   
            	/* 两次电压比较符合 */
				dAngle=F_ANG(ctx->Upp, F_AMP(ctx->Upp))-F_ANG(ctx->Iph, F_AMP(ctx->Iph));
				if (fallinto_zone(dAngle, ctx->p_ang1, ctx->p_ang2))
				    uFlags |=DIR_F_PICKUP;
				else
				    uFlags &=~DIR_F_PICKUP;
            }
            else
            {   
            	/* 上次电压不符合, 置符合标志，但不进行方向判别，保留原方向 */
            	uFlags |=DIR_F_VOLTOK;
            }
        }
        else
        {   
	    if (F_AMP(ctx->Upp3)> ctx->U_2V)
	    {
	        	/* 本次电压不符合 */
	            uFlags &=~DIR_F_VOLTOK;
	            if (!(uFlags & DIR_F_CALLED))
	            {   
	                uFlags |=DIR_F_CALLED;
				    dAngle=F_ANG(ctx->Upp3, F_AMP(ctx->Upp3))-F_ANG(ctx->Iph, F_AMP(ctx->Iph));
	                if (fallinto_zone(dAngle, ctx->p_ang1, ctx->p_ang2))
	                    uFlags |=DIR_F_PICKUP;
	                else
	                    uFlags &=~DIR_F_PICKUP;
	            }
	    }
            /* 保留原方向*/
	}
    }
    else
   	{
		/* 初次比相 */
	    uFlags |=DIR_F_CHECKED;
		if (F_AMP(ctx->Upp) > ctx->U_1V)
		{
			uFlags |=DIR_F_VOLTOK;
			uFlags |=DIR_F_PICKUP;
		}
		else
		{
			uFlags &=~DIR_F_VOLTOK;
			uFlags &=~DIR_F_PICKUP;		
	    }
	}
	*(ctx->p_flag) =uFlags;
	return ((uFlags & DIR_F_PICKUP)?TRUE:FALSE);	
}

/* 零序方向元件 */
static BOOL zero_dir(DIR_CTX *ctx)
{
	long dAngle;
	
	if (F_AMP(ctx->U0) < ctx->U0_2V)
		return FALSE;

	dAngle = F_ANG(ctx->U0, F_AMP(ctx->U0))-F_ANG(ctx->I0, F_AMP(ctx->I0));
	
	return fallinto_zone(dAngle, ctx->n_ang1, ctx->n_ang2);
}


/* index for user defined data in pelm->aulUser[] */
#define MY_23_A_FLAG		    (pelm->aulUser[0])
#define MY_23_B_FLAG            (pelm->aulUser[1])
#define MY_23_C_FLAG            (pelm->aulUser[2])
#define MY_23_TIMER1    ((TMAXTIMERELAY*)(pelm->apvUser[0]))

void PRLIB_23_SCAN(EP_ELEMENT *pelm)
{
    DIR_CTX		ctx;
	COMPLEX  Ua,Ub,Uc;
	BOOL ui_fx;
	if (pRunSet->bSetChg)
	{
		InitTR((&MY_23_TIMER1->tMaxTimeRelay[0]), 100, 0);
		InitTR((&MY_23_TIMER1->tMaxTimeRelay[1]), 100, 0);
		InitTR((&MY_23_TIMER1->tMaxTimeRelay[2]), 100, 0);
		InitTR((&MY_23_TIMER1->tMaxTimeRelay[3]), 100, 0);
	}
	RD_Last_P(MMI_MEA_UA, FT_SAM_POINT_N<<2, &Ua);
	RD_Last_P(MMI_MEA_UB, FT_SAM_POINT_N<<2, &Ub);
	RD_Last_P(MMI_MEA_UC, FT_SAM_POINT_N<<2, &Uc);
	if (pelm->ppioIn[_PR_23_VVMODE].bVal)	//Y
	{
	   Ub.x = 0;
	   Ub.y = 0;
	}

	ctx.p_ang1 =pelm->ppioIn[_PR_23_PANG1].lVal;
	ctx.p_ang2 =pelm->ppioIn[_PR_23_PANG2].lVal;
	ctx.n_ang1 =pelm->ppioIn[_PR_23_NANG1].lVal;
	ctx.n_ang2 =pelm->ppioIn[_PR_23_NANG2].lVal;
	ctx.U0_2V  =pRunSet->dU0_8V>>2;

    if(pelm->ppioIn[_PR_23_I_ENABLE].bVal)
	{
		ctx.Iph     = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_IA]].UI;
	    ctx.U_2V    = pRunSet->dU_16V[0]>>3;
		ctx.U_1V    = ctx.U_2V/2;
		if (!pelm->ppioIn[_PR_23_VVMODE].bVal)
	    {
	       ctx.Upp     = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_UB]].Uxx;
		   ctx.Upp3    = SUB(Ub,Uc);
		}
		else
		{
		   ctx.Upp     = SIG(aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_UC]].UI);
		   ctx.Upp3    = SIG(Uc);
		}
		ctx.p_flag = &MY_23_A_FLAG;
		ui_fx = phase_dir(&ctx);
		RunTR((&MY_23_TIMER1->tMaxTimeRelay[0]),ui_fx);
		pelm->aioOut[_PR_23_A_DIR].bVal = MY_23_TIMER1->tMaxTimeRelay[0].boolTrip;

		ctx.Iph     = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_IB]].UI;
		ctx.U_2V    = pRunSet->dU_16V[1]>>3;
		ctx.U_1V    = ctx.U_2V/2;

	    if(!pelm->ppioIn[_PR_23_VVMODE].bVal)
		{
		   ctx.Upp     = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_UC]].Uxx;
		   ctx.Upp3    = SUB(Uc,Ua);
	    }
		else
		{
		   ctx.Upp     = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_UC]].Uxx;
		   ctx.Upp3    = SUB(Uc,Ua);
		}
		ctx.p_flag = &MY_23_B_FLAG;
		ui_fx = phase_dir(&ctx);
		RunTR((&MY_23_TIMER1->tMaxTimeRelay[1]),ui_fx);
		pelm->aioOut[_PR_23_B_DIR].bVal = MY_23_TIMER1->tMaxTimeRelay[1].boolTrip;

		ctx.Iph     = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_IC]].UI;
		ctx.U_2V    = pRunSet->dU_16V[2]>>3;
		ctx.U_1V    = ctx.U_2V/2;
	    if(!pelm->ppioIn[_PR_23_VVMODE].bVal)
		{
		   ctx.Upp     = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_UA]].Uxx;
		   ctx.Upp3    = SUB(Ua,Ub);
	    }
		else
		{
		   ctx.Upp     = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_UA]].UI;
		   ctx.Upp3    = Ua;
		}
		ctx.p_flag = &MY_23_C_FLAG;
		ui_fx = phase_dir(&ctx);
		RunTR((&MY_23_TIMER1->tMaxTimeRelay[2]),ui_fx);
		pelm->aioOut[_PR_23_C_DIR].bVal = MY_23_TIMER1->tMaxTimeRelay[2].boolTrip;
    }
	else
	{
		ResetTR(&MY_23_TIMER1->tMaxTimeRelay[0]);
		ResetTR(&MY_23_TIMER1->tMaxTimeRelay[1]);
		ResetTR(&MY_23_TIMER1->tMaxTimeRelay[2]);
	}

    if(pelm->ppioIn[_PR_23_I0_ENABLE].bVal)
	{
		if(!pelm->ppioIn[_PR_23_3U0].bVal)
		{
		   if((pCfg->mmi_meaNo_ai[MMI_MEA_I0]<0)||(pCfg->mmi_meaNo_ai[MMI_MEA_U0]<0))
		   	  pelm->aioOut[_PR_23_N_DIR].bVal =FALSE;
		   else
		   {
		      ctx.I0 = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_I0]].UI;
		      ctx.U0 = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_U0]].UI;
		       ui_fx = zero_dir(&ctx);
		       RunTR((&MY_23_TIMER1->tMaxTimeRelay[3]),ui_fx);
		       pelm->aioOut[_PR_23_N_DIR].bVal = MY_23_TIMER1->tMaxTimeRelay[3].boolTrip;
		   }
		}
		else if(!pelm->ppioIn[_PR_23_VVMODE].bVal)
		{
		   ctx.I0 = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_IA]].SUI0;
		   ctx.U0 = aiProtCal[pCfg->mmi_meaNo_ai[MMI_MEA_UA]].SUI0;
		   ui_fx = zero_dir(&ctx);
		   RunTR((&MY_23_TIMER1->tMaxTimeRelay[3]),ui_fx);
		   pelm->aioOut[_PR_23_N_DIR].bVal = MY_23_TIMER1->tMaxTimeRelay[3].boolTrip;
		}
		else
		{
		   pelm->aioOut[_PR_23_N_DIR].bVal =FALSE;
		}
    }
	else
	{
		ResetTR(&MY_23_TIMER1->tMaxTimeRelay[3]);
	}
}

int PRLIB_23(EP_ELEMENT *pelm)
{
   MY_23_A_FLAG = 0;
   MY_23_B_FLAG = 0;
   MY_23_C_FLAG = 0;
		
   pelm->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
   pelm->ppioIn = (VALUE_TYPE*)malloc(_PR_23_INPUT*sizeof(VALUE_TYPE));
   pelm->aioOut = (VALUE_TYPE*)malloc(_PR_23_OUTPUT*sizeof(VALUE_TYPE));

   pelm->aioOut[_PR_23_A_DIR].bVal = FALSE;
   pelm->aioOut[_PR_23_B_DIR].bVal = FALSE;
   pelm->aioOut[_PR_23_C_DIR].bVal = FALSE;
   pelm->aioOut[_PR_23_N_DIR].bVal = FALSE;

   pelm->Scan_Func = PRLIB_23_SCAN;
   
   return 0;
}

#define MY_24_TIMER1   ((TIMERELAY *)(pelm->apvUser[0]))
void PRLIB_24_SCAN(EP_ELEMENT *pelm)
{
	DWORD dU0;

    if (pRunSet->bSetChg)
	{
		InitTR(MY_24_TIMER1, pRunSet->dTPt, 0);
		SetTR_TRTT(MY_24_TIMER1, 0);
	}
	if (!pelm->ppioIn[_PR_24_ENABLE].bVal)
	{
		ResetTR(MY_24_TIMER1);
		pelm->aioOut[_PR_24_PICKUP].bVal=FALSE;
		pelm->aioOut[_PR_24_OPERATE].bVal=FALSE;
		return;
	}

	dU0 = pVal->dU[3];

	if(!pelm->ppioIn[_PR_24_VVMODE].bVal)
	{
	   if(dU0 < pRunSet->dU0_8V)
	   {
		  BOOL bWy,bYl;
		  bWy = bYl = FALSE;

		  if((pVal->dU[0] < pRunSet->dU_16V[0]>>1)&&
		  	 (pVal->dU[1] < pRunSet->dU_16V[1]>>1)&&
		  	 (pVal->dU[2] < pRunSet->dU_16V[2]>>1))
		  	 bWy = TRUE;
		  if((pVal->dI[0] > pRunSet->dIMin[0])||
		  	 (pVal->dI[1] > pRunSet->dIMin[1])||
		  	 (pVal->dI[2] > pRunSet->dIMin[2]))
		  	 bYl =  TRUE;
		  if(bWy && bYl)
		  	 pelm->aioOut[_PR_24_PICKUP].bVal = TRUE;
		  else
		  	 pelm->aioOut[_PR_24_PICKUP].bVal = FALSE;
		  
	   }
	   else
	   {
	       DWORD dUmax,dUmin,dU16;
	   
		   dUmax = dUmin = pVal->dUxx[0];
		   if(pVal->dUxx[1] > dUmax)
		   	 dUmax = pVal->dUxx[1];
		   else
		   	 dUmin = pVal->dUxx[1];
		   if(pVal->dUxx[2] > dUmax)
		   	 dUmax = pVal->dUxx[2];
		   else if(pVal->dUxx[2] < dUmin)
		   	 dUmin = pVal->dUxx[2];

           dU16 = (pRunSet->dU_16V[0]+pRunSet->dU_16V[1]+pRunSet->dU_16V[2])/3;
		   if(dUmin < dU16)
		   	  pelm->aioOut[_PR_24_PICKUP].bVal = TRUE;
		   else if((dUmax - dUmin) > dU16)
		   	  pelm->aioOut[_PR_24_PICKUP].bVal = TRUE;
		   else
		   	  pelm->aioOut[_PR_24_PICKUP].bVal = FALSE;
		   
	   }
	}
	else
	{
		 BOOL bUab,bUbc,bUca;
		 if(dU0 > pRunSet->dU0_8V)
		 {
		     pelm->aioOut[_PR_24_PICKUP].bVal = TRUE;
		 }
		 else
		 {
		     bUab = (pVal->dUxx[0] > pRunSet->dUxxMin[0])?TRUE:FALSE;
			 bUbc = (pVal->dUxx[1] > pRunSet->dUxxMin[1])?TRUE:FALSE;
			 bUca = (pVal->dUxx[2] > pRunSet->dUxxMin[2])?TRUE:FALSE;
			 if(bUab&&bUbc&&bUca)
				pelm->aioOut[_PR_24_PICKUP].bVal = FALSE;
			 else if(bUab||bUbc||bUca)
				pelm->aioOut[_PR_24_PICKUP].bVal = TRUE;
			 else if((pVal->dI[0] > pRunSet->dIMin[0])||
		  	         (pVal->dI[1] > pRunSet->dIMin[1])||
		  	         (pVal->dI[2] > pRunSet->dIMin[2]))
			    pelm->aioOut[_PR_24_PICKUP].bVal = TRUE;
			 else
			 	pelm->aioOut[_PR_24_PICKUP].bVal = FALSE;
		 }
	}
	RunTR(MY_24_TIMER1,pelm->aioOut[_PR_24_PICKUP].bVal);
	pelm->aioOut[_PR_24_OPERATE].bVal=MY_24_TIMER1->boolTrip;
	
}

int PRLIB_24(EP_ELEMENT *pelm)
{
   pelm->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
   pelm->ppioIn = (VALUE_TYPE*)malloc(_PR_24_INPUT*sizeof(VALUE_TYPE));
   pelm->aioOut = (VALUE_TYPE*)malloc(_PR_24_OUTPUT*sizeof(VALUE_TYPE));

   pelm->aioOut[_PR_24_PICKUP].bVal = FALSE;
   pelm->aioOut[_PR_24_OPERATE].bVal = FALSE;

   pelm->Scan_Func = PRLIB_24_SCAN;
   
   return 0;
}

#define MY_25_TIMER1   ((TIMERELAY *)(pelm->apvUser[0]))
void PRLIB_25_SCAN(EP_ELEMENT *pelm)
{
	BOOL  bGy;
	int   i;
	DWORD udz;
    if (pRunSet->bSetChg)
	{		
		InitTR(MY_25_TIMER1, pRunSet->dTGy, 0);
		SetTR_TRTT(MY_25_TIMER1, 0);
	}

	bGy = FALSE;
	for(i=0; i<3; i++)
	{
	   if((pelm->ppioIn[_PR_25_UAB+i].ulVal > pRunSet->dUYy[i])&&
	   	   pRunSet->bPTU[i])
	   {
	       bGy = TRUE;
		   break;
	   }
	}
	pelm->aioOut[_PR_25_YY].bVal = bGy;
	
	if (!pelm->ppioIn[_PR_25_ENABLE].bVal)
	{
		ResetTR(MY_25_TIMER1);
		pelm->aioOut[_PR_25_PICKUP].bVal=FALSE;
		pelm->aioOut[_PR_25_OPERATE].bVal=FALSE;
		return;
	}

    bGy = FALSE;
	for(i=0; i<3; i++)
	{
	   if(pelm->aioOut[_PR_25_OPERATE].bVal)
		   udz = pRunSet->dUGyRet[i];
	   else
	   	   udz = pRunSet->dUGy[i];

	   if((pelm->ppioIn[_PR_25_UAB+i].ulVal > udz)&&
	   	   pRunSet->bPTU[i])
	   {
	       bGy = TRUE;
		   break;
	   }
	}
	pelm->aioOut[_PR_25_PICKUP].bVal = bGy;
	if(pelm->ppioIn[_PR_25_BLOCKED].bVal)
	{
	   ResetTR(MY_25_TIMER1);
	   pelm->aioOut[_PR_25_OPERATE].bVal = FALSE;
	}
	RunTR(MY_25_TIMER1, bGy);
	pelm->aioOut[_PR_25_OPERATE].bVal = MY_25_TIMER1->boolTrip;
	
}

int PRLIB_25(EP_ELEMENT *pelm)
{
   pelm->apvUser[0] = (TIMERELAY*)malloc(sizeof(TIMERELAY));
   pelm->ppioIn = (VALUE_TYPE*)malloc(_PR_25_INPUT*sizeof(VALUE_TYPE));
   pelm->aioOut = (VALUE_TYPE*)malloc(_PR_25_OUTPUT*sizeof(VALUE_TYPE));

   pelm->aioOut[_PR_25_PICKUP].bVal = FALSE;
   pelm->aioOut[_PR_25_OPERATE].bVal = FALSE;
   pelm->Scan_Func = PRLIB_25_SCAN;
   
   return 0;
}

/*重合闸*/
enum{
	T1C=0,T2C,T3C,T4C,CHARGE_T,REC_FAIL_T,NEXT_OPEN_T,T1S
};

#define MY_26_TIMER  ((TMAXTIMERELAY *)(pelm->apvUser[0]))
#define MY_26_RECNUM (pelm->aulUser[0])
#define MY_26_TRIP   (pelm->aulUser[1])
#define MY_26_CYCLE  (pelm->aulUser[2])
#define b1sFlag  (pelm->aulUser[3])
#define bTripFlag  (pelm->aulUser[4])

/*信号复归，则重合逻辑充电时间按0计算，为测试方便*/
static void PRLIB_26_SCAN(EP_ELEMENT *pelm)
{
    DWORD dRetT;
	BOOL bCharge,bLastTrip,bPickUp,bOperate,bDisCharge;
	BOOL bFlag;

	if(pRunSet->bSetChg||pelm->bSetChg)
	{
		InitTR(&(MY_26_TIMER->tMaxTimeRelay[T1C]),pelm->ppioIn[_PR_26_T_CH].ulVal,0);
		InitTR(&(MY_26_TIMER->tMaxTimeRelay[T2C]),pelm->ppioIn[_PR_26_T_CH2].ulVal,0);
		InitTR(&(MY_26_TIMER->tMaxTimeRelay[T3C]),pelm->ppioIn[_PR_26_T_CH3].ulVal,0);
		InitTR(&(MY_26_TIMER->tMaxTimeRelay[T4C]),pelm->ppioIn[_PR_26_T_CH4].ulVal,0);
	 	InitTR(&(MY_26_TIMER->tMaxTimeRelay[REC_FAIL_T]),0,0);
		InitTR(&(MY_26_TIMER->tMaxTimeRelay[T1S]),10000,0);
	    if(pRunSet->bCd)
		    InitTR(&(MY_26_TIMER->tMaxTimeRelay[CHARGE_T]),150000,0);
		else
			InitTR(&(MY_26_TIMER->tMaxTimeRelay[CHARGE_T]),0,0);
		
		SetTR_TRTT(&(MY_26_TIMER->tMaxTimeRelay[T1C]),0);
		SetTR_TRTT(&(MY_26_TIMER->tMaxTimeRelay[T2C]),0);
		SetTR_TRTT(&(MY_26_TIMER->tMaxTimeRelay[T3C]),0);
		SetTR_TRTT(&(MY_26_TIMER->tMaxTimeRelay[T4C]),0);
		SetTR_TRTT(&(MY_26_TIMER->tMaxTimeRelay[CHARGE_T]),0);
		SetTR_TRTT(&(MY_26_TIMER->tMaxTimeRelay[REC_FAIL_T]),0);
		SetTR_TRTT(&(MY_26_TIMER->tMaxTimeRelay[T1S]),0);	

		pelm->bSetChg = FALSE;
		MY_26_RECNUM = 0;
		MY_26_CYCLE = 0;

	}


	RunTR(&(MY_26_TIMER->tMaxTimeRelay[T1S]), TRUE);/* 1S定时器 */
	if(MY_26_TIMER->tMaxTimeRelay[T1S].boolTrip)
	{
	   ResetTR(&(MY_26_TIMER->tMaxTimeRelay[T1S]));
	   b1sFlag = !b1sFlag;
	}
	if((pelm->ppioIn[_PR_26_ENABLE].bVal==FALSE)||
	   (pelm->ppioIn[_PR_26_FDTJ].bVal))
    {
		MY_26_TRIP=0xa5a5;
		bTripFlag=FALSE;
		if(pelm->ppioIn[_PR_26_ENABLE].bVal==FALSE)
		   b1sFlag = FALSE;
		pelm->aioOut[_PR_26_CD_PICK].bVal =FALSE;
		pelm->aioOut[_PR_26_QD_PICK].bVal =FALSE;
		pelm->aioOut[_PR_26_FD_PICK].bVal =FALSE;
		pelm->aioOut[_PR_26_DZ_PICK].bVal =FALSE;
		pelm->aioOut[_PR_26_CH1_OPERATE].bVal =FALSE;
		pelm->aioOut[_PR_26_CH2_OPERATE].bVal =FALSE;
		pelm->aioOut[_PR_26_CH3_OPERATE].bVal =FALSE;
		pelm->aioOut[_PR_26_CH4_OPERATE].bVal =FALSE;
		pelm->aioOut[_PR_26_FAIL].bVal =FALSE;
		pelm->aioOut[_PR_26_T_1S].bVal=b1sFlag;
		ResetTR(&(MY_26_TIMER->tMaxTimeRelay[T1C]));
		ResetTR(&(MY_26_TIMER->tMaxTimeRelay[T2C]));
		ResetTR(&(MY_26_TIMER->tMaxTimeRelay[T3C]));
		ResetTR(&(MY_26_TIMER->tMaxTimeRelay[T4C]));
		ResetTR(&(MY_26_TIMER->tMaxTimeRelay[CHARGE_T]));
		ResetTR(&(MY_26_TIMER->tMaxTimeRelay[REC_FAIL_T]));
		ResetTR(&(MY_26_TIMER->tMaxTimeRelay[NEXT_OPEN_T]));
		
		MY_26_RECNUM=0;
		MY_26_CYCLE=0;

		return;
    }
	bDisCharge=FALSE;
	if(pelm->ppioIn[_PR_26_CDTJ].bVal)
		bCharge=TRUE;
	else
		bCharge=FALSE;
	if(pelm->ppioIn[_PR_26_QDTJ].bVal)
		bPickUp=TRUE;
	else
		bPickUp=FALSE;
	if(pelm->ppioIn[_PR_26_DZTJ].bVal)
		bOperate=TRUE;
	else
		bOperate=FALSE;
	if(!(pelm->aioOut[_PR_26_CD_PICK].bVal))/* 重合闸未充电满 */
		RunTR(&(MY_26_TIMER->tMaxTimeRelay[CHARGE_T]), bCharge&&(!bDisCharge));/* 充电灯 */
	bFlag=(!(MY_26_TIMER->tMaxTimeRelay[CHARGE_T].boolTrip)&&bPickUp&&(!bOperate));
	RunTR(&(MY_26_TIMER->tMaxTimeRelay[REC_FAIL_T]),bFlag);

	if((MY_26_RECNUM==0)&&(MY_26_CYCLE==0))
       bLastTrip=false;
#if(TYPE_USER == USER_MULTIMODE)
	else if ((MY_26_RECNUM==0)&&(MY_26_CYCLE == 1))
	    bLastTrip=pelm->aioOut[_PR_26_CH4_OPERATE].bVal;
#endif
	else
	 	bLastTrip=pelm->aioOut[_PR_26_CH1_OPERATE+MY_26_RECNUM-1].bVal;
	bFlag=((MY_26_TIMER->tMaxTimeRelay[CHARGE_T].boolTrip)&&bPickUp&&bOperate);

	if(!bLastTrip)
	{	
		
		RunTR(&(MY_26_TIMER->tMaxTimeRelay[MY_26_RECNUM]),bFlag);
		//重合后，下一次重合闸定值加9秒放电重合闸
//			bTripFlag=FALSE;
		if(MY_26_TIMER->tMaxTimeRelay[MY_26_RECNUM].boolTrip)
		{
		    MY_26_RECNUM=MY_26_RECNUM+1;
	#if(TYPE_USER == USER_MULTIMODE)
			if(MY_26_RECNUM==4)
			{
				MY_26_RECNUM=0;
				MY_26_CYCLE=1;
			}
	#endif
			if(MY_26_RECNUM < 4)
			{
				
			  dRetT=pRunSet->dTCHBS+pRunSet->dTCHBAY+(pelm->ppioIn[_PR_26_T_CH+MY_26_RECNUM].ulVal);
			  InitTR(&(MY_26_TIMER->tMaxTimeRelay[NEXT_OPEN_T]), dRetT, 0);
			  bTripFlag=TRUE;
				MY_26_TRIP=0xa5a5;
			}
		}
	}
	else
	{
	#if(TYPE_USER == USER_MULTIMODE)
	   if ((MY_26_RECNUM == 0)&&(MY_26_CYCLE == 1))
	   	 RunTR(&(MY_26_TIMER->tMaxTimeRelay[3]),bFlag);
	   else
    #endif  	
	     RunTR(&(MY_26_TIMER->tMaxTimeRelay[MY_26_RECNUM-1]),bFlag);
	}
#if(TYPE_USER != USER_MULTIMODE)
	if(MY_26_RECNUM==(pelm->ppioIn[_PR_26_NUM_CH].ulVal))/*重合次数满*/
	{
       pelm->aioOut[_PR_26_FD_PICK].bVal=TRUE;/*置放电输出*/
	   if(pRunSet->bCd)
		    InitTR(&(MY_26_TIMER->tMaxTimeRelay[CHARGE_T]),150000,0);
		else
			InitTR(&(MY_26_TIMER->tMaxTimeRelay[CHARGE_T]),20000,0);
	}
	RunTR(&(MY_26_TIMER->tMaxTimeRelay[NEXT_OPEN_T]),bTripFlag&!bPickUp);
	if(MY_26_TIMER->tMaxTimeRelay[NEXT_OPEN_T].boolTrip)
	{
		MY_26_TRIP=0x5a5a;
		bTripFlag=FALSE;
	}
	if(!(MY_26_TIMER->tMaxTimeRelay[NEXT_OPEN_T].boolTrip)&&(MY_26_TRIP==0x5a5a))
	{
		MY_26_TRIP=0xa5a5;
		pelm->aioOut[_PR_26_FD_PICK].bVal=TRUE;
		InitTR(&(MY_26_TIMER->tMaxTimeRelay[CHARGE_T]),0,0);
	}
#endif
	pelm->aioOut[_PR_26_CD_PICK].bVal=MY_26_TIMER->tMaxTimeRelay[CHARGE_T].boolTrip;
	pelm->aioOut[_PR_26_QD_PICK].bVal=(MY_26_TIMER->tMaxTimeRelay[CHARGE_T].boolTrip)&&bPickUp;
	pelm->aioOut[_PR_26_DZ_PICK].bVal=MY_26_TIMER->tMaxTimeRelay[T1C].boolTrip||MY_26_TIMER->tMaxTimeRelay[T2C].boolTrip||
								     MY_26_TIMER->tMaxTimeRelay[T3C].boolTrip||MY_26_TIMER->tMaxTimeRelay[T4C].boolTrip;
	pelm->aioOut[_PR_26_CH1_OPERATE].bVal=MY_26_TIMER->tMaxTimeRelay[T1C].boolTrip;
	pelm->aioOut[_PR_26_CH2_OPERATE].bVal=MY_26_TIMER->tMaxTimeRelay[T2C].boolTrip;
	pelm->aioOut[_PR_26_CH3_OPERATE].bVal=MY_26_TIMER->tMaxTimeRelay[T3C].boolTrip;
	pelm->aioOut[_PR_26_CH4_OPERATE].bVal=MY_26_TIMER->tMaxTimeRelay[T4C].boolTrip;
	pelm->aioOut[_PR_26_FAIL].bVal=MY_26_TIMER->tMaxTimeRelay[REC_FAIL_T].boolTrip;
	pelm->aioOut[_PR_26_T_1S].bVal=b1sFlag;

	return;
}

int PRLIB_26(EP_ELEMENT *pelm)
{
   MY_26_RECNUM = 0;
   MY_26_TRIP = 0xa5a5;
   b1sFlag=bTripFlag=FALSE;

   pelm->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
   pelm->ppioIn = (VALUE_TYPE*)malloc(_PR_26_INPUT*sizeof(VALUE_TYPE));
   pelm->aioOut = (VALUE_TYPE*)malloc(_PR_26_OUTPUT*sizeof(VALUE_TYPE));

   pelm->aioOut[_PR_26_CD_PICK].bVal = FALSE;
   pelm->aioOut[_PR_26_QD_PICK].bVal = FALSE;
   pelm->aioOut[_PR_26_DZ_PICK].bVal = FALSE;
   pelm->aioOut[_PR_26_FD_PICK].bVal = FALSE;
   pelm->aioOut[_PR_26_CH1_OPERATE].bVal = FALSE;
   pelm->aioOut[_PR_26_CH2_OPERATE].bVal = FALSE;
   pelm->aioOut[_PR_26_CH3_OPERATE].bVal = FALSE;
   pelm->aioOut[_PR_26_CH4_OPERATE].bVal = FALSE;
   pelm->aioOut[_PR_26_FAIL].bVal = FALSE;
   pelm->aioOut[_PR_26_T_1S].bVal = FALSE;

   pelm->Scan_Func =PRLIB_26_SCAN;
   return 0;
}

#define MY_27_TIMER1   ((TIMERELAY *)(pelm->apvUser[0]))
#define MY_27_TIMER2   ((TIMERELAY *)(pelm->apvUser[1]))
static void PRLIB_27_SCAN(EP_ELEMENT *pelm)
{
	BOOL   bYl, bWl;
	
    if (pRunSet->bSetChg)
	{
		InitTR(MY_27_TIMER1, pelm->ppioIn[_PR_27_T_WL].ulVal, 0);
		SetTR_TRTT(MY_27_TIMER1, 0);
		InitTR(MY_27_TIMER2, pelm->ppioIn[_PR_27_T_YL].ulVal, 0);		
	}
	if (!pelm->ppioIn[_PR_27_ENABLE].bVal)
	{
		ResetTR(MY_27_TIMER1);
		ResetTR(MY_27_TIMER2);
		pelm->aioOut[_PR_27_PICKUP].bVal=FALSE;
		pelm->aioOut[_PR_27_OPERATE].bVal=FALSE;
		pelm->aioOut[_PR_27_YL_OPERATE].bVal=FALSE;
		return;
	}

    if((pVal->dI[0] > pRunSet->dIMin[0])&&
	   (pVal->dI[1] > pRunSet->dIMin[1])&&
	   (pVal->dI[2] > pRunSet->dIMin[2]))
	   bYl = TRUE;
	else
	   bYl = FALSE;

	RunTR(MY_27_TIMER2,bYl);
	pelm->aioOut[_PR_27_YL_OPERATE].bVal=MY_27_TIMER2->boolTrip&&pelm->ppioIn[_PR_27_CTRL_YL].bVal;

#if 1
	if((pVal->dI[0] < pRunSet->dIMin[0])&&
	   (pVal->dI[1] < pRunSet->dIMin[1])&&
	   (pVal->dI[2] < pRunSet->dIMin[2]))
	{
	    bWl = TRUE;
		if(pelm->ppioIn[_PR_27_CTRL_YL].bVal)
		   bWl = MY_27_TIMER2->boolTrip;
	}
	else
	    bWl = FALSE;
#else 
    if(pVal->dI[0] < pRunSet->dIMin[0])
		bWl = TRUE;
	else
		bWl = FALSE;
#endif
	pelm->aioOut[_PR_27_PICKUP].bVal = bWl;

	RunTR(MY_27_TIMER1, bWl);
	pelm->aioOut[_PR_27_OPERATE].bVal = MY_27_TIMER1->boolTrip;
}

int PRLIB_27(EP_ELEMENT *pelm)
{
	
	pelm->apvUser[0] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->apvUser[1] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->ppioIn =(VALUE_TYPE *)malloc(_PR_27_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut =(VALUE_TYPE *)malloc(_PR_27_OUTPUT*sizeof(VALUE_TYPE));
		
	pelm->aioOut[_PR_27_PICKUP].bVal=FALSE;
	pelm->aioOut[_PR_27_OPERATE].bVal=FALSE;
	pelm->aioOut[_PR_27_YL_OPERATE].bVal=FALSE;

	pelm->Scan_Func=PRLIB_27_SCAN;
	
	return 0;	

}

#define MY_28_TIMER1   ((TIMERELAY *)(pelm->apvUser[0]))
#define MY_28_TIMER2   ((TIMERELAY *)(pelm->apvUser[1]))
#define MY_28_I_NOW    (pelm->aulUser[0])
#define MY_28_IM_NOW   (pelm->aulUser[1])
#define MY_28_I_LAST   (pelm->aulUser[2])
#define MY_28_IM_LAST  (pelm->aulUser[3])
#define MY_28_IM_START (pelm->aulUser[4])
static void PRLIB_28_SCAN(EP_ELEMENT *pelm)
{

   BOOL bQd;
   int i;
   DWORD scale;
   BOOL bHwZd;

   if(pRunSet->bSetChg)
   {
      if(pRunSet->bHw2Zd)
	  {
	    InitTR(MY_28_TIMER1, 2000, 0);
		InitTR(MY_28_TIMER2, 0, 500);
      }
	  else
	    InitTR(MY_28_TIMER1, pRunSet->dTDjqd+2000, 0);
	  SetTR_TRTT(MY_28_TIMER1, 0);
   }

   if(!pelm->ppioIn[_PR_28_ENABLE].bVal)
   {
      pelm->aioOut[_PR_28_DLQD].bVal = TRUE;
	  pelm->aioOut[_PR_28_DJQD].bVal = FALSE;
	  return;
   }
   if((pVal->dI[0] > pRunSet->dIMin[0])||
	  (pVal->dI[1] > pRunSet->dIMin[1])||
	  (pVal->dI[2] > pRunSet->dIMin[2]))
   {
      MY_28_I_NOW = true;
	  if((pVal->dI[0] > pRunSet->dIM[0])||
	     (pVal->dI[1] > pRunSet->dIM[1])||
	     (pVal->dI[2] > pRunSet->dIM[2]))
	  	  MY_28_IM_NOW = true;
	  else
	  	  MY_28_IM_NOW = false;
   }
   else
   {
        MY_28_I_NOW  = false;
		MY_28_IM_NOW = false;
   }
   bHwZd = false;
   for(i=0; i<3; i++)
   {
       if (pVal->dI[i] > 0)
          scale = pVal->dI_Hw2[i]*100/pVal->dI[i];
	   else
	   	  scale = 0;
		  
#if (TYPE_USER == USER_BEIJING)
       if(scale >= 5)
#else
	   if(scale >= 15)
#endif
	   {
	      bHwZd = true;
		  break;
	   }
   }
   bQd = FALSE;
   if(MY_28_I_NOW == false)
      pelm->aioOut[_PR_28_DLQD].bVal = FALSE;
   else if(MY_28_IM_LAST== false)
   {
	  MY_28_IM_START = true;
	  pelm->aioOut[_PR_28_DLQD].bVal = TRUE;
   }
   if(MY_28_IM_START == true)
   {
	   if((MY_28_I_NOW == false)&&(MY_28_IM_LAST == true))
	       MY_28_IM_START = false;  
	   else
	   	   bQd = TRUE;
   }
   RunTR(MY_28_TIMER1, bQd);
   RunTR(MY_28_TIMER2, bHwZd);
   if(pRunSet->bHw2Zd)
   {
      pelm->aioOut[_PR_28_DJQD].bVal =bQd&&(!MY_28_TIMER1->boolTrip||MY_28_TIMER2->boolTrip);
   }
   else
   {
      pelm->aioOut[_PR_28_DJQD].bVal = bQd&&!MY_28_TIMER1->boolTrip;
   }
   if(!pelm->aioOut[_PR_28_DJQD].bVal)
   {
      MY_28_IM_START = false;
   }
   MY_28_I_LAST  = MY_28_I_NOW;
   MY_28_IM_LAST = MY_28_IM_NOW;
}

int PRLIB_28(EP_ELEMENT *pelm)
{
	
	pelm->apvUser[0] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->apvUser[1] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->ppioIn =(VALUE_TYPE *)malloc(_PR_28_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut =(VALUE_TYPE *)malloc(_PR_28_OUTPUT*sizeof(VALUE_TYPE));
		
	pelm->aioOut[_PR_28_DLQD].bVal=FALSE;
	pelm->aioOut[_PR_28_DJQD].bVal=FALSE;

	pelm->Scan_Func=PRLIB_28_SCAN;
	
	return 0;	

}

#define MY_29_TIMER1   ((TIMERELAY *)(pelm->apvUser[0]))
static void PRLIB_29_SCAN(EP_ELEMENT *pelm)
{
    BOOL bNoU,bNoI;
	int i;

    if(pRunSet->bSetChg)
    {
	   InitTR(MY_29_TIMER1, pRunSet->dTWY, 0);
	   SetTR_TRTT(MY_29_TIMER1, 0);

    }
	
    if(!pelm->ppioIn[_PR_29_ENABLE].bVal)
    {
	   ResetTR(MY_29_TIMER1);
	   pelm->aioOut[_PR_29_OPERATE].bVal = FALSE;
	   return;
    }
	
	bNoU = TRUE;

    for(i=0; i<3; i++)
    {
        if((pelm->ppioIn[_PR_29_UAB+i].ulVal > pRunSet->dUxxMin[0]) && pRunSet->bPTU[i])
			bNoU = FALSE;
    }

	bNoI = pelm->ppioIn[_PR_29_WL].bVal;
	RunTR(MY_29_TIMER1, bNoU&bNoI);
	pelm->aioOut[_PR_29_OPERATE].bVal = MY_29_TIMER1->boolTrip;
}

int PRLIB_29(EP_ELEMENT *pelm)
{
	
	pelm->apvUser[0] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->ppioIn =(VALUE_TYPE *)malloc(_PR_29_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut =(VALUE_TYPE *)malloc(_PR_29_OUTPUT*sizeof(VALUE_TYPE));
		
	pelm->aioOut[_PR_29_OPERATE].bVal=FALSE;

	pelm->Scan_Func=PRLIB_29_SCAN;
	
	return 0;	

}

#ifdef INCLUDE_PR_PRO
#define MY_30_TIMER1   ((TIMERELAY *)(pelm->apvUser[0]))
static void PRLIB_30_SCAN(EP_ELEMENT *pelm)
{
    BOOL bDf = FALSE;
	BOOL bDy = FALSE;
	BOOL bI  = FALSE;
	
    if (pPublicSet->bSetChg)
    {
	   InitTR(MY_30_TIMER1, pPublicSet->dTDf, 0);
	   SetTR_TRTT(MY_30_TIMER1, 0);
    }

	if (!pelm->ppioIn[_PR_30_TIME].bVal)
		return;
	
    if (!pelm->ppioIn[_PR_30_ENABLE].bVal)
    {
	   ResetTR(MY_30_TIMER1);
	   pelm->aioOut[_PR_30_PICK].bVal = FALSE;
	   pelm->aioOut[_PR_30_OPERATE].bVal = FALSE;
	   return;
    }
    
	if (pelm->ppioIn[_PR_30_FREQ].ulVal < pPublicSet->dFreq)
	{
	    if (pPublicSet->bDfJl && (pelm->ppioIn[_PR_30_FSLIP].ulVal > pPublicSet->dFSlip))
			bDf = TRUE;
		else if (!pPublicSet->bDfJl &&(pelm->ppioIn[_PR_30_FSLIP].ulVal < pPublicSet->dFSlip))
		    bDf = TRUE;
			
	}

	if (pelm->ppioIn[_PR_30_FREQ].ulVal < pPublicSet->dFreqMin)
		bDf = FALSE;

	if (pelm->ppioIn[_PR_30_UAB].ulVal < pPublicSet->dUFreq[0])
		bDy = TRUE;
		

    if (pelm->ppioIn[_PR_30_IA].ulVal > pPublicSet->dIFreq[0])
	   bI = TRUE;
	
    pelm->aioOut[_PR_30_PICK].bVal = bDf;
	if(pelm->ppioIn[_PR_30_BLOCK].bVal||bDy||((!bI) & pPublicSet->bDfIBs))
		bDf = FALSE;

	RunTR(MY_30_TIMER1, bDf);
	pelm->aioOut[_PR_30_OPERATE].bVal = MY_30_TIMER1->boolTrip;
}

int PRLIB_30(EP_ELEMENT *pelm)
{
    pelm->apvUser[0] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->ppioIn =(VALUE_TYPE *)malloc(_PR_30_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut =(VALUE_TYPE *)malloc(_PR_30_OUTPUT*sizeof(VALUE_TYPE));
		
	pelm->aioOut[_PR_30_OPERATE].bVal=FALSE;

	pelm->Scan_Func=PRLIB_30_SCAN;
	
	return 0;
}

static void PRLIB_31_SCAN(EP_ELEMENT *pelm)
{
 	long angle,angle1,angle2;
	long udiff,fdiff,fsdiff,t;
	long un_udiff,un_fdiff,un_fsdiff;
	
    if (!pelm->ppioIn[_PR_31_ENABLE].bVal)
    {
	   pelm->aioOut[_PR_31_OPERATE].bVal = FALSE;
	   return;
    }
	
    if (!pelm->ppioIn[_PR_31_UENABLE].bVal)
    {
	   pelm->aioOut[_PR_31_OPERATE].bVal = FALSE;
	   return;
    }
	
	if (!pelm->ppioIn[_PR_31_PRETQ].bVal)
	{
	  
		angle1 = F_ANG(pTqVal->cTqU[0], pTqVal->dTqU[0]);
		angle2 = F_ANG(pTqVal->cTqU[1], pTqVal->dTqU[1]);
		angle = angle1 - angle2;

		if (angle > 11796480)
			angle -= 23592960;
		if (angle < -11796480)
			angle += 23592960;

		if (angle < 0)
		    angle = -angle; 

		if (angle < pPublicSet->dTqDqAngle)
			pelm->aioOut[_PR_31_OPERATE].bVal = TRUE;
		else
			pelm->aioOut[_PR_31_OPERATE].bVal = FALSE;
				
	}
	else
	{
	    udiff = pTqVal->dTqU[0] - pTqVal->dTqU[1];
	    un_udiff = (udiff > 0) ? udiff : -udiff;
		if (un_udiff > pPublicSet->dTqUDiff)
		{
		   pelm->aioOut[_PR_31_OPERATE].bVal = FALSE;
		   return;
		}
		fdiff = pTqVal->wTqFreq[0] - pTqVal->wTqFreq[1];
		un_fdiff = (fdiff > 0) ? fdiff : -fdiff;
		if (un_fdiff > pPublicSet->dTqFDiff)
		{
		   pelm->aioOut[_PR_31_OPERATE].bVal = FALSE;
		   return;
		}
		fsdiff = pTqVal->wTqFSlip[0] - pTqVal->wTqFSlip[1];
		un_fsdiff = (fsdiff > 0) ? fsdiff : -fsdiff;
		if (un_fsdiff > pPublicSet->dTqFsDiff)
		{
		   pelm->aioOut[_PR_31_OPERATE].bVal = FALSE;
		   return;
		}
		t = pPublicSet->dTqTAng/10;
		angle = 360*fdiff*t/100+ 180*fsdiff*t*t/10000;
		angle1 = F_ANG(pTqVal->cTqU[0], pTqVal->dTqU[0])>>8;
		angle2 = F_ANG(pTqVal->cTqU[1], pTqVal->dTqU[1])>>8;

		angle1 = ((angle1 - angle2)*1000)>>8;
		pelm->aioOut[_PR_31_ANGLE1].lVal = angle1;

		angle = angle - angle1;     //扩大100倍
        
		if (angle > 18000)
			angle -= 36000;
		if (angle < -18000)
			angle += 36000;

		if (angle < 0)
		    angle = -angle; 

		pelm->aioOut[_PR_31_ANGLE2].lVal = angle;

		if (angle < 1500)
		    pelm->aioOut[_PR_31_OPERATE].bVal = TRUE;
		else
			pelm->aioOut[_PR_31_OPERATE].bVal = FALSE;	
		
	}
 
}

int PRLIB_31(EP_ELEMENT *pelm)
{
 	pelm->ppioIn =(VALUE_TYPE *)malloc(_PR_31_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut =(VALUE_TYPE *)malloc(_PR_31_OUTPUT*sizeof(VALUE_TYPE));
		
	pelm->aioOut[_PR_31_OPERATE].bVal=FALSE;

	pelm->Scan_Func=PRLIB_31_SCAN;
	
	return 0;
}

#endif

/*单测有压合闸，X闭锁，双压闭锁*/
enum{ZT, USXT, UFXT, USXBS, UFXBS, USXJS, UFXJS,DU};
#define MY_121_TIMER1   ((TMAXTIMERELAY*)(pelm->apvUser[0]))
#define MY_121_S_BLOCK  (pelm->aulUser[0])
#define MY_121_F_BLOCK  (pelm->aulUser[1])
#define MY_121_DU_BLOCK (pelm->aulUser[2])

void PRLIB_121_SCAN(EP_ELEMENT *pelm)
{
   BOOL bSingleUS,bSingleUF, bDoubleU, bNoU;
   BOOL bUS1,bUS2,bUF1,bUF2, bX,bXF,bXS;

   if (pPublicSet->bSetChg)
   {
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[ZT]), pPublicSet->dTWY, 0);
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[ZT]), 0);
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[DU]), pPublicSet->dTSY2, 0);
		 
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[USXT]), pPublicSet->dTX, 0);
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[USXT]), 2*pPublicSet->dTWY);
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]), pPublicSet->dTX, 0);
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]), 2*pPublicSet->dTWY);

		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[USXBS]), pPublicSet->dTX, 0);
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[USXBS]), 2*pPublicSet->dTWY);
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXBS]), pPublicSet->dTX, 0);
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[UFXBS]), 2*pPublicSet->dTWY);
		
#ifdef LW
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[USXJS]), 0, 0);
#else
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[USXJS]), pPublicSet->dTX, 0);
#endif
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[USXJS]), 0);
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXJS]), pPublicSet->dTX, 0);
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[UFXJS]), 0);

		if(pelm->ppioIn[_PR_121_XBS_FX].ulVal == 1)
			MY_121_S_BLOCK = 1;
		if(pelm->ppioIn[_PR_121_XBS_FX].ulVal == 2)
			MY_121_F_BLOCK = 1;
   }
	 if(pelm->bSetChg)
	 {
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[USXT]), pelm->ppioIn[_PR_121_T_ZSYHZ].ulVal , 0);
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[USXT]), 2*pPublicSet->dTWY);
		InitTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]), pelm->ppioIn[_PR_121_T_ZSYHZ].ulVal , 0);
		SetTR_TRTT(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]), 2*pPublicSet->dTWY);
		pelm->bSetChg = FALSE;
	 }
	 
   if (!pelm->ppioIn[_PR_121_ENABLE].bVal|!pelm->ppioIn[_PR_121_QD].bVal)
   {
		ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[ZT]));
		ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[USXT]));
		ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]));
		ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[USXBS]));
		ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXBS]));
		ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[USXJS]));
		ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXJS]));
		ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[DU]));
		
		pelm->aioOut[_PR_121_OPERATE].bVal=FALSE;

		if(!pelm->ppioIn[_PR_121_ENABLE].bVal)
		{
			pelm->aioOut[_PR_121_X_BLOCK].bVal = FALSE;
		  	MY_121_F_BLOCK = MY_121_S_BLOCK = 0;
			
		}
		else
		{
			pelm->aioOut[_PR_121_X_BLOCK].bVal = (MY_121_S_BLOCK|MY_121_F_BLOCK)&pPublicSet->bXBS;
		   if (MY_121_F_BLOCK)
		 	  pelm->aioOut[_PR_121_BLOCK_FX].ulVal = 2;
		   if (MY_121_S_BLOCK)
		  	 pelm->aioOut[_PR_121_BLOCK_FX].ulVal = 1; 
			
		}
		pelm->aioOut[_PR_121_U_BLOCK].bVal=FALSE;
		MY_121_DU_BLOCK=0;
		return;
   }

   bUS1 = bUF1 = bUS2 = bUF2 = FALSE;
   if(pelm->ppioIn[_PR_121_US].ulVal > pelm->ppioIn[_PR_121_US1_PICK].ulVal)
   	  bUS1 = TRUE;
   else if(pelm->ppioIn[_PR_121_US].ulVal < pelm->ppioIn[_PR_121_US2_PICK].ulVal)
   	  bUS2 = TRUE;
   if(pelm->ppioIn[_PR_121_UF].ulVal > pelm->ppioIn[_PR_121_UF1_PICK].ulVal)
   	  bUF1 = TRUE;
   else if(pelm->ppioIn[_PR_121_UF].ulVal < pelm->ppioIn[_PR_121_UF2_PICK].ulVal)
   	  bUF2 = TRUE;

 //  if(bUS2&bUF2)
 //  {
  //    pelm->aioOut[_PR_121_FX].bVal = 0;
  // }
   	
   bNoU = bUS2&bUF2;
   bDoubleU = bUS1&bUF1;
   bSingleUS = bUS1&bUF2;
   bSingleUF = bUS2&bUF1;

   RunTR(&(MY_121_TIMER1->tMaxTimeRelay[ZT]), bNoU);
   RunTR(&(MY_121_TIMER1->tMaxTimeRelay[USXJS]), bUS1);
   RunTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXJS]), bUF1);

   if(MY_121_TIMER1->tMaxTimeRelay[ZT].boolTrip)
   {
     if(MY_121_TIMER1->tMaxTimeRelay[USXBS].boolStart&!MY_121_TIMER1->tMaxTimeRelay[USXBS].boolTrip)
        MY_121_S_BLOCK = 1;
	 if(MY_121_TIMER1->tMaxTimeRelay[UFXBS].boolStart&!MY_121_TIMER1->tMaxTimeRelay[UFXBS].boolTrip)
	 	MY_121_F_BLOCK = 1;
	 ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[USXT]));
	 ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]));
	 ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[USXBS]));
	 ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXBS]));
#if (TYPE_USER != USER_BEIJING) 
	 pelm->aioOut[_PR_121_U_BLOCK].bVal = FALSE;
#endif
   }

   if(MY_121_TIMER1->tMaxTimeRelay[USXJS].boolTrip)
   {
     if (MY_121_S_BLOCK == 1)                        // for
	 {
	     pRunPublic->bs_type &= ~HZ_BS_SR;
	     pRunPublic->js_type |= HZ_JS_DY;

	 }
     MY_121_S_BLOCK = 0;
   }
   if(MY_121_TIMER1->tMaxTimeRelay[UFXJS].boolTrip)
   {
     if (MY_121_F_BLOCK == 1)
	 {
	    pRunPublic->bs_type &= ~HZ_BS_FR;
	    pRunPublic->js_type |= HZ_JS_DY;
	 }
     MY_121_F_BLOCK = 0;
   }
  
   pelm->aioOut[_PR_121_X_BLOCK].bVal = (MY_121_S_BLOCK|MY_121_F_BLOCK)&pPublicSet->bXBS;

   if (MY_121_F_BLOCK)
 	  pelm->aioOut[_PR_121_BLOCK_FX].ulVal = 2;
   if (MY_121_S_BLOCK)
  	 pelm->aioOut[_PR_121_BLOCK_FX].ulVal = 1; //modify by lijun 2017-03-17
   	 

   bXS = (MY_121_TIMER1->tMaxTimeRelay[USXT].boolStart&!MY_121_TIMER1->tMaxTimeRelay[USXT].boolTrip);
   bXF = (MY_121_TIMER1->tMaxTimeRelay[UFXT].boolStart&!MY_121_TIMER1->tMaxTimeRelay[UFXT].boolTrip);
   bX = bXS| bXF;
   if(bXS) pelm->aioOut[_PR_121_X_FX].ulVal = 1;
   if(bXF) pelm->aioOut[_PR_121_X_FX].ulVal = 2;
   if(!bX) pelm->aioOut[_PR_121_X_FX].ulVal = 0;
 
 #if (TYPE_USER == USER_SHANDONG) 
 	if(bDoubleU&bX)
 #else  
   if(bDoubleU)
 #endif
   	 MY_121_DU_BLOCK = 1;
   else if(!bDoubleU)
   	 MY_121_DU_BLOCK = 0;
	 
   RunTR(&(MY_121_TIMER1->tMaxTimeRelay[DU]), MY_121_DU_BLOCK);
	 
   if (MY_121_TIMER1->tMaxTimeRelay[DU].boolTrip&pPublicSet->bDUBS)
      pelm->aioOut[_PR_121_U_BLOCK].bVal = TRUE;
#if ((TYPE_USER != USER_BEIJING) && (TYPE_USER != USER_SHANDONG))
   else
	   pelm->aioOut[_PR_121_U_BLOCK].bVal = FALSE;
#endif

		if (bX && !MY_121_TIMER1->tMaxTimeRelay[ZT].boolTrip)
		{
			if(!bSingleUS)
				SetTRHold(&(MY_121_TIMER1->tMaxTimeRelay[USXT]), 0);
			if(!bSingleUF)
				SetTRHold(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]), 0);
		}
		if(bX  && MY_121_TIMER1->tMaxTimeRelay[ZT].boolTrip)
		{
			SetTRHold(&(MY_121_TIMER1->tMaxTimeRelay[USXT]), 1);
			SetTRHold(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]), 1);
		}

	
   RunTR(&(MY_121_TIMER1->tMaxTimeRelay[USXT]), bSingleUS);
   RunTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]), bSingleUF);

   RunTR(&(MY_121_TIMER1->tMaxTimeRelay[USXBS]), bSingleUS);
   RunTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXBS]), bSingleUF);
   
	 if(pelm->aioOut[_PR_121_X_BLOCK].bVal||
   	  pelm->aioOut[_PR_121_U_BLOCK].bVal||
   	  pelm->ppioIn[_PR_121_BLOCK].bVal)
   {
		 if(!(pRunPublic->dw_NVRAM_BS & (PR_XBS_S | PR_YBS_S)))
      ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[USXT]));
		 if(!(pRunPublic->dw_NVRAM_BS & (PR_XBS_F | PR_YBS_F)))
			ResetTR(&(MY_121_TIMER1->tMaxTimeRelay[UFXT]));

      pelm->aioOut[_PR_121_OPERATE].bVal = FALSE;
      return;
   }
	 
   pelm->aioOut[_PR_121_OPERATE].bVal = MY_121_TIMER1->tMaxTimeRelay[USXT].boolTrip|MY_121_TIMER1->tMaxTimeRelay[UFXT].boolTrip;
   if(pelm->aioOut[_PR_121_OPERATE].bVal)
   {
      if(bSingleUS)
        pelm->aioOut[_PR_121_FX].bVal = 1;
	  else
	  	pelm->aioOut[_PR_121_FX].bVal = 2;

   }
}

int PRLIB_121(EP_ELEMENT *pelm)
{
	
	pelm->apvUser[0] =(TMAXTIMERELAY *)malloc(sizeof(TMAXTIMERELAY));
	pelm->ppioIn  =(VALUE_TYPE *)malloc(_PR_121_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut  =(VALUE_TYPE *)malloc(_PR_121_OUTPUT*sizeof(VALUE_TYPE));

	pelm->ppioIn[_PR_121_XBS_FX].ulVal = 0;
		
	pelm->aioOut[_PR_121_OPERATE].bVal=FALSE;
	pelm->aioOut[_PR_121_X_BLOCK].bVal=FALSE;
	pelm->aioOut[_PR_121_U_BLOCK].bVal=FALSE;
	pelm->aioOut[_PR_121_FX].bVal=FALSE;
	pelm->aioOut[_PR_121_X_FX].bVal = FALSE;
	pelm->aioOut[_PR_121_BLOCK_FX].bVal = FALSE;
  MY_121_S_BLOCK=MY_121_F_BLOCK=MY_121_DU_BLOCK=0;

	pelm->Scan_Func=PRLIB_121_SCAN;
	
	return 0;	
}
/*合闸闭锁,分闸闭锁*/
enum{UYT=1, USYJS, UFYJS, FZBS1, FZBS2};
#define MY_122_TIMER1  ((TMAXTIMERELAY*)(pelm->apvUser[0]))
#define MY_122_GL      (pelm->aulUser[0])
#define MY_122_HZ      (pelm->aulUser[1])
#define MY_122_FX      (pelm->aulUser[2])
#define MY_122_S_BLOCK (pelm->aulUser[3])
#define MY_122_F_BLOCK (pelm->aulUser[4])
#define MY_122_SU0_BLOCK (pelm->aulUser[5])
#define MY_122_FU0_BLOCK (pelm->aulUser[6])
void PRLIB_122_SCAN(EP_ELEMENT *pelm)
{
   BOOL bUS1, bUF1, bUS2, bUF2, bY;
   BOOL bDoubleU;

   if (pPublicSet->bSetChg)
   {
		InitTR(&(MY_122_TIMER1->tMaxTimeRelay[ZT]), pPublicSet->dTWY, 0);
		SetTR_TRTT(&(MY_122_TIMER1->tMaxTimeRelay[ZT]), 0);
		InitTR(&(MY_122_TIMER1->tMaxTimeRelay[UYT]), pPublicSet->dTY, 0);
		SetTR_TRTT(&(MY_122_TIMER1->tMaxTimeRelay[UYT]), 2*pPublicSet->dTWY + 2000);
#ifdef LW
		InitTR(&(MY_122_TIMER1->tMaxTimeRelay[USYJS]), 0, 0);
#else
		InitTR(&(MY_122_TIMER1->tMaxTimeRelay[USYJS]), pPublicSet->dTX, 0);
#endif
		SetTR_TRTT(&(MY_122_TIMER1->tMaxTimeRelay[USYJS]), 0);
		InitTR(&(MY_122_TIMER1->tMaxTimeRelay[UFYJS]), pPublicSet->dTX, 0);
		SetTR_TRTT(&(MY_122_TIMER1->tMaxTimeRelay[UFYJS]), 0);
		InitTR(&(MY_122_TIMER1->tMaxTimeRelay[FZBS1]), pPublicSet->dTY  + 100, 0);
		SetTR_TRTT(&(MY_122_TIMER1->tMaxTimeRelay[FZBS1]), 0);
		InitTR(&(MY_122_TIMER1->tMaxTimeRelay[FZBS2]), pPublicSet->dTFZ2, 0);
		SetTR_TRTT(&(MY_122_TIMER1->tMaxTimeRelay[FZBS2]), 0);
		if(pelm->ppioIn[_PR_122_Y_FX].ulVal == 1)
			MY_122_S_BLOCK = 1;
		if(pelm->ppioIn[_PR_122_Y_FX].ulVal == 2)
			MY_122_F_BLOCK = 1;
   }
   if (!pelm->ppioIn[_PR_122_ENABLE].bVal)
   {
		ResetTR(&(MY_122_TIMER1->tMaxTimeRelay[ZT]));
		ResetTR(&(MY_122_TIMER1->tMaxTimeRelay[UYT]));
		ResetTR(&(MY_122_TIMER1->tMaxTimeRelay[USYJS]));
		ResetTR(&(MY_122_TIMER1->tMaxTimeRelay[UFYJS]));
		ResetTR(&(MY_122_TIMER1->tMaxTimeRelay[FZBS1]));
		ResetTR(&(MY_122_TIMER1->tMaxTimeRelay[FZBS2]));
		pelm->aioOut[_PR_122_Y_BLOCK].bVal=FALSE;
		pelm->aioOut[_PR_122_FZ_BLOCK].bVal=FALSE;
		pelm->aioOut[_PR_122_U0_BLOCK].bVal=FALSE;
		MY_122_GL=MY_122_HZ=MY_122_FX=MY_122_S_BLOCK=MY_122_F_BLOCK=0;
		MY_122_SU0_BLOCK = MY_122_FU0_BLOCK = 0;
		return;
   }

   bY = MY_122_TIMER1->tMaxTimeRelay[UYT].boolStart&!MY_122_TIMER1->tMaxTimeRelay[UYT].boolTrip;

   bUS1 = bUF1 = bUS2 = bUF2 = bDoubleU = FALSE;

   if (pelm->ppioIn[_PR_122_GL].bVal&&!MY_122_GL&&bY)
   {
#if( (TYPE_USER == USER_SHANDONG) ||(TYPE_USER == USER_BEIJING))//对于电磁型开关
	 	 InitTR(&(MY_122_TIMER1->tMaxTimeRelay[ZT]), pPublicSet->dTWY, 0);
#else
      	 InitTR(&(MY_122_TIMER1->tMaxTimeRelay[ZT]), 3000, 0);
#endif
	   	MY_122_GL = 1;
   }
   if(!(pelm->ppioIn[_PR_122_GL].bVal)&&MY_122_GL)
   {
	   InitTR(&(MY_122_TIMER1->tMaxTimeRelay[ZT]), pPublicSet->dTWY, 0);
	   MY_122_GL = 0;
   }
   if(pelm->ppioIn[_PR_122_US].ulVal > pelm->ppioIn[_PR_122_US1_PICK].ulVal)
   	  bUS1 = TRUE;
   else if(pelm->ppioIn[_PR_122_US].ulVal < pelm->ppioIn[_PR_122_US2_PICK].ulVal)
   	  bUS2 = TRUE;
   if(pelm->ppioIn[_PR_122_UF].ulVal > pelm->ppioIn[_PR_122_UF1_PICK].ulVal)
   	  bUF1 = TRUE;
   else if(pelm->ppioIn[_PR_122_UF].ulVal < pelm->ppioIn[_PR_122_UF2_PICK].ulVal)
   	  bUF2 = TRUE;
 
   bDoubleU = bUS1|bUF1; //有压即可，不用判双压

   RunTR(&(MY_122_TIMER1->tMaxTimeRelay[ZT]), bUS2&bUF2);
   RunTR(&(MY_122_TIMER1->tMaxTimeRelay[USYJS]), bUS1);
   RunTR(&(MY_122_TIMER1->tMaxTimeRelay[UFYJS]), bUF1);

   if((MY_122_TIMER1->tMaxTimeRelay[ZT].boolTrip||(pPublicSet->bIProtect&&pPublicSet->bDlxbsMode&&MY_122_GL)) && bY)
   {
      if((pelm->ppioIn[_PR_122_FX].ulVal==1) && MY_122_GL)
         MY_122_F_BLOCK = 1;
	  else if(MY_122_GL)
	  	 MY_122_S_BLOCK = 1;
	  ResetTR(&(MY_122_TIMER1->tMaxTimeRelay[UYT]));
   }
   if (pelm->ppioIn[_PR_122_U0].bVal && bY)
   {
      if (pelm->ppioIn[_PR_122_FX].ulVal==1)
	  	 MY_122_FU0_BLOCK = 1;
	  else
	  	 MY_122_SU0_BLOCK = 1;
   }
   if(MY_122_TIMER1->tMaxTimeRelay[USYJS].boolTrip&&!pelm->ppioIn[_PR_122_U0].bVal&&!(pPublicSet->bIProtect&&MY_122_GL))
   {
      if (MY_122_S_BLOCK == 1)
	  {
	     pRunPublic->bs_type &= ~HZ_BS_S;
	     pRunPublic->js_type |= HZ_JS_DY;
      }
	  if (MY_122_SU0_BLOCK == 1)
	  {
	     pRunPublic->bs_type &= ~HZ_BS_U0;
	     pRunPublic->js_type |= HZ_JS_DY;
	  }
      MY_122_S_BLOCK = 0;
	  MY_122_SU0_BLOCK = 0;
   }
   if(MY_122_TIMER1->tMaxTimeRelay[UFYJS].boolTrip&&!pelm->ppioIn[_PR_122_U0].bVal&&!(pPublicSet->bIProtect&&MY_122_GL))
   {
      if (MY_122_F_BLOCK == 1)
	  {
	     pRunPublic->bs_type &= ~HZ_BS_F;
	     pRunPublic->js_type |= HZ_JS_DY;
      }
	  if (MY_122_FU0_BLOCK == 1)
	  {
	     pRunPublic->bs_type &= ~HZ_BS_U0;
	     pRunPublic->js_type |= HZ_JS_DY;
	  }
      MY_122_F_BLOCK = 0;
	  MY_122_FU0_BLOCK = 0;
   }
   pelm->aioOut[_PR_122_Y_BLOCK].bVal = (MY_122_S_BLOCK|MY_122_F_BLOCK)&pPublicSet->bYBS;
   pelm->aioOut[_PR_122_U0_BLOCK].bVal = (MY_122_SU0_BLOCK|MY_122_FU0_BLOCK);

   if(pelm->ppioIn[_PR_122_HZ_IN].bVal)
   	 ResetTR(&(MY_122_TIMER1->tMaxTimeRelay[ZT]));

   MY_122_HZ = pelm->ppioIn[_PR_122_HZ_IN].bVal|(MY_122_HZ&bDoubleU);
      
   RunTR(&(MY_122_TIMER1->tMaxTimeRelay[UYT]), MY_122_HZ);
	 
   RunTR(&(MY_122_TIMER1->tMaxTimeRelay[FZBS1]), MY_122_HZ&!pelm->ppioIn[_PR_122_FAULT].bVal);
   if(MY_122_TIMER1->tMaxTimeRelay[FZBS1].boolTrip)
   {
      pelm->aioOut[_PR_122_FZ_BLOCK].bVal = TRUE;
   }
   RunTR(&(MY_122_TIMER1->tMaxTimeRelay[FZBS2]), pelm->aioOut[_PR_122_FZ_BLOCK].bVal);
   if(MY_122_TIMER1->tMaxTimeRelay[FZBS2].boolTrip)
   {
      pelm->aioOut[_PR_122_FZ_BLOCK].bVal = FALSE;
   }

    if (MY_122_F_BLOCK)
   	  pelm->aioOut[_PR_122_BLOCK_FX].ulVal = 2;
   	if (MY_122_S_BLOCK)
   	 pelm->aioOut[_PR_122_BLOCK_FX].ulVal = 1;

}

int PRLIB_122(EP_ELEMENT *pelm)
{

	pelm->apvUser[0] =(TMAXTIMERELAY *)malloc(sizeof(TMAXTIMERELAY));
	pelm->ppioIn  =(VALUE_TYPE *)malloc(_PR_122_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut  =(VALUE_TYPE *)malloc(_PR_122_OUTPUT*sizeof(VALUE_TYPE));

	pelm->ppioIn[_PR_122_Y_FX].ulVal  = 0;
	
	pelm->aioOut[_PR_122_Y_BLOCK].bVal=FALSE;
	pelm->aioOut[_PR_122_FZ_BLOCK].bVal=FALSE;
	pelm->aioOut[_PR_122_U0_BLOCK].bVal=FALSE;
	pelm->aioOut[_PR_122_BLOCK_FX].bVal=FALSE;
	
	MY_122_GL=MY_122_HZ=MY_122_FX=MY_122_S_BLOCK=MY_122_F_BLOCK=0;
	MY_122_SU0_BLOCK = MY_122_FU0_BLOCK = 0;


	pelm->Scan_Func=PRLIB_122_SCAN;
	
	return 0;	
}


/*瞬时加压闭锁*/
enum{USSY1,USSY2,UFSY1,UFSY2,USSJS,UFSJS,USWY,UFWY};
enum{USSY3,UFSY3,USWY1,UFWY1}; // 瞬压最大200ms
#define MY_123_TIMER1  ((TMAXTIMERELAY*)(pelm->apvUser[0]))
#define MY_123_TIMER2  ((TMAXTIMERELAY*)(pelm->apvUser[1]))

#define MY_123_S_BLOCK (pelm->aulUser[0])
#define MY_123_F_BLOCK (pelm->aulUser[1])
#define MY_123_S_QD    (pelm->aulUser[2])
#define MY_123_F_QD    (pelm->aulUser[3])
void PRLIB_123_SCAN(EP_ELEMENT *pelm)
{
	BOOL bUS1, bUF1, bUS2, bUF2, bUSYS, bUSYF;
    if (pPublicSet->bSetChg)
	{
		InitTR(&(MY_123_TIMER1->tMaxTimeRelay[USSY1]), pPublicSet->dTSY1, 1000);
		SetTR_TRTT(&(MY_123_TIMER1->tMaxTimeRelay[USSY1]), 0);
		InitTR(&(MY_123_TIMER1->tMaxTimeRelay[USSY2]), pPublicSet->dTY, 1000);
		SetTR_TRTT(&(MY_123_TIMER1->tMaxTimeRelay[USSY2]), 0);
#if(TYPE_USER == USER_SHANDONG)
		InitTR(&(MY_123_TIMER2->tMaxTimeRelay[USSY3]), pPublicSet->dTSY2, 5000);
#else
		InitTR(&(MY_123_TIMER2->tMaxTimeRelay[USSY3]), pPublicSet->dTY, 5000);
#endif
		SetTR_TRTT(&(MY_123_TIMER2->tMaxTimeRelay[USSY3]), 0);
		InitTR(&(MY_123_TIMER1->tMaxTimeRelay[UFSY1]), pPublicSet->dTSY1, 5000);
		SetTR_TRTT(&(MY_123_TIMER1->tMaxTimeRelay[UFSY1]), 0);
		InitTR(&(MY_123_TIMER1->tMaxTimeRelay[UFSY2]), pPublicSet->dTY, 5000);
		SetTR_TRTT(&(MY_123_TIMER1->tMaxTimeRelay[UFSY2]), 0);
#if(TYPE_USER == USER_SHANDONG)
		InitTR(&(MY_123_TIMER2->tMaxTimeRelay[UFSY3]),pPublicSet->dTSY2, 5000);
#else
		InitTR(&(MY_123_TIMER2->tMaxTimeRelay[UFSY3]),pPublicSet->dTY, 5000);
#endif
		SetTR_TRTT(&(MY_123_TIMER2->tMaxTimeRelay[UFSY3]), 0);
		InitTR(&(MY_123_TIMER1->tMaxTimeRelay[USSJS]), pPublicSet->dTX, 5000);
		SetTR_TRTT(&(MY_123_TIMER1->tMaxTimeRelay[USSJS]), 0);
		InitTR(&(MY_123_TIMER1->tMaxTimeRelay[UFSJS]), pPublicSet->dTX, 5000);
		SetTR_TRTT(&(MY_123_TIMER1->tMaxTimeRelay[UFSJS]), 0);
		InitTR(&(MY_123_TIMER1->tMaxTimeRelay[USWY]), 1000, 3000);
		SetTR_TRTT(&(MY_123_TIMER1->tMaxTimeRelay[USWY]), 0);
		InitTR(&(MY_123_TIMER1->tMaxTimeRelay[UFWY]), 1000, 3000);
		SetTR_TRTT(&(MY_123_TIMER1->tMaxTimeRelay[UFWY]), 0);

		InitTR(&(MY_123_TIMER2->tMaxTimeRelay[USWY1]), 500, 0);
		SetTR_TRTT(&(MY_123_TIMER2->tMaxTimeRelay[USWY1]), 0);
		InitTR(&(MY_123_TIMER2->tMaxTimeRelay[UFWY1]), 500, 0);
		SetTR_TRTT(&(MY_123_TIMER2->tMaxTimeRelay[UFWY1]), 0);
	}
	if (!pelm->ppioIn[_PR_123_ENABLE].bVal)
	{
		ResetTR(&(MY_123_TIMER1->tMaxTimeRelay[USSY1]));
		ResetTR(&(MY_123_TIMER1->tMaxTimeRelay[USSY2]));
		ResetTR(&(MY_123_TIMER2->tMaxTimeRelay[USSY3]));
		ResetTR(&(MY_123_TIMER1->tMaxTimeRelay[UFSY1]));
		ResetTR(&(MY_123_TIMER1->tMaxTimeRelay[UFSY2]));
		ResetTR(&(MY_123_TIMER2->tMaxTimeRelay[UFSY3]));
		pelm->aioOut[_PR_123_SY_BLOCK].bVal=FALSE;
		pelm->aioOut[_PR_123_CY_BLOCK].bVal=FALSE;
		MY_123_S_BLOCK=MY_123_F_BLOCK=MY_123_S_QD = MY_123_F_QD = 0;
		return;
	}
	if(pelm->ppioIn[_PR_123_FX].ulVal == 1)
		MY_123_S_BLOCK = 1;
	else if(pelm->ppioIn[_PR_123_FX].ulVal == 2)
		MY_123_F_BLOCK = 1;

	bUS1 = bUF1 = bUS2 = bUF2 = bUSYS = bUSYF= FALSE;
    if(pelm->ppioIn[_PR_123_US].ulVal > pelm->ppioIn[_PR_123_US1_PICK].ulVal)
		bUS1 = TRUE;
	else if(pelm->ppioIn[_PR_123_US].ulVal < pelm->ppioIn[_PR_123_US2_PICK].ulVal)
		bUS2 = TRUE;
	if(pelm->ppioIn[_PR_123_UF].ulVal > pelm->ppioIn[_PR_123_UF1_PICK].ulVal)
		bUF1 = TRUE;
	else if(pelm->ppioIn[_PR_123_UF].ulVal < pelm->ppioIn[_PR_123_UF2_PICK].ulVal)
		bUF2 = TRUE;
	
	 if(pelm->ppioIn[_PR_123_US].ulVal > pelm->ppioIn[_PR_123_US2_PICK].ulVal)
	 	bUSYS = TRUE;
	 if(pelm->ppioIn[_PR_123_UF].ulVal > pelm->ppioIn[_PR_123_UF2_PICK].ulVal)
	 	bUSYF = TRUE;

    //bUSYS=bUSYS&&(!bUS1);
    //bUSYF=bUSYF&&(!bUF1);   
    
	RunTR(&(MY_123_TIMER1->tMaxTimeRelay[USSY1]), bUSYS);
	RunTR(&(MY_123_TIMER1->tMaxTimeRelay[UFSY1]), bUSYF);
	RunTR(&(MY_123_TIMER1->tMaxTimeRelay[USSY2]), bUSYS);
	RunTR(&(MY_123_TIMER1->tMaxTimeRelay[UFSY2]), bUSYF);
	
	RunTR(&(MY_123_TIMER2->tMaxTimeRelay[USSY3]), bUSYS);
	RunTR(&(MY_123_TIMER2->tMaxTimeRelay[UFSY3]), bUSYF);
	
	RunTR(&(MY_123_TIMER1->tMaxTimeRelay[USSJS]), bUS1);
	RunTR(&(MY_123_TIMER1->tMaxTimeRelay[UFSJS]), bUF1);
	RunTR(&(MY_123_TIMER1->tMaxTimeRelay[USWY]),  bUS2);
    RunTR(&(MY_123_TIMER1->tMaxTimeRelay[UFWY]),  bUF2);
	RunTR(&(MY_123_TIMER2->tMaxTimeRelay[USWY1]), bUS2);
	RunTR(&(MY_123_TIMER2->tMaxTimeRelay[UFWY1]), bUF2);

	
	if(MY_123_TIMER1->tMaxTimeRelay[USWY].boolTrip)
	    MY_123_S_QD = 1;
	if(MY_123_TIMER1->tMaxTimeRelay[UFWY].boolTrip)
		MY_123_F_QD = 1;

	// 瞬压
	{
		if(MY_123_TIMER2->tMaxTimeRelay[USSY3].boolTrip && (pelm->ppioIn[_PR_123_X_FX].bVal == 2))
			MY_123_S_QD = 0;
		if(MY_123_TIMER2->tMaxTimeRelay[UFSY3].boolTrip && (pelm->ppioIn[_PR_123_X_FX].bVal == 1))
			MY_123_F_QD = 0;
	}
	 // 残压
	{
		if(MY_123_TIMER1->tMaxTimeRelay[USSY2].boolTrip)
			MY_123_S_QD = 0;
		if(MY_123_TIMER1->tMaxTimeRelay[UFSY2].boolTrip)
			MY_123_F_QD = 0;
	}

	
	if(MY_123_S_QD&&MY_123_TIMER2->tMaxTimeRelay[USWY1].boolTrip&&MY_123_TIMER1->tMaxTimeRelay[USSY1].boolTrip&&!MY_123_TIMER1->tMaxTimeRelay[USSY2].boolTrip)
	{
		MY_123_S_BLOCK = 1;
		MY_123_S_QD = 0;
	}
	if(MY_123_F_QD&&MY_123_TIMER2->tMaxTimeRelay[UFWY1].boolTrip&&MY_123_TIMER1->tMaxTimeRelay[UFSY1].boolTrip&&!MY_123_TIMER1->tMaxTimeRelay[UFSY2].boolTrip)
	{
		MY_123_F_BLOCK = 1;
		MY_123_F_QD = 0;
	}
    if(MY_123_TIMER1->tMaxTimeRelay[USSJS].boolTrip)
	{
	    if (MY_123_S_BLOCK == 1)// modify by lijun 2017-03-17
	 	   pRunPublic->js_type |= HZ_JS_DY;
	    MY_123_S_BLOCK = 0;

	}
	if(MY_123_TIMER1->tMaxTimeRelay[UFSJS].boolTrip)
	{ 
	    if (MY_123_F_BLOCK == 1)// modify by lijun 2017-03-17
	 	    pRunPublic->js_type |= HZ_JS_DY;
	    MY_123_F_BLOCK = 0;

	}

    if ((MY_123_S_BLOCK|MY_123_F_BLOCK)&pPublicSet->bSYBS)
	{
	   if ((pelm->ppioIn[_PR_123_X_FX].bVal == 0) ||(MY_123_S_BLOCK && (pelm->ppioIn[_PR_123_X_FX].bVal == 1)) 
		 	|| (MY_123_F_BLOCK && (pelm->ppioIn[_PR_123_X_FX].bVal == 2)))
	      pelm->aioOut[_PR_123_CY_BLOCK].bVal = TRUE;
	   if (MY_123_S_BLOCK && (pelm->ppioIn[_PR_123_X_FX].bVal == 2))
	   	  pelm->aioOut[_PR_123_SY_BLOCK].bVal = TRUE;
	   if (MY_123_F_BLOCK && (pelm->ppioIn[_PR_123_X_FX].bVal == 1))
	   	  pelm->aioOut[_PR_123_SY_BLOCK].bVal = TRUE;
	   if(MY_123_S_BLOCK)
			pelm->aioOut[_PR_123_OUTPUT_FX].ulVal = 1;
	   if(MY_123_F_BLOCK)
			pelm->aioOut[_PR_123_OUTPUT_FX].ulVal = 2;
    }
	else
	{
		pelm->aioOut[_PR_123_CY_BLOCK].bVal = FALSE;
		pelm->aioOut[_PR_123_SY_BLOCK].bVal = FALSE;
	}			
}

int PRLIB_123(EP_ELEMENT *pelm)
{
	pelm->apvUser[0] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
	pelm->apvUser[1] = (TMAXTIMERELAY*)malloc(sizeof(TMAXTIMERELAY));
	
	pelm->ppioIn  = (VALUE_TYPE *)malloc(_PR_123_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut  = (VALUE_TYPE *)malloc(_PR_123_OUTPUT*sizeof(VALUE_TYPE));
	pelm->ppioIn[_PR_123_FX].ulVal  = 0;
	pelm->aioOut[_PR_123_SY_BLOCK].bVal=FALSE;
	pelm->aioOut[_PR_123_CY_BLOCK].bVal=FALSE;
	pelm->aioOut[_PR_123_OUTPUT_FX].bVal=FALSE;
	MY_123_S_BLOCK=MY_123_F_BLOCK=0;

	pelm->Scan_Func=PRLIB_123_SCAN;
	return 0;
}


#define MY_124_TIMER1   ((TIMERELAY*)(pelm->apvUser[0]))
#define MY_124_TIMER2   ((TMAXTIMERELAY*)(pelm->apvUser[1]))
void PRLIB_124_SCAN(EP_ELEMENT *pelm)
{
   long lDiff;
   BOOL bU,bUS ;
   COMPLEX cua,cuc;
   long ang1, ang2, ang;

   if (pPublicSet->bSetChg)
   {
		InitTR(MY_124_TIMER1, pPublicSet->dTX, 0);
		SetTR_TRTT(MY_124_TIMER1, 0);
		InitTR(&(MY_124_TIMER2->tMaxTimeRelay[0]), 500, 0);
		InitTR(&(MY_124_TIMER2->tMaxTimeRelay[1]), 500, 0);
		InitTR(&(MY_124_TIMER2->tMaxTimeRelay[2]), 500, 0);
		InitTR(&(MY_124_TIMER2->tMaxTimeRelay[3]), 500, 0);
   }

   bU = FALSE;
   if (pelm->ppioIn[_PR_124_US].ulVal > pelm->ppioIn[_PR_124_US_PICK].lVal)
   	  bU =  TRUE;
   RunTR(&(MY_124_TIMER2->tMaxTimeRelay[0]), bU);
   
   bU = FALSE;
   if (pelm->ppioIn[_PR_124_US].ulVal < pelm->ppioIn[_PR_124_US_PICK].lVal*3/4)
   	  bU = TRUE;
   RunTR(&(MY_124_TIMER2->tMaxTimeRelay[1]), bU);

   bU = FALSE;
   if (pelm->ppioIn[_PR_124_UF].ulVal > pelm->ppioIn[_PR_124_UF_PICK].lVal)
   	  bU =  TRUE;
   RunTR(&(MY_124_TIMER2->tMaxTimeRelay[2]), bU);
   
   bU = FALSE;
   if (pelm->ppioIn[_PR_124_UF].ulVal < pelm->ppioIn[_PR_124_UF_PICK].lVal*3/4)
   	  bU = TRUE;
   RunTR(&(MY_124_TIMER2->tMaxTimeRelay[3]), bU);
   
   pelm->aioOut[_PR_124_S].bVal = MY_124_TIMER2->tMaxTimeRelay[0].boolTrip&&!MY_124_TIMER2->tMaxTimeRelay[1].boolTrip;
   pelm->aioOut[_PR_124_F].bVal = MY_124_TIMER2->tMaxTimeRelay[2].boolTrip&&!MY_124_TIMER2->tMaxTimeRelay[3].boolTrip;

   
   if (!pelm->ppioIn[_PR_124_ENABLE].bVal)
   {
		ResetTR(MY_124_TIMER1);
		pelm->aioOut[_PR_124_OPERATE].bVal=FALSE;
		return;
   }

   bUS = FALSE;
   bU = FALSE;
   if ((pelm->ppioIn[_PR_124_US].ulVal < pelm->ppioIn[_PR_124_US_PICK].lVal)||
   	   (pelm->ppioIn[_PR_124_UF].ulVal < pelm->ppioIn[_PR_124_UF_PICK].lVal))
   	   bUS = TRUE;
   if (bUS)
   {
       pelm->aioOut[_PR_124_OPERATE].bVal = FALSE;
	   ResetTR(MY_124_TIMER1);
	   return;
   }

   if(pelm->ppioIn[_PR_124_BLOCK].bVal)
   {
      ResetTR(MY_124_TIMER1);
	  pelm->aioOut[_PR_124_OPERATE].bVal=FALSE;
	  return;
   }

   cua.x = cua.y = cuc.x = cuc.y = 0;

   if (pelm->ppioIn[_PR_124_SINDEX].lVal >= 0)
      cua = aiProtCal[pelm->ppioIn[_PR_124_SINDEX].ulVal].UI;
  
   if (pelm->ppioIn[_PR_124_FINDEX].lVal >= 0)
      cuc = aiProtCal[pelm->ppioIn[_PR_124_FINDEX].ulVal].UI;
   
   lDiff = pelm->ppioIn[_PR_124_US].lVal - pelm->ppioIn[_PR_124_UF].lVal;
   if(lDiff < 0)
   {
      lDiff = -lDiff;
   }
 
   if(lDiff < pPublicSet->dUDiff)
   	 bU = TRUE;

   ang1= F_ANG(cua, F_AMP(cua));
   ang2= F_ANG(cuc, F_AMP(cuc));
   ang = ang1 - ang2;

   if (ang > 11796480)
	   ang -= 23592960;
   if (ang < -11796480)
	   ang += 23592960;

   if (ang < 0)
	   ang = -ang; 
   if (ang > pPublicSet->dUAng)
   	  bU = FALSE;

   RunTR(MY_124_TIMER1, bU);

   pelm->aioOut[_PR_124_OPERATE].bVal = MY_124_TIMER1->boolTrip;
}

int PRLIB_124(EP_ELEMENT *pelm)
{
	
	pelm->apvUser[0] =(TIMERELAY *)malloc(sizeof(TIMERELAY));
	pelm->apvUser[1] =(TMAXTIMERELAY *)malloc(sizeof(TMAXTIMERELAY));
	pelm->ppioIn  =(VALUE_TYPE *)malloc(_PR_124_INPUT*sizeof(VALUE_TYPE));
	pelm->aioOut  =(VALUE_TYPE *)malloc(_PR_124_OUTPUT*sizeof(VALUE_TYPE));
		
	pelm->aioOut[_PR_124_OPERATE].bVal=FALSE;
	
	pelm->Scan_Func=PRLIB_124_SCAN;
	
	return 0;	
}
#endif
