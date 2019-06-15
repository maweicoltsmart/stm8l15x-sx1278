#include "comport.h"
#include "stm8l15x_clk.h"
#include "ring_buf.h"
#include "cfg_parm.h"
#include "board.h"
#include "delay.h"

ring_buffer_t uart_rx_ring_buf;//,uart_tx_ring_buf;

void ComportInit(void)
{
    GPIO_Init(SX1278_RX_PORT, SX1278_RX_PIN, GPIO_Mode_In_PU_No_IT); // UART RX
    USART_Cmd(USART1, DISABLE);
    USART_DeInit(USART1);
    ring_buffer_init(&uart_rx_ring_buf);
    //ring_buffer_init(&uart_tx_ring_buf);
    /* Enable USART clock */
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
    
    /* USART configuration */
    if((GetRunModePin() == En_Test_Mode)  || (GetRunModePin() == En_Config_Mode)  || (stTmpCfgParm.inNetMode == TRUE))
    {
        USART_Init(USART1, 9600,
                  USART_WordLength_8b,
                  USART_StopBits_1,
                  USART_Parity_No,
                  (USART_Mode_TypeDef)(USART_Mode_Rx | USART_Mode_Tx));
        /* Configure USART Tx as alternate function push-pull  (software pull up)*/
        GPIO_ExternalPullUpConfig(SX1278_TX_PORT, SX1278_TX_PIN, ENABLE);

        /* Configure USART Rx as alternate function push-pull  (software pull up)*/
        GPIO_ExternalPullUpConfig(SX1278_RX_PORT, SX1278_RX_PIN, ENABLE);
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
        EXTI_SetPinSensitivity(EXTI_Pin_2, EXTI_Trigger_Falling); // UART RX
        GPIO_Init(SX1278_RX_PORT, SX1278_RX_PIN, GPIO_Mode_In_PU_IT); // UART RX
    }
    else if(GetRunModePin() == En_Low_Power_Mode)
    {
        USART_Init(USART1, cfg_parm_get_uart_baud(),
                  USART_WordLength_8b,
                  USART_StopBits_1,
                  cfg_parm_get_uart_parity(),
                  (USART_Mode_TypeDef)(USART_Mode_Rx | USART_Mode_Tx));
        if(stTmpCfgParm.option.optionbit.io_pushpull == 1)
        {
            /* Configure USART Tx as alternate function push-pull  (software pull up)*/
            GPIO_ExternalPullUpConfig(SX1278_TX_PORT, SX1278_TX_PIN, ENABLE);

            /* Configure USART Rx as alternate function push-pull  (software pull up)*/
            GPIO_ExternalPullUpConfig(SX1278_RX_PORT, SX1278_RX_PIN, ENABLE);
        }
        else
        {
            /* Configure USART Tx as alternate function push-pull  (software pull up)*/
            GPIO_ExternalPullUpConfig(SX1278_TX_PORT, SX1278_TX_PIN, DISABLE);

            /* Configure USART Rx as alternate function push-pull  (software pull up)*/
            GPIO_ExternalPullUpConfig(SX1278_RX_PORT, SX1278_RX_PIN, DISABLE);
        }
        USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
    }
    else // normal mode & wakeup mode
    {
        USART_Init(USART1, cfg_parm_get_uart_baud(),
                  USART_WordLength_8b,
                  USART_StopBits_1,
                  cfg_parm_get_uart_parity(),
                  (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));
        if(stTmpCfgParm.option.optionbit.io_pushpull == 1)
        {
            /* Configure USART Tx as alternate function push-pull  (software pull up)*/
            GPIO_ExternalPullUpConfig(SX1278_TX_PORT, SX1278_TX_PIN, ENABLE);

            /* Configure USART Rx as alternate function push-pull  (software pull up)*/
            GPIO_ExternalPullUpConfig(SX1278_RX_PORT, SX1278_RX_PIN, ENABLE);
        }
        else
        {
            /* Configure USART Tx as alternate function push-pull  (software pull up)*/
            GPIO_ExternalPullUpConfig(SX1278_TX_PORT, SX1278_TX_PIN, DISABLE);

            /* Configure USART Rx as alternate function push-pull  (software pull up)*/
            GPIO_ExternalPullUpConfig(SX1278_RX_PORT, SX1278_RX_PIN, DISABLE);
        }
        /* Enable the USART Receive interrupt: this interrupt is generated when the USART
          receive data register is not empty */
        USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    }
    /* Enable USART */
    USART_Cmd(USART1, ENABLE);
}

#ifdef _RAISONANCE_
#define PUTCHAR_PROTOTYPE int putchar (char c)
#define GETCHAR_PROTOTYPE int getchar (void)
#elif defined (_COSMIC_)
#define PUTCHAR_PROTOTYPE char putchar (char c)
#define GETCHAR_PROTOTYPE char getchar (void)
#else /* _IAR_ */
#define PUTCHAR_PROTOTYPE int putchar (int c)
#define GETCHAR_PROTOTYPE int getchar (void)
#endif /* _RAISONANCE_ */

/**
  * @brief Retargets the C library printf function to the USART.
  * @param[in] c Character to send
  * @retval char Character sent
  * @par Required preconditions:
  * - None
  */
PUTCHAR_PROTOTYPE
{
  USART_ClearFlag(USART1,USART_FLAG_TC);
  /* Write a character to the USART */
  USART_SendData8(USART1, c);
  /* Loop until the end of transmission */
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

  return (c);
}
/**
  * @brief Retargets the C library scanf function to the USART.
  * @param[in] None
  * @retval char Character to Read
  * @par Required preconditions:
  * - None
  */
GETCHAR_PROTOTYPE
{
  int c = 0;
  /* Loop until the Read data register flag is SET */
  while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
    c = USART_ReceiveData8(USART1);
    return (c);
 }
/*
void ComportTxStart(void)
{
    char temp = 0;
    if((USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET) && (!ring_buffer_is_empty(&uart_tx_ring_buf)))
    {
        GPIO_ResetBits(SX1278_AUX_PORT, SX1278_AUX_PIN);
        DelayMs(2);
        ring_buffer_dequeue(&uart_tx_ring_buf, &temp);
        USART_SendData8(USART1, temp);
        USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    }
}*/