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
#include "usermcufunction.h"

uint8_t fast_adv;
uint8_t xdata xpresskeyVolup;
uint8_t xdata xpresskeyVoldown;
uint8_t xdata xpresskeyVideo;
uint8_t xdata xpresskeyInvert;
uint8_t xdata xpresskeyCapture;
uint8_t xdata isLongPressed = 1;


extern bit IntoSleepFlag;

void _3nop_delay(void);
void handleCMD(uint8_t *pcBuf);
void key_handleEvent(void);
uint8_t checkSum(uint8_t *pcBuf, uint8_t lenght);
void respondMaster(uint8_t *ptrChar, uint8_t DataLen);

#define STRICKCNTLEVEL_UP 3600 

#ifdef _PROFILE_HOGP_
#ifdef _PROFILE_HOGP_KEYBOARD_
#define HDL_HIDS_REPORT_TAB_KEY_CTRL                            0
#define HDL_HIDS_REPORT_TAB_KEY_DATA0                           2
#define HDL_HIDS_REPORT_TAB_KEY_DATA1                           3
#define HDL_HIDS_REPORT_TAB_KEY_DATA2                           4
#define HDL_HIDS_REPORT_TAB_KEY_DATA3                           5
#define HDL_HIDS_REPORT_TAB_KEY_DATA4                           6
#define HDL_HIDS_REPORT_TAB_KEY_DATA5                           7
#endif
#endif

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
    uint16_t xdata usStrickCnt = 0;
    bit isNeedSleeping = FALSE;

    while(~P0_4);

    /* 1.关闭全局中断 */
    InterruptDisable();

    /* 2.初始化MCU */
    InitMCU();
    internalRC = 0; //0 :Using External RC 32.768KHz ; 1:Using internal RC 32.000KHz

    InitCrystalCL(0x0D);  //AMICCOM CrystalCL 0x32(MD8107-A05)(18pF) / 0x14(MD8107-A06)(12pF) / 0x0D(9pF)

    Temp = InitRF();
    if(Temp != SUCCESS) {
        /*******Calibration Error*******/
    }

    IAP_Initialize();
    OTA_Initialize();

    InitBLE();
    BLE_SetTxPower(0); //level 0 ~ 7 : 0 => -17dBm; 1 => -15dBm; 2 => -10dBm; 3 => -5dBm; 4 => 0dBm; 5 => FCC/CE Setting; 6 => 4dBm; 7 => 7dBm
    /* 3.开中断 */
    InterruptEnable();
    //EIE |= EKEYINT; /* ʹ���ⲿ�����ж� */
    //EIP |= EKEYPRI; /* �ⲿ�ж����ȼ��� */
    //EIF |= CLEAR_KEYINTFLAG; /* ����жϱ�־λ */

    Timer500ms_flag = 0; /* 清除Timer500ms_flag，每隔0.5s会被置位 */
    RF_Timer500ms(ENABLE);

    fast_adv = 1;

    while(1)
    {
        OTA_Process();

        if(FALSE == IntoSleepFlag)
        {
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
                    ADV_InitStructure.ADV_Timing = 768; // ADV Interval = (20ms + 0.625ms*ADV_Timing) = 100 ms
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
                        respondMaster(ptrChar, DataLen);
                    }
                }

#ifdef KYE_DEBUG
                if(P0_3)
                {
                    P0OE = 0x02;
                    //P0_1 = 0;
                }
                if(P0_5)
                {
                    P0OE &= ~0x02;
                }
#endif
                key_handleEvent();
                if(ble_state == CONNECT_ESTABLISH_STATE)
                {
                    BLE_AutoPwrDown_Enable();
                }
            }

            if(Timer500ms_flag)
            {
                usStrickCnt = usStrickCnt + 1;
                if(STRICKCNTLEVEL_UP == usStrickCnt)
                {
                    usStrickCnt = 0;
                    isNeedSleeping = TRUE;
                }
                Timer500ms_flag = 0;
            }

            if(isNeedSleeping)
            {
                entrySleep();
                isNeedSleeping = FALSE;
            }
        }
        else
        {
            isNeedSleeping = FALSE;
            _nop_();
        }
    }
}

