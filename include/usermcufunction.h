#ifndef _USERMCUFUNCTION_H_
#define _USERMCUFUNCTION_H_
#include "define.h"

#define IAPLAT_BT_CAMERA
#define BT_CUSTOMER_NAME
#define _RESPONSE_USER_
#define _PROFILE_HOGP_USER_DEMO_

#define EKEYINT         0x10 /* 按键中断使能 */
#define EKEYPRI         0x10 /* 按键中断优先级1 */
#define CLEAR_KEYINTFLAG      0x10 /* 外部中断标志位 */

#define PWUP_P0         0x00 /* low level / port wake up 端口唤醒 */
#define PWUP_P0OE       0x40 /* 00:input 01:output */
#define PWUP_P0PUN      (0xD0) /* 0(bit):pull up 1(bit):HZ */
#define PWUP_P0WUN      (0xD0) /* port wake up */

#define BLE_MACADDRESS_SIZE  6
#define BLE_POWERSAVE_LEVEL (0x2343C) /* 1s~3s */

typedef struct BLE_CMDREQUEST
{
    uint8_t head; /* 0xAA:有子命令,0x55:无子命令 */
    uint8_t length;
    uint8_t cmd;
    uint8_t subcmd;
    uint8_t payload[15];
    uint8_t CheckSum;
}BLE_CMD_Req_S;

typedef enum{
    BLE0_CODE_FIRST=0,
    BLE0_CODE_SLEEP,
    BLE0_CODE_LONG_PRESSED,
    BLE0_CODE_GET_BATTERY,
    BLE0_CODE_OSTYPE,
    BLE0_CODE_END,
}BLE_CODE0_E;

typedef enum{
    BLE_MASTER_OSTYPE_FIRST=0,
    BLE_MASTER_OSTYPE_IOS,
    BLE_MASTER_OSTYPE_ANDROID,
    BLE_MASTER_OSTYYPE_WINDOWSPHONE,
    BLE_MASTER_OSTYPE_END,
}BLE_MASTER_OSTYPE_E;

typedef enum{
    LED_FLAG_NOBLINK=0,
    LED_FLAG_BLINK_1S,
    LED_FLAG_END,
}LED_FLAG_BLINK_E;

#undef KYE_DEBUG

extern void userInitKey(void);
extern void userInitUart(void);
extern void userInitUart0_timer2(void);
extern void reset(void);
extern void entrySleep(void);
extern void userReMCUfun(void);

#endif
