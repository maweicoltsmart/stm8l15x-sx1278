#include "normal_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"
#include <stdio.h>
#include <string.h>
#include "platform.h"

#define BUFFER_SIZE                                 9 // Define the payload size here


static uint16_t BufferSize = BUFFER_SIZE;			// RF buffer size
static uint8_t Buffer[BUFFER_SIZE];					// RF buffer

static uint8_t EnableMaster = true; 				// Master/Slave selection

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

void normal_mode_routin(void)
{
    static char RadioTxBuffer[58];
    uint8_t RadioTxLen = 0;
    TimerTime_t timestamp;
    BoardDisableIrq();
    TIM4_Config();
    RTC_Config();
    ComportInit();
    BoardEnableIrq();
    Radio->Init( );
    Radio->StartRx( );
    printf("normal\r\n");
    while(GetRunModePin() == En_Normal_Mode)
    {
        uint8_t i;
        
        switch( Radio->Process( ) )
        {
        case RF_RX_TIMEOUT:
            // Send the next PING frame
            Buffer[0] = 'P';
            Buffer[1] = 'I';
            Buffer[2] = 'N';
            Buffer[3] = 'G';
            for( i = 4; i < BufferSize; i++ )
            {
                Buffer[i] = i - 4;
            }
            Radio->SetTxPacket( Buffer, BufferSize );
            break;
        case RF_RX_DONE:
            Radio->GetRxPacket( Buffer, ( uint16_t* )&BufferSize );
        
            if( BufferSize > 0 )
            {
                if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 )
                {
                    // Indicates on a LED that the received frame is a PONG
                    //LedToggle( LED_GREEN );

                    // Send the next PING frame            
                    Buffer[0] = 'P';
                    Buffer[1] = 'I';
                    Buffer[2] = 'N';
                    Buffer[3] = 'G';
                    // We fill the buffer with numbers for the payload 
                    for( i = 4; i < BufferSize; i++ )
                    {
                        Buffer[i] = i - 4;
                    }
                    Radio->SetTxPacket( Buffer, BufferSize );
                }
                else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                { // A master already exists then become a slave
                    EnableMaster = false;
                    //LedOff( LED_RED );
                }
            }            
            break;
        case RF_TX_DONE:
            // Indicates on a LED that we have sent a PING
            //LedToggle( LED_RED );
            Radio->StartRx( );
            break;
        default:
            break;
        }
#if 0
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
#endif
    }
}