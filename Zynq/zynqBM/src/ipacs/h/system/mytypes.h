/*------------------------------------------------------------------------
 Module:		mytypes.h
 Author:		solar
 Project:		 
 Creation Date: 2008-10-29
 Description:	
------------------------------------------------------------------------*/

#ifndef _MYTYPES_H
#define _MYTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#if	!defined(EOF) || (EOF!=(-1))
#define EOF		(-1)
#endif

#if	!defined(EOS) || (EOS!='\0')
#define EOS		'\0'	
#endif

#if	!defined(FALSE) || (FALSE!=0)
#define FALSE		0
#endif

#if	!defined(TRUE) || (TRUE!=1)
#define TRUE		1
#endif

#if	!defined(false) || (false!=0)
#define false		0
#endif

#if	!defined(true) || (true!=1)
#define true		1
#endif

typedef struct{
		unsigned char mtsl;
		unsigned char mtsh;
		unsigned char exps;
}trifloat;

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

#ifdef __cplusplus
typedef void (*ENTRYPTR)(...);     /* ptr to function returning void */
#else
typedef void (*ENTRYPTR) ();	   /* ptr to function returning void */
#endif			/* _cplusplus */

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned long  uint32;

typedef signed char	   int8;
typedef signed short   int16;
typedef signed long	   int32;

typedef unsigned char   INT8U;      /* Unsigned  8 bit quantity                                        */
typedef signed   char   INT8S;      /* Signed    8 bit quantity                                        */
typedef unsigned short  INT16U;     /* Unsigned 16 bit quantity                                        */
typedef signed   short  INT16S;     /* Signed   16 bit quantity                                        */
typedef unsigned int    INT32U;     /* Unsigned 32 bit quantity                                        */
typedef signed   int    INT32S;     /* Signed   32 bit quantity                                        */

#define  _BIG_ENDIAN        1
#define  _LITTILE_ENDIAN    2

#if (TYPE_CPU == CPU_STM32F4) || (TYPE_CPU == CPU_SAM9X25) || (TYPE_CPU == CPU_ZYNQ)
#define _BYTE_ORDER        _LITTILE_ENDIAN
#endif


#if (TYPE_OS == OS_RT)
#define OK         0
#define ERROR     -1

#define WAIT_FOREVER  -1

#define FAST	  register
#define IMPORT	  extern
#define LOCAL	  static

typedef int       STATUS;
typedef char      BOOL;
#endif

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;


#if (TYPE_OS == OS_RT)
#define myinline    static __inline
#endif

#define LOBYTE(w)     ((BYTE)(w))
#define HIBYTE(w)     ((BYTE)((WORD)(w) >> 8))
#define LOWORD(l)     ((WORD)(l))
#define HIWORD(l)     ((WORD)((DWORD)(l) >> 16))
#define MAKEWORD(low,high)  ((WORD)((BYTE)(low)|(((WORD)((BYTE)(high)))<< 8)))
#define MAKEDWORD(low,high) ((DWORD)(((WORD)(low))|(((DWORD)((WORD)(high)))<< 16)))
#define ADJUSTWORD(x)   (MAKEWORD(HIBYTE(x),LOBYTE(x)))
#define ADJUSTDWORD(x)  (MAKEDWORD(ADJUSTWORD(HIWORD(x)),ADJUSTWORD(LOWORD(x))))

#define BYTE2BCD(v)     ((((BYTE)v/10)<<4)|((BYTE)v%10))
#define BCD2BYTE(v)     (((BYTE)v>>4)*10+((BYTE)v&0x0F))
#define WORD2BCD(v)     ((BYTE2BCD((WORD)v/100)<<8) | (BYTE2BCD((WORD)v%100)))
#define BCD2WORD(v)     (BCD2BYTE((WORD)v>>8) *100 + BCD2BYTE((WORD)v&0xFF))
#define DWORD2BCD(v)    (WORD2BCD((DWORD)v/10000)<<16) | (WORD2BCD((DWORD)v%10000)))
#define BCD2DWORD(v)    (BCD2WORD((DWORD)v>>16) *10000 + BCD2WORD((DWORD)v&0xFFFF))

#ifdef __cplusplus
}
#endif


#endif /*_MYTYPES_H*/
