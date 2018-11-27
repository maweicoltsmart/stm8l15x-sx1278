#include "test_mode.h"
#include "board.h"
#include "stdio.h"
#include "radio.h"
#include "cfg_parm.h"
#include "manufacture_parm.h"
#include "delay.h"
#include "comport.h"

void test_mode_routin(void)
{
    // cfg gpio & radio
    RTC_Config();
    TIM4_Config();
    ComportInit();
    cfg_parm_factory_reset();
    stManufactureParm.softwareversion = VERSION_STR;
    GPIO_Init(SX1278_AUX_PORT, SX1278_AUX_PIN, GPIO_Mode_In_FL_No_IT); // AUX mode input
    printf("test\r\n");
    while(GetRunModePin() == En_Test_Mode)
    {
        GPIO_ResetBits(SX1278_IO1_PORT, SX1278_IO1_PIN);
        DelayMs( 1 );
        GPIO_SetBits(SX1278_IO1_PORT, SX1278_IO1_PIN);
        DelayMs( 1 );
    }
}