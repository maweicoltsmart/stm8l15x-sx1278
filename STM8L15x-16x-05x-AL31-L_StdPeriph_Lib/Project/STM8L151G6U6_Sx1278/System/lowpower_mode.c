#include "lowpower_mode.h"
#include "board.h"
#include "radio.h"
#include "comport.h"
#include "timer.h"
#include "cfg_parm.h"
#include <stdio.h>

#define RF_FREQUENCY                                433000000 // Hz
#define TX_OUTPUT_POWER                             20        // dBm

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
static RadioEvents_t LowPowerModeRadioEvents;
States_t LowPowerModeState = LOWPOWER;
//__eeprom uint32_t factory = 'a';
/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void LowPowerModeOnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void LowPowerModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void LowPowerModeOnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void LowPowerModeOnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void LowPowerModeOnRxError( void );

void LowPowerModeOnCadDone( bool channelActivityDetected );

void lowpower_mode_routin(void)
{
    PWR_UltraLowPowerCmd(ENABLE);
    // cfg gpio & radio    
    // Radio initialization
    LowPowerModeRadioEvents.TxDone = LowPowerModeOnTxDone;
    LowPowerModeRadioEvents.RxDone = LowPowerModeOnRxDone;
    LowPowerModeRadioEvents.TxTimeout = LowPowerModeOnTxTimeout;
    LowPowerModeRadioEvents.RxTimeout = LowPowerModeOnRxTimeout;
    LowPowerModeRadioEvents.RxError = LowPowerModeOnRxError;
    LowPowerModeRadioEvents.CadDone = LowPowerModeOnCadDone;
    BoardDisableIrq();
    Radio.Init( &LowPowerModeRadioEvents );
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
    Radio.Sleep( );
    Radio.Rx( 0 );
    Radio.Sleep( );
    /* RTC configuration -------------------------------------------*/
    RTC_Config();
    RTC_WakeUpCmd(DISABLE);
    RTC_SetWakeUpCounter((uint16_t)(cfg_parm_get_wakeup_time() * 1000.0 / 488.28125) - 1);
    RTC_WakeUpCmd(ENABLE);
    CLK_LSICmd(ENABLE);
    BoardEnableIrq();
    
    while(GetRunModePin() == En_Low_Power_Mode)
    {
        RadioState_t rfstatus;
        BoardDisableIrq();
        rfstatus = Radio.GetStatus();
        BoardEnableIrq();
        if((rfstatus == RF_IDLE) && (ring_buffer_is_empty(&uart_tx_ring_buf)) && (USART_GetFlagStatus(USART1,USART_FLAG_TXE) == SET) && (USART_GetFlagStatus(USART1,USART_FLAG_TC) == SET))
        {
            BoardDisableIrq();
            
            /* Enter Wait for interrupt mode*/
            Radio.Sleep( );
            BoardEnableIrq();
            halt();
            
        }
    }
    // reset mcu and get run mode again
    WWDG->CR = 0x80;//WDGA=1执行这一句后，无条件软复位
}

void LowPowerModeOnTxDone( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    LowPowerModeState = TX;
}

void LowPowerModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    char temp = 0;
    
    ring_buffer_queue_arr(&uart_tx_ring_buf, (const char *)payload, size);
    Radio.Sleep( );
    //Radio.Rx( 0 );
    if(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == SET)
    {
        ring_buffer_dequeue(&uart_tx_ring_buf, &temp);
        USART_SendData8(USART1, temp);
        USART_ITConfig(USART1, USART_IT_TC, ENABLE);
    }
    RTC_WakeUpCmd(DISABLE);
    RTC_SetWakeUpCounter((uint16_t)(cfg_parm_get_wakeup_time() * 1000.0 / 488.28125) - 1);
    RTC_WakeUpCmd(ENABLE);
}

void LowPowerModeOnTxTimeout( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    LowPowerModeState = TX_TIMEOUT;
}

void LowPowerModeOnRxTimeout( void )
{
    Radio.Sleep( );
    RTC_WakeUpCmd(DISABLE);
    RTC_SetWakeUpCounter((uint16_t)(cfg_parm_get_wakeup_time() * 1000.0 / 488.28125) - 1);
    RTC_WakeUpCmd(ENABLE);
    //Radio.Rx( 0 );
    LowPowerModeState = RX_TIMEOUT;
}

void LowPowerModeOnRxError( void )
{
    Radio.Sleep( );
    RTC_WakeUpCmd(DISABLE);
    RTC_SetWakeUpCounter((uint16_t)(cfg_parm_get_wakeup_time() * 1000.0 / 488.28125) - 1);
    RTC_WakeUpCmd(ENABLE);
    //Radio.Rx( 0 );
    LowPowerModeState = RX_ERROR;
}

void LowPowerModeOnCadDone( bool channelActivityDetected )
{
    Radio.Sleep( );
    if(channelActivityDetected)
    {
        RTC_WakeUpCmd(DISABLE);
        printf("rx\r\n");
        Radio.Rx( 1000 );
    }
    else
    {
        halt();
    }
}