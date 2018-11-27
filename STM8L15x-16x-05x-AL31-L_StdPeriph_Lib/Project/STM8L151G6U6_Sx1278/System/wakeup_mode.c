#include "wakeup_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"
#include <stdio.h>

void wakeup_mode_routin(void)
{
    static char RadioTxBuffer[58];
    uint8_t RadioTxLen = 0;
    TimerTime_t timestamp;
    BoardDisableIrq();
    TIM4_Config();
    RTC_Config();
    ComportInit();
    BoardEnableIrq();
    printf("wakeup\r\n");
    while(GetRunModePin() == En_Wake_Up_Mode)
    {
        if(RadioTxLen != ring_buffer_num_items(&uart_rx_ring_buf))
        {
            RadioTxLen = ring_buffer_num_items(&uart_rx_ring_buf);
            timestamp = TimerGetCurrentTime();
        }
        else
        {
            if((RadioTxLen >= 58) || ((TimerGetElapsedTime(timestamp) > 3 * 8 * 1000 / (float)cfg_parm_get_uart_baud()) && (RadioTxLen > 0)))
            {
                BoardDisableIrq();
                BoardEnableIrq();
            }
        }
    }
}