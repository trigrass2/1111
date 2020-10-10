/*
 * @Author: zhongwei
 * @Date: 2019-11-19 11:22:41
 * @LastEditors: zhongwei
 * @LastEditTime: 2019-11-19 11:24:41
 * @Description: LIB库统一定义
 * @FilePath: \ZynqBM\src\include\plt_Lib.h
 */

#ifndef SRC_INCLUDE_PLT_LIB_H_
#define SRC_INCLUDE_PLT_LIB_H_

#define STRUCT_SET(to, from)    (to) = (from)

#define E_MEMSET    memset
#define E_MEMCHR    memchr
#define E_MEMCHECK  memcheck
#define E_MEMCPY    memcpy
#define E_MEMZERO   memzero

#define E_ATOI      atoi
#define E_STRLEN    strlen

#define E_STRNCPY   strncpy         //原始的strncpy
#define E_STRNCAT   strncat         //原始的strncat

#define E_STRCPY    strcpy
#define E_STRCAT    strcat
#define E_STRCMP    strcmp

#define E_SPRINTF   sprintf
#define E_SNPRINTF  snprintf
#define E_VSPRINTF  vsprintf    
#define E_VSNPRINTF vsnprintf

/************************************************************************/
//  new/delete
/************************************************************************/
#ifdef __cplusplus          //只有c++支持
    #define E_NEW new
    #define E_DELETE delete
#endif

/************************************************************************/
//  malloc calloc free
/************************************************************************/

#ifdef _PROT_UNIT_
	extern void * mem_Malloc(size_t nSize);
    #define E_MALLOC    mem_Malloc
#else
    #define E_MALLOC    f_malloc
    #define E_CALLOC    calloc
    #define E_FREE      free
#endif

/************************************************************************/
//  else
/************************************************************************/
#define E_PRINTF    printf
#define E_STRTOK    strtok
#define E_SSCANF    sscanf
#define E_MEMMOVE   memmove
#define E_FREAD     fread
#define E_FGETS     fgets
#define E_FGETC     fgetc
#define E_OPEN      open
#define E_CLOSE     close
#define E_READ      read
#define E_WRITE     write
#define E_FPRINTF   fprintf
#define E_FOPEN     fopen
#define E_FCLOSE    fclose
#define E_FREAD     fread
#define E_FWRITE    fwrite
#define E_REMOVE    remove

#endif /* SRC_INCLUDE_PLT_LIB_H_ */
