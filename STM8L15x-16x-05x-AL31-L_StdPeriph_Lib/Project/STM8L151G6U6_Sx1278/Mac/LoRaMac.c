/*!
 * \file      LoRaMac.c
 *
 * \brief     LoRa MAC layer implementation
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
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 */
#include <stdio.h>
#include "utilities.h"
#include "LoRaMac.h"
#include "LoRaMacCrypto.h"
#include "cfg_parm.h"
#include "board.h"
#include "comport.h"
#include "modem.h"
#include "ring_buf.h"
#include "timer.h"
/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      1
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false
/*!
 * Maximum length of the fOpts field
 */
#define LORA_MAC_COMMAND_MAX_FOPTS_LENGTH           15

/*!
 * Device nonce is a random value extracted by issuing a sequence of RSSI
 * measurements
 */
static uint16_t LoRaMacDevNonce,LoRaMacDevNonceCopy;

/*!
 * Radio events function pointer
 */
static RadioEvents_t LoRaMacRadioEvents;

/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
TimerEvent_t AckTimeoutTimer;
uint32_t TxDoneTimerTick;
uint8_t GlobalChannel;
uint8_t GlobalDR;
LoRaMacFrameCtrl_t RxfCtrl;
uint8_t RxDataLen = 0;
uint8_t *pRxDataBuf = NULL;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
static void LoRaMacOnRadioTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
static void LoRaMacOnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
static void LoRaMacOnRadioTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx error event
 */
