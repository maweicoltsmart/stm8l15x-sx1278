#include <stdio.h>
#include <string.h>
#include "modem.h"
#include "radio.h"
#include "cfg_parm.h"
#include "LoRaMac.h"
#include "comport.h"

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
    /*switch(ev)
    {
        case EV_JOINING:
          break;
        case EV_JOINED:
        case EV_TXCOMPLETE:
        case EV_RXCOMPLETE:
        default:
          break;
    }*/
    //printf("%s\r\n",evnames[ev]);
    BoardDisableIrq();
    for(uint8_t loop = 0;loop < strlen(evnames[ev]);loop ++)
    {
        ringbuf_put(&uart_tx_ring_buf,evnames[ev][loop]);
    }
    if(ev != EV_RXCOMPLETE)
    {
        ringbuf_put(&uart_tx_ring_buf,'\r');
        ringbuf_put(&uart_tx_ring_buf,'\n');
    }
    BoardEnableIrq();
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
    } else if(cmd == 'f' && len == 2 && MODEM.cmdbuf[1] == '?') { // ATV? query version
        rspbuf += cpystr(rspbuf, "OK,");
        rspbuf += cpystr(rspbuf, (char const*)factorystring);
        ok = 1;
    } else if(cmd == 'z' && len == 1) { // ATZ reset
        Radio.Sleep( );
        rst = true;
        ok = 1;
    } else if(cmd == 'q' && len == 1) { // ATQ quit net mode
        stTmpCfgParm.inNetMode = FALSE;
        cfg_parm_restore();
        rst = true;
        ok = 1;
    } else if(cmd == '&' && len == 2 && tolower(MODEM.cmdbuf[1]) == 'f') { // AT&F factory reset
        Radio.Sleep( );
        stTmpCfgParm.netState = LORAMAC_IDLE;
        cfg_parm_factory_reset();
        rst = true;
        ok = 1;
    }
    else if(cmd == 'j' && len == 1) { // ATJ join network
        stTmpCfgParm.netState = LORAMAC_IDLE;
        cfg_parm_restore();
        ok = 1;
    }else if(cmd == 'j' && len >= 2) { // JOIN parameters
        if(MODEM.cmdbuf[1] == '?' && len == 2) { // ATJ? query (deveui,appeui)
            uint8_t tmp[16];
            rspbuf += cpystr(rspbuf, "OK,");
            reverse(tmp, stTmpCfgParm.LoRaMacDevEui, 8);
            rspbuf += puthex(rspbuf, tmp, 8);
            *rspbuf++ = ',';
            reverse(tmp, stTmpCfgParm.LoRaMacAppEui, 8);
            rspbuf += puthex(rspbuf, tmp, 8);
            *rspbuf++ = ',';
            reverse(tmp, stTmpCfgParm.LoRaMacAppKey, 16);
            rspbuf += puthex(rspbuf, tmp, 16);
            ok = 1;
        } else if(MODEM.cmdbuf[1] == '=' && len == 2+16+1+32) { // ATJ= set (deveui,appeui,devkey)
            uint8_t tmp[16];
            if( gethex(tmp, MODEM.cmdbuf+2, 16) == 8 &&
            MODEM.cmdbuf[2+16] == ',' &&
            gethex(tmp, MODEM.cmdbuf+2+16+1, 32) == 16 ) {
                gethex(tmp, MODEM.cmdbuf+2, 16);
                reverse(stTmpCfgParm.LoRaMacAppEui, tmp, 8);
                gethex(tmp, MODEM.cmdbuf+2+16+1, 32);
                reverse(stTmpCfgParm.LoRaMacAppKey, tmp, 16);
                stTmpCfgParm.netState = LORAMAC_IDLE;
                cfg_parm_restore();
                ok = 1;
            }
        }
    }else if(cmd == 't' && len >= 5) { // confirm,port[,data]  (0,FF[,112233...])
        uint8_t port,comfirm,datalen,channel = 0;

        if((MODEM.cmdbuf[1]=='0' || MODEM.cmdbuf[1]=='1') && MODEM.cmdbuf[2]==',' && // conf
            gethex(&port, MODEM.cmdbuf+1+1+1, 2) == 1 && port) { // port
              comfirm = (MODEM.cmdbuf[1] - '0')?1:0;
            datalen = 0;
            if(len > 5 && MODEM.cmdbuf[5]==',') { // data
                datalen = gethex(MODEM.cmdbuf, MODEM.cmdbuf+6, len-6);
            }
        }
        if(stTmpCfgParm.netState == LORAMAC_JOINED_IDLE)
        {
            if(len == 5 || datalen <= 51) {
                if((port > 0) && (port < 224)){
                    stTmpCfgParm.netState = LORAMAC_TX_ING;
                    ok = SendFrameOnChannel(channel,MODEM.cmdbuf,datalen,comfirm,port);
                }
            }
        }
    }else if(cmd == 'p' && len >= 2) { // JOIN parameters
        if(MODEM.cmdbuf[1] == '?' && len == 2) { // ATP? query (tx power)
            rspbuf += cpystr(rspbuf, "OK,");
            // reverse(tmp, stTmpCfgParm.ChannelMask, 3);
            rspbuf += puthex(rspbuf, &stTmpCfgParm.TxPower, 1);
            ok = 1;
        } else if(MODEM.cmdbuf[1] == '=' && len == 2+2) { // ATP= set (tx power)
            uint8_t tmp[3];
            if( gethex(tmp, MODEM.cmdbuf+2, 2) == 1) {
                gethex(&stTmpCfgParm.TxPower, MODEM.cmdbuf+2, 2);
                // reverse(stTmpCfgParm.ChannelMask, tmp, 3);
                cfg_parm_restore();
                ok = 1;
            }
        }
    }else if(cmd == 'c' && len >= 2) { // CHANNEL parameters
        uint8_t tmp[3];
        if(MODEM.cmdbuf[1] == '?' && len == 2) { // ATC? query (channel mask)
            rspbuf += cpystr(rspbuf, "OK,");
            memcpy(tmp,stTmpCfgParm.ChannelMask,3);
            reverse(tmp, tmp, 3);
            rspbuf += puthex(rspbuf, tmp, 3);
            ok = 1;
        } else if(MODEM.cmdbuf[1] == '=' && len == 2+6) { // ATC= set (channel mask)
            uint8_t tmp[3];
            if( gethex(tmp, MODEM.cmdbuf+2, 6) == 3) {
                memcpy(stTmpCfgParm.ChannelMask,tmp,3);
                reverse(stTmpCfgParm.ChannelMask, stTmpCfgParm.ChannelMask, 3);
                cfg_parm_restore();
                ok = 1;
            }
        }
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
    BoardDisableIrq();
    for(uint8_t loop = 0;loop < MODEM.rsplen;loop ++)
    {
        ringbuf_put(&uart_tx_ring_buf,MODEM.cmdbuf[loop]);
    }
    BoardEnableIrq();
    if(rst == true)
    {
        //Reset_Handler();
        __iar_program_start();
    }
    frame_init(&rxframe, MODEM.cmdbuf, sizeof(MODEM.cmdbuf));
}