#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syscfg.h"

void gb104s(WORD wTaskID);	
int ReadGb104sDeadValue(WORD parano,char*pbuf);
int WriteGb104sDeadValue(WORD parano,char*pbuf);
int WriteGb104sDeadValueFile(void);

void gb101s(WORD wTaskID);	
void ctpri(WORD thid);
void ctsec(WORD thid);
void maint(WORD wTaskID);

void mmis(WORD wTaskID);
void modbusm(WORD wTaskID);
void gb104m(WORD wTaskID);
void gb101m(WORD wTaskID);		
//void ctpri(WORD wTaskID);
//void ctsec(WORD wTaskID);
void gps(WORD wTaskID);
void sac103m(WORD wTaskID);
void ipacs103m(WORD wTaskID);
void gb103m(WORD wTaskID);
void mdcu101m(WORD wTaskID);
#ifdef INCLUDE_FA_SH
void gb104shs(WORD wTaskID);
void gb104shm(WORD wTaskID);
#endif
#ifdef INCLUDE_CDT
void cdt92m(WORD wTaskID);
void cdt92s(WORD wTaskID);
#endif
#ifdef INCLUDE_FA_S
void mdcp(WORD wTaskID);
#endif

void dlqm(WORD wTaskID);
void drqm(WORD wTaskID);

#if (TYPE_USER == USER_SHANGHAIJY)
void sf103m(WORD wTaskID);
void pn103m(WORD wTaskID);	
#endif

#ifdef __cplusplus
}
#endif
#endif
