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

/*********************************************************************
** Global Variables
*********************************************************************/
uint8_t xdata		auth_req;
uint8_t xdata		init_req;
uint8_t xdata		index;
uint8_t xdata		auth_init_finish;
uint8_t xdata		scale_index;
uint8_t xdata		KeyWakeup;
uint8_t xdata		scalebuf[10];
uint16_t xdata	weightvalue;
uint32_t xdata	datevalue;
uint8_t xdata	sendscaledata;
uint8_t xdata	waitpacket;
//////////////////TWOR///////////////////////
uint8_t xdata	TworTimer;
///////////////////TWOR//////////////////////


static const char ascii[] = "0123456789ABCDEF";

void En_P30_Wakeup_Init(void);
void En_P30_Wakeup_Enable(void);

bit IntoSleepFlag = FALSE;
bit RecoveryFlag  = FALSE;

const uint8_t code send_scale_data[60]=
{
		0x03,0x00,0x34,0xFE,0x01,0x00,0x34,0x27,
		0x12,0x00,0x00,0x0A,0x00,0x12,0x26,0x43,
		0x33,0x46,0x45,0x30,
		0x30,0x30,0x36,0x45,0x30,0x35,0x35,0x44,
		0x36,0x42,0x34,0x43,0x41,0x46,0x46,0x30,
		0x30,0x30,0x30,0x30,
		0x30,0x46,0x46,0x30,0x30,0x30,0x30,0x30,
		0x30,0x30,0x30,0x30,0x39,0x18,0x00,0x00,
		0x00,0x00,0x00,0x00
};


void InitMCU(void);
/*********************************************************************
** InitMCU
*********************************************************************/
void InitMCU(void)
{
 //   P0 	  = 0xD5;
 //   P0OE  = 0xAF;
 //   P0PUN = 0xAF;
 //   P0WUN = 0xFF;
    P0 = ~0x2a;
    P0OE = 0x2a;
    P0PUN = 0x2a;
    P0WUN = 0xFF;
    
    P1 	  = 0xFF;
    P1OE  = 0x0C;
    P1PUN = 0x0C;
    P1WUN = 0xFF;
    
    P3 	  = 0xFF;
    P3OE  = 0x00;
    P3PUN = 0x03;
    P3WUN = 0xFF;
    
    /*********************************************/
    /*    MCU Frequency setting (Dont Modify)    */
    PCONE |= 0x01;  //SYSCLK = 16MHz / 2 = 8MHz  */
    PCON  |= 0x01;  //Enable CKSE                */
    /*********************************************/
    RSFLAG = 0x07;
    check_stable = 0x5AA5;
}

bit isIntoSleep(void)
{
		return IntoSleepFlag;
}

void setIntoSleepFlag(bit enable)
{
		IntoSleepFlag = enable;
}

void En_Recovery(void)
{
    //ʹ��5s��ʱ
    Twor05Timer(ENABLE);
}

/**  ����*/
void En_Sleep(void)
{
	
			ADV_InitDef ADV_InitStructure;
	
		 //��ֹ�㲥
			ADV_InitStructure.ADV_RndEnable = DISABLE;
			ADV_InitStructure.ADV_TOEnable = DISABLE;
			ADV_InitStructure.ADV_Run = DISABLE;     // disable adv.
	
			BLE_ADV_Cmd(&ADV_InitStructure);
	
			// ��ֹ5s��ʱ
			Twor05Timer(DISABLE);
	
			En_P30_Wakeup_Init();
	
			//ʹ��p3.0���ⲿ�ж�
			En_P30_Wakeup_Enable();
	

		 //����
		 BLE_AutoPwrDown_Enable();
		 //clock 16M -> 8M
	
	
		 PCON |= 0x01;
}

/*ʹ���ⲿ�����ж�*/
void En_P30_Wakeup_Enable(void)
{
		P3WUN &= ~0x01;		// P3_0 ,Enable wakeup
		EIE |= 0x10;  
}

/** ��ʼ���ⲿ����*/
void En_P30_Wakeup_Init(void)
{
        //p3_1    -- ���͹���
        P3OE &= ~0x02;
		P3PUN &= ~0x02;
        P3 &= ~0x02;
    
		IOSEL &=~0x01;		// P3_0 use for normal IO
		P3OE &=~0x01;
		P3PUN &=~0x01;		// P3_0 pull up
	
		P3 |= 0x01;
	
  //      PCONE   &= ~0xc0;
		PCONE  |= 0x80;    // rising edge interrupt

		EIF |= 0x10;		// CLR KEYINT FLAG
}

