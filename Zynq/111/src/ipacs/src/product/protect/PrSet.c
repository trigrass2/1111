#include "syscfg.h"
#include "sys.h"
#include "myio.h"

#ifdef INCLUDE_PR
#include "PrSet.h"

BYTE prTableBuf[MAX_TABLE_FILE_SIZE];
static BYTE prSetTmp[PR_SET_SIZE+sizeof(struct VFileHead)];
BYTE FileSetNum = 0;

/*Ϊ��������ǰ�ĳ���ֵ���ݣ���ֵ�Ժ�������������*/

//������������ ȱʡֵ��
TSETTABLE tSetTable[]={
  {"��ѹ�塭��������",{0x20,0x00,0x00,0x00,0x00,0x00},{0x20,0x00,0xff,0xff,0xff,0xff},{0x20,0x00,0x01,0x00,0x00,0x00}},
  {"������һ��������",{0x21,0x00,0x00,0x00,0x00,0x00},{0x21,0x00,0xff,0xff,0xff,0xff},{0x21,0x00,0x00,0x0F,0x00,0x00}},//0000��FFFF
  {"�����ֶ���������",{0x21,0x00,0x00,0x00,0x00,0x00},{0x21,0x00,0xff,0xff,0xff,0xff},{0x21,0x00,0x00,0x00,0x00,0x00}},//0000��FFFF

  {"������Ρ�������",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20��100.0
  {"�������ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},//0.00��20.00
  {"������Ρ�������",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20��100.0
  {"�������ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},//0.00��20.00
  {"������Ρ�������",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20��100.0
  {"�������ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x99,0x00,0x00},{0x25,0x02,0x00,0x99,0x00,0x00}},//0.00��20.00
  {"�����Ƕ����ޡ���",{0x27,0x82,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x00,0x00,0x00}},//0.20��100.0
  {"�����Ƕ����ޡ���",{0x27,0x82,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x00,0x00,0x00}},//0.00��20.00
  {"�����ŵ���������",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00}},//0.20��100.0
  {"������ʱ�䡭����",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x20,0x01,0x00},{0x25,0x02,0x00,0x20,0x01,0x00}},//0.10��60.00
  
  {"���������ɡ�����",{0x24,0x02,0x20,0x00,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00}},//0.20��100.0
  {"����������ʱ�䡭",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x72,0x00},{0x25,0x02,0x00,0x00,0x72,0x00}},//0.00��20.0 0.014
  {"�����ڶϵ�������",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20��100.0
  {"����ѹ����������",{0x23,0x02,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x40,0x00,0x00},{0x23,0x02,0x00,0x01,0x00,0x00}},//0~250
  {"����ѹʱ�䡭����",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},

  {"�͵�ѹ����������",{0x23,0x02,0x00,0x20,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x00,0x07,0x00,0x00}},
  {"�͵�ѹʱ�䡭����",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"��ѹ������ѹ����",{0x2B,0x02,0x00,0x01,0x00,0x00},{0x2B,0x01,0x00,0x10,0x00,0x00},{0x2B,0x01,0x00,0x01,0x00,0x00}},
  {"��·��ѹ��������",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x00,0x07,0x00,0x00}},
  {"��·��ѹ��������",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x00,0x03,0x00,0x00}}, 
  {"��ѹ������������",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x10,0x00,0x00},{0x23,0x01,0x00,0x10,0x00,0x00}},//0.8~1.5Un
  {"������Ρ�������",{0x24,0x02,0x01,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.8~1.5Un
  {"�������ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"������Ρ�������",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00}},//0.8~1.5Un
  {"�������ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x72,0x00},{0x25,0x02,0x00,0x20,0x00,0x00}},
  {"������Ρ�������",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00},{0x24,0x01,0x00,0x10,0x00,0x00}},//0.8~1.5Un
  {"�������ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x72,0x00},{0x25,0x02,0x00,0x20,0x00,0x00}},
  {"�����Ƕ����ޡ���",{0x27,0x82,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x00,0x00,0x00}},//0.20��100.0
  {"�����Ƕ����ޡ���",{0x27,0x82,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x80,0x01,0x00},{0x27,0x02,0x00,0x00,0x00,0x00}},//0.00��20.00
  {"С������ѹͻ�䡭",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x10,0x00,0x00,0x00}}, 
  {"С��������ͻ�䡭",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x05,0x00,0x00,0x00}}, 
  {"С��������ͻ�䡭",{0x24,0x02,0x00,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x10,0x00,0x00,0x00}},
  {"С�����ӵ�ʱ�䡭",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x05,0x00,0x00}},
  {"С�����ӵ�ϵ����",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x10,0x00,0x00,0x00},{0x25,0x01,0x05,0x00,0x00,0x00}},

  {"�����غ�բ�����",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},//0.20��100.0
  {"�غ�բ����������",{0x22,0x01,0x00,0x00,0x00,0x00},{0x22,0x01,0x30,0x00,0x00,0x00},{0x22,0x01,0x10,0x00,0x00,0x00}},//0.8~1.5Un
  {"һ��ʱ�䡭������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x06,0x00},{0x25,0x02,0x00,0x03,0x00,0x00}},//0.8~1.5Un
  {"����ʱ�䡭������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x20,0x01,0x00}},
  {"����ʱ�䡭������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x20,0x01,0x00}},//0.8~1.5Un
  {"�غϱ���ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x05,0x00,0x00}},//0.8~1.5Un
  {"�źű���ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x10,0x00,0x00}},
  {"���ϵƸ���ʱ�䡭",{0x25,0x01,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x00,0x32,0x04},{0x25,0x01,0x00,0x80,0x28,0x00}},
  {"�����ʱ�䡭����",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x20,0x00,0x00,0x00}},
  {"����ٸ���ʱ�䡭",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x00,0x20,0x01,0x00}},
  {"�ⲿң����բ����",{0x22,0x00,0x00,0x00,0x00,0x00},{0x22,0x00,0x00,0x03,0x00,0x00},{0x22,0x00,0x00,0x03,0x00,0x00}},

  {"��Ƶ������ֵ����",{0x29,0x02,0x00,0x45,0x00,0x00},{0x29,0x02,0x00,0x60,0x00,0x00},{0x29,0x02,0x00,0x60,0x00,0x00}},
  {"��Ƶ����ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"��Ƶ������ֵ����",{0x29,0x02,0x00,0x45,0x00,0x00},{0x29,0x02,0x00,0x60,0x00,0x00},{0x29,0x02,0x00,0x45,0x00,0x00}},
  {"��Ƶ����ʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"��������ٵ�����",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},
  {"�������ٵ�����",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},
  {"��������ʱ�䡭",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x03,0x00},{0x25,0x02,0x20,0x00,0x00,0x00}},
  {"ĸ����ѹ��������",{0x23,0x02,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x80,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00}},//0~250
  {"ĸ����ѹʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},
  {"ĸ��Ƿѹ��������",{0x23,0x02,0x00,0x20,0x00,0x00},{0x23,0x01,0x00,0x22,0x00,0x00},{0x23,0x01,0x00,0x07,0x00,0x00}},
  {"ĸ��Ƿѹʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x00,0x01,0x00}},

};
BYTE SET_NUMBER = sizeof(tSetTable)/sizeof(tSetTable[0]);

TYBTABLE tYbTable[] =
{
  {"���ϼ�⡭������", 0x00,  YB_GZJC},
  {"����һ�Ρ�������", 0x02,  YB_I1},
  {"�������Ρ�������", 0x02,  YB_I2},
  {"�������Ρ�������", 0x02,  YB_I3},
  {"�����ɡ���������", 0x02,  YB_GFH},
  {"��ѹ������������", 0x02,  YB_GY},
  {"��ѹ���ء�������", 0x02,  YB_DY},
  {"����һ�Ρ�������", 0x02,  YB_I01},
  {"�������Ρ�������", 0x02,  YB_I02},
  {"�������Ρ�������", 0x02,  YB_I03},
  {"����ӿ����������", 0x02,  YB_DJQD},
  {"PT���ߡ���������", 0x02,  YB_PT},
  {"�غ�բ����������", 0x02,  YB_CH},
  {"�������١�������", 0x02,  YB_JS},
  {"��Ƶ������������", 0x02,  YB_GP},
  {"��Ƶ������������", 0x02,  YB_DP},
  {"ĸ��Ƿѹ��������", 0x02,  YB_MXQY},
  {"�ⲿң����բͶ��",0x02,YB_TZ},
  
};
BYTE YB_NUMBER=sizeof(tYbTable)/sizeof(tYbTable[0]);

char* szFsxCuv[] = {"����1   ","����2   ","����3   "};
char* szQd[]     = {"��ʱ��  ","��ʱ��  "};
char* szDz[]     = {"����    ","��բ    "};
char* szBS[]     = {"������  ","����    "};
char* bsJS[]     = {"�Զ�����","�ֶ�����"};
char *szTt[]     = {"�˳�    ","Ͷ��    "};
char *szJr[]     = {"δ����  ","����    "};
char* szPMx[]    = {"ָ����·","ָ��ĸ��"};
char* szCd[]     = {"��������","�̵籣��"};
char* szTrip[]   = {"����    ","����ʧѹ","����ʧѹ","��������","����բ  "};
char* szReport[] = {"����    ","��բ    ","�غ�ʧ��"};
char* szKgMd[]   = {"��·    ","����    "};
char* szFaultDz[]   = {"�澯    ","��բ    "};
char *szXDLJr[]     = {"��ѹ    ","���ѹ  "};


TKGTABLE tKG1Table[] =
{
  {"��ʱ�����ߡ�����",0x02, {KG_0,KG_1,0xff}, 2, 3,szFsxCuv},
  {"�����ɱ���������",0x02, {KG_2,0xff,0xff}, 1, 2,szQd},
  {"�����ɹ��϶�����",0x02, {KG_3,0xff,0xff}, 1, 2,szDz},
  {"PT���߱���������",0x02, {KG_4,0xff,0xff}, 1, 2,szBS},
  {"�͵�ѹ����������",0x02, {KG_5,0xff,0xff}, 1, 2,szBS},
  {"��������Ԫ������",0x02, {KG_6,0xff,0xff}, 1, 2,szTt},
  {"��������ָ�򡭡�",0x02, {KG_7,0xff,0xff}, 1, 2,szPMx},
  {"����ѹԪ��������",0x02, {KG_8,0xff,0xff}, 1, 2,szTt},
  {"UA���ѹ��������",0x02, {KG_9,0xff,0xff}, 1, 2,szJr},
  {"UB���ѹ��������",0x02, {KG_10,0xff,0xff}, 1, 2,szJr},
  {"UC���ѹ��������",0x02, {KG_11,0xff,0xff},1, 2,szJr},
  {"��������ָ�򡭡�",0x02, {KG_12,0xff,0xff},1, 2,szPMx},
  {"��������Ԫ������",0x02, {KG_13,0xff,0xff},1, 2,szTt},
  {"������ѹ��������",0x02, {KG_14,0xff,0xff},1, 2,szTt},
  {"������غϱ�����",0x02, {KG_15,0xff,0xff},1, 2,szTt},
  {"�غ�բ���ģʽ��",0x02, {KG_16,0xff,0xff},1, 2,szCd},
  {"�����ź���������",0x02, {KG_17,KG_18,KG_19}, 3, 5, szTrip},
  {"�����ϱ���������",0x02, {KG_20,KG_21,0xff},2, 3, szReport},
  {"��·��ѹ��⡭��",0x02, {KG_22,0xff,0xff},1, 2, szTt},
  {"������������¼��",0x02, {KG_23,0xff,0xff},1, 2, szTt},
  {"���ڶϵ���������",0x02, {KG_24,0xff,0xff},1, 2, szTt},
  {"����ӿ��г���ƶ�",0x02, {KG_25,0xff,0xff},1, 2, szTt},
  {"���ϵ��Զ����顭",0x02, {KG_26,0xff,0xff},1, 2, szTt},
  {"����һ�γ��ڡ���",0x02, {KG_27,0xff,0xff},1, 2, szFaultDz},
  {"�������γ��ڡ���",0x02, {KG_28,0xff,0xff},1, 2, szFaultDz},
  {"����������ڡ���",0x02, {KG_29,0xff,0xff},1, 2, szFaultDz},
  {"�������͵�ѹ����",0x02, {KG_30,0xff,0xff}, 1, 2,szBS},
  {"�������η���Ԫ��",0x02, {KG_31,0xff,0xff},1, 2,szTt},
};
const BYTE KG1_NUMBER=sizeof(tKG1Table)/sizeof(tKG1Table[0]);

char* szSgzYxMode[] = {"����Ԫ��", "��      "};
char* szSignMode[] =  {"��      ", "��      "};
char* sZCHZMode[] = {"���� ",  "����ѹ"};
TKGTABLE tKG2Table[] = 
{
  {"����ң�����롭��",0x02, {KG_2,0xff,0xff},1,2,szSgzYxMode},
  {"���ؽ������ԡ���",0x02, {KG_3,0xff,0xff},1,2,szSignMode},
  {"��������¼������",0x02, {KG_4,0xff,0xff},1, 2, szTt},
  {"��·ʧѹ¼������",0x02, {KG_5,0xff,0xff},1, 2, szTt},
  {"�����ѹ¼������",0x02, {KG_6,0xff,0xff},1, 2, szTt},
  {"�������¼������",0x02, {KG_7,0xff,0xff},1, 2, szTt},
  {"С�����ӵ�Ͷ�롭",0x02, {KG_8,0xff,0xff},1, 2, szTt},
  {"С�����ӵس��ڡ�",0x02, {KG_9,0xff,0xff},1, 2, szFaultDz},
  {"С�������뷽ʽ��",0x02, {KG_10,0xff,0xff},1, 2, szXDLJr},
  {"��������¼������",0x02, {KG_11,0xff,0xff},1, 2, szTt},
  {"������բ�쳣Ͷ��",0x02, {KG_12,0xff,0xff},1, 2, szTt},
  {"�ӵ��жϵ�������",0x02, {KG_13,0xff,0xff},1,2,szSignMode},
  {"��ѹ���϶���",0x02, {KG_14,0xff,0xff},1, 2, szDz},
  {"��ѹ���϶���",0x02, {KG_15,0xff,0xff},1, 2, szDz},
  {"�������ͻ��¼��",0x02, {KG_16,0xff,0xff},1, 2, szTt},
  {"��Ƶ���϶�������",0x02, {KG_17,0xff,0xff},1, 2, szDz},
  {"��Ƶ���϶�������",0x02, {KG_18,0xff,0xff},1, 2, szDz},
  {"�������γ��ڡ���",0x02, {KG_19,0xff,0xff},1, 2, szFaultDz},
  {"ĸ��Ƿѹ���ڡ���",0x02, {KG_20,0xff,0xff},1, 2, szFaultDz},
  
};
BYTE KG2_NUMBER=sizeof(tKG2Table)/sizeof(tKG2Table[0]);

TSETTABLE tSetPubTable[]={
  {"��ѹ�塭��������",{0x20,0x00,0x00,0x00,0x00,0x00},{0x20,0x00,0xff,0xff,0xff,0xff},{0x20,0x00,0x00,0x00,0x00,0x00}},
  {"������һ��������",{0x21,0x00,0x00,0x00,0x00,0x00},{0x21,0x00,0xff,0xff,0xff,0xff},{0x21,0x00,0x00,0x00,0x00,0x00}},//0000��FFFF
  {"�����ֶ���������",{0x21,0x00,0x00,0x00,0x00,0x00},{0x21,0x00,0xff,0xff,0xff,0xff},{0x21,0x00,0x00,0x00,0x00,0x00}},//0000��FFFF

  {"SL���ߡ���������",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x20,0x00,0x00,0x00},{0x2D,0x00,0x01,0x00,0x00,0x00}},
  {"X ʱ�ޡ���������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x00,0x01,0x00},{0x25,0x01,0x70,0x00,0x00,0x00}},
  {"Y ʱ�ޡ���������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x20,0x00,0x00},{0x25,0x01,0x50,0x00,0x00,0x00}},
  {"��բ��������ʱ��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x20,0x00,0x00},{0x25,0x01,0x00,0x00,0x00,0x00}},
  {"��ѹʱ�䡭������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x02,0x00,0x32,0x00,0x00}},
  {"��ѹ������������",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x10,0x00,0x00},{0x23,0x01,0x00,0x10,0x00,0x00}},//0.8~1.5Un
  {"˲ѹ��ѹʱ�䡭��",{0x25,0x03,0x05,0x00,0x00,0x00},{0x25,0x03,0x00,0x00,0x01,0x00},{0x25,0x03,0x00,0x01,0x00,0x00}},
  {"��ѹ������������",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x20,0x00,0x00},{0x23,0x01,0x00,0x03,0x00,0x00}},
  {"����ѹ�������",{0x23,0x01,0x00,0x00,0x00,0x00},{0x23,0x01,0x00,0x03,0x00,0x00},{0x23,0x01,0x00,0x03,0x00,0x00}},
  {"ʧѹ��բʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x35,0x00,0x00,0x00}},
  {"����������������",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x09,0x00,0x00,0x00},{0x2D,0x00,0x01,0x00,0x00,0x00}},
  {"����������λʱ��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x35,0x00,0x00,0x00}},
  {"���������ۼ�ʱ��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x35,0x00,0x00,0x00}}, 
#ifdef INCLUDE_PR_PRO
  {"ͬ�ڵ�Ƶ���ߡ���",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x20,0x00,0x00,0x00},{0x2D,0x00,0x01,0x00,0x00,0x00}},
  {"��Ƶ����Ƶ�ʻ���",{0x2A,0x02,0x50,0x00,0x00,0x00},{0x2A,0x01,0x00,0x02,0x00,0x00},{0x2A,0x01,0x10,0x00,0x00,0x00}},
  {"��Ƶ��ֵ��������",{0x29,0x02,0x00,0x45,0x00,0x00},{0x29,0x01,0x95,0x04,0x00,0x00},{0x29,0x01,0x90,0x04,0x00,0x00}},
  {"��Ƶʱ�䡭������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x00,0x01,0x00},{0x25,0x02,0x00,0x20,0x00,0x00}},
  {"��Ƶ��ѹ��������",{0x23,0x02,0x00,0x15,0x00,0x00},{0x23,0x02,0x00,0x20,0x01,0x00},{0x23,0x02,0x00,0x20,0x01,0x00}},
  {"��Ƶ������������",{0x24,0x02,0x04,0x00,0x00,0x00},{0x24,0x02,0x00,0x00,0x01,0x00},{0x24,0x02,0x00,0x00,0x01,0x00}},
  {"ͬ�ڽǲ������",{0x27,0x02,0x00,0x10,0x00,0x00},{0x27,0x02,0x00,0x50,0x00,0x00},{0x27,0x02,0x00,0x30,0x00,0x00}},
  {"��բ��ǰʱ�䡭��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x02,0x00,0x02,0x00,0x00},{0x25,0x02,0x20,0x00,0x00,0x00}},
  {"׼ͬ�ڵ�ѹ���",{0x23,0x02,0x00,0x00,0x00,0x00},{0x23,0x02,0x00,0x20,0x00,0x00},{0x23,0x02,0x00,0x00,0x00,0x00}},
  {"׼ͬ��Ƶ�ʲ��",{0x29,0x02,0x00,0x00,0x00,0x00},{0x29,0x02,0x00,0x02,0x00,0x00},{0x29,0x02,0x50,0x00,0x00,0x00}},
  {"׼ͬ�ڼ��ٶȡ���",{0x2A,0x02,0x00,0x00,0x00,0x00},{0x2A,0x02,0x00,0x05,0x00,0x00},{0x2A,0x02,0x00,0x01,0x00,0x00}},
 #endif
  {" C ʱ�䡭��������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x20,0x00,0x00},{0x25,0x01,0x00,0x03,0x00,0x00}},
  {" S ʱ�䡭��������",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x00,0x01,0x00},{0x25,0x01,0x70,0x05,0x00,0x00}},
  {"ѡ����բ�غ�ʱ��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x20,0x00,0x00},{0x25,0x01,0x20,0x00,0x00,0x00}},
  {"�ӵع�������ʱ��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00}},
  {"�ӵع��Ϸ���ʱ��",{0x25,0x02,0x00,0x00,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00},{0x25,0x01,0x00,0x10,0x00,0x00}},
  {"ѹ��ϻ��ǲ��",{0x27,0x02,0x00,0x10,0x00,0x00},{0x27,0x02,0x00,0x50,0x00,0x00},{0x27,0x02,0x00,0x30,0x00,0x00}},
  {"¼������(����)��",{0x2D,0x00,0x20,0x00,0x00,0x00},{0x2D,0x00,0x28,0x01,0x00,0x00},{0x2D,0x00,0x64,0x00,0x00,0x00}},

  {"��ѹ�����ۼƴ���",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x00,0x01,0x00,0x00},{0x2D,0x00,0x04,0x00,0x00,0x00}},
  {"��ѹ�����ۼ�ʱ��",{0x25,0x00,0x00,0x00,0x00,0x00},{0x25,0x00,0x00,0x09,0x00,0x00},{0x25,0x00,0x00,0x03,0x00,0x00}},
  {"��ѹ����һ���ߡ�",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x20,0x00,0x00,0x00},{0x2D,0x00,0x02,0x00,0x00,0x00}},
  {"ĸ�߲��������",{0x24,0x02,0x05,0x00,0x00,0x00},{0x24,0x02,0x00,0x50,0x01,0x00},{0x24,0x02,0x00,0x50,0x01,0x00}},
 
  {"������բ�ۼƴ���",{0x2D,0x00,0x01,0x00,0x00,0x00},{0x2D,0x00,0x00,0x01,0x00,0x00},{0x2D,0x00,0x03,0x00,0x00,0x00}},//add by wdj 
  {"������բ�趨ʱ��",{0x25,0x00,0x00,0x00,0x00,0x00},{0x25,0x00,0x00,0x60,0x00,0x00},{0x25,0x00,0x00,0x06,0x00,0x00}},

  //��ӵ�ѹʱ����ͨ�����64��Ĭ��0��Ч
  {"��Դ���ѹͨ����",{0x2D,0x00,0x00,0x00,0x00,0x00},{0x2D,0x00,0x64,0x00,0x00,0x00},{0x2D,0x00,0x00,0x00,0x00,0x00}},
  {"���ɲ��ѹͨ����",{0x2D,0x00,0x00,0x00,0x00,0x00},{0x2D,0x00,0x64,0x00,0x00,0x00},{0x2D,0x00,0x00,0x00,0x00,0x00}},
};
 
BYTE SET_PUB_NUMBER = sizeof(tSetPubTable)/sizeof(tSetPubTable[0]);

TYBTABLE tYbPubTable[] =
{
  {"S ���ܡ���������", 0x02,  YB_SUDY},
  {"L ���ܡ���������", 0x02,  YB_SUSY},
  {"��ѹ������������", 0x02,  YB_U0},
  {"ѹ��ϻ����ܡ���", 0x02,  YB_UDIFF},
  {"��ѹ��բ��������", 0x02,  YB_SLWY},
  {"����������������", 0x02,  YB_DLCNT},
#ifdef INCLUDE_PR_PRO
  {"��Ƶ���ء�������", 0x02,  YB_DF},
  {"�ֶ�ͬ�ں�բ����", 0x02,  YB_SHTQ},
#endif
  {"�ӵع��ϴ�����", 0x02,  YB_JD},
  {"Ӳ����ѹģ�鴦��", 0x02,  YB_YJCY},
  {"ĸ�߲Ͷ�롭��",0x02,   YB_BUS},

  {"������բ������բ",0x02,	YB_LXFZ}, //add by wdj

};
BYTE YB_PUB_NUMBER=sizeof(tYbPubTable)/sizeof(tYbPubTable[0]);

char* szUIType[]  = {"��ѹ��  ","��ѹ����"};
char* szPower[]   = {"UA      ","UC      "};
char* szPubLine[] = {"������  ","˫����  "};
#ifdef INCLUDE_PR_PRO
char* szDf[]     = {"����    ","����    "};
char* szChMode[] = {"����    ","����ѹ  ","��ͬ��  ","��ѹͬ��"};
char* szShMode[] = {"����    ","����ѹ  ","��ͬ��  ","׼ͬ��  ","��ѹͬ��","��ѹ׼ͬ"};
char* szWyAny[]  = {"����·��","����һ��"};
char* szTqLine[] = {"������  ","˫����  "};
#endif
TKGTABLE tKG1PubTable[] = 
{
  {"X ʱ�ޱ���������",0x02, {KG_0,0xff,0xff},1, 2,szBS},
  {"Y ʱ�ޱ���������",0x02, {KG_1,0xff,0xff},1, 2,szBS},
  {"˫ѹ������������",0x02, {KG_2,0xff,0xff},1, 2,szBS},
  {"˲ѹ������������",0x02, {KG_3,0xff,0xff},1, 2,szBS},
  {"��ѹ�����͡�����",0x02, {KG_4,0xff,0xff},1, 2,szUIType},
  {"��ѹ�����жϡ���",0x02, {KG_5,0xff,0xff},1, 2,szDz},
#ifdef INCLUDE_PR_PRO
  {"��Ƶ������������",0x02, {KG_6,0xff,0xff},1, 2,szDf},
  {"��Ƶ������������",0x02, {KG_7,0xff,0xff},1, 2,szTt},
  {"�غ�բ��ʽ������",0x02, {KG_8,KG_9,0xff},2,4,szChMode},
  {"�ֶ���բ��ʽ����",0x02, {KG_10,KG_11,KG_12},3,6,szShMode},
  {"�غ���ѹ��顭��",0x02, {KG_13,0xff,0xff},1, 2,szWyAny},
  {"ͬ�ڵ����ߡ�����",0x02, {KG_14,0xff,0xff},1, 2,szTqLine},
#endif
  {"��բ����������ʽ",0x02, {KG_15,0xff,0xff},1, 2,bsJS},
  {"�׶�FTU Ͷ�롭��",0x02, {KG_16,0xff,0xff},1, 2,szTt},	
  {"����Ӧ����·��",0x02, {KG_17,0xff,0xff},1, 2,szTt},
  {"����Ӧ����ӵء�",0x02, {KG_18,0xff,0xff},1, 2,szTt},
  {"��Դͨ����������",0x02, {KG_19,0xff,0xff},1, 2,szPower},
  {"�ϳɹ���բ������",0x02, {KG_20,0xff,0xff},1, 2,szBS},
  {"��ѹ�ͱ���������",0x02, {KG_21,0xff,0xff},1, 2,szPubLine},    
  {"��һ���׶�Ͷ�롭",0x02, {KG_22,0xff,0xff},1, 2,szTt},  
  {"ĸ�߹��϶�������",0x02, {KG_23,0xff,0xff},1, 2,szDz},  
};

BYTE KG1_PUB_NUMBER=sizeof(tKG1PubTable)/sizeof(tKG1PubTable[0]);

char* szWorkMode[] ={"�ֶ�    ","����    ","����    ","�ֽ�    "};
char* faWorkMode[] ={"Ĭ�� ","��ѹ��","������","����Ӧ��"};
#if (TYPE_USER == USER_BEIJING)
char* ptKgMode[] = {"���ɿ���","��ſ���"};
#endif
#if (TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_WDG)
char* ptAutoMode[] = {"Ͷ��","�˳�"};
#endif
char* ptFaTripMode[] = {"������","����"};
char* ptDLXBSMode[] = {"��ʧѹ","����ʧѹ"};
char* pt_SUSYMode[] = {"��բ","�澯"};

TKGTABLE tKG2PubTable[] = 
{
   {"����ģʽ��������",0x02, {KG_0,KG_1,KG_2},3,4,szWorkMode},
   {"��ѹ������ģʽ��",0x02, {KG_3,KG_4,KG_5},3,4,faWorkMode},
#if (TYPE_USER == USER_BEIJING)
   {"���׿������͡���",0x02, {KG_6,0xff,0xff},1,2,ptKgMode},
#endif
#if (TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_WDG)
	{"�Զ�������������",0x02, {KG_7,0xff,0xff},1,2,ptAutoMode},
#endif
	{"�����ͺϹ����ж�",0x02, {KG_8,0xff,0xff},1,2,ptDLXBSMode},	
	{"�����ͺϹ��ϳ���",0x02, {KG_9,0xff,0xff},1,2,ptFaTripMode},
	{"L ���ܺ�բ���ڡ�",0x02, {KG_10,0xff,0xff},1,2,pt_SUSYMode},		

};

BYTE KG2_PUB_NUMBER=sizeof(tKG2PubTable)/sizeof(tKG2PubTable[0]);

#ifdef INCLUDE_61850
TYBXMLTABLE gYbXmlTable[]=
{
    {false, false, 0, "���ϼ��", "", "",  {255,}, {255,}},
    {true,  true,  1, "����һ��", "PTOC", "OvCur", {SET_I1, SET_T1, 255,}, {SET_IANG1, SET_IANG2, SET_IKG,255,}},
    {true,  true,  2, "��������", "PTOC", "OvCur", {SET_I2, SET_T2, 255,}, {255,}},
    {true,  true,  3, "��������", "PTOC", "OvCur", {SET_I3, SET_T3, 255,}, {255,}},
    {true,  true,  4, "������", "PTOC", "OvCur", {SET_IGFH, SET_TGFH, 255,}, {255,}},
    {true,  true,  1, "��ѹ", "PTOV", "OvVol", {SET_UGY, SET_TGY, 255,}, {255,}},
    {true,  true,  1, "��ѹ����", "PTUV", "UnVol", {SET_UDY, SET_TDY, 255,}, {SET_USLIP, 255,}},
    {true,  true,  1, "����һ��", "PTOC", "ZerOc", {SET_3I01, SET_TI01, 255,}, {SET_3U0, SET_I0ANG1, SET_I0ANG2, 255,}},
    {true,  true,  2, "��������", "PTOC", "ZerOC", {SET_3I02, SET_TI02, 255,}, {255,}},
    {true,  true,  3, "��������", "PTOC", "ZerOc", {SET_3I03, SET_TI03, 255,}, {255,}},
    {true,  true,  5, "����ӿ��", "PTOC", "OvCur", {255,},{SET_IM, SET_TQDQJ, 255,}},
    {true , false, 0, "PT����", "", "",  {255,}, {255,}},
    {true,  true,  1, "�غ�բ", "RREC", "Rec",   {255,},{SET_CHNUM, SET_TCH1, SET_TCH2, SET_TCH3, SET_TQDQJ, 255,}},
    {true,  false, 0, "��ѹ", "", "",  {255,}, {255,}},
    {true,  false, 0, "��������", "", "", {255,}, {255,}},
#ifdef INCLUDE_PR_U
	{true,  true,  1, "��ѹ��բ", "RREC", "Vol",   {255,},  {SET_TX, SET_TY, 255,}},
	{true,  false, 0, "L�ͺ�բ", "", "", {255,}, {255,}},
	{true,  true,  1, "��ѹ","PTOV", "OvVol0", {255,}, {SET_TU0, 255,}},
	{true,  true,  2, "ѹ��ϻ�", "RREC", "Vol",    {255,}, {SET_UDIFF, 255,}},
#endif
	{true,  true,  1, "��Ƶ����", "PTUF", "UnHz",   {SET_FREQ, SET_TDF, SET_UFREQ, SET_IFREQ, 255}, {SET_IFREQ,255}},
	{true,  true,  2, "�ֺ�ͬ��", "RREC", "Rec",    {255,}, {SET_TQTDQ,SET_TQUDIFF, SET_TQFDIFF, SET_TQFSDIFF,255}},
};

TSETXMLTABLE gSetXmlTable[]=
{
    {0,   "Yb",          "ѹ��",             XML_DA_TYPE_INT},
    {0,   "KG1",         "������һ",         XML_DA_TYPE_INT},
    {0,   "KG2",         "�����ֶ�",         XML_DA_TYPE_INT},
    {1,   "StrVal",      "����I�ε���",      XML_DA_TYPE_VAL},
    {1,   "OpDlTmms",    "����I��ʱ��",      XML_DA_TYPE_INT},
    {2,   "StrVal",      "����II�ε���",     XML_DA_TYPE_VAL},
    {2,   "OpDlTmms",    "����II��ʱ��",     XML_DA_TYPE_INT},
    {3,   "StrVal",      "����III�ε���",    XML_DA_TYPE_VAL},
    {3,   "OpDlTmms",    "����III��ʱ��",    XML_DA_TYPE_INT},
    {1,   "StrAngLow",   "�����Ƕ�����",     XML_DA_TYPE_VAL},
    {1,   "StrAngHigh",  "�����Ƕ�����",     XML_DA_TYPE_VAL},
    {10,  "StrVal",      "�����ŵ���",       XML_DA_TYPE_VAL},
    {10,  "OpDlTmms",    "������ʱ��",       XML_DA_TYPE_INT},
    {4,   "StrVal",      "�����ɵ���",       XML_DA_TYPE_VAL},
    {4,   "OpDlTmms",    "������ʱ��",       XML_DA_TYPE_INT},
    {1,   "StrValZd",    "�����ڶϵ���",     XML_DA_TYPE_VAL},
    {5,   "StrVal",      "����ѹ",           XML_DA_TYPE_VAL},
    {5,   "OpDlTmms",    "����ѹʱ��",       XML_DA_TYPE_INT},
    {6,   "StrVal",      "�͵�ѹ",           XML_DA_TYPE_VAL},
    {5,   "OpDlTmms",    "�͵�ѹʱ��",       XML_DA_TYPE_INT},
    {7,   "StrValU",     "��ѹ",             XML_DA_TYPE_VAL},
    {7,   "StrVal",      "����I�ε���",      XML_DA_TYPE_VAL},
    {7,   "OpDlTmms",    "����I��ʱ��",      XML_DA_TYPE_INT},
    {8,   "StrVal",      "����II�ε���",     XML_DA_TYPE_VAL},
    {8,   "OpDlTmms",    "����II��ʱ��",     XML_DA_TYPE_INT},
    {9,   "StrVal",      "����III�ε���",    XML_DA_TYPE_VAL},
    {9,   "OpDlTmms",    "����III��ʱ��",    XML_DA_TYPE_INT},
    {7,   "StrAngLow",   "�����Ƕ�����",     XML_DA_TYPE_VAL},
    {7,   "StrAngHigh",  "�����Ƕ�����",     XML_DA_TYPE_VAL},
    {12,  "RecNum",      "�غ�բ����",       XML_DA_TYPE_INT},
    {12,  "Rec1Tmms",    "һ���غ�ʱ��",     XML_DA_TYPE_INT},
    {12,  "Rec2Tmms",    "�����غ�ʱ��",     XML_DA_TYPE_INT},
    {12,  "Rec3Tmms",    "�����غ�ʱ��",     XML_DA_TYPE_INT},
    {255, "RecBsTmms",   "�غϱ���ʱ��",     XML_DA_TYPE_INT},
    {255, "UVOpDlTmms",  "ʧѹ��բʱ��",     XML_DA_TYPE_INT},
#ifdef INCLUDE_PR_U
    {15,  "OpDlTmmsX",   "��ѹ��բXʱ��",    XML_DA_TYPE_INT},
    {15,  "OpDlTmmsY",   "��ѹ��բYʱ��",    XML_DA_TYPE_INT},
    {255, "UVBsDlTmms",  "��բ��������",     XML_DA_TYPE_INT},
    {17,  "U0OpDlTmms",  "��ѹ��բʱ��",     XML_DA_TYPE_INT},
    {18,  "StrDiffU",    "����ѹ��",         XML_DA_TYPE_VAL},
#endif
    {6,   "RteUBlkVal",  "��ѹ����",         XML_DA_TYPE_VAL},
    {19,  "BlkValRteHz", "Ƶ�ʻ���",         XML_DA_TYPE_VAL},
    {19,  "StrVal",      "��Ƶ��ֵ",         XML_DA_TYPE_VAL},
    {19,  "OpDlTmms",    "��Ƶʱ��",         XML_DA_TYPE_INT},
    {19,  "BlkVal",      "��Ƶ������ѹ",     XML_DA_TYPE_VAL},
    {19,  "BlkValA",     "��Ƶ��������",     XML_DA_TYPE_VAL},
    {12,  "SynBlkDifAng","ͬ�ڽǲ�",         XML_DA_TYPE_VAL},
    {20,  "RecDifTmms",  "��բ��ǰʱ��",     XML_DA_TYPE_INT},
    {20,  "RecDifU",     "׼ͬ�ڵ�ѹ��",     XML_DA_TYPE_VAL},
    {20,  "RecDifF",     "׼ͬ��Ƶ�ʲ�",     XML_DA_TYPE_VAL},
    {20,  "RecDifFS",    "׼ͬ�ڼ��ٶ�Ƶ��", XML_DA_TYPE_VAL},
};

#endif

const char sRelayName[] = "Relay";

TTABLETYPE tSetType;
VPrRunSet *prRunSet[2];
VPrSetPublic *prSetPublic[2];
VPrNbSet prNbSet;
BYTE *caSetBuf;
BYTE *prSet[MAX_FD_NUM*2];   //������չ�����
BYTE prSetBuf[PR_SET_SIZE*(MAX_FD_NUM*2)];   //������չ�����
extern VFdProtCal *fdProtVal;
extern VPrRunInfo *prRunInfo;
extern int g_prInit;
extern VPrRunPublic *prRunPublic;
//��ȡ��ֵ����
BYTE GetSetOpt( TSETVAL *v)
{
  return ((v->bySetAttrib)&SETATTRIB_OPTMASK);
}

BYTE GetSetType( TSETVAL *v)
{
  return ((v->bySetAttrib)&SETATTRIB_TYPEMASK);
}

//ѹ��BCD����
BOOL IsBcd(BYTE b)
{
  if(((b&0xf)<0x0a)&&((b&0xf0)<0xa0))return TRUE;
  return FALSE;
}

//����Ƿ�BCD��ʽ
BOOL IsBcdSet( TSETVAL *v)
{
  if(GetSetType(v) == SETTYPE_DEC) return FALSE;
  if(GetSetType(v) == SETTYPE_IP)  return FALSE;
  if(!IsBcd(v->byValLol))	return FALSE;
  if(!IsBcd(v->byValLoh))   return FALSE;
  if(!IsBcd(v->byValHil))	return FALSE;
  if(!IsBcd(v->byValHih))	return FALSE;
  return TRUE;
}
/*
DWORD FixHex(DWORD f)
{
  WORD t1,t2;
  t1=(WORD)(f>>16),t2=(WORD)f;
  return (((DWORD)BcdHex(t1)<<16)+FraHex(t2));
}*/
//������ֵԭʼֵת���ɸ�����
//bset[0]=��ֵ����
//bset[1]=��ֵ��ֵ�ĵ��ֽ�
//bset[2]=��ֵ��ֵ�ĸ��ֽ�
//hset=���,���и�16λΪ��������,��16λΪС������
BOOL SetToFloat( TSETVAL *bval,float *fval)
{
  BYTE attrib;
  if (!IsBcdSet(bval))	return FALSE;
  attrib=bval->byValAttrib;
  if(attrib & VALUEATTRIB_DOT2)
  {
    if(attrib & VALUEATTRIB_DOT1)
	{
	  //3λС��
	  *fval = (char)(bval->byValLol&0xf)*0.001+(char)(bval->byValLol>>4)*0.01;
	  *fval += (char)(bval->byValLoh&0xf)*0.1+(char)(bval->byValLoh>>4)*1.0;
      *fval += (char)(bval->byValHil&0xf)*10+(char)(bval->byValHil>>4)*100;
	  *fval += (char)(bval->byValHih&0xf)*1000+(char)(bval->byValHih>>4)*10000;
    }
    else
	{ 
	  //2λС��
      *fval = (char)(bval->byValLol&0xf)*0.01+(char)(bval->byValLol>>4)*0.1;
	  *fval += (char)(bval->byValLoh&0xf)+(char)(bval->byValLoh>>4)*10;
      *fval += (char)(bval->byValHil&0xf)*100+(char)(bval->byValHil>>4)*1000;
	  *fval += (char)(bval->byValHih&0xf)*10000+(char)(bval->byValHih>>4)*100000;
    }
  }
  else
  {
    if(attrib & VALUEATTRIB_DOT1)
	{
	  //1λС��
      *fval = (char)(bval->byValLol&0xf)*0.1+(char)(bval->byValLol>>4)*1.0;
	  *fval += (char)(bval->byValLoh&0xf)*10+(char)(bval->byValLoh>>4)*100;
      *fval += (char)(bval->byValHil&0xf)*1000+(char)(bval->byValHil>>4)*10000;
	  *fval += (char)(bval->byValHih&0xf)*100000+(char)(bval->byValHih>>4)*1000000;
    }
    else
	{ 
	  //��С��
      *fval = (char)(bval->byValLol&0xf)*1.0+(char)(bval->byValLol>>4)*10;
	  *fval += (char)(bval->byValLoh&0xf)*100+(char)(bval->byValLoh>>4)*1000;
      *fval += (char)(bval->byValHil&0xf)*10000+(char)(bval->byValHil>>4)*100000;
	  *fval += (char)(bval->byValHih&0xf)*1000000+(char)(bval->byValHih>>4)*10000000;
    }
  }
  if(attrib & VALUEATTRIB_SIG)	
  {
	  *fval =-*fval;
  }
  return TRUE;
}

//Ĭ����λС��
BOOL FloatToSet(float fval, TSETVAL *bval)
{
   BYTE val,index;
   BYTE *pbval = &(bval->byValLol);
   float fvalue = fval*1000;
   DWORD dvalue;
   int i;

   val = bval->bySetAttrib;
   memset((BYTE*)bval, 0, sizeof(TSETVAL));
   bval->bySetAttrib = val;

   if(fvalue < 0)
   {
      bval->byValAttrib |= VALUEATTRIB_SIG;
	  fvalue = -fvalue;
   }
   
   dvalue = fvalue;
   index = 0;
   bval->byValAttrib |= VALUEATTRIB_DOT2;
   for(i=0; i<9; i++)
   {
       val = dvalue%10;
	   if((i == 0) && (val == 0))
	   {
	      dvalue = dvalue/10;
		  continue;
	   }
	   else if(i == 0)
	   	  bval->byValAttrib |= VALUEATTRIB_DOT1;
	   *pbval |= val<<(4*(index%2));
	   index++;
	   if((index%2) == 0)
	   	  pbval++;
	   dvalue = dvalue/10;
	   if((dvalue == 0)||(index >= 8))
	   	  break;
   }
   
   return true;
}


//����ֵת����16����,����鶨ֵ�ķ�Χ
BOOL FloatSetRange(int i, int flag, float *pFloat)
{
    float fMin,fMax;

    //����ֵ
    if(!SetToFloat(( TSETVAL *)(caSetBuf+i*sizeof(TSETVAL)),pFloat))
    {
	    return false;
    }
	if (flag == PR_SET_FD)
    {
       SetToFloat(( TSETVAL *)(&tSetTable[i].tMin),&fMin);
       SetToFloat(( TSETVAL *)(&tSetTable[i].tMax),&fMax);
	}
	else if (flag == PR_SET_PUB)
	{
       SetToFloat(( TSETVAL *)(&tSetPubTable[i].tMin),&fMin);
       SetToFloat(( TSETVAL *)(&tSetPubTable[i].tMax),&fMax);
	}
    if((*pFloat < fMin) || (*pFloat > fMax))
    {
       return false;
    }
    return true;
}

enum SETRET
{
   SET_OK       = 0,
   SET_TYPEERR  = 0x01,
   SET_VALUEERR = 0x02,
   SET_OTHERERR = 0x04
};
//ת��ѹ��
enum SETRET CvtYb(int i,DWORD *pYb, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  BYTE b1,b2;
  WORD w1,w2;
  enum SETRET ret = SET_OK;
  //��鶨ֵ���
  if (i<0 || i>=SET_NUMBER)
  	ret = SET_OTHERERR;
  
  //��鶨ֵ����
  pSet += i;
  if (GetSetType(pSet) != SETTYPE_YB)
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	 pDef = &(tSetTable[i].tDef);
  else
  	 pDef = &(tSetPubTable[i].tDef);

  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
     memcpy(pSet, pDef, sizeof(TSETVAL));
  
  b1=pSet->byValLol;
  b2=pSet->byValLoh;

  w1=((WORD)b2<<8)|b1;

  b1=pSet->byValHil;
  b2=pSet->byValHih;
  w2=((WORD)b2<<8)|b1;

  *pYb = w2<<16|w1;
 
  return ret;
}

enum SETRET CvtIp(int i,DWORD *pIp, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  BYTE b1,b2;
  WORD w1,w2;
  enum SETRET ret = SET_OK;
  //��鶨ֵ���
  if (i<0 || i>=SET_NUMBER)
  	 ret = SET_OTHERERR;
  
  //��鶨ֵ����
  pSet += i;
  if (GetSetType(pSet) != SETTYPE_IP)
     ret = SET_TYPEERR;

  if ((ret != SET_OK) ||(GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, &(tSetTable[i].tDef), sizeof(TSETVAL));

  b1=pSet->byValLol;
  b2=pSet->byValLoh;

  w1=((WORD)b2<<8)|b1;
  

  b1=pSet->byValHil;
  b2=pSet->byValHih;

  w2=((WORD)b2<<8)|b1;

  *pIp = w2<<16|w1;
 
  return ret;
}

//ת��������
enum SETRET CvtK(int i,DWORD *pK, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  BYTE b1,b2;
  WORD w1,w2;
  enum SETRET ret = SET_OK;

  //��鶨ֵ���
  if (i<0 || i>=SET_NUMBER)
  	 ret = SET_OTHERERR;
  
  //��鶨ֵ����
  pSet += i;
  if((GetSetType(pSet)!= SETTYPE_KG)&&(GetSetType(pSet)!= SETTYPE_DEC))
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	pDef = &(tSetTable[i].tDef);
  else
  	pDef = &(tSetPubTable[i].tDef);
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	 memcpy(pSet, pDef, sizeof(TSETVAL));

  b1=pSet->byValLol;
  b2=pSet->byValLoh;
  w1=((WORD)b2<<8)|b1;

  b1=pSet->byValHil;
  b2=pSet->byValHih;
  w2=((WORD)b2<<8)|b1;

  *pK=((DWORD)w2<<16)|w1;
 
  return ret;
}

//ת��ϵ��
enum SETRET CvtF(int i, DWORD *pF, DWORD dKF, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  enum SETRET ret = SET_OK;
  
  //��鶨ֵ���
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //��鶨ֵ����
  pSet += i;
  if ((GetSetType(pSet) != SETTYPE_FACTOR) && (GetSetType(pSet)!= SETTYPE_HZ))
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	pDef = &(tSetTable[i].tDef);
  else
  	pDef = &(tSetPubTable[i].tDef);
  
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));

  //ת����ֵ
  if (!FloatSetRange(i,flag, &f))
  {
     SetToFloat(pDef, &f);
	 memcpy(pSet, pDef, sizeof(TSETVAL));
	 *pF= f*dKF;
	 ret = SET_VALUEERR;
     return ret;
  }
  *pF= f*dKF;
  return ret;
}

enum SETRET CvtHz(int i, DWORD *pHz, DWORD dKHz, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  enum SETRET ret = SET_OK;
  
  //��鶨ֵ���
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //��鶨ֵ����
  pSet += i;
  if ((GetSetType(pSet) != SETTYPE_HZ)&&(GetSetType(pSet)!= SETTYPE_HZPERS))
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	pDef = &(tSetTable[i].tDef);
  else
  	pDef = &(tSetPubTable[i].tDef);
  
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));

  //ת����ֵ
  if (!FloatSetRange(i, flag, &f))
  {
     SetToFloat(pDef, &f);
	 memcpy(pSet, pDef, sizeof(TSETVAL));
	 *pHz= f*dKHz;
	 ret = SET_VALUEERR;
     return ret;
  }
  *pHz= f*dKHz;
  return ret;
}

//ת��������ֵ
//*pKI: ���棬iKI:ct/in*ijz=ct(ijz=in)
enum SETRET CvtI(int i,DWORD *pI,long *pKI,int iKI, int num, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  int j;
  enum SETRET ret = SET_OK;
 
  //��鶨ֵ���
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //��鶨ֵ����
  pSet += i;
  if (GetSetType(pSet) != SETTYPE_A)
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	pDef = &(tSetTable[i].tDef);
  else
  	pDef = &(tSetPubTable[i].tDef);
  
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));
 
  //ת����ֵ
  if (!FloatSetRange(i, flag, &f))
  {
    SetToFloat(pDef, &f);
	memcpy(pSet, pDef, sizeof(TSETVAL));
	for (j=0; j<num; j++,pI++,pKI++)
	  *pI= f*(*pKI)/iKI;
	ret = SET_VALUEERR;
    return ret;
  }
  for (j=0; j<num; j++,pI++,pKI++)
	 *pI= f*(*pKI)/iKI;
  return ret;
}

//ת����ѹ��ֵ
//*pKU: ���棬iKU:pt/un*ujz(ujz=100)
enum SETRET CvtU(int i,DWORD *pU,long *pKU,int iKU, int num, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  int k;
  enum SETRET ret = SET_OK;
 
  //��鶨ֵ���
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;

  //��鶨ֵ����
  pSet += i;
  if ((GetSetType(pSet) != SETTYPE_U) && (GetSetType(pSet)!= SETTYPE_UPERS))
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	 pDef = &(tSetTable[i].tDef);
  else
  	 pDef = &(tSetPubTable[i].tDef);
  if((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));
 
  //ת����ֵ
  if (!FloatSetRange(i, flag, &f))
  {
     SetToFloat(pDef, &f);
	 memcpy(pSet, pDef, sizeof(TSETVAL));
	 for (k=0; k<num; k++,pU++,pKU++)
	    *pU= f*(*pKU)/iKU;
     ret = SET_VALUEERR;
	 return ret;
  }
  for (k=0; k<num; k++,pU++,pKU++)
	  *pU= f*(*pKU)/iKU;
  return ret;
}

//ת��ʱ�䶨ֵ
enum SETRET CvtT(int i,DWORD *pT,DWORD dKT, BOOL zero, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  TSETVAL *pDef;
  float f;
  enum SETRET ret = SET_OK;

  //��鶨ֵ���
  if (i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //��鶨ֵ����
  pSet += i;
  if (GetSetType(pSet) != SETTYPE_S)
     ret = SET_TYPEERR;

  if (flag == PR_SET_FD)
  	 pDef = &(tSetTable[i].tDef);
  else
  	 pDef = &(tSetPubTable[i].tDef);
  if ((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4) ))
  	memcpy(pSet, pDef, sizeof(TSETVAL));

  //ת����ֵ
  if (!FloatSetRange(i, flag, &f))
  {
      SetToFloat(pDef, &f);
	  memcpy(pSet, pDef, sizeof(TSETVAL));
	  *pT= f*dKT;
	  if ((*pT < RELAY_DELAY_5MS) && !zero)
	  	*pT = RELAY_DELAY_5MS;
	  ret = SET_VALUEERR;
	  return ret;
  }
  *pT= f*dKT;
  if ((*pT < RELAY_DELAY_5MS) && !zero)
  	 *pT = RELAY_DELAY_5MS;
  return ret;
}

enum SETRET CvtAng(int i, long *pAng, DWORD dKAng, int flag)
{
  TSETVAL *pSet=(TSETVAL*)caSetBuf;
  float f;
  enum SETRET ret = SET_OK;
 
  //��鶨ֵ���
  if(i < 0 || i > SET_NUMBER)  
  	ret = SET_OTHERERR;
  
  //��鶨ֵ����
  pSet += i;
  if(GetSetType(pSet) != SETTYPE_DEGREE)
     ret = SET_TYPEERR;

  if((ret != SET_OK) || (GetSetOpt(pSet) !=  (SETOPT_WRITE << 4)))
  	memcpy(pSet, &(tSetTable[i].tDef), sizeof(TSETVAL));
 
  //ת����ֵ
  if(!FloatSetRange(i,flag, &f))
  {
      SetToFloat(( TSETVAL *)(&tSetTable[i].tDef),&f);
	  memcpy(pSet, &(tSetTable[i].tDef), sizeof(TSETVAL));
	  *pAng= f*dKAng;
      ret = SET_VALUEERR;
	  return ret;
  }
  *pAng= f*dKAng;
  return ret;
}

	
enum SETRET ReverseKSet(int i, BYTE *pSet, DWORD value)
{
    TSETVAL *ptr = ((TSETVAL*)pSet)+i;
	
	//��鶨ֵ���
    if (i<0 || i>=SET_NUMBER) return SET_OTHERERR;
  
  
    ptr->byValHih = HIBYTE(HIWORD(value));
	ptr->byValHil = LOBYTE(HIWORD(value));
	ptr->byValLoh = HIBYTE(LOWORD(value));
	ptr->byValLol = LOBYTE(LOWORD(value));

	return SET_OK;
}

VPrRunSet * prGetRunSet(int fd)
{
    VPrRunSet *pprRunSet;
   	VPrRunInfo *pInfo = prRunInfo+fd;

	pprRunSet = prRunSet[pInfo->set_no]+fd;

	return pprRunSet;
}

void prNbSetDef()
{
	prNbSet.fUxxMin  = 30.0;
	prNbSet.fUxx75   = 75.0;
	prNbSet.fIMin  = 0.05;
	prNbSet.fTLqd  = 0.2;
	prNbSet.fTPt   = 9;
	prNbSet.fTYy   = 30;
	prNbSet.fTBhfg = 3;
	prNbSet.fTBsfg = 2;
	prNbSet.fTIRet = 0;           
	prNbSet.fTI0Ret= 3;
	prNbSet.fTURet = 3;
	prNbSet.fTRet  = 120;
	prNbSet.fTJsIn = 120;
	prNbSet.fFreqMin = 45.0;

}

void prGetUpValRet(DWORD *pVal, DWORD *pValRet, DWORD num) 
{
    int i;
	for (i=0; i<num; i++)
       *(pValRet+i) = *(pVal+i)*94/100;
}

void prGetLowValRet(DWORD *pVal, DWORD *pValRet, DWORD num)
{
    int i;
	for (i=0; i<num; i++)
		*(pValRet+i) = *(pVal+i)*105/100;
}

void prCheckConflict(void)
{
    int iNo;
    VPrSetPublic*pprRunSet;
	//VPrRunSet *pRunSet;
	//VPrRunInfo *pInfo;

	if (prRunPublic->set_no == 0)
		iNo = 1;
	else
		iNo = 0;
	
    pprRunSet = prSetPublic[iNo];

	//pInfo  = prRunInfo + pprRunSet->bySLLine;
	//pRunSet = prRunSet[pInfo->set_no] + pprRunSet->bySLLine; //

	if ((pprRunSet->dYb&(PR_YB_SUDY|PR_YB_SUSY)) && (pprRunSet->dYb&PR_YB_UDIFF))
		pprRunSet->dYb &= ~PR_YB_UDIFF;
}

void prPubSetMode(VPrSetPublic* pprRunSet)
{
#if(DEV_SP != DEV_SP_DTU)  // ֻ��FTU���д˹���
	DWORD dKG1_OLD = 0;
	//ģʽ��ʱ��ɾ������߶�ֵ�йص�
    if (pprRunSet->byWorkMode == PR_MODE_SEG)
    {
    //    pprRunSet->dYb &= ~PR_YB_SUSY;
		//pRunSet->dYb &= PR_YB_I1|PR_YB_I2|PR_YB_I3|PR_YB_GZJC_EN;
    }
	else if (pprRunSet->byWorkMode == PR_MODE_TIE)
	{
	    pprRunSet->dYb &= ~PR_YB_SUDY;
		//pRunSet->dYb &= PR_YB_I1|PR_YB_I2|PR_YB_I3|PR_YB_GZJC_EN;
	}
	else if (pprRunSet->byWorkMode == PR_MODE_PR)
	{
	    pprRunSet->dYb = 0;
	}
	else if (pprRunSet->byWorkMode == PR_MODE_FJ)
	{
	    pprRunSet->dYb = 0;
		//pRunSet->dYb &= PR_YB_I1|PR_YB_I2|PR_YB_I3|PR_YB_GZJC_EN;
	}
	//���ݹ���ģʽ�ٻ�дѹ�嶨ֵ,Ŀǰֻ���˹�����ֵ

	dKG1_OLD = pprRunSet->dKG1;
	if(pprRunSet->bFaWorkMode == PR_FA_U)  // ��ѹʱ����
	{
		pprRunSet->dYb |= (PR_YB_SUDY | PR_YB_SLWY);
		pprRunSet->dYb &=  ~PR_YB_I_DL;
		pprRunSet->dKG1 |=  (PR_CFG1_X_BS | PR_CFG1_Y_BS |PR_CFG1_SY_BS);	
		pprRunSet->dKG1 &= ~(PR_CFG1_UI_TYPE |PR_CFG1_FTU_SD |PR_CFG1_ZSY_DXJD |PR_CFG1_ZSY_XJDL);
	}
	else if (pprRunSet->bFaWorkMode == PR_FA_I)  // ��ѹ����ʱ����
	{
		pprRunSet->dYb |= (PR_YB_SUDY |PR_YB_I_DL);
		pprRunSet->dYb &=  ~PR_YB_SLWY;
		pprRunSet->dKG1 |=  (PR_CFG1_X_BS |PR_CFG1_Y_BS |PR_CFG1_SY_BS | PR_CFG1_UI_TYPE |PR_CFG1_FZ_BS);	
		pprRunSet->dKG1 &= ~( PR_CFG1_ZSY_DXJD |PR_CFG1_ZSY_XJDL);
	}
	else if(pprRunSet->bFaWorkMode == PR_FA_ZSY) // ����Ӧ��
	{
		pprRunSet->dYb |= (PR_YB_SUDY | PR_YB_SLWY);
		pprRunSet->dYb &=  ~PR_YB_I_DL;
		pprRunSet->dKG1 |=  (PR_CFG1_Y_BS |PR_CFG1_SY_BS |PR_CFG1_ZSY_DXJD |PR_CFG1_ZSY_XJDL);	
		pprRunSet->dKG1 &=~ (PR_CFG1_UI_TYPE);
	}
		
	ReverseKSet(SET_PUB_YB,caSetBuf,pprRunSet->dYb);
	if(pprRunSet->dKG1 != dKG1_OLD)
	{
		ReverseKSet(SET_PUB_KG1,caSetBuf,pprRunSet->dKG1);
		if (pprRunSet->dKG1&PR_CFG1_X_BS)        pprRunSet->bXBS      = true; else pprRunSet->bXBS      = false;
		if (pprRunSet->dKG1&PR_CFG1_Y_BS)        pprRunSet->bYBS      = true; else pprRunSet->bYBS      = false;
		if (pprRunSet->dKG1&PR_CFG1_DU_BS)       pprRunSet->bDUBS     = true; else pprRunSet->bDUBS     = false;
		if (pprRunSet->dKG1&PR_CFG1_SY_BS)       pprRunSet->bSYBS     = true; else pprRunSet->bSYBS     = false;
		if (pprRunSet->dKG1&PR_CFG1_UI_TYPE)     pprRunSet->bIProtect = true; else pprRunSet->bIProtect = false; 
		if (pprRunSet->dKG1&PR_CFG1_U0_TRIP)     pprRunSet->bU0Trip   = true; else pprRunSet->bU0Trip   = false;
		
#ifdef INCLUDE_PR_PRO	
		if (pprRunSet->dKG1&PR_CFG1_DFJL)        pprRunSet->bDfJl     = true; else pprRunSet->bDfJl     = false;
		if (pprRunSet->dKG1&PR_CFG1_DF_IBS)      pprRunSet->bDfIBs    = true; else pprRunSet->bDfIBs    = false; 
		pprRunSet->bTqMode = (pprRunSet->dKG1&PR_CFG1_CHTQ_MODE)>>KG_8;
		pprRunSet->bShTqMode = (pprRunSet->dKG1&PR_CFG1_SHTQ_MODE)>>KG_10;
		if (pprRunSet->dKG1&PR_CFG1_WY_ANY)      pprRunSet->bTqWyAny  = true; else pprRunSet->bTqWyAny  = false; 
		if (pprRunSet->dKG1&PR_CFG1_TQ_LINE)     pprRunSet->bTqLine   = true; else pprRunSet->bTqLine   = false;
#endif
	    if (pprRunSet->dKG1&PR_CFG1_JS_MODE)     pprRunSet->bJsMode   = true; else pprRunSet->bJsMode   = false;
		if (pprRunSet->dKG1&PR_CFG1_FTU_SD)     pprRunSet->bFTUSD = true; else pprRunSet->bFTUSD = false; 
		if (pprRunSet->dKG1&PR_CFG1_ZSY_XJDL)    pprRunSet->bXJDL =  true; else pprRunSet->bXJDL =  false;
		if (pprRunSet->dKG1&PR_CFG1_ZSY_DXJD)    pprRunSet->bDXJD =  true; else pprRunSet->bDXJD =  false;
		if (pprRunSet->dKG1&PR_CFG1_POWER_INDEX) pprRunSet->byPowerIndex = 1; else pprRunSet->byPowerIndex = 0; 
         if (pprRunSet->dKG1&PR_CFG1_FZ_BS) 	pprRunSet->bFZBS = true;else pprRunSet->bFZBS = false;      
	}
#endif
}

enum SETRET prPubSetConv(int fd)
{
    int  i, iNo;
    enum SETRET ret,ret1;
	DWORD dK, dKU;
#ifdef INCLUDE_PR_PRO
	DWORD dKI;
#endif
	VFdCfg *pfdCfg;
	VPrRunSet *pRunSet;
	VPrRunInfo *pInfo;
	VPrSetPublic* pprRunSet;

	if (prRunPublic[fd].set_no == 0)
		iNo = 1;
	else
		iNo = 0;
	
    pprRunSet = prSetPublic[iNo]+fd;
    memset(pprRunSet, 0, sizeof(VPrSetPublic));
	
	ret1 = SET_OK;
	
	if ((ret = CvtYb(SET_PUB_YB,&(pprRunSet->dYb), PR_SET_PUB)) != SET_OK)
		ret1 = ret;
	
    if ((ret = CvtK(SET_PUB_KG1,&(pprRunSet->dKG1), PR_SET_PUB)) != SET_OK)
		ret1 = ret;
	if (pprRunSet->dKG1&PR_CFG1_X_BS)        pprRunSet->bXBS      = true;
	if (pprRunSet->dKG1&PR_CFG1_Y_BS)        pprRunSet->bYBS      = true;
	if (pprRunSet->dKG1&PR_CFG1_DU_BS)       pprRunSet->bDUBS     = true;
	if (pprRunSet->dKG1&PR_CFG1_SY_BS)       pprRunSet->bSYBS     = true;
	if (pprRunSet->dKG1&PR_CFG1_UI_TYPE)     pprRunSet->bIProtect = true;
	if (pprRunSet->dKG1&PR_CFG1_U0_TRIP)     pprRunSet->bU0Trip   = true;
	
#ifdef INCLUDE_PR_PRO	
	if (pprRunSet->dKG1&PR_CFG1_DFJL)        pprRunSet->bDfJl     = true;
	if (pprRunSet->dKG1&PR_CFG1_DF_IBS)      pprRunSet->bDfIBs    = true;
	pprRunSet->bTqMode = (pprRunSet->dKG1&PR_CFG1_CHTQ_MODE)>>KG_8;
	pprRunSet->bShTqMode = (pprRunSet->dKG1&PR_CFG1_SHTQ_MODE)>>KG_10;
	if (pprRunSet->dKG1&PR_CFG1_WY_ANY)      pprRunSet->bTqWyAny  = true;
	if (pprRunSet->dKG1&PR_CFG1_TQ_LINE)     pprRunSet->bTqLine   = true;
#endif
    if (pprRunSet->dKG1&PR_CFG1_JS_MODE)     pprRunSet->bJsMode   = true;
	if (pprRunSet->dKG1&PR_CFG1_FTU_SD)     pprRunSet->bFTUSD = true;
	if (pprRunSet->dKG1&PR_CFG1_ZSY_XJDL)    pprRunSet->bXJDL =  true;
	if (pprRunSet->dKG1&PR_CFG1_ZSY_DXJD)    pprRunSet->bDXJD =  true;
	
	
	if (pprRunSet->dKG1&PR_CFG1_POWER_INDEX) pprRunSet->byPowerIndex = 1;
	if (pprRunSet->dKG1&PR_CFG1_FZ_BS) 	pprRunSet->bFZBS = true;

	if (pprRunSet->dKG1&PR_CFG1_BUS_OPT)     pprRunSet->bBusTrip = true;
		
	 if ((ret = CvtK(SET_PUB_KG2,&(pprRunSet->dKG2), PR_SET_PUB)) != SET_OK)
		ret1 = ret;
	
	pprRunSet->byWorkMode = (pprRunSet->dKG2&PR_CFG2_CONTROL_MODE);
	
	pprRunSet->bFaWorkMode = (pprRunSet->dKG2&PR_CFG2_FA_MODE) >> KG_3;

	//pprRunSet->bFaSelectMode = (pprRunSet->dKG2&PR_CFG2_FA_SELECT_MODE) >> KG_7;
	
#if ((TYPE_USER == USER_GUIYANG)&&(DEV_SP == DEV_SP_WDG))
	pprRunSet->bAuto = (pprRunSet->dKG2&PR_CFG2_AUTO_MODE) >> KG_7;
#endif	
	pprRunSet->bDlxbsMode = (pprRunSet->dKG2&PR_CFG2_DLXBS_MODE) >> KG_8;	
	pprRunSet->bFaTripMode = (pprRunSet->dKG2&PR_CFG2_FA_I_TRIP) >> KG_9;	
	pprRunSet->bSUSYMode =  (pprRunSet->dKG2&PR_CFG2_SUSY_TRIP) >> KG_10;	

#if (TYPE_USER == USER_BEIJING)
	if(pprRunSet->dKG2 & PR_CFG2_KG_MODE)	 pprRunSet->bKgMode = true;
#endif

    if ((ret = CvtK(SET_PUB_SLFD,&dK, PR_SET_PUB)) != SET_OK)
		ret1 = ret;
    //pprRunSet->bySLLine = dK - 1;
	pprRunSet->bySLLine = fd;
	if (pprRunSet->bySLLine >= fdNum) 
	{
	    pprRunSet->bySLLine = 0;
		ret1 = SET_VALUEERR;
	}
	ReverseKSet(SET_PUB_SLFD, caSetBuf, pprRunSet->bySLLine+1);//ǿ��slline��Ӧÿ�����
	
#ifdef INCLUDE_PR_PRO
    if ((ret = CvtK(SET_PUB_TQFD,&dK, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
    pprRunSet->byTqLine = dK - 1;

	if (pprRunSet->byTqLine >= fdNum) 
	{
	    pprRunSet->byTqLine = 0;
		ret1 = SET_VALUEERR;
	}
#endif

   if ((ret = CvtK(SET_PUB_ICNT,&(pprRunSet->dICnt), PR_SET_PUB)) != SET_OK)
		ret1 = ret;

   if ((ret = CvtK(SET_PUB_DYLJ,&(pprRunSet->dDYLJ), PR_SET_PUB)) != SET_OK)
		ret1 = ret;
   if ((ret = CvtT(SET_PUB_TDYLJ,&(pprRunSet->dTDYLJ),10000, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

   if ((ret = CvtK(SET_PUB_LXFZ,&(pprRunSet->dLXFZ), PR_SET_PUB)) != SET_OK)  //������բ����  add by wdj
		ret1 = ret;
   if ((ret = CvtT(SET_PUB_TLXFZ,&(pprRunSet->dTLXFZ),10000, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

   if ((ret = CvtK(SET_PUB_RECORD_NUM,&(pprRunSet->dRecNum), PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	pprRunSet->dRecNum = pprRunSet->dRecNum/256*100 + (pprRunSet->dRecNum%256)/16*10 + pprRunSet->dRecNum%16;  // 16 ����ת10����,ֻת����λ

	//��ѹͨ�� ,����Խ��
	if ((ret = CvtK(SET_PUB_AI_S,&(pprRunSet->dAi_S_No), PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	pprRunSet->dAi_S_No = pprRunSet->dAi_S_No/256*100 + (pprRunSet->dAi_S_No%256)/16*10 + pprRunSet->dAi_S_No%16;  // 16 ����ת10����,ֻת����λ
	if(pprRunSet->dAi_S_No > MAX_AI_NUM)
		pprRunSet->dAi_S_No = 0;
	
	if ((ret = CvtK(SET_PUB_AI_F,&(pprRunSet->dAi_F_No), PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	pprRunSet->dAi_F_No = pprRunSet->dAi_F_No/256*100 + (pprRunSet->dAi_F_No%256)/16*10 + pprRunSet->dAi_F_No%16;  // 16 ����ת10����,ֻת����λ
	if(pprRunSet->dAi_F_No > MAX_AI_NUM)
		pprRunSet->dAi_F_No = 0;
	
	dK = 10000;
	if ((ret = CvtT(SET_PUB_TX,&(pprRunSet->dTX),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TY,&(pprRunSet->dTY),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TFZ,&(pprRunSet->dTFZ2),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TSYCY,&(pprRunSet->dTSYCY),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TU0,&(pprRunSet->dTU0),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TWY,&(pprRunSet->dTWY),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TJD,&(pprRunSet->dTJD),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TJDRevs,&(pprRunSet->dTJDrevs),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TICNT1,&(pprRunSet->dTIDl1),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TICNT2,&(pprRunSet->dTIDl2),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	
	if ((ret = CvtT(SET_PUB_TC,&(pprRunSet->dTC),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TS,&(pprRunSet->dTS),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_PUB_TXXCH,&(pprRunSet->dTXXCH),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_PUB_TSYCY,&(pprRunSet->dTSY1),dK, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	pprRunSet->dTSY2 = 2000; // ˫ѹ��˲ѹ������ʱ��
	pprRunSet->dTFZ1 = 3000;

	pfdCfg = pFdCfg + pprRunSet->bySLLine;
	pInfo  = prRunInfo + pprRunSet->bySLLine;
	pRunSet = prRunSet[pInfo->set_no] + pprRunSet->bySLLine;

    for (i=0; i<3; i++)
    {
	    pprRunSet->dSLU[i] = pRunSet->dUYy[i];
		pprRunSet->dSLUMin[i] = pRunSet->dUWy[i];
    }
	
	if (tSetType.dExt == PROT_SET_1)
        dKU = pfdCfg->pt*100/pfdCfg->Un;
	else
		dKU = 100;
	if((ret = CvtU(SET_PUB_UDIFF,&(pprRunSet->dUDiff),&(pfdCfg->gain_u[0]), dKU, 1, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	if (tSetType.dExt == PROT_SET_1)
		dK = pfdCfg->ct;
	else
		dK = 5;
	if ((ret = CvtI(SET_PUB_IBUS, &pprRunSet->dIBus,&(pfdCfg->gain_i[0]), dK, 1, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	
	if(g_Sys.MyCfg.dwCfg & 0x10)
	{
		if((ret = CvtU(SET_PUB_U0,&(pprRunSet->dU0),&(pfdCfg->gain_u[3]), dKU/10, 1, PR_SET_PUB))!=SET_OK)
			ret1 = ret;
	}
	else
	{
		if((ret = CvtU(SET_PUB_U0,&(pprRunSet->dU0),&(pfdCfg->gain_u[3]), dKU, 1, PR_SET_PUB))!=SET_OK)
			ret1 = ret;
    }
	if((ret = CvtU(SET_PUB_UCY,pprRunSet->dUCY,&(pfdCfg->gain_u[0]), dKU, 3, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	dK = 65536;
	if ((ret = CvtAng(SET_PUB_UANG,&(pprRunSet->dUAng),dK, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	prPubSetMode(pprRunSet);
	
	if((pprRunSet->dYb&(PR_YB_SUDY|PR_YB_SUSY)) && (pprRunSet->dYb&PR_YB_UDIFF))
		pprRunSet->dYb &= ~PR_YB_UDIFF;

#ifdef INCLUDE_PR_PRO
	pfdCfg = pFdCfg + pprRunSet->byTqLine;
	pInfo  = prRunInfo + pprRunSet->byTqLine;
	pRunSet = prRunSet[pInfo->set_no] + pprRunSet->byTqLine;

	if (tSetType.dExt == PROT_SET_1)
		dKI = pfdCfg->ct;
	else
		dKI = 5;
	if (tSetType.dExt == PROT_SET_1)
        dKU = pfdCfg->pt*100/pfdCfg->Un;
	else
		dKU = 100;

	if ((ret = CvtHz(SET_PUB_FREQ,  (DWORD*)&(pprRunSet->dFreq), 100, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_PUB_IFREQ, pprRunSet->dIFreq, pfdCfg->gain_i, dKI, 3, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtU(SET_PUB_UFREQ, pprRunSet->dUFreq, pfdCfg->gain_u, dKU, 3, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_PUB_TDF, &(pprRunSet->dTDf), 1000, false, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtHz(SET_PUB_FSLIP, (DWORD*)&(pprRunSet->dFSlip), 100, PR_SET_PUB))!= SET_OK)
		ret1 = ret;

	pprRunSet->dFreqMin = prNbSet.fFreqMin*100;

	pfdCfg = pFdCfg + pprRunSet->byTqLine;
	pInfo  = prRunInfo + pprRunSet->byTqLine;
	pRunSet = prRunSet[pInfo->set_no] + pprRunSet->byTqLine;

	if (tSetType.dExt == PROT_SET_1)
        dKU = pfdCfg->pt*100/pfdCfg->Un;
	else
		dKU = 100;

	if((ret = CvtHz(SET_PUB_TQFDIFF, (DWORD*)&(pprRunSet->dTqFDiff), 100, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if((ret = CvtHz(SET_PUB_TQFSDIFF, (DWORD*)&(pprRunSet->dTqFsDiff), 100, PR_SET_PUB))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtU(SET_PUB_TQUDIFF, (DWORD*)&(pprRunSet->dTqUDiff), pfdCfg->gain_u, dKU, 1, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if((ret = CvtT(SET_PUB_TQTDQ, &(pprRunSet->dTqTAng), 1000, true, PR_SET_PUB))!=SET_OK)
		ret1 = ret;
	if((ret = CvtAng(SET_PUB_TQDQJ, &(pprRunSet->dTqDqAngle), 65536, PR_SET_PUB))!=SET_OK)
		ret1 = ret;

	for (i=0; i<3; i++)
    {
	    pprRunSet->dTqU75[i] = pRunSet->dUxx75[i];
		pprRunSet->dTqUMin[i] = pRunSet->dUMin[i];
    }

	pprRunSet->dTTqTwj= 30000;
	pprRunSet->dTTqYk = 60000;

#endif

//	extNvRamSet(NVRAM_DEV_PUBYB,(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
	if(0 == fd)
		extNvRamSet(NVRAM_DEV_PUBYB,(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
	else
		extNvRamSet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
	
	pprRunSet->bSetChg = TRUE;

	return ret1;
}

enum SETRET prFdSetConv(int fd)
{
	int  iNo,i,ai_chn;
	DWORD dK;	
	VPrRunSet *pprRunSet;
	VFdCfg *pfdCfg = pFdCfg+fd;
	VPrRunInfo *pInfo = prRunInfo+fd;
	enum SETRET ret,ret1;
	BOOL bZero;

	if (pInfo->set_no == 0)
		iNo = 1;
	else
		iNo = 0;
	
	pprRunSet = prRunSet[iNo]+fd;
    memset(pprRunSet, 0, sizeof(VPrRunSet));

    ret1 = SET_OK;
    if ((ret = CvtYb(SET_YB, &(pprRunSet->dYb), PR_SET_FD)) != SET_OK)
		ret1 = ret;

	for (i=MMI_MEA_IA; i<MMI_MEA_NUM; i++)
	{
		ai_chn = pfdCfg->mmi_meaNo_ai[i];
        //����߻Ṳ��ѹ,���ĳһ������Ч����ͨ����Ч
		if ((ai_chn>=0) && (pprRunSet->dYb&PR_YB_GZJC_EN))
			pAiCfg[ai_chn].protect_enable = 1;		
	}
 
	if ((ret = CvtK(SET_KG1, &(pprRunSet->dKG1), PR_SET_FD)) != SET_OK)
		ret1 = ret;


	if (!(pprRunSet->dKG1 & PR_CFG1_GFH_TRIP)) pprRunSet->bGfhGJ = true;
	if (pprRunSet->dKG1 & PR_CFG1_FSX_START)   pprRunSet->bGfhSX = true;
	pprRunSet->bFsx = pprRunSet->dKG1&PR_CFG1_FSX_CURV;

	if (pprRunSet->dKG1 & PR_CFG1_U_PT)        pprRunSet->bPTBS     = true;
	if (pprRunSet->dKG1 & PR_CFG1_I_DY)        pprRunSet->bIDYBS    = true;
	if (pprRunSet->dKG1 & PR_CFG1_I_DIR)       pprRunSet->bIDir     = true;
	if (pprRunSet->dKG1 & PR_CFG1_I_FX)        pprRunSet->bIDirMX  = true;
	if (pprRunSet->dKG1 & PR_CFG1_DY_CYY)      pprRunSet->bCYY      = true;
	if (pprRunSet->dKG1 & PR_CFG1_PT_A)        pprRunSet->bPTU[0]   = true;
	if (pprRunSet->dKG1 & PR_CFG1_PT_B)        pprRunSet->bPTU[1]   = true;
	if (pprRunSet->dKG1 & PR_CFG1_PT_C)        pprRunSet->bPTU[2]   = true;
	if (pprRunSet->dKG1 & PR_CFG1_I0_DIR)      pprRunSet->bI0Dir    = true;
	if (pprRunSet->dKG1 & PR_CFG1_I0_FX)       pprRunSet->bI0DirMX  = true;
	if (pprRunSet->dKG1 & PR_CFG1_I0_U )       pprRunSet->bI0U      = true;
	if (pprRunSet->dKG1 & PR_CFG1_CH_DDLBS)    pprRunSet->bDdlBsch  = true;
	if (pprRunSet->dKG1 & PR_CFG1_CD_MODE)     pprRunSet->bCd       = true;
	pprRunSet->bITrip = (pprRunSet->dKG1&PR_CFG1_TRIP_MODE)>>KG_17;
	pprRunSet->bIReport = (pprRunSet->dKG1&PR_CFG1_FAULT_REPORT)>>KG_20;
	if (pprRunSet->dKG1 & PR_CFG1_TEST_VOL)    pprRunSet->bTestVol  = true;
	if (pprRunSet->dKG1 & PR_CFG1_BHQD_QUIT)   pprRunSet->bQdQuit   = true;
	if (pprRunSet->dKG1 & PR_CFG1_FZD_BH)      pprRunSet->bfZdbh    = true;
	if (pprRunSet->dKG1 & PR_CFG1_HW2_ZD)      pprRunSet->bHw2Zd    = true;
	//if (pprRunSet->dKG1 & PR_CFG1_PR_TRIP)     pprRunSet->bPrTrip   = true;
	if(pprRunSet->dKG1 & PR_CFG1_LED_FG)     pprRunSet->bLedZdFg   = true;

	pprRunSet->bI0GJ = pprRunSet->bI1GJ = pprRunSet->bI2GJ = pprRunSet->bI3GJ = true;
	if (pprRunSet->dKG1 & PR_CFG1_I0_TRIP)    pprRunSet->bI0Trip   = true;
	if (pprRunSet->dKG1 & PR_CFG1_I1_TRIP)    pprRunSet->bI1Trip  = true;
	if (pprRunSet->dKG1 & PR_CFG1_I2_TRIP)    pprRunSet->bI2Trip  = true;


	if (pprRunSet->dKG1 & PR_CFG1_I2_DY)        pprRunSet->bI2DYBS    = true;
	if (pprRunSet->dKG1 & PR_CFG1_I2_DIR)       pprRunSet->bI2Dir     = true;


	if ((ret = CvtK(SET_KG2, &(pprRunSet->dKG2), PR_SET_FD)) != SET_OK)
		ret1 = ret;

	pprRunSet->bIYxDir = !((pprRunSet->dKG2&PR_CFG2_IYX_DIR)>>KG_2);
    pprRunSet->coef = ((pprRunSet->dKG2&PR_CFG2_COEF)>>KG_3) ? -1 : 1;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_GL)      pprRunSet->bRcdGl= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_SY)      pprRunSet->bRcdSy= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_U0)      pprRunSet->bRcdU0= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_I0)      pprRunSet->bRcdI0= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_I0TB)      pprRunSet->bRcdI0TB= true;
	if (pprRunSet->dKG2 & PR_CFG2_RCD_GLBH)      pprRunSet->bRcdGlbh= true;
	if (pprRunSet->dKG2 & PR_CFG2_SWITCH_TRIP_F)      pprRunSet->bSwitchTrip= true; //add by lijun  2018-05-25  ������բ�쳣������
    
	if (pprRunSet->dKG2 & PR_CFG2_XDL_YB)      pprRunSet->bXDLYB = true;  
	pprRunSet->bXDLGJ= true;
	if (pprRunSet->dKG2 & PR_CFG2_XDL_TRIP)   pprRunSet->bXDLTrip= true;
	if (pprRunSet->dKG2 & PR_CFG2_XDL_JR)     pprRunSet->bJrU   = true;    else pprRunSet->bJrU0 = true;
  if (pprRunSet->dKG2 & PR_CFG2_XDL_DLFX)     pprRunSet->bXdlFx   = true;    else pprRunSet->bXdlFx = false;
	if (!(pprRunSet->dKG2 & PR_CFG2_GY_TRIP))     pprRunSet->bGyGJ   = true;else pprRunSet->bGyGJ = false; // ��ѹ�澯
	if (!(pprRunSet->dKG2 & PR_CFG2_DY_TRIP))     pprRunSet->bDyGJ   = true;else pprRunSet->bDyGJ = false; // ��ѹ�澯

	if (!(pprRunSet->dKG2 & PR_CFG2_GF_TRIP))     pprRunSet->bGpGJ   = true; else pprRunSet->bGpGJ   = false;//��Ƶ�澯
	if (!(pprRunSet->dKG2 & PR_CFG2_DF_TRIP))     pprRunSet->bDpGJ   = true; else pprRunSet->bDpGJ   = false;//��Ƶ�澯

	if (pprRunSet->dKG2 & PR_CFG2_I3_TRIP)    pprRunSet->bI3Trip  = true;
	if (!(pprRunSet->dKG2 & PR_CFG2_MXQY_TRIP))	  pprRunSet->bMXQYGJ = true;else pprRunSet->bMXQYGJ = false; // ĸ��Ƿѹ�澯


	if (tSetType.dExt == PROT_SET_1)
		dK = pfdCfg->ct;
	else
		dK = 5;
	if ((ret = CvtI(SET_I1,pprRunSet->dI[0],pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dI[0], pprRunSet->dIRet[0], 3);
	if ((ret = CvtI(SET_I2,pprRunSet->dI[1],pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dI[1], pprRunSet->dIRet[1], 3);
	if ((ret = CvtI(SET_I3,pprRunSet->dI[2],pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dI[2], pprRunSet->dIRet[2], 3);
	if ((ret = CvtI(SET_IM, pprRunSet->dIM, pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_IGFH, pprRunSet->dIGfh, pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dIGfh, pprRunSet->dIGfhRet, 3);
	if ((ret = CvtI(SET_IKG,  pprRunSet->dIKG, pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_ITB,  &pprRunSet->dIInK, pfdCfg->gain_i, dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_CHBSDL,&(pprRunSet->dCHBSDL),pfdCfg->gain_i, dK,1, PR_SET_FD))!=SET_OK)     // �غϱ�������
		ret1 = ret;

    if (tSetType.dExt == PROT_SET_1)
		dK = pfdCfg->ct0;
	else
		dK = pfdCfg->In0;
	if ((ret = CvtI(SET_IGFH,&(pprRunSet->dIGfh[3]), &(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_3I01,&(pprRunSet->dI0[0]),&(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_3I02,&(pprRunSet->dI0[1]),&(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_3I03,&(pprRunSet->dI0[2]),&(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
    prGetUpValRet(pprRunSet->dI0, pprRunSet->dI0Ret, 3);
		
	if ((ret = CvtI(SET_I0TB,&pprRunSet->dII0nK,&(pfdCfg->gain_i0), dK, 1, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	
	if (tSetType.dExt == PROT_SET_1)
        dK = pfdCfg->pt*100/pfdCfg->Un;
	else
		dK = 100;
	if ((ret = CvtU(SET_UGY, pprRunSet->dUGy, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dUGy, pprRunSet->dUGyRet, 3);
	if ((ret = CvtU(SET_UDY, pprRunSet->dUDy, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetLowValRet(pprRunSet->dUDy, pprRunSet->dUDyRet, 3);
	if ((ret = CvtU(SET_USLIP, pprRunSet->dUSlip, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtU(SET_UYY, pprRunSet->dUYy, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtU(SET_UWY, pprRunSet->dUWy, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;

	if ((ret = CvtU(SET_UUMX, pprRunSet->dUUMX, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetUpValRet(pprRunSet->dUUMX, pprRunSet->dUUMXRet, 3);
	if ((ret = CvtU(SET_UDMX, pprRunSet->dUDMX, pfdCfg->gain_u, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	prGetLowValRet(pprRunSet->dUDMX, pprRunSet->dUDMXRet, 3);
	
    if(g_Sys.MyCfg.dwCfg & 0x10)
	{
		if ((ret = CvtU(SET_3U0,&(pprRunSet->dU0),&(pfdCfg->gain_u0), dK/10, 1, PR_SET_FD))!= SET_OK)
			ret1 = ret;
		if ((ret = CvtU(SET_U0TB,&(pprRunSet->dIUnK),&(pfdCfg->gain_u0), dK/10, 1, PR_SET_FD))!= SET_OK)
			ret1 = ret;
    }
	else
	{
		if ((ret = CvtU(SET_3U0,&(pprRunSet->dU0),&(pfdCfg->gain_u0), dK, 1, PR_SET_FD))!= SET_OK)
			ret1 = ret;
		if ((ret = CvtU(SET_U0TB,&(pprRunSet->dIUnK),&(pfdCfg->gain_u0), dK, 1, PR_SET_FD))!= SET_OK)
			ret1 = ret;
	}
	
	prGetUpValRet(&pprRunSet->dU0, &pprRunSet->dU0Ret, 1);
		
	if ((ret = CvtF(SET_CHNUM, (DWORD*)&(pprRunSet->bCHNum), 1, PR_SET_FD))!=SET_OK)
		ret1 = ret;
    if ((ret = CvtF(SET_YXTZ, (DWORD*)&(pprRunSet->wYxNum), 1, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	dK = 10000;
	bZero = false;
	if ((ret = CvtT(SET_T1,&(pprRunSet->dT[0]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_T2,&(pprRunSet->dT[1]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_T3,&(pprRunSet->dT[2]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TDJQD,&(pprRunSet->dTDjqd),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TGFH,&(pprRunSet->dTGfh), dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TGY,&(pprRunSet->dTGy),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TDY,&(pprRunSet->dTDy),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TI01,&(pprRunSet->dT0[0]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TI02,&(pprRunSet->dT0[1]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TI03,&(pprRunSet->dT0[2]),dK, bZero, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TCH1,&(pprRunSet->dTCH1),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TCH2,&(pprRunSet->dTCH2),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TCH3,&(pprRunSet->dTCH3),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TCHBS,&(pprRunSet->dTCHBS),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TKEEP,&(pprRunSet->dTRet),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_LED_TKEEP,&(pprRunSet->dLedTret),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_TJS,&(pprRunSet->dTJS),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_TJSFG,&(pprRunSet->dTJSFG),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_TDXJD,&(pprRunSet->dTDXJD),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtT(SET_XDLCOEF,&(pprRunSet->wXDLCoef),dK, false, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtHz(SET_FGP,&(pprRunSet->dGFreq),100,PR_SET_FD)) != SET_OK)  // ��Ƶ��ֵ
		ret1 = ret;
	if ((ret = CvtT(SET_TGP,&(pprRunSet->dTGFreq),dK, false, PR_SET_FD))!=SET_OK) //��Ƶʱ��
		ret1 = ret;
	
	if ((ret = CvtHz(SET_FDP,&(pprRunSet->dDFreq),100,PR_SET_FD)) != SET_OK)  // ��Ƶ��ֵ
		ret1 = ret;
	if ((ret = CvtT(SET_TDP,&(pprRunSet->dTDFreq),dK, false, PR_SET_FD))!=SET_OK) //��Ƶʱ��
		ret1 = ret;
	
	if ((ret = CvtT(SET_I0TJS,&(pprRunSet->dI0TJS),dK, true, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	if ((ret = CvtT(SET_TUMX,&(pprRunSet->dTUMX),dK, true, PR_SET_FD))!=SET_OK)//ĸ�߹���ѹʱ��
			ret1 = ret;
	if ((ret = CvtT(SET_TDMX,&(pprRunSet->dTDMX),dK, true, PR_SET_FD))!=SET_OK)//ĸ�ߵ͵�ѹʱ��
			ret1 = ret;


	pprRunSet->dLedTret = pprRunSet->dLedTret;

	dK = 65536;
	if ((ret = CvtAng(SET_IANG1,&(pprRunSet->lPang1),dK, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtAng(SET_IANG2,&(pprRunSet->lPang2),dK, PR_SET_FD))!=SET_OK)
		ret1 = ret;

	 if (pprRunSet->bIDirMX)
    {
       pprRunSet->lPang1 += 180<<16;
	   pprRunSet->lPang2 += 180<<16;
	   if(pprRunSet->lPang1 > (360<<16))
	   	pprRunSet->lPang1 -= 360<<16;
	   if(pprRunSet->lPang2 > (360<<16))
	   	pprRunSet->lPang2 -= 360<<16;
    }

	if (pprRunSet->lPang1 > pprRunSet->lPang2)
		pprRunSet->lPang1 -= 360<<16;

    if ((ret = CvtAng(SET_I0ANG1,&(pprRunSet->lNang1),dK, PR_SET_FD))!=SET_OK)
		ret1 = ret;
	if ((ret = CvtAng(SET_I0ANG2,&(pprRunSet->lNang2),dK, PR_SET_FD))!=SET_OK)
		ret1 = ret;

    if (pprRunSet->bI0DirMX)
    {
       pprRunSet->lNang1 += 180<<16;
	   pprRunSet->lNang2 += 180<<16;
	   if(pprRunSet->lNang1 > (360<<16))
	   	pprRunSet->lNang1 -= 360<<16;
	   if(pprRunSet->lNang2 > (360<<16))
	   	pprRunSet->lNang2 -= 360<<16;
    }
	if (pprRunSet->lNang1 > pprRunSet->lNang2)
	    pprRunSet->lNang1 -= 360<<16;

	pprRunSet->dU0_8V = 8.0*pfdCfg->gain_u0/100;
	for (i=0; i<3; i++)
	{
	  pprRunSet->dU_16V[i] = 16.0*pfdCfg->gain_u[i]/100;
	  pprRunSet->dI_1A[i]  = 1.0*pfdCfg->gain_i[i]/5;
	  pprRunSet->dUxxMin[i]= 30*pfdCfg->gain_u[i]/100;
	  pprRunSet->dUxx75[i] = 75*pfdCfg->gain_u[i]/100;
	  pprRunSet->dIMin[i]  = prNbSet.fIMin*pfdCfg->gain_i[i]/5;
	  pprRunSet->dI1_old[i]= pprRunSet->dI[0][i];

	  if (pfdCfg->cfg & FD_CFG_PT_58V)
	  {
	     pprRunSet->dUMin[i] = pprRunSet->dUxxMin[i]*1000/1732;
		 pprRunSet->dU75[i]  = pprRunSet->dUxx75[i]*1000/1732;
	  }
	  else
	  {
	     pprRunSet->dUMin[i] = pprRunSet->dUxxMin[i];
		 pprRunSet->dU75[i] = pprRunSet->dUxx75[i];
	  }
	}
	pprRunSet->dUMin[3] = prNbSet.fUxxMin/2*pfdCfg->gain_u0/100;
	pprRunSet->dIMin[3] = prNbSet.fIMin/2*pfdCfg->gain_i0/pfdCfg->In0;
	pprRunSet->dTLqd  = prNbSet.fTLqd*10000;
	pprRunSet->dTPt   = prNbSet.fTPt*10000;
	pprRunSet->dTBhfg = prNbSet.fTBhfg*10000;
	pprRunSet->dTBsfg = prNbSet.fTBsfg*10000;
	pprRunSet->dTYy   = prNbSet.fTYy*10000;
	pprRunSet->dTIRet = prNbSet.fTIRet*10000;
	pprRunSet->dTI0Ret= prNbSet.fTI0Ret*10000;
	pprRunSet->dTURet = prNbSet.fTURet*10000;
	pprRunSet->dTJSIN = prNbSet.fTJsIn*10000;
	pprRunSet->dTCHBAY= 90000;
	pprRunSet->dTWY   = 1000;
	
	if (tSetType.dExt == PROT_SET_1)
		dK = pfdCfg->ct;
	else
		dK = 5;

	if ((ret = CvtI(SET_IHJS,pprRunSet->dI[3],pfdCfg->gain_i, dK, 3, PR_SET_FD))!= SET_OK)
		ret1 = ret;
	if ((ret = CvtI(SET_I0HJS,&(pprRunSet->dI0[3]),&pfdCfg->gain_i0, dK, 1, PR_SET_FD))!= SET_OK)//modify by wdj 3->1
		ret1 = ret;	

    if ((pprRunSet->dYb&PR_YB_I3) && (pprRunSet->dI[3][0] > pprRunSet->dI[2][0]))
	   	memcpy(pprRunSet->dI[3], pprRunSet->dI[2], 12);
	else if ((pprRunSet->dYb&PR_YB_I2) && (pprRunSet->dI[3][0] > pprRunSet->dI[1][0]))
	    memcpy(pprRunSet->dI[3], pprRunSet->dI[1], 12);
	else if (pprRunSet->dI[3][0] > pprRunSet->dI[0][0])
	    memcpy(pprRunSet->dI[3], pprRunSet->dI[0], 12);
	
	if ((pprRunSet->dYb&PR_YB_I03) && (pprRunSet->dI0[3] > pprRunSet->dI0[2]))
		pprRunSet->dI0[3] = pprRunSet->dI0[2];
	else if ((pprRunSet->dYb&PR_YB_I02)  && (pprRunSet->dI0[3] > pprRunSet->dI0[1]))
		pprRunSet->dI0[3] = pprRunSet->dI0[1];
	else if (pprRunSet->dI0[3] > pprRunSet->dI0[0])
		pprRunSet->dI0[3] = pprRunSet->dI0[0];
	
	pprRunSet->bSetChg = TRUE;
	return ret1;
}

int prFdSetDef()
{	
    int i;
    TSETVAL *pb=(TSETVAL *)caSetBuf;
    memset(caSetBuf, 0, SET_NUMBER*sizeof(TSETVAL));
	
	for(i=0; i<SET_NUMBER; i++)
	{
		*pb = tSetTable[i].tDef;
		pb++;
	}
	return OK;
}

int prPubSetDef()
{	
    int i;
    TSETVAL *pb=(TSETVAL *)caSetBuf;
    memset(caSetBuf, 0, SET_PUB_NUMBER*sizeof(TSETVAL));
	
	for(i=0; i<SET_PUB_NUMBER; i++)
	{
		*pb = tSetPubTable[i].tDef;
		pb++;
	}
	return OK;
}

int prFdSetPartDef(BYTE begin)
{	
    int i;
    TSETVAL *pb=(TSETVAL *)(caSetBuf+ begin*sizeof(TSETVAL));
	if(begin >= SET_NUMBER)
		return ERROR;
	
	for(i = begin; i<SET_NUMBER; i++)
	{
		*pb = tSetTable[i].tDef;
		pb++;
	}
	return OK;
}

int prPubSetPartDef(BYTE begin)
{	
    int i;
    TSETVAL *pb=(TSETVAL *)(caSetBuf+ begin*sizeof(TSETVAL));
	if(begin >= SET_PUB_NUMBER)
		return ERROR;
	
	for(i=begin; i<SET_PUB_NUMBER; i++)
	{
		*pb = tSetPubTable[i].tDef;
		pb++;
	}
	return OK;
}


void prFillTableType()
{
    strcpy(tSetType.szName, sRelayName);
    tSetType.dType = DEV_SP;
	tSetType.dExt  = PROT_TYPE;
}

BOOL prSetTypeJudgy(TTABLETYPE *pType)
{
   if(strcmp(pType->szName, tSetType.szName))
   	  return false;
 //  if(pType->dType != tSetType.dType)
 //  	  return false;
   if(pType->dExt != tSetType.dExt)
   	  return false;
   return true;
}

int prSetTableRead()
{
	char fname[MAXFILENAME];
    struct VFileHead *head;
    TTABLETYPE *pTag;


    sprintf(fname, "ProSetTable.cfg");
	if (ReadParaFile(fname, prTableBuf, sizeof(prTableBuf)) == ERROR)
		return ERROR;

	head = (struct VFileHead *)prTableBuf;	
    pTag = (TTABLETYPE*)(head+1);

	if(prSetTypeJudgy(pTag))
	   return OK;
	else
		return ERROR;

}

int prSetTableWrite()
{
	char fname[MAXFILENAME];
	BYTE *pTemp;
	BYTE kg_num;
	BYTE i,j;
    struct VFileHead *head;

    pTemp = prTableBuf;
	
	head  = (struct VFileHead *)prTableBuf;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	pTemp += sizeof(struct VFileHead);
	
    memcpy(pTemp, (BYTE*)&tSetType, sizeof(TTABLETYPE));
	pTemp += sizeof(TTABLETYPE);
	memcpy(pTemp, (BYTE*)&YB_NUMBER, 1);
	pTemp += 1;
	memcpy(pTemp, (BYTE*)tYbTable, sizeof(tYbTable));
	pTemp += sizeof(tYbTable);

	kg_num = KG1_NUMBER;
	memcpy(pTemp, (BYTE*)&kg_num, 1);
	pTemp += 1;

	for(i=0; i<KG1_NUMBER; i++)
	{
	   memcpy(pTemp, (BYTE*)(tKG1Table+i), sizeof(TKGTABLE)-4);
	   pTemp += sizeof(TKGTABLE)-4;
	   for(j=0; j<tKG1Table[i].byMsNum; j++)
	   {
	      memcpy(pTemp, (BYTE*)(tKG1Table[i].szBit[j]), (4*2+1));
	      pTemp += (4*2+1);
	   }
	}

	kg_num = KG2_NUMBER;
	memcpy(pTemp, (BYTE*)&kg_num, 1);
	pTemp += 1;
	for(i=0; i<KG2_NUMBER; i++)
	{
	   memcpy(pTemp, (BYTE*)(tKG2Table+i), sizeof(TKGTABLE)-4);
	   pTemp += sizeof(TKGTABLE)-4;
	   for(j=0; j<tKG2Table[i].byMsNum; j++)
	   {
	      memcpy(pTemp, (BYTE*)(tKG2Table[i].szBit[j]), (4*2+1));
	      pTemp += (4*2+1);
	   }
	}

	memcpy(pTemp, (BYTE*)&SET_NUMBER, 1);
	pTemp += 1;
	memcpy(pTemp, (BYTE*)tSetTable, sizeof(tSetTable));
	pTemp += sizeof(tSetTable);
	
	head->nLength = pTemp - prTableBuf;
	sprintf(fname, "ProSetTable.cfg");
	if(WriteParaFile(fname, (struct VFileHead *)prTableBuf) == ERROR)
		return ERROR;
	
	return OK;
}

int prPubSetTableWrite()
{
	char fname[MAXFILENAME];
	BYTE *pTemp;
	BYTE kg_num;
	BYTE i,j;
		struct VFileHead *head;

	pTemp = prTableBuf;
	
	head  = (struct VFileHead *)prTableBuf;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	pTemp += sizeof(struct VFileHead);
	
    memcpy(pTemp, (BYTE*)&tSetType, sizeof(TTABLETYPE));
	pTemp += sizeof(TTABLETYPE);
	memcpy(pTemp, (BYTE*)&YB_PUB_NUMBER, 1);
	pTemp += 1;
	memcpy(pTemp, (BYTE*)tYbPubTable, sizeof(tYbPubTable));
	pTemp += sizeof(tYbPubTable);

	kg_num = KG1_PUB_NUMBER;
	memcpy(pTemp, (BYTE*)&kg_num, 1);
	pTemp += 1;

	for(i=0; i<KG1_PUB_NUMBER; i++)
	{
	   memcpy(pTemp, (BYTE*)(tKG1PubTable+i), sizeof(TKGTABLE)-4);
	   pTemp += sizeof(TKGTABLE)-4;
	   for(j=0; j<tKG1PubTable[i].byMsNum; j++)
	   {
	      memcpy(pTemp, (BYTE*)(tKG1PubTable[i].szBit[j]), (4*2+1));
	      pTemp += (4*2+1);
	   }
	}

	kg_num = KG2_PUB_NUMBER;
	memcpy(pTemp, (BYTE*)&kg_num, 1);
	pTemp += 1;
	for(i=0; i<KG2_PUB_NUMBER; i++)
	{
	   memcpy(pTemp, (BYTE*)(tKG2PubTable+i), sizeof(TKGTABLE)-4);
	   pTemp += sizeof(TKGTABLE)-4;
	   for(j=0; j<tKG2PubTable[i].byMsNum; j++)
	   {
	      memcpy(pTemp, (BYTE*)(tKG2PubTable[i].szBit[j]), (4*2+1));
	      pTemp += (4*2+1);
	   }
	}

	memcpy(pTemp, (BYTE*)&SET_PUB_NUMBER, 1);
	pTemp += 1;
	memcpy(pTemp, (BYTE*)tSetPubTable, sizeof(tSetPubTable));
	pTemp += sizeof(tSetPubTable);

		
	head->nLength = pTemp - prTableBuf;
	sprintf(fname, "ProSetPubTable.cfg");
	if(WriteParaFile(fname, (struct VFileHead *)prTableBuf) == ERROR)
		return ERROR;

	return OK;  
}

#ifdef INCLUDE_61850
int prSetXmlWrite()
{
    char fname[MAXFILENAME];
	BYTE *pTemp;
	BYTE i,j;
	int len;
    struct VFileHead *head;
	TYBXMLTABLE * pSet = gYbXmlTable;

    pTemp = prTableBuf;
	
	head  = (struct VFileHead *)prTableBuf;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	pTemp += sizeof(struct VFileHead);
	
    memcpy(pTemp, (BYTE*)&tSetType, sizeof(TTABLETYPE));
	pTemp += sizeof(TTABLETYPE);

	memcpy(pTemp, (BYTE*)&YB_NUMBER, 1);
	pTemp += 1;
	len = sizeof(gYbXmlTable);
	memcpy(pTemp, (BYTE*)gYbXmlTable, sizeof(gYbXmlTable));
	pTemp += sizeof(gYbXmlTable);

	memcpy(pTemp, (BYTE*)&SET_NUMBER, 1);
	pTemp += 1;
	len = sizeof(gSetXmlTable);
	memcpy(pTemp, (BYTE*)gSetXmlTable, sizeof(gSetXmlTable));
	pTemp += sizeof(gSetXmlTable);

	head->nLength = pTemp - prTableBuf;
	sprintf(fname, "ProSetXml.cfg");
	if(WriteParaFile(fname, (struct VFileHead *)prTableBuf) == ERROR)
		return ERROR;

	return OK;  

}
#endif

int prTableFileJudgy()
{
    prFillTableType();
    if(prSetTableRead() == ERROR)
    {
		prSetTableWrite();
		prPubSetTableWrite();
#ifdef INCLUDE_61850
        prSetXmlWrite();
#endif
		return ERROR;
    }
	return OK;
}

int prPubSetRead(void)
{
   	struct VFileHead *head;
    TTABLETYPE *pType;
	enum SETRET ret = SET_OK;
   
	head = (struct VFileHead *)prSetTmp;	
    pType = (TTABLETYPE*)(head+1);
	caSetBuf  = (BYTE *)(pType+1);

	if (ReadParaFile("fdPubSet.cfg", prSetTmp, sizeof(prSetTmp)) == ERROR)
	{
	    prPubSetDef(); 
	}
	else if (!prSetTypeJudgy(pType))
	{
	    prPubSetDef();
		ret = SET_OTHERERR;
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "����·���������ļ���ƥ��");
	}
	else
	{
		if(FileSetNum <  SET_PUB_NUMBER)
		{
			//����Ĭ��ֵ
			prPubSetPartDef(FileSetNum);
			//�����ļ�
			if (WriteParaFile("fdPubSet.cfg", (struct VFileHead *)prSetTmp) == ERROR)
					ret = SET_OTHERERR;
			WriteDoEvent(NULL, 0, "������ֵģ������������ȱ�ٵĲ��ֶ�ֵ!");
		}
	}

    ret |= prPubSetConv(0);
	if(ret != SET_OK)
	{
	   WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "����·������������");
	   prPubSetTableWrite();
	}
	

	memcpy(prSet[prRunPublic[0].prSetnum], caSetBuf, SET_PUB_NUMBER*sizeof(TSETVAL));

	
	if (prRunPublic[0].set_no == 0)
		prRunPublic[0].set_no = 1;
	else
		prRunPublic[0].set_no = 0;
	return ret;
}

int prPubOtherSetRead(int fd)
{
   	char fname[MAXFILENAME];
   	struct VFileHead *head;
    TTABLETYPE *pType;
	enum SETRET ret = SET_OK;
   
	head = (struct VFileHead *)prSetTmp;	
    pType = (TTABLETYPE*)(head+1);
	caSetBuf  = (BYTE *)(pType+1);

	sprintf(fname, "fd%02dPubSet.cfg", fd+1);
	if (ReadParaFile(fname, prSetTmp, sizeof(prSetTmp)) == ERROR)
	{
	    prPubSetDef(); 
	}
	else if (!prSetTypeJudgy(pType))
	{
	    prPubSetDef();
		ret = SET_OTHERERR;
		WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "������ֵ%d�����ļ���ƥ��",fd+1);
	}
	else
	{
		if(FileSetNum <  SET_PUB_NUMBER)
		{
			//����Ĭ��ֵ
			prPubSetPartDef(FileSetNum);
			//�����ļ�
			if (WriteParaFile(fname, (struct VFileHead *)prSetTmp) == ERROR)
					ret = SET_OTHERERR;
			WriteDoEvent(NULL, 0, "������ֵ%dģ������������ȱ�ٵĲ��ֶ�ֵ!",fd+1);
		}
	}

    ret |= prPubSetConv(fd);
	if(ret != SET_OK)
	{
	   WriteWarnEvent(NULL, SYS_ERR_CFG, 0, "������ֵ%d�����ļ���ƥ��",fd+1);
	   prPubSetTableWrite();
	}
	
	memcpy(prSet[prRunPublic[fd].prSetnum], caSetBuf, SET_PUB_NUMBER*sizeof(TSETVAL));
	
	if (prRunPublic[fd].set_no == 0)
		prRunPublic[fd].set_no = 1;
	else
		prRunPublic[fd].set_no = 0;
	return ret;
}

int prFdSetRead(int fd)
{
   	char fname[MAXFILENAME];
   	struct VFileHead *head;
    TTABLETYPE *pType;
	enum SETRET ret = SET_OK;
   
       //pr disable 
   	if (pFdCfg[fd].cfg & 0x10) return ERROR;
   	
	head = (struct VFileHead *)prSetTmp;	
    pType = (TTABLETYPE*)(head+1);
	caSetBuf  = (BYTE *)(pType+1);

	sprintf(fname, "fd%dset.cfg", fd+1);
	if (ReadParaFile(fname, prSetTmp, sizeof(prSetTmp)) == ERROR)
	{
	    prFdSetDef(); 
	}
	else if (!prSetTypeJudgy(pType))
	{
	    prFdSetDef();
		ret = SET_OTHERERR;
		WriteWarnEvent(NULL, SYS_ERR_CFG, fd+1, "����%d���������ļ���ƥ��", fd+1);
	}
	else
	{
		if(FileSetNum <  SET_NUMBER)
		{
			//����Ĭ��ֵ
			prFdSetPartDef(FileSetNum);
			//�����ļ�
			if (WriteParaFile(fname, (struct VFileHead *)prSetTmp) == ERROR)
					ret = SET_OTHERERR;
			WriteDoEvent(NULL, 0, "����%d������ֵģ������������ȱ�ٵĲ��ֶ�ֵ!",fd+1);
		}
	}
	
    ret |= prFdSetConv(fd);
	if(ret != SET_OK)
	{
	   WriteWarnEvent(NULL, SYS_ERR_CFG, fd+1, "����%d������������", fd+1);
	   prSetTableWrite();
	}
	
	memcpy(prSet[fd+1], caSetBuf, SET_NUMBER*sizeof(TSETVAL));
	
	prFdSetNoSwitch(fd);


	return OK;
}

int TestPrSet(int fd)
{
	TSETVAL *tset = (TSETVAL*)(prSet[fd]);
	TSETVAL *bval;
	int num,i;
   	VPrRunInfo *pInfo = prRunInfo+fd;
	VPrRunSet *pprRunSet = prRunSet[pInfo->set_no]+fd;
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD len,yb;
	BYTE *ptemp;

	
	bval = tset + SET_I1;
	FloatToSet(1.5, bval);
	bval = tset + SET_I2;
	FloatToSet(1.0, bval);
	bval = tset + SET_3I01;
	FloatToSet(0.5, bval);
	bval = tset + SET_T1;
	FloatToSet(0, bval);
	bval = tset + SET_T2;
	FloatToSet(1, bval);
	bval = tset + SET_TI01;
	FloatToSet(2, bval);
	bval = tset + SET_CHNUM;
	FloatToSet(1, bval);
	num = 1;
	for (i=0; i<num; i++)
	{
	    bval = tset + SET_TCH1 + i;
		FloatToSet(2, bval);
	}

	yb = pprRunSet->dYb;
	yb &= ~(PR_YB_I02|PR_YB_I03);
	ReverseKSet(SET_YB, (BYTE*)tset, yb);

	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)tset, SET_NUMBER*sizeof(TSETVAL));
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		return 2;
	}		
	
	caSetBuf = prSet[fd];
	prFdSetConv(fd);
	
	return 0;

}

/*�Ƚ�װ�������ģ���ļ�����������ģ���ļ�ֻ�жϸ��������ж����� ����0,������ȫһ��
����1��ѹ�塢������1��������2������һ�����������ɱ���ģ�弴��
����2����ֵ������һ������Ҫ�������ɱ���ģ����Ҫ��д��ֵ�ļ�
����1: �ļ���ȡ���ݣ�����2: num ȡ��ֵ�ĸ���������3: fd = 0������ֵģ�壬1Ϊ����ģ��
*/	
int  prSetJudgyNum(BYTE *filetemp, BYTE* num ,int fd)
{
	BYTE* pTemp;
	BYTE kg_num,temp_num;
	int ret = 0;
	int i,j;

	//����
	pTemp = filetemp + sizeof(struct VFileHead);
	pTemp += sizeof(TTABLETYPE);
	kg_num = *pTemp;
	if(fd == 0) // ������ֵ
	{
		if(kg_num != YB_PUB_NUMBER)
			ret = 1;
		pTemp += 1;
		pTemp += sizeof(TYBTABLE)*kg_num;
		kg_num = *pTemp;
		if(kg_num != KG1_PUB_NUMBER)
			ret = 1;
		pTemp += 1;
		for(i = 0; i < kg_num ; i++)
		{
			pTemp += sizeof(TKGTABLE)-5;
			temp_num = *pTemp;
			pTemp++;
			for(j = 0 ; j < temp_num ; j++)
			{
				pTemp += (4*2+1);
			}		
		}
		kg_num = *pTemp;
		if(kg_num != KG2_PUB_NUMBER)
			ret = 1;
		pTemp++;

		for(i= 0 ; i < kg_num ; i++)
		{
			pTemp +=  sizeof(TKGTABLE)-5;
			temp_num = *pTemp;
			pTemp++;
			for( j = 0 ; j < temp_num ; j++)
			{
				pTemp += (4*2 + 1);
			}
		}
		kg_num = *pTemp;
		if(kg_num < SET_PUB_NUMBER) //�ȳ�����
			ret = 2;
		else if(kg_num > SET_PUB_NUMBER)
			ret = 1;
		*num = kg_num;
	}
	else
	{
		if(kg_num != YB_NUMBER)
			ret = 1;
		pTemp += 1;
		pTemp += sizeof(TYBTABLE)*kg_num;
		kg_num = *pTemp;
		if(kg_num != KG1_NUMBER)
			ret = 1;
		pTemp += 1;
		for(i = 0; i < kg_num ; i++)
		{
			pTemp += sizeof(TKGTABLE)-5;
			temp_num = *pTemp;
			pTemp++;
			for(j = 0 ; j < temp_num ; j++)
			{
				pTemp += (4*2+1);
			}		
		}
		kg_num = *pTemp;
		if(kg_num != KG2_NUMBER)
			ret = 1;
		pTemp++;

		for(i= 0 ; i < kg_num ; i++)
		{
			pTemp +=  sizeof(TKGTABLE)-5;
			temp_num = *pTemp;
			pTemp++;
			for( j = 0 ; j < temp_num ; j++)
			{
				pTemp += (4*2 + 1);
			}
		}
		kg_num = *pTemp;
		if(kg_num < SET_NUMBER) //�ȳ�����
			ret = 2;
		else if(kg_num > SET_NUMBER)
			ret = 1;
		*num = kg_num;
	}
	
	return ret;
}


int prSetInit(void)
{
    int i;
	prRunSet[0] = (VPrRunSet *)malloc(fdNum*sizeof(VPrRunSet));
	prRunSet[1] = (VPrRunSet *)malloc(fdNum*sizeof(VPrRunSet));

    if (!prRunSet[0] || !prRunSet[1] || !prRunInfo || !fdProtVal)  return ERROR;

    memset(prSetBuf, 0, sizeof(prSetBuf));
    memset(prSetTmp, 0, sizeof(prSetTmp));
	memset(prTableBuf, 0, sizeof(prTableBuf));

	prSetPublic[0] = (VPrSetPublic *)malloc(fdNum*sizeof(VPrSetPublic));
	prSetPublic[1] = (VPrSetPublic *)malloc(fdNum* sizeof(VPrSetPublic));

	if (!prSetPublic[0] || !prSetPublic[1]) return ERROR;	

	prNbSetDef();

	for(i = 0; i < fdNum; i++)
	{
		if(i == 0)
			prRunPublic[i].prSetnum = 0;
		else
			prRunPublic[i].prSetnum = fdNum + i;
	}
	

    if(prTableFileJudgy() == OK) //�б���ģ���ļ�ok������error
    {
		i = prSetJudgyNum(prTableBuf,&FileSetNum,1);
		if( i > 0)
			prSetTableWrite();
		
	    for (i=0; i<fdNum; i++)
	    {
			prSet[i+1] = prSetBuf + PR_SET_SIZE * (i+1);
			prFdSetRead(i);
		}

		if (ReadParaFile("ProSetPubTable.cfg", prTableBuf, sizeof(prTableBuf)) == ERROR)
		{
			prPubSetTableWrite();
			FileSetNum = SET_PUB_NUMBER;
		}
		else 
		{
			i = prSetJudgyNum(prTableBuf,&FileSetNum,0);
			if( i > 0)
				prPubSetTableWrite();
		}
		prSet[0] = prSetBuf;
		prPubSetRead(); //��һ���߹�����ֵ

		//�������߹�����ֵ
		for (i=1; i<fdNum; i++)
	    {
			prSet[prRunPublic[i].prSetnum] = prSetBuf + PR_SET_SIZE * (fdNum+i);
			prPubOtherSetRead(i);
		}
		
    }
	else
	{
	   	caSetBuf= prSetTmp;
	    prFdSetDef(); 
		for (i=0; i<fdNum; i++)
	    {
			prSet[i+1] = prSetBuf + PR_SET_SIZE * (i+1);
			prFdSetConv(i);
			memcpy(prSet[i+1], caSetBuf, SET_NUMBER*sizeof(TSETVAL));
	
			prFdSetNoSwitch(i);
		}

		prPubSetDef();
		prSet[0] = prSetBuf;
		prPubSetConv(0);
		memcpy(prSet[0], caSetBuf, SET_PUB_NUMBER*sizeof(TSETVAL));

		if (prRunPublic->set_no == 0)
		    prRunPublic->set_no = 1;
	    else
		    prRunPublic->set_no = 0;

		//�������߹�����ֵ
		for (i=1; i<fdNum; i++)
	    {
			prSet[fdNum+i] = prSetBuf + PR_SET_SIZE * (i+fdNum);
			prPubSetConv(i);
			memcpy(prSet[prRunPublic[i].prSetnum], caSetBuf, SET_PUB_NUMBER*sizeof(TSETVAL));

			if (prRunPublic[i].set_no == 0)
			    prRunPublic[i].set_no = 1;
		    else
			    prRunPublic[i].set_no = 0;
		}
	}
	return OK;
}

void prFdSetNoSwitch(int fd)
{
	VPrRunInfo *pInfo = prRunInfo+fd;
	
    if (pInfo->set_no == 0)
		pInfo->set_no = 1;
	else
		pInfo->set_no = 0;
}

#ifdef INCLUDE_FA_S
int ReadConsValueSet(int fd,int flag,BYTE*pBuf)
{
	VPrConsVlaueSet*pSet = (VPrConsVlaueSet*)pBuf;	
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunSet *pprRunSet;
	float	 fChNum;
	float fval;
	VPrRunInfo *pInfo;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
	

	switch(flag)
	{
		case PR_DLQ:
		case PR_FHKG:
		case PR_GLBGZ:
		case PR_DLQCH:
		case PR_CKTZ:
		case PR_CKII:
		case PR_BDZCH:
		case PR_DLJSCH:
			/*һ�ι���ѹ��*/
			if(pprRunSet->dYb&  PR_YB_I1)
				pSet->byIb = 0x1 ;
			else
				pSet->byIb = 0x0 ;

			/*���ι���ѹ��*/
			if(pprRunSet->dYb&  PR_YB_I2)
				pSet->byIIb = 0x1 ;
			else
				pSet->byIIb = 0x0 ;
			
			/*�غ�բѹ��*/	
			if(pprRunSet->dYb & PR_YB_CH)
				pSet->bCHZYB = 0x1;
			else
				pSet->bCHZYB = 0x0;
			/*һ�ι�����ֵ*/	
			bval = tset + SET_I1;
			SetToFloat(bval,&fval );
			pSet->fI1GLDZ = fval;
			/*һ�ι���ʱ��*/
			bval = tset + SET_T1;
			SetToFloat(bval,&fval);
			pSet->fT1GL = fval;
			/*��բ��ʽ*/
			pSet->bTZFS =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_TRIP_MODE) >> KG_17) + 1 ;		
			/*�����Ϸ�ʽ*/
			pSet->bFAReport = (BYTE)((pprRunSet->dKG1 & PR_CFG1_FAULT_REPORT) >> (KG_20)) + 1;	

			/*һ�γ���ѹ��*/		
			pSet->byIPrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	
			
			/*�غ�բ����*/
			bval = tset + SET_CHNUM;
			SetToFloat(bval,&fChNum);
			pSet->bCHNum = (BYTE)fChNum;
			/*һ���غ�բʱ��*/
			bval = tset + SET_TCH1;
			SetToFloat(bval,&fval);
			pSet->fTCH1 = fval;
			/*�����غ�բʱ��*/
			bval = tset + SET_TCH2;
			SetToFloat(bval,&fval);
			pSet->fTCH2 = fval;
			/*�����غ�ʱ��*/
			bval = tset + SET_TCH3;
			SetToFloat(bval,&fval);
			pSet->fTCH3 = fval;
			/*�غϱ���ʱ��*/
			bval = tset + SET_TCHBS;
			SetToFloat(bval,&fval);
			pSet->fTCHBS = fval;
			/*���ι�����ֵ*/	
			bval = tset + SET_I2;
			SetToFloat(bval,&fval );
			pSet->fI2GLDZ = fval;
			/*����ʱ�䶨ֵ*/
			bval = tset + SET_T2;
			SetToFloat(bval,&fval);
			pSet->fT2GL = fval;

			bval = tset + SET_IANG1;
			SetToFloat(bval,&fval );
			pSet->AngleLow = fval;
			
			bval = tset + SET_IANG2;
			SetToFloat(bval,&fval);
			pSet->AngleUpp = fval;

			bval = tset + SET_3U0;
			SetToFloat(bval,&fval);
			pSet->XDL_U0 = fval;

			bval = tset + SET_U0TB;
			SetToFloat(bval,&fval);
			pSet->XDL_U0_diff = fval;

			bval = tset + SET_I0TB;
			SetToFloat(bval,&fval);
			pSet->XDL_I0_diff = fval;

			bval = tset + SET_ITB;
			SetToFloat(bval,&fval);
			pSet->XDL_I_diff = fval;

			bval = tset + SET_TDXJD;
			SetToFloat(bval,&fval);
			pSet->XDL_Time = fval;

			bval = tset + SET_XDLCOEF;
			SetToFloat(bval,&fval);
			pSet->XDL_Coef = fval;	

			/*���γ���ѹ��*/		
			pSet->byIIPrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I2_TRIP) >> (KG_28))  ;	

			if(pprRunSet->dKG1&  PR_CFG1_I_DIR)
				pSet->byGLFx = 0x1 ;
			else
				pSet->byGLFx = 0x0 ;

			if(pprRunSet->dKG2&  PR_CFG2_IYX_DIR)
				pSet->byGLYx = 0x1 ;
			else
				pSet->byGLYx = 0x0 ;

			if(pprRunSet->dKG2&  PR_CFG2_XDL_YB)
				pSet->byXDLJD = 0x1 ;
			else
				pSet->byXDLJD = 0x0 ;

			if(pprRunSet->dKG2&  PR_CFG2_XDL_TRIP)
				pSet->byXDLJD_Trip = 0x1 ;
			else
				pSet->byXDLJD_Trip = 0x0 ;

			break;
		default:
			break;
   }
	return 0;
}		
int ReadVoltageSet(int fd, BYTE*pBuf)
{
	VPrdyxSet *pSet = (VPrdyxSet*)pBuf;	
	TSETVAL *tset;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);	
	TSETVAL *bval;
	VPrSetPublic *pprRunSet;
	VPrRunSet *pprRunSet1;	
	VPrRunInfo *pInfo;
	float fval;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);

	/*����ʧѹ��բ*/
	if(pprRunSet->dYb&  PR_YB_SUDY)
		pSet->BSFunyb = 0x1 ;
	else
		pSet->BSFunyb = 0x0 ;
	/*���翪�ع���ѹ��*/
	if(pprRunSet->dYb&  PR_YB_SUSY )
		pSet->BLFunyb = 0x1 ;
	else
		pSet->BLFunyb = 0x0 ;

	if(pprRunSet->dYb&  PR_YB_SLWY)
		pSet->LoseVolYb = 0x1;    
	else
		pSet->LoseVolYb = 0x0;
	
	if(pprRunSet->dYb&  PR_YB_U0)
		pSet->U0Yb = 0x1;    
	else
		pSet->U0Yb = 0x0;
	
	if(pprRunSet->dKG1 &  PR_CFG1_X_BS)
		pSet->BXbsyb = 0x1;    
	else
		pSet->BXbsyb = 0x0;

	if(pprRunSet->dKG1 &  PR_CFG1_Y_BS)
		pSet->BYbsyb = 0x1;    
	else
		pSet->BYbsyb = 0x0;

	if(pprRunSet->dKG1 &  PR_CFG1_SY_BS)
		pSet->BSybsyb = 0x1;    
	else
		pSet->BSybsyb = 0x0;

	if(pprRunSet->dKG1 &  PR_CFG1_FZ_BS)
		pSet->FZBS = 0x1;    
	else
		pSet->FZBS = 0x0;
	
	
	 /*SL���ߺ�*/
	 pSet->BSLLineNo = pprRunSet->bySLLine + 1;
	 /*Xʱ��*/
	 bval = tset + SET_PUB_TX;	
	 SetToFloat(bval,&fval);
	 pSet->fXsx = fval;
	 /*Yʱ��*/
	 bval = tset + SET_PUB_TY;
	 SetToFloat(bval,&fval);
	 pSet->fYsx = fval;
	 /*Zʱ��*/
	 bval = tset + SET_PUB_TWY;
	 SetToFloat(bval,&fval);
	 pSet->fZsx = fval;
	  /*��բ��������ʱ��*/
	 bval = tset + SET_PUB_TFZ;
	 SetToFloat(bval,&fval);
	 pSet->fZbstime = fval;
	 
	/*��ѹʱ��*/
	 bval = tset + SET_PUB_TSYCY;
	 SetToFloat(bval,&fval);
	 pSet->fTcytime = fval;
	 /* ��ѹ��ֵ */
	 bval = tset + SET_PUB_UCY;
	 SetToFloat(bval,&fval);
	 pSet->cydz = fval;
	 //��ѹ��ֵ
	 bval = tset + SET_PUB_U0;
	 SetToFloat(bval,&fval);
	 pSet->U0Dz = fval;

	 if(pprRunSet1->dKG2&  PR_CFG2_XDL_YB)
		 pSet->byXDLJD = 0x1 ;
	 else
		 pSet->byXDLJD = 0x0 ;
	 
	 if(pprRunSet1->dKG2&  PR_CFG2_XDL_TRIP)
		 pSet->byXDLJD_Trip = 0x1 ;
	 else
		 pSet->byXDLJD_Trip = 0x0 ;
	 
	 bval = tset1 + SET_3U0;
	 SetToFloat(bval,&fval);
	 pSet->XDL_U0 = fval;
	 
	 bval = tset1 + SET_U0TB;
	 SetToFloat(bval,&fval);
	 pSet->XDL_U0_diff = fval;
	 
	 bval = tset1 + SET_I0TB;
	 SetToFloat(bval,&fval);
	 pSet->XDL_I0_diff = fval;
	 
	 bval = tset1 + SET_ITB;
	 SetToFloat(bval,&fval);
	 pSet->XDL_I_diff = fval;
	 
	 bval = tset1 + SET_TDXJD;
	 SetToFloat(bval,&fval);
	 pSet->XDL_Time = fval;
	 
	 bval = tset1 + SET_XDLCOEF;
	 SetToFloat(bval,&fval);
	 pSet->XDL_Coef = fval;  
	 
	return 0;
}

int ReadCurrentSet(int fd, BYTE*pBuf)
{
	VPrdljsxSet*pSet = (VPrdljsxSet*)pBuf;	
	TSETVAL *tset;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	TSETVAL *bval1;
	VPrSetPublic *pprRunSet;
	VPrRunSet *pprRunSet1;
	float bCHNum;
	float fval;
	VPrRunInfo *pInfo;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
	/*��������ѹ��*/
 	if(pprRunSet->dYb & PR_YB_I_DL)		
  		pSet->BDljsyb = 0x1;
	else
		pSet->BDljsyb = 0x0;
	/*SL���ߺ�*/
	pSet->BSLLineNo = pprRunSet->bySLLine + 1;		
	/*������������*/
	pSet->BDljsNUM = (BYTE)pprRunSet->dICnt ; 
	/*����������λʱ��*/
	bval = tset + SET_PUB_TICNT1;
	SetToFloat(bval,&fval);
	pSet->fTdljsReset = fval;
	/*���������ۼ�ʱ��*/
	bval = tset + SET_PUB_TICNT2;
	SetToFloat(bval,&fval);
	pSet->fTdljsTotal = fval;

	/*һ�ι���ѹ��*/
	if(pprRunSet1->dYb&  PR_YB_I1)
		pSet->OCIYb = 0x1 ;
	else
		pSet->OCIYb = 0x0 ;
	/*�غ�բѹ��*/	
	if(pprRunSet1->dYb & PR_YB_CH)
		pSet->bCHZYB = 0x1;
	else
		pSet->bCHZYB = 0x0;
	/*һ�ι�����ֵ*/	
	bval1 = tset1 + SET_I1;
	SetToFloat(bval1,&fval );
	pSet->OCIDz = fval;
	/*һ�ι���ʱ��*/
	bval1 = tset1 + SET_T1;
	SetToFloat(bval1,&fval);
	pSet->OCITime = fval;
	/*��բ��ʽ*/
	pSet->bTZFS =  (BYTE)((pprRunSet1->dKG1 & PR_CFG1_TRIP_MODE) >> KG_17) + 1 ;		
	/*�����Ϸ�ʽ*/
	pSet->bFAReport = (BYTE)((pprRunSet1->dKG1 & PR_CFG1_FAULT_REPORT) >> (KG_20)) + 1;	


	/*�غ�բ����*/
	bval1 = tset1 + SET_CHNUM;
	SetToFloat(bval1,&bCHNum);
	pSet->bCHNum = (BYTE)bCHNum;
	/*һ���غ�բʱ��*/
	bval1 = tset1 + SET_TCH1;
	SetToFloat(bval1,&fval);
	pSet->fTCH1 = fval;
	/*�����غ�բʱ��*/
	bval1 = tset1 + SET_TCH2;
	SetToFloat(bval1,&fval);
	pSet->fTCH2 = fval;
	/*�����غ�ʱ��*/
	bval1 = tset1 + SET_TCH3;
	SetToFloat(bval1,&fval);
	pSet->fTCH3 = fval;
	/*�غϱ���ʱ��*/
	bval1 = tset1 + SET_TCHBS;
	SetToFloat(bval1,&fval);
	pSet->fTCHBS = fval;

	/*һ�γ���ѹ��*/	
	pSet->byIPrYb =  (BYTE)((pprRunSet1->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	
	
	
	return 0;
}

int ReadZsyVoltageSet(int fd, BYTE*pBuf)
{
		VPrzsyxSet *pSet = (VPrzsyxSet*)pBuf; 
		TSETVAL *tset;
		TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
		TSETVAL *bval;
		TSETVAL *bval1;
		VPrSetPublic *pprRunSet;
		VPrRunSet *pprRunSet1;
		VPrRunInfo *pInfo;
		float fval;

		tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
		pInfo = prRunInfo + fd;
		pprRunSet = prSetPublic[pInfo->set_no]+fd;
		pprRunSet1 = prRunSet[pInfo->set_no]+fd;
		/*����ʧѹ��բ*/
		if(pprRunSet->dYb&  PR_YB_SUDY)
			pSet->BSFunyb = 0x1 ;
		else
			pSet->BSFunyb = 0x0 ;
		/*���翪�ع���ѹ��*/
		if(pprRunSet->dYb&  PR_YB_SUSY )
			pSet->BLFunyb = 0x1 ;
		else
			pSet->BLFunyb = 0x0 ;

		if(pprRunSet->dYb&  PR_YB_SLWY)
			pSet->LoseVolYb = 0x1;    
		else
			pSet->LoseVolYb = 0x0;  

		if(pprRunSet->dYb&  PR_YB_U0)
			pSet->U0Yb = 0x1;    
		else
			pSet->U0Yb = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_X_BS)
			pSet->BXbsyb = 0x1;    
		else
			pSet->BXbsyb = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_Y_BS)
			pSet->BYbsyb = 0x1;    
		else
			pSet->BYbsyb = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_SY_BS)
			pSet->BSybsyb = 0x1;    
		else
			pSet->BSybsyb = 0x0;

		
		if(pprRunSet->dKG1 &  PR_CFG1_FZ_BS)
			pSet->FZBS = 0x1;    
		else
			pSet->FZBS = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_ZSY_XJDL)
			pSet->byxjdl= 0x1;    
		else
			pSet->byxjdl = 0x0;

		/*SL���ߺ�*/
		pSet->BSLLineNo = pprRunSet->bySLLine + 1;
		/*Xʱ��*/
		bval = tset + SET_PUB_TX;  
		SetToFloat(bval,&fval);
		pSet->fXsx = fval;
		/*Yʱ��*/
		bval = tset + SET_PUB_TY;
		SetToFloat(bval,&fval);
		pSet->fYsx = fval;
		/*Zʱ��*/
		bval = tset + SET_PUB_TWY;
		SetToFloat(bval,&fval);
		pSet->fZsx = fval;
		/*Sʱ��*/
		bval = tset + SET_PUB_TS;
		SetToFloat(bval,&fval);
		pSet->fSsx = fval;
		/*Cʱ��*/
		bval = tset + SET_PUB_TC;
		SetToFloat(bval,&fval);
		pSet->fCsx = fval;
		/*��ѹʱ��*/
		bval = tset + SET_PUB_TSYCY;
		SetToFloat(bval,&fval);
		pSet->fTcytime = fval;
		/*��ѹ��ֵ*/
		bval = tset + SET_PUB_UCY;
		SetToFloat(bval,&fval);
		pSet->cydz = fval;

		 //��ѹ��ֵ
		bval = tset + SET_PUB_U0;
		SetToFloat(bval,&fval);
		pSet->U0Dz = fval;
		
		 //��բ��������ʱ��
		bval = tset + SET_PUB_TFZ;
		SetToFloat(bval,&fval);
		pSet->fZbstime = fval;

		
		/*һ�ι���ѹ��*/	
		if(pprRunSet1->dYb &  PR_YB_I1)
			pSet->byIb = 0x1 ;
		else
			pSet->byIb = 0x0 ;

		if(pprRunSet1->dKG2&  PR_CFG2_XDL_YB)
			pSet->byXDLJD = 0x1 ;
		else
			pSet->byXDLJD = 0x0 ;
		
		if(pprRunSet1->dKG2&  PR_CFG2_XDL_TRIP)
			pSet->byXDLJD_Trip = 0x1 ;
		else
			pSet->byXDLJD_Trip = 0x0 ;
		
		/*һ�ι�����ֵ*/	
		bval1 = tset1 + SET_I1;
		SetToFloat(bval1,&fval );
		pSet->fI1GLDZ = fval;
		/*һ�ι���ʱ��*/
		bval1 = tset1 + SET_T1;
		SetToFloat(bval1,&fval);
		pSet->fT1GL = fval;
		/*һ�γ���ѹ��*/
		pSet->byIPrTripYb =  (BYTE)((pprRunSet1->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	

		bval = tset1 + SET_3U0;
		SetToFloat(bval,&fval);
		pSet->XDL_U0 = fval;
		
		bval = tset1 + SET_U0TB;
		SetToFloat(bval,&fval);
		pSet->XDL_U0_diff = fval;
		
		bval = tset1 + SET_I0TB;
		SetToFloat(bval,&fval);
		pSet->XDL_I0_diff = fval;
		
		bval = tset1 + SET_ITB;
		SetToFloat(bval,&fval);
		pSet->XDL_I_diff = fval;
		
		bval = tset1 + SET_TDXJD;
		SetToFloat(bval,&fval);
		pSet->XDL_Time = fval;
		
		bval = tset1 + SET_XDLCOEF;
		SetToFloat(bval,&fval);
		pSet->XDL_Coef = fval;	

		if(pprRunSet->dKG1 &  PR_CFG1_ZSY_DXJD)
			pSet->bYbZsyjd= 0x1;    
		else
			pSet->bYbZsyjd = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_DU_BS)
			pSet->bYbDu= 0x1;    
		else
			pSet->bYbDu = 0x0;
		
		if(pprRunSet->dKG1 &  PR_CFG1_U0_TRIP)
			pSet->bYbU0Trip= 0x1;    
		else
			pSet->bYbU0Trip = 0x0;

		bval = tset + SET_PUB_TU0;
		SetToFloat(bval,&fval);
		pSet->fTU0 = fval;
		
		return 0;
  }
int ReadDyDlVoltageSet(int fd, BYTE*pBuf)
{
		VPrdydlSet *pSet = (VPrdydlSet*)pBuf; 
		TSETVAL *tset;
		TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
		TSETVAL *bval;
		TSETVAL *bval1;
		VPrSetPublic *pprRunSet;
		VPrRunSet *pprRunSet1;
		VPrRunInfo *pInfo;
		float fval;

		tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
		pInfo = prRunInfo + fd;
		pprRunSet = prSetPublic[pInfo->set_no]+fd;
		pprRunSet1 = prRunSet[pInfo->set_no]+fd;
		/*�����ѹ*/
		if(pprRunSet->dYb&  PR_YB_SUDY)
			pSet->BSFunyb = 0x1 ;
		else
			pSet->BSFunyb = 0x0 ;
		
		/*����ʧѹ*/
		if(pprRunSet->dYb&  PR_YB_SUSY )
			pSet->BLFunyb = 0x1 ;
		else
			pSet->BLFunyb = 0x0 ;
		
		/* ��ѹ��բ*/
		if(pprRunSet->dYb&  PR_YB_SLWY)
			pSet->LoseVolYb = 0x1;    
		else
			pSet->LoseVolYb = 0x0;	

		if(pprRunSet->dYb&  PR_YB_U0)
			pSet->U0Yb = 0x1;    
		else
			pSet->U0Yb = 0x0;
		
		/* X����ѹ��*/
		if(pprRunSet->dKG1 &  PR_CFG1_X_BS)
			pSet->BXbsyb = 0x1;    
		else
			pSet->BXbsyb = 0x0;
		/* Y����ѹ��*/
		if(pprRunSet->dKG1 &  PR_CFG1_Y_BS)
			pSet->BYbsyb = 0x1;    
		else
			pSet->BYbsyb = 0x0;
		
		/* ��ѹ����ѹ��*/
		if(pprRunSet->dKG1 &  PR_CFG1_SY_BS)
			pSet->BSybsyb = 0x1;    
		else
			pSet->BSybsyb = 0x0;

			/* ��ѹ����ѹ��*/
		if(pprRunSet->dKG1 &  PR_CFG1_FZ_BS)
			pSet->FZBS = 0x1;    
		else
			pSet->FZBS = 0x0;
		
		/*��ѹ������ѹ��*/
		if(pprRunSet->dKG1 &  PR_CFG1_UI_TYPE)  
			pSet->BDyDlYb = 0x1;    
		else
			pSet->BDyDlYb = 0x0;

		if(pprRunSet->dKG1 &  PR_CFG1_DU_BS)  
			pSet->bYbDu = 0x1;    
		else
			pSet->bYbDu = 0x0;
		
		if(pprRunSet->dKG1 &  PR_CFG1_U0_TRIP)  
			pSet->bYbU0Trip = 0x1;    
		else
			pSet->bYbU0Trip = 0x0;
		
		//���ߺ�	
		 pSet->BSLLineNo = pprRunSet->bySLLine + 1;
		// Xʱ��
		 bval = tset + SET_PUB_TX;	
		 SetToFloat(bval,&fval);
		 pSet->fXsx = fval;
		 //Yʱ��
		 bval = tset + SET_PUB_TY;
		 SetToFloat(bval,&fval);
		 pSet->fYsx = fval;
		 //Zʱ��
		 bval = tset + SET_PUB_TWY;
		 SetToFloat(bval,&fval);
		 pSet->fZsx = fval;
		 //��ѹʱ��
		 bval = tset + SET_PUB_TSYCY;
		 SetToFloat(bval,&fval);
		 pSet->fTcytime = fval;
		 //��ѹ��ֵ
		 bval = tset + SET_PUB_UCY;
		 SetToFloat(bval,&fval);
		 pSet->cydz = fval;
		//��ѹ��ֵ
		bval = tset + SET_PUB_U0;
		SetToFloat(bval,&fval);
		pSet->U0Dz = fval;

		//��ѹʱ��
		bval = tset + SET_PUB_TU0;
		SetToFloat(bval,&fval);
		pSet->fTU0 = fval;

		//��բ��������ʱ��
		bval = tset + SET_PUB_TFZ;
		SetToFloat(bval,&fval);
		pSet->fZbstime = fval;
		
		// ����һ��ѹ��
		if(pprRunSet1->dYb &  PR_YB_I1)
			pSet->byIb = 0x1 ;
		else
			pSet->byIb = 0x0 ;
		//����һ�ζ�ֵ
		bval1 = tset1 + SET_I1;
		SetToFloat(bval1,&fval );
		pSet->fI1GLDZ = fval;
		//����һ��ʱ�䶨ֵ
		bval1 = tset1 + SET_T1;
		SetToFloat(bval1,&fval);
		pSet->fT1GL = fval;
		//����һ�γ���ѹ��
		pSet->byIPrTripYb =  (BYTE)((pprRunSet1->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	

		if(pprRunSet1->dKG2&  PR_CFG2_XDL_YB)
			pSet->byXDLJD = 0x1 ;
		else
			pSet->byXDLJD = 0x0 ;
		
		if(pprRunSet1->dKG2&  PR_CFG2_XDL_TRIP)
			pSet->byXDLJD_Trip = 0x1 ;
		else
			pSet->byXDLJD_Trip = 0x0 ;
		
		bval = tset1 + SET_3U0;
		SetToFloat(bval,&fval);
		pSet->XDL_U0 = fval;
		
		bval = tset1 + SET_U0TB;
		SetToFloat(bval,&fval);
		pSet->XDL_U0_diff = fval;
		
		bval = tset1 + SET_I0TB;
		SetToFloat(bval,&fval);
		pSet->XDL_I0_diff = fval;
		
		bval = tset1 + SET_ITB;
		SetToFloat(bval,&fval);
		pSet->XDL_I_diff = fval;
		
		bval = tset1 + SET_TDXJD;
		SetToFloat(bval,&fval);
		pSet->XDL_Time = fval;
		
		bval = tset1 + SET_XDLCOEF;
		SetToFloat(bval,&fval);
		pSet->XDL_Coef = fval;	

		
		return 0;
  }

int ReadI0ValueSet(int fd,BYTE*pBuf)
{
	VPrI0VlaueSet*pSet = (VPrI0VlaueSet*)pBuf;	
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunSet *pprRunSet;
	float	 fChNum;
	float fval;
	VPrRunInfo *pInfo;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
		
	/*һ�ι���ѹ��*/
	if(pprRunSet->dYb&  PR_YB_I1)
		pSet->byIb = 0x1 ;
	else
		pSet->byIb = 0x0 ;

	/*���ι���ѹ��*/
	if(pprRunSet->dYb&  PR_YB_I2)
		pSet->byIIb = 0x1 ;
	else
		pSet->byIIb = 0x0 ;

	/*����һ��ѹ��*/
	if(pprRunSet->dYb&  PR_YB_I01)
		pSet->bI0Yb = 0x1 ;
	else
		pSet->bI0Yb = 0x0 ;

	/*�غ�բѹ��*/	
	if(pprRunSet->dYb & PR_YB_CH)
		pSet->bCHZYB = 0x1;
	else
		pSet->bCHZYB = 0x0;
	/*һ�ι�����ֵ*/	
	bval = tset + SET_I1;
	SetToFloat(bval,&fval );
	pSet->fI1GLDZ = fval;
	/*һ�ι���ʱ��*/
	bval = tset + SET_T1;
	SetToFloat(bval,&fval);
	pSet->fT1GL = fval;
	/*��բ��ʽ*/
	pSet->bTZFS =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_TRIP_MODE) >> KG_17) + 1 ;		
	/*�����Ϸ�ʽ*/
	pSet->bFAReport = (BYTE)((pprRunSet->dKG1 & PR_CFG1_FAULT_REPORT) >> (KG_20)) + 1;	

	/*һ�γ���ѹ��*/		
	pSet->byIPrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I1_TRIP) >> (KG_27))  ;	

	/*�غ�բ����*/
	bval = tset + SET_CHNUM;
	SetToFloat(bval,&fChNum);
	pSet->bCHNum = (BYTE)fChNum;
	/*һ���غ�բʱ��*/
	bval = tset + SET_TCH1;
	SetToFloat(bval,&fval);
	pSet->fTCH1 = fval;
	/*�����غ�բʱ��*/
	bval = tset + SET_TCH2;
	SetToFloat(bval,&fval);
	pSet->fTCH2 = fval;
	/*�����غ�ʱ��*/
	bval = tset + SET_TCH3;
	SetToFloat(bval,&fval);
	pSet->fTCH3 = fval;
	/*�غϱ���ʱ��*/
	bval = tset + SET_TCHBS;
	SetToFloat(bval,&fval);
	pSet->fTCHBS = fval;
	/*���ι�����ֵ*/	
	bval = tset + SET_I2;
	SetToFloat(bval,&fval );
	pSet->fI2GLDZ = fval;
	/*����ʱ�䶨ֵ*/
	bval = tset + SET_T2;
	SetToFloat(bval,&fval);
	pSet->fT2GL = fval;
	
	/*����һ�ζ�ֵ*/
	bval = tset + SET_3I01;
	SetToFloat(bval,&fval);
	pSet->fI0Dz = fval;

	/*����һ��ʱ��*/
	bval = tset + SET_TI01;
	SetToFloat(bval,&fval);
	pSet->fTI0 = fval;

	
	bval = tset + SET_IANG1;
	SetToFloat(bval,&fval );
	pSet->AngleLow = fval;

	bval = tset + SET_IANG2;
	SetToFloat(bval,&fval);
	pSet->AngleUpp = fval;
	
	bval = tset + SET_3U0;
	SetToFloat(bval,&fval);
	pSet->XDL_U0 = fval;

	bval = tset + SET_U0TB;
	SetToFloat(bval,&fval);
	pSet->XDL_U0_diff = fval;

	bval = tset + SET_I0TB;
	SetToFloat(bval,&fval);
	pSet->XDL_I0_diff = fval;

	bval = tset + SET_ITB;
	SetToFloat(bval,&fval);
	pSet->XDL_I_diff = fval;

	bval = tset + SET_TDXJD;
	SetToFloat(bval,&fval);
	pSet->XDL_Time = fval;

	bval = tset + SET_XDLCOEF;
	SetToFloat(bval,&fval);
	pSet->XDL_Coef = fval;	
	

	/*����һ�γ���*/
	pSet->byI0PrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I0_TRIP) >> (KG_29))  ;	
	/*���γ���ѹ��*/		
	pSet->byIIPrTripYb =  (BYTE)((pprRunSet->dKG1 & PR_CFG1_I2_TRIP) >> (KG_28))  ;
	
	if(pprRunSet->dKG1&  PR_CFG1_I_DIR)
		pSet->byGLFx = 0x1 ;
	else
		pSet->byGLFx = 0x0 ;

	if(pprRunSet->dKG2&  PR_CFG2_IYX_DIR)
		pSet->byGLYx = 0x1 ;
	else
		pSet->byGLYx = 0x0 ;

	if(pprRunSet->dKG2&  PR_CFG2_XDL_YB)
		 pSet->byXDLJD = 0x1 ;
	 else
		 pSet->byXDLJD = 0x0 ;
	 
	 if(pprRunSet->dKG2&  PR_CFG2_XDL_TRIP)
		 pSet->byXDLJD_Trip = 0x1 ;
	 else
		 pSet->byXDLJD_Trip = 0x0 ;
	 
   
	return 0;
}
int FaSimGetPara(int fd, int flag,BYTE *pBuf)
{
	
	if (g_prInit == 0)
		return 0;
	
	smMTake(prSetSem);//?�Ƿ���Ҫ

	switch(flag)
	{
		case PR_DLQ:
		case PR_FHKG:
		case PR_GLBGZ:
		case PR_DLQCH:
		case PR_CKTZ:
		case PR_CKII:
		case PR_BDZCH:
		case PR_DLJSCH:
			ReadConsValueSet(fd,flag,pBuf);
			break;	
		case PR_DYZ:
			ReadVoltageSet(fd, pBuf);
			break;
		case PR_DLJSX:
			ReadCurrentSet(fd, pBuf);
			break;
		case PR_ZSYX:
			ReadZsyVoltageSet(fd, pBuf);
		  	break;
		case PR_DYDLX:
			ReadDyDlVoltageSet(fd, pBuf);
		  	break;
		case PR_CKI0TZ:
			ReadI0ValueSet(fd,pBuf);
			break;
		default:
			break;
	}

	smMGive(prSetSem);
  
	return 0;
}
void ClearVoltageYB(int fd)

{
	TSETVAL *tset = (TSETVAL*)(prSet[0]);
	VPrSetPublic *pprRunSet;
	VPrRunInfo *pInfo;
		
	DWORD yb;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;

	/*���������ֵ����ѹ��*/
	yb = pprRunSet->dYb;
	yb &= ~ PR_PUBYB_MASK;
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);



}
void ClearYBSet(int fd)
{
	TSETVAL *tset;
	VPrSetPublic *pprRunSet;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet1;
	
		
	DWORD yb,yb1;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
		
	/*���������ֵ����ѹ��*/
	yb = pprRunSet->dYb;
	yb &= ~ PR_PUBYB_MASK;
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);

	yb = pprRunSet->dKG1;
	yb &= ~ PR_PUBCFG1_MASK;
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);

	/*�������1����ѹ��*/
	yb1 = pprRunSet1->dYb;
   	yb1 &= ~PR_YB_MASK;	
	pprRunSet1->dYb = yb1;		
   	ReverseKSet(SET_YB, (BYTE*)tset1, yb1);
	
	yb1 = pprRunSet1->dKG1;
   	yb1 &= ~ PR_CFG1_MASK;	
	pprRunSet1->dKG1 = yb1;		
   	ReverseKSet(SET_KG1, (BYTE*)tset1, yb1);

	yb1 = pprRunSet1->dKG2;
   	yb1 &= ~ PR_CFG2_MASK;	
	pprRunSet1->dKG2 = yb1;		
   	ReverseKSet(SET_KG2, (BYTE*)tset1, yb1);

	caSetBuf = prSet[prRunPublic[fd].prSetnum];
	prPubSetConv(fd);

	if (prRunPublic[fd].set_no == 0)
		prRunPublic[fd].set_no = 1;
	else
		prRunPublic[fd].set_no = 0;
	
	caSetBuf = prSet[fd+1];	
	prFdSetConv(fd);

	prFdSetNoSwitch(fd);
	
}
int WriteConsValueSet(int fd, int flag,BYTE*pSet)
{
	VPrConsVlaueSet*ptr = (VPrConsVlaueSet*)pSet;
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunSet *pprRunSet;
	VPrRunInfo *pInfo;
		
	DWORD yb,TZFS,FAReport,CHNum,PrITripYb,PrIITripYb;

	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;


	switch(flag)
	{
		case PR_DLQ:
		case PR_FHKG:
		case PR_GLBGZ:
		case PR_DLQCH:
		case PR_CKTZ:
		case PR_CKII:
		case PR_BDZCH:
		case PR_DLJSCH:
			/*һ�ι���ѹ��*/
		 	yb = pprRunSet->dYb;
		  	if (ptr->byIb==0)
	       		 yb &= ~PR_YB_I1;
		   	else
				 yb |= PR_YB_I1;
			/*���ι���ѹ��   */
			if (ptr->byIIb==0)
	       		 yb &= ~PR_YB_I2;
		   	else
				 yb |= PR_YB_I2;
			/*�غ�բѹ��*/
		  	if ((ptr->bCHZYB == 0))
	       		 yb &= ~PR_YB_CH ;
		   	else
				 yb |= PR_YB_CH ;
			pprRunSet->dYb = yb;	
		   	ReverseKSet(SET_YB, (BYTE*)tset, yb);
			/*һ�ι�����ֵ*/
			bval = tset + SET_I1;
			FloatToSet(ptr->fI1GLDZ, bval);
			/*һ�ι���ʱ��*/
			bval = tset + SET_T1;
			FloatToSet(ptr->fT1GL, bval);
			/*���ι�����ֵ*/
			bval = tset + SET_I2;
			FloatToSet(ptr->fI2GLDZ, bval);
			/*���ι���ʱ��*/
			bval = tset + SET_T2;
			FloatToSet(ptr->fT2GL, bval);

			bval = tset + SET_IANG1;
			FloatToSet(ptr->AngleLow, bval);

			bval = tset + SET_IANG2;
			FloatToSet(ptr->AngleUpp, bval);
			
			
			/*��բ��ʽ*/	
			TZFS  = (pprRunSet->dKG1&~PR_CFG1_TRIP_MODE) | ((ptr->bTZFS - 1) << KG_17) ;  // D17-D19 ��բ��ʽ
			pprRunSet->dKG1 = TZFS;	
			ReverseKSet(SET_KG1, (BYTE*)tset, TZFS);
			/*�����Ϸ�ʽ*/
			FAReport = (pprRunSet->dKG1&~PR_CFG1_FAULT_REPORT) |( (ptr->bFAReport - 1) << (KG_20)); // D20-D21 �ϱ����Ϸ�ʽ
			pprRunSet->dKG1 = FAReport;
			ReverseKSet(SET_KG1, (BYTE*)tset, FAReport);

			/*һ�γ���ѹ��*/	
			PrITripYb = (pprRunSet->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrTripYb) << (KG_27)); //��������Ͷ�� D27
			pprRunSet->dKG1 = PrITripYb;
			ReverseKSet(SET_KG1, (BYTE*)tset, PrITripYb);

			/*���γ���ѹ��*/	
			PrIITripYb = (pprRunSet->dKG1 & ~PR_CFG1_I2_TRIP) |( (ptr->byIIPrTripYb) << (KG_28)); //��������Ͷ�� D28
			pprRunSet->dKG1 = PrIITripYb;
			ReverseKSet(SET_KG1, (BYTE*)tset, PrIITripYb);

			yb = pprRunSet->dKG1;
			if (ptr->byGLFx == 0) 
				yb &= ~ PR_CFG1_I_DIR;
			else
				yb |=  PR_CFG1_I_DIR;	
			pprRunSet->dKG1 = yb;
			ReverseKSet(SET_KG1, (BYTE*)tset, yb);

			yb = pprRunSet->dKG2;
			if (ptr->byGLYx == 0) 
				yb &= ~ PR_CFG2_IYX_DIR;
			else
				yb |=  PR_CFG2_IYX_DIR;	

			if (ptr->byXDLJD == 0) 
				yb &= ~ PR_CFG2_XDL_YB;
			else
				yb |=  PR_CFG2_XDL_YB;	

			if (ptr->byXDLJD_Trip == 0) 
				yb &= ~ PR_CFG2_XDL_TRIP;
			else
				yb |=  PR_CFG2_XDL_TRIP;				
			
			pprRunSet->dKG2 = yb;
			ReverseKSet(SET_KG2, (BYTE*)tset, yb);
			
			/*�غ�բ����*/
			CHNum = ptr->bCHNum; 	
			bval = tset + SET_CHNUM;	
			FloatToSet(CHNum, bval);			
			
			/*һ���غ�բʱ��*/
			bval = tset + SET_TCH1;
			FloatToSet(ptr->fTCH1, bval);
			/*�����غ�բʱ��*/
			bval = tset + SET_TCH2;
			FloatToSet(ptr->fTCH2, bval);
			/*�����غ�բʱ��*/
			bval = tset + SET_TCH3;
			FloatToSet(ptr->fTCH3, bval);	
			/*�غϱ���ʱ��*/
			bval = tset + SET_TCHBS;
			FloatToSet(ptr->fTCHBS, bval);

			//��ѹ
			bval = tset + SET_3U0;
			FloatToSet(ptr->XDL_U0, bval);		

			//С������ѹͻ��
			bval = tset +   SET_U0TB;
			FloatToSet(ptr->XDL_U0_diff, bval);		

			//С��������ͻ��
			bval = tset + SET_I0TB;
			FloatToSet(ptr->XDL_I0_diff, bval);		

			//С��������ͻ��	
			bval = tset + SET_ITB;
			FloatToSet(ptr->XDL_I_diff, bval);		
			
			//С�����ӵ�ʱ��	
			bval = tset + SET_TDXJD;
			FloatToSet(ptr->XDL_Time, bval);		
			
			//С�����ӵ�ϵ��
			bval = tset + SET_XDLCOEF;
			FloatToSet(ptr->XDL_Coef, bval);		

			break;
		default:
			break;
	}
	
	
	return 0;
}
int WriteVoltageSet(int fd, BYTE*pSet)
{
	VPrdyxSet*ptr = ( VPrdyxSet*)pSet;
	TSETVAL *tset;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrSetPublic *pprRunSet;
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet1;
	DWORD yb;
	BYTE SLLineNo;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;


	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
	/*����ʧѹ��բ*/
	yb = pprRunSet->dYb;
	if(ptr->BSFunyb == 0)
		yb &= ~ PR_YB_SUDY;
	else
		yb |= PR_YB_SUDY;
	/*���翪�ع���ѹ��*/
	if(ptr->BLFunyb == 0)
		yb &= ~ PR_YB_SUSY;
	else
		yb |=  PR_YB_SUSY;	

	if (ptr->LoseVolYb == 0)
		yb &= ~ PR_YB_SLWY;
	else
		yb |=  PR_YB_SLWY;

	if (ptr->U0Yb == 0)
		yb &= ~ PR_YB_U0;
	else
		yb |=  PR_YB_U0;
			
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);

	yb = pprRunSet->dKG1;
	/*X��������ѹ��*/
	if (ptr->BXbsyb == 0) 
		yb &= ~ PR_CFG1_X_BS;
	else
		yb |=  PR_CFG1_X_BS;	
		
	/*Y��������ѹ��*/ 
	if (ptr->BYbsyb == 0)
		yb &= ~ PR_CFG1_Y_BS;
	else
		yb |=  PR_CFG1_Y_BS;	
	/*  �ϳɹ�������բ*/
	if (ptr->FZBS == 0)
		yb &= ~ PR_CFG1_FZ_BS;
	else
		yb |=  PR_CFG1_FZ_BS;	
	
		
	/*˲ѹ��������ѹ��*/
	if (ptr->BSybsyb == 0)
		yb &= ~ PR_CFG1_SY_BS;
	else
		yb |=  PR_CFG1_SY_BS;	
     yb &= ~(PR_CFG1_ZSY_XJDL|PR_CFG1_ZSY_DXJD|PR_CFG1_UI_TYPE);
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);
		
	/*SL���ߺ�*/
	SLLineNo = ptr->BSLLineNo;
	ReverseKSet(SET_PUB_SLFD, (BYTE*)tset, SLLineNo);
	/*Xʱ��*/
	bval = tset + SET_PUB_TX;
	FloatToSet(ptr->fXsx, bval);
	/*Yʱ��*/
	bval = tset + SET_PUB_TY;
	FloatToSet(ptr->fYsx, bval);
	/*Zʱ��*/
	bval = tset + SET_PUB_TWY;
	FloatToSet(ptr->fZsx, bval);

	bval = tset + SET_PUB_TSYCY;
	FloatToSet(ptr->fTcytime, bval);
	bval = tset + SET_PUB_UCY;
	FloatToSet(ptr->cydz, bval);
	//��ѹ��ֵ
	bval = tset + SET_PUB_U0;
	FloatToSet(ptr->U0Dz, bval);
	//��բ��������ʱ��
	bval = tset + SET_PUB_TFZ;
	FloatToSet(ptr->fZbstime, bval);

	yb = pprRunSet1->dKG2;
	
	if (ptr->byXDLJD == 0) 
		yb &= ~ PR_CFG2_XDL_YB;
	else
		yb |=  PR_CFG2_XDL_YB;	
	
	if (ptr->byXDLJD_Trip == 0) 
		yb &= ~ PR_CFG2_XDL_TRIP;
	else
		yb |=  PR_CFG2_XDL_TRIP;				
	
	pprRunSet1->dKG2 = yb;
	ReverseKSet(SET_KG2, (BYTE*)tset1, yb);

	//��ѹ
	bval = tset1 + SET_3U0;
	FloatToSet(ptr->XDL_U0, bval);		
	
	//С������ѹͻ��
	bval = tset1 +	SET_U0TB;
	FloatToSet(ptr->XDL_U0_diff, bval); 	
	
	//С��������ͻ��
	bval = tset1 + SET_I0TB;
	FloatToSet(ptr->XDL_I0_diff, bval); 	
	
	//С��������ͻ�� 
	bval = tset1 + SET_ITB;
	FloatToSet(ptr->XDL_I_diff, bval);		
	
	//С�����ӵ�ʱ�� 
	bval = tset1 + SET_TDXJD;
	FloatToSet(ptr->XDL_Time, bval);		
	
	//С�����ӵ�ϵ��
	bval = tset1 + SET_XDLCOEF;
	FloatToSet(ptr->XDL_Coef, bval);	

	return 0;
 }

int WriteCurrentSet(int fd, BYTE*pSet)
{
	VPrdljsxSet*ptr = ( VPrdljsxSet*)pSet;
	TSETVAL *tset;
	TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	TSETVAL *bval1;
	VPrSetPublic *pprRunSet;
	VPrRunSet *pprRunSet1;
	VPrRunInfo *pInfo;
	DWORD yb,yb1,DljsNUM,TZFS,FAReport,CHNum, PrITripYb;
	BYTE SLLineNo;
	pInfo = prRunInfo + fd;
	pprRunSet = prSetPublic[pInfo->set_no]+fd;
	pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
	/*��������ѹ��*/
 	yb = pprRunSet->dYb;
  	if (ptr->BDljsyb == 0)
   		yb &= ~PR_YB_I_DL;
   	else
		yb |= PR_YB_I_DL;
	pprRunSet->dYb = yb;		
   	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);
		
  yb = pprRunSet->dKG1;
	yb &= ~(PR_CFG1_ZSY_XJDL|PR_CFG1_ZSY_DXJD|PR_CFG1_UI_TYPE);
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);	
		
	SLLineNo = ptr->BSLLineNo;							
	ReverseKSet(SET_PUB_SLFD, (BYTE*)tset, SLLineNo);		
	/*������������*/
	DljsNUM = ptr->BDljsNUM;	
	ReverseKSet(SET_PUB_ICNT, (BYTE*)tset, DljsNUM);
	/*����������λʱ��*/
	bval = tset + SET_PUB_TICNT1;
	FloatToSet(ptr->fTdljsReset, bval);
	/*���������ۼ�ʱ��*/
	bval = tset + SET_PUB_TICNT2;
	FloatToSet(ptr->fTdljsTotal, bval);		
      /*һ�ι���ѹ��*/
	yb1 = pprRunSet1->dYb;
	if (ptr->OCIYb==0)
		yb1 &= ~PR_YB_I1;
	else
	 	yb1 |= PR_YB_I1;
	/*�غ�բѹ��*/
	if ((ptr->bCHZYB == 0))
		 yb1 &= ~PR_YB_CH ;
	else
	 	yb1 |= PR_YB_CH ;
	pprRunSet1->dYb = yb1;	
	ReverseKSet(SET_YB, (BYTE*)tset1, yb1);
	/*һ�ι�����ֵ*/
	bval1 = tset1 + SET_I1;
	FloatToSet(ptr->OCIDz, bval1);
	/*һ�ι���ʱ��*/
	bval1 = tset1 + SET_T1;
	FloatToSet(ptr->OCITime, bval1);
	/*��բ��ʽ*/	
	TZFS  = (pprRunSet1->dKG1&~PR_CFG1_TRIP_MODE) | ((ptr->bTZFS - 1) << KG_17) ;  // D17-D19 ��բ��ʽ
	pprRunSet1->dKG1 = TZFS;	
	ReverseKSet(SET_KG1, (BYTE*)tset1, TZFS);
	/*�����Ϸ�ʽ*/
	FAReport = (pprRunSet1->dKG1&~PR_CFG1_FAULT_REPORT) |( (ptr->bFAReport - 1) << (KG_20)); // D20-D21 �ϱ����Ϸ�ʽ
	pprRunSet1->dKG1 = FAReport;
	ReverseKSet(SET_KG1, (BYTE*)tset1, FAReport);
	/*�غ�բ����*/
	CHNum = ptr->bCHNum; 	
	bval1 = tset1 + SET_CHNUM;			
	FloatToSet(CHNum, bval1);	

	/*һ���غ�բʱ��*/
	bval1 = tset1 + SET_TCH1;
	FloatToSet(ptr->fTCH1, bval1);

	/* �����غ�ʱ��*/
	bval1 = tset1 + SET_TCH2;
	FloatToSet(ptr->fTCH2, bval1);

	/*�����غ�ʱ��*/
	bval1 = tset1 + SET_TCH3;
	FloatToSet(ptr->fTCH3, bval1);
	/*�غϱ���ʱ��*/
	bval1 = tset1 + SET_TCHBS;
	FloatToSet(ptr->fTCHBS, bval1);

	/*һ�γ���ѹ��*/	
	PrITripYb = (pprRunSet1->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrYb) << (KG_27)); //��������Ͷ�� D27
	pprRunSet1->dKG1 = PrITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset1, PrITripYb);

  return 0;		
}
int WriteZsyVoltageSet(int fd, BYTE*pSet)
  {
    VPrzsyxSet*ptr = ( VPrzsyxSet*)pSet;
    TSETVAL *tset;
    TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
    TSETVAL *bval;
    TSETVAL *bval1;
    VPrSetPublic *pprRunSet;
    VPrRunSet *pprRunSet1;
    VPrRunInfo *pInfo;      
    DWORD yb,yb1,PrITripYb;
    BYTE SLLineNo;
    pInfo = prRunInfo + fd;
    pprRunSet = prSetPublic[pInfo->set_no]+fd;
    pprRunSet1 = prRunSet[pInfo->set_no]+fd;

	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
    /*����ʧѹ��բ*/
	yb = pprRunSet->dYb;
	if(ptr->BSFunyb == 0)
	  yb &= ~ PR_YB_SUDY;
	else
	  yb |= PR_YB_SUDY;
	/*���翪�ع���ѹ��*/
	if(ptr->BLFunyb == 0)
	  yb &= ~ PR_YB_SUSY;
	else
	  yb |=  PR_YB_SUSY;  

	if (ptr->LoseVolYb == 0)
	  yb &= ~ PR_YB_SLWY;
	else
	  yb |=  PR_YB_SLWY;
	
	if (ptr->U0Yb == 0)
		yb &= ~ PR_YB_U0;
	else
		yb |=  PR_YB_U0;
	    
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);

	yb = pprRunSet->dKG1;
	/*X��������ѹ��*/
	if (ptr->BXbsyb == 0) 
	  yb &= ~ PR_CFG1_X_BS;
	else
	  yb |=  PR_CFG1_X_BS;  
	  
	/*Y��������ѹ��*/ 
	if (ptr->BYbsyb == 0)
	  yb &= ~ PR_CFG1_Y_BS;
	else
	  yb |=  PR_CFG1_Y_BS;  

	/*  �ϳɹ�������բ*/
	if (ptr->FZBS == 0)
		yb &= ~ PR_CFG1_FZ_BS;
	else
		yb |=  PR_CFG1_FZ_BS;
	  
	/*˲ѹ��������ѹ��*/
	if (ptr->BSybsyb == 0)
	  yb &= ~ PR_CFG1_SY_BS;
	else
	  yb |=  PR_CFG1_SY_BS; 

	if(ptr->byxjdl ==0)
		yb &= ~ PR_CFG1_ZSY_XJDL;
	else
		yb |=  PR_CFG1_ZSY_XJDL;

	if(ptr->bYbZsyjd)
		yb |= PR_CFG1_ZSY_DXJD;
	else
		yb &= ~PR_CFG1_ZSY_DXJD;

	if(ptr->bYbDu)
		yb |= PR_CFG1_DU_BS;
	else
		yb &= ~PR_CFG1_DU_BS;

	if(ptr->bYbU0Trip)
		yb |= PR_CFG1_U0_TRIP;
	else
		yb &= ~PR_CFG1_U0_TRIP;
	
	yb &= ~PR_CFG1_UI_TYPE;
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);
	  
	/*SL���ߺ�*/
	SLLineNo = ptr->BSLLineNo;
	ReverseKSet(SET_PUB_SLFD, (BYTE*)tset, SLLineNo);
	/*Xʱ��*/
	bval = tset + SET_PUB_TX;
	FloatToSet(ptr->fXsx, bval);
	/*Yʱ��*/
	bval = tset + SET_PUB_TY;
	FloatToSet(ptr->fYsx, bval);
	/*Zʱ��*/
	bval = tset + SET_PUB_TWY;
	FloatToSet(ptr->fZsx, bval);
	/*Sʱ��*/
	bval = tset + SET_PUB_TS;
	FloatToSet(ptr->fSsx, bval);
	/*Cʱ��*/
	bval = tset + SET_PUB_TC;
	FloatToSet(ptr->fCsx, bval);
	bval = tset + SET_PUB_TSYCY;
	FloatToSet(ptr->fTcytime, bval);
	bval = tset + SET_PUB_UCY;
	FloatToSet(ptr->cydz, bval);
	//��ѹ��ֵ
	bval = tset + SET_PUB_U0;
	FloatToSet(ptr->U0Dz, bval);
	//��ѹʱ��
	bval = tset + SET_PUB_TU0;
	FloatToSet(ptr->fTU0, bval);
	//��բ��������ʱ��
	bval = tset + SET_PUB_TFZ;
	FloatToSet(ptr->fZbstime, bval);

	/*һ�ι���ѹ��*/
	yb1 = pprRunSet1->dYb;
	if (ptr->byIb==0)
		yb1 &= ~PR_YB_I1;
	else
		yb1 |= PR_YB_I1;
	pprRunSet1->dYb = yb1;	
	ReverseKSet(SET_YB, (BYTE*)tset1, yb1);

	yb = pprRunSet1->dKG2;

	if (ptr->byXDLJD == 0) 
		yb &= ~ PR_CFG2_XDL_YB;
	else
		yb |=  PR_CFG2_XDL_YB;	
	
	if (ptr->byXDLJD_Trip == 0) 
		yb &= ~ PR_CFG2_XDL_TRIP;
	else
		yb |=  PR_CFG2_XDL_TRIP;				
	
	pprRunSet1->dKG2 = yb;
	ReverseKSet(SET_KG2, (BYTE*)tset1, yb);

	
	/*һ�ι�����ֵ*/
	bval1 = tset1 + SET_I1;
	FloatToSet(ptr->fI1GLDZ, bval1);
	/*һ�ι���ʱ��*/
	bval1 = tset1 + SET_T1;
	FloatToSet(ptr->fT1GL, bval1);

	//��ѹ
	bval = tset1 + SET_3U0;
	FloatToSet(ptr->XDL_U0, bval);		
	
	//С������ѹͻ��
	bval = tset1 +	SET_U0TB;
	FloatToSet(ptr->XDL_U0_diff, bval); 	
	
	//С��������ͻ��
	bval = tset1 + SET_I0TB;
	FloatToSet(ptr->XDL_I0_diff, bval); 	
	
	//С��������ͻ�� 
	bval = tset1 + SET_ITB;
	FloatToSet(ptr->XDL_I_diff, bval);		
	
	//С�����ӵ�ʱ�� 
	bval = tset1 + SET_TDXJD;
	FloatToSet(ptr->XDL_Time, bval);		
	
	//С�����ӵ�ϵ��
	bval = tset1 + SET_XDLCOEF;
	FloatToSet(ptr->XDL_Coef, bval);	

	/*һ�γ���ѹ��*/	
	PrITripYb = (pprRunSet1->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrTripYb) << (KG_27)); //��������Ͷ�� D27
	pprRunSet1->dKG1 = PrITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset1, PrITripYb);  

    return 0;
   }
int WriteDYDLVoltageSet(int fd, BYTE*pSet)
{
    VPrdydlSet*ptr = ( VPrdydlSet*)pSet;
    TSETVAL *tset;
    TSETVAL *tset1 = (TSETVAL*)(prSet[fd+1]);
    TSETVAL *bval;
    TSETVAL *bval1;
    VPrSetPublic *pprRunSet;
    VPrRunSet *pprRunSet1;
    VPrRunInfo *pInfo;
    DWORD yb,yb1,PrITripYb;
    BYTE SLLineNo;
	
	tset = (TSETVAL*)(prSet[prRunPublic[fd].prSetnum]);
	
    pInfo = prRunInfo + fd;
    pprRunSet = prSetPublic[pInfo->set_no]+fd;
    pprRunSet1 = prRunSet[pInfo->set_no]+fd;
	yb = pprRunSet->dYb;
	if(ptr->BSFunyb == 0)
	  yb &= ~ PR_YB_SUDY;
	else
	  yb |= PR_YB_SUDY;
	if(ptr->BLFunyb == 0)
	  yb &= ~ PR_YB_SUSY;
	else
	  yb |=  PR_YB_SUSY;  

	if (ptr->LoseVolYb == 0)
	  yb &= ~ PR_YB_SLWY;
	else
	  yb |=  PR_YB_SLWY;

	if (ptr->U0Yb == 0)
		yb &= ~ PR_YB_U0;
	else
		yb |=  PR_YB_U0;
	pprRunSet->dYb = yb;
	ReverseKSet(SET_PUB_YB, (BYTE*)tset, yb);

	yb = pprRunSet->dKG1;
	if (ptr->BXbsyb == 0) 
	  yb &= ~ PR_CFG1_X_BS;
	else
	  yb |=  PR_CFG1_X_BS;  
	if (ptr->BYbsyb == 0)
	  yb &= ~ PR_CFG1_Y_BS;
	else
	  yb |=  PR_CFG1_Y_BS;  
	if (ptr->BSybsyb == 0)
	  yb &= ~ PR_CFG1_SY_BS;
	else
	  yb |=  PR_CFG1_SY_BS; 
	if (ptr->BDyDlYb == 0)
	  yb &= ~ PR_CFG1_UI_TYPE;
	else
	  yb |=  PR_CFG1_UI_TYPE; 
	if (ptr->FZBS == 0)
		yb &= ~ PR_CFG1_FZ_BS;
	else
		yb |=  PR_CFG1_FZ_BS;

	if (ptr->bYbDu)
		yb |= PR_CFG1_DU_BS;
	else
		yb &= ~PR_CFG1_DU_BS;

	if (ptr->bYbU0Trip)
		yb |= PR_CFG1_U0_TRIP;
	else
		yb &= ~PR_CFG1_U0_TRIP;
	
	 yb &= ~(PR_CFG1_ZSY_XJDL|PR_CFG1_ZSY_DXJD);
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_PUB_KG1, (BYTE*)tset, yb);
	SLLineNo = ptr->BSLLineNo;
	ReverseKSet(SET_PUB_SLFD, (BYTE*)tset, SLLineNo);
	bval = tset + SET_PUB_TX;
	FloatToSet(ptr->fXsx, bval);
	bval = tset + SET_PUB_TY;
	FloatToSet(ptr->fYsx, bval);
	bval = tset + SET_PUB_TWY;
	FloatToSet(ptr->fZsx, bval);
	bval = tset + SET_PUB_TSYCY;
	FloatToSet(ptr->fTcytime, bval);
	bval = tset + SET_PUB_UCY;
	FloatToSet(ptr->cydz, bval);
	//��ѹ��ֵ
	bval = tset + SET_PUB_U0;
	FloatToSet(ptr->U0Dz, bval);

	//��ѹ��ֵ
	bval = tset + SET_PUB_TU0;
	FloatToSet(ptr->fTU0, bval);

	//��բ��������ʱ��
	bval = tset + SET_PUB_TFZ;
	FloatToSet(ptr->fZbstime, bval);

	
	yb1 = pprRunSet1->dYb;
	if (ptr->byIb==0)
		yb1 &= ~PR_YB_I1;
	else
		yb1 |= PR_YB_I1;
	pprRunSet1->dYb = yb1;	
	ReverseKSet(SET_YB, (BYTE*)tset1, yb1);

	yb = pprRunSet1->dKG2;

	if (ptr->byXDLJD == 0) 
		yb &= ~ PR_CFG2_XDL_YB;
	else
		yb |=  PR_CFG2_XDL_YB;	
	
	if (ptr->byXDLJD_Trip == 0) 
		yb &= ~ PR_CFG2_XDL_TRIP;
	else
		yb |=  PR_CFG2_XDL_TRIP;				
	
	pprRunSet1->dKG2 = yb;
	ReverseKSet(SET_KG2, (BYTE*)tset1, yb);

	
	bval1 = tset1 + SET_I1;
	FloatToSet(ptr->fI1GLDZ, bval1);
	bval1 = tset1 + SET_T1;
	FloatToSet(ptr->fT1GL, bval1);
	PrITripYb = (pprRunSet1->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrTripYb) << (KG_27)); //��������Ͷ�� D27
	pprRunSet1->dKG1 = PrITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset1, PrITripYb);  

		//��ѹ
	bval = tset1 + SET_3U0;
	FloatToSet(ptr->XDL_U0, bval);		
	
	//С������ѹͻ��
	bval = tset1 +	SET_U0TB;
	FloatToSet(ptr->XDL_U0_diff, bval); 	
	
	//С��������ͻ��
	bval = tset1 + SET_I0TB;
	FloatToSet(ptr->XDL_I0_diff, bval); 	
	
	//С��������ͻ�� 
	bval = tset1 + SET_ITB;
	FloatToSet(ptr->XDL_I_diff, bval);		
	
	//С�����ӵ�ʱ�� 
	bval = tset1 + SET_TDXJD;
	FloatToSet(ptr->XDL_Time, bval);		
	
	//С�����ӵ�ϵ��
	bval = tset1 + SET_XDLCOEF;
	FloatToSet(ptr->XDL_Coef, bval);	
    return 0;
   }
int WriteI0ValueSet(int fd, BYTE*pSet)
{
	
	VPrI0VlaueSet*ptr = (VPrI0VlaueSet*)pSet;
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunSet *pprRunSet;
	VPrRunInfo *pInfo;
		
	DWORD yb,TZFS,FAReport,CHNum,PrITripYb,PrIITripYb,PrI0TripYb;

	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;

	/*һ�ι���ѹ��*/
	yb = pprRunSet->dYb;
	if (ptr->byIb==0)
		yb &= ~PR_YB_I1;
	else
	 	yb |= PR_YB_I1;
	/*���ι���ѹ��   */
	if (ptr->byIIb==0)
		yb &= ~PR_YB_I2;
	else
		yb |= PR_YB_I2;
	/*�غ�բѹ��*/
	if ((ptr->bCHZYB == 0))
		yb &= ~PR_YB_CH ;
	else
		yb |= PR_YB_CH ;
	//����һ��ѹ��
	if ((ptr->bI0Yb == 0))
		yb &= ~PR_YB_I01 ;
	else
		yb |= PR_YB_I01 ;
	pprRunSet->dYb = yb;	
	ReverseKSet(SET_YB, (BYTE*)tset, yb);
	/*һ�ι�����ֵ*/
	bval = tset + SET_I1;
	FloatToSet(ptr->fI1GLDZ, bval);
	/*һ�ι���ʱ��*/
	bval = tset + SET_T1;
	FloatToSet(ptr->fT1GL, bval);
	/*���ι�����ֵ*/
	bval = tset + SET_I2;
	FloatToSet(ptr->fI2GLDZ, bval);
	/*���ι���ʱ��*/
	bval = tset + SET_T2;
	FloatToSet(ptr->fT2GL, bval);

	//����һ�ζ�ֵ
	bval = tset + SET_3I01;
	FloatToSet(ptr->fI0Dz, bval);
	//����һ��ʱ��
	bval = tset + SET_TI01;
	FloatToSet(ptr->fTI0, bval);
	
	/*��բ��ʽ*/	
	TZFS  = (pprRunSet->dKG1&~PR_CFG1_TRIP_MODE) | ((ptr->bTZFS - 1) << KG_17) ;  // D17-D19 ��բ��ʽ
	pprRunSet->dKG1 = TZFS;	
	ReverseKSet(SET_KG1, (BYTE*)tset, TZFS);
	/*�����Ϸ�ʽ*/
	FAReport = (pprRunSet->dKG1&~PR_CFG1_FAULT_REPORT) |( (ptr->bFAReport - 1) << (KG_20)); // D20-D21 �ϱ����Ϸ�ʽ
	pprRunSet->dKG1 = FAReport;
	ReverseKSet(SET_KG1, (BYTE*)tset, FAReport);

	/*һ�γ���ѹ��*/	
	PrITripYb = (pprRunSet->dKG1 & ~PR_CFG1_I1_TRIP) |( (ptr->byIPrTripYb) << (KG_27)); //��������Ͷ�� D27
	pprRunSet->dKG1 = PrITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset, PrITripYb);

	/*���γ���ѹ��*/	
	PrIITripYb = (pprRunSet->dKG1 & ~PR_CFG1_I2_TRIP) |( (ptr->byIIPrTripYb) << (KG_28)); //��������Ͷ�� D28
	pprRunSet->dKG1 = PrIITripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset, PrIITripYb);

	
	//����һ�γ���ѹ��
	PrI0TripYb = (pprRunSet->dKG1 & ~PR_CFG1_I0_TRIP) |( (ptr->byI0PrTripYb) << (KG_29)); //��������Ͷ�� D28
	pprRunSet->dKG1 = PrI0TripYb;
	ReverseKSet(SET_KG1, (BYTE*)tset, PrI0TripYb);

	yb = pprRunSet->dKG1;
	if (ptr->byGLFx == 0) 
		yb &= ~ PR_CFG1_I_DIR;
	else
		yb |=  PR_CFG1_I_DIR;	
	pprRunSet->dKG1 = yb;
	ReverseKSet(SET_KG1, (BYTE*)tset, yb);

	yb = pprRunSet->dKG2;
	if (ptr->byGLYx == 0) 
		yb &= ~ PR_CFG2_IYX_DIR;
	else
		yb |=  PR_CFG2_IYX_DIR;	

	if (ptr->byXDLJD == 0) 
		yb &= ~ PR_CFG2_XDL_YB;
	else
		yb |=  PR_CFG2_XDL_YB;	
	
	if (ptr->byXDLJD_Trip == 0) 
		yb &= ~ PR_CFG2_XDL_TRIP;
	else
		yb |=  PR_CFG2_XDL_TRIP;		
	
	pprRunSet->dKG2 = yb;
	ReverseKSet(SET_KG2, (BYTE*)tset, yb);


	/*�غ�բ����*/
	CHNum = ptr->bCHNum; 	
	bval = tset + SET_CHNUM;	
	FloatToSet(CHNum, bval);			

	/*һ���غ�բʱ��*/
	bval = tset + SET_TCH1;
	FloatToSet(ptr->fTCH1, bval);
	/*�����غ�բʱ��*/
	bval = tset + SET_TCH2;
	FloatToSet(ptr->fTCH2, bval);
	/*�����غ�բʱ��*/
	bval = tset + SET_TCH3;
	FloatToSet(ptr->fTCH3, bval);	
	/*�غϱ���ʱ��*/
	bval = tset + SET_TCHBS;
	FloatToSet(ptr->fTCHBS, bval);

	bval = tset + SET_IANG1;
	FloatToSet(ptr->AngleLow, bval);

	bval = tset + SET_IANG2;
	FloatToSet(ptr->AngleUpp, bval);

	//��ѹ
	bval = tset + SET_3U0;
	FloatToSet(ptr->XDL_U0, bval);		
	
	//С������ѹͻ��
	bval = tset +	SET_U0TB;
	FloatToSet(ptr->XDL_U0_diff, bval); 	
	
	//С��������ͻ��
	bval = tset + SET_I0TB;
	FloatToSet(ptr->XDL_I0_diff, bval); 	
	
	//С��������ͻ�� 
	bval = tset + SET_ITB;
	FloatToSet(ptr->XDL_I_diff, bval);		
	
	//С�����ӵ�ʱ�� 
	bval = tset + SET_TDXJD;
	FloatToSet(ptr->XDL_Time, bval);		
	
	//С�����ӵ�ϵ��
	bval = tset + SET_XDLCOEF;
	FloatToSet(ptr->XDL_Coef, bval);

	
	return 0;
}

int FaSimWriteSet(int fd, int flag, BYTE *pSet)
{

   	VPrRunInfo *pInfo = prRunInfo+fd;
	int no;
	
	if (g_prInit == 0)
		return ERROR;

	smMTake(prSetSem);

	if(pInfo->set_no == 0)
		no = 1;
	else 
        no = 0;

	memcpy(prRunSet[no], prRunSet[pInfo->set_no], sizeof(VPrRunSet));
	
	switch (flag)
	{
		case PR_DLQ:
		case PR_FHKG:
		case PR_GLBGZ:
		case PR_DLQCH:
		case PR_CKTZ:
		case PR_CKII:
		case PR_BDZCH:
		case PR_DLJSCH:
			WriteConsValueSet(fd,flag,pSet);
			break;
		case PR_DYZ:
			WriteVoltageSet(fd, pSet);
			break;
		case PR_DLJSX:
			WriteCurrentSet(fd, pSet);
			break;
		case PR_ZSYX:
			WriteZsyVoltageSet(fd,pSet);
			break;
		case PR_DYDLX:
			WriteDYDLVoltageSet(fd,pSet);
			break;
		case PR_CKI0TZ:
			WriteI0ValueSet(fd,pSet);
			break;
		default:
			break;
	}

	smMGive(prSetSem);

	caSetBuf = prSet[prRunPublic[fd].prSetnum];
	
	prPubSetConv(fd);

	if (prRunPublic[fd].set_no == 0)
		prRunPublic[fd].set_no = 1;
	else
		prRunPublic[fd].set_no = 0;
	
	caSetBuf = prSet[fd+1];	
	prFdSetConv(fd);

	prFdSetNoSwitch(fd);
    return OK;
}
#endif

int ReadSetLen(int fd, BYTE *pBuf, VPrSetHead* pHead)
{
    struct VFileHead *pFile = (struct VFileHead*)prTableBuf;
    BYTE *pTemp;

	if (g_prInit == 0)
		return 0;
    if (fd < 0) 
	{
	    pHead->type |= PR_SET_ERR;
		memcpy(pBuf, pHead, sizeof(VPrSetHead));
	    return sizeof(VPrSetHead);
    }

	if (pHead->type & PR_SET_DESCRIBE)
	{
	    if (pHead->offset == 0)
	    {
	       if (fd == 0)
		   	  ReadParaFile("ProSetPubTable.cfg", prTableBuf, MAX_TABLE_FILE_SIZE);
		   else
	          ReadParaFile("ProSetTable.cfg", prTableBuf, MAX_TABLE_FILE_SIZE);
		   pHead->total = pFile->nLength - sizeof(struct VFileHead);
	    }	
		pTemp = prTableBuf + sizeof(struct VFileHead);
	}
	else if (pHead->type & PR_SET_PARA)
	{
	    if(pHead->offset == 0)
		{
		    ReadSet(fd, prTableBuf);
			if (fd == 0)
				pHead->total = sizeof(TTABLETYPE) + SET_PUB_NUMBER*sizeof(TSETVAL);
			else
			    pHead->total = sizeof(TTABLETYPE) + SET_NUMBER*sizeof(TSETVAL);
	    }
		pTemp = prTableBuf;

	}
	if((pHead->offset + pHead->len) > pHead->total)
	{
		pHead->len = pHead->total - pHead->offset;
	    pHead->type |= PR_SET_END;
	}
	memcpy(pBuf, pHead, sizeof(VPrSetHead));
	memcpy(pBuf+sizeof(VPrSetHead), pTemp+pHead->offset, pHead->len);

	return (pHead->len+sizeof(VPrSetHead));
}

int WriteSetLen(int fd, BYTE *pBuf, VPrSetHead* pHead)
{
    BYTE *pTemp;
	int len;
	if (g_prInit == 0)
		return 0;

	if (fd < 0) 
	{
	    pHead->type |= PR_SET_ERR;
	    return ERROR;
    }

    pTemp = prTableBuf;
	if (fd == 0)
		len = sizeof(TTABLETYPE) + SET_PUB_NUMBER*sizeof(TSETVAL);
	else
		len = sizeof(TTABLETYPE) + SET_NUMBER*sizeof(TSETVAL);

	if (pHead->total > len)
	{
		pHead->type |= PR_SET_ERR;
		return ERROR;
	}
	if (pHead->type & PR_SET_PARA)
	{
	   memcpy(pTemp+pHead->offset, pBuf, pHead->len);
	   if(pHead->type & PR_SET_END)
	   	  WriteSet(fd, pTemp);
	}
	return pHead->len;
}

#ifdef INCLUDE_EXT_MMI
//wdg TI1==0 default
int WriteMMIPrSet_1(int fd, BYTE *pSet)
{
  
	VMMIPrSet_1 *ptr =(VMMIPrSet_1 *)pSet;
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet; 
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD yb,kg1;
	DWORD len;
	BYTE *ptemp;

	if(g_prInit == 0)
		return -1;

    pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
	
	smMTake(prSetSem);
	
	bval = tset + SET_T2;
	FloatToSet(ptr->T2Set, bval);

	bval = tset + SET_TI02;
	FloatToSet(ptr->T0Set, bval);
	
	bval = tset + SET_3U0;
	FloatToSet(ptr->U0Set, bval);


	yb = pprRunSet->dYb;
	if ((ptr->I2Set < 10e-6)&&(ptr->I2Set > -10e-6))
		yb &= ~PR_YB_I2;
	else
	{
		yb |= PR_YB_I2;
		bval = tset + SET_I2;
		FloatToSet(ptr->I2Set, bval);
	}
	if ((ptr->U0Set < 10e-6)&&(ptr->U0Set > -10e-6))
		yb &= ~PR_YB_I02;
	else
	{
		yb |= PR_YB_I02;
		bval = tset +SET_3U0 ;
		FloatToSet(ptr->U0Set, bval);
	}
	
	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6))
		yb &= ~PR_YB_I02;
	else
	{
		yb |= PR_YB_I02;
		bval = tset + SET_3I02;
		FloatToSet(ptr->I0Set, bval);
	}
	ReverseKSet(SET_YB, (BYTE*)tset, yb);

	kg1 = pprRunSet->dKG1;	

	if ((ptr->I2Set < 10e-6)&&(ptr->I2Set > -10e-6))

	{
		kg1 &= ~(PR_CFG1_I2_TRIP);
	}

	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6)&&(ptr->U0Set < 10e-6)&&(ptr->U0Set > -10e-6))
	{	
		kg1 &= ~(PR_CFG1_I0_TRIP);
	}

	kg1 &= ~(PR_CFG1_I0_U|PR_CFG1_CD_MODE);
	ReverseKSet(SET_KG1, (BYTE*)tset, kg1);


	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)tset, SET_NUMBER*sizeof(TSETVAL));

	caSetBuf = prSet[fd+1];
	prFdSetConv(fd);
	prFdSetNoSwitch(fd);
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}		
	smMGive(prSetSem);
	
	return 0;


}
int WriteMMIPrSet(int fd, BYTE *pSet)
{
    VMMIPrSet *ptr = (VMMIPrSet *)pSet;
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet; 
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD yb,kg1;
	DWORD len;
	BYTE *ptemp;

	if(g_prInit == 0)
		return -1;

    pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
	
	smMTake(prSetSem);
	bval = tset + SET_T1;
#if (TYPE_USER == USER_BEIJING)
	FloatToSet(ptr->T1Set, bval);
#else
	FloatToSet(0, bval);
#endif
	bval = tset + SET_T2;
	FloatToSet(ptr->T2Set, bval);

#if (TYPE_USER == USER_BEIJING)
	bval = tset + SET_TI02;
	FloatToSet(ptr->T0Set, bval);
#else
	bval = tset + SET_TI01;
	FloatToSet(ptr->T0Set, bval);
#endif	

	yb = pprRunSet->dYb;
	if ((ptr->I1Set < 10e-6)&&(ptr->I1Set > -10e-6))
        yb &= ~PR_YB_I1;
	else
	{
		yb |= PR_YB_I1;
		bval = tset + SET_I1;
		FloatToSet(ptr->I1Set, bval);
	}
	if ((ptr->I2Set < 10e-6)&&(ptr->I2Set > -10e-6))
		yb &= ~PR_YB_I2;
	else
	{
		yb |= PR_YB_I2;
		bval = tset + SET_I2;
		FloatToSet(ptr->I2Set, bval);
	}
#if (TYPE_USER != USER_BEIJING)
	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6))
		yb &= ~PR_YB_I01;
	else
	{
		yb |= PR_YB_I01;
		bval = tset + SET_3I01;
	FloatToSet(ptr->I0Set, bval);
	}
	yb &= ~(PR_YB_I02|PR_YB_I03);
	ReverseKSet(SET_YB, (BYTE*)tset, yb);
#else
	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6))
		yb &= ~PR_YB_I02;
	else
	{
		yb |= PR_YB_I02;
		bval = tset + SET_3I02;
	FloatToSet(ptr->I0Set, bval);
	}
	ReverseKSet(SET_YB, (BYTE*)tset, yb);
#endif

	kg1 = pprRunSet->dKG1;

#if 0
#if (TYPE_USER != USER_BEIJING)
	if (ptr->I0Gj)
	{
		kg1 &= ~PR_CFG1_I0_TRIP;
	}
	else
	{
		kg1 |= PR_CFG1_I0_TRIP;
	}
#endif
	if ((ptr->I1Set < 10e-6)&&(ptr->I1Set > -10e-6))
        kg1 &= ~(PR_CFG1_I1_TRIP );

	if ((ptr->I2Set < 10e-6)&&(ptr->I2Set > -10e-6))
		kg1 &= ~(PR_CFG1_I2_TRIP);

	if ((ptr->I0Set < 10e-6)&&(ptr->I0Set > -10e-6))
		kg1 &= ~(PR_CFG1_I0_TRIP);
#endif

#ifndef _YIERCI_RH_
#ifdef _TEST_VER_
      if((ptr->I1Set > 49) && (ptr->I1Set < 50))
        kg1 |= (PR_CFG1_I1_TRIP);
      if((ptr->I2Set > 49) && (ptr->I2Set < 50))
        kg1 |= (PR_CFG1_I2_TRIP);
#endif
#endif

	kg1 &= ~(PR_CFG1_I0_U|PR_CFG1_CD_MODE);
	ReverseKSet(SET_KG1, (BYTE*)tset, kg1);


	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)tset, SET_NUMBER*sizeof(TSETVAL));

	caSetBuf = prSet[fd+1];
	prFdSetConv(fd);
	prFdSetNoSwitch(fd);
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}		
	smMGive(prSetSem);
	
	return 0;

}
#if (TYPE_USER == USER_BEIJING)
int WriteMMIU0Set(int fd,float u0set)
{
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	TSETVAL *bval;
	VPrRunInfo *pInfo;
	VPrSetPublic* pprPubRunSet;
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD yb;
	DWORD len;
	BYTE *ptemp;
	float u0setold;

	if(g_prInit == 0)
	return -1;

	pInfo = prRunInfo + fd;
	pprPubRunSet = prSetPublic[prRunPublic->set_no];
	smMTake(prSetSem);
	
	bval = tset + SET_3U0;
	SetToFloat(bval,&u0setold);
	if(((u0set > 10e-6)||(u0set < -10e-6)))
		FloatToSet(u0set, bval);
	
	if(u0setold == u0set)
	{
		smMGive(prSetSem);
		return 0;
	}
	
	yb = pprPubRunSet->dYb;
	if ((u0set < 10e-6)&&(u0set > -10e-6))
		yb &= ~PR_YB_U0;
	else
		yb |= PR_YB_U0;
	tset = (TSETVAL*)(prSet[0]);
	ReverseKSet(SET_YB, (BYTE*)tset, yb);
	bval = tset + SET_PUB_U0;
	if((u0set > 10e-6)||(u0set < -10e-6))
		FloatToSet(u0set, bval);
	
	
	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)prSet[fd+1], SET_NUMBER*sizeof(TSETVAL));

	caSetBuf = prSet[fd+1];
	prFdSetConv(fd);
	prFdSetNoSwitch(fd);
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}

	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);
	len = sizeof(TTABLETYPE);
	memcpy(wrSet,&tSetType,len);
	memcpy(wrSet+len,prSet[0],SET_NUMBER*sizeof(TSETVAL));
	if (WriteParaFile("fdPubSet.cfg", (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}
	caSetBuf = prSet[0];
	if(prPubSetConv() != SET_OK) return ERROR;
	if(prRunPublic->set_no == 0)
		prRunPublic->set_no =1;
	else
		prRunPublic->set_no = 0;
	
	smMGive(prSetSem);
	
	return 0;
	
}
#endif	
#endif
#ifdef INCLUDE_FA

int prSwitchFaMod(int pr, int fd, int wdg_flag)
{
    int i;
	static BOOL change[32] = {1};//��ֹһֱ��д���硣
	DWORD dread;
	DWORD dreadagain;
	VPrRunInfo *pInfo;
	VPrRunSet *pprSet;
    VPrSetPublic* pprRunSet;

	if(fd > fdNum)
		return ERROR;
    if(fd > 31)
		return ERROR;

    pprRunSet = prSetPublic[prRunPublic[fd].set_no]+fd;

	pInfo = prRunInfo + fd;
	pprSet = prRunSet[pInfo->set_no]+fd;

#if(TYPE_USER == USER_GUIYANG)
#if(DEV_SP == DEV_SP_WDG)
	//if(pprPubRunSet->bFaSelectMode == PR_FA_CENTRALIZED)  //����ʽ���л�
	if(GetExtMmiYx(XINEXTMMI_MS))
	{
#else
	if(GetExtMmiYx(EXTMMI_AUTO))  //����ʽ���л�
	{
#endif
		change[fd] = true;
		return 0;
	}
#endif

	if (wdg_flag)//�͵�
	{
		if(pr)
			pprSet->bCHNum = 2;
		if(0 == pprRunSet->dYb)
		{	
			if(true == change[fd])
			{
				if(0 == fd)
			    	extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&dread,sizeof(DWORD));
				else
					extNvRamGet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&dread,sizeof(DWORD));
				for(i=0; i<50; i++);
				
				if(0 == fd)
			    	extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&dreadagain,sizeof(DWORD));
				else
					extNvRamGet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&dreadagain,sizeof(DWORD));
				
			    if(dread == dreadagain)
			   	{
					change[fd] = false;
					pprRunSet->dYb = dread;
					   		  
				}
			}
		}
	}
	else //������
	{
		change[fd] = true;
		
		if(pr)
			pprSet->bCHNum = 1;
		
		if(pprRunSet->dYb)
		{
			if(0 == fd)
				extNvRamSet(NVRAM_DEV_PUBYB,(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
			else
				extNvRamSet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&pprRunSet->dYb,sizeof(DWORD));
			for(i=0; i<50; i++);

			if(0 == fd)
				extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&dread,sizeof(DWORD));
			else
				extNvRamGet(NVRAm_DEV_PUBYB_MULTI+4*(fd-1),(BYTE*)&dread,sizeof(DWORD));
			
			if(pprRunSet->dYb == dread)//�����쳣�ȴ���һ����������д��
			{
				pprRunSet->dYb = 0;	
			}
		}
	}

	return 0;

}

#endif
#endif

#ifdef _TEST_VER_
#if(DEV_SP == DEV_SP_DTU)
int BOMAPrset(int fd,WORD on)
{
	TSETVAL *tset = (TSETVAL*)(prSet[fd+1]);
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet; 
	BYTE *wrSet;
	char fname[MAXFILENAME];
	struct VFileHead *head;
	DWORD yb,kg1;
	DWORD len;
	BYTE *ptemp;
	BOOL  bWrite;
	
		
	 pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;

	smMTake(prSetSem);
	
	kg1 = pprRunSet->dKG1;
	yb = pprRunSet->dYb;

	bWrite = 0;
	if (on == 0)
	{
		if(kg1 & PR_CFG1_I1_TRIP)
		{
       		 kg1 &= ~PR_CFG1_I1_TRIP;
			 bWrite = 1;
		}
		/*if(yb & PR_YB_I1)
		{
			yb &= ~(PR_YB_I1);
			bWrite = 1;
		}*/
	}
	else
	{
		if((kg1 & PR_CFG1_I1_TRIP) == 0)
		{
			kg1 |= PR_CFG1_I1_TRIP;
			bWrite = 1;
		}
		/*if((yb & PR_YB_I1) == 0)
		{
			yb |= PR_YB_I1;
			bWrite = 1;
		}*/
	}

	if (on == 0)
	{
		if(kg1 & PR_CFG1_I2_TRIP)
		{
			kg1 &= ~PR_CFG1_I2_TRIP;
			bWrite = 1;
		}
		/*if(yb & PR_YB_I2)
		{
			yb &= ~(PR_YB_I2);
			bWrite = 1;
		}*/
	}
	else
	{
		if((kg1 & PR_CFG1_I2_TRIP) == 0)
		{
			kg1 |= PR_CFG1_I2_TRIP;
			bWrite = 1;
		}
		/*if((yb & PR_YB_I2) == 0)
		{
			yb |= PR_YB_I2;
			bWrite = 1;
		}*/
	}
	if(bWrite == 0)
	{
		smMGive(prSetSem);
		return 0;
	}
	
	ReverseKSet(SET_KG1, (BYTE*)tset, kg1);
	ReverseKSet(SET_YB, (BYTE*)tset, yb);

	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);

	len = sizeof(TTABLETYPE);
	ptemp = wrSet + len;
	memcpy(wrSet, &tSetType, len);
	memcpy(ptemp, (BYTE*)tset, SET_NUMBER*sizeof(TSETVAL));

	caSetBuf = prSet[fd+1];
	prFdSetConv(fd);
	prFdSetNoSwitch(fd);
	
	sprintf(fname, "fd%dset.cfg", fd+1);
	if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return 2;
	}		
	smMGive(prSetSem);

	return 0;

}
#endif
#endif

#if defined(_YIERCI_RH_) ||(TYPE_USER == USER_GUIYANG)
// 1Ϊ ���þ͵�(�ֶ�)��0 Ϊ������(����); 
int SetFaTypePrset(BYTE on) 
{
	TSETVAL *tset = (TSETVAL*)(prSet[0]);
	DWORD yb_kg1_kg2 = 0;
	DWORD yb_read,yb_readagain;
	VPrSetPublic *pprSetPublic;
	struct VFileHead *head;
	BYTE *wrSet;
	DWORD len;
	int i;
	pprSetPublic = prSetPublic[prRunPublic->set_no];
    if(g_prInit == 0)
		return -1;
    
	yb_kg1_kg2 = pprSetPublic->dKG2;
	len = yb_kg1_kg2;
	yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE; 
	
	if(on)
	{
		yb_kg1_kg2 |= PR_MODE_SEG;
	}
	else 
	{
		yb_kg1_kg2 |= PR_MODE_PR;
	}
	
	if(len == yb_kg1_kg2) //��֮ǰһ������
	{
		return OK;
	}

	if(on)//�͵�
	{
	    extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&yb_read,sizeof(DWORD));
		for(i=0; i<50; i++);
	    extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&yb_readagain,sizeof(DWORD));

	    if(yb_read == yb_readagain)
	   		ReverseKSet(SET_YB, (BYTE*)tset, yb_read);
		else
			return ERROR;
	}
	else//����
	{
		
		extNvRamSet(NVRAM_DEV_PUBYB,(BYTE*)&pprSetPublic->dYb,sizeof(DWORD));
		for(i=0; i<50; i++);
		extNvRamGet(NVRAM_DEV_PUBYB,(BYTE*)&yb_read,sizeof(DWORD));
	
		if(pprSetPublic->dYb != yb_read)//�����쳣�ȴ���һ����������д��
			return ERROR;
	}
	
	ReverseKSet(SET_KG2, (BYTE*)tset, yb_kg1_kg2);

	caSetBuf = prSet[0];
	if(prPubSetConv(0) != SET_OK) return ERROR;
	
	smMTake(prSetSem);

	head = (struct VFileHead *)prSetTmp;	
	head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
	head->wVersion = CFG_FILE_VER;
	head->wAttr = CFG_ATTR_NULL;
	wrSet = (BYTE*)(head+1);
	len = sizeof(TTABLETYPE);
	memcpy(wrSet,&tSetType,len);
	memcpy(wrSet+len,prSet[0],SET_PUB_NUMBER*sizeof(TSETVAL));
	
	if(WriteParaFile("fdPubSet.cfg",(struct VFileHead*)prSetTmp) == ERROR)
	{
		smMGive(prSetSem);
		return ERROR;
	}
	smMGive(prSetSem);
	if(prRunPublic->set_no == 0)
		prRunPublic->set_no =1;
	else
		prRunPublic->set_no = 0;
	return OK;				
}
#endif
extern int fdNum;
int ReadSetName(int fd, BYTE * pName)
{
    WORD iofd;
#if (TYPE_IO == IO_TYPE_MASTER)
	struct VExtIoConfig *pCfg;
#endif
	int ret = OK;	
		
	if (fd < 0) return ERROR;

	memset(pName, 0, PR_SET_NAME);

    if ((fd == 0)|| (fd > fdNum))
		strcpy((char*)pName, "ProSetPubTable.cfg");
	else if (GetMyIoNo(4, (WORD)(fd-1), &iofd) == OK)
		strcpy((char*)pName, "ProSetTable.cfg");
#if (TYPE_IO == IO_TYPE_MASTER)
	else
	{
	    pCfg = GetExtIoNo(4, (WORD)(fd-1), &iofd);
		if (pCfg == NULL)
			return ERROR;
		else
		    sprintf((char*)pName, "ProSetTable%d.cfg", pCfg->pAddrList->ExtIoAddr.wAddr);
	}
#endif
	return ret;

}

int ReadSetNum(int fd)
{
    WORD iofd;
    if (fd < 0) return ERROR;

	if ((fd == 0) || (fd > fdNum))
	{
	    if (GetMyIoNo(4, 0, &iofd) == OK)
	#ifdef INCLUDE_PR
		   return SET_PUB_NUMBER;
	#else
       return 0;
	#endif
	}

	if (GetMyIoNo(4, fd-1, &iofd) == OK)
	#ifdef INCLUDE_PR
		return SET_NUMBER;
   #else
	   return 0;
	#endif

	return ERROR;
}

//������������ֻ�ڵ�һ��������İ���
int ReadPubSet(BYTE *pSet,int fd)
{
    WORD iofd;
#if (TYPE_IO == IO_TYPE_MASTER)
	struct VExtIoConfig *pCfg;
	VExtIoCmd *pCmd = (VExtIoCmd *)g_Sys.byIOCmdBuf;
	BYTE *pData = (BYTE *)(pCmd+1);
#endif
    int ret = OK;

    if (GetMyIoNo(4, 0, &iofd) == OK)
    {
#ifdef INCLUDE_PR
//       if (g_prInit == 0)
//		   return ERROR;
       memcpy(pSet, &tSetType, sizeof(TTABLETYPE));
	   memcpy(pSet+sizeof(TTABLETYPE), prSet[prRunPublic[fd].prSetnum], SET_PUB_NUMBER*sizeof(TSETVAL));
#endif
    }
#if (TYPE_IO == IO_TYPE_MASTER)	
    else
    {
	   pCfg = GetExtIoNo(4, 0, &iofd);
	   if (pCfg == NULL)
		   return ERROR;
	   else
	   {
		   smMTake(g_Sys.dwExtIoDataSem);
           pCmd->addr = pCfg->pAddrList->ExtIoAddr.wAddr;
		   pCmd->cmd = EXTIO_CMD_PRSET_READ;
		   pCmd->len = sizeof(BYTE);
		   *pData = (BYTE)0;
		   ret = ExtIoCmd(NULL, NULL, SECTOTICK(2));
		   if (ret == OK)
			   memcpy(pSet, pData+1,PR_SET_SIZE);
		   smMGive(g_Sys.dwExtIoDataSem);			
	   }
   }
#endif
	 return ret;
}

int WritePubSet(BYTE *pSet,int fd)
{
    WORD iofd;
#ifdef INCLUDE_PR
	struct VFileHead *head;
#endif

    int ret = OK;

	if (GetMyIoNo(4, 0, &iofd) == OK)
	{	
#ifdef INCLUDE_PR
 //       if (g_prInit == 0)
//		   return ERROR;
	    caSetBuf  = pSet + sizeof(TTABLETYPE);

		if (prPubSetConv(fd) != SET_OK) return 1;	

		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		memcpy((head+1), pSet, PR_SET_SIZE);
	
		if (WriteParaFile("fdPubSet.cfg", (struct VFileHead *) prSetTmp) == ERROR)
		{
			return 2;
		}	
		memcpy(prSet[prRunPublic[fd].prSetnum], pSet+sizeof(TTABLETYPE), SET_PUB_NUMBER*sizeof(TSETVAL));
	
		if (prRunPublic[fd].set_no == 0)
			prRunPublic[fd].set_no = 1;
		else
			prRunPublic[fd].set_no = 0;
#endif
	}
	return ret;
}

int ReadSet(int fd, BYTE *pSet)
{
    WORD iofd;
    int ret = OK;	

 		
	if (fd < 0) return ERROR;

	memset(pSet, 0, PR_SET_SIZE);

	if (fd == 0)
       ReadPubSet(pSet,0); 
	else if(fd > fdNum)
	   ReadPubSet(pSet,fd-fdNum);
	else if (GetMyIoNo(4, (WORD)(fd-1), &iofd) == OK)
	{	
#ifdef INCLUDE_PR
  //      if (g_prInit == 0)
	//	   return ERROR;
        memcpy(pSet, &tSetType, sizeof(TTABLETYPE));
	    memcpy(pSet+sizeof(TTABLETYPE), prSet[iofd+1], SET_NUMBER*sizeof(TSETVAL));
#endif
	}	
	return ret;
}

/* 0   �ɹ�
   ��0 ����
    -1 ���ߺŴ���
     1 ��ֵ�Ƿ�
     2 д��ֵ�ļ�����
*/     
int WriteSet(int fd, BYTE *pSet)
{
#ifdef INCLUDE_PR
	struct VFileHead *head;
	char fname[MAXFILENAME];
#endif
    WORD iofd;
    int ret = OK;	
  
	if (fd < 0) return ERROR;

	if (fd == 0)
		WritePubSet(pSet,0);
	else if(fd > fdNum)
		WritePubSet(pSet,fd-fdNum);
	else if(GetMyIoNo(4, (WORD)(fd-1), &iofd) == OK)
	{	
#ifdef INCLUDE_PR
	    caSetBuf  = pSet + sizeof(TTABLETYPE);

		if (prFdSetConv(iofd) != SET_OK) return 1;	

		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		memcpy((head+1), pSet, PR_SET_SIZE);
	
		sprintf(fname, "fd%dset.cfg", iofd+1);
		if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
		{
			return 2;
		}		
		
		memcpy(prSet[iofd+1], pSet+sizeof(TTABLETYPE), SET_NUMBER*sizeof(TSETVAL));
	
		prFdSetNoSwitch(iofd);
#endif
	}
	return ret;
}



void WritePrFile(int no)
{
#ifdef INCLUDE_PR
		struct VFileHead *head;
		char fname[MAXFILENAME];
		BYTE *wrSet;
		WORD len;
		
		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		wrSet = (BYTE*)(head+1);
		len = sizeof(TTABLETYPE);
		memcpy(wrSet,&tSetType,len);
		memcpy(wrSet+len,prSet[no],SET_NUMBER*sizeof(TSETVAL));
		
		if(no == 0)
		{
			if (WriteParaFile("fdPubSet.cfg", (struct VFileHead *) prSetTmp) == ERROR)
			{
				return ;
			}
		}
		else if(no > fdNum)
		{
			sprintf(fname, "fd%02dPubSet.cfg",no-fdNum+1);
			if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
			{
				return ;
			}
		}
		else
		{
			sprintf(fname, "fd%dset.cfg",no);
			if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
			{
				return ;
			}	
		}
#endif
}

#if (TYPE_USER != USER_FUJIAN)
BOOL b_YB_CHN = 0;
WORD w_JD_MODE = 0;	
int ReadPrPara(WORD parano,char *pbuf,WORD type)
{
#if 0
//#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	WORD fdno,lineno;
	BOOL bvalue;
	WORD svalue;
	DWORD dvalue; 
	float fvalue;
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	VPrSetPublic  *pprPubRunSet;
	TSETVAL *tset,*tpubset;
	TSETVAL *bval;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	bvalue = svalue = fvalue = dvalue = 0;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return ERROR;
	
	if(parano < PRPARA_LINE_ADDR)
	{
		fdno = 0;
		pInfo = prRunInfo + fdno;
		pprRunSet = prRunSet[pInfo->set_no] + fdno;
		tset = (TSETVAL*)(prSet[fdno+1]);

		pprPubRunSet = prSetPublic[prRunPublic->set_no];
		tpubset = (TSETVAL*)prSet[0];

		parano = parano - PRPARA_ADDR;
			
		switch(parano)
		{
			case PRP_YB_LEDKEEP:  //���ϵ��Զ�����
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bLedZdFg)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			
				break;
			case PRP_T_LEDKEEP: //���ϵ��Զ�����ʱ��
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_LED_TKEEP;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_T_YXKEEP: //����ң�ű���ʱ��
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TKEEP;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_YB_FTUSD:	//�׶�FTUͶ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->bFTUSD)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_T_X:	//X
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TX;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);

				break;
			case PRP_T_Y:  //Y
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TY;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_T_C:	//C
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TC;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_T_S:  //S
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TS;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;

			case PRP_T_DXJD:  //����ӵ���բʱ��
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TDXJD;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;

			case PRP_T_HZ: //ѡ����բ�غ�ʱ�䶨ֵ
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tpubset + SET_PUB_TXXCH;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_YB_ZSYI:	//����Ӧ����·���ϴ���
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->bXJDL)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_ZSYI0:  //����Ӧ����ӵع��ϴ���
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->bDXJD)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_CH1:  //һ���غ�բͶ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->dYb & PR_YB_CH)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_T_CH1:  //һ���غ�ʱ��
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TCH1;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_YB_IH:	//�����������Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bDdlBsch)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_I_IH:  //��������ض�ֵ
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_CHBSDL;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
			case PRP_YB_CHN: // ����غ�բͶ��
				
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;

				bval = tset + SET_CHNUM;
				SetToFloat(bval,&fvalue);
				svalue = (WORD)fvalue;
				
				if(b_YB_CHN)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_I_CHN: // �غ�բ����
				fParaInfo->type = USHORT_TYPE;
				fParaInfo->len = USHORT_LEN;
				bval = tset + SET_CHNUM;
				SetToFloat(bval,&fvalue);
				svalue = (WORD)fvalue;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				
				break;
			case PRP_T_CHN: // �ǵ�һ���غ�բ��ʱ
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TCH2;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_YB_JDMODE: // ϵͳ�ӵط�ʽ
				fParaInfo->type = USHORT_TYPE;
				fParaInfo->len = USHORT_LEN;
				svalue = w_JD_MODE;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				
				break;
			case PRP_YB_FDFJ:  // �ֶλ�ֽ�
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->byWorkMode == PR_MODE_FJ)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_YB_FDLL: // ����ֶ�ģʽ
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprPubRunSet->byWorkMode == PR_MODE_SEG)
					bvalue = 0;
				else
					bvalue = 1;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_JDFA: // �͵���FA ģʽ
				fParaInfo->type = USHORT_TYPE;
				fParaInfo->len = USHORT_LEN;
				if(pprPubRunSet->bFaWorkMode == PR_FA_ZSY)
					svalue = 0;
				else if(pprPubRunSet->bFaWorkMode == PR_FA_U)
					svalue = 1;
				else if(pprPubRunSet->bFaWorkMode == PR_FA_I)
					svalue = 2;
				else 
					svalue = 1;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				
				break;
			default:
				fParaInfo->type = 4;
				fParaInfo->len = 0;
				svalue = 0;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				break;
		}
	}
	else if(parano >= PRPARA_LINE_ADDR)
	{
		if(parano > (PRPARA_LINE_ADDR + PRPARA_LINE_NUM*fdNum -1))
			return ERROR;
			
		fdno = (parano - PRPARA_LINE_ADDR)/PRPARA_LINE_NUM;
		lineno = parano - PRPARA_LINE_ADDR - fdno*PRPARA_LINE_NUM;
		
		pInfo = prRunInfo + fdno;
		pprRunSet = prRunSet[pInfo->set_no] + fdno;
		tset = (TSETVAL*)(prSet[fdno+1]);

		switch(lineno)
		{
			case PRP_YB_GLTRIP://����ͣ����բͶ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bITrip == 0)
					bvalue = 0;
				else
					bvalue = 1;
			
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			  break;
			case PRP_YB_I1TT://����һ�θ澯Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->dYb & PR_YB_I1)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
			case PRP_YB_I1TRIP://����һ�γ���Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if((pprRunSet->bI1Trip) && (pprRunSet->dYb & PR_YB_I1))
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
			case PRP_I_I1://����һ�ζ�ֵ
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_I1;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_T_I1://����һ��ʱ��

				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_T1;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);

				break;	
			case PRP_YB_I2TT://��������Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->dYb & PR_YB_I2)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_YB_I2TRIP://�������γ���
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if((pprRunSet->bI2Trip) &&(pprRunSet->dYb & PR_YB_I2))
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_I_I2://�������ζ�ֵ
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_I2;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_T_I2://��������ʱ��

				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_T2;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				
				break;
			case PRP_YB_I0TT://��������Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->dYb & PR_YB_I01)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_I0TRIP://������������Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if((pprRunSet->bI0Trip)&&(pprRunSet->dYb & PR_YB_I01))
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_I_I0://����������ֵ
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_3I01;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			
				break;
			case PRP_T_I0://��������ʱ��
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_TI01;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);	
				break;
				
			case PRP_YB_ILTT:   //С�����ӵ�Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				bvalue = 1;
				if(pprRunSet->bXDLYB)
					bvalue = 1;
				else
					bvalue = 0;	
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
				
			case PRP_YB_ILTRIP://С�����ӵس���Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bXDLTrip && pprRunSet->bXDLYB)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
			case PRP_YB_FZDBS: // ���ڶϵ�������Ͷ��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bfZdbh)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
			case PRP_I_FZDBS: // ���ڶϵ�����ֵ
				fParaInfo->type = FLOAT_TYPE;
				fParaInfo->len = FLOAT_LEN;
				bval = tset + SET_IKG;
				SetToFloat(bval,&fvalue);
				memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
				break;
#if 0
			case PRP_YB_GLLB:  //����¼��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bRcdGl)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_YB_SYLB:  //ʧѹ¼��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bRcdSy)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);

				break;
			case PRP_YB_U0LB:   //�����ѹ¼��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bRcdU0)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				
				break;
			case PRP_YB_I0LB:  //�������ͻ��¼��
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				if(pprRunSet->bRcdI0TB)
					bvalue = 1;
				else
					bvalue = 0;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
#endif
			default://����
				fParaInfo->type = 4;
				fParaInfo->len = 0;
				svalue = 0;
				memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
				break;
			
		}
	}

	return OK;
#else
	return ERROR;
#endif
}

/*0 �ɹ�
1 ��Ϣ���ַ����
2 �������͡����ȴ���
3 ֵ��Χ����
4 ����
*/
int WritePrParaYZ(WORD parano,char *pbuf)
{
#if 0
//#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	WORD fdno,lineno;
	WORD svalue;
	float fvalue;
	DWORD dvalue;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	svalue = fvalue = dvalue =0;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	if(parano < PRPARA_LINE_ADDR)
	{
		parano = parano - PRPARA_ADDR;
		
		switch(parano)
		{
			case PRP_YB_LEDKEEP:  //���ϵ��Զ�����
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
			
				break;
			case PRP_T_LEDKEEP: //���ϵ��Զ�����ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 86400)
					return 3;
				
				break;
			case PRP_T_YXKEEP: //����ң�ű���ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 300)
					return 3;
			
				break;
			case PRP_YB_FTUSD:	//�׶�FTUͶ��

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_T_X:	//X
				
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				break;
			case PRP_T_Y:  //Y
				
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;

				
				break;
			case PRP_T_C:	//C

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 6000)
					return 3;
				
				break;
			case PRP_T_S:  //S
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 6000)
					return 3;
				
				break;

			case PRP_T_DXJD:  //����ӵ���բʱ��

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				break;
				
			case PRP_T_HZ: //ѡ����բ�غ�ʱ�䶨ֵ
				
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;

				break;
			case PRP_YB_ZSYI:	//����Ӧ����·���ϴ���

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_ZSYI0:  //����Ӧ����ӵع��ϴ���

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_CH1:  //һ���غ�բͶ��

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_T_CH1:  //һ���غ�ʱ��

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue < 0.01) || ((fvalue) > 600))
					return 3;
			
				break;
			case PRP_YB_IH:	//�����������Ͷ��
			
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_I_IH:  //��������ض�ֵ

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
			
				break;
			case PRP_YB_CHN:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_I_CHN:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				
				break;
			case PRP_T_CHN:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue < 0) || ((fvalue*1000) > 100000))
					return 3;
				break;
			case PRP_YB_JDMODE:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				break;
			case PRP_YB_FDFJ:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_FDLL:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_JDFA:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				break;
			default:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
		}
	}
	else if(parano >= PRPARA_LINE_ADDR)
	{
		if(parano > (PRPARA_LINE_ADDR + PRPARA_LINE_NUM*fdNum -1))
			return 1;
			
		fdno = (parano - PRPARA_LINE_ADDR)/PRPARA_LINE_NUM;
		lineno = parano -PRPARA_LINE_ADDR - fdno*PRPARA_LINE_NUM;

		switch(lineno)
		{
			case PRP_YB_GLTRIP://����ͣ����բͶ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
			  break;
			case PRP_YB_I1TT://����һ��Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_I1TRIP://����һ�γ���Ͷ��
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_I_I1://����һ�ζ�ֵ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
			
				break;
			case PRP_T_I1://����һ��ʱ��

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				
				if((fvalue*1000) > 100000)
					return 3;
				break;	
			case PRP_YB_I2TT://��������TT
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;

				break;
			case PRP_YB_I2TRIP://�������γ���
				if  ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_I_I2://�������ζ�ֵ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
			
				break;
			case PRP_T_I2://��������ʱ��

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;
				break;
			case PRP_YB_I0TT://��������Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_I0TRIP://������������Ͷ��
				if  ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_I_I0://����������ֵ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 10) || ((fvalue*1000) > 150000))
					return 3;
			
				break;
			case PRP_T_I0://��������ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;
			
				break;
			case PRP_YB_ILTT://С�����ӵ�Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_ILTRIP://С�����ӵس���Ͷ��
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
			case PRP_YB_FZDBS:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_I_FZDBS:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 10) || ((fvalue*1000) > 150000))
					return 3;

				break;
#if 0
		      case PRP_YB_GLLB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_SYLB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_U0LB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
			case PRP_YB_I0LB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
#endif
			default://����
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				break;
		}
	}
	return OK;
#else
	return ERROR;
#endif
}

/*0 �ɹ�
1 ��Ϣ���ַ����
2 �������͡����ȴ���
3 ֵ��Χ����
4 ����
5 дʧ��
*/
int WritePrPara(WORD parano,char *pbuf,WORD type)
{
#if 0
//#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	WORD fdno,lineno;
	BOOL bvalue;
	WORD svalue;
	DWORD dvalue;
	float fvalue;
	TSETVAL *tset,*tpubset;
	TSETVAL *bval;
	DWORD yb_kg1_kg2;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	bvalue = svalue = fvalue = dvalue = 0;
	
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	if(parano < PRPARA_LINE_ADDR)
	{
		fdno = 0;
		tset = (TSETVAL*)(prSet[fdno+1]);
		tpubset = (TSETVAL*)prSet[0];
		
		parano = parano - PRPARA_ADDR;
		switch(parano)
		{
			case PRP_YB_LEDKEEP:  //���ϵ��Զ�����
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_LED_FG;
				else
					yb_kg1_kg2 &= ~PR_CFG1_LED_FG;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_T_LEDKEEP: //���ϵ��Զ�����ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 86400)
					return 3;
				bval = tset + SET_LED_TKEEP;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_YXKEEP: //����ң�ű���ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 300)
					return 3;
				bval = tset + SET_TKEEP;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_YB_FTUSD:	//�׶�FTUͶ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG1,&yb_kg1_kg2,PR_SET_PUB);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_FTU_SD;
				else 
					yb_kg1_kg2 &= ~PR_CFG1_FTU_SD;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				
				break;
			case PRP_T_X:	//X
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				bval = tpubset + SET_PUB_TX;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_Y:  //Y
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				bval = tpubset + SET_PUB_TY;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_C:	//C
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 6000)
					return 3;
				
				bval = tpubset + SET_PUB_TC;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_S:  //S
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 6000)
					return 3;
				
				bval = tpubset + SET_PUB_TS;
				FloatToSet(fvalue,bval);
				
				break;
				
			case PRP_T_DXJD:  //����ӵ���բʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				bval = tset + SET_TDXJD;
				FloatToSet(fvalue,bval);
				
				break;
				
			case PRP_T_HZ: //ѡ����բ�غ�ʱ�䶨ֵ

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(fvalue > 60)
					return 3;
				
				bval = tpubset + SET_PUB_TXXCH;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_YB_ZSYI:	//����Ӧ����·���ϴ���
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG1,&yb_kg1_kg2,PR_SET_PUB);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_ZSY_XJDL;
				else 
					yb_kg1_kg2 &= ~PR_CFG1_ZSY_XJDL;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_YB_ZSYI0:  //����Ӧ����ӵع��ϴ���

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG1,&yb_kg1_kg2,PR_SET_PUB);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_ZSY_DXJD;
				else 
					yb_kg1_kg2 &= ~PR_CFG1_ZSY_DXJD;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_YB_CH1:  //һ���غ�բͶ��

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_CH;
				else
					yb_kg1_kg2 &= ~PR_YB_CH;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				
				
				break;
			case PRP_T_CH1:  //һ���غ�ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue < 0.01) || (fvalue > 600))
					return 3;
				bval = tset + SET_TCH1;
				FloatToSet(fvalue,bval);
			
				break;
			case PRP_YB_IH:	//�����������Ͷ��
				
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_CH_DDLBS;
				else
					yb_kg1_kg2 &= ~PR_CFG1_CH_DDLBS;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_I_IH:  //��������ض�ֵ

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
				bval = tset + SET_CHBSDL;
				FloatToSet(fvalue,bval);
			
				break;
			case PRP_YB_CHN:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_CH;
				b_YB_CHN = bvalue;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_I_CHN:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if(svalue > 3)
					svalue = 3;
				
				fvalue = svalue;
				bval = tset + SET_CHNUM;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_T_CHN:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue < 0) || ((fvalue*1000) > 100000))
					return 3;
				
				bval = tset + SET_TCH2;
				FloatToSet(fvalue,bval);
				bval = tset + SET_TCH3;
				FloatToSet(fvalue,bval);
				
				break;
			case PRP_YB_JDMODE:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				w_JD_MODE = svalue;
				if(w_JD_MODE <2) // 0\1 ��С����Ͷ��
				{
					caSetBuf = prSet[fdno+1];
					CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_CFG2_XDL_YB;
					ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);	
				}
				else if(w_JD_MODE == 2) // 2 
				{
					caSetBuf = prSet[fdno+1];
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				
				break;
			case PRP_YB_FDFJ:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_PUB);
				yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE;
				if(bvalue)
					yb_kg1_kg2 |= PR_MODE_FJ;
				else
					yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_YB_FDLL:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_PUB);
				yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE;
				if(bvalue)
					yb_kg1_kg2 |= PR_MODE_TIE;
				else
					yb_kg1_kg2 &= ~PR_CFG2_CONTROL_MODE;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				caSetBuf = prSet[0];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_PUB);
				if(bvalue)
				{
					yb_kg1_kg2 |= PR_YB_SUSY;
					yb_kg1_kg2 &= ~PR_YB_SUDY;
				}
				else
				{
					yb_kg1_kg2 |= PR_YB_SUDY;
					yb_kg1_kg2 &= ~PR_YB_SUSY;
				}
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_YB_JDFA:
				if((fParaInfo->type != USHORT_TYPE) || (fParaInfo->len != USHORT_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				bvalue = svalue;
				caSetBuf = prSet[0];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_PUB);
				yb_kg1_kg2 &= ~PR_CFG2_FA_MODE;
				if(bvalue == 0)
					yb_kg1_kg2 |= (PR_FA_ZSY << KG_3);
				else if(bvalue == 1)
					yb_kg1_kg2 |= (PR_FA_U << KG_3);
				else if(bvalue == 2)
					yb_kg1_kg2 |= (PR_FA_I << KG_3);
				else
					yb_kg1_kg2 &= ~PR_CFG2_FA_MODE;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				break;
			default:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				break;
		}
	}
	else if(parano >= PRPARA_LINE_ADDR)
	{
		if(parano > (PRPARA_LINE_ADDR + PRPARA_LINE_NUM*fdNum -1))
			return 1;
			
		fdno = (parano - PRPARA_LINE_ADDR)/PRPARA_LINE_NUM;
		lineno = parano - PRPARA_LINE_ADDR - fdno*PRPARA_LINE_NUM;
		tset = (TSETVAL*)(prSet[fdno + 1]);

		switch(lineno)
		{
			case PRP_YB_GLTRIP://����ͣ����բͶ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= (1 << KG_17);
				else
					yb_kg1_kg2 &= ~PR_CFG1_TRIP_MODE;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
			  break;

			case PRP_YB_I1TT://����һ��Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_I1;
				else
					yb_kg1_kg2 &= ~PR_YB_I1;
		
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);

				if(bvalue == 0)
				{
					CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
					ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				}

	     /*			                                      // gysh
				if((yb_kg1_kg2 & PR_CFG1_I1_WARN)|(yb_kg1_kg2 & PR_CFG1_I1_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
					*/
				break;
			case PRP_YB_I1TRIP://����һ�γ���Ͷ��
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I1_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				
				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		
					yb_kg1_kg2 |= PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				
			/*	                                           // gysh
				if((yb_kg1_kg2 & PR_CFG1_I1_WARN)|(yb_kg1_kg2 & PR_CFG1_I1_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I1;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				*/
				break;
			case PRP_I_I1://����һ�ζ�ֵ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
				
				bval = tset + SET_I1;
				FloatToSet(fvalue,bval);
				break;
			case PRP_T_I1://����һ��ʱ��

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;
				bval = tset + SET_T1;
				FloatToSet(fvalue,bval);
			
				break;	
			case PRP_YB_I2TT://��������Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
  
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_I2;
				else
					yb_kg1_kg2 &= ~PR_YB_I2;
		
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);

				if(bvalue == 0)
				{
					CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
					ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				}
			/*	
				if((yb_kg1_kg2 & PR_CFG1_I2_WARN)|(yb_kg1_kg2 & PR_CFG1_I2_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
           */
				break;
			case PRP_YB_I2TRIP://�������γ���
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I2_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		
					yb_kg1_kg2 |= PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
			/*	                                         // gysh
				if((yb_kg1_kg2 & PR_CFG1_I2_WARN)|(yb_kg1_kg2 & PR_CFG1_I2_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I2;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				*/
				break;
			case PRP_I_I2://�������ζ�ֵ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 50) || ((fvalue*1000) > 150000))
					return 3;
				
				bval = tset + SET_I2;
				FloatToSet(fvalue,bval);
			
				break;
			case PRP_T_I2://��������ʱ��

				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;
				bval = tset + SET_T2;
				FloatToSet(fvalue,bval);
			
				break;
			case PRP_YB_I0TT://��������Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_I01;
				else
					yb_kg1_kg2 &= ~PR_YB_I01;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);

				if(bvalue == 0)
				{
					CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
					ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				}

		/*		
				if((yb_kg1_kg2 & PR_CFG1_I0_WARN)|(yb_kg1_kg2 & PR_CFG1_I0_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
	   */	
				break;
			case PRP_YB_I0TRIP://������������Ͷ��
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I0_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		
					yb_kg1_kg2 |= PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
	   /*	
				if((yb_kg1_kg2 & PR_CFG1_I0_WARN)|(yb_kg1_kg2 & PR_CFG1_I0_TRIP))
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_YB_I01;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
		*/		
				break;
			case PRP_I_I0://����������ֵ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 10) || ((fvalue*1000) > 150000))
					return 3;
				bval = tset + SET_3I01;
				FloatToSet(fvalue,bval);	
				break;
			case PRP_T_I0://��������ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if((fvalue*1000) > 100000)
					return 3;	
				bval = tset + SET_TI01;
				FloatToSet(fvalue,bval);
				break;

			case PRP_YB_ILTT://С�����ӵظ澯Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_XDL_YB;
				else
					yb_kg1_kg2 &= ~PR_CFG2_XDL_YB;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				if(bvalue == 0)
				{
					CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 &= ~PR_CFG2_XDL_TRIP;
					ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				}
				
				break;

			case PRP_YB_ILTRIP://С�����ӵس���Ͷ��
				if ((g_Sys.byDevType&0x03) == 1)
					return 4;

				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_XDL_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG2_XDL_TRIP;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				if(bvalue)
				{
					CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
					yb_kg1_kg2 |= PR_CFG2_XDL_YB;
					ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				}
				
				break;
			case PRP_YB_FZDBS:	
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_FZD_BH;
				else
					yb_kg1_kg2 &= ~PR_CFG1_FZD_BH;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
				break;
			case PRP_I_FZDBS:
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);

				if(((fvalue*1000) < 10) || ((fvalue*1000) > 150000))
					return 3;
				bval = tset + SET_IKG;
				FloatToSet(fvalue,bval);
				
				break;
#if 0
			case PRP_YB_GLLB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_RCD_GL;
				else
					yb_kg1_kg2 &= ~PR_CFG2_RCD_GL;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				break;
				
			case PRP_YB_SYLB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_RCD_SY;
				else
					yb_kg1_kg2 &= ~PR_CFG2_RCD_SY;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				break;
				
			case PRP_YB_U0LB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_RCD_U0;
				else
					yb_kg1_kg2 &= ~PR_CFG2_RCD_U0;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				break;
				
			case PRP_YB_I0LB:
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[fdno+1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_RCD_I0TB;
				else
					yb_kg1_kg2 &= ~PR_CFG2_RCD_I0TB;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
				break;
#endif
			default://����
				fParaInfo->type = BOOL_TYPE;
				fParaInfo->len = BOOL_LEN;
				bvalue = 1;
				memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
				break;
		}
	}
	return OK;
#else
	return ERROR;
#endif
}
#else
#if(DEV_SP != DEV_SP_DTU)
int ReadPrPara(WORD parano,char *pbuf,WORD type)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	BOOL bvalue;
	WORD svalue;
	float fvalue;
	
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	VPrSetPublic  *pprPubRunSet;
	TSETVAL *tset;
	TSETVAL *bval;
	VFdCfg *pfdcfg;
	bvalue = svalue = fvalue = 0;
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return ERROR;
	pInfo = prRunInfo;
	pprRunSet = prRunSet[pInfo->set_no];
	tset = (TSETVAL*)(prSet[1]); //��ȡ��һ���ߣ�����ftu��dtuδ������ʱ������
	pfdcfg = g_Sys.MyCfg.pFd;
	pprPubRunSet = prSetPublic[prRunPublic->set_no];	
	switch(parano)
	{
		case 0x8228: //����һ��Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((pprRunSet->bI1Trip) && (pprRunSet->dYb & PR_YB_I1)) //ѹ�弰���ڶ�Ͷ��Ϊ��բ������Ϊ�澯
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8229: //����һ�ζ�ֵ A
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x822A: //����һ��ʱ��  S������Ϊms
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x822B: //����һ�ξ���ѹ����  ѹ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bIDYBS) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x822C: //����һ�ξ��������   
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bIDir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x822D: //��������Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((pprRunSet->bI2Trip) &&(pprRunSet->dYb & PR_YB_I2))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x822E: //�������ζ�ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I2;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x822F: //��������ʱ�� s
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T2;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8230: //�������ξ���ѹ����
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2DYBS) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8231: //�������ξ��������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2Dir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8232: //��������ٶ�ֵ A
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_IHJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8233: //���������ʱ�� s
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8234: //�����ɸ澯��ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_IGFH;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8235: //�����ɸ澯ʱ��
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGFH;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8236: //����һ��Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((pprRunSet->bI0Trip)&&(pprRunSet->dYb & PR_YB_I01))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8237:  //����һ�ζ�ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_3I01;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8238:  //����һ��ʱ��
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TI01;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8239:   //�����ξ��������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI0Dir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x823A:  //�������ٶ�ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I0HJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x823B:  //��������ʱ��
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I0TJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x823C:  //�غ�բͶ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_CH)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x823D:  //�غ�բ����ѹͶ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
#ifdef INCLUDE_PR_PRO			
			if(pprPubRunSet->bTqMode & PR_CH_MODE_WY)
				bvalue = 1;
			else
				bvalue = 0;
#else
			bvalue = 0;
#endif			
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x823E:  //�غ�բ��ͬ��Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
#ifdef INCLUDE_PR_PRO			
			if(pprPubRunSet->bTqMode &  PR_CH_MODE_TQ)
				bvalue = 1;
			else
				bvalue = 0;
#else
			bvalue = 0;
#endif
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x823F:  //�غ�բʱ��
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TCH1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8240:  // С�����ӵظ澯Ͷ��  0�˳�,1�澯
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			bvalue = 1;
			if(pprRunSet->bXDLYB)
				bvalue = 1;
			else
				bvalue = 0;	
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8241:  //�����ѹ��ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_3U0;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8242:  //��ѹ����Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bGyGJ)&&(pprRunSet->dYb & PR_YB_GY))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8243:  //��ѹ������ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_UGY;
			SetToFloat(bval,&fvalue);
			if(type ==0)
				fvalue = fvalue/pfdcfg->Un;
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8244:  //��ѹ������ʱ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGY;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8245:  //��Ƶ����Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bGpGJ)&&(pprRunSet->dYb & PR_YB_GP))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8246:  //��Ƶ������ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_FGP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8247:  //��Ƶ������ʱ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x8248:  //��Ƶ����Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bDpGJ)&&(pprRunSet->dYb & PR_YB_DP))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x8249:  //��Ƶ������ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_FDP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x824A: //��Ƶ������ʱ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TDP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x824B: //��������Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((pprRunSet->bI3Trip) && (pprRunSet->dYb & PR_YB_I3)) //ѹ�弰���ڶ�Ͷ��Ϊ��բ������Ϊ�澯
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 0x824C: //�������ζ�ֵ A
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I3;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x824D: //��������ʱ��  S������Ϊms
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T3;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 0x824E:
		case 0x824F:
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
		default:
#if 0
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
#endif
			return ERROR;

	}
	
		
	return OK;
#else
	return ERROR;
#endif	
}

int WritePrParaYZ(WORD parano,char *pbuf)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	return CheckParaType_Len(fParaInfo->type,fParaInfo->len);
#else
	return ERROR;
#endif	
}

int WritePrPara(WORD parano,char *pbuf,WORD type)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	BOOL bvalue;
	WORD svalue;
	DWORD dvalue; 
	float fvalue;
	DWORD yb_kg1_kg2;
//	VPrRunInfo *pInfo;
//	VPrRunSet *pprRunSet;
//	VPrSetPublic  *pprPubRunSet;
	TSETVAL *tset;
	TSETVAL *bval;
	VFdCfg *pfdcfg;
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	bvalue = svalue = fvalue = dvalue = 0;
//	pInfo = prRunInfo;
//	pprRunSet = prRunSet[pInfo->set_no];
	tset = (TSETVAL*)(prSet[1]);

//	pprPubRunSet = prSetPublic[prRunPublic->set_no];
	pfdcfg = g_Sys.MyCfg.pFd;
	switch(parano)
	{
		case 0x8228: //����һ��Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I1_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_I1;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 0x8229: //����һ�ζ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_I1;
				FloatToSet(fvalue,bval);
			break;
		case 0x822A: //����һ��ʱ��
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_T1;
				FloatToSet(fvalue,bval);
			break;
		case 0x822B: //����һ�ξ���ѹ����
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I_DY;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I_DY;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
					yb_kg1_kg2 |= PR_YB_DY;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
			break;
		case 0x822C: //����һ�ξ��������
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I_DIR;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I_DIR;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 0x822D: //��������Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I2_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_I2;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 0x822E: //�������ζ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_I2;
				FloatToSet(fvalue,bval);
			break;
		case 0x822F: //��������ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_T2;
				FloatToSet(fvalue,bval);
			break;
		case 0x8230: //�������ξ���ѹ����
			
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I2_DY;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I2_DY;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
					yb_kg1_kg2 |= PR_YB_DY;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
			break;
		case 0x8231: //�������ξ��������
			
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I2_DIR;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I2_DIR;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
				
			break;
		case 0x8232: //��������ٶ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_IHJS;
				FloatToSet(fvalue,bval);
			break;
		case 0x8233: //���������ʱ��
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_TJS;
				FloatToSet(fvalue,bval);
			break;
		case 0x8234: //�����ɸ澯��ֵ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_IGFH;
				FloatToSet(fvalue,bval);
			break;
		case 0x8235: //�����ɸ澯ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 600000) //600s
					return 3;
				bval = tset + SET_TGFH;
				FloatToSet(fvalue,bval);
			break;
		case 0x8236: //����һ��Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I0_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_I01;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 0x8237:  //����һ�ζ�ֵ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_3I01;
				FloatToSet(fvalue,bval);
			break;
		case 0x8238:  //����һ��ʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_TI01;
				FloatToSet(fvalue,bval);
			break;
		case 0x8239:   //�����ξ��������
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG1_I0_DIR;
				else
					yb_kg1_kg2 &= ~PR_CFG1_I0_DIR;
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 0x823A:  //�������ٶ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100A
					return 3;
				bval = tset + SET_I0HJS;
				FloatToSet(fvalue,bval);
			break;
		case 0x823B:  //��������ʱ��
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_I0TJS;
				FloatToSet(fvalue,bval);
			break;
		case 0x823C:  //�غ�բͶ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_YB_CH;
				else
					yb_kg1_kg2 &= ~PR_YB_CH;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 0x823D:  //�غ�բ����ѹͶ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
#ifdef INCLUDE_PR_PRO	
				caSetBuf = prSet[0];
				CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= (1<<KG_8); //
				else
					yb_kg1_kg2 &= ~(1<<KG_8); //
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
#endif
			break;
		case 0x823E:  //�غ�բ��ͬ��Ͷ��    ����
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
#ifdef INCLUDE_PR_PRO	
				caSetBuf = prSet[0];
				CvtK(SET_KG1,&yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |=  (2<<KG_8); //
				else
					yb_kg1_kg2 &= ~ (2<<KG_8); //
				ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
#endif
			break;
		
		case 0x823F:  //�غ�բʱ��
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 600000)
					return 3;
				bval = tset + SET_TCH1;
				FloatToSet(fvalue,bval);
			break;
		case 0x8240:  // С�����ӵظ澯Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_XDL_YB;
				else
					yb_kg1_kg2 &= ~PR_CFG2_XDL_YB;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
			break;
		case 0x8241:  //�����ѹ��ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_3U0;
				FloatToSet(fvalue,bval);
			break;
		case 0x8242:  //��ѹ����Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_GY_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG2_GY_TRIP;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				if(bvalue)
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
					yb_kg1_kg2 |= PR_YB_GY;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
				else
				{
					CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
					yb_kg1_kg2 &= ~PR_YB_GY;
					ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
				}
			break;
		case 0x8243:  //��ѹ������ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if(type ==0)
				fvalue = fvalue*pfdcfg->Un;
			
				dvalue = fvalue*1000;
				if(dvalue > 400000) //400V
					return 3;
				bval = tset + SET_UGY;
				FloatToSet(fvalue,bval);
			break;
		case 0x8244:  //��ѹ������ʱ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //100s
					return 3;
				bval = tset + SET_TGY;
				FloatToSet(fvalue,bval);
			break;
		case 0x8245:  //��Ƶ����Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_GF_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG2_GF_TRIP;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_GP;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		
		case 0x8246:  //��Ƶ������ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000)
					return 3;
				bval = tset + SET_FGP;
				FloatToSet(fvalue,bval);
			break;
		case 0x8247:  //��Ƶ������ʱ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //
					return 3;
				bval = tset + SET_TGP;
				FloatToSet(fvalue,bval);
			break;
		case 0x8248:  //��Ƶ����Ͷ��
				if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
					return 2;
				memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				if((svalue != 0) && (svalue != 1))
					return 3;
				
				bvalue = svalue;
				caSetBuf = prSet[1];
				CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
				if(bvalue)
					yb_kg1_kg2 |= PR_CFG2_DF_TRIP;
				else
					yb_kg1_kg2 &= ~PR_CFG2_DF_TRIP;
				ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_DP;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 0x8249:  //��Ƶ������ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) //
					return 3;
				bval = tset + SET_FDP;
				FloatToSet(fvalue,bval);
			break;
		case 0x824A: //��Ƶ������ʱ
				if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
					return 2;
				memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
				dvalue = fvalue*1000;
				if(dvalue > 100000) 
					return 3;
				bval = tset + SET_TDP;
				FloatToSet(fvalue,bval);
			break;
		case 0x824B: //��������Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_I3_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_I3_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			yb_kg1_kg2 |= PR_YB_I3;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
		break;
	case 0x824C: //�������ζ�ֵ
		if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_I3;
			FloatToSet(fvalue,bval);
		break;
	case 0x824D: //��������ʱ��
		if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_T3;
			FloatToSet(fvalue,bval);
		break;
		default:
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
	}
	
		
	return OK;
#else
	return ERROR;
#endif	
}
#else
#define PARAFDNUM 42
WORD SNnum = 1;
int ReadPrPara(WORD parano,char *pbuf,WORD type)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	BOOL bvalue;
	WORD svalue,fd;
	float fvalue;
	
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	TSETVAL *tset;
	TSETVAL *bval;
	VFdCfg *pfdcfg;
	bvalue = svalue = fvalue = fd =0;
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return ERROR;

	parano = parano - PRPARA_ADDR + 1;
	if(parano < 5)
	{
		if(parano == 1)
		{
			fParaInfo->type = USHORT_TYPE;
			fParaInfo->len = USHORT_LEN;
			memcpy(fParaInfo->valuech,&SNnum,fParaInfo->len);
		}
		else
		{
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
		}
		return OK;
	}
		
	fd = (parano - 5)/PARAFDNUM;
	parano = parano - fd*PARAFDNUM;
	
	pInfo = prRunInfo+fd;
	pprRunSet = prRunSet[pInfo->set_no]+fd;
	pfdcfg = g_Sys.MyCfg.pFd+fd;
	tset = (TSETVAL*)(prSet[fd+1]); 

	switch(parano)
	{
		case 5: //����һ��Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_I1) //ѹ�弰���ڶ�Ͷ��Ϊ��բ������Ϊ�澯
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 6: //����һ�ζ�ֵ A
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 7: //����һ��ʱ��  S������Ϊms
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T1;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 8: //��������Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_I2)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 9: //�������ζ�ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I2;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 10: //��������ʱ�� s
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_T2;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 11: //�����ɸ澯Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_GFH)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 12: //�����ɸ澯��ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_IGFH;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 13: //�����ɸ澯ʱ��
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGFH;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 14: //����һ��Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_I01)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 15:  //����һ�ζ�ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_3I01;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 16:  //����һ��ʱ��
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TI01;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 17:  // С�����ӵظ澯Ͷ��  0�˳�,1�澯
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			bvalue = 1;
			if(pprRunSet->bXDLYB)
				bvalue = 1;
			else
				bvalue = 0;	
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 18:  //�����ѹ��ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_3U0;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 19: //С�����ӵ���ʱ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TDXJD;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 20: //С�����ӵر�����ʽ 2Ϊ����3Ϊ����
			fParaInfo->type = USHORT_TYPE;
			fParaInfo->len = USHORT_LEN;
			if(pprRunSet->bXdlFx)
				svalue = 3;
			else
				svalue = 2;	
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
		case 21: //����һ�γ���Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI1Trip) //
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 22: //����һ�θ�ѹ����              ��ѹ������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bIDYBS) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);		
			break;
		case 23: //����һ�η������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bIDir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 24: //�������γ���Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2Trip) //
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 25: //�������θ�ѹ����             ��ѹ������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2DYBS) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 26: //�������ξ��������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI2Dir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 27: //����һ�γ���Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI0Trip)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 28:// ��������Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb&PR_YB_JS)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 29:  //�������ٶ�ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I0HJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 30:  //��������ʱ��
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_I0TJS;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 31:   //�����ξ��������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->bI0Dir) 
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 32: //С�����ӵس���Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			bvalue = 1;
			if(pprRunSet->bXDLTrip)
				bvalue = 1;
			else
				bvalue = 0;	
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 33: //Uab��ѹ����Ͷ��               ȱ��������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bGyGJ)&&(pprRunSet->dYb & PR_YB_GY))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 34: //Ucb��ѹ����Ͷ��               ȱ��������
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if((!pprRunSet->bGyGJ)&&(pprRunSet->dYb & PR_YB_GY))
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 35:  //��ѹ������ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_UGY;
			SetToFloat(bval,&fvalue);
			if(type ==0)
				fvalue = fvalue/pfdcfg->Un;
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 36:  //��ѹ������ʱ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGY;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 37:  //��Ƶ����Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_GP)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 38:  //��Ƶ������ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_FGP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 39:  //��Ƶ������ʱ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TGP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 40:  //��Ƶ����Ͷ��
			fParaInfo->type = BOOL_TYPE;
			fParaInfo->len = BOOL_LEN;
			if(pprRunSet->dYb & PR_YB_DP)
				bvalue = 1;
			else
				bvalue = 0;
			memcpy(fParaInfo->valuech,&bvalue,fParaInfo->len);
			break;
		case 41:  //��Ƶ������ֵ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_FDP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		case 42: //��Ƶ������ʱ
			fParaInfo->type = FLOAT_TYPE;
			fParaInfo->len = FLOAT_LEN;
			bval = tset + SET_TDP;
			SetToFloat(bval,&fvalue);
			memcpy(fParaInfo->valuech,&fvalue,fParaInfo->len);
			break;
		default:
			fParaInfo->type = 4;
			fParaInfo->len = 0;
			svalue = 0;
			memcpy(fParaInfo->valuech,&svalue,fParaInfo->len);
			break;
	}
	return OK;
#else
	return ERROR;
#endif	
}

int WritePrParaYZ(WORD parano,char *pbuf)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	return CheckParaType_Len(fParaInfo->type,fParaInfo->len);
#else
	return ERROR;
#endif	
}

int WritePrPara(WORD parano,char *pbuf,WORD type)
{
#ifdef INCLUDE_PR
	struct VParaInfo *fParaInfo;
	BOOL bvalue;
	WORD svalue,fd;
	DWORD dvalue; 
	float fvalue;
	DWORD yb_kg1_kg2;
//	VPrRunInfo *pInfo;
//	VPrRunSet *pprRunSet;
//	VPrSetPublic  *pprPubRunSet;
	TSETVAL *tset;
	TSETVAL *bval;
	VFdCfg *pfdcfg;
	fParaInfo = (struct VParaInfo*) pbuf;
	if((parano < PRPARA_ADDR) || (parano > PRPARA_LINE_ADDREND))
		return 1;
	
	bvalue = svalue = fvalue = dvalue = fd =0;
	
	parano = parano - PRPARA_ADDR + 1;
	if(parano < 5)
	{
		if(parano == 1)
		{

			memcpy(&SNnum,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			
return OK;
		}
		else
			return OK;
	}
		
	fd = (parano - 5)/PARAFDNUM;
	parano = parano - fd*PARAFDNUM;
	pfdcfg = g_Sys.MyCfg.pFd+fd;
	tset = (TSETVAL*)(prSet[fd+1]); 
	
	switch(parano)
	{
		case 5: //����һ�θ澯Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			if(svalue == 1)
				yb_kg1_kg2 |= PR_YB_I1;
			else
				yb_kg1_kg2 &= ~PR_YB_I1;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 6: //����һ�ζ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_I1;
			FloatToSet(fvalue,bval);
			break;
		case 7: //����һ��ʱ��
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_T1;
			FloatToSet(fvalue,bval);
			break;
		case 8: //�������θ澯Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			if(svalue == 1)
				yb_kg1_kg2 |= PR_YB_I2;
			else
				yb_kg1_kg2 &= ~PR_YB_I2;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 9: //�������ζ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_I2;
			FloatToSet(fvalue,bval);
			break;
		case 10: //��������ʱ��
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_T2;
			FloatToSet(fvalue,bval);
			break;
		case 11: //�����ɸ澯Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG1_GFH_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			if(svalue == 1)
				yb_kg1_kg2 |= PR_YB_GFH;
			else
				yb_kg1_kg2 &= ~PR_YB_GFH;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 12: //�����ɶ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_IGFH;
			FloatToSet(fvalue,bval);
			break;
		case 13: //�����ɸ澯ʱ��
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 600000) //600s
				return 3;
			bval = tset + SET_TGFH;
			FloatToSet(fvalue,bval);
			break;
		case 14: //����һ�θ澯Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			if(svalue == 1)
				yb_kg1_kg2 |= PR_YB_I01;
			else
				yb_kg1_kg2 &= ~PR_YB_I01;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 15:  //����һ�ζ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_3I01;
			FloatToSet(fvalue,bval);
			break;
		case 16:  //����һ��ʱ��
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_TI01;
			FloatToSet(fvalue,bval);
			break;
		case 17:  //С�����ӵظ澯
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 &= ~PR_CFG2_XDL_TRIP;
			if(svalue == 1)
				yb_kg1_kg2 |= PR_CFG2_XDL_YB;
			else
				yb_kg1_kg2 &= ~PR_CFG2_XDL_YB;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
			break;		
		case 18: //��ѹ��ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_3U0;
			FloatToSet(fvalue,bval);
			break;
		case 19: //�ӵ���ʱ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_TDXJD;
			FloatToSet(fvalue,bval);
			break;
		case 20: //�ӵر�����ʽ
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(svalue == 2)
				yb_kg1_kg2 &= ~PR_CFG2_XDL_DLFX;
			else
				yb_kg1_kg2 |= PR_CFG2_XDL_DLFX;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
			break;		
		case 21: //����1�γ���Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(svalue)
				yb_kg1_kg2 |= PR_CFG1_I1_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I1_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			yb_kg1_kg2 |= PR_YB_I1;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 22: //����һ�ξ���ѹ����   
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I_DY;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I_DY;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			if(bvalue)
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_DY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			break;
		case 23: //����һ�ξ��������
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I_DIR;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I_DIR;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 24: //����2�γ���Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(svalue)
				yb_kg1_kg2 |= PR_CFG1_I2_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I2_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			yb_kg1_kg2 |= PR_YB_I1;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);			
			break;
		case 25: //�������ξ���ѹ����   
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I2_DY;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I2_DY;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			if(bvalue)
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_DY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			break;
		case 26: //�������ξ��������
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I2_DIR;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I2_DIR;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 27: //����һ�γ���
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I0_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I0_TRIP;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			yb_kg1_kg2 |= PR_YB_I01;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 28: //��������
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			caSetBuf = prSet[fd + 1];
			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			if(svalue)
				yb_kg1_kg2 |= PR_YB_JS;
			else
				yb_kg1_kg2 &= ~PR_YB_JS;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 29: //��������ٶ�ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100A
				return 3;
			bval = tset + SET_I0HJS;
			FloatToSet(fvalue,bval);
			break;
		case 30: //���������ʱ��
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_I0TJS;
			FloatToSet(fvalue,bval);
			break;
		case 31:   //�����ξ��������
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd + 1];
			CvtK(SET_KG1, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG1_I0_DIR;
			else
				yb_kg1_kg2 &= ~PR_CFG1_I0_DIR;
			ReverseKSet(SET_KG1,caSetBuf,yb_kg1_kg2);
			break;
		case 32: //С�����ӵس���Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			yb_kg1_kg2 |= PR_CFG2_XDL_YB;
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_XDL_TRIP;
			else
			{
				yb_kg1_kg2 &= ~PR_CFG2_XDL_TRIP;
				yb_kg1_kg2 &= ~PR_CFG2_XDL_YB;
			}
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
			break;
		case 33: //Uab��ѹ����Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_GY_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_GY_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			if(bvalue)
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_GY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			else
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 &= ~PR_YB_GY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			break;
		case 34: //Ucb��ѹ����Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_GY_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_GY_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			if(bvalue)
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 |= PR_YB_GY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			else
			{
				CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
				yb_kg1_kg2 &= ~PR_YB_GY;
				ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			}
			break;
		case 35:  //��ѹ������ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if(type ==0)
				fvalue = fvalue*pfdcfg->Un;
			dvalue = fvalue*1000;
			if(dvalue > 400000) //400V
				return 3;
			bval = tset + SET_UGY;
			FloatToSet(fvalue,bval);
			break;
		case 36:  //��ѹ������ʱ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //100s
				return 3;
			bval = tset + SET_TGY;
			FloatToSet(fvalue,bval);
			break;
		case 37:  //��Ƶ����Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_GF_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_GF_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			if(bvalue)
				yb_kg1_kg2 |= PR_YB_GP;
			else
				yb_kg1_kg2 &= ~PR_YB_GP;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		
		case 38:  //��Ƶ������ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //250V
				return 3;
			bval = tset + SET_FGP;
			FloatToSet(fvalue,bval);
			break;
		case 39:  //��Ƶ������ʱ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //
				return 3;
			bval = tset + SET_TGP;
			FloatToSet(fvalue,bval);
			break;
		case 40:  //��Ƶ����Ͷ��
			if((fParaInfo->type != BOOL_TYPE)||(fParaInfo->len != BOOL_LEN))
				return 2;
			memcpy(&svalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			if((svalue != 0) && (svalue != 1))
				return 3;
			
			bvalue = svalue;
			caSetBuf = prSet[fd+1];
			CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
			if(bvalue)
				yb_kg1_kg2 |= PR_CFG2_DF_TRIP;
			else
				yb_kg1_kg2 &= ~PR_CFG2_DF_TRIP;
			ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);

			CvtYb(SET_YB, &yb_kg1_kg2, PR_SET_FD);		//��ҪͶѹ��
			if(bvalue)
				yb_kg1_kg2 |= PR_YB_DP;
			else
				yb_kg1_kg2 &= ~PR_YB_DP;
			ReverseKSet(SET_YB,caSetBuf,yb_kg1_kg2);
			break;
		case 41:  //��Ƶ������ֵ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) //
				return 3;
			bval = tset + SET_FDP;
			FloatToSet(fvalue,bval);
			break;
		case 42: //��Ƶ������ʱ
			if((fParaInfo->type != FLOAT_TYPE) || (fParaInfo->len != FLOAT_LEN))
				return 2;
			memcpy(&fvalue,(BYTE*)fParaInfo->valuech,fParaInfo->len);
			dvalue = fvalue*1000;
			if(dvalue > 100000) 
				return 3;
			bval = tset + SET_TDP;
			FloatToSet(fvalue,bval);
			break;
		default:
			
			break;
	}
	return OK;
#else
	return ERROR;
#endif	
}
#endif
#endif

int WritePrParaFile()
{
#ifdef INCLUDE_PR
	WORD fdno,len,i;
	struct VFileHead *head;
	BYTE *wrSet;
	char fname[MAXFILENAME];
	
	if(g_prInit == 0) return ERROR;
	fdno = g_Sys.MyCfg.wFDNum;
	if(fdno == 0) return ERROR;
	
	//��������
	{
		caSetBuf = prSet[0];
		if(prPubSetConv(0) != SET_OK) return ERROR;
		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		wrSet = (BYTE*)(head+1);
		len = sizeof(TTABLETYPE);
		memcpy(wrSet,&tSetType,len);
		memcpy(wrSet+len,prSet[0],SET_PUB_NUMBER*sizeof(TSETVAL));
		
		if(WriteParaFile("fdPubSet.cfg",(struct VFileHead*)prSetTmp) == ERROR)
		{
			return ERROR;
		}
		if(prRunPublic->set_no == 0)
			prRunPublic->set_no =1;
		else
			prRunPublic->set_no = 0;
	}
	//���߲���
	for(i=0;i<fdno;i++)
	{
		caSetBuf = prSet[i+1];
		if(prFdSetConv(i) != SET_OK) return ERROR;
		
		head = (struct VFileHead *)prSetTmp;	
		head->nLength = sizeof(struct VFileHead)+PR_SET_SIZE;
		head->wVersion = CFG_FILE_VER;
		head->wAttr = CFG_ATTR_NULL;
		wrSet = (BYTE*)(head+1);
		len = sizeof(TTABLETYPE);
		memcpy(wrSet,&tSetType,len);
		memcpy(wrSet+len,prSet[i+1],SET_NUMBER*sizeof(TSETVAL));
		
		sprintf(fname, "fd%dset.cfg", i+1);
		if (WriteParaFile(fname, (struct VFileHead *) prSetTmp) == ERROR)
		{
			return ERROR;
		}		
		prFdSetNoSwitch(i);
	}
	
	return OK;
#else
	return ERROR;
#endif
}

BOOL ReadGLLB(WORD fd)	
{
#ifdef INCLUDE_PR
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	if(fd >= fdNum) 
		return 0;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no] + fd;
	if(pprRunSet->bRcdGl)
		return 1;
	else
#endif
		return 0;
}
void WriteGLLB(WORD fd, BOOL gllb)
{
#ifdef INCLUDE_PR
	DWORD yb_kg1_kg2 =0;
	if(fd >= fdNum) 
		return ;
	caSetBuf = prSet[fd+1];
	CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
	if(gllb)
		yb_kg1_kg2 |= PR_CFG2_RCD_GL;
	else
		yb_kg1_kg2 &= ~PR_CFG2_RCD_GL;
	ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
#endif
}
BOOL ReadWYLB(WORD fd)	
{
#ifdef INCLUDE_PR
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	if(fd >= fdNum) 
		return 0;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no] + fd;
	if(pprRunSet->bRcdSy)
		return 1;
	else
