#ifndef _EXTMMI_H
#define _EXTMMI_H

#include "pcol.h"


#define MAINT_CMD_SYSTEM       0x09
#define MAINT_CMD_DATA         0x0c

#define MAINT_ACK_OK          0x8001
#define MAINT_ACK_ERR         0x8002

#define MAINT_CODE	           0x68
#define MAINT_CRCLEN           2

#define MAINT_FILE_NAMEERR     0x12
#define MAINT_FILE_ERR         0x10
#define MAINT_DISK_ERR         0x20
#define MAINT_BOOT_ERR         0x30
#define MAINT_SEC_ERR          0x40
#define MAINT_CMD_ERR          0x1

#define MAINT_LEN_MAX          1280
#define MAINT_BUF_MAX          1024

#define FILE_NAME_MAX          64





#define SET_LEDIO         0x0101
#define YK_BM             0x0102
#define YK_T              0x0103
#define YX_T              0x0104
#define YC_ZERO           0x0105
#define YK_YB             0x0106
#define YK_TYPE           0x0107
#define READ_PRPARA       0x0108
#define ERR_CODE          0x0109



#define GET_IO            0x0201
#define WRITE_PRPARA      0x0202
#define WRITE_PRPARA_FILE      0x0203
#define WRITE_RUN         0x0204


#define GET_CLK			  0x0a04	//��ȡʱ��
#define SET_CLK			  0x0405	//����ʱ��

#define RESET_PROTECT				0x0523	//��������
#define YC_PARAZERO           0x0545
#define YC_PARASET			  0x0542	//ң���������

#define YC_PARAVALSET       0x0544
#define SET_BH			0x052d	//��װ������ֵ

#define GET_ycfloat          0x0c1f  //ycfloat           
#define GET_accurve          0x0c20  //��������  
#define GET_dotdef          0x0c22  //�������     
#define GET_BH          0x0c27  //��ַ��Ϣ
#define GET_YXZ          0x0c28  //��Чֵ
#define GET_YC1				0x0c2b	//��ң��
#define GET_YC2				0x0c2c	//��ң��
#define GET_Lubo                  0x0c35 //�ֶ�����¼��



#pragma pack(1)

typedef struct 
{
    BYTE StartCode1;  	//�����ַ�1
    BYTE Len1Low;
    BYTE Len1High;
    BYTE Len2Low;
    BYTE Len2High;
    BYTE StartCode2;  	//�����ַ�2
    BYTE Address;      	//�����ַ
    BYTE CMD;
    BYTE afn;
}VMaintFrame; 	

#pragma pack()



typedef struct
{
        VPcol Col;

		BYTE byCnt;
		BYTE bCommCnt;
		DWORD dwCommCtrlTimer;

		DWORD dwOffset;
		int  dwLen;
		BOOL bEnd;

        BYTE rcdIndex;
		DWORD byBuf[MAINT_BUF_MAX];
		int prosetnum;
}VMaint;
#endif