void initIsr_timer1(void)
{
    CKCON=(CKCON&0xef)|0x10;//set timer1 clock
    TMOD=(TMOD&0x0F)|0x20;//timer1,mode2
//	TH0 = 56; //100us
//  TL0 = 56;
    TH1 = 56;
    TL1 = 56;
    TF1 = 0; // Clear any pending Timer1 interrupts
    ET1 = 1; // Enable Timer1 interrupt
    TR1 = 0;//timer0 enable
}

void enable_timer1(void)
{
	ET1 = 1; // Enable Timer0 interrupt
    TR1 = 1;//timer0 enable
}

void disable_timer1(void)
{
    ET1 = 0; 
    TR1 = 0;
}

/*********************************************************************
**  Prcss_Key
*********************************************************************/
#ifdef _PROFILE_TAOBAO_
void Prcss_Weight(void)
{
	uint8_t i,result,tmp;
			
	switch(scale_index)
  {
      case 0x00:
					for(i=0;i<20;i++)
							att_HDL_TAOBAO_DATI[i] = send_scale_data[i];
							
					att_HDL_TAOBAO_DATI[10]=scalebuf[0];
					scalebuf[0]++;
P0_1 = ~P0_1;
					result = BLE_SendData(att_HDL_TAOBAO_DATI,ATT_HDL_TAOBAO_DATI_INIT,20);
					if(result == SUCCESS)
						scale_index++;											
          break;

      case 0x01:
					for(i=0;i<20;i++)
							att_HDL_TAOBAO_DATI[i] = send_scale_data[i+20];
					att_HDL_TAOBAO_DATI[1]=scalebuf[1];
					att_HDL_TAOBAO_DATI[2]=scalebuf[2];
					att_HDL_TAOBAO_DATI[3]=scalebuf[3];
					att_HDL_TAOBAO_DATI[4]=scalebuf[4];
					att_HDL_TAOBAO_DATI[8]=scalebuf[5];
					att_HDL_TAOBAO_DATI[9]=scalebuf[6];
					att_HDL_TAOBAO_DATI[10]=scalebuf[7];
					att_HDL_TAOBAO_DATI[11]=scalebuf[8];
					att_HDL_TAOBAO_DATI[12]=scalebuf[9];

					result = BLE_SendData(att_HDL_TAOBAO_DATI,ATT_HDL_TAOBAO_DATI_INIT,20);
					if(result == SUCCESS)
					{

						weightvalue = weightvalue+100;

						tmp=toascii(weightvalue,3);
						scalebuf[1]=tmp;
						tmp=toascii(weightvalue,2);
						scalebuf[2]=tmp;						
						tmp=toascii(weightvalue,1); 
						scalebuf[3]=tmp;						
						tmp=toascii(weightvalue,0);
						scalebuf[4]=tmp;
						
						datevalue = datevalue+100;
						tmp=toascii(datevalue,4);
						scalebuf[5]=tmp;
						tmp=toascii(datevalue,3);
						scalebuf[6]=tmp;						
						tmp=toascii(datevalue,2);
						scalebuf[7]=tmp;						
						tmp=toascii(datevalue,1);
						scalebuf[8]=tmp;						
						tmp=toascii(datevalue,0);
						scalebuf[9]=tmp;	
											
						scale_index++;
					}
					break;
											
      case 0x02:
					for(i=0;i<20;i++)
							att_HDL_TAOBAO_DATI[i] = send_scale_data[i+20+20];
					result = BLE_SendData(att_HDL_TAOBAO_DATI,ATT_HDL_TAOBAO_DATI_INIT,20);
					if(result == SUCCESS)
					{
						scale_index++;
						KeyWakeup = 0;
          }
					break;

      default:
          break;
  }																	

}
#endif
/*********************************************************************
**  toascii
*********************************************************************/
uint8_t toascii(uint32_t in, uint8_t unit)
{
	uint8_t tmp;

	tmp = ascii[(in>>(unit*4))&0x0F];
	return tmp;
}

/////////////////TWOR//////////////////////////////////////////////
/*********************************************************************
**  Enable05Timer
*********************************************************************/
void Twor05Timer(uint8_t TimerEnable)
{
	if(TimerEnable == ENABLE)
	{
		TworTimer=1;
		A8107_RCSelect(internalRC);
		//XBYTE[GIO2_REG]=0x10;
		XBYTE[RCOSC1_REG]=0x1F;
		XBYTE[CKO_REG]=0x52;
		XBYTE[VCOC_REG]=0xE0;
		//EIE = EIE | 0x08;
	}
	else
	{
		TworTimer=0;
		A8107_RCSelect(internalRC);
		XBYTE[CKO_REG]=0x52;
	}
}
//////////////////TWOR///////////////////////////////////////////////