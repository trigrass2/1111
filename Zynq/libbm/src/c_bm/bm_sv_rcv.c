/*
 * @Author: zhongwei
 * @Date: 2020/4/1 16:12:16
 * @Description: SV接收处理代码
 * @File: bm_sv_rcv.c
 *
*/
#include "plt_include.h"
#include "bm_sv_rcv.h"

/**
 * @Function: on_bm_sv_rcv
 * @Description:  BM接收到一帧SV数据的处理
 * @author zhongwei (2020/4/1 17:34:51)
 * 
 * @param pSv SV帧报文解析结构
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/

//最大10次，#define MAX_POLL_MSG_CNT 10 	//每次拉取的最大包数

DWORD nSamBuf[10][64]; //最大64通道
WORD nSamCnt = 0;


SECTION_PLT_CODE void on_bm_sv_rcv(const TFRAME_SV_RCV * pSv)
{
    memcpy(nSamBuf[nSamCnt],pSv->sv_data,pSv->bay_cnt<<6);
	nSamCnt++;
}

