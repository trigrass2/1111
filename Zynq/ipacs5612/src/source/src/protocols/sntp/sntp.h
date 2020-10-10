#ifndef _SNTP_H
#define _SNTP_H

#include "mytypes.h"

#define SNTP_LI_MASK        0xC0
#define SNTP_LI_0           0x00           /* no warning  */
#define SNTP_LI_1           0x40           /* last minute has 61 seconds */
#define SNTP_LI_2           0x80           /* last minute has 59 seconds */
#define SNTP_LI_3           0xC0          /* 3 bit version number field */

#define SNTP_VN_MASK        0x38
#define SNTP_VN_0           0x00           /* not supported */
#define SNTP_VN_1           0x08           /* the earliest version */
#define SNTP_VN_2           0x10     
#define SNTP_VN_3           0x18           /* VxWorks implements this version */
#define SNTP_VN_4           0x20           /* the latest version, not yet solidified */
#define SNTP_VN_5           0x28           /* reserved */
#define SNTP_VN_6           0x30           /* reserved */
#define SNTP_VN_7           0x38           /* reserved *//* 3 bit mode field */

#define SNTP_MODE_MASK      0x07      
#define SNTP_MODE_0         0x00           /* reserve */
#define SNTP_MODE_1         0x01           /* symmetric active */
#define SNTP_MODE_2         0x02           /* symmetric passive */
#define SNTP_MODE_3         0x03           /* client */
#define SNTP_MODE_4         0x04           /* server */
#define SNTP_MODE_5         0x05           /* broadcast */
#define SNTP_MODE_6         0x06           /* reserve for NTP control 					      message */
#define SNTP_MODE_7         0x07           /* reserve for private use *//* 8 bit stratum number. Only the first 2 are valid for SNTP. */

#define SNTP_STRATUM_0      0x00           /* unspecified or unavailable */
#define SNTP_STRATUM_1      0x01           /* primary source */

#define SNTP_UNIX_OFFSET    0x83aa7e80     /* 1970 - 1900 in seconds */
#define TIMEVAL_USEC_MAX    1000000

#define SNTP_CLIENT_REQUEST 0x0B           /* standard SNTP client request */

typedef struct sntpPacket    
{    
     BYTE     leapVerMode;    
     BYTE     stratum;                     
	 char     poll;    
	 char     precision;    
	 DWORD    rootDelay;    
	 DWORD    rootDispersion;    
	 DWORD    referenceIdentifier;   
	 DWORD    referenceTimestampSec;    
	 DWORD    referenceTimestampFrac;    /* client transmission time, in 64-bit NTP timestamp format */    
	 DWORD    originateTimestampSec;    
	 DWORD    originateTimestampFrac;    /* server reception time, in 64-bit NTP timestamp format */    
	 DWORD    receiveTimestampSec;    
	 DWORD    receiveTimestampFrac;    /* server transmission time, in 64-bit NTP timestamp format */    
	 DWORD    transmitTimestampSec;    
	 DWORD    transmitTimestampFrac;    
} SNTP_PACKET;

#endif
