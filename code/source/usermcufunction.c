#include "usermcufunction.h"
#include "A8107.h"
#include "define.h"
#include "mcufunction.h"
#include "LibFunction.h"

extern void disable_timer1(void);

uint8_t gaucBLE_ADDRESS[BLE_MACADDRESS_SIZE] = {0};

bit IntoSleepFlag = FALSE;

void reset(void)
{
    _nop_();
}

/*********************************************************************
** InitMCU
*
******************************************************************************
**  ROUTINE NAME: initMcu                                                   **
**  I/O define  :                                                           **
**     Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   | **
**  Port 0|  P07  |  P06  |  P05  |  P04  |  P03  |  P02  |  P01  |  P00  | **
**   I/O  | undef | IO(O) | KEY(I)| undef | KEY(I)| KEY(I)| KEY(I)| KEY(I)| **
**  Port 1|  P17  |  P16  |  P15  |  P14  |  P13  |  P12  |  P11  |  P10  | **
**   I/O  | undef | undef | undef | undef | undef | undef | undef | undef | **
**  Port 3|  P37  |  P36  |  P35  |  P34  |  P33  |  P32  |  P31  |  P30  | **
**   I/O  | undef | undef | undef | undef | undef | undef | Tx(O) | Rx(I) | **
******************************************************************************
*1.初始化P00(Vol-) P01(Vol+) P02(video) P03(invert) P05(capture)按键
*2.初始化串口引脚 P30 P31
*3.初始化系统时钟，使能引脚功能
*4.清除RSFLAG寄存器标志
*********************************************************************/
void initKey(void)
{
    /*
    ** Port 0/1/2
    ** OE  - 0:input     1:output
    ** PUN - 0:Pull-up   1:HZ
    ** WUn - 0:Wakeup    1:No Wakeup
    */
    P0 = PWUP_P0;
    P0OE = PWUP_P0OE; //P06:输出
    P0PUN = PWUP_P0PUN; //0x2F;输入浮空
    P0WUN |= ~(PWUP_P0WUN); //0xFF;引脚不唤醒

    EIE |= EKEYINT; /* 使能外部按键中断 */
    EIP |= EKEYPRI; /* 外部中断优先级高 */
    EIF |= CLEAR_KEYINTFLAG; /* 清除中断标志位 */    
}

void initUart(void)
{

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
