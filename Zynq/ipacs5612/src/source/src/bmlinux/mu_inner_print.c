/*
 * @Author: zhongwei
 * @Date: 2020/3/25 19:19:04
 * @Description: ����BM��ӡ��Linux����Ϣ����
 * @File: mu_inner_print.cc
 * 
 * 
*/
#include "plt_include.h"
#include "mu_inner_print.h"
#include <syslog.h>


#define _MAX_LINE_LEN 256
static char _line_c[_MAX_LINE_LEN + 8];
static int32 _line_cnt = 0;
static int _last_begin = -1;

static void _print_line(void)
{
    if (_line_cnt > 0 && _line_cnt <= _MAX_LINE_LEN)
    {
        _line_c[_line_cnt] = 0;

        if (!IsDebugMessage(2))
        {
            PRINTF("[BM]");
            PRINTF(_line_c);
        }

        syslog(LOG_INFO | g_syslog_bm, _line_c);
    }

    _line_cnt = 0;
}

static void _line_add_char(char c)
{
    if (c == '\r')  //����
    {
        return;
    }

    if (c == '\n')
    {
        if (_line_cnt > 0)
        {
            _print_line();
        }

        PRINTF("\n\r");

        return;
    }

    if (_line_cnt < 0 || _line_cnt >= _MAX_LINE_LEN)
    {
        _print_line();
    }

    _line_c[_line_cnt++] = c;
    if (c == '%' || c == '\\')
    {
        _line_c[_line_cnt++] = c;       //%��Ҫת�崦��
    }
}

/**
 * @Function: mc_Print_from_BM
 * @Description: ��BM���մ�ӡ��Ϣ���� 
 * @author zhongwei (2020/2/7 11:31:33)
 * 
 * @param void 
 * 
*/
void mc_Print_from_BM(void)
{
    TPRINT_TO_LINUX_STRUCT * pPrintMemory = GetSharedPrintMemory();

    if (IsSharedBMMemValid() && pPrintMemory->nBufferLen > 0)
    {
        int32 nBegin = pPrintMemory->nBegin;
        int32 nEnd = pPrintMemory->nEnd;

        if (_last_begin == nBegin && _line_cnt > 0)   //û�������ݣ��������ǰ�Ļ�������
        {
            _print_line();
        }

        if (nBegin != nEnd)
        {
            if (nEnd >=0 && nEnd < pPrintMemory->nBufferLen)
            {
                if (nBegin >=0 && nBegin < pPrintMemory->nBufferLen)
                {
                    int i;

                    if (nBegin < nEnd)
                    {
                        //��ӡ��nBegin-nEnd��Χ�ڵ�����
                        for (i=nBegin;i<nEnd;i++)
                        {
                            char cc = pPrintMemory->buf[i];
                            if (cc != 0)
                            {
                                _line_add_char(cc);
                            }
                        }
                    }
                    else
                    {
                        for (i=nBegin;i<pPrintMemory->nBufferLen;i++)
                        {
                            char cc = pPrintMemory->buf[i];
                            if (cc != 0)
                            {
                                _line_add_char(cc);
                            }
                        }
                        for (i=0;i<nEnd;i++)
                        {
                            char cc = pPrintMemory->buf[i];
                            if (cc != 0)
                            {
                                _line_add_char(cc);
                            }
                        }
                    }
                }

                pPrintMemory->nBegin = nEnd;
            }
        }

        _last_begin = nBegin;
    }
}
