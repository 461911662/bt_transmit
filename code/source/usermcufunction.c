#include "usermcufunction.h"

extern void disable_timer1(void);

void reset(void)
{
    disable_timer1();
}