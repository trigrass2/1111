/*------------------------------------------------------------------------
 Module:		spi.h
 Author:		
 Project:		
 Creation Date: 
 Description:	
------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 $Log: $
------------------------------------------------------------------------*/
#ifndef _SPI_H_
#define _SPI_H_
#include "bsp.h"

void SPIInit(int no, int baud, int bytes);
int SPITrans_BAddr(int no,BYTE phy_no,BYTE *send,BYTE *recv,int len);

#endif /* _I2C_H */

