#include "wakeup_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"
#include <stdio.h>

#define RF_FREQUENCY                                470000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       12         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        100         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;

#define RX_TIMEOUT_VALUE                            1000

/*!
 * Radio events function pointer
 */
static RadioEvents_t WakeUpModeRadioEvents;
States_t WakeUpModeState = LOWPOWER;
//__eeprom uint32_t factory = 'a';
/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void WakeUpModeOnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void WakeUpModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void WakeUpModeOnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void WakeUpModeOnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void WakeUpModeOnRxError( void );

void WakeUpModeOnCadDone( bool channelActivityDetected );

void wakeup_mode_routin(void)
{
    static char RadioTxBuffer[58];
    uint8_t RadioTxLen = 0;
    TimerTime_t timestamp;
    // cfg gpio & radio    
    // Radio initialization
    WakeUpModeRadioEvents.TxDone = WakeUpModeOnTxDone;
    WakeUpModeRadioEvents.RxDone = WakeUpModeOnRxDone;
    WakeUpModeRadioEvents.TxTimeout = WakeUpModeOnTxTimeout;
    WakeUpModeRadioEvents.RxTimeout = WakeUpModeOnRxTimeout;
    WakeUpModeRadioEvents.RxError = WakeUpModeOnRxError;
    BoardDisableIrq();
    Radio.Init( &WakeUpModeRadioEvents );
    //factory = 433000000;
    Radio.SetChannel( stTmpCfgParm.channel.channelbit.channelno * 1000000 + 410000000 );
    Radio.SetTxConfig( MODEM_LORA, cfg_parm_get_tx_power(), 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, true, 8, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, true, 8, LORA_IQ_INVERSION_ON, true );
    Radio.Rx( 0 ); // 0: receive RxContinuous
    BoardEnableIrq();
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
                if(Radio.GetStatus() != RF_TX_RUNNING)
                {
                    RadioTxLen = ring_buffer_dequeue_arr(&uart_rx_ring_buf,RadioTxBuffer,58);
                    //BoardDisableIrq();
                    Radio.Send( (uint8_t*)RadioTxBuffer, RadioTxLen );
                    //BoardEnableIrq();
                }
                BoardEnableIrq();
            }
        }
    }
    // reset mcu and get run mode again
    WWDG->CR = 0x80;//WDGA=1执行这一句后，无条件软复位
}

void WakeUpModeOnTxDone( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    WakeUpModeState = TX;
}

void WakeUpModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    char temp = 0;
    
    ring_buffer_queue_arr(&uart_tx_ring_buf, (const char *)payload, size);
    Radio.Sleep( );
    Radio.Rx( 0 );
    if(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET)
    {
        ring_buffer_dequeue(&uart_tx_ring_buf, &temp);
        USART_SendData8(USART1, temp);
        USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    }
}

void WakeUpModeOnTxTimeout( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    WakeUpModeState = TX_TIMEOUT;
}

void WakeUpModeOnRxTimeout( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    WakeUpModeState = RX_TIMEOUT;
}

void WakeUpModeOnRxError( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    WakeUpModeState = RX_ERROR;
}