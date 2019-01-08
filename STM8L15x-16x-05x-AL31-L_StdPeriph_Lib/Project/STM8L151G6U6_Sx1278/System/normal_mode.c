#include "normal_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"
#include <stdio.h>
#include "crc8.h"

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
    uint8_t RadioTxLen = 0;
    TimerTime_t timestamp;
    // cfg gpio & radio    
    // Radio initialization
    NormalModeRadioEvents.TxDone = NormalModeOnTxDone;
    NormalModeRadioEvents.RxDone = NormalModeOnRxDone;
    NormalModeRadioEvents.TxTimeout = NormalModeOnTxTimeout;
    NormalModeRadioEvents.RxTimeout = NormalModeOnRxTimeout;
    NormalModeRadioEvents.RxError = NormalModeOnRxError;
    BoardDisableIrq();
    TIM4_Config();
    RTC_Config();
    Radio.Init( &NormalModeRadioEvents );
    //factory = 433000000;
    Radio.SetChannel( stTmpCfgParm.channel.channelbit.channelno * 1000000 + 410000000 );
    Radio.SetTxConfig( MODEM_LORA, cfg_parm_get_tx_power(), 0, cfg_parm_get_air_bandwith(),
                                   cfg_parm_get_air_sf(), LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, false, 0, LORA_IQ_INVERSION_ON, 3000 );

    Radio.SetRxConfig( MODEM_LORA, cfg_parm_get_air_bandwith(), cfg_parm_get_air_sf(),
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, false, 0, LORA_IQ_INVERSION_ON, true );
    Radio.Rx( 0 ); // 0: receive RxContinuous
    ComportInit();
    BoardEnableIrq();
    printf("normal\r\n");
    GPIO_SetBits(SX1278_AUX_PORT, SX1278_AUX_PIN);
    while((GetRunModePin() == En_Normal_Mode) || (!ring_buffer_is_empty(&uart_rx_ring_buf)) || (!ring_buffer_is_empty(&uart_tx_ring_buf)) || (Radio.GetStatus() == RF_TX_RUNNING))
    {
        ClearWWDG();
        IndicationRfTxFifoStatus();
        if(RadioTxLen != ring_buffer_num_items(&uart_rx_ring_buf))
        {
            RadioTxLen = ring_buffer_num_items(&uart_rx_ring_buf);
            timestamp = TimerGetCurrentTime();
        }
        else
        {
            if((RadioTxLen >= 58 + 3) || ((TimerGetElapsedTime(timestamp) > 3 * 8 * 1000 / (float)cfg_parm_get_uart_baud()) && (RadioTxLen > 0)))
            {
                BoardDisableIrq();
                if(Radio.GetStatus() != RF_TX_RUNNING)
                {
                    if(stTmpCfgParm.option.optionbit.dest_transmit)
                    {
                        RadioTxLen = ring_buffer_dequeue_arr(&uart_rx_ring_buf,RadioTxBuffer,58 + 3);
                    }
                    else
                    {
                        RadioTxBuffer[0] = stTmpCfgParm.channel.channelbit.channelno;
                        RadioTxLen = 1 + ring_buffer_dequeue_arr(&uart_rx_ring_buf,RadioTxBuffer + 1,58);
                    }
                    RadioTxBuffer[RadioTxLen] = crc8((uint8_t *)RadioTxBuffer,RadioTxLen);
                    RadioTxLen ++;
                    //BoardDisableIrq();
                    Radio.Sleep( );
                    Radio.Send( (uint8_t*)RadioTxBuffer, RadioTxLen );
                    /*for(int i = 0;i < RadioTxLen;i ++)
                    {
                        putchar(RadioTxBuffer[i]);
                    }*/
                    //BoardEnableIrq();
                }
                BoardEnableIrq();
            }
        }
    }
}

void NormalModeOnTxDone( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    NormalModeState = TX;
}

void NormalModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    uint8_t crc;
    crc = crc8(payload,size - 1);
    if(crc == payload[size - 1])
    {
        if(stTmpCfgParm.option.optionbit.dest_transmit)
        {
            if((IS_BROADCAST_ADDR(*(uint16_t*)payload)) && (payload[2] == stTmpCfgParm.channel.channelbit.channelno))
            {
                ring_buffer_queue_arr(&uart_tx_ring_buf, (const char *)(payload + 3), size - 3 - 1);
            }
        }
        else
        {
            if(payload[0] == stTmpCfgParm.channel.channelbit.channelno)
            {
                ring_buffer_queue_arr(&uart_tx_ring_buf, (const char *)(payload + 1), size - 1 - 1);
            }
        }
    }
    Radio.Sleep( );
    Radio.Rx( 0 );
    ComportTxStart();
}

void NormalModeOnTxTimeout( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    NormalModeState = TX_TIMEOUT;
    printf("txtimeout\r\n");
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