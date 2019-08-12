#include "debug.h"
#include "A8107.h"

uint8_t idata UartTxBuf[64];
uint8_t idata *Uartptr;
uint8_t idata UartSendCnt;
bit     idata UartTxtrasmitFlag;

int dprintf(const char *cc)
{
    uint32_t len = 0;
    if(UartTxtrasmitFlag)
    {
        return len;
    }
    
    memset((void*)UartTxBuf, 0, 64);
    strncpy(UartTxBuf, cc, 10);
    Uartptr = UartTxBuf;
    while(*Uartptr++)
    {
        len++;
    }
    Uartptr = UartTxBuf;
    if(64 <= len && UartTxBuf[63] != '\n')
    {
        //UartTxBuf[62] = '\r';
        UartTxBuf[63] = '\n';
    }
    
    if(64 > len && UartTxBuf[len-1] != '\n')
    {
        if(63 > len)
        {
            //UartTxBuf[len] = '\r';
            UartTxBuf[len++] = '\n';
        }
        else
        {
            //UartTxBuf[len-2] = '\r';
            UartTxBuf[len-1] = '\n';
        }
    }

    if(0 == len)
    {
        return len;
    }

    while(TB8); /* wait for a minute */
    
    SBUF = UartTxBuf[0];
    UartTxtrasmitFlag = 1;
    UartSendCnt = len;

	return len;
}

