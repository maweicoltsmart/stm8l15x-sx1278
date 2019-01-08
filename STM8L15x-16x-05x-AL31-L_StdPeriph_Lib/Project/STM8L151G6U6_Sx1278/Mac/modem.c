#include <stdio.h>
#include <string.h>
#include "modem.h"
#include "radio.h"
#include "cfg_parm.h"

// transient state
static struct {
    uint8_t cmdbuf[128];
    uint16_t rsplen;
    uint8_t txpending;
    //osjob_t alarmjob;
    //osjob_t blinkjob;
    //osjob_t ledjob;
} MODEM;

void modem_init () {
    memset(&MODEM, 0, sizeof(MODEM));
    frame_init(&rxframe, MODEM.cmdbuf, sizeof(MODEM.cmdbuf));
    //TimerInit( &Led1Timer_OffLine, OnLed1TimerEventNetOffline );
}

void onEvent (ev_t ev)
{
    // take action on specific events
    switch(ev)
    {
        case EV_JOINING:
          break;
        case EV_JOINED:
        case EV_TXCOMPLETE:
        case EV_RXCOMPLETE:
        default:
          break;
    }
}

// called by frame job
// process command and prepare response in MODEM.cmdbuf[]
void modem_rxdone () {
    uint8_t ok = 0;
    uint8_t cmd = tolower(MODEM.cmdbuf[0]);
    uint16_t len = rxframe.len;
    uint8_t* rspbuf = MODEM.cmdbuf;
    uint8_t rst = FALSE;
    if(len == 0) { // AT
        ok = 1;
    } else if(cmd == 'v' && len == 2 && MODEM.cmdbuf[1] == '?') { // ATV? query version
        rspbuf += cpystr(rspbuf, "OK,");
        rspbuf += cpystr(rspbuf, VERSION_STR);
        ok = 1;
    } else if(cmd == 'z' && len == 1) { // ATZ reset
        Radio.Sleep( );
        rst = true;
        ok = 1;
    } else if(cmd == 'q' && len == 1) { // ATZ reset
        stTmpCfgParm.inNetMode = FALSE;
        cfg_parm_restore();
        rst = true;
        ok = 1;
    } else if(cmd == '&' && len == 2 && tolower(MODEM.cmdbuf[1]) == 'f') { // AT&F factory reset
        Radio.Sleep( );
        cfg_parm_factory_reset();
        rst = true;
        ok = 1;
    }

    // send response
    if(ok) {
    if(rspbuf == MODEM.cmdbuf) {
        rspbuf += cpystr(rspbuf, "OK");
    }
    } else {
    rspbuf += cpystr(rspbuf, "ERROR");
    }
    *rspbuf++ = '\r';
    *rspbuf++ = '\n';
    MODEM.rsplen = rspbuf - MODEM.cmdbuf;
    for(uint8_t loop = 0;loop < MODEM.rsplen;loop ++)
    {
        putchar(MODEM.cmdbuf[loop]);
    }
    if(rst == true)
    {
        //Reset_Handler();
    }
    frame_init(&rxframe, MODEM.cmdbuf, sizeof(MODEM.cmdbuf));
}