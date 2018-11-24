/**
  ******************************************************************************
  * @file    delay.c
  * @author  Microcontroller Division
  * @version V1.2.0
  * @date    09/2010
  * @brief   delay functions
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */

/* Includes ------------------------------------------------------------------*/

#include "stm8l15x_clk.h"
/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "delay.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TIM2_PERIOD  (uint8_t) 7

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t TimingDelay;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @addtogroup TIMING_DELAY_Functions
  * @{
  */

/**
  * @brief  timing delay init:to generate 1 ms time base using TIM2 update interrupt
  * @note   The external low speed clock (LSI) is used to ensure timing accuracy.
  *         This function should be updated in case of use of other clock source.      
  * @param  None
  * @retval None
  */
#if 0
void TimingDelay_Init(void)
{
  /* Enable TIM2 clock */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM2, ENABLE);

  /* Remap TIM2 ETR to LSI: TIM2 external trigger becomes controlled by LSI clock */
  //SYSCFG_REMAPPinConfig(REMAP_Pin_TIM2TRIGLSI, ENABLE);

  /* Enable LSI clock */
  //CLK_LSIConfig(CLK_LSI_ON);
  /* Wait for LSIRDY flag to be reset */
  //while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);

  /* TIM2 configuration:
     - TIM2 ETR is mapped to LSI
     - TIM2 counter is clocked by LSI div 4
      so the TIM2 counter clock used is LSI / 4 = 32.768 / 4 = 8.192 KHz
     TIM2 Channel1 output frequency = TIM2CLK / (TIM2 Prescaler * (TIM2_PERIOD + 1))
                                    = 8192 / (1 * 8) = 1024 Hz                */
                                    
  /* Time Base configuration */
  TIM2_TimeBaseInit(TIM2_Prescaler_1, TIM2_CounterMode_Up, TIM2_PERIOD);
  TIM2_ETRClockMode2Config(TIM2_ExtTRGPSC_DIV4, TIM2_ExtTRGPolarity_NonInverted, 0);

  TIM2_UpdateRequestConfig(TIM2_UpdateSource_Global);

  /* Clear TIM2 update flag */
  TIM2_ClearFlag(TIM2_FLAG_Update);

  /* Enable update interrupt */
  TIM2_ITConfig(TIM2_IT_Update, ENABLE);

  TIM2_Cmd(ENABLE);
}
#endif
/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;
  while (TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @note   This function should be called in the
  *         TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQHandler in the stm8l15x_it.c file.
  *
  *       // INTERRUPT_HANDLER(TIM2_UPD_OVF_TRG_BRK_USART2_TX_IRQHandler, 19)
  *       // {
  *           // TimingDelay_Decrement(); 
  *           // TIM2_ClearITPendingBit(TIM2_IT_Update);
  *             
  *       // }  
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
}

void DelayMs( uint32_t ms )
{
    for(int16_t i = 0;i < 2200;i ++);
}

void delay_10us(u16 n_10us)
{
/* Init TIMER 4 */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

/* prescaler: / (2^0) = /1 */
  TIM4->PSCR = 0;

/* SYS_CLK_HSI_DIV1 Auto-Reload value: 16M / 1 = 16M, 16M / 100k = 160 */
  TIM4->ARR = 160;

/* Counter value: 10, to compensate the initialization of TIMER */
  TIM4->CNTR = 10;

/* clear update flag */
  TIM4->SR1 &= ~TIM4_SR1_UIF;

/* Enable Counter */
  TIM4->CR1 |= TIM4_CR1_CEN;

  while(n_10us--)
  {
    while((TIM4->SR1 & TIM4_SR1_UIF) == 0) ;
    TIM4->SR1 &= ~TIM4_SR1_UIF;
  }

/* Disable Counter */
  TIM4->CR1 &= ~TIM4_CR1_CEN;
 CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);

}