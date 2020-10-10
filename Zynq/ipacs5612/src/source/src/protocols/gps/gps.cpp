#include "bsp.h"
#include "sys.h"
#include "gps.h"
#include "protocol.h"

VSysClock GpsTime;
VCalClock GpsCalTime;
VGPSPCL *pGps;

int ggpsstart=0;
DWORD gpscnt;

DWORD gadnum;

#if (TYPE_CPU == CPU_STM32F4)
void gps_io_init(void)
{


#if (DEV_SP == DEV_SP_DTU)
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    sysPinInit(GPIOC, &GPIO_InitStructure);

	syscfgEXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource9);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line9;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	stm32EXTIInit(&EXTI_InitStructure);
	EXTI->PR = EXTI_Line9;
	stm32fIntEnable(EXTI9_5_IRQn, INT_PRIO_UART);
#endif

#if (DEV_SP == DEV_SP_WDG)
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

  sysPinInit(GPIOD, &GPIO_InitStructure);

	syscfgEXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource3);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	stm32EXTIInit(&EXTI_InitStructure);
	EXTI->PR = EXTI_Line3;
	stm32fIntEnable(EXTI3_IRQn, INT_PRIO_UART);
	
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    sysPinInit(GPIOH, &GPIO_InitStructure);

	CLEAR_GPIO(GPIOH,GPIO_Pin_11);
#endif
}

void gps_isr(void)
{
  DWORD tick, interval;
	static BOOL bSynFlag= FALSE;
	static DWORD lasttick;
		
	tick = TIM5->CNT;

	interval = tick - lasttick;

	lasttick = TIM5->CNT;

//	if (interval != 10000)
 // 		WriteWarnEvent(NULL, SYS_ERR_AD, 0, "pps pulse %d\n", interval);
	
	if (!bSynFlag)
	{
		  TIM2->CNT = 1;
	      bSynFlag = TRUE;
	}
	else
	{  
		if ((interval < 10050)&&(interval > 9950))
		{
			if (TIM2->CNT < (TIM2->ARR>>1))
			{
				TIM2->CNT = 1;
			}
			else
				TIM2->CNT = 0;
			
			if(GpsTime.bySecond)
			{
				SetSysClock(&GpsTime, SYSCLOCK);		
				GpsTime.bySecond = 0;
			}
		}
		else
			WriteWarnEvent(NULL, SYS_ERR_AD, 0, "pps pulse %d\n", interval);
		
	}
	
	
}
#elif (TYPE_CPU == CPU_SAM9X25)
extern DWORD gUsCnt;
void EXT_GpsHandler(int vector, void *param)
{
	  DWORD isr;
    /*  Pointer to the PIO controller which has the pin(s). */

   isr = vector - 3*32;
#if(DEV_SP == DEV_SP_DTU_PU)	
	if (isr == GPIO_PinSource10)
  {
		gps_isr();
	}
#else
	if (isr == GPIO_PinSource2)
  {
		gps_isr();
	}
#endif
    
}
void gps_io_init()
{
#if (DEV_SP == DEV_SP_DTU)
	Pin   GPIO_InitStructure;
	
	GPIO_InitStructure.id         =  ID_PIOC;
	GPIO_InitStructure.attribute  =  PIO_IT_RISE_EDGE;
	GPIO_InitStructure.type       =  PIO_INPUT;
	GPIO_InitStructure.mask       =  GPIO_Pin_2; 
	GPIO_InitStructure.pio        =  GPIOC;
	sysPinInit(GPIOC,&GPIO_InitStructure);
	rt_hw_interrupt_install(GPIO_PinSource2 + 3*32 ,EXT_GpsHandler,RT_NULL,"gps_io");	
	PIO_ConfigureIt(&GPIO_InitStructure);
	rt_hw_interrupt_umask(GPIO_PinSource2 + 3*32);
 #endif
#if (DEV_SP == DEV_SP_DTU_PU)
    Pin   GPIO_InitStructure;

    GPIO_InitStructure.id         =  ID_PIOC;
    GPIO_InitStructure.attribute  =  PIO_IT_RISE_EDGE;
    GPIO_InitStructure.type       =  PIO_INPUT;
    GPIO_InitStructure.mask       =  GPIO_Pin_10; 
    GPIO_InitStructure.pio        =  GPIOC;
    sysPinInit(GPIOC,&GPIO_InitStructure);
    
    rt_hw_interrupt_install(GPIO_PinSource10 + 3*32 ,EXT_GpsHandler,RT_NULL,"gps_io");  
    PIO_ConfigureIt(&GPIO_InitStructure);
    rt_hw_interrupt_umask(GPIO_PinSource10 + 3*32);
    
    GPIO_InitStructure.id         =  ID_PIOA;
    GPIO_InitStructure.attribute  =  PIO_IT_RISE_EDGE;
    GPIO_InitStructure.type       =  PIO_INPUT;
    GPIO_InitStructure.mask       =  GPIO_Pin_19; 
    GPIO_InitStructure.pio        =  GPIOA;
    sysPinInit(GPIOA,&GPIO_InitStructure);

    GPIO_InitStructure.id         =  ID_PIOA;
    GPIO_InitStructure.attribute  =  PIO_DEFAULT;
    GPIO_InitStructure.type       =  PIO_OUTPUT_0;
    GPIO_InitStructure.mask       =  GPIO_Pin_20;
    sysPinInit(GPIOA,&GPIO_InitStructure);
    
    //复位
    SET_GPIO(GPIOA, GPIO_Pin_20);  
    Slow(200);
    CLEAR_GPIO(GPIOA, GPIO_Pin_20);

   
 #endif

}

