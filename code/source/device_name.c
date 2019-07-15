#define _DEVICE_NAME_C_
#include "define.h"

//DEFINE_DEVICE_NAME[0] = Data Length
//DEFINE_DEVICE_NAME[1] ~ DEFINE_DEVICE_NAME[15] = Data

const uint8_t code DEFINE_DEVICE_NAME[16] =
{
    0x0C,	'D',	'e',	'm',	'o',	' ',	'U',	's',
    'e',	'r',	'd',	'e',	'f',	0x00,	0x00,	0x00,		//Demo Keyboard
};
