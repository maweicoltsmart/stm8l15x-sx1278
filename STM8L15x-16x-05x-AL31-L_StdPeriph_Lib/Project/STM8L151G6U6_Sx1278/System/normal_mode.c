#include "normal_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"

#define RF_FREQUENCY                                470000000 // Hz
#define TX_OUTPUT_POWER                             14        // dBm

#define LORA_BANDWIDTH                              2         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
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
static RadioEvents_t NormalModeRadioEvents;
States_t NormalModeState = LOWPOWER;
//__eeprom uint32_t factory = 'a';
/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void NormalModeOnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void NormalModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void NormalModeOnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void NormalModeOnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void NormalModeOnRxError( void );


void normal_mode_routin(void)
{
    static char RadioTxBuffer[58];
    uint8_t RadioTxLen = 0;
    TimerTime_t timestamp;
    // cfg gpio & radio    
    // Radio initialization
    NormalModeRadioEvents.TxDone = NormalModeOnTxDone;
    NormalModeRadioEvents.RxDone = NormalModeOnRxDone;
    NormalModeRadioEvents.TxTimeout = NormalModeOnTxTimeout;
    NormalModeRadioEvents.RxTimeout = NormalModeOnRxTimeout;
    NormalModeRadioEvents.RxError = NormalModeOnRxError;

    Radio.Init( &NormalModeRadioEvents );
    //factory = 433000000;
    Radio.SetChannel( stTmpCfgParm.channel.channelbit.channelno * 1000000 + 410000000 );
    Radio.SetTxConfig( MODEM_LORA, cfg_parm_get_tx_power(), 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
    Radio.Rx( 0 ); // 0: receive RxContinuous
    while(GetRunModePin() == En_Normal_Mode)
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
                RadioTxLen = ring_buffer_dequeue_arr(&uart_rx_ring_buf,RadioTxBuffer,58);
                Radio.Send( (uint8_t*)RadioTxBuffer, RadioTxLen );
            }
        }
    }
    // reset mcu and get run mode again
    WWDG->CR = 0x80;//WDGA=1执行这一句后，无条件软复位
}

void NormalModeOnTxDone( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    NormalModeState = TX;
}

void NormalModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    uint8_t temp = 0;
    Radio.Sleep( );
    ring_buffer_queue_arr(&uart_tx_ring_buf, (const char *)payload, size);
    Radio.Rx( 0 );
    ring_buffer_dequeue(&uart_tx_ring_buf, &temp);
    USART_SendData8(USART1, temp);
    USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    /*BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;*/
    NormalModeState = RX;
}

void NormalModeOnTxTimeout( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    NormalModeState = TX_TIMEOUT;
}

void NormalModeOnRxTimeout( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    NormalModeState = RX_TIMEOUT;
}

void NormalModeOnRxError( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    NormalModeState = RX_ERROR;
}