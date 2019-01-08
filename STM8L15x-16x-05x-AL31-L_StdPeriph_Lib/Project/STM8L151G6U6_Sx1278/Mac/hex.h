#ifndef __HEX_H__
#define __HEX_H__

#include "board.h"

uint8_t gethex (uint8_t* dst, const uint8_t* src, uint16_t len);
uint8_t puthex (uint8_t* dst, const uint8_t* src, uint8_t len);
uint8_t int2hex (uint8_t* dst, uint32_t v);
uint8_t hex2int (uint32_t* n, const uint8_t* src, uint8_t len);
uint8_t dec2int (uint32_t* n, const uint8_t* src, uint8_t len);
void reverse (uint8_t* dst, const uint8_t* src, uint8_t len);
uint8_t tolower (uint8_t c);
uint8_t toupper (uint8_t c);
uint8_t cpystr (uint8_t* dst, const char* src);
uint8_t cmpstr (uint8_t* buf, uint8_t len, char* str);

#endif