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
#include "debug.h"
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
uint8_t xdata isLongPressed = 0; /* 确定蓝牙键盘是否采用长按机制, 0:采用,1:不采用 */
uint32_t xdata isKeyLowPowerFlag = 0; /* Master是否进入省电模式的标志 */
uint16_t xdata usStrickCnt = 0; /* 用于MCU计时 */
uint16_t xdata usStrickCntSave = 0; /* 保存上次MCU计数值 */
uint8_t xdata gucCurMasterOSType = 0; /* 当前模块Master系统类型 */


extern bit IntoSleepFlag;
extern uint8_t gaucBLE_ADDRESS[BLE_MACADDRESS_SIZE];

void _3nop_delay(void);
void handleCMD(uint8_t *pcBuf);
void key_handleEventForandroid(void);
void key_handleEventForios(void);
sint8_t checkSum(uint8_t *pcBuf, uint8_t lenght);
void respondMaster(uint8_t *ptrChar, uint8_t DataLen);
void ledblink(uint8_t isblink, uint8_t uiFlag);
void sendBattery(uint16_t value);
uint16_t getBattery();
void sendOsType(BLE_CMD_Req_S *pstBLE_CMD_Req);
uint8_t getOsType(BLE_CMD_Req_S *pstBLE_CMD_Req);

#define STRICKCNTLEVEL_UP 36000 /* 5 hours */ 

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

#ifdef _PROFILE_HOGP_
#ifdef _PROFILE_HOGP_COMSUMER_
#define HDL_HIDS_REPORT_TAB_CSKEY_L                             0
#define HDL_HIDS_REPORT_TAB_CSKEY_H                             1
#endif
#endif

