#ifndef __COMPORT_H__
#define __COMPORT_H__
#include "ring_buf.h"

extern ring_buffer_t uart_rx_ring_buf,uart_tx_ring_buf;

void ComportInit(void);
void ComportTxStart(void);

#endif