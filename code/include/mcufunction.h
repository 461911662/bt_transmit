/*****************************************************************************
**               AMICCOM Electronics Corporation Document                   **
**          Copyright (c) 2011-2014 AMICCOM Electronics Corporation         **
**                                                                          **
**      A3,1F,No.1, Li-Hsin 1th Road, Science_Based Industrid Park,         **
**                       Hsin_Chu City, 300, Taiwan, ROC.                   **
**                 Tel: 886-3-5785818   Fax: 886-3-5785819                  **
**         E-mail:info@amiccom.com.tw  http: //www.amiccom.com.tw           **
*****************************************************************************/
#ifndef _MCUFUNCTION_H_
#define _MCUFUNCTION_H_

extern bit isIntoSleep(void);
extern void setIntoSleepFlag(bit enable);
extern bit IntoSleepFlag;
extern bit RecoveryFlag;

extern void InitMCU(void);
extern void En_Sleep(void);
extern void En_P30_Wakeup_Enable(void);
extern void En_P30_Wakeup_Init(void);
extern void En_Recovery(void);

#endif
