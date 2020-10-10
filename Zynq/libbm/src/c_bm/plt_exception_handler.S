#include "xil_errata.h"
#include "bspconfig.h"

.global _GetSysModeSp
.global _GetIrqModeSp
.global _GetSysModeLr
.global _GetIrqModeLr

/*
	  获取USR/SYS模式的SP值
*/
_GetSysModeSp:
 mrs r3, cpsr	          /* 读 cpsr_c */ 
	and r2, r3, #0xff
	msr cpsr_c, #0xdf	       /* 切换到SYS模式，并且关中断 */
	DSB
	mov r0, sp
	msr cpsr_c, r3
 DSB
	BX LR

_GetSysModeLr:
 mrs r3, cpsr	          /* 读 cpsr_c */ 
	and r2, r3, #0xff
	msr cpsr_c, #0xdf	       /* 切换到SYS模式，并且关中断 */
	DSB
	mov r0, lr
	msr cpsr_c, r3
 DSB
	BX LR

/*
	  获取IRQ模式的SP值
*/
_GetIrqModeSp:
 mrs r3, cpsr	          /* 读 cpsr_c */ 
	and r2, r3, #0xff
	msr cpsr_c, #0xd2	       /* 切换到IRQ模式，并且关中断 */
	DSB
	mov r0, sp
	msr cpsr_c, r3
 DSB
	BX LR

_GetIrqModeLr:
 mrs r3, cpsr	          /* 读 cpsr_c */ 
	and r2, r3, #0xff
	msr cpsr_c, #0xd2	       /* 切换到IRQ模式，并且关中断 */
	DSB
	mov r0, lr
	msr cpsr_c, r3
 DSB
	BX LR
