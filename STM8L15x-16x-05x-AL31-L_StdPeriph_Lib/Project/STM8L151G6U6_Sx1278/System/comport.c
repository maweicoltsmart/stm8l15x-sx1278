#include "comport.h"
#include "stm8l15x_clk.h"
#include "ring_buf.h"
#include "cfg_parm.h"
#include "board.h"

ring_buffer_t uart_rx_ring_buf,uart_tx_ring_buf;

void ComportInit(void)
{
    ring_buffer_init(&uart_rx_ring_buf);
    ring_buffer_init(&uart_tx_ring_buf);
    /* Enable USART clock */
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);
    
    /* USART configuration */
    if((GetRunModePin() == En_Test_Mode) || (GetRunModePin() == En_Config_Mode))
    {
        USART_Init(USART1, 9600,
                  USART_WordLength_8b,
                  USART_StopBits_1,
                  USART_Parity_No,
                  (USART_Mode_TypeDef)(USART_Mode_Rx | USART_Mode_Tx));
    }
    else
    {
        USART_Init(USART1, cfg_parm_get_uart_baud(),
                  USART_WordLength_8b,
                  USART_StopBits_1,
                  cfg_parm_get_uart_parity(),
                  (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx));
        if(stTmpCfgParm.option.io_pushpull == 1)
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
    }
    /* Enable the USART Receive interrupt: this interrupt is generated when the USART
      receive data register is not empty */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    /* Enable the USART Transmit complete interrupt: this interrupt is generated when the USART
    transmit Shift Register is empty */
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);

    /* Enable USART */
    USART_Cmd(USART1, ENABLE);
}