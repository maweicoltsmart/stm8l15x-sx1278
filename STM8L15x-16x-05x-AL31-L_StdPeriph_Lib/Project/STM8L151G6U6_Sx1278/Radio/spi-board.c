/*!
 * \file      spi-board.c
 *
 * \brief     Target board SPI driver implementation
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
#include "spi-board.h"
#include "stm8l15x_spi.h"
#include "board.h"

void SpiInit( void )
{
    /* Enable SPI clock */
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);
    //GPIO_Init(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_Mode_Out_PP_High_Fast); //NSS片选
    //GPIO_Init(SPI_SCK_PORT, SPI_SCK_PIN, GPIO_Mode_Out_PP_High_Fast); //SCK
    //GPIO_Init(SPI_MOSI_PORT, SPI_MOSI_PIN, GPIO_Mode_Out_PP_High_Fast); //MOSI
    //主机模式，配置为输入  
    //GPIO_Init(SPI_MISO_PORT, SPI_MISO_PIN, GPIO_Mode_In_PU_No_IT); //MISO
    /* Set the MOSI,MISO and SCK at high level */
    GPIO_ExternalPullUpConfig(GPIOB, SPI_NSS_PIN | SPI_SCK_PIN| \
                              SPI_MOSI_PIN | SPI_MISO_PIN, ENABLE);
    /* SX1278_SPI Config */
    SPI_Init(SPI1, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_16, SPI_Mode_Master,
             SPI_CPOL_Low, SPI_CPHA_1Edge, SPI_Direction_2Lines_FullDuplex,
             SPI_NSS_Soft, 0x07);


    /* SX1278_SPI enable */
    SPI_Cmd(SPI1, ENABLE);

    /* Set MSD ChipSelect pin in Output push-pull high level */
    GPIO_Init(SPI_NSS_PORT, SPI_NSS_PIN, GPIO_Mode_Out_PP_High_Fast);
}

void SpiDeInit( void )
{
    SpiInit();
}

uint16_t SpiInOut( uint16_t outData )
{
    /* Loop while DR register in not emplty */
    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_TXE) == RESET);

    /* Send byte through the SPI peripheral */
    SPI_SendData(SPI1, outData);

    /* Wait to receive a byte */
    while (SPI_GetFlagStatus(SPI1, SPI_FLAG_RXNE) == RESET);

    /* Return the byte read from the SPI bus */
    return SPI_ReceiveData(SPI1);
}