#ifdef _PROFILE_HOGP_
#ifdef _PROFILE_HOGP_COMSUMER_
const uint8_t code HID_RPT_CS_KEY_DEMO[][2] =
{
    {0xE9, 0x00,},  //vol+
    {0xEA, 0x00,},  //vol-
    {0xE2, 0x00,},  //Mute
    {0xB0, 0x00,},  //Play
    {0xB1, 0x00,},  //Pause
    //{0xB2, 0x00,},  //Record
    {0xB3, 0x00,},  //Fast forward
    {0xB4, 0x00,},  //Rewind
    {0xB5, 0x00,},  //Scan next track
    {0xB6, 0x00,},  //Scan previous track
    {0xB7, 0x00,},  //Stop
    {0xB8, 0x00,},  //Eject
    {0x8A, 0x01,},  //Email reader
    {0x96, 0x01,},  //Internet browser
    {0x9E, 0x01,},  //Terminal lock/screensaver
    {0xC6, 0x01,},  //Research/search browser
    {0x2D, 0x02,},  //Zoom in
};
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
    uint8_t xdata   *pucDefADDR = NULL;
    int xdata i, isok;
    bit isNeedSleeping = FALSE;

    while(~P0_4);

    /* 1.关闭全局中断 */
    InterruptDisable();

    /* 2.初始化MCU */
    InitMCU();
    initKey();
    initUart();

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

    /* delay ~300ms */
    for(i=0; i<10000; i++)
    {
        _3nop_delay();
    }
    initUart0_timer2();

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
                ledblink(FALSE, LED_FLAG_NOBLINK);
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

                IAP_ReadData(0x2FA, gaucBLE_ADDRESS, BLE_MACADDRESS_SIZE);
                pucDefADDR = BLE_GetConnectServerAddress();
                isok = 0;
                for(i=0;i<6;i++)
                {
                    //isok *= 255;
                    //isok += *pucDefADDR++;
                    if(pucDefADDR[i] == 0x00)
                    {
                        isok = 0xCC;
                        break;
                    }
                }
                //if(isok == 0xE7F1E468767C)
                if(isok == 0x02)
                {
                    isok = 0xCC;
                }

                //if(6 == i)
                //{
                //    isok = 0x00;
                //}
                //if(NULL == pucDefADDR)
                //{
                //    key_handleEventForios();
                //}
                //if(0xCC != isok)
                //{
                //    key_handleEventForios();
                //}
                //else
                //{
                //    key_handleEventForandroid();
                //}
                
                //if(0 == strncmp(gaucBLE_ADDRESS, pucDefADDR, BLE_MACADDRESS_SIZE))
                if(BLE_MASTER_OSTYPE_IOS == gucCurMasterOSType)
                {
                    key_handleEventForios();
                }
                else
                {
                    key_handleEventForandroid();
                }
                
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
        if(CONNECT_ESTABLISH_STATE != ble_state)
        {
            ledblink(TRUE, LED_FLAG_BLINK_1S);
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

uint16_t getBattery()
{
    uint16_t usValue;
    P3OE &= 0xFD;  /* P3.1 input */
    P3PUN |= ~0xFD; /* no wakeup */
    //ADCCH = 0x0F; /* select P3.1 for ADC, and Enable ADC analog input */

    /* setting and running */
    XBYTE[EXT1_REG] = 0x58;  /* disable REGR */
    XBYTE[ADCAVG1_REG] = 0x02; /* enable the SAR ADC */
    XBYTE[ADCCTL_REG] = 0x01; /* single mode and ADC measurement enable */

    usValue = 0;
    while(XBYTE[ADCCTL_REG] & 0x01);
    usValue = XBYTE[ADCAVG1_REG] & 0xF0;
    usValue <<= 4;
    usValue |= XBYTE[ADCAVG2_REG];
    XBYTE[ADCCTL_REG] = 0x00;
    XBYTE[ADCAVG1_REG] = 0x00;
    //ADCCH = &0xFE; /* disable ADC analog input */

    return usValue;
}

void sendBattery(uint16_t value)
{
    uint8_t result;

    if((att_HDL_USER_DEFINE_01_DATAN01_CLIENT_CHARACTERISTIC_CONFIGURATION[0] & 
       GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
    {
        att_HDL_USER_DEFINE_01_DATAN01[0] = value&0xFF00 >> 8;
        att_HDL_USER_DEFINE_01_DATAN01[1] = value&0xFF;
    }

    result = BLE_SendData(att_HDL_USER_DEFINE_01_DATAN01, ATT_HDL_USER_DEFINE_01_DATAN01_INIT, 2);
}

uint8_t getOsType(BLE_CMD_Req_S *pstBLE_CMD_Req)
{
    if(NULL == pstBLE_CMD_Req)
    {
        return ERR_FAILED;
    }
    if(BLE_MASTER_OSTYPE_IOS == pstBLE_CMD_Req->payload[0])
    {
        gucCurMasterOSType = BLE_MASTER_OSTYPE_IOS;
    }
    else if(BLE_MASTER_OSTYPE_ANDROID == pstBLE_CMD_Req->payload[0])
    {
        gucCurMasterOSType = BLE_MASTER_OSTYPE_ANDROID;
    }
    else if(BLE_MASTER_OSTYYPE_WINDOWSPHONE == pstBLE_CMD_Req->payload[0])
    {
        gucCurMasterOSType = BLE_MASTER_OSTYYPE_WINDOWSPHONE;
    }
    else
    {
        gucCurMasterOSType = BLE_MASTER_OSTYPE_FIRST;
        return ERR_FAILED;
    }
    pstBLE_CMD_Req->payload[1] = 0xCC;

    return SUCCESS;
}

void sendOsType(BLE_CMD_Req_S *pstBLE_CMD_Req)
{
    uint8_t i;
    uint8_t *pucTmp = NULL;

    if(NULL == pstBLE_CMD_Req)
    {
        return;
    }   
    pucTmp = (uint8_t*)pstBLE_CMD_Req;
    pstBLE_CMD_Req->CheckSum = 256 - (checkSum(pucTmp, 19))&0xFF;

    if((att_HDL_USER_DEFINE_01_DATAN01_CLIENT_CHARACTERISTIC_CONFIGURATION[0] & 
       GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
    {
        for(i=0; i<20; i++)
        {
            att_HDL_USER_DEFINE_01_DATAN01[i] = pucTmp[i];
        }
    }

    (void)BLE_SendData(att_HDL_USER_DEFINE_01_DATAN01, ATT_HDL_USER_DEFINE_01_DATAN01_INIT, 20);
}

void handleCMD(uint8_t *pcBuf)
{
    sint32_t result = 0;
    BLE_CMD_Req_S *pstBLE_CMD_Req = NULL;
    if(NULL == pcBuf)
    {
        return ;
    }

    pstBLE_CMD_Req = (BLE_CMD_Req_S *)pcBuf;
    result = (sint8_t)checkSum(pcBuf, 20);
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
        case BLE0_CODE_GET_BATTERY:
            result = getBattery();
            sendBattery((uint16_t)result);
            break;
        case BLE0_CODE_OSTYPE:
            result = getOsType(pstBLE_CMD_Req);
            if(SUCCESS == result)
            {
                sendOsType(pstBLE_CMD_Req);
            }
            break;
        default:
            break;
    }
}

sint8_t checkSum(uint8_t *pcBuf, uint8_t lenght)
{
    uint8_t i;
    sint8_t cCheckSum=0;
    if(pcBuf == NULL || lenght < 0)
    {
        return 255;
    }

    for(i=0; i<lenght; i++)
    {
        cCheckSum += pcBuf[i]; 
    }

    return cCheckSum;
}

void _3nop_delay(void)
{
    _nop_();
    _nop_();
    _nop_();   
}

void key_handleEventForandroid(void)
{
    uint8_t xdata result, result1;
    uint16_t i,j;
    bit usFlag = FALSE;
    j=j; /* not visibel warning */
    isKeyLowPowerFlag = 0;

    /* 键盘处理 */
    /* Volume- */
    if(~P0_0)
    {
        /* 消抖 */
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
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
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
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
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
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

        //if(isLongPressed) 取消长按功能
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
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
            {
                _3nop_delay();
            }
        }
        dprintf("0123456789\n");

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

        //if(isLongPressed) 取消长按功能
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
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
            {
                _3nop_delay();
            }
        }

        if(P0_5)
        {
            return ;
        }

        //if(isLongPressed) 取消长按功能
        {
            do
            {
                isKeyLowPowerFlag++;
                if(BLE_POWERSAVE_LEVEL <= isKeyLowPowerFlag && FALSE == usFlag)
                {
                    if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                        & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
                    {
                        att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x27; /* key0 */
                        result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                    }

                    do
                    {
                        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
                        {
                            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x00; /* key0 */
                            result1 = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                            if(result1 == SUCCESS)
                            {
                                usFlag = TRUE;
                                break;
                            }
                        }
                    }while(SUCCESS == result);
                }
                _nop_();
            }while(~P0_5);

            if(TRUE == usFlag)
            {
                return;
            }
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
    }
    
}

void key_handleEventForios(void)
{
    uint8_t xdata result, result1;
    uint16_t i,j;
    bit usFlag = FALSE;
    j=j; /* not visibel warning */
    isKeyLowPowerFlag = 0;

    /* 键盘处理 */
    /* Volume- */
    if(~P0_0)
    {
        /* 消抖 */
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
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
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x1F; /* key2 */
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

        /* Pressed delay 500ms */
        for(i=0; i<18888; i++)
        {
            if(P0_0)
            {
                break;
            }

            _3nop_delay();
        }

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
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
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
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x1E; /* key1 */
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

        /* Pressed delay 500ms */
        for(i=0; i<18888; i++)
        {
            if(P0_1)
            {
                break;
            }

            _3nop_delay();
        }

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
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
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
                result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyVideo = 0;
                }
            }
        }while(xpresskeyVideo);

        //if(isLongPressed) //取消长按功能
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
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
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
                result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyInvert = 0;
                }
            }
        }while(xpresskeyInvert);

        //if(isLongPressed) //取消长按功能
        {
            do
            {
                _nop_();
            }while(~P0_3);
        }
    }

    /* enter + key8/volume+ */
    if(~P0_5)
    {
        /* 消抖 */
        //for(i=0; i<1000; i++)
        for(i=0; i<250; i++)
        {
            //for(j=0; j<10; j++)
            {
                _3nop_delay();
            }
        }

        if(P0_5)
        {
            return ;
        }

        //if(isLongPressed) //取消长按功能
        {
            do
            {
                isKeyLowPowerFlag++;
                if(BLE_POWERSAVE_LEVEL <= isKeyLowPowerFlag  && FALSE == usFlag)
                {
                    if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                        & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
                    {
                        att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x27; /* key0 */
                        result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                    }

                    do
                    {
                        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
                        {
                            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x00; /* key0 */
                            result1 = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                            if(result1 == SUCCESS)
                            {
                                usFlag = FALSE;
                                break;
                            }
                        }
                    }while(SUCCESS == result);
                    isKeyLowPowerFlag = 0;
                }
                _nop_();
            }while(~P0_5);

            if(TRUE == usFlag)
            {
                return;
            }
        }

        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
        {
            /* 照相 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x58;//0x28; /* entry */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA1] = 0x25; /* key8 */
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
                result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyCapture = 0;
                }
            }
        }while(xpresskeyCapture);

