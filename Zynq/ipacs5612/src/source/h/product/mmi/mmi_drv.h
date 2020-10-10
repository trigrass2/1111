#ifndef  _MMI_H
#define _MMI_H

#define KEY_OPEN         0   	  // 分闸
#define KEY_CLOSE        1   	  // 合闸
#define KEY_RESET        2   	  // 复归
#define KEY_HAND         3   	  // 手动
#define KEYSCANTIME      10       //100ms

#define EV_PROTECT_RESET 0x01     //保护复归事件

void mmi_drv(void);

#endif
