/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1276-Hal.c
 * \brief      SX1276 Hardware Abstraction Layer
 *
 * \version    2.0.B2 
 * \date       Nov 21 2012
 * \author     Miguel Luis
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include "board.h"
#include <stdbool.h> 
#include "platform.h"
#include "delay.h"
#include "system.h"

#if defined( USE_SX1276_RADIO )

#include "sx1276-Hal.h"

#define DIO0_IOPORT                                 SX1278_DIO0_PORT
#define DIO0_PIN                                    SX1278_DIO0_PIN
#define RXTX_IOPORT                                 SX1278_RF_SWITCH_PORT
#define RXTX_PIN                                    SX1278_RF_SWITCH_PIN

void SX1276InitIo( void )
{
    GPIO_Init(SX1278_NRST_PORT, SX1278_NRST_PIN, GPIO_Mode_Out_PP_Low_Fast); // SX1278 RST
    GPIO_Init(SX1278_RF_SWITCH_PORT, SX1278_RF_SWITCH_PIN, GPIO_Mode_Out_PP_Low_Fast); // SX1278 RF SWITCH
    BoardDisableIrq();
    EXTI_SetPinSensitivity(SX1278_DIO0_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO0
    EXTI_SetPinSensitivity(SX1278_DIO1_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO1
    EXTI_SetPinSensitivity(SX1278_DIO2_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO2
    EXTI_SetPinSensitivity(SX1278_DIO3_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO3
    EXTI_SetPinSensitivity(SX1278_DIO4_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO4
    // EXTI_SetPinSensitivity(SX1278_DIO5_EXTI_PIN, EXTI_Trigger_Rising); // SX1278 DIO5
    GPIO_Init(SX1278_DIO0_PORT, SX1278_DIO0_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO0
    GPIO_Init(SX1278_DIO1_PORT, SX1278_DIO1_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO1
    GPIO_Init(SX1278_DIO2_PORT, SX1278_DIO2_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO2
    GPIO_Init(SX1278_DIO3_PORT, SX1278_DIO3_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO3
    GPIO_Init(SX1278_DIO4_PORT, SX1278_DIO4_PIN, GPIO_Mode_In_FL_IT); // SX1278 DIO4
    GPIO_Init(SX1278_DIO5_PORT, SX1278_DIO5_PIN, GPIO_Mode_In_FL_No_IT); // SX1278 DIO5
    EXTI_ClearITPendingBit(SX1278_DIO0_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO1_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO2_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO3_EXTI_IT_PIN);
    EXTI_ClearITPendingBit(SX1278_DIO4_EXTI_IT_PIN);
    // EXTI_ClearITPendingBit(SX1278_DIO5_EXTI_IT_PIN);
    BoardEnableIrq();
}

void SX1276SetReset( uint8_t state )
{
    if( state == RADIO_RESET_ON )
    {
        // Set RESET pin to 0
        GPIO_ResetBits(SX1278_NRST_PORT, SX1278_NRST_PIN); // SX1278 RST went to low

        // Wait 1 ms
        DelayMs( 1 );
    }
    else
    {
        // Configure RESET as input
        GPIO_SetBits(SX1278_NRST_PORT, SX1278_NRST_PIN); // SX1278 RST went to high

        // Wait 6 ms
        DelayMs( 6 );
    }
}

void SX1276Write( uint8_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

void SX1276Read( uint8_t addr, uint8_t *data )
{
    SX1276ReadBuffer( addr, data, 1 );
}

void SX1276WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //GpioWrite( &SX1276.Spi.Nss, 0 );
    GPIO_ResetBits(SPI_NSS_PORT, SPI_NSS_PIN); /* CS pin low */

    SpiInOut( addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        SpiInOut( buffer[i] );
    }

    //GpioWrite( &SX1276.Spi.Nss, 1 );
    GPIO_SetBits(SPI_NSS_PORT, SPI_NSS_PIN); /* CS pin high */
}

void SX1276ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    //GpioWrite( &SX1276.Spi.Nss, 0 );
    GPIO_ResetBits(SPI_NSS_PORT, SPI_NSS_PIN); /* CS pin low */

    SpiInOut( addr & 0x7F );

    for( i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( 0 );
    }

    //NSS = 1;
    //GpioWrite( &SX1276.Spi.Nss, 1 );
    GPIO_SetBits(SPI_NSS_PORT, SPI_NSS_PIN); /* CS pin high */
}

void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1276WriteBuffer( 0, buffer, size );
}

void SX1276ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1276ReadBuffer( 0, buffer, size );
}

inline uint8_t SX1276ReadDio0( void )
{
  return GPIO_ReadInputDataBit(SX1278_DIO0_PORT, SX1278_DIO0_PIN)?1:0;
}

inline uint8_t SX1276ReadDio1( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO1_PORT, SX1278_DIO1_PIN)?1:0;
}

inline uint8_t SX1276ReadDio2( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO2_PORT, SX1278_DIO2_PIN)?1:0;
}

inline uint8_t SX1276ReadDio3( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO3_PORT, SX1278_DIO3_PIN)?1:0;
}

inline uint8_t SX1276ReadDio4( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO4_PORT, SX1278_DIO4_PIN)?1:0;
}

inline uint8_t SX1276ReadDio5( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO5_PORT, SX1278_DIO5_PIN)?1:0;
}

inline void SX1276WriteRxTx( uint8_t txEnable )
{
    if( txEnable != 0 )
    {
        GPIO_ResetBits(SX1278_RF_SWITCH_PORT, SX1278_RF_SWITCH_PIN); /* TX rf switch pin low */
        DelayMs(1);
    }
    else
    {
        DelayMs(1);
        GPIO_SetBits(SX1278_RF_SWITCH_PORT, SX1278_RF_SWITCH_PIN); /* RX rf switch pin high */
    }
}

#endif // USE_SX1276_RADIO
