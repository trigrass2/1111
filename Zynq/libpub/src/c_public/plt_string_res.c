/*
 * @Author: zhongwei
 * @Date: 2019/12/27 11:28:08
 * @Description: 字符串资源管理
 * @File: plt_string_res.c
 *
*/
#include "plt_inc_c.h"
#include "plt_string_res.h"
#include "plt_list.h"

typedef struct{
    list_t _str_list;       //注册字符串资源列表
    uint32 _str_list_cnt;   //注册个数
    TSTRING_RESOURCE ** pArray;        //从list转换成数组的字符串资源数组
    uint32 nArrayCnt;
    TID nBeginId;                        //当前注册字符串资源的启示ID值

    list_t _str_list_after;                 //在注册字符串资源转换为数组后注册的字符串资源
    uint32 _str_list_after_cnt;
    TID nAfterBeginId;                   //在注册字符串资源转换为数组后注册的字符串资源个数

    //内建字符串资源
    const TSTRING_RESOURCE *pBuildInRes;    //内建字符串资源头指针
    uint32 nBuildInResCnt;                  //内建字符串资源个数
    TID nBuildInBeginId;                    //内建字符串资源起始ID
}TSTRING_RES_REGISTER;

SECTION_PLT_DATA static TSTRING_RES_REGISTER _RegisterString;

//初始化注册字符串
SECTION_PLT_CODE void PowerOn_InitStringResource(void)
{

    E_MEMSET(&_RegisterString, 0, sizeof(_RegisterString));

    list_init(&_RegisterString._str_list);      //初始化链表
    list_init(&_RegisterString._str_list_after);

    //前面的是内建字符串资源
    _RegisterString.nBeginId = _RegisterString.nBuildInResCnt + _RegisterString.nBuildInBeginId;
}

/**
 * @Function: RegisterStringResource
 * @Description:  字符串资源注册
 * @author zhongwei (2019/12/27 13:36:24)
 * 
 * @param pRes      需要注册的字符串资源 
 * @param bMalloc   是否需要为该字符串资源重新分配空间
 * 
 * @return TID 注册返回ID值
 * 
*/
SECTION_PLT_CODE TID RegisterStringResource(const TSTRING_RESOURCE * pRes, BOOL bMalloc)
{
    TID nResID = ID_STRING_NULL;
    TSTRING_RESOURCE * pNewRes = NULL;
    if (bMalloc)
    {
        pNewRes = (TSTRING_RESOURCE *)E_MALLOC(sizeof(TSTRING_RESOURCE));
        if (pNewRes != NULL)
        {
            int i;
            for (i=0;i<LANGUAGE_COUNT;i++)
            {
                const char *sz = pRes->str[i];
                if (str_IsStringEmpty(sz))
                {
                    pNewRes->str[i] = "";
                }
                else
                {
                    int len = strlen(sz);
                    pNewRes->str[i] = (const char *)E_MALLOC(len + 1);
                    if(pNewRes->str[i] != NULL)
                    {
                        E_MEMCPY((char *)pNewRes->str[i], sz, len+1);
                    }
                }
            }
        }
    }
    else
    {
        pNewRes = (TSTRING_RESOURCE *)pRes;
    }

    if (pNewRes != NULL)    //注册
    {
        if (_RegisterString.pArray != NULL) //新注册的需要注册到after
        {
            list_append(&_RegisterString._str_list_after, pNewRes);    //添加到列表中
            nResID = _RegisterString._str_list_after_cnt + _RegisterString.nAfterBeginId;
            _RegisterString._str_list_after_cnt++;
        }
        else
        {
            list_append(&_RegisterString._str_list, pNewRes);    //添加到列表中
            nResID = _RegisterString._str_list_cnt + _RegisterString.nBeginId;
            _RegisterString._str_list_cnt++;
        }
    }

    return nResID;
}

//注册字符串
SECTION_PLT_CODE TID RegisterStringResourceStr(const char * cn, const char * en, const char * ru)
{
    TSTRING_RESOURCE res;
#ifdef  _LANGUAGE_CN_ENABLE
    res.str[LANGUAGE_CN] = cn;
#endif
#ifdef  _LANGUAGE_EN_ENABLE
    res.str[LANGUAGE_EN] = en;
#endif
#ifdef  _LANGUAGE_RU_ENABLE
    res.str[LANGUAGE_RU] = ru;
#endif
    return RegisterStringResource(&res, PLT_TRUE);
}

