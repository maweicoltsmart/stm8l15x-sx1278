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
 * \file       sx1232-Hal.c
 * \brief      SX1232 Hardware Abstraction Layer
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

#if defined( USE_SX1232_RADIO )

#include "sx1232-Hal.h"

#define DIO0_IOPORT                                 SX1278_DIO0_PORT
#define DIO0_PIN                                    SX1278_DIO0_PIN
#define RXTX_IOPORT                                 SX1278_RF_SWITCH_PORT
#define RXTX_PIN                                    SX1278_RF_SWITCH_PIN

void SX1232InitIo( void )
{
}

void SX1232SetReset( uint8_t state )
{
}

void SX1232Write( uint8_t addr, uint8_t data )
{
    SX1232WriteBuffer( addr, &data, 1 );
}

void SX1232Read( uint8_t addr, uint8_t *data )
{
    SX1232ReadBuffer( addr, data, 1 );
}

void SX1232WriteBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
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

void SX1232ReadBuffer( uint8_t addr, uint8_t *buffer, uint8_t size )
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

void SX1232WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1232WriteBuffer( 0, buffer, size );
}

void SX1232ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1232ReadBuffer( 0, buffer, size );
}

inline uint8_t SX1232ReadDio0( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO0_PORT, SX1278_DIO0_PIN)?1:0;
}

inline uint8_t SX1232ReadDio1( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO1_PORT, SX1278_DIO1_PIN)?1:0;
}

inline uint8_t SX1232ReadDio2( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO2_PORT, SX1278_DIO2_PIN)?1:0;
}

inline uint8_t SX1232ReadDio3( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO3_PORT, SX1278_DIO3_PIN)?1:0;
}

inline uint8_t SX1232ReadDio4( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO4_PORT, SX1278_DIO4_PIN)?1:0;
}

inline uint8_t SX1232ReadDio5( void )
{
    return GPIO_ReadInputDataBit(SX1278_DIO5_PORT, SX1278_DIO5_PIN)?1:0;
}

#endif // USE_SX1232_RADIO
