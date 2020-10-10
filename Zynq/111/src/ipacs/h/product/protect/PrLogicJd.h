#ifndef _PRLOGICJD_H
#define _PRLOGICJD_H

void StartWavJdRcd(int fd);
void WaveJd_Interrupt(void);

void PrJdJudgeInit(void);
void PrJdJudge(void);
#endif
