#ifndef  _MMI_H
#define _MMI_H

#define KEY_OPEN         0   	  // ��բ
#define KEY_CLOSE        1   	  // ��բ
#define KEY_RESET        2   	  // ����
#define KEY_HAND         3   	  // �ֶ�
#define KEYSCANTIME      10       //100ms

#define EV_PROTECT_RESET 0x01     //���������¼�

void mmi_drv(void);

#endif
