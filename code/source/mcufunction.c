/*****************************************************************************
**               AMICCOM Electronics Corporation Document                   **
**          Copyright (c) 2011-2014 AMICCOM Electronics Corporation         **
**                                                                          **
**      A3,1F,No.1, Li-Hsin 1th Road, Science_Based Industrid Park,         **
**                       Hsin_Chu City, 300, Taiwan, ROC.                   **
**                 Tel: 886-3-5785818   Fax: 886-3-5785819                  **
**         E-mail:info@amiccom.com.tw  http: //www.amiccom.com.tw           **
*****************************************************************************/
#include "define.h"
#include "A8107.h"
#include "LibFunction.h"
#include "usermcufunction.h"

/*********************************************************************
** InitMCU
*
******************************************************************************
**  ROUTINE NAME: initMcu                                                   **
**  I/O define  :                                                           **
**     Bit|   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   | **
**  Port 0|  P07  |  P06  |  P05  |  P04  |  P03  |  P02  |  P01  |  P00  | **
**   I/O  | undef | undef | KEY(I)| undef | KEY(I)| KEY(I)| KEY(I)| KEY(I)| **
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
void InitMCU(void);
/*********************************************************************
** InitMCU
*********************************************************************/
void InitMCU(void)
{
    uint16_t idata i;

    /*
    ** Port 0/1/2
    ** OE  - 0:input     1:output
    ** PUN - 0:Pull-up   1:HZ
    ** WUn - 0:Wakeup    1:No Wakeup
    */
#ifdef _USERMCUFUNCTION_H_
    P0 = PWUP_P0;
    P0OE = PWUP_P0OE;
    P0PUN = PWUP_P0PUN; //0x2F;输入浮空
    P0WUN |= ~(PWUP_P0WUN); //0xFF;引脚不唤醒

    P1    = 0xFF;
    P1OE  = 0x00;
    P1PUN = 0x00;
    P1WUN = 0xFF;

    P2    = 0xFF;
    P2OE  = 0x00;
    P2PUN = 0x00;
    P2WUN = 0xFF;

    P3    = 0xFF;
    P3OE  = 0x00;
    P3PUN = 0x00;
    P3WUN = 0xFF;
#else
    P0    = 0xFF;
    P0OE  = 0x00;
    P0PUN = 0x00;
    P0WUN = 0xFF;

    P1    = 0xFF;
    P1OE  = 0x00;
    P1PUN = 0x00;
    P1WUN = 0xFF;

    P2    = 0xFF;
    P2OE  = 0x00;
    P2PUN = 0x00;
    P2WUN = 0xFF;

    P3    = 0xFF;
    P3OE  = 0x00;
    P3PUN = 0x00;
    P3WUN = 0xFF;
#endif

    //clear XData RAM
    for (i = 0x0000; i < 0x2000; i++)
        XBYTE[i] = 0x00;

    /*********************************************/
    /*    MCU Frequency setting (Dont Modify)    */
    PCONE |= 0x01;  //SYSCLK = 16MHz / 2 = 8MHz  */
    PCON  = (PCON&0xEF) | 0x01;  //Enable CKSE   */
    RSFLAG = 0x07;  //CLEAR PORF,RESETNF         */
    /*********************************************/

    check_stable = 0x5AA5;

#ifdef _USERMCUFUNCTION_H_
    EIE |= EKEYINT; /* 使能外部按键中断 */
    EIP |= EKEYPRI; /* 外部中断优先级高 */
    EIF |= CLEAR_KEYINTFLAG; /* 清除中断标志位 */
#endif
}

