#include "lowpower_mode.h"
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

static void SetTxCfg(void)
{
    Radio.SetTxConfig( MODEM_LORA, cfg_parm_get_tx_power(), 0, cfg_parm_get_air_bandwith(),
                                   cfg_parm_get_air_sf(), LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, false, 0, LORA_IQ_INVERSION_ON, 3000 );
}

static void SetRxCfg(void)
{
    Radio.SetRxConfig( MODEM_LORA, cfg_parm_get_air_bandwith(), cfg_parm_get_air_sf(),
                                   LORA_CODINGRATE, 0, (uint16_t)(cfg_parm_get_air_baud() * (cfg_parm_get_wakeup_time() / 1000.0) + 3 + 8 + cfg_parm_get_air_baud() * 0.005),
                                   (uint16_t)(cfg_parm_get_air_baud() * (cfg_parm_get_wakeup_time() / 1000.0) + 3 + 8 + cfg_parm_get_air_baud() * 0.005), LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, false, 0, LORA_IQ_INVERSION_ON, true );
}

void lowpower_mode_routin(void)
{
    //PWR_UltraLowPowerCmd(ENABLE);
    // cfg gpio & radio    
    // Radio initialization
    LowPowerModeRadioEvents.TxDone = LowPowerModeOnTxDone;
    LowPowerModeRadioEvents.RxDone = LowPowerModeOnRxDone;
    LowPowerModeRadioEvents.TxTimeout = LowPowerModeOnTxTimeout;
    LowPowerModeRadioEvents.RxTimeout = LowPowerModeOnRxTimeout;
    LowPowerModeRadioEvents.RxError = LowPowerModeOnRxError;
    LowPowerModeRadioEvents.CadDone = LowPowerModeOnCadDone;
    BoardDisableIrq();
    TIM4_Config();
    Radio.Init( &LowPowerModeRadioEvents );
    //factory = 433000000;
    Radio.SetChannel( stTmpCfgParm.channel.channelbit.channelno * 1000000 + 410000000 );
    SetTxCfg();
    SetRxCfg();
    Radio.Sleep( );
    Radio.Rx( 0 );
    Radio.Sleep( );
    /* RTC configuration -------------------------------------------*/
    RTC_Config();
    
    SetTxCfg();
    SetRxCfg();
    ComportInit();
    BoardEnableIrq();
    printf("lowpower\r\n");
    GPIO_SetBits(SX1278_AUX_PORT, SX1278_AUX_PIN);
    while((GetRunModePin() == En_Low_Power_Mode) || (!ring_buffer_is_empty(&uart_tx_ring_buf)) || (Radio.GetStatus() == RF_TX_RUNNING))
    {
        ClearWWDG();
        if((Radio.GetStatus() == RF_IDLE) && (ring_buffer_is_empty(&uart_tx_ring_buf)) && (USART_GetFlagStatus(USART1,USART_FLAG_TXE) == SET) && (USART_GetFlagStatus(USART1,USART_FLAG_TC) == SET))
        {
            BoardDisableIrq();
            
            /* Enter Wait for interrupt mode*/
            Radio.Sleep( );
            BoardEnableIrq();
            halt();
            
        }
    }
}

void LowPowerModeOnTxDone( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    LowPowerModeState = TX;
}

void LowPowerModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
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
    //Radio.Rx( 0 );
    ComportTxStart();
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
    printf("rxtimeout\r\n");
    RTC_SetWakeUpCounter((uint16_t)(cfg_parm_get_wakeup_time() * 1000.0 / 488.28125) - 1);
    RTC_WakeUpCmd(ENABLE);
    //Radio.Rx( 0 );
    LowPowerModeState = RX_TIMEOUT;
}

void LowPowerModeOnRxError( void )
{
    Radio.Sleep( );
    RTC_WakeUpCmd(DISABLE);
    printf("rxerror\r\n");
    RTC_SetWakeUpCounter((uint16_t)(cfg_parm_get_wakeup_time() * 1000.0 / 488.28125) - 1);
    RTC_WakeUpCmd(ENABLE);
    //Radio.Rx( 0 );
    LowPowerModeState = RX_ERROR;
}

void LowPowerModeOnCadDone( bool channelActivityDetected )
{
    Radio.Sleep( );
    //printf("cadone\r\n");
    if(channelActivityDetected)
    {
        RTC_WakeUpCmd(DISABLE);
        //printf("rx\r\n");
        Radio.Rx( 0 );
    }
    else
    {
        halt();
    }
}