static void LoRaMacOnRadioRxError( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
static void LoRaMacOnRadioRxTimeout( void );
static void RadioSetTx(void)
{
    uint8_t channellist[24];
    uint8_t enablechannel = 0;
    Radio.Sleep( );
    for(uint8_t loop1 = 0;loop1 < 3;loop1 ++)
    {
        for(uint8_t loop2 = 0;loop2 < 8;loop2 ++)
        {
            if(stTmpCfgParm.ChannelMask[loop1] & (1 << loop2))
            {
                channellist[enablechannel] = loop1 * 8 + loop2;
                enablechannel ++;
            }
        }
    }
    if(enablechannel < 1)
    {
        return;
    }
    uint8_t loop3 = 0;
    do{
          GlobalChannel = channellist[randr(0,enablechannel - 1)];//[loop3 % enablechannel];
          loop3 ++;
    }while(!Radio.IsChannelFree ( MODEM_LORA, GlobalChannel * 200000 + 428200000, -90, 5 ) && (loop3 < enablechannel * 2));
    GlobalDR = 12;//- GlobalChannel % 6;
    Radio.SetChannel( GlobalChannel * 200000 + 428200000 );
    Radio.SetTxConfig( MODEM_LORA, stTmpCfgParm.TxPower, 0, 0,
                                   GlobalDR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, false, 0, LORA_IQ_INVERSION_ON, 4000 );
}

static void RadioSetRx(void)
{
    Radio.Sleep( );
    Radio.SetChannel( (GlobalChannel + 24)* 200000 + 428200000 );

    Radio.SetRxConfig( MODEM_LORA, 0, GlobalDR,
                                   LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                                   LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   0, true, false, 0, LORA_IQ_INVERSION_ON, true );
}

static void LoRaMacOnRadioTxDone( void )
{
    TxDoneTimerTick = TimerGetCurrentTime( );
    if(stTmpCfgParm.netState >= LORAMAC_JOINED)
    {
        stTmpCfgParm.netState = LORAMAC_TX_DONE;
        onEvent(EV_TXCOMPLETE);
    }
    if(stTmpCfgParm.netState < LORAMAC_JOINED)
    {
        stTmpCfgParm.netState = LORAMAC_JOIN_TX_DONE;
    }
    Radio.Sleep( );
    //RTC_WakeUpCmd(ENABLE);
}

static void LoRaMacOnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    LoRaMacHeader_t macHdr;
    //LoRaMacFrameCtrl_t fCtrl;

    uint8_t pktHeaderLen = 0;
    uint32_t address = 0;
    uint8_t appPayloadStartIndex = 0;
    uint8_t port = 0xFF;
    uint8_t frameLen = 0;
    uint32_t mic = 0;
    uint32_t micRx = 0;
    uint8_t *LoRaMacRxPayload = (uint8_t *)RadioTxBuffer;
    uint16_t sequenceCounter = 0;
    uint16_t sequenceCounterPrev = 0;
    uint16_t sequenceCounterDiff = 0;
    uint32_t downLinkCounter = 0;

    uint8_t *nwkSKey = stTmpCfgParm.LoRaMacNwkSKey;
    uint8_t *appSKey = stTmpCfgParm.LoRaMacAppSKey;

    bool isMicOk = false;
    
    Radio.Sleep( );

    macHdr.Value = payload[pktHeaderLen++];

    switch( macHdr.Bits.MType )
    {
        case FRAME_TYPE_JOIN_ACCEPT:
            if( stTmpCfgParm.netState >= LORAMAC_JOINED )
            {
                return;
            }
            LoRaMacJoinDecrypt( payload + 1, size - 1, stTmpCfgParm.LoRaMacAppKey, LoRaMacRxPayload + 1 );

            LoRaMacRxPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacRxPayload, size - LORAMAC_MFR_LEN, stTmpCfgParm.LoRaMacAppKey, &mic );
            mic += LoRaMacDevNonce;

            micRx |= ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN];
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 3] << 24 );

            if( micRx == mic )
            {
                LoRaMacJoinComputeSKeys( stTmpCfgParm.LoRaMacAppKey, LoRaMacRxPayload + 1, LoRaMacDevNonceCopy, stTmpCfgParm.LoRaMacNwkSKey, stTmpCfgParm.LoRaMacAppSKey );

                stTmpCfgParm.LoRaMacNetID = ( uint32_t )LoRaMacRxPayload[4];
                stTmpCfgParm.LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[5] << 8 );
                stTmpCfgParm.LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[6] << 16 );

                stTmpCfgParm.LoRaMacDevAddr = ( uint32_t )LoRaMacRxPayload[7];
                stTmpCfgParm.LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[8] << 8 );
                stTmpCfgParm.LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[9] << 16 );
                stTmpCfgParm.LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[10] << 24 );
                stTmpCfgParm.ChannelMask[0] = stTmpCfgParm.LoRaMacNetID & 0x000000ff;
                stTmpCfgParm.ChannelMask[1] = (stTmpCfgParm.LoRaMacNetID & 0x0000ff00) >> 8;
                stTmpCfgParm.ChannelMask[2] = (stTmpCfgParm.LoRaMacNetID & 0x00ff0000) >> 16;
                stTmpCfgParm.netState = LORAMAC_JOINED;
                cfg_parm_restore();
                onEvent(EV_JOINED);
            }
            break;
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            {
                if( stTmpCfgParm.netState < LORAMAC_JOINED )
                {
                    return;
                }
                // Check if the received payload size is valid

                address = payload[pktHeaderLen++];
                address |= ( (uint32_t)payload[pktHeaderLen++] << 8 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 16 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 24 );

                if( address != stTmpCfgParm.LoRaMacDevAddr )
                {
                    return;
                }
                else
                {
                    nwkSKey = stTmpCfgParm.LoRaMacNwkSKey;
                    appSKey = stTmpCfgParm.LoRaMacAppSKey;
                    downLinkCounter = stTmpCfgParm.DownLinkCounter;
                }

                RxfCtrl.Value = payload[pktHeaderLen++];

                sequenceCounter = ( uint16_t )payload[pktHeaderLen++];
                sequenceCounter |= ( uint16_t )payload[pktHeaderLen++] << 8;

                appPayloadStartIndex = 8 + RxfCtrl.Bits.FOptsLen;

                micRx |= ( uint32_t )payload[size - LORAMAC_MFR_LEN];
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 1] << 8 );
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 2] << 16 );
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 3] << 24 );

                sequenceCounterPrev = ( uint16_t )downLinkCounter;
                sequenceCounterDiff = ( sequenceCounter - sequenceCounterPrev );

                if( sequenceCounterDiff < ( 1 << 15 ) )
                {
                    downLinkCounter += sequenceCounterDiff;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                    }
                }
                else
                {
                    // check for sequence roll-over
                    uint32_t  downLinkCounterTmp = downLinkCounter + 0x10000 + ( int16_t )sequenceCounterDiff;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounterTmp, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                        downLinkCounter = downLinkCounterTmp;
                    }
                }

                if( isMicOk == true )
                {
                    if( ( ( size - 4 ) - appPayloadStartIndex ) > 0 )
                    {
                        port = payload[appPayloadStartIndex++];
                        frameLen = ( size - 4 ) - appPayloadStartIndex;
                        LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                       frameLen,
                                                       appSKey,
                                                       address,
                                                       DOWN_LINK,
                                                       downLinkCounter,
                                                       LoRaMacRxPayload );
                        onEvent(EV_RXCOMPLETE);
                        if(RxfCtrl.Bits.Ack)
                        {
                            //if(stTmpCfgParm.netState == LORAMAC_TX_WAIT_RESP1)
                            {
                                ringbuf_put(&uart_tx_ring_buf,',');
                                ringbuf_put(&uart_tx_ring_buf,'A');
                                ringbuf_put(&uart_tx_ring_buf,'1');
                            }
                            /*if(stTmpCfgParm.netState == LORAMAC_TX_WAIT_RESP2)
                            {
                                putchar(',');
                                putchar('A');
                                putchar('2');
                            }*/
                        }
                        // RxDataLen = puthex (payload, (const uint8_t*) RadioTxBuffer, RxDataLen);
                        ringbuf_put(&uart_tx_ring_buf,',');
                        uint8_t temphexbyte[2];
                        RxDataLen = 0;
                        RxDataLen += puthex (temphexbyte, &port, 1);
                        ring_buffer_queue_arr(&uart_tx_ring_buf, temphexbyte, 2);
                        ringbuf_put(&uart_tx_ring_buf,',');
                        RxDataLen += 1;
                        for(uint8_t loop8 = 0;loop8 < frameLen;loop8++)
                        {
                            RxDataLen += puthex (temphexbyte, (const uint8_t*) &LoRaMacRxPayload[loop8], 1);
                            ring_buffer_queue_arr(&uart_tx_ring_buf, temphexbyte, 2);
                        }
                        ringbuf_put(&uart_tx_ring_buf,'\r');
                        ringbuf_put(&uart_tx_ring_buf,'\n');

                    }
                }
            }
            if(stTmpCfgParm.netState >= LORAMAC_JOINED)
            {
                stTmpCfgParm.netState = LORAMAC_JOINED_IDLE;
            }
            break;
        case FRAME_TYPE_PROPRIETARY:
            {
                break;
            }
        default:
                break;
    }
}

