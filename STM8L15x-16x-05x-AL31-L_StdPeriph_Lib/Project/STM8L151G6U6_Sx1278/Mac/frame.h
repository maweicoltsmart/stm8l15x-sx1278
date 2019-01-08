#ifndef __FRAME_H__
#define __FRAME_H__

#include "board.h"
#include "modem.h"

#define MAX_LEN_FRAME   128

// frame rx/tx state
#define FRAME_PREAMBLE_FF   0xA5
#define FRAME_PREAMBLE_55   0xA6
#define FRAME_PREAMBLE_AA   0xA7
#define FRAME_PREAMBLE_END   0xA8
#define FRAME_INIT   0x00
#define FRAME_A_A    0xA1
#define FRAME_A_T    0xA2
#define FRAME_A_OK   0xA3
#define FRAME_A_ERR  0xA4
#define FRAME_B_B    0xB1
#define FRAME_B_LEN  0xB2
#define FRAME_B_LRC  0xB3
#define FRAME_B_OK   0xB4
#define FRAME_B_ERR  0xB5

typedef struct {
    uint8_t state;
    uint8_t *buf;
    uint16_t len;
    uint16_t max;
    uint8_t lrc;
 } FRAME;

extern FRAME rxframe;

void frame_init (FRAME* f, uint8_t* buf, uint16_t max);
uint8_t frame_rx (uint8_t c);

#endif