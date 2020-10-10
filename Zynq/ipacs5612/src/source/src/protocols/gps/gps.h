#ifndef _GPS_H
#define _GPS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mytypes.h"
void gps_isr(void);
typedef struct {
    BYTE	*buf;			/*?????*/
	WORD	size;			/*?????*/
	WORD	rp;				/*???*/
	WORD	wp;				/*???*/
}VCommBuf1;

class VGPSPCL{
	private:
	
	public: 		
		VGPSPCL();

		int m_thid;
		int m_commid;

        char v[20];
		BOOL m_bfirst;
		WORD m_cnt;
		VCommBuf1 m_Rec;	

		char AtoC(char c);
		int GpsCheck(BYTE* buf, DWORD len, DWORD *pdeallen);
		void RxdFrame(void);
		void NeatCommBuf(VCommBuf1 *pBuf);
		void Run(void);

};
void GpsTest(BYTE on);

#ifdef __cplusplus
}
#endif
#endif