static void LoRaMacOnRadioTxTimeout( void )
{
    Radio.Sleep( );
    if(stTmpCfgParm.netState >= LORAMAC_JOINED)
    {
        stTmpCfgParm.netState = LORAMAC_JOINED_IDLE;
    }
    if(stTmpCfgParm.netState < LORAMAC_JOINED)
    {
        stTmpCfgParm.netState = LORAMAC_IDLE;
    }
}

static void LoRaMacOnRadioRxError( void )
{
    if(GetRunModePin() == En_Normal_Mode)
    {
        // macHdr.Bits.RFU = CLASS_C;
        RadioSetRx();
        Radio.Rx(0);
    }
    else if(GetRunModePin() == En_Low_Power_Mode)
    {
      // macHdr.Bits.RFU = CLASS_B;
    }
    else
    {
        // macHdr.Bits.RFU = CLASS_A;
        Radio.Sleep( );
    }
    
    /*if(stTmpCfgParm.netState > LORAMAC_JOINED)
    {
        stTmpCfgParm.netState = LORAMAC_JOINED_IDLE;
    }*/
}

static void LoRaMacOnRadioRxTimeout( void )
{
    Radio.Sleep( );
    /*if(stTmpCfgParm.netState > LORAMAC_JOINED)
    {
        stTmpCfgParm.netState = LORAMAC_JOINED_IDLE;
    }*/
}

