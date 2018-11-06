#include <string.h>
#include "config_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"

void config_mode_routin(void)
{
    char cmdbyte;
    char cmdbuf[6];
    // cfg gpio & radio
    GPIO_Init(SX1278_AUX_PORT, SX1278_AUX_PIN, GPIO_Mode_Out_PP_Low_Fast); // AUX mode output
    while(GetRunModePin() == En_Config_Mode)
    {
        if(ring_buffer_num_items(&uart_rx_ring_buf) > 0)
        {
            ring_buffer_dequeue(&uart_rx_ring_buf, &cmdbyte);
            switch(cmdbyte)
            {
              case 0xC0: // 掉电记忆
              case 0xC2: // 掉电不保存
                while(ring_buffer_num_items(&uart_rx_ring_buf) < 5);
                ring_buffer_dequeue_arr(&uart_rx_ring_buf,cmdbuf,5);
                memcpy((uint8_t *)&stTmpCfgParm,cmdbuf,5);
                if(cmdbyte == 0xC0)
                {
                    cfg_parm_restore();
                }
                break;
              case 0xC1:
                while(ring_buffer_num_items(&uart_rx_ring_buf) < 2);
                ring_buffer_dequeue_arr(&uart_rx_ring_buf,cmdbuf,2);
                if((cmdbuf[0] == 0xC1) && (cmdbuf[1] == 0xC1))
                {
                    cmdbuf[0] = 0xC0;
                    memcpy(cmdbuf + 1,(uint8_t*)&stTmpCfgParm,5);
                    ring_buffer_queue_arr(&uart_tx_ring_buf, (const char *)(cmdbuf + 1), 5);
                    USART_SendData8(USART1, cmdbuf[0]);
                }
                break;
              case 0xC3:
                while(ring_buffer_num_items(&uart_rx_ring_buf) < 2);
                ring_buffer_dequeue_arr(&uart_rx_ring_buf,cmdbuf,2);
                if((cmdbuf[0] == 0xC3) && (cmdbuf[1] == 0xC3))
                {
                    cmdbuf[0] = 0xC3;
                    cmdbuf[1] = 32; // 433mhz
                    cmdbuf[2] = 0;
                    cmdbuf[3] = 0;
                    ring_buffer_queue_arr(&uart_tx_ring_buf, (const char *)(cmdbuf + 1), 3);
                    USART_SendData8(USART1, cmdbuf[0]);
                }
                break;
              case 0xC4:
                while(ring_buffer_num_items(&uart_rx_ring_buf) < 2);
                ring_buffer_dequeue_arr(&uart_rx_ring_buf,cmdbuf,2);
                if((cmdbuf[0] == 0xC4) && (cmdbuf[1] == 0xC4))
                {
                    GPIO_ResetBits(SX1278_AUX_PORT, SX1278_AUX_PIN);
                }
                break;
              default:
                break;
            }
        }
    }
    // reset mcu and get run mode again
}