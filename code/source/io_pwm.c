
#include "..\include\io_base.h"
#include "..\include\io_pwm.h"
#include "..\include\A8107.h"
#include "..\include\servicegen.h"
#include "..\include\LibFunction.h"
#include "..\include\mcufunction.h"


// ADC_Val 采集的最大值近似电压值为4.2 -- 3110    3.4v电压为2580
#define min     2580
#define max     3110
#define level   100

#define MaxValue 3110  

#define rdFlashAddress 0x02fa
/*--------------------------------------------------------------------------*/
/* TYPE DEFINITION                                                          */
/*--------------------------------------------------------------------------*/
stConfigParam ConfigParam;

bit          bcut_run;                      //led 指示参数
uint8_t      led_duty_cycle;
uint8_t      led_run_count;

uint8_t      Motors_run_count;              //马达执行计数

uint8_t      Vibration_count;


bit          batterySend_ena;                    //是否使能电量通知
bit          bWorkEna;                             //当前是否工作中

bit          bNonProgrammeMotors_Ena;        //非编程马达震动使能位
bit          bProgrammeMotors_Ena;          //编程马达使能位
bit          bDivider_Ena;
bit          bMotors_Ena;                     //马达震动使能位（在3.5v以上马达使能震动）

uint8_t      exampleCount;                  //电量采集周期

uint16_t     sec_base;                      //1s 基本时间
bit          bTime_base_out;                  //单位时间标志
uint8_t      xdata Buf[5];                      //反馈数据缓存

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTION PROTOTYPE                                                 */
/*--------------------------------------------------------------------------*/
uint16_t GetADC12BitValue(void);
void BLE_SendDataPacket(uint8_t *sendBuf ,uint8_t sendSize);
void detectCharge(void);
void ledHint(bit enable);
void motorShock(bit enable);
void BatterySend(bit enable);
void Work(bit enable);
void clear(void);
void reset(void);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS DEFINITION                                               */
/*--------------------------------------------------------------------------*/
/******************************************************
  delay
*******************************************************/
void Delay(uint16_t time)
{
    uint16_t i,j;
    for (i=0; i<time; i++)
    {
        for (j=0; j<time; j++)
        {
            ;
        }
    }
}

/**
*   获取mac地址
*/
void acquireMacAddress(void)
{
    int flashAddress = rdFlashAddress;
    uint8_t mac[8],i;
    
    //头部
    mac[0] = 0xbf;
    mac[1] = 0xbf;
    
    for(i = 2; i < 6; i++)
    {
        mac[i] = CBYTE[flashAddress ++];
    }
    
    BLE_SendDataPacket(mac,sizeof(mac));
}

/**
 *  检测充电，
**/
void detectCharge(void)
{
		P3OE  &= 0xFE;
	  P3PUN |= 0x01;

		P3_0 = 1;
//		Delay(3);
    if (P3_0 == 0)
    {
        // 没有充电
        Buf[0] = 0xcf;
        Buf[1] = 0xcf;
        Buf[2] = 0x00;
        BLE_SendDataPacket(Buf,3);
    }
    else
    {
        //正在充电
        Buf[0] = 0xcf;
        Buf[1] = 0xcf;
        Buf[2] = 0x01;
        BLE_SendDataPacket(Buf,3);
    }
}

// 得到0-100之间的值
uint8_t getRate(sint16_t value)
{
    if(value < 0)
        value = 0;
    else if(value > 100)
        value = 100;

    return value;
}

void sendBattery()
{
    uint16_t ADC_Val;
    uint8_t averge = 0;

    //返回数据给手机
    //获取电压的值。


    ADC_Val = GetADC12BitValue();
    
    
    averge = (ADC_Val - min) / (float)(MaxValue - min) * level;

    if(averge > 100)
        averge = 100;
    
    if(averge < 0)
        averge = 0;
    

//    averge = getRate((ADC_Val - min) / 5);///(max - min) * level;
//    averge = (float)(ADC_Val - min)/(float)(max - min) * level;

    Buf[0] = 0xcd;
    Buf[1] = 0XCD;
    Buf[2] = averge;
    Buf[3] = ADC_Val / 256;
    Buf[4] = ADC_Val % 256;

//   Buf[4] = 0xaa;
    BLE_SendDataPacket(Buf,5);
}