#endif
		return 0;
}
void WriteWYLB(WORD fd, BOOL wllb)
{
#ifdef INCLUDE_PR
	DWORD yb_kg1_kg2 =0;
	if(fd >= fdNum) 
		return ;
	caSetBuf = prSet[fd+1];
	CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
	if(wllb)
		yb_kg1_kg2 |= PR_CFG2_RCD_SY;
	else
		yb_kg1_kg2 &= ~PR_CFG2_RCD_SY;
	ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
#endif
}
BOOL ReadI0LB(WORD fd)	
{
#ifdef INCLUDE_PR
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	if(fd >= fdNum) 
		return 0;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no] + fd;
	if(pprRunSet->bRcdI0TB)
		return 1;
	else
#endif
		return 0;
}
void WriteI0LB(WORD fd, BOOL i0lb)
{
#ifdef INCLUDE_PR
	DWORD yb_kg1_kg2 = 0;
	if(fd >= fdNum) 
		return ;
	caSetBuf = prSet[fd+1];
	CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
	if(i0lb)
		yb_kg1_kg2 |= PR_CFG2_RCD_I0TB;
	else
		yb_kg1_kg2 &= ~PR_CFG2_RCD_I0TB;
	ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
#endif
}
BOOL ReadU0LB(WORD fd)	
{
#ifdef INCLUDE_PR
	VPrRunInfo *pInfo;
	VPrRunSet *pprRunSet;
	if(fd >= fdNum) 
		return 0;
	pInfo = prRunInfo + fd;
	pprRunSet = prRunSet[pInfo->set_no] + fd;
	if(pprRunSet->bRcdU0)
		return 1;
	else
#endif
		return 0;
}
void WriteU0LB(WORD fd, BOOL u0lb)
{
#ifdef INCLUDE_PR
	DWORD yb_kg1_kg2 = 0;
	if(fd >= fdNum) 
		return ; 
	caSetBuf = prSet[fd+1];
	CvtK(SET_KG2, &yb_kg1_kg2, PR_SET_FD);
	if(u0lb)
		yb_kg1_kg2 |= PR_CFG2_RCD_U0;
	else
		yb_kg1_kg2 &= ~PR_CFG2_RCD_U0;
	ReverseKSet(SET_KG2,caSetBuf,yb_kg1_kg2);
#endif
}


