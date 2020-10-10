#ifndef HT1625_H
#define HT1625_H
#ifdef __cplusplus
extern "C" {
#endif
enum{
	C0,
	C1,
	C2,
	C3,
	C4,
	C5,
	C6,
	C7,
	C8,
	C9,
	CA,
	CB,
	CC,
	CD,
	CE,
	CF,
	CDOT,
	CNULL,
	CH,
	CR,
	CO,
	CK,
	CY,
	CX,
	CS,
	CL,
	CT,
	CP,
	CV,
	CI,
	CW,
	CM,
	CN
	
};
#define LIGHT0 MCF_GPIO_CLRUC=~0x1//=P3^6;  //±³¹â
#define LIGHT1 MCF_GPIO_SETUC=0x1//=P3^6;  //±³¹â
void test();
void HT1625_init();
void dispvol();
void disp_I();
void disp_P();
void disp_Q();
void sendtoHT1625();
void clearHT1625();
void disp_DC();
void disp_HZ();
void disp_cos();
void disp_YK_select();
void disp_YK_act();
void dispASCII(BYTE off,char cc);
void dispchar(BYTE off,BYTE data);
void dispdigital(BYTE off,BYTE data);
void dispyear(BYTE flag);
void dispYB(char*name,BYTE flag);
void dispalllight();
void dispyx();
#ifdef __cplusplus
}
#endif

#endif
