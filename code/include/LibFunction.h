/*****************************************************************************
**               AMICCOM Electronics Corporation Document                   **
**          Copyright (c) 2011-2015 AMICCOM Electronics Corporation         **
**                                                                          **
**      A3,1F,No.1, Li-Hsin 1th Road, Science_Based Industrid Park,         **
**                       Hsin_Chu City, 300, Taiwan, ROC.                   **
**                 Tel: 886-3-5785818   Fax: 886-3-5785819                  **
**         E-mail:info@amiccom.com.tw  http: //www.amiccom.com.tw           **
*****************************************************************************/
#ifndef _LIBFUNCTION_H_
#define _LIBFUNCTION_H_

#include "servicegen.h"

/** @defgroup ADV_Channel 
* {
*/
typedef enum
{ 
    ADV_CH37 = 0x01,
    ADV_CH38 = 0x02,
    ADV_CH39 = 0x04,
}TE_ADVCHANNEL;
/**
* @}
*/

/** @defgroup ADV_Type 
* {
*/
#define ADV_IND            0x00
#define ADV_NONCONN_IND    0x02
#define ADV_SCAN_IND       0x06
/**
* @}
*/

typedef struct
{
    TE_ADVCHANNEL ADV_Channel;       /*!< Specifies the ADV_Channel.
                                          This parameter can be a value of @ref TE_ADVCHANNEL */
                                       
    uint8_t ADV_Type;                /*!< Specifies the ADV_Type.
                                          This parameter can be a value of @ref ADV_Type    */
                                    
    uint16_t ADV_Timing;             /*!< Specifies the ADV_Timing.
                                          ADV Interval = (20ms + 0.625ms*ADV_Timing), ADV_Timing: 0 ~ 15967 */
                                    
    uint16_t ADV_TO;                 /*!< Specifies the ADV_TO(Timeout).
                                          ADV Timeout = ADV Interval * ADV_TO, ADV_TO: 1 ~ 60000 */
    
    uint8_t ADV_RndEnable;           /*!< Specifies the ADV_RndEnable.
                                          If ADV_RndEnable = 1, ADV Interval = (20ms + 0.625ms*ADV_Timing) + random time(0~10ms) */
    
    uint8_t ADV_TOEnable;            /*!< Specifies the ADV_TOEnable(TimeoutEnable).
                                          If ADV_RndEnable = DISABLE, ADV Interval will change to MAX(1s or ADV Interval) after the ADV Timeout
                                          If ADV_RndEnable = ENABLE, ADV will stop after the ADV timeout counter*/

    uint8_t ADV_Run;                 /*!< Specifies the ADV_Run.
                                          If ADV_Run = DISABLE, ADV will stop at ADV mode.
                                          If ADV_Run = ENABLE, ADV will start at standby mode*/
}ADV_InitDef;

/*********************************************************************
** Lib Define 
*********************************************************************/
#define STANDBY_STATE                 0x00
#define ADV_STATE                     0x01
#define CONNECT_GO_STATE              0x02
#define CONNECT_ESTABLISH_STATE       0x03

/*********************************************************************
** Link Layer System Define
*********************************************************************/
//Error Code
#define SUCCESS                                                      0x00
#define ERR_ADV_CHANNEL                                              0x01
#define ERR_ADV_TYPE                                                 0x02
#define ERR_ADV_TIMING                                               0x03
#define ERR_ADV_TIMEOUT                                              0x04
#define ERR_ADV_RANDOM_DELAY_FAULT                                   0x05
#define ERR_ADV_TIMEOUT_CTRL_FAULT                                   0x06
#define ERR_ADV_RUN_CTRL_FAULT                                       0x07
#define ERR_NO_FOUNDED_SERVICE                                       0x08
#define ERR_OVER_RANGE                                               0x09
#define ERR_SEND_DATA_BUSY                                           0x0A
#define ERR_CHARACTERISTIC_MISMATCH                                  0x0B
#define ERR_INDICATE_BUSY                                            0x0C
#define ERR_MIN_INTERVAL_OVER_RANGE                                  0x0D
#define ERR_MAX_INTERVAL_OVER_RANGE                                  0x0E
#define ERR_MAX_INTERVAL_SHALL_BE_GREATER_THAN_MIN_INTERVAL          0x0F
#define ERR_LANTENCY_OVER_RANGE                                      0x10
#define ERR_TIMEOUT_OVER_RANGE                                       0x11
#define ERR_WAITING_OTHER_CONNECTION_PARAMETER_UPDATE_REQUEST        0x12
#define ERR_NO_CONNECTION_PARAMETER_UPDATE_REQUEST                   0x13
#define ERR_WAITING_CONNECTION_PARAMETER_UPDATE_RESULT               0x14
#define ERR_BLE_STATE                                                0x15
#define ERR_SERVICE_NOTIFICATION_NOT_ENABLE                          0x16
#define ERR_SERVICE_INDICATE_NOT_ENABLE                              0x17
#define ERR_NO_TERMINATE_REQUEST                                     0x18
#define ERR_PROCESSING_TERMINATE_REQUEST                             0x19
#define ERR_FINISH_TERMINATE                                         0x1A
#define ERR_LENGTH_MISTAKE                                           0x1B
#define ERR_NO_APPLE_NOTIFICATION_CENTER_SERVICE_DISCOVER_COMMAND    0x1C
#define ERR_APPLE_NOTIFICATION_CENTER_SERVICE_DISCOVERING            0x1D
#define ERR_NOT_FOUND_APPLE_NOTIFICATION_CENTER_SERVICE              0x1E
#define ERR_APPLE_NOFITIFCATION_CENTER_SERVICE_EXIST                 0x1F
#define ERR_ANCS_DATASOURCE_NOT_SUPPORTED                            0x20
#define ERR_ANCS_CONTROL_POINT_NOT_SUPPORTED                         0x21
#define ERR_ANCS_CONTROL_POINT_BUSY                                  0x22
#define ERR_ANCS_SYSTEM_BUSY                                         0x23

