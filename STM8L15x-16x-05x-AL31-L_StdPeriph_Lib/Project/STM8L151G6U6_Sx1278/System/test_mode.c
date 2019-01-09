#include "test_mode.h"
#include "board.h"
#include "stdio.h"
#include "radio.h"
#include "cfg_parm.h"
#include "manufacture_parm.h"
#include "delay.h"
#include "comport.h"

#define TX_OUTPUT_POWER                             20        // 20 dBm
#define TX_TIMEOUT                                  65535     // seconds (MAX value)
#define RX_TIMEOUT                                  65535     // seconds (MAX value)

typedef enum
{
    LOWPOWER,
    RX,
    RX_ERROR,
    TX,
}States_t;

static States_t TestModeState = LOWPOWER;
static RadioEvents_t TestModeRadioEvents;
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
    static int channel = 0;
    
    // Restarts continuous wave transmission when timeout expires
    //printf("tx timeout!\r\n");
    Radio.Sleep();
    DelayMs(5);
    Radio.SetTxContinuousWave( 430000000 + channel * 5000000, TX_OUTPUT_POWER, 10 );
    if(++ channel > 16)
      channel = 0;
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

void test_mode_routin(void)
{
    // cfg gpio & radio
    RTC_Config();
    TIM4_Config();
    ComportInit();
    cfg_parm_factory_reset();
    stManufactureParm.softwareversion = VERSION_STR;
    GPIO_Init(SX1278_AUX_PORT, SX1278_AUX_PIN, GPIO_Mode_In_FL_No_IT); // AUX mode input
        // Radio initialization
    //TestModeRadioEvents.TxDone = TestModeOnTxDone;
    //TestModeRadioEvents.RxDone = TestModeOnRxDone;
    //TestModeRadioEvents.RxTimeout = TestModeOnRxTimeout;
    //TestModeRadioEvents.RxError = TestModeOnRxError;
    TestModeRadioEvents.TxTimeout = TestModeOnRadioTxTimeout;
    Radio.Init( &TestModeRadioEvents );
    //Radio.Sleep();
    //printf("checking now!\r\n");
    
    //printf("checking tx power!\r\n");
    //while(GPIO_ReadInputDataBit(SX1278_AUX_PORT, SX1278_AUX_PIN) != 1);
    TestModeState = TX;
    TestModeOnRadioTxTimeout();
    /*
    //printf("checking rx sense!\r\n");
    while(GPIO_ReadInputDataBit(SX1278_M0_PORT, SX1278_M0_PIN) != 1);
    TestModeState = RX;
    Radio.Sleep();
    Radio.Rx(0); // 0: receive RxContinuous
    //printf("checking rx sense!\r\n");
    while((GPIO_ReadInputDataBit(SX1278_M1_PORT, SX1278_M1_PIN) != 1) || (TestModeState != LOWPOWER));
    TestModeState = LOWPOWER;
    Radio.Sleep();
    */
    //printf("test\r\n");
    while(GetRunModePin() == En_Test_Mode)
    {
        ClearWWDG();
        GPIO_ResetBits(SX1278_IO1_PORT, SX1278_IO1_PIN);
        DelayMs( 1 );
        GPIO_SetBits(SX1278_IO1_PORT, SX1278_IO1_PIN);
        DelayMs( 1 );
    }
    //printf("check over!\r\n");
    // reset mcu and get run mode again
}