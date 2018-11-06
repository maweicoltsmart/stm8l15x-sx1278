/*!
 * \file      sx1276-board.c
 *
 * \brief     Target board SX1276 driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
 */
#include <stddef.h>
#include "delay.h"
#include "radio.h"
#include "sx1276-board.h"
#include "system.h"
#include "board.h"

/*!
 * \brief Gets the board PA selection configuration
 *
 * \param [IN] channel Channel frequency in Hz
 * \retval PaSelect RegPaConfig PaSelect value
 */
static uint8_t SX1276GetPaSelect( uint32_t channel );

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    SX1276Init,
    SX1276GetStatus,
    SX1276SetModem,
    SX1276SetChannel,
    SX1276IsChannelFree,
    SX1276Random,
    SX1276SetRxConfig,
    SX1276SetTxConfig,
    SX1276CheckRfFrequency,
    SX1276GetTimeOnAir,
    SX1276Send,
    SX1276SetSleep,
    SX1276SetStby,
    SX1276SetRx,
    SX1276StartCad,
    SX1276SetTxContinuousWave,
    SX1276ReadRssi,
    SX1276Write,
    SX1276Read,
    SX1276WriteBuffer,
    SX1276ReadBuffer,
    SX1276SetMaxPayloadLength,
    SX1276SetPublicNetwork,
    SX1276GetWakeupTime,
    NULL, // void ( *IrqProcess )( void )
    NULL, // void ( *RxBoosted )( uint32_t timeout ) - SX126x Only
    NULL, // void ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime ) - SX126x Only
};

/*!
 * Debug GPIO pins objects
 */
#if defined( USE_RADIO_DEBUG )
Gpio_t DbgPinTx;
Gpio_t DbgPinRx;
#endif

