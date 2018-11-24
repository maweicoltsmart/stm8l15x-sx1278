#include "board.h"
#include "comport.h"
#include "spi-board.h"
#include "sx1276-board.h"
#include "cfg_parm.h"
#include "delay.h"
#include "system.h"

/**
  * @brief  Configure TIM4 peripheral   
  * @param  None
  * @retval None
  */
#define TIM4_PERIOD       124
__IO uint32_t TimingDelay;

void RTC_Config(void)
{
  /* Enable RTC clock */
  CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_1);
  /* Wait for LSI clock to be ready */
  //while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
  /* wait for 1 second for the LSE Stabilisation */
  //LSI_StabTime();
  CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);

  /* Configures the RTC wakeup timer_step = RTCCLK/16 = LSE/16 = 488.28125 us */
  RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
  //RTC_SetWakeUpCounter((uint16_t)(cfg_parm_get_wakeup_time() * 1000.0 / 488.28125) - 1);
  /* Enable wake up unit Interrupt */
  RTC_ClearITPendingBit(RTC_IT_WUT);
  if(GetRunModePin() == En_Low_Power_Mode)
  {
      RTC_ITConfig(RTC_IT_WUT, ENABLE);
      RTC_WakeUpCmd(DISABLE);
      RTC_SetWakeUpCounter((uint16_t)(cfg_parm_get_wakeup_time() * 1000.0 / 488.28125) - 1);
      RTC_WakeUpCmd(ENABLE);
      CLK_LSICmd(ENABLE);
  }
  else
  {
      RTC_ITConfig(RTC_IT_WUT, DISABLE);
      RTC_WakeUpCmd(DISABLE);
      CLK_LSICmd(DISABLE);
      CLK_PeripheralClockConfig(CLK_Peripheral_RTC, DISABLE);
      return;
  }
}

void TIM4_Config(void)
{
  /* TIM4 configuration:
   - TIM4CLK is set to 16 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
   clock used is 16 MHz / 128 = 125 000 Hz
  - With 125 000 Hz we can generate time base:
      max time base is 2.048 ms if TIM4_PERIOD = 255 --> (255 + 1) / 125000 = 2.048 ms
      min time base is 0.016 ms if TIM4_PERIOD = 1   --> (  1 + 1) / 125000 = 0.016 ms
  - In this example we need to generate a time base equal to 1 ms
   so TIM4_PERIOD = (0.001 * 125000 - 1) = 124 */
  /* Enable TIM4 CLK */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
  /* Time base configuration */
  TIM4_TimeBaseInit(TIM4_Prescaler_128, TIM4_PERIOD);
  /* Clear TIM4 update flag */
  TIM4_ClearFlag(TIM4_FLAG_Update);
  /* Enable update interrupt */
  TIM4_ClearITPendingBit(TIM4_IT_Update);
  if(GetRunModePin() == En_Config_Mode)
  {
      CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);
      TIM4_ITConfig(TIM4_IT_Update, DISABLE);
  }
  else
  {
      TIM4_ITConfig(TIM4_IT_Update, ENABLE);
  }
  TIM4_Cmd(ENABLE);
}

void InitRunModePin(void)
{
    GPIO_Init(SX1278_TEST_PORT, SX1278_TEST_PIN, GPIO_Mode_In_FL_No_IT); // test mode input
    GPIO_Init(SX1278_M0_PORT, SX1278_M0_PIN, GPIO_Mode_In_PU_No_IT); // M0 mode input
    GPIO_Init(SX1278_M1_PORT, SX1278_M1_PIN, GPIO_Mode_In_PU_No_IT); // M1 mode input
}

Run_Mode_Type GetRunModePin(void)
{
    if(GPIO_ReadInputDataBit(SX1278_TEST_PORT, SX1278_TEST_PIN))
      return En_Test_Mode;
    else if((GPIO_ReadInputDataBit(SX1278_M0_PORT, SX1278_M0_PIN) == 0) && (GPIO_ReadInputDataBit(SX1278_M1_PORT, SX1278_M1_PIN) == 0))
      return En_Normal_Mode;
    else if((GPIO_ReadInputDataBit(SX1278_M0_PORT, SX1278_M0_PIN)) && (GPIO_ReadInputDataBit(SX1278_M1_PORT, SX1278_M1_PIN) == 0))
      return En_Wake_Up_Mode;
    else if((GPIO_ReadInputDataBit(SX1278_M0_PORT, SX1278_M0_PIN) == 0) && (GPIO_ReadInputDataBit(SX1278_M1_PORT, SX1278_M1_PIN)))
      return En_Low_Power_Mode;
    else
      return En_Config_Mode;
    //return En_Low_Power_Mode;
}

void BoardInitMcu( void )
{
    BoardDisableIrq();
    CLK_SYSCLKSourceSwitchCmd(ENABLE);
    CLK_HSICmd(ENABLE);
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
    /* system clock prescaler: 1*/
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
    while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI)
    {}
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOC);
    GPIO_DeInit(GPIOD);
    InitRunModePin();
    GPIO_Init(SX1278_IO6_PORT, SX1278_IO6_PIN, GPIO_Mode_Out_PP_Low_Fast); // IO6
    GPIO_Init(SX1278_IO5_PORT, SX1278_IO5_PIN, GPIO_Mode_Out_PP_Low_Fast); // IO5
    GPIO_Init(SX1278_IO4_PORT, SX1278_IO4_PIN, GPIO_Mode_Out_PP_Low_Fast); // IO4
    GPIO_Init(SX1278_IO3_PORT, SX1278_IO3_PIN, GPIO_Mode_Out_PP_Low_Fast); // IO3
    GPIO_Init(SX1278_IO2_PORT, SX1278_IO2_PIN, GPIO_Mode_Out_PP_Low_Fast); // IO2
    GPIO_Init(SX1278_IO1_PORT, SX1278_IO1_PIN, GPIO_Mode_Out_PP_Low_Fast); // IO1
    cfg_parm_factory_reset();
    cfg_parm_dump_to_ram();
    // ComportInit();
    if(stTmpCfgParm.option.optionbit.io_pushpull == 1)
    {
        GPIO_Init(SX1278_AUX_PORT, SX1278_AUX_PIN, GPIO_Mode_Out_PP_High_Fast); // AUX mode output
    }
    else
    {
        GPIO_Init(SX1278_AUX_PORT, SX1278_AUX_PIN, GPIO_Mode_Out_OD_HiZ_Fast); // AUX mode output
    }
    
    
    BEEP_Cmd(DISABLE);
    BEEP_LSClockToTIMConnectCmd(ENABLE);
    BEEP_LSICalibrationConfig(32768);
    //TIM4_Config();
    SpiInit( );
    SX1276IoInit( );
    BoardEnableIrq();
}