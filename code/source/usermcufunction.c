#include "usermcufunction.h"
#include "A8107.h"
#include "define.h"
#include "mcufunction.h"
#include "LibFunction.h"

extern void disable_timer1(void);

void reset(void)
{
    _nop_();
}

void reMCUfun(void)
{
    /* 关外部中断 */
    InterruptDisable();

    /* 确认MUC配置 */
    P0 = 0x00;
    P0OE = 0x00;
    P0PUN = 0x2F;
    P0WUN |= ~0xD0; /* 禁止引脚唤醒 */

    /*********************************************/
    /*    MCU Frequency setting (Dont Modify)    */
    PCONE |= 0x01;  //SYSCLK = 16MHz / 2 = 8MHz  */
    PCON  |= 0x01;  //Enable CKSE                */
    /*********************************************/

    EIE |= 0x10; /* 使能外部按键中断 */
    EIP |= 0x10; /* 外部中断优先级高 */

    InterruptEnable();
}

/* 进入休眠模式 */
void entrySleep()
{
    /* 断开BLE连接 */
    if(ble_state == CONNECT_ESTABLISH_STATE)
    {
        BLE_setTerminate();
    }

    /* 设置睡眠flag */
    setIntoSleepFlag(TRUE);

    /* 设置引脚唤醒 */
    P0WUN = 0xD0; /* P0_0 P0_1 P0_2 P0_3 P0_5 is wakeup */

    /* 失能定时器计时 */
    Twor05Timer(DISABLE);

    /* BLE休眠使能 */
    BLE_AutoPwrDown_Enable();
}