#ifdef _PROFILE_HOGP_COMSUMER_
        do
        {
            if((att_HDL_HIDS_REPORT_CSI_CLIENT_CHARACTERISTIC_CONFIGURATION[0]
                & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
            {
                att_HDL_HIDS_REPORT_CSI[HDL_HIDS_REPORT_TAB_CSKEY_L] = HID_RPT_CS_KEY_DEMO[0][0];
                att_HDL_HIDS_REPORT_CSI[HDL_HIDS_REPORT_TAB_CSKEY_H] = HID_RPT_CS_KEY_DEMO[0][1];
                result = BLE_SendData(att_HDL_HIDS_REPORT_CSI,ATT_HDL_HIDS_REPORT_CSI_INIT,ATT_HDL_HIDS_REPORT_CSI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyCapture = 1;
                }
            }
        }while(!xpresskeyCapture);
        
        do
        {        
            if((att_HDL_HIDS_REPORT_CSI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
                & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
            {
                att_HDL_HIDS_REPORT_CSI[HDL_HIDS_REPORT_TAB_CSKEY_L] = 0x00;
                att_HDL_HIDS_REPORT_CSI[HDL_HIDS_REPORT_TAB_CSKEY_H] = 0x00;
                result = BLE_SendData(att_HDL_HIDS_REPORT_CSI,ATT_HDL_HIDS_REPORT_CSI_INIT,ATT_HDL_HIDS_REPORT_CSI_INIT[4]);
                if(result == SUCCESS)
                {
                    xpresskeyCapture = 0;
                }
            }
        }while(xpresskeyCapture);
#endif
    }

}

void ledblink(uint8_t isblink, uint8_t uiFlag)
{
    uint16_t i,j;
    if(uiFlag >= LED_FLAG_END || (FALSE != isblink && TRUE != isblink))
    {
        return;
    }

    if(FALSE == isblink)
    {
        P0_6 = 1; //no light
        return;
    }

    if(usStrickCntSave == usStrickCnt)
    {
        return;
    }

    if(LED_FLAG_BLINK_1S == uiFlag)
    {
        // 1s blink blink...
        if(0 == usStrickCnt)
        {
            for(i=0; i<1000; i++)
            {
                for(j=0; j<333; j++)
                {
                    _3nop_delay();
                }
            }
            P0_6 = ~P0_6;
        }
        else
        {
            if(0 == usStrickCnt%2)
            {
                usStrickCntSave = usStrickCnt;
                P0_6 = ~P0_6;
            }
        }
    }

    return;
}
