/*!
 * \file      delay.h
 *
 * \brief     Delay implementation
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
 */
#ifndef __DELAY_H__
#define __DELAY_H__

//#include <stdint.h>
#include "stm8l15x.h"

void Delay(__IO uint32_t nTime);
void TimingDelay_Decrement(void);
void TimingDelay_Init(void);

/*! 
 * Blocking delay of "s" seconds
 */
void DelayS( float s );

/*! 
 * Blocking delay of "ms" milliseconds
 */
void DelayMs( uint32_t ms );
void delay_10us(u16 n_us);


#endif // __DELAY_H__

