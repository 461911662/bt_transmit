/*****************************************************************************
**               AMICCOM Electronics Corporation Document                   **
**          Copyright (c) 2011-2014 AMICCOM Electronics Corporation         **
**                                                                          **
**      A3,1F,No.1, Li-Hsin 1th Road, Science_Based Industrid Park,         **
**                       Hsin_Chu City, 300, Taiwan, ROC.                   **
**                 Tel: 886-3-5785818   Fax: 886-3-5785819                  **
**         E-mail:info@amiccom.com.tw  http: //www.amiccom.com.tw           **
*****************************************************************************/
#ifndef _PROPRIETARY_H_
#define _PROPRIETARY_H_

/*--------------------------------------------------------------------------*/
/* INCLUDE FILE                                                             */
/*--------------------------------------------------------------------------*/
#include "..\include\define.h"

/*--------------------------------------------------------------------------*/
/* CONSTANT DECLARATION                                                     */
/*--------------------------------------------------------------------------*/

#define FUN_PROPRIETARY

/*--------------------------------------------------------------------------*/
/* MACRO DEFINITION                                                         */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* TYPE DEFINITION                                                          */
/*--------------------------------------------------------------------------*/
typedef enum
{
    PR_SUCCESS,
    PR_ERR_HANDLE,
    PR_ERR_RECALL,
    PR_ERR_NOINIT,
    PR_ERR_LACKOFTIME,
    PR_ERR_USING,
    PR_ERR_CHANNEL,
    PR_ERR_ID,
    PR_ERR_DATARATE,
    PR_ERR_TXLEN,
    PR_ERR_RXLEN,
    PR_ERR_TIMEOUT,
    PR_ERR_CRC,
}TE_PR_Code;

typedef enum
{
    DR500K_DV186K,  /*<< For A7105, Data rate <=500K, Deviation = 250K */
    DR1MB_DV250K,   /*<< Data rate < 1M, Deviation = 250K */
    DR1M_DV250K,    /*<< Data rate = 1M, Deviation = 250K */
    //DR2M_DV500K     /*<< Data rate = 2M, Deviation = 500K */
}TE_PR_DR_Item;

typedef struct
{
    TE_PR_DR_Item   Prop_DRI;       /*<< Data rate item */
    uint8_t         Prop_SDR;       /*<< Setting "Data Rate = 1M/(Prop_SDR+1)", used in DR500K_DV186K and DR1MB_DV250K item */
    uint8_t         Prop_Ext;       /*<< RUF */
}TS_PR_DataRate;

typedef struct
{
    uint8_t*        Prop_ID;        /*<< The ID length must is four byte */
    TS_PR_DataRate  Prop_DR;        /*<< Data Rate = 1M/(Prop_DR + 1)) */
	uint8_t         Prop_TxWait;    /*<< Tx waitting (Unit: Prop_TxWait * 1us) */
    uint8_t         Prop_RxWS;      /*<< Rx window size (Unit: Prop_RxWS * 1ms) */
}TS_PR_Param;

/*--------------------------------------------------------------------------*/
/* LOCAL VARIABLES DEFINITION                                               */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTION PROTOTYPE                                                 */ 
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS DEFINITION                                               */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTANT DECLARATION                                                     */
/*--------------------------------------------------------------------------*/
#define     RF_RATE                 1000
#define     RF_MAXCHANNEL           0x50
#define     RF_ID_LEN               0x04
#define     RF_ID_BUF_SIZE          (RF_ID_LEN << 1)
#define     RF_MIN_TRX_LEN          0x01
#define     RF_MAX_TRX_LEN          0x40

#define     RF_ADV_RESERVE          0x0A // 0.15625ms*10 = 1.56ms
#define     RF_CON_RESERVE          0x23 // 0.15625ms*35 = 5.5ms
#define     RF_MAX_RESERVE          RF_CON_RESERVE // 0.15625ms*35 = 5.5ms

/*--------------------------------------------------------------------------*/
/* TYPE DEFINITION                                                          */
/*--------------------------------------------------------------------------*/
typedef enum
{
    MODE_BLE,
    MODE_PROP
}TE_Prop_Mode;

typedef struct
{
    uint8_t         RF_InUse;
    uint8_t         RF_Start;
    uint8_t         RF_WaitFlag;
    TE_Prop_Mode    RF_Mode;
    uint8_t         RF_Channel;
    uint8_t         RF_ID[RF_ID_BUF_SIZE];
    uint8_t         RF_SDR;
    uint8_t         RF_DEV;
    uint32_t        RF_TxTimes;
    uint16_t        RF_TxWait;
    uint8_t         RF_RxWS;
}_TS_Prop_Param;

/*--------------------------------------------------------------------------*/
/* LOCAL VARIABLES DEFINITION                                               */
/*--------------------------------------------------------------------------*/

//static _TS_Prop_Param    g_TSPropParm;
extern _TS_Prop_Param    g_TSPropParm;
extern uint8_t  xdata    PR_Timer1Flag;
//extern uint8_t  xdata    PR_Timer1Flag;

/*--------------------------------------------------------------------------*/
/* GLOBAL VARIABLES DEFINITION                                              */
/*--------------------------------------------------------------------------*/

#ifdef FUN_PROPRIETARY
extern uint8_t xdata g_RxCnt;
#endif

/*--------------------------------------------------------------------------*/
/* GLOBAL FUNCTIONS DEFINITION                                              */ 
/*--------------------------------------------------------------------------*/

#ifdef FUN_PROPRIETARY
void Prop_Initialize(void);
TE_PR_Code Prop_Create(TS_PR_Param*, void**);
TE_PR_Code Prop_Delete(void**);
TE_PR_Code Prop_Start(void*, uint8_t);
TE_PR_Code Prop_Stop(void*);
uint8_t Prop_RunTx(void* pvHandle, uint8_t* u8TxBuf, uint8_t u8TxLen);
uint8_t Prop_RunRx(void* pvHandle, uint8_t* pu8RxBuf, uint8_t u8RxLen);
void DisableRxWinS(void);

uint8_t Prop_GetRFMode(void);
extern void EnablePRTimer1(void);
extern void DisablePRTimer1(void);
extern void Timer1PRInit(void);
extern uint16_t Get_Interval_Time(void);
extern uint16_t Get_Interval_Time_Left(void);

#endif

#endif