void gps_isr()
{
    DWORD tick, interval;
	static BOOL bSynFlag= FALSE;
	static DWORD lasttick;
		
	tick = gUsCnt*10;

	interval = tick - lasttick;

	lasttick = gUsCnt*10;

	
	if (!bSynFlag)
	{
//		  TIM2->CNT = 1;
	      bSynFlag = TRUE;
	}
	else
	{  
		if ((interval < 10050)&&(interval > 9950))
		{
	//		if (TIM2->CNT < (TIM2->ARR>>1))
	//		{
	//			TIM2->CNT = 1;
	//		}
	//		else
	//			TIM2->CNT = 0;
			
			if(GpsTime.bySecond)
			{
				SetSysClock(&GpsTime, SYSCLOCK);		
				GpsTime.bySecond = 0;
			}
		}
		else
			WriteDoEvent(NULL, SYS_ERR_AD, 0, "pps pulse %d\n", interval);
		
	}
	
	
}
#endif



extern "C" void gps(WORD wTaskID)
{
    pGps = new VGPSPCL;

	if (pGps == NULL) return;
	
	gps_io_init();

	pGps->m_thid =  wTaskID;
	pGps->m_commid = wTaskID;

	for(;;)
		pGps->Run();
}



VGPSPCL::VGPSPCL()
{
    m_Rec.size = 2048;
	m_Rec.buf = (BYTE*)malloc(2048);
	m_Rec.wp = m_Rec.rp = 0;
	m_bfirst = TRUE;
	m_cnt = 0;

}

char VGPSPCL::AtoC(char c)
{
	if(c >= 0x30 && c <= 0x39)
		return c - 0x30;
	else if(c >= 0x41 && c <= 0x46)
		return c-0x41+10;
	else if(c >= 0x61 && c <= 0x66)
		return c - 0x61 + 10;
	else return 0;

}

//0 报文OK，1 less，2错误
int VGPSPCL::GpsCheck(BYTE* buf, DWORD len, DWORD *pdeallen)
{
	BYTE check = 0;
	int i = 0;
	
	*pdeallen = 0;
	while(++i < len)
	{
		if(buf[i-1] == 0x0d && buf[i] == 0x0a)
			break;
	}
	
	if((i >= len) || (buf[i-1] != 0x0d) || (buf[i] != 0x0a))
	{
		if(i > 200) //太长才赋值，否则按帧less处理
		{
			*pdeallen = len;
			return 2;
		}
		else
			return 1;
	}
	len = i;

	*pdeallen = len+1;
	
	for(i = 1; i < len; i++)
	{
		if(buf[i] == '*')
		{
			if(check == (AtoC(buf[i+1]) * 16 + AtoC(buf[i+2])))
				return 0; //报文OK
			else 
				return 2; //报文错
		}
		if(buf[i] == 0x0d && buf[i+1] == 0x0a) 
			return 2;
		check ^= buf[i];
	}
	return 0;
}