uint8_t type = 0;
void HandlerData(uint8_t *Buf)
{
    uint8_t tmp;


    //接受收据处理
    if(Buf[5]==0X82 && Buf[0]==0XBB)
    {
        tmp = (Buf[5]+Buf[4]+Buf[3]+Buf[2])&0xFF;

        if(Buf[1]==tmp)//校验数据
       {

        //判断是否有休息指令
        if((Buf[3] & 0x08) != 0)
        {

            //使能休眠标志位
            //	  setIntoSleepFlag(TRUE);
            IntoSleepFlag = TRUE;

            //断开连接
            BLE_SetTerminate();
					

            return;
        }
        
        //判断是获取mac地址
        if(Buf[3] & 0x10)
        {
            acquireMacAddress();
        }
        

        // 控制方式
        ConfigParam.ControlType = (emControlType)(Buf[3] & 0x03);

        //使能马达震动位bit2
        if(Buf[3] & 0x04)
        {
            bMotors_Ena = FALSE;
            clear();
        }
        else
        {
            bMotors_Ena = TRUE;
        }

        //当马达禁止震动时（低电压时）不接受指令操作。
        if(!bMotors_Ena) return;

        if (Buf[2] == 0)//有无基本编程
        {
            if (Buf[4] & 0x80)
            {
                //使能马达控制
                bNonProgrammeMotors_Ena = TRUE;
                //马达工作周期
                ConfigParam.Motors_duty_cycle = Buf[4] & 0x7f;
            }
            else
            {
                clear();
            }
        }
        else if (!(ConfigParam.ShockType & 0x0f))
        {
            bProgrammeMotors_Ena = TRUE;
            ConfigParam.ShockType = (emShockType)(Buf[2] & 0x0f);
            Vibration_count = 0;
        }
       }
    }
}

void clear(void)
{
    //停止震动
    bNonProgrammeMotors_Ena = FALSE;

    //震动方式
    ConfigParam.ShockType = 0;

    bProgrammeMotors_Ena = FALSE;
    Vibration_count = 0;
}

/*****************************************************************************
    @fn         uint16_t GetADC12BitValue(void)
    @brief      Obtain 12bit ADC value
    @param      N/A
    @return     ADC value
*****************************************************************************/
uint16_t GetADC12BitValue(void)
{
    uint16_t tmp;
    
    /* User can modify ADC analog input pin (Default Selection P3.1)*/
    P3OE &= 0xFD;
    P3PUN |= ~0xFD;
    ADCCH = 0x0F; // Enable ADC analog input and Select P3.1 as ADC analog input.

    /* ADC Setting and Running */
    XBYTE[EXT1_REG] = 0x58;     // Disable REGR
    XBYTE[ADCAVG1_REG] = 0x02; // Enable the SAR ADC
    XBYTE[ADCCTL_REG] = 0x01; // Single mode and ADC measurement enable
    tmp = 0;
    while (XBYTE[ADCCTL_REG] & 0x01);
    tmp = (XBYTE[ADCAVG1_REG] & 0xF0);
    tmp <<= 4;
    tmp |= XBYTE[ADCAVG2_REG];
    XBYTE[ADCCTL_REG] = 0x00;
    XBYTE[ADCAVG1_REG] = 0x00;
    ADCCH &= 0xFE; // Disable ADC analog input.
    return tmp;
}

void connected_init(void)
{
    if(!bWorkEna)
    {
        enable_timer1();
        Work(TRUE);
        sendBattery();
    }
}

void timer_detect(void)
{
    if (ConfigParam.ClkTickCtr >= 500)//50ms
    {
        ConfigParam.ClkTickCtr = 0;

        bTime_base_out = TRUE;
        sec_base ++;

        if (sec_base >= 30)//2s
        {
            if(sec_base == 30)
                detectCharge();

            if(sec_base >= 60) {
                sec_base = 0;
                sendBattery();
            }
        }
    }
}



void BatterySend(bit enable)
{
    batterySend_ena = enable;
}

void Work(bit enable)
{
    bWorkEna = enable;
}

/*
 *空中升级需要关闭定时器，以免影响升级
 */
void update_init(void)
{
    disable_timer1();
}


void pwm_init(void)
{
    motorShock(RESET);  // 马达控制初始为低

    ledHint(RESET);      // 工作指示


    ConfigParam.Motors_duty_cycle = 100;
    bNonProgrammeMotors_Ena = FALSE;
    bProgrammeMotors_Ena = FALSE;
    bDivider_Ena = FALSE;
    bMotors_Ena  = TRUE;
    ConfigParam.ClkTickCtr = 0;

    Motors_run_count = 0;

    bcut_run = 1;
    led_duty_cycle = 6;
    led_run_count = 0;


    Vibration_count = 0;
}

void reset(void)
{
    disable_timer1();
    Work(FALSE);
    ledHint(RESET);
    motorShock(DISABLE);


    ConfigParam.Motors_duty_cycle = 100;
    led_duty_cycle = 6;
    bProgrammeMotors_Ena = FALSE;
    bNonProgrammeMotors_Ena = FALSE;
    bDivider_Ena = FALSE;
    bMotors_Ena  = TRUE;
    ConfigParam.ShockType = sync;
    ConfigParam.ClkTickCtr = 0;
    Motors_run_count = 0;
}