void LoRaMacOnRadioCadDone( bool channelActivityDetected )
{
    Radio.Sleep( );
    //printf("cadone\r\n");
    if(channelActivityDetected)
    {
        RTC_WakeUpCmd(DISABLE);
        //printf("rx\r\n");
        Radio.Rx( 0 );
    }
    else
    {
        //halt();
    }
}

LoRaMacStatus_t SendFrameOnChannel( uint8_t channel,uint8_t *data,uint8_t len,uint8_t confirm,uint8_t fPort)
{
    uint8_t *LoRaMacBuffer = (uint8_t *)RadioTxBuffer;
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    // uint8_t fPort = 1;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    
    macHdr.Bits.Major = 1;
    if(confirm == 1)
        macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
    else
        macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
    if(GetRunModePin() == En_Normal_Mode)
    {
        macHdr.Bits.RFU = CLASS_C;
    }
    else if(GetRunModePin() == En_Low_Power_Mode)
    {
        macHdr.Bits.RFU = CLASS_B;
    }
    else
    {
        macHdr.Bits.RFU = CLASS_A;
    }
    //macHdr.Bits.RFU = CLASS_C;
    fCtrl.Value = 0;

    LoRaMacBuffer[pktHeaderLen++] = macHdr.Value;
    LoRaMacBuffer[pktHeaderLen++] = ( stTmpCfgParm.LoRaMacDevAddr ) & 0xFF;
    LoRaMacBuffer[pktHeaderLen++] = ( stTmpCfgParm.LoRaMacDevAddr >> 8 ) & 0xFF;
    LoRaMacBuffer[pktHeaderLen++] = ( stTmpCfgParm.LoRaMacDevAddr >> 16 ) & 0xFF;
    LoRaMacBuffer[pktHeaderLen++] = ( stTmpCfgParm.LoRaMacDevAddr >> 24 ) & 0xFF;

    LoRaMacBuffer[pktHeaderLen++] = fCtrl.Value;

    LoRaMacBuffer[pktHeaderLen++] = stTmpCfgParm.UpLinkCounter & 0xFF;
    LoRaMacBuffer[pktHeaderLen++] = ( stTmpCfgParm.UpLinkCounter >> 8 ) & 0xFF;

    // Store MAC commands which must be re-send in case the device does not receive a downlink anymore
    //MacCommandsBufferToRepeatIndex = ParseMacCommandsToRepeat( MacCommandsBuffer, MacCommandsBufferIndex, MacCommandsBufferToRepeat );
              
    LoRaMacBuffer[pktHeaderLen++] = fPort;

    LoRaMacPayloadEncrypt( (uint8_t* ) data, len, stTmpCfgParm.LoRaMacAppSKey, stTmpCfgParm.LoRaMacDevAddr, UP_LINK, stTmpCfgParm.UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
              
    pktHeaderLen = pktHeaderLen + len;

    LoRaMacComputeMic( LoRaMacBuffer, pktHeaderLen, stTmpCfgParm.LoRaMacNwkSKey, stTmpCfgParm.LoRaMacDevAddr, UP_LINK, stTmpCfgParm.UpLinkCounter, &mic );

    LoRaMacBuffer[pktHeaderLen ++] = mic & 0xFF;
    LoRaMacBuffer[pktHeaderLen ++] = ( mic >> 8 ) & 0xFF;
    LoRaMacBuffer[pktHeaderLen ++] = ( mic >> 16 ) & 0xFF;
    LoRaMacBuffer[pktHeaderLen ++] = ( mic >> 24 ) & 0xFF;
    RadioSetTx();    // Send now
    Radio.Send( LoRaMacBuffer, pktHeaderLen );
    return TRUE;
}

LoRaMacStatus_t SendJoinRequest( void )
{
    uint8_t *LoRaMacBuffer = (uint8_t *)RadioTxBuffer;
    LoRaMacHeader_t macHdr;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    
    macHdr.Bits.Major = 1;
    macHdr.Bits.MType = FRAME_TYPE_JOIN_REQ;
    if(GetRunModePin() == En_Normal_Mode)
    {
        macHdr.Bits.RFU = CLASS_C;
    }
    else if(GetRunModePin() == En_Low_Power_Mode)
    {
        macHdr.Bits.RFU = CLASS_B;
    }
    else
    {
        macHdr.Bits.RFU = CLASS_A;
    }
    LoRaMacBuffer[pktHeaderLen++] = macHdr.Value;
    memcpyr( LoRaMacBuffer + pktHeaderLen, stTmpCfgParm.LoRaMacAppEui, 8 );
    pktHeaderLen += 8;
    memcpyr( LoRaMacBuffer + pktHeaderLen, stTmpCfgParm.LoRaMacDevEui, 8 );
    pktHeaderLen += 8;

    LoRaMacDevNonce = rand1();//Radio.Random( );
    LoRaMacDevNonceCopy = (LoRaMacDevNonce << 8) | (LoRaMacDevNonce >> 8);
    LoRaMacBuffer[pktHeaderLen++] = LoRaMacDevNonce & 0xFF;
    LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

    LoRaMacJoinComputeMic( LoRaMacBuffer, pktHeaderLen & 0xFF, stTmpCfgParm.LoRaMacAppKey, &mic );

    LoRaMacBuffer[pktHeaderLen++] = mic & 0xFF;
    LoRaMacBuffer[pktHeaderLen++] = ( mic >> 8 ) & 0xFF;
    LoRaMacBuffer[pktHeaderLen++] = ( mic >> 16 ) & 0xFF;
    LoRaMacBuffer[pktHeaderLen++] = ( mic >> 24 ) & 0xFF;
    // BoardDisableIrq();
    Radio.Sleep( );
    RTC_WakeUpCmd(DISABLE);
    RadioSetTx();
    // Send now
    Radio.Send( LoRaMacBuffer, pktHeaderLen );
    // BoardEnableIrq();
    onEvent(EV_JOINING);
    return LORAMAC_STATUS_OK;
}
LoRaMacStatus_t LoRaMacInitialization( void )
{
    modem_init ();
    PWR_UltraLowPowerCmd(DISABLE); // TIM2Ê±ÖÓ»áÓÐÑÓ³Ù
    BoardDisableIrq();
    TIM4_Config();
    LoRaMacRadioEvents.TxDone = LoRaMacOnRadioTxDone;
    LoRaMacRadioEvents.RxDone = LoRaMacOnRadioRxDone;
    LoRaMacRadioEvents.RxError = LoRaMacOnRadioRxError;
    LoRaMacRadioEvents.TxTimeout = LoRaMacOnRadioTxTimeout;
    LoRaMacRadioEvents.RxTimeout = LoRaMacOnRadioRxTimeout;
    LoRaMacRadioEvents.CadDone = LoRaMacOnRadioCadDone;

    Radio.Init( &LoRaMacRadioEvents );

    //factory = 433000000;
    // RadioSetTx();
    // RadioSetRx();    
    Radio.Sleep( );
    Radio.SetPublicNetwork( 0 );
    //printf("net mode\r\n");
    GPIO_SetBits(SX1278_AUX_PORT, SX1278_AUX_PIN);
    // Random seed initialization
    srand1( Radio.Random() );
    RTC_Config();
    RTC_WakeUpCmd(DISABLE);
    ComportInit();
    BoardEnableIrq();
    return LORAMAC_STATUS_OK;
}

void LoRaMacStateCheck( void )
{
    char byte;
    
    LoRaMacInitialization();
    while(stTmpCfgParm.inNetMode == TRUE)
    {
        ClearWWDG();

        while(ring_buffer_num_items(&uart_rx_ring_buf) > 0)
        {
            ring_buffer_dequeue(&uart_rx_ring_buf,&byte);
            frame_rx(byte);
        }
        while(ring_buffer_num_items(&uart_tx_ring_buf) > 0)
        {
            ring_buffer_dequeue(&uart_tx_ring_buf,&byte);
            putchar(byte);
        }
        switch(stTmpCfgParm.netState)
        {
            case LORAMAC_IDLE:
              SendJoinRequest();
              stTmpCfgParm.netState = LORAMAC_JOINING;
              break;
            case LORAMAC_JOINING:
              halt();
              break;
            case LORAMAC_JOIN_TX_DONE:
              if(TimerGetElapsedTime(TxDoneTimerTick) < 900)
              {
                  
              }
              else
              {
                  RadioSetRx();
                  Radio.Rx(2200);
                  stTmpCfgParm.netState = LORAMAC_JOINING_WAIT_ACCEPT1;
              }
              break;
            case LORAMAC_JOINING_WAIT_ACCEPT1:
              if(TimerGetElapsedTime(TxDoneTimerTick) < 3100)
              {
              }
              else
              {
                  stTmpCfgParm.netState = LORAMAC_IDLE;
              }
              break;
            case LORAMAC_JOINING_WAIT_ACCEPT2:
              break;
            case LORAMAC_JOINED:
              stTmpCfgParm.netState = LORAMAC_JOINED_IDLE;
              break;
            case LORAMAC_JOINED_IDLE:
              if(GetRunModePin() == En_Normal_Mode)
              {
                  // macHdr.Bits.RFU = CLASS_C;
                  RadioSetRx();
                  Radio.Rx(0);
              }
              else if(GetRunModePin() == En_Low_Power_Mode)
              {
                  // macHdr.Bits.RFU = CLASS_B;
              }
              else
              {
                  // macHdr.Bits.RFU = CLASS_A;
                  Radio.Sleep( );
              }
              if((ring_buffer_is_empty(&uart_rx_ring_buf)) && (ring_buffer_is_empty(&uart_tx_ring_buf)))
              {
                  halt();
              }
              break;
            case LORAMAC_TX_ING:
              if((ring_buffer_is_empty(&uart_rx_ring_buf)) && (ring_buffer_is_empty(&uart_tx_ring_buf)))
              {
                  halt();
              }
              break;
            case LORAMAC_TX_DONE:
              if(GetRunModePin() == En_Normal_Mode)
              {
                  // macHdr.Bits.RFU = CLASS_C;
                  stTmpCfgParm.netState = LORAMAC_JOINED_IDLE;
              }
              else if(GetRunModePin() == En_Low_Power_Mode)
              {
                  // macHdr.Bits.RFU = CLASS_B;
                  stTmpCfgParm.netState = LORAMAC_JOINED_IDLE;
              }
              else
              {
                  // macHdr.Bits.RFU = CLASS_A;
                  if(TimerGetElapsedTime(TxDoneTimerTick) < 900)
                  {
                      
                  }
                  else
                  {
                      RadioSetRx();
                      Radio.Rx(2200);
                      stTmpCfgParm.netState = LORAMAC_TX_WAIT_RESP1;
                  }
              }
              
              break;
            case LORAMAC_TX_WAIT_RESP1:
              if(TimerGetElapsedTime(TxDoneTimerTick) < 3100)
              {
                  
              }
              else
              {
                  stTmpCfgParm.netState = LORAMAC_JOINED_IDLE;
              }
              break;
             case LORAMAC_TX_WAIT_RESP2:
              break;
            default:
              break;
        }
    }
}
