/*
 * @Author: zhongwei
 * @Date: 2020/2/6 16:11:27
 * @Description: 与BM内部通讯
 * @File: mu_inner_comm.h
 *
*/

#ifndef SRC_LINUX_MC_MU_INNER_COMM_H_
#define SRC_LINUX_MC_MU_INNER_COMM_H_

//发送命令行到BM
void bmdo(const char * cmd);

//BM<->Linux内部通讯初始化
void PowerOn_Init_McInnerComm(void);



typedef struct
{
	WORD   wReadPtr;						
	WORD   wWritePtr;
	WORD   wBufSize;
	BYTE   *pBuf;			
}VInnerBuf;

struct VInnerFrame
{
    BYTE StartCode1;  	//启动字符1
    BYTE Len1Low;
    BYTE Len1High;
    BYTE Len2Low;
    BYTE Len2High;
    BYTE StartCode2;  	//启动字符2
    BYTE Address;      	//对象地址
    BYTE CMD;
    BYTE afn;
}; 

#define INNERBUFLEN	 1024
#define INNER_FRAME_SHIELD   0xFFFF0000	   //屏蔽字
#define INNER_FRAME_OK	    0x00010000	   //检测到一个完整的帧
#define INNER_FRAME_ERR	    0x00020000	   //检测到一个校验错误的帧
#define INNER_FRAME_LESS	    0x00030000	   //检测到一个不完整的帧（还未收齐）

#define INNER_ACK_OK          0x8001
#define INNER_ACK_ERR         0x8002

#define INNER_SET_LEDIO        0x0101
#define INNER_YK_BM            0x0102
#define INNER_YK_T             0x0103
#define INNER_YX_T             0x0104
#define INNER_YC_ZERO          0x0105
#define INNER_YK_YB            0x0106
#define INNER_YK_TYPE          0x0107
#define INNER_READ_PRPARA      0x0108
#define INNER_ERR_CODE         0x0109


#define INNER_GET_IO            0x0201
#define INNER_WRITE_PRPARA      0x0202
#define INNER_WRITE_PRPARA_FILE 0x0203
#define INNER_WRITE_RUN         0x0204
           


#define INNER_GET_CLK				0x0a04	//获取时钟
#define INNER_SET_CLK				0x0405	//设置时钟
#define INNER_RESET_PROTECT			0x0523	//复归命令


#define INNER_CMD_ERR          0x1

int smMTake(DWORD smid);
int smMGive(DWORD smid);
DWORD smMCreate(void);
void mc_innerbuf_init(void);

#endif /* SRC_LINUX_MC_MU_INNER_COMM_H_ */