//将注册字符串转换为数组
/* 
    需要注册的字符串资源注册完成后调用，可以加快字符串访问速度 
*/
SECTION_PLT_CODE void RegisterStringToArray(void)
{
    if (_RegisterString.pArray == NULL && _RegisterString._str_list_cnt > 0)
    {
        _RegisterString.nArrayCnt = 0;
        _RegisterString.pArray = (TSTRING_RESOURCE **)E_MALLOC(_RegisterString._str_list_cnt * sizeof(TSTRING_RESOURCE *));
        if (_RegisterString.pArray != NULL)
        {
            node_t *pos_ptr = NULL;
            TSTRING_RESOURCE *pRes;
            pos_ptr = list_iter_head(&_RegisterString._str_list);

            while (list_iter_next(&_RegisterString._str_list, (void **)&pRes, &pos_ptr) && _RegisterString.nArrayCnt < _RegisterString._str_list_cnt)
            {
                _RegisterString.pArray[_RegisterString.nArrayCnt++] = pRes;
            }
        }

        _RegisterString.nAfterBeginId = _RegisterString.nBeginId + _RegisterString.nArrayCnt;
    }
}

//读取注册字符串
SECTION_PLT_CODE const TSTRING_RESOURCE * get_RegStringResource(TID id)
{
    if (id >= _RegisterString.nBuildInBeginId)
    {
        if (id < (int32)(_RegisterString.nBuildInResCnt + _RegisterString.nBuildInBeginId))
        {
            return &_RegisterString.pBuildInRes[id - _RegisterString.nBuildInBeginId];
        }
        else
        {
            if (_RegisterString.pArray != NULL)
            {
                if (id >= _RegisterString.nBeginId && id < (int32)(_RegisterString.nBeginId + _RegisterString.nArrayCnt))
                {
                    return _RegisterString.pArray[id - _RegisterString.nBeginId];
                }
            }
            else
            {
                if (id >= _RegisterString.nBeginId && id < (int32)(_RegisterString.nBeginId + _RegisterString._str_list_cnt))
                {
                    TSTRING_RESOURCE * pRes = NULL;
                    list_get(&_RegisterString._str_list, id - _RegisterString.nBeginId, (void **)&pRes);
                    return pRes;
                }
            }

            if (id >= _RegisterString.nAfterBeginId && id < (int32)(_RegisterString.nAfterBeginId + _RegisterString._str_list_after_cnt))
            {
                TSTRING_RESOURCE *pRes = NULL;
                list_get(&_RegisterString._str_list_after, id - _RegisterString.nAfterBeginId, (void **)&pRes);
                return pRes;
            }
        }
    }

    return NULL;
}

extern int8 i_GetCurLanguage(void);
//读取注册字符串
const char * get_RegStringResourceLocal(const TSTRING_RESOURCE * pRes)
{
    if (pRes != NULL)
    {
#ifdef _MANG_UNIT_

        int8 lang = i_GetCurLanguage();
        if (lang >= 0 && lang < LANGUAGE_COUNT)
        {
            return pRes->str[lang];
        }
        else
        {
            return pRes->str[0];        //返回默认语言
        }
#else
        return pRes->str[LANGUAGE_CN];      //总是返回中文语言
#endif
    }

    return "";
}

//读取注册字符串
SECTION_PLT_CODE const char * get_RegStringResourceStrLocal(TID id)
{
    const TSTRING_RESOURCE * pRes = get_RegStringResource(id);
    return get_RegStringResourceLocal(pRes);
}

//获取字符串资源总个数
SECTION_PLT_CODE int32 get_RegStringResourceCount(void)
{
    return (_RegisterString.nBuildInResCnt + _RegisterString._str_list_cnt + _RegisterString._str_list_after_cnt);
}

SECTION_PLT_CODE const char * get_InnerStringResourceLocal(const TINNER_STRING_RESOURCE * pRes)
{
    if (pRes != NULL)
    {
#ifdef _MANG_UNIT_
        if (i_GetCurLanguage() == LANGUAGE_CN)
        {
            return pRes->cn;
        }
        else
        {
            return pRes->en;
        }
#endif
        return pRes->cn;            //... 以后处理
    }

    return "";
}