void BLE_SendDataPacket(uint8_t *sendBuf ,uint8_t sendSize)
{
    if((att_HDL_USER_DEFINE_01_DATAN01_CLIENT_CHARACTERISTIC_CONFIGURATION[0] 
        & GATT_DESCRIPTORS_CLIENT_CHARACTERISTIC_CONFIGURATION_NOTIFICATION) != 0){
        BLE_SendData(sendBuf,ATT_HDL_USER_DEFINE_01_DATAN01_INIT,sendSize);
    }
}

void motorShock(bit enable)
{
    switch(ConfigParam.ControlType)
    {
    case left:
        MOTOR1 = enable;
        MOTOR2 = DISABLE;
        break;
    case right:
        MOTOR1 = DISABLE;
        MOTOR2 = enable;
        break;
    case sync:
        MOTOR1 = enable;
        MOTOR2 = enable;
        break;

    default:
        break;
    }
}

void ledHint(bit enable)
{
    LED_WORK_GREEN = enable;
}

void pwm_deal(void)
{

    if(!bDivider_Ena && bNonProgrammeMotors_Ena == TRUE)
    {
        Motors_run_count ++;
        if (Motors_run_count > 99)
        {
            Motors_run_count = 0;
        }
        if (Motors_run_count >= ConfigParam.Motors_duty_cycle)
        {
            motorShock(RESET);
        }
        else
        {
            motorShock(SET);
        }
    }
    else
    {
        motorShock(RESET);
    }


    //工作指示
    if (bWorkEna == TRUE)
    {
        led_run_count ++;
        if (led_run_count >= 150)		//
        {
            led_run_count = 0;
        }
        if (led_run_count >= led_duty_cycle)
        {
            ledHint(RESET);
        }
        else
        {
            ledHint(SET);
        }
    }
    else
    {
        led_run_count = 0;
    }
}

void mode_stop(void)
{
    Vibration_count = 0;
    bNonProgrammeMotors_Ena = FALSE;
    ConfigParam.Motors_duty_cycle = 0;

    Buf[0] = 0XAB;
    Buf[1] = 0XAB;
    Buf[2] = ConfigParam.ShockType;
    Buf[3] = ConfigParam.ShockType;
    BLE_SendDataPacket((uint8_t*)Buf,4);

    ConfigParam.ShockType = nc;
}


void sub_function(void)
{
    if(bNonProgrammeMotors_Ena == TRUE && bProgrammeMotors_Ena == FALSE)
    {
        if (bTime_base_out == SET)
        {
            if (Vibration_count % ConfigParam.Vibration_Stand == 0)
            {
                bDivider_Ena = !bDivider_Ena;
            }
            if (Vibration_count < 60)
            {
                Vibration_count ++;
            }
            else
            {
                Vibration_count = 0;
            }
        }

        bTime_base_out = RESET;
    }

    if(bProgrammeMotors_Ena != TRUE)
        return;


    bDivider_Ena = FALSE;

    switch(ConfigParam.ShockType)//振动
    {
    case soft://弱到强
        bNonProgrammeMotors_Ena = TRUE;
        if (bTime_base_out == SET)
        {
            if (ConfigParam.Motors_duty_cycle < 99)
            {
                ConfigParam.Motors_duty_cycle += 2;
            }
            else
            {
                mode_stop();
            }
        }

        break;
    case ripple://弱振
        bNonProgrammeMotors_Ena = TRUE;
        ConfigParam.Motors_duty_cycle = 50;
        if (bTime_base_out == SET)
        {
            if (Vibration_count < 60)
            {
                Vibration_count ++;
            }
            else
            {
                mode_stop();
            }
        }
        break;
    case slow://振0.2  停0.2
        if (bTime_base_out == SET)
        {
            Vibration_count ++;
            if (Vibration_count % 4 == 0)
            {
                bNonProgrammeMotors_Ena = !bNonProgrammeMotors_Ena;
                ConfigParam.Motors_duty_cycle = 100;
            }
            if (Vibration_count == 60)
            {
                mode_stop();
            }
        }
        break;
    case strong://强振
        bNonProgrammeMotors_Ena = TRUE;
        ConfigParam.Motors_duty_cycle = 100;
        if (bTime_base_out == SET)
        {
            if (Vibration_count < 60)
            {
                Vibration_count ++;

            }
            else
            {
                mode_stop();
            }
        }

        break;
    case dynamism: //动感
        ConfigParam.Motors_duty_cycle = 100;
//		bNonProgrammeMotors_Ena = TRUE;
        if (bTime_base_out == SET)
        {
            if (Vibration_count % ConfigParam.Vibration_Stand == 0)
            {
                bNonProgrammeMotors_Ena = !bNonProgrammeMotors_Ena;
            }
            if (Vibration_count < 60)
            {
                Vibration_count ++;
            }
            else
            {
                mode_stop();
            }
        }
        break;
    default:
        break;
    }

    bTime_base_out = RESET;
}




















