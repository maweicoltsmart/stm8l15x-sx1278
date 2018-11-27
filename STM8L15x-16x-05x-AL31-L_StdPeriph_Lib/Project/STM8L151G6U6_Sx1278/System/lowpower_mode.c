#include "lowpower_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"
#include <stdio.h>

void lowpower_mode_routin(void)
{
    PWR_UltraLowPowerCmd(ENABLE);
    BoardDisableIrq();
    TIM4_Config();
    /* RTC configuration -------------------------------------------*/
    RTC_Config();
    ComportInit();
    BoardEnableIrq();
    printf("lowpower\r\n");
    while(GetRunModePin() == En_Low_Power_Mode)
    {
    }
}