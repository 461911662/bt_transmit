#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "define.h"

extern uint8_t idata UartTxBuf[64];
extern uint8_t idata *Uartptr;
extern uint8_t idata UartSendCnt;
extern bit     idata UartTxtrasmitFlag;

extern int dprintf(const char *cc);

#endif