/*****************************************************************************
**               AMICCOM Electronics Corporation Document                   **
**          Copyright (c) 2011-2015 AMICCOM Electronics Corporation         **
**                                                                          **
**      A3,1F,No.1, Li-Hsin 1th Road, Science_Based Industrid Park,         **
**                       Hsin_Chu City, 300, Taiwan, ROC.                   **
**                 Tel: 886-3-5785818   Fax: 886-3-5785819                  **
**         E-mail:info@amiccom.com.tw  http: //www.amiccom.com.tw           **
*****************************************************************************/
#include "define.h"
#include "A8107.h"
#include "LibFunction.h"
#include "mcufunction.h"
#include "IAP.h"
#include "servicegen.h"
#include "OTA.h"

uint8_t fast_adv;
/*******************************************************************
 *
 * main - main function
 * Description:
 *      The function is the system entry point. The whole system is
 *      start from here.
 *
 ******************************************************************/
void main(void)
{
    ADV_InitDef ADV_InitStructure;

    uint8_t xdata   DataLen;
    uint8_t xdata   *ptrChar = NULL;
    uint8_t xdata   UpdateOTASpeed;
    uint8_t xdata   Temp;

    while(~P0_4);

    InterruptDisable();
    InitMCU();
    internalRC = 0; //0 :Using External RC 32.768KHz ; 1:Using internal RC 32.000KHz

    InitCrystalCL(0x32);  //AMICCOM CrystalCL 0x32(MD8107-A05)(18pF) / 0x14(MD8107-A06)(12pF) / 0x0D(9pF)

    Temp = InitRF();
    if(Temp != SUCCESS) {
        /*******Calibration Error*******/
    }

    IAP_Initialize();
    OTA_Initialize();

    InitBLE();
    BLE_SetTxPower(6); //level 0 ~ 7 : 0 => -17dBm; 1 => -15dBm; 2 => -10dBm; 3 => -5dBm; 4 => 0dBm; 5 => FCC/CE Setting; 6 => 4dBm; 7 => 7dBm
    InterruptEnable();

    RF_Timer500ms(DISABLE);

    fast_adv = 1;

    while(1)
    {
        OTA_Process();

        /* User Can Add Code */
        if(ble_state == STANDBY_STATE)
        {
            if(RF_Get_Partial_InitRF_Result() != SUCCESS)
            {
                /*******Calibration Error*******/

                /*******************************/
            }

            if(fast_adv)
            {
                fast_adv = 0;

                ADV_InitStructure.ADV_Timing = 8; // ADV Interval = (20ms + 0.625ms*ADV_Timing) = 25 ms
                ADV_InitStructure.ADV_TO = 4800;  // ADV Timeout = ADV Interval * ADV_TO = 25ms*4800 = 120 sec
                ADV_InitStructure.ADV_Channel = (TE_ADVCHANNEL)(ADV_CH37 | ADV_CH38 | ADV_CH39);
                ADV_InitStructure.ADV_Type = ADV_IND;
                ADV_InitStructure.ADV_RndEnable = DISABLE;
                ADV_InitStructure.ADV_TOEnable = ENABLE;
                ADV_InitStructure.MACAddr_Type = ADDR_PUBLIC;
                ADV_InitStructure.ADV_Run = ENABLE;
                BLE_ADV_Cmd(&ADV_InitStructure);
            }
            else
            {
                ADV_InitStructure.ADV_Timing = 128; // ADV Interval = (20ms + 0.625ms*ADV_Timing) = 100 ms
                ADV_InitStructure.ADV_TO = 1500;  // ADV Timeout = ADV Interval * ADV_TO = 100ms*1500 = 150 sec
                ADV_InitStructure.ADV_Channel = (TE_ADVCHANNEL)(ADV_CH37 | ADV_CH38 | ADV_CH39);
                ADV_InitStructure.ADV_Type = ADV_IND;
                ADV_InitStructure.ADV_RndEnable = DISABLE;
                ADV_InitStructure.ADV_TOEnable = DISABLE;
                ADV_InitStructure.MACAddr_Type = ADDR_PUBLIC;
                ADV_InitStructure.ADV_Run = ENABLE;
                BLE_ADV_Cmd(&ADV_InitStructure);
            }
        }
        else if(ble_state == ADV_STATE)
        {
            UpdateOTASpeed = ENABLE;

            if(ble_state == ADV_STATE)
            {
                BLE_AutoPwrDown_Enable();
            }
        }
        else if(ble_state == CONNECT_ESTABLISH_STATE)
        {
            fast_adv = 1;

            if (UpdateOTASpeed)
            {
                Temp = (uint8_t)OTA_UpdataSpeed();

                if ((OTA_SUCCESSFUL == Temp) || (OTA_COUNT_MAX == Temp))
                    UpdateOTASpeed = DISABLE;
            } /* Changing OTA download speeds */

            if (BLE_writeEventFlag)
            {
                ptrChar = BLE_GetWriteEvent();
                DataLen = BLE_GetWriteEvent_Length();

                if (ptrChar == &att_HDL_OTA_OTA_DATA)
                {
                    OTA_SetReceiveData((uint16_t)att_HDL_OTA_OTA_DATA, DataLen);
                    OTA_Process();
                } /* Characteristic Data below the OTA Service */
                else if (ptrChar == &att_HDL_OTA_OTA_CMD_CONTROL_POINT)
                {
                    OTA_SetReceiveData((uint16_t)att_HDL_OTA_OTA_CMD_CONTROL_POINT, DataLen);
                    OTA_Process();
                } /* Characteristic Command below the OTA Service */
                else
                {

                }
            }

            if(ble_state == CONNECT_ESTABLISH_STATE)
            {
                BLE_AutoPwrDown_Enable();
            }
        }
    }
}
