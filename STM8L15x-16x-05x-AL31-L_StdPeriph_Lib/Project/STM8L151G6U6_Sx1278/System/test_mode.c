#include "test_mode.h"
#include "board.h"
#include "stdio.h"
#include "radio.h"
#include "cfg_parm.h"
#include "manufacture_parm.h"
#include "delay.h"
#include "comport.h"
#include "timer.h"

#define TX_OUTPUT_POWER                             20        // 20 dBm
#define TX_TIMEOUT                                  65535     // seconds (MAX value)
#define RX_TIMEOUT                                  65535     // seconds (MAX value)
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
    RX_ERROR,
    TX,
}States_t;

static States_t TestModeState = LOWPOWER;
static RadioEvents_t TestModeRadioEvents;
static int16_t TestRssi = 0,TestSnr = 0;
/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void TestModeOnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void TestModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void TestModeOnRadioTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void TestModeOnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void TestModeOnRxError( void );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void TestModeOnRadioTxTimeout( void )
{
    Radio.Sleep();
    //DelayMs(5);
    Radio.SetTxContinuousWave( 433000000, TX_OUTPUT_POWER, TX_TIMEOUT );
}

void TestModeOnTxDone( void )
{
    Radio.Sleep( );
    Radio.Rx( 0 );
    TestModeState = TX;
}

void TestModeOnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    //printf("OnRxDone!\r\nrssi = %d, snr = %d\r\n", rssi, snr);
    Radio.Sleep( );
    /*BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;*/
    TestRssi = rssi;
    TestSnr = snr;
    TestModeState = LOWPOWER;
}

void TestModeOnRxTimeout( void )
{
    //printf("OnRxTimeout!\r\n");
    Radio.Sleep( );
    Radio.Rx( 0 );
    TestModeState = RX;
}

void TestModeOnRxError( void )
{
    //printf("OnRxError!\r\n");
    Radio.Sleep( );
    Radio.Rx( 0 );
    TestModeState = RX;
}

Run_Mode_Type TestGetRunModePin(void)
{
    if((GPIO_ReadInputDataBit(SX1278_M0_PORT, SX1278_M0_PIN) == 0) && (GPIO_ReadInputDataBit(SX1278_M1_PORT, SX1278_M1_PIN) == 0))
      return En_Normal_Mode;
    else if((GPIO_ReadInputDataBit(SX1278_M0_PORT, SX1278_M0_PIN)) && (GPIO_ReadInputDataBit(SX1278_M1_PORT, SX1278_M1_PIN) == 0))
      return En_Wake_Up_Mode;
    else if((GPIO_ReadInputDataBit(SX1278_M0_PORT, SX1278_M0_PIN) == 0) && (GPIO_ReadInputDataBit(SX1278_M1_PORT, SX1278_M1_PIN)))
      return En_Low_Power_Mode;
    else
      return En_Config_Mode;
    //return En_Low_Power_Mode;
}

void test_mode_routin(void)
{
    char cmdbyte;
    // char cmdbuf[40];
    
    PWR_UltraLowPowerCmd(DISABLE); // TIM2Ê±ÖÓ»áÓÐÑÓ³Ù
    BoardDisableIrq();
    //TIM4_Config();
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);
    TestModeRadioEvents.TxDone = NULL;
    TestModeRadioEvents.RxDone = NULL;
    TestModeRadioEvents.TxTimeout = NULL;
    TestModeRadioEvents.RxTimeout = NULL;
    TestModeRadioEvents.RxError = NULL;
    TestModeRadioEvents.CadDone = NULL;

    //RTC_Config();

    Radio.Init( &TestModeRadioEvents );
    Radio.Sleep( );
    ComportInit();
    BoardEnableIrq();
    //printf("config\r\n");
    // cfg gpio & radio
    //GPIO_Init(SX1278_TEST_PORT, SX1278_TEST_PIN, GPIO_Mode_Out_PP_Low_Fast);
    GPIO_SetBits(SX1278_AUX_PORT, SX1278_AUX_PIN);
    while(1)//(GetRunModePin() == En_Test_Mode)
    {
        switch(TestGetRunModePin())
        {
            case En_Config_Mode:
              Radio.Sleep( );
              Radio.Init( &TestModeRadioEvents );
              Radio.Sleep( );
              //while(TestGetRunModePin() == En_Config_Mode);
              halt();
              break;
            case En_Normal_Mode:
              Radio.Sleep( );
              Radio.Init( &TestModeRadioEvents );
              Radio.Sleep( );
              TestModeOnRadioTxTimeout();
              while(TestGetRunModePin() == En_Normal_Mode);
              //halt();
              break;
            case En_Wake_Up_Mode:
              Radio.Sleep( );
              Radio.Init( &TestModeRadioEvents );
              Radio.Sleep( );
              Radio.SetChannel( 433000000 );
              Radio.SetRxConfig( MODEM_LORA, 2, 12,
                                       LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                       LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                       0, true, false, 0, LORA_IQ_INVERSION_ON, true );
              Radio.Rx( 0 ); // 0: receive RxContinuous
              while(TestGetRunModePin() == En_Wake_Up_Mode);
              //halt();
              putchar((uint8_t)(TestRssi & 0x00ff));
              putchar((uint8_t)((TestRssi & 0xff00) >> 8));
              putchar((uint8_t)(TestSnr & 0x00ff));
              putchar((uint8_t)((TestSnr & 0xff00) >> 8));
              break;
            case En_Low_Power_Mode:
              Radio.Sleep( );
              Radio.Init( &TestModeRadioEvents );
              Radio.Sleep( );
              volatile uint32_t timertick;
              timertick = TimerGetCurrentTime( );
              while(TimerGetElapsedTime(timertick) < 300);
              if(ring_buffer_num_items(&uart_rx_ring_buf) > 7)
              {
                  while(ring_buffer_dequeue(&uart_rx_ring_buf, &cmdbyte))
                  {
                      if(cmdbyte == '"')
                      {
                          uint8_t datalen = 0;
                          
                          do{
                              ring_buffer_dequeue(&uart_rx_ring_buf, &cmdbyte);
                              factorystring[datalen ++] = cmdbyte;
                          }
                          while(cmdbyte != '"');
                          datalen --;
                          factorystring[datalen - 1] = 0x00;
                          for(int8_t index = 7;index >= 0;index -- )
                          {
                              LoRaMacDevEuiInFlash[index] = factorystring[-- datalen];
                          }
                          putchar('o');
                          putchar('k');
                          putchar('!');
                          putchar('!');
                      }
                  }
              }
              else
              {
                  halt();
              }
              break;
        }
    }
    //printf("check over!\r\n");
    // reset mcu and get run mode again
}