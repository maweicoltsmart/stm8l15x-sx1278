/*******************************************************************************
 *                                                                             *
 *                            8 bit CRC Calculation                            *
 *                                                                             *
 *                                   crc8.h                                    *
 *                                                                             *
 *                                   Module                                    *
 *                                                                             *
 *                    Copyright (C) 1997 Ulrik H?rlyk Hjort                    *
 *                                                                             *
 *   8 bit crc calculation is free software;  you can  redistribute it         *
 *   and/or modify it under terms of the  GNU General Public License           *
 *   as published  by the Free Software  Foundation;  either version 2,        *
 *   or (at your option) any later version.                                    *
 *   8 bit crc calculation is distributed in the hope that it will be          *
 *   useful, but WITHOUT ANY WARRANTY;  without even the  implied warranty     *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   *
 *   See the GNU General Public License for  more details.                     *
 *   You should have  received  a copy of the GNU General                      *
 *   Public License  distributed with Yolk.  If not, write  to  the  Free      *
 *   Software Foundation,  51  Franklin  Street,  Fifth  Floor, Boston,        *
 *   MA 02110 - 1301, USA.                                                     *
 *                                                                             *
 ******************************************************************************/
#ifndef __CRC8_H__
#define __CRC8_H__

extern unsigned char crc8_table[256]; /* 8-bit crc table */

#define CRC8(CRC, C)  ((CRC = crc8_table[CRC ^ C]) & 0xFF)


void init_crc8();
unsigned char crc8(unsigned char *data,unsigned char len);

#endif