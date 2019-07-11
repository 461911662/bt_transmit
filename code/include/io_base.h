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

//��� 1�� 2
#define		  MOTOR1		      P0_3
#define     MOTOR2                P0_5
#define     MOTOR(x)        ((x == 1)? MOTOR1 : MOTOR2)

//����ָʾ��
#define		LED_WORK_GREEN		  P0_1


typedef enum CONTROLTYPE { //������

    left = 0,         //�����
    sync,             //ͬ��
    right             //�����
} emControlType;

typedef enum SHOCKTYPE {
    nc = 0,              //��
    soft,                //����
    ripple,              //ˮ��
    slow,                //΢��
    strong,              //ǿ��
    dynamism             //����
} emShockType;

typedef struct CONIFGPARAM {
    uint16_t           ClkTickCtr;         //��λʱ�� 100us
    emControlType      ControlType;                  //�����Ʒ�ʽ
    uint8_t            Motors_duty_cycle;            //��﹤������
    emShockType        ShockType;                    //�𶯷�ʽ
    uint8_t            Vibration_Stand;              //����ģʽ�ж�λ
} stConfigParam;

#endif