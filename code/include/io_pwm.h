#ifndef _io_pwm_h_
#define _io_pwm_h_

#include "define.h"



extern void reset(void);
extern void pwm_init(void);
extern void pwm_deal(void); 
extern void sub_function(void);

extern void enable_timer1(void);
extern void disable_timer1(void);

extern void initIsr_timer1(void);
extern void update_init(void);

extern void BLE_SendDataPacket(uint8_t *sendBuf ,uint8_t sendSize);
extern void UserDataHandler_Connection(void);
extern void HandlerData(uint8_t *Buf);
extern void timer_detect(void);
extern void connected_init(void);
#endif