void SX1276IoInit( void )
{
    GPIO_Init(SX1278_NRST_PORT, SX1278_NRST_PIN, GPIO_Mode_Out_PP_Low_Fast); // SX1278 RST
    GPIO_Init(SX1278_RF_SWITCH_PORT, SX1278_RF_SWITCH_PIN, GPIO_Mode_Out_PP_Low_Fast); // SX1278 RF SWITCH

#if defined( USE_RADIO_DEBUG )
    GpioInit( &DbgPinTx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPinRx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

/*
static Gpio_t *DioIrqs[] = {
    &SX1276.DIO0,
    &SX1276.DIO1,
    &SX1276.DIO2,
    &SX1276.DIO3,
    &SX1276.DIO4,
    &SX1276.DIO5
};

static ext_irq_cb_t ExtIrqHandlers[] = {
    Dio0IrqHandler,
    Dio1IrqHandler,
    Dio2IrqHandler,
    Dio3IrqHandler,
    Dio4IrqHandler,
    Dio5IrqHandler
};

static void DioIrqHanlderProcess( uint8_t index )
{
    if( ( DioIrqs[index] != NULL ) && ( DioIrqs[index]->IrqHandler != NULL ) )
    {
        DioIrqs[index]->IrqHandler( DioIrqs[index]->Context );
    }
}

static void Dio0IrqHandler( void )
{
    //DioIrqHanlderProcess( 0 );
    SX1276OnDio0Irq();
}

static void Dio1IrqHandler( void )
{
    //DioIrqHanlderProcess( 1 );
    SX1276OnDio1Irq();
}

static void Dio2IrqHandler( void )
{
    //DioIrqHanlderProcess( 2 );
    SX1276OnDio2Irq();
}

static void Dio3IrqHandler( void )
{
    //DioIrqHanlderProcess( 3 );
    SX1276OnDio3Irq();
}

static void Dio4IrqHandler( void )
{
    //DioIrqHanlderProcess( 4 );
    SX1276OnDio4Irq();
}

static void Dio5IrqHandler( void )
{
    //DioIrqHanlderProcess( 5 );
    SX1276OnDio5Irq();
}
*/

/*
static void IoIrqInit( uint8_t index, DioIrqHandler *irqHandler )
{
    DioIrqs[index]->IrqHandler = irqHandler;
    ext_irq_register( DioIrqs[index]->pin, ExtIrqHandlers[index] );
}
*/
void SX1276IoIrqInit( void )
{
    BoardDisableIrq();
    EXTI_SetPinSensitivity(SX1278_DIO0_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO0
    EXTI_SetPinSensitivity(SX1278_DIO1_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO1
    EXTI_SetPinSensitivity(SX1278_DIO2_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO2
    EXTI_SetPinSensitivity(SX1278_DIO3_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO3
    EXTI_SetPinSensitivity(SX1278_DIO4_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO4
    EXTI_SetPinSensitivity(SX1278_DIO5_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO5
    EXTI_ClearITPendingBit(SX1278_DIO0_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO1_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO2_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO3_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO4_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO5_EXTI_IT_PIN);
    GPIO_Init(SX1278_DIO0_PORT, SX1278_DIO0_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO0
    GPIO_Init(SX1278_DIO1_PORT, SX1278_DIO1_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO1
    GPIO_Init(SX1278_DIO2_PORT, SX1278_DIO2_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO2
    GPIO_Init(SX1278_DIO3_PORT, SX1278_DIO3_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO3
    GPIO_Init(SX1278_DIO4_PORT, SX1278_DIO4_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO4
    GPIO_Init(SX1278_DIO5_PORT, SX1278_DIO5_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO5
    BoardEnableIrq();
    
  /*
    for( int8_t i = 0; i < 5; i++ )
    {
        IoIrqInit( i, irqHandlers[i] );
    }
  */
}

void SX1276IoDeInit( void )
{
  /*
    GpioInit( &SX1276.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );

    ext_irq_init( );

    GpioInit( &SX1276.DIO0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_0, PINMUX_PA20A_EIC_EXTINT4 );
    GpioInit( &SX1276.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_1, PINMUX_PA21A_EIC_EXTINT5 );
    GpioInit( &SX1276.DIO2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_2, PINMUX_PB12A_EIC_EXTINT12 );
    GpioInit( &SX1276.DIO3, RADIO_DIO_3, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_3, PINMUX_PB13A_EIC_EXTINT13 );
    GpioInit( &SX1276.DIO4, RADIO_DIO_4, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_4, PINMUX_PB14A_EIC_EXTINT14 );
    GpioInit( &SX1276.DIO5, RADIO_DIO_5, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    gpio_set_pin_function( RADIO_DIO_5, PINMUX_PB15A_EIC_EXTINT15 );
  */
}

/*!
 * \brief Enables/disables the TCXO if available on board design.
 *
 * \param [IN] state TCXO enabled when true and disabled when false.
 */
static void SX1276SetBoardTcxo( uint8_t state )
{
    // No TCXO component available on this board design.
#if 0
    if( state == true )
    {
        TCXO_ON( );
        DelayMs( BOARD_TCXO_WAKEUP_TIME );
    }
    else
    {
        TCXO_OFF( );
    }
#endif
}

uint32_t SX1276GetBoardTcxoWakeupTime( void )
{
    return 1;//BOARD_TCXO_WAKEUP_TIME;
}

void SX1276Reset( void )
{
    // Enables the TCXO if available on the board design
    SX1276SetBoardTcxo( true );

    // Set RESET pin to 0
    GPIO_ResetBits(SX1278_NRST_PORT, SX1278_NRST_PIN); // SX1278 RST went to low

    // Wait 1 ms
    DelayMs( 1 );

    // Configure RESET as input
    GPIO_SetBits(SX1278_NRST_PORT, SX1278_NRST_PIN); // SX1278 RST went to high

    // Wait 6 ms
    DelayMs( 6 );
}

void SX1276SetRfTxPower( int8_t power )
{
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    paConfig = SX1276Read( REG_PACONFIG );
    paDac = SX1276Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1276GetPaSelect( SX1276.Settings.Channel );

    if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( power < 5 )
            {
                power = 5;
            }
            if( power > 20 )
            {
                power = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
            if( power < 2 )
            {
                power = 2;
            }
            if( power > 17 )
            {
                power = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
        if( power > 0 )
        {
            if( power > 15 )
            {
                power = 15;
            }
            paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( 7 << 4 ) | ( power );
        }
        else
        {
            if( power < -4 )
            {
                power = -4;
            }
            paConfig = ( paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( 0 << 4 ) | ( power + 4 );
        }
    }
    SX1276Write( REG_PACONFIG, paConfig );
    SX1276Write( REG_PADAC, paDac );
}

static uint8_t SX1276GetPaSelect( uint32_t channel )
{
    if( channel < RF_MID_BAND_THRESH )
    {
        return RF_PACONFIG_PASELECT_PABOOST;
    }
    else
    {
        return RF_PACONFIG_PASELECT_RFO;
    }
}

void SX1276SetAntSwLowPower( bool status )
{
    // No antenna switch available.
    // Just control the TCXO if available.
    if( RadioIsActive != status )
    {
        RadioIsActive = status;

        if( status == false )
        {
            SX1276SetBoardTcxo( true );
            SX1276AntSwInit( );
        }
        else
        {
            SX1276SetBoardTcxo( false );
            SX1276AntSwDeInit( );
        }
    }
}

void SX1276AntSwInit( void )
{
    //Chip_IOCON_PinSetMode(LPC_IOCON,IOCON_PIO17,PIN_MODE_REPEATER);
    //Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT, 0, 17);
    GPIO_ResetBits(SX1278_RF_SWITCH_PORT, SX1278_RF_SWITCH_PIN); /* rf switch pin low */
}

void SX1276AntSwDeInit( void )
{
    //Chip_IOCON_PinSetMode(LPC_IOCON,IOCON_PIO17,PIN_MODE_INACTIVE);
    //Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT,0,17);
    GPIO_ResetBits(SX1278_RF_SWITCH_PORT, SX1278_RF_SWITCH_PIN); /* rf switch pin low */
}

void SX1276SetAntSw( uint8_t opMode )
{
    // No antenna switch available
    switch( opMode )
    {
    case RFLR_OPMODE_TRANSMITTER:
        //Chip_GPIO_SetPinOutLow(LPC_GPIO_PORT,0,17);
        GPIO_ResetBits(SX1278_RF_SWITCH_PORT, SX1278_RF_SWITCH_PIN); /* rf switch pin low */
        break;
    case RFLR_OPMODE_RECEIVER:
    case RFLR_OPMODE_RECEIVER_SINGLE:
    case RFLR_OPMODE_CAD:
    default:
        //Chip_GPIO_SetPinOutHigh(LPC_GPIO_PORT,0,17);
        GPIO_SetBits(SX1278_RF_SWITCH_PORT, SX1278_RF_SWITCH_PIN); /* rf switch pin high */
        break;
    }
}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

#if defined( USE_RADIO_DEBUG )
void SX1276DbgPinTxWrite( uint8_t state )
{
    GpioWrite( &DbgPinTx, state );
}

void SX1276DbgPinRxWrite( uint8_t state )
{
    GpioWrite( &DbgPinRx, state );
}
#endif
