/*
 * @Author: zhongwei
 * @Date: 2019/11/27 10:48:15
 * @Description: ��˴�ӡ�����linux�˵Ĺ��ܽӿ�
 * @File: plt_mc_print.c
 *
*/

#include "plt_include.h"
#include "plt_mc_print.h"

SECTION_PLT_CODE void PowerOn_Init_PrintToLinux(void)
{
    TPRINT_TO_LINUX_STRUCT * pPrintMemory = GetSharedPrintMemory();
    pPrintMemory->nBufferLen = MAX_PRINT_TO_LINUX_BUFFER_LEN;   //����������
}

/**
 * @Function: Print_To_Linux
 * @Description: ��ӡ����������ڴ棬���ַ��������Linux 
 * @author zhongwei (2019/11/28 9:37:38)
 * 
 * @param str 
 * 
 * @return SECTION_PLT_CODE void 
 * 
*/
SECTION_PLT_CODE void Print_To_Linux(const char * str)
{
    TPRINT_TO_LINUX_STRUCT * pPrintMemory = GetSharedPrintMemory();
    ASSERT_L2(pPrintMemory != NULL);
    ASSERT_L2(str != NULL);

    if (IsSharedBMMemValid() && pPrintMemory->nBufferLen == MAX_PRINT_TO_LINUX_BUFFER_LEN)
    {
        int32 nLen = E_STRLEN(str); //�ַ�������
        int32 nFreeAfter;
        int32 nCpyAfter;
        int32 nCpyNext;
        int32 nEnd = pPrintMemory->nEnd;
        if (nLen > 512)
        {
            return; //�����ַ�����������
        }
        //���endֵ�Ƿ���ȷ
        if (nEnd < 0 && nEnd >= pPrintMemory->nBufferLen)
        {
            nEnd=0;
        }

        nFreeAfter = pPrintMemory->nBufferLen - pPrintMemory->nEnd;
        nCpyAfter = f_min(nFreeAfter, nLen);
        E_MEMCPY(&pPrintMemory->buf[nEnd], str, nCpyAfter);
        nEnd += nCpyAfter;
        if (nEnd >= pPrintMemory->nBufferLen)
        {
            nEnd = 0;
        }

        if (nCpyAfter < nLen)
        {
            nCpyNext = nLen - nCpyAfter;
            E_MEMCPY(&pPrintMemory->buf[0], &str[nCpyAfter], nCpyNext);
            nEnd = nCpyNext;
        }
        pPrintMemory->nEnd = nEnd;
    }
}

