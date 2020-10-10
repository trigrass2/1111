/*
 * @Author: zhongwei
 * @Date: 2020/4/1 16:12:16
 * @Description: SV���մ������
 * @File: bm_sv_rcv.c
 *
*/
#include "plt_include.h"
#include "bm_sv_rcv.h"

/**
 * @Function: on_bm_sv_rcv
 * @Description:  BM���յ�һ֡SV���ݵĴ���
 * @author zhongwei (2020/4/1 17:34:51)
 * 
 * @param pSv SV֡���Ľ����ṹ
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/

//���10�Σ�#define MAX_POLL_MSG_CNT 10 	//ÿ����ȡ��������

DWORD nSamBuf[10][64]; //���64ͨ��
WORD nSamCnt = 0;


SECTION_PLT_CODE void on_bm_sv_rcv(const TFRAME_SV_RCV * pSv)
{
    memcpy(nSamBuf[nSamCnt],pSv->sv_data,pSv->bay_cnt<<6);
	nSamCnt++;
}

