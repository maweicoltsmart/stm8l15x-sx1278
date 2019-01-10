#ifndef __MODEM_H__
#define __MODEM_H__

#include "board.h"
#include "hex.h"
#include "frame.h"

// modem version
#define VERSION_MAJOR 1
#define VERSION_MINOR 2
#define VERSION_STR   "VERSION 1.2 ("__DATE__" "__TIME__")"

// Event types for event callback
enum _ev_t { EV_JOINING,
             EV_JOINED,
             EV_TXCOMPLETE,
             EV_RESET,
             EV_RXCOMPLETE};
typedef enum _ev_t ev_t;

static const char* evnames[] = {
    [EV_JOINING]        = "EV_JOINING",
    [EV_JOINED]         = "EV_JOINED",
    [EV_TXCOMPLETE]     = "EV_TXCOMPLETE",
    [EV_RESET]          = "EV_RESET",
    [EV_RXCOMPLETE]     = "EV_RXCOMPLETE",
};

void modem_init ();
void modem_rxdone (void);
void onEvent (ev_t ev);

#endif