/*********************************************************************
** Define DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION
*********************************************************************/
#define GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION       0x01  
#define GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_INDICATION         0x02  

/*********************************************************************
** Extern Global Variables
*********************************************************************/
extern bit internalRC;
extern uint8_t xdata ble_state;
extern uint8_t xdata adv_data[32];
extern uint8_t xdata scan_resp_data[32];
extern bit AutoSet_BatteryResultFlag;
extern bit BLE_writeEventFlag;
extern uint8_t xdata rf_config_w[111];
extern bit BLE_ANCS_NotificationEvent;
extern uint8_t xdata att_HDL_ANCS_CHARACTERISTIC_NOTIFICATION_SOURCE[];
extern uint8_t xdata att_HDL_ANCS_NOTIFICATION_SOURCE[];
extern uint8_t xdata att_HDL_ANCS_CLIENT_CHARACTERISTIC_CONFIGURATION[];
extern uint8_t xdata att_HDL_ANCS_CONTROL_POINT[];
extern uint8_t xdata att_HDL_ANCS_DATA_SOURCE[];
extern uint8_t xdata att_HDL_ANCS_DATA_SOURCE_CLIENT_CHARACTERISTIC_CONFIGURATION[];
extern const uint8_t code A8107_ConfigTab[];


extern uint8_t xdata		auth_req;
extern uint8_t xdata		init_req;
extern uint8_t xdata		index;
extern uint8_t xdata		auth_init_finish;
extern uint8_t xdata		scale_index;
extern uint8_t xdata		KeyWakeup;
extern uint8_t xdata		scalebuf[10];
extern uint16_t xdata	weightvalue;
extern uint32_t xdata	datevalue;
extern uint8_t xdata	sendscaledata;
extern uint8_t xdata	waitpacket;
////////////TWOR////////////////////////////////
extern uint8_t xdata	TworTimer;
extern uint8_t xdata	TworTimerFlag;
////////////TWOR///////////////////////////////
extern uint8_t xdata BLE_INT_FLAG;
extern uint8_t xdata	OTA_START_FLAG;
extern uint8_t     xdata   OTA_RECONNECT_FLAG;
extern uint16_t xdata check_stable;

/*********************************************************************
** Extern Function
*********************************************************************/
extern void InterruptDisable(void);
extern void InterruptEnable(void);
extern void InitCrystalCL(uint8_t databyte);
extern void InitRF(void);
extern void InitBLE(void);
extern void BLE_SetTxPower(uint8_t amplevel);
extern uint8_t BLE_ADV_Cmd(ADV_InitDef* ADV_InitStruct);
extern void BLE_SLAVE_PROCESS(void);
extern void StrobeCmd(uint8_t cmd);
extern void A8107_WriteID(void);
extern void BLE_AutoPwrDown_Enable(void);
extern void BLE_AutoPwrDown_Disable(void);
extern uint8_t BLE_InitImmediateAlertLevel(uint8_t tmplevel);
extern uint8_t BLE_InitLinkLossAlertLevel(uint8_t tmplevel);
extern uint16_t BLE_GetLinkLossCount(void);
extern void BLE_SetTerminate(void);
extern uint8_t BLE_GetPwrstate(void);
extern uint32_t BLE_GetEmptyCount(void);
extern uint8_t BLE_SetConnectParameterUpdateRequest(uint16_t minInterval,uint16_t maxInterval,uint16_t paraLatency,uint16_t paratimeout);
extern uint8_t BLE_GetConnectUpdateResult(void);
extern uint8_t BLE_SendData(uint8_t* ServiceBuffer, uint8_t * SERVICEBUFFER_INITIAL, uint8_t tmplength);
extern uint8_t* BLE_GetWriteEnvet(void);
extern uint8_t RF_GetBatteryResult(void);
extern uint16_t RF_GetRSSIResult(void);
extern uint8_t* BLE_GetConnectParameter(void);
extern uint8_t BLE_GetTerminateResult(void);
extern uint8_t BLE_GetWriteEnvet_Length(void);
extern uint8_t BLE_ANCS_Discover(void);
extern uint8_t BLE_GetANCS_Result(void);
extern uint8_t BLE_ANCS_NotificationSource_Enable(void);
extern uint8_t BLE_ANCS_NotificationSource_Disable(void);
extern uint8_t BLE_ANCS_DataSource_Enable(void);
extern uint8_t BLE_ANCS_DataSource_Disable(void);
extern uint8_t* BLE_Get_ANCS_Nofitication_Envet(void);
extern uint8_t BLE_Get_ANCS_Notification_Length(void);
extern uint8_t BLE_ANCS_Send_Control_Point(uint8_t tmplength);
extern void BLE_whiteList_Enable(void);
extern void BLE_whiteList_Disable(void);
extern uint8_t BLE_GetwhiteList_Status(void);
extern uint8_t* BLE_GetConnectServerAddress(void);
extern void BLE_SetWhiteList(uint8_t* tmpbuf);
////////////////TWOR///////////////////////////////////
extern void A8107_RCSelect(bit rcselect);
////////////////TWOR///////////////////////////////////
////////////////TWOR///////////////////////////////////
extern void Twor05Timer(uint8_t TimerEnable);
////////////////TWOR///////////////////////////////////
extern void Prcss_Weight(void);
extern uint8_t toascii(uint32_t in, uint8_t unit);


#endif