//do timer
void VGPSPCL::RxdFrame(void)
{
	int len,n,i;
	DWORD deallen;
	BYTE *p = m_Rec.buf+m_Rec.rp;
	BYTE *GpsBuf = NULL;

	while(1)
	{
	p = m_Rec.buf+m_Rec.rp;
	len = m_Rec.wp - m_Rec.rp;
	if ( len < 6 )  
		return;
	
	deallen = 0;	
	
	while(len > 5) //最少6个字节才能判断
	{
		if(p[0] == '$' && p[1] == 'G' && (p[2] == 'N' || p[2] == 'P')&& p[3] == 'R' && p[4] == 'M' && p[5] == 'C')
			break;	
		p++;
		len--;		
		m_Rec.rp++;	
	}
	
	if(p[1]=='G' && (p[2] == 'N' || p[2] == 'P') && p[3]=='R' && p[4]=='M' && p[5]=='C')
	{
		GpsBuf = p;
		len = GpsCheck(GpsBuf,len, &deallen);
		if(len == 1) //less
			return;
		if(!len)
		{
			n = 0;
			for(i = 0; i < deallen; i++)
			{
				if(GpsBuf[i] == '*') break;
				if(n >= 20) break;
				if(GpsBuf[i] == ',') 
				{
					v[n++] = i+1;
				}
			}
			if(GpsBuf[v[1]] == 'A')
				i = 0;
			if((v[9] - v[8] >= 6) && (GpsBuf[v[1]] == 'A'))
			{
				i = 1;
				GpsTime.wYear= 2000+AtoC(GpsBuf[4+v[8]]) * 10 + AtoC(GpsBuf[5+v[8]]);
				GpsTime.byMonth = AtoC(GpsBuf[2+v[8]]) * 10 + AtoC(GpsBuf[3+v[8]]);
				GpsTime.byDay = AtoC(GpsBuf[0+v[8]]) * 10 + AtoC(GpsBuf[1+v[8]]);
				GpsTime.byHour = AtoC(GpsBuf[0+v[0]]) * 10 + AtoC(GpsBuf[1+v[0]]);
				GpsTime.byMinute = AtoC(GpsBuf[2+v[0]]) * 10 + AtoC(GpsBuf[3+v[0]]);
				GpsTime.bySecond = AtoC(GpsBuf[4+v[0]]) * 10 + AtoC(GpsBuf[5+v[0]]);
				GpsTime.wMSecond = 0;
				ToCalClock(&GpsTime, &GpsCalTime);
				GpsCalTime.dwMinute += 8*60;
                CalClockTo(&GpsCalTime, &GpsTime);
			}
		}
		
	}
	m_Rec.rp += deallen;
}
}

void VGPSPCL::NeatCommBuf(VCommBuf1 *pBuf)
{
    int i, j;
    if (pBuf->rp == 0)
	{
		return ; 
	}

	if (pBuf->rp >= pBuf->wp)
	{
		pBuf->rp = pBuf->wp=0;
		return ;
	}

	if (pBuf->wp >= 2048)
	{
		pBuf->rp = 0;
		pBuf->wp = 0;
		return ;
	}

	i = 0; 
	j = pBuf->rp;
	while (j < pBuf->wp)
	{
		pBuf->buf[i++] = pBuf->buf[j++];
	}

	pBuf->rp = 0; 
	pBuf->wp = i; 
}

void VGPSPCL::Run(void)
{
    DWORD events;
    int rc;
    int no;
    no = -1;

    for (; ;)
    {
#if (DEV_SP == DEV_SP_DTU_PU)
        evReceive(m_thid , EV_RX_AVAIL|EV_TM2,&events);
        if(events&EV_RX_AVAIL)
        {
            if (no == -1)
            	no = tmEvAfter(m_thid, 20, EV_TM2);
        }
        if (events & EV_TM2) 
        {
            NeatCommBuf(&m_Rec);
            rc = commRead(m_commid, m_Rec.buf+m_Rec.wp, 2048-m_Rec.wp, 0);
            if (rc>0)
            {		
                m_Rec.wp += rc;
                RxdFrame();
            }
            tmDelete(m_thid, no);
            no = -1;
        }
#else

        evReceive(m_thid , EV_RX_AVAIL,&events);
        if (events & EV_RX_AVAIL) 
        {
            NeatCommBuf(&m_Rec);
            rc = commRead(m_commid, m_Rec.buf+m_Rec.wp, 2048-m_Rec.wp, 0);
            if (rc>0)
            {		
                m_Rec.wp += rc;
                RxdFrame();
            }
        }
#endif
    }
}
void GpsTest(BYTE on)
{
    /*
    int rc;
    char str[]="$PMTK010,001*2E\r\n";
    NeatCommBuf(&m_Send);
    memcpy(m_Send.buf+m_Send.wp,str,strlen(str));
    rc=commWrite(m_commid,m_Send.buf+m_Send.rp,m_Send.wp-m_Send.rp, 0);
    if (rc>0)	
        m_Rec.rp += rc;
    */
    //上电
#if (TYPE_CPU == CPU_SAM9X25)
    if(on == 1)
    {
        SET_GPIO(GPIOA, GPIO_Pin_18);
        Slow(200);
        SET_GPIO(GPIOA, GPIO_Pin_20);  
        Slow(200);
        CLEAR_GPIO(GPIOA, GPIO_Pin_20);
    }
    else if (on == 0)
    {
        CLEAR_GPIO(GPIOA, GPIO_Pin_18);
        Slow(200);
    }
#endif
}



