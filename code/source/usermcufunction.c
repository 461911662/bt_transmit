#include "usermcufunction.h"
#include "A8107.h"
#include "define.h"
#include "mcufunction.h"
#include "LibFunction.h"

extern void disable_timer1(void);

bit IntoSleepFlag = FALSE;

void reset(void)
{
    _nop_();
}

void reMCUfun(void)
{
    /* 关外部中断 */
    InterruptDisable();

    /* 确认MUC配置 */
    P0 = PWUP_P0;
    P0OE = PWUP_P0OE;
    P0PUN = PWUP_P0PUN;
    P0WUN |= ~PWUP_P0WUN; /* 禁止引脚唤醒 */

    /*********************************************/
    /*    MCU Frequency setting (Dont Modify)    */
    PCONE |= 0x01;  //SYSCLK = 16MHz / 2 = 8MHz  */
    PCON  |= 0x01;  //Enable CKSE                */
    /*********************************************/

    EIE |= EKEYINT; /* 使能外部按键中断 */
    EIP |= EKEYPRI; /* 外部中断优先级高 */

    InterruptEnable();
}

/* 进入休眠模式 */
void entrySleep()
{
    /* 断开BLE连接 */
    if(ble_state == CONNECT_ESTABLISH_STATE)
    {
        BLE_SetTerminate();
    }

    /* 设置睡眠flag */
    IntoSleepFlag = TRUE;

    /* 设置引脚唤醒 */
    P0WUN = PWUP_P0WUN; /* P0_0 P0_1 P0_2 P0_3 P0_5 is wakeup */

    /* 失能定时器计时 */
    RF_Timer500ms(DISABLE);

    /* BLE休眠使能 */
    BLE_AutoPwrDown_Enable();
}