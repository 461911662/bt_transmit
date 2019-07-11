#ifndef _io_base_h_
#define _io_base_h_

#include "define.h"


#define		SET  			1
#define		RESET   	0

#define   enabled   1
#define   disabled  0


// OE  - 0:input    1:output
// PUN - 1:HZ       0:Pull-up
// WUn - 0:wakeup 1:No Wakeup

//马达 1， 2
#define		  MOTOR1		      P0_3
#define     MOTOR2                P0_5
#define     MOTOR(x)        ((x == 1)? MOTOR1 : MOTOR2)

//工作指示灯
#define		LED_WORK_GREEN		  P0_1


typedef enum CONTROLTYPE { //马达控制

    left = 0,         //左马达
    sync,             //同步
    right             //右马达
} emControlType;

typedef enum SHOCKTYPE {
    nc = 0,              //无
    soft,                //轻柔
    ripple,              //水波
    slow,                //微按
    strong,              //强震
    dynamism             //动感
} emShockType;

typedef struct CONIFGPARAM {
    uint16_t           ClkTickCtr;         //单位时间 100us
    emControlType      ControlType;                  //马达控制方式
    uint8_t            Motors_duty_cycle;            //马达工作周期
    emShockType        ShockType;                    //震动方式
    uint8_t            Vibration_Stand;              //动感模式判断位
} stConfigParam;

#endif