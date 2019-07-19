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


uint8_t xdata HID_report_MS_key_temp;
uint8_t xdata HID_report_KB_key_temp; /* 键盘 */
uint8_t xdata isPressedKey = 0;

uint8_t xdata xpresskeyVolup;
uint8_t xdata xpresskeyVoldown;
uint8_t xdata xpresskeyVideo;
uint8_t xdata xpresskeyInvert;
uint8_t xdata xpresskeyCapture;

uint8_t xdata mousedata;

void _3nop_delay(void);
void handleCMD(uint8_t *pcBuf);
void key_handleEvent(void);
uint8_t checkSum(uint8_t *pcBuf, uint8_t lenght);
void respondMaster(uint8_t *ptrChar, uint8_t DataLen);

#define STRICKCNTLEVEL_UP 600 

#ifdef _PROFILE_HOGP_
#ifdef _PROFILE_HOGP_COMSUMER_
#define HDL_HIDS_REPORT_TAB_CSKEY_L                             0
#define HDL_HIDS_REPORT_TAB_CSKEY_H                             1
#endif

#ifdef _PROFILE_HOGP_MOUSE_
#define HDL_HIDS_REPORT_TAB_KEY_L_R                             0
#define HDL_HIDS_REPORT_TAB_DIR_L_R_L                           1
#define HDL_HIDS_REPORT_TAB_DIR_L_R_H                           2
#define HDL_HIDS_REPORT_TAB_DIR_U_D_L                           3
#define HDL_HIDS_REPORT_TAB_DIR_U_D_H                           4
#define HDL_HIDS_REPORT_TAB_ROL_U_D                             5
#endif

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
    
    bit isUpdate = FALSE;
    bit isUpdate1 = FALSE;
    bit isWorking = FALSE;
    bit isNeedSleeping = FALSE;

    uint16_t xdata usStrickCnt = 0;
    uint8_t xdata   Temp;
    uint8_t xdata   UpdateOTASpeed;
    uint8_t xdata   DataLen;
    uint8_t xdata   AdvFastSpeed = ENABLE;
    uint8_t xdata   *ptrChar = NULL;
	uint16_t xdata 	 waitpacket_tmp=0, waitpacket_tmp1=0, waitpacket_tmp2=0;
	//while(~P0_4);

    /* 1.关闭全局中断 */
    InterruptDisable();
    
    /* 2.初始化MCU    */
    InitMCU();
    internalRC = 0; /* 0 :Using External RC 32.768KHz ; 1:Using internal RC 32.000KHz */

    InitCrystalCL(0x0d);  //AMICCOM CrystalCL 0x32(MD8107-A05)(18pF)/ 0x14(MD8107-A06)(12pF)/ 0x0D(9pF)
    InitRF();
    IAP_Initialize();
    OTA_Initialize();

    InitBLE();
    BLE_SetTxPower(0); //level 0 ~ 7 : 0 => -17dBm; 1 => -15dBm; 2 => -10dBm; 3 => -5dBm; 4 => 0dBm; 5 => 2dBm; 6 => 4dBm; 7 => 7dBm

    /* 3.开中断 */
    InterruptEnable();
    EIE |= EKEYINT; /* 使能外部按键中断 */
    EIP |= EKEYPRI; /* 外部中断优先级高 */
    EIF |= CLEAR_KEYINTFLAG; /* 清除中断标志位 */

    /* 启动0.5s定时器 */
	//IOSEL|=0x08;
	IOSEL=0x00;
	TworTimer=0;
	TworTimerFlag=0; /* 清除TworTimerFlag，每隔0.5秒会被置位 */
	Twor05Timer(ENABLE);
    setIntoSleepFlag(FALSE);

	BLE_INT_FLAG=0;  /* 清除该标志位，此标志位可被BLE定时器置位                   */
	OTA_RECONNECT_FLAG = DISABLE; /* 開始 OTA 程序前，自動斷線並重新連線後之提示 FLAG */
        
    //user add code.
    
	while(1)
    {
        OTA_Process();

        if(!isIntoSleep())
        {
            /* User Can Add Code */
            if(ble_state == STANDBY_STATE)
            {
                reset();
                isWorking = FALSE;

                /* 初始化广播,并开启广播 */
                ADV_InitStructure.ADV_Timing = 768;//1568; // ADV Interval = (20ms + 0.625ms*ADV_Timing) = 20ms + 0.625ms*768 = 500ms
                ADV_InitStructure.ADV_TO = 1500;  // ADV Timeout = ADV Interval * ADV_TO = 20ms*1500 = 30 sec
                ADV_InitStructure.ADV_Channel = (TE_ADVCHANNEL)(ADV_CH37 | ADV_CH38 | ADV_CH39);// ;
                ADV_InitStructure.ADV_Type = ADV_IND;
                ADV_InitStructure.ADV_RndEnable = DISABLE;
                ADV_InitStructure.ADV_TOEnable = ENABLE;
                ADV_InitStructure.ADV_Run = ENABLE;
                BLE_ADV_Cmd(&ADV_InitStructure);

    			auth_req = FAIL;
    			init_req = FAIL;
    			auth_init_finish = FAIL;
    			index = 0;
    			scale_index = 0;
    			KeyWakeup = 0;
    			sendscaledata = FAIL;
    			waitpacket=0;
            }
            else if(ble_state == ADV_STATE)
            {
                UpdateOTASpeed = ENABLE;

                isUpdate = FALSE;
                isUpdate1 = FALSE;
                isWorking = FALSE;

                reset();
            }
            else if(ble_state == CONNECT_ESTABLISH_STATE)
            {
                isWorking = TRUE;
                isNeedSleeping = FALSE;
                setIntoSleepFlag(FALSE);
                
                if (UpdateOTASpeed)
                {
                    Temp = (uint8_t)OTA_UpdataSpeed();

                    if ((OTA_SUCCESSFUL == Temp) || (OTA_COUNT_MAX == Temp))
                        UpdateOTASpeed = DISABLE;
                } /* Changing OTA download speeds */
            
                if (BLE_writeEventFlag)
                {
                    ptrChar = BLE_GetWriteEnvet();
                    DataLen = BLE_GetWriteEnvet_Length();
                    respondMaster(ptrChar, DataLen);
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
            }

            if(TworTimerFlag)
            {
                usStrickCnt = usStrickCnt + 1;
                if(STRICKCNTLEVEL_UP == usStrickCnt)
                {
                    usStrickCnt = 0;
                    isNeedSleeping = TRUE;
                }

                if(OTA_RECONNECT_FLAG == DISABLE)
                {
                    if(OTA_START_FLAG==1)
                    _nop_();
                }
                TworTimerFlag=0;
            }

            if(isNeedSleeping)
            {
                //if(ble_state == CONNECT_ESTABLISH_STATE) /* 5分钟未连接关闭蓝牙模块 */
                //{
                //    isNeedSleeping = FALSE;
                //    continue;
                //}
                entrySleep();
                isNeedSleeping = FALSE;
                //break;
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
        case BLE0_CODE_FIRST:
            entrySleep(); /* 进入休眠，等待按键唤醒 */
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
    uint16_t i;

    /* 键盘处理 */

    if(~P0_0)
    {
        for(i=0; i<1000; i++)
            _3nop_delay(); /* 消抖 */

        if(P0_0)
        {
            return ;
        }

        if((att_HDL_HIDS_REPORT_KBI_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
            & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0)
        {
            /* 照相 */
            att_HDL_HIDS_REPORT_KBI[HDL_HIDS_REPORT_TAB_KEY_DATA0] = 0x58; /* enter */
            result = BLE_SendData(att_HDL_HIDS_REPORT_KBI,ATT_HDL_HIDS_REPORT_KBI_INIT,ATT_HDL_HIDS_REPORT_KBI_INIT[4]);
            if(result == SUCCESS)
            {
                xpresskeyCapture = 1;
            }
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
                    xpresskeyCapture = 0;
                }
            }
        }while(xpresskeyCapture);        

        do
        {
            _nop_();
        }while(~P0_0);
    }

    //if(P0_5)
    if(~P0_5)
    {
        for(i=0; i<1000; i++)
            _3nop_delay(); /* 消抖 */

        if(P0_5)
        //if(~P0_5)
        {
            return ;
        }

        if(1 != isPressedKey)
        {
            return;
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

        do
        {
            _nop_();
        }while(~P0_5);
        
        isPressedKey = 0;
    }

    //if(P0_3)
    if(~P0_3)
    {
        for(i=0; i<1000; i++)
            _3nop_delay(); /* 消抖 */

        //if(~P0_3)
        if(P0_3)
        {
            return ;
        }

        if(0 != isPressedKey)
        {
            return;
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

        do
        {
            _nop_();
        }while(~P0_3);

        isPressedKey = 1;
    }

    if(~P0_2)
    {
        for(i=0; i<1000; i++)
            _3nop_delay(); /* 消抖 */

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

        do
        {
            _nop_();
        }while(~P0_2);
    }
   
    if(~P0_1)
    {
        for(i=0; i<1000; i++)
            _3nop_delay(); /* 消抖 */

        if(P0_1)
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

        do
        {
            _nop_();
        }while(~P0_1);
    }

}