void respondMaster(uint8_t *ptrChar, uint8_t DataLen)
{
    uint8_t xdata i;
    uint8_t xdata result;
    DataLen = DataLen;

#ifdef _PROFILE_USER_DEFINE_01_
    if(ptrChar == &att_HDL_USER_DEFINE_01_DATAW01)
    {
        handleCMD(att_HDL_USER_DEFINE_01_DATAW01);
    }
    else if(ptrChar == &att_HDL_USER_DEFINE_01_DATAW02)
    {
            if((att_HDL_USER_DEFINE_01_DATAN02_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
            {
                    for(i=0;i<20;i++)
                            att_HDL_USER_DEFINE_01_DATAN02[i] = att_HDL_USER_DEFINE_01_DATAW02[i];

                    result = BLE_SendData(att_HDL_USER_DEFINE_01_DATAN02,ATT_HDL_USER_DEFINE_01_DATAN02_INIT,5);

                    if(result == SUCCESS)
                    {
                            _nop_();
                            //P0_0 = ~P0_0;
                    }
                    else
                    {
                            _nop_();
                            //P0_1 = ~P0_1;
                    }
            }

    }
#endif
    else
        ;

}

void handleCMD(uint8_t *pcBuf)
{
    uint8_t result = 0;
    BLE_CMD_Req_S *pstBLE_CMD_Req = NULL;
    if(NULL == pcBuf)
    {
        return ;
    }

    pstBLE_CMD_Req = (BLE_CMD_Req_S *)pcBuf;
    result = checkSum(pcBuf, 20);
    if(0 != result)
    {
        return;
    }

    switch(pstBLE_CMD_Req->cmd)
    {
        case BLE0_CODE_SLEEP:
            entrySleep(); /* 进入永久休眠，等待按键唤醒 */
            break;
        case BLE0_CODE_LONG_PRESSED:
            if(pstBLE_CMD_Req->payload[0])
            {
                isLongPressed = 1;
            }
            else
            {
                isLongPressed = 0;
            }
            break;
        default:
            break;
    }
}

uint8_t checkSum(uint8_t *pcBuf, uint8_t lenght)
{
    uint8_t i, ucCheckSum=0;
    if(pcBuf == NULL || lenght < 0)
    {
        return 255;
    }

    for(i=0; i<lenght; i++)
    {
        ucCheckSum += pcBuf[i]; 
    }

    return ucCheckSum;
}

void _3nop_delay(void)
{
    _nop_();
    _nop_();
    _nop_();   
}

void key_handleEvent(void)
{
    uint8_t xdata result;
    uint16_t i,j;

    /* 键盘处理 */
    /* Volume- */
    if(~P0_0)
    {
        /* 消抖 */
        for(i=0; i<1000; i++)
        {
            for(j=0; j<10; j++)
            {
                _3nop_delay();
            }
        }

        if(P0_0)
        {
            return ;
        }

        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
        {
            /* 焦距- */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x81; /* Volume- */
            result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
            if(result == SUCCESS)
            {
                xpresskeyVoldown = 1;
            }
        }
        else
        {
            return;
        }
        
        do
        {
            if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
            {
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x00;
                result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyVoldown = 0;
                }
            }
        }while(xpresskeyVoldown);

        if(isLongPressed)
        {
            do
            {
                _nop_();
            }while(~P0_0);
        }
    }

    /* Volume+ */
    if(~P0_1)
    {
        /* 消抖 */
        for(i=0; i<1000; i++)
        {
            for(j=0; j<10; j++)
            {
                _3nop_delay();
            }
        }

        if(P0_1)
        {
            return ;
        }

        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
        {
            /* 焦距+ */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x80; /* Volume+ */
            result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
            if(result == SUCCESS)
            {
                xpresskeyVolup = 1;
            }
        }
        else
        {
            return;
        }

        do
        {
            if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
            {
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x00;
                result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyVolup = 0;
                }
            }
        }while(xpresskeyVolup);

        if(isLongPressed)
        {
            do
            {
                _nop_();
            }while(~P0_1);
        }
    }

    /* key4 */
    if(~P0_2)
    {
        /* 消抖 */
        for(i=0; i<1000; i++)
        {
            for(j=0; j<10; j++)
            {
                _3nop_delay();
            }
        }

        if(P0_2)
        {
            return ;
        }

        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
        {
            /* 摄像 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x21; /* key4 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA1] = 0x72; /* fingerprint identify end */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA2] = 0x69; /* 401 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA3] = 0x6E; /* fingerprint up */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA4] = 0x6F; /* fingerprint finger up */
            result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
            if(result == SUCCESS)
            {
                xpresskeyVideo = 1;
            }
        }
        else
        {
            return;
        }

        do
        {
            if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
            {
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA1] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA2] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA3] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA4] = 0x00;
                result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyVideo = 0;
                }
            }
        }while(xpresskeyVideo);

        if(isLongPressed)
        {
            do
            {
                _nop_();
            }while(~P0_2);
        }
    }

    /* key3 */
    if(~P0_3)
    {
        /* 消抖 */
        for(i=0; i<1000; i++)
        {
            for(j=0; j<10; j++)
            {
                _3nop_delay();
            }
        }

        if(P0_3)
        {
            return ;
        }

        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
        {
            /* 前后镜头切换 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x20; /* key3 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA1] = 0x72; /* fingerprint identify end */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA2] = 0x69; /* 401 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA3] = 0x6E; /* fingerprint up */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA4] = 0x6F; /* fingerprint finger up */
            result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
            if(result == SUCCESS)
            {
                xpresskeyInvert = 1;
            }
        }
        else
        {
            return;
        }

        do
        {
            if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
            {
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA1] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA2] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA3] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA4] = 0x00;
                result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyInvert = 0;
                }
            }
        }while(xpresskeyInvert);

        if(isLongPressed)
        {
            do
            {
                _nop_();
            }while(~P0_3);
        }
    }

    /* enter */
    if(~P0_5)
    {
        /* 消抖 */
        for(i=0; i<1000; i++)
        {
            for(j=0; j<10; j++)
            {
                _3nop_delay();
            }
        }

        if(P0_5)
        {
            return ;
        }

        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
        {
            /* 照相 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x28; /* entry */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA1] = 0x72; /* fingerprint identify end */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA2] = 0x69; /* 401 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA3] = 0x6E; /* fingerprint up */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA4] = 0x6F; /* fingerprint finger up */
            result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
            if(result == SUCCESS)
            {
                xpresskeyCapture = 1;
            }
        }
        else
        {
            return;
        }

        do
        {
            if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
            {
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA1] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA2] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA3] = 0x00;
                att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA4] = 0x00;
                result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyCapture = 0;
                }
            }
        }while(xpresskeyCapture);        

        if(isLongPressed)
        {
            do
            {
                _nop_();
            }while(~P0_5);
        }
    }
}