// ��ȡ����һ��ֵ ��������ʱ
long ReadIVIT(int no,int fd)
{
	long ycvalue = 0;
#ifdef INCLUDE_PR
	TSETVAL *tset,*bval;
	float fvalue;
	if((g_prInit == 1) && (fd < fdNum))
	{
		tset = (TSETVAL*)(prSet[fd+1]);

		switch(no)
		{
			case 0: //���򱣻���ʱ
				bval = tset + SET_TI01;
				SetToFloat(bval,&fvalue);
				ycvalue = fvalue*1000;
				break;
			case 1: //���򱣻�������ֵһ��ֵ
				bval = tset + SET_3I01;
				SetToFloat(bval,&fvalue);
			  ycvalue = fvalue*pFdCfg[fd].ct0/pFdCfg[fd].In0;
				break;
			case 2: //������ʱ �ٶ�
				bval = tset + SET_T1;
				SetToFloat(bval,&fvalue);
				ycvalue = fvalue*1000;
				break;
			case 3: //������ֵһ��ֵ (�ٶ�)
				bval = tset + SET_I1;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*pFdCfg[fd].ct/pFdCfg[fd].In;
				break;
			case 4: //����һ�α�����ֵ(����ֵ)
				bval = tset + SET_I1;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
			  	break;
			case 5:  //�������ζ�ֵ
				bval = tset + SET_I2;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 6: //����������ʱ
				bval = tset + SET_T2;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 7: //�������ζ�ֵ
				bval = tset + SET_I3;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 8: //����������ʱ
				bval = tset + SET_T3;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 9: //���������ֵ
				bval = tset + SET_3I01;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 10: //һ���غ�ʱ��
				bval = tset +  SET_TCH1;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 11: //�غϷŵ�ʱ��
				bval = tset +  SET_TCH1;
				SetToFloat(bval,&fvalue);
				ycvalue = fvalue*1000;
				bval = tset +  SET_TCHBS;
				SetToFloat(bval,&fvalue);
				ycvalue += fvalue*1000;
				ycvalue += 9*1000; //�ŵ�ʱ���9��
				break;
			case 12: //����ѹ
				bval = tset + SET_UGY;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 13: //����ѹʱ��
				bval = tset + SET_TGY;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;

			case 14: //��Ƶ��ֵ
				bval = tset + SET_FGP;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;
			case 15: //��Ƶʱ��
				bval = tset + SET_TGP;
				SetToFloat(bval,&fvalue);
			  	ycvalue = fvalue*1000;
				break;

			default:
				break;
		}
	}
#endif
	return ycvalue;
}

