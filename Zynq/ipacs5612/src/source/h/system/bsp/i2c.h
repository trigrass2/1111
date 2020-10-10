#ifndef _I2C_H
#define _I2C_H

#include "mytypes.h"

void I2CInit(int no);

int I2CSendBuffer_WAddr(int no,BYTE *buf, BYTE id, WORD addr, DWORD len);
int I2CReceBuffer_WAddr(int no,BYTE *buf, BYTE id, WORD addr, DWORD len);

int I2CSendBuffer_BAddr(int no,BYTE *buf, BYTE id, BYTE addr, DWORD len);
int I2CReceBuffer_BAddr(int no,BYTE *buf, BYTE id, BYTE addr, DWORD len);

#endif // !_I2C_H
