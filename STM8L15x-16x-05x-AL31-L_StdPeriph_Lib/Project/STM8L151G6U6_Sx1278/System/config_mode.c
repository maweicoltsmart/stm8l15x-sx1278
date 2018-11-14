#include <string.h>
#include "config_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"
#include "delay.h"
#include <stdio.h>

static RadioEvents_t ConfigModeRadioEvents;

void config_mode_routin(void)
{
    char cmdbyte;
    char cmdbuf[6];
    BoardDisableIrq();
    USART_Cmd(USART1, DISABLE);
    ConfigModeRadioEvents.TxDone = NULL;
    ConfigModeRadioEvents.RxDone = NULL;
    ConfigModeRadioEvents.TxTimeout = NULL;
    ConfigModeRadioEvents.RxTimeout = NULL;
    ConfigModeRadioEvents.RxError = NULL;
    ConfigModeRadioEvents.CadDone = NULL;

    RTC_ITConfig(RTC_IT_WUT, DISABLE);

    Radio.Init( &ConfigModeRadioEvents );
    Radio.Sleep( );
    USART_DeInit(USART1);
    USART_Init(USART1, 9600,
                  USART_WordLength_8b,
                  USART_StopBits_1,
                  USART_Parity_No,
                  (USART_Mode_TypeDef)(USART_Mode_Tx));
    /* Configure USART Tx as alternate function push-pull  (software pull up)*/
    GPIO_ExternalPullUpConfig(SX1278_TX_PORT, SX1278_TX_PIN, ENABLE);
    USART_Cmd(USART1, ENABLE);
    EXTI_SetPinSensitivity(EXTI_Pin_2, EXTI_Trigger_Falling); // UART RX
    GPIO_Init(SX1278_RX_PORT, SX1278_RX_PIN, GPIO_Mode_In_FL_IT); // UART RX
    BoardEnableIrq();
    // cfg gpio & radio
    GPIO_Init(SX1278_AUX_PORT, SX1278_AUX_PIN, GPIO_Mode_Out_PP_Low_Fast); // AUX mode output
    while(GetRunModePin() == En_Config_Mode)
    {
        halt();
        if(ring_buffer_num_items(&uart_rx_ring_buf) > 0)
        {
            ring_buffer_dequeue(&uart_rx_ring_buf, &cmdbyte);
            switch(cmdbyte)
            {
              case 0xC0: // 掉电记忆
              case 0xC2: // 掉电不保存
                while(ring_buffer_num_items(&uart_rx_ring_buf) < 5)
                {
                    halt();
                }
                ring_buffer_dequeue_arr(&uart_rx_ring_buf,cmdbuf,5);
                stTmpCfgParm.addr_h = cmdbuf[0];
                stTmpCfgParm.addr_l = cmdbuf[1];
                stTmpCfgParm.speed.speed = cmdbuf[2];
                stTmpCfgParm.channel.channel = cmdbuf[3];
                stTmpCfgParm.option.option = cmdbuf[4];
                if(cmdbyte == 0xC0)
                {
                    cfg_parm_restore();
                }
                putchar(cmdbyte);
                for(uint8_t i = 0;i < 5;i ++)
                {
                    putchar(cmdbuf[i]);
                }
                break;
              case 0xC1:
                while(ring_buffer_num_items(&uart_rx_ring_buf) < 2)
                {
                    halt();
                }
                ring_buffer_dequeue_arr(&uart_rx_ring_buf,cmdbuf,2);
                if((cmdbuf[0] == 0xC1) && (cmdbuf[1] == 0xC1))
                {
                    cmdbuf[0] = 0xC0;
                    cmdbuf[1] = stTmpCfgParm.addr_h;
                    cmdbuf[2] = stTmpCfgParm.addr_l;
                    cmdbuf[3] = stTmpCfgParm.speed.speed;
                    cmdbuf[4] = stTmpCfgParm.channel.channel;
                    cmdbuf[5] = stTmpCfgParm.option.option;
                    for(uint8_t i = 0;i < 6;i ++)
                    {
                        putchar(cmdbuf[i]);
                    }
                }
                break;
              case 0xC3:
                while(ring_buffer_num_items(&uart_rx_ring_buf) < 2)
                {
                    halt();
                }
                ring_buffer_dequeue_arr(&uart_rx_ring_buf,cmdbuf,2);
                if((cmdbuf[0] == 0xC3) && (cmdbuf[1] == 0xC3))
                {
                    cmdbuf[0] = 0xC3;
                    cmdbuf[1] = 0x32; // 433mhz
                    cmdbuf[2] = 0x44;
                    cmdbuf[3] = 0x14;
                    for(uint8_t i = 0;i < 4;i ++)
                    {
                        putchar(cmdbuf[i]);
                    }
                }
                break;
              case 0xC4:
                while(ring_buffer_num_items(&uart_rx_ring_buf) < 2)
                {
                    halt();
                }
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