#include "stm8l15x.h"
#include "cfg_parm.h"
#include <string.h>
#include <stdio.h>
#include "LoRaMac.h"

__eeprom st_cfg_pkg stNvCfgParm;
st_cfg_pkg stTmpCfgParm;
__eeprom uint8_t LoRaMacDevEuiInFlash[8] = {0x01,0x00,0x00,0x00,0x14,0x03,0x19,0x20};
__eeprom uint8_t factorystring[250] = {0x00};
void cfg_parm_factory_reset(void)
{
    memset(&stTmpCfgParm,0,5);
    stTmpCfgParm.addr_h = 0x00;
    stTmpCfgParm.addr_l = 0x00;
    stTmpCfgParm.speed.speedbit.radio_baud = 2;
    stTmpCfgParm.speed.speedbit.uart_baud = 0x03;
    stTmpCfgParm.speed.speedbit.uart_parity = 0x00; // 8n1
    stTmpCfgParm.channel.channelbit.channelno = 0x17;
    stTmpCfgParm.option.optionbit.tx_power = 0x00; // 20dbm
    stTmpCfgParm.option.optionbit.fec = 1;
    stTmpCfgParm.option.optionbit.radio_wakeup_time = 0x00;
    stTmpCfgParm.option.optionbit.io_pushpull = 1;
    stTmpCfgParm.option.optionbit.dest_transmit = 0x00;
    stTmpCfgParm.inNetMode = TRUE;
    stTmpCfgParm.netState = LORAMAC_IDLE;
    //{ 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }
    //stTmpCfgParm.LoRaMacDevEui = ;
    stTmpCfgParm.LoRaMacAppEui[0] = 'M';
    stTmpCfgParm.LoRaMacAppEui[1] = 'J';
    stTmpCfgParm.LoRaMacAppEui[2] = '-';
    stTmpCfgParm.LoRaMacAppEui[3] = 'M';
    stTmpCfgParm.LoRaMacAppEui[4] = 'o';
    stTmpCfgParm.LoRaMacAppEui[5] = 'd';
    stTmpCfgParm.LoRaMacAppEui[6] = 'e';
    stTmpCfgParm.LoRaMacAppEui[7] = 'm';
    stTmpCfgParm.LoRaMacAppKey[0] = 0x2B;
    stTmpCfgParm.LoRaMacAppKey[1] = 0x7E;
    stTmpCfgParm.LoRaMacAppKey[2] = 0x15;
    stTmpCfgParm.LoRaMacAppKey[3] = 0x16;
    stTmpCfgParm.LoRaMacAppKey[4] = 0x28;
    stTmpCfgParm.LoRaMacAppKey[5] = 0xAE;
    stTmpCfgParm.LoRaMacAppKey[6] = 0xD2;
    stTmpCfgParm.LoRaMacAppKey[7] = 0xA6;

    stTmpCfgParm.LoRaMacAppKey[8] = 0xAB;
    stTmpCfgParm.LoRaMacAppKey[9] = 0xF7;
    stTmpCfgParm.LoRaMacAppKey[10] = 0x15;
    stTmpCfgParm.LoRaMacAppKey[11] = 0x88;
    stTmpCfgParm.LoRaMacAppKey[12] = 0x09;
    stTmpCfgParm.LoRaMacAppKey[13] = 0xCF;
    stTmpCfgParm.LoRaMacAppKey[14] = 0x4F;
    stTmpCfgParm.LoRaMacAppKey[15] = 0x3C;
    //stTmpCfgParm.LoRaMacNwkSKey = ;
    //stTmpCfgParm.LoRaMacAppSKey = ;
    stTmpCfgParm.UpLinkCounter = 0;
    stTmpCfgParm.DownLinkCounter = 0;
    stTmpCfgParm.ChannelMask[0] = 0x41;
    stTmpCfgParm.ChannelMask[1] = 0x00;
    stTmpCfgParm.ChannelMask[2] = 0x00;
    stTmpCfgParm.TxPower = 20;

    stNvCfgParm.addr_h = stTmpCfgParm.addr_h;
    stNvCfgParm.addr_l = stTmpCfgParm.addr_l;
    stNvCfgParm.speed.speed = stTmpCfgParm.speed.speed;
    stNvCfgParm.channel.channel = stTmpCfgParm.channel.channel;
    stNvCfgParm.option.option = stTmpCfgParm.option.option;
    stNvCfgParm.inNetMode = stTmpCfgParm.inNetMode;
    stNvCfgParm.netState = stTmpCfgParm.netState;
    stNvCfgParm.UpLinkCounter = stTmpCfgParm.UpLinkCounter;
    stNvCfgParm.DownLinkCounter = stTmpCfgParm.DownLinkCounter;
    for(uint8_t i = 0;i < 8;i ++)
    {
        stTmpCfgParm.LoRaMacDevEui[i] = LoRaMacDevEuiInFlash[i];
        stNvCfgParm.LoRaMacDevEui[i] = LoRaMacDevEuiInFlash[i];
        stNvCfgParm.LoRaMacAppEui[i] = stTmpCfgParm.LoRaMacAppEui[i];
    }
    for(uint8_t i = 0;i < 16;i ++)
    {
        stNvCfgParm.LoRaMacAppKey[i] = stTmpCfgParm.LoRaMacAppKey[i];
    }
    stNvCfgParm.ChannelMask[0] = stTmpCfgParm.ChannelMask[0];
    stNvCfgParm.ChannelMask[1] = stTmpCfgParm.ChannelMask[1];
    stNvCfgParm.ChannelMask[2] = stTmpCfgParm.ChannelMask[2];
    stNvCfgParm.TxPower = stTmpCfgParm.TxPower;
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stTmpCfgParm.addr_h,stTmpCfgParm.addr_l,stTmpCfgParm.speed.speed,stTmpCfgParm.channel.channel,stTmpCfgParm.option.option);
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stNvCfgParm.addr_h,stNvCfgParm.addr_l,stNvCfgParm.speed.speed,stNvCfgParm.channel.channel,stNvCfgParm.option.option);
}

void cfg_parm_restore(void)
{
    stNvCfgParm.addr_h = stTmpCfgParm.addr_h;
    stNvCfgParm.addr_l = stTmpCfgParm.addr_l;
    stNvCfgParm.speed.speed = stTmpCfgParm.speed.speed;
    stNvCfgParm.channel.channel = stTmpCfgParm.channel.channel;
    stNvCfgParm.option.option = stTmpCfgParm.option.option;
    stNvCfgParm.inNetMode = stTmpCfgParm.inNetMode;
    stNvCfgParm.netState = stTmpCfgParm.netState;
    
    for(uint8_t i = 0;i < 8;i ++)
      stNvCfgParm.LoRaMacAppEui[i] = stTmpCfgParm.LoRaMacAppEui[i];
    for(uint8_t i = 0;i < 16;i ++)
    {
        stNvCfgParm.LoRaMacAppKey[i] = stTmpCfgParm.LoRaMacAppKey[i];
        stNvCfgParm.LoRaMacNwkSKey[i] = stTmpCfgParm.LoRaMacNwkSKey[i];
        stNvCfgParm.LoRaMacAppSKey[i] = stTmpCfgParm.LoRaMacAppSKey[i];
    }
      
    stNvCfgParm.LoRaMacNetID = stTmpCfgParm.LoRaMacNetID;
    stNvCfgParm.LoRaMacDevAddr = stTmpCfgParm.LoRaMacDevAddr;
    stNvCfgParm.UpLinkCounter = stTmpCfgParm.UpLinkCounter;
    stNvCfgParm.DownLinkCounter = stTmpCfgParm.DownLinkCounter;
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stTmpCfgParm.addr_h,stTmpCfgParm.addr_l,stTmpCfgParm.speed.speed,stTmpCfgParm.channel.channel,stTmpCfgParm.option.option);
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stNvCfgParm.addr_h,stNvCfgParm.addr_l,stNvCfgParm.speed.speed,stNvCfgParm.channel.channel,stNvCfgParm.option.option);
    stNvCfgParm.ChannelMask[0] = stTmpCfgParm.ChannelMask[0];
    stNvCfgParm.ChannelMask[1] = stTmpCfgParm.ChannelMask[1];
    stNvCfgParm.ChannelMask[2] = stTmpCfgParm.ChannelMask[2];
    stNvCfgParm.TxPower = stTmpCfgParm.TxPower;
}

void cfg_parm_dump_to_ram(void)
{
    stTmpCfgParm.addr_h = stNvCfgParm.addr_h;
    stTmpCfgParm.addr_l = stNvCfgParm.addr_l;
    stTmpCfgParm.speed.speed = stNvCfgParm.speed.speed;
    stTmpCfgParm.channel.channel = stNvCfgParm.channel.channel;
    stTmpCfgParm.option.option = stNvCfgParm.option.option;
    stTmpCfgParm.inNetMode = stNvCfgParm.inNetMode;
    stTmpCfgParm.netState = stNvCfgParm.netState;
    
    for(uint8_t i = 0;i < 8;i ++)
    {
        stTmpCfgParm.LoRaMacDevEui[i] = stNvCfgParm.LoRaMacDevEui[i];
        stTmpCfgParm.LoRaMacAppEui[i] = stNvCfgParm.LoRaMacAppEui[i];
    }
    
    for(uint8_t i = 0;i < 16;i ++)
    {
        stTmpCfgParm.LoRaMacAppKey[i] = stNvCfgParm.LoRaMacAppKey[i];
        stTmpCfgParm.LoRaMacNwkSKey[i] = stNvCfgParm.LoRaMacNwkSKey[i];
        stTmpCfgParm.LoRaMacAppSKey[i] = stNvCfgParm.LoRaMacAppSKey[i];
    }
      
    stTmpCfgParm.LoRaMacNetID = stNvCfgParm.LoRaMacNetID;
    stTmpCfgParm.LoRaMacDevAddr = stNvCfgParm.LoRaMacDevAddr;
    stTmpCfgParm.UpLinkCounter = stNvCfgParm.UpLinkCounter;
    stTmpCfgParm.DownLinkCounter = stNvCfgParm.DownLinkCounter;
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stTmpCfgParm.addr_h,stTmpCfgParm.addr_l,stTmpCfgParm.speed.speed,stTmpCfgParm.channel.channel,stTmpCfgParm.option.option);
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stNvCfgParm.addr_h,stNvCfgParm.addr_l,stNvCfgParm.speed.speed,stNvCfgParm.channel.channel,stNvCfgParm.option.option);
    stTmpCfgParm.ChannelMask[0] = stNvCfgParm.ChannelMask[0];
    stTmpCfgParm.ChannelMask[1] = stNvCfgParm.ChannelMask[1];
    stTmpCfgParm.ChannelMask[2] = stNvCfgParm.ChannelMask[2];
    stTmpCfgParm.TxPower = stNvCfgParm.TxPower;
}

uint8_t cfg_parm_get_tx_power(void)
{
    uint8_t tx_power;
    switch(stTmpCfgParm.option.optionbit.tx_power)
    {
        case 0:
          tx_power = 20;
          break;
        case 1:
          tx_power = 17;
          break;
        case 2:
          tx_power = 14;
          break;
        case 3:
          tx_power = 10;
          break;
        default:
          tx_power = 20;
          break;
    }
    return tx_power;
}

uint32_t cfg_parm_get_uart_baud(void)
{
    uint32_t uart_baud;
    switch(stTmpCfgParm.speed.speedbit.uart_baud)
    {
        case 0:
          uart_baud = 1200;
          break;
        case 1:
          uart_baud = 2400;
          break;
        case 2:
          uart_baud = 4800;
          break;
        case 3:
          uart_baud = 9600;
          break;
        case 4:
          uart_baud = 19200;
          break;
        case 5:
          uart_baud = 38400;
          break;
        case 6:
          uart_baud = 57600;
          break;
        case 7:
          uart_baud = 115200;
          break;
        default:
          uart_baud = 9600;
          break;
    }
    return uart_baud;
}

USART_Parity_TypeDef cfg_parm_get_uart_parity(void)
{
    USART_Parity_TypeDef parity ;
    switch(stTmpCfgParm.speed.speedbit.uart_parity)
    {
        case 0:
          parity = USART_Parity_No;
          break;
        case 1:
          parity = USART_Parity_Odd;
          break;
        case 2:
          parity = USART_Parity_Even;
          break;
        case 3:
          parity = USART_Parity_No;
          break;
        default:
          parity = USART_Parity_No;
          break;
    }
    return parity;
}

float cfg_parm_get_air_baud(void)
{
    float airbaud ;
    switch(stTmpCfgParm.speed.speedbit.radio_baud)
    {
        case 0:
          airbaud = 61.035156; //300 250k sf = 12
          break;
        case 1:
          airbaud = 122.070310; //1200 500k sf = 12
          break;
        case 2:
          airbaud = 244.140620; //2400 500k sf = 11
          break;
        case 3:
          airbaud = 488.281240; //4800 500k sf = 10
          break;
        case 4:
          airbaud = 976.562500; //7200 500k sf = 9
          break;
        case 5:
          airbaud = 1953.125000; //9600 500k sf = 8
          break;
        case 6:
          airbaud = 3906.250000; //14400 500k sf = 7
          break;
        case 7:
          airbaud = 7812.500000; //19200 500k sf = 6
          break;
        default:
          airbaud = 1953.125; // 2400
          break;
    }
    return airbaud;
}

uint8_t cfg_parm_get_air_bandwith(void)
{
    uint16_t bandwith ;
    //  0: 125 kHz,
    //  1: 250 kHz,
    //  2: 500 kHz,
    switch(stTmpCfgParm.speed.speedbit.radio_baud)
    {
        case 0:
          bandwith = 1;
          break;
        case 1:
          bandwith = 2;
          break;
        case 2:
          bandwith = 2;
          break;
        case 3:
          bandwith = 2;
          break;
        case 4:
          bandwith = 2;
          break;
        case 5:
          bandwith = 2;
          break;
        case 6:
          bandwith = 2;
          break;
        case 7:
          bandwith = 2;
          break;
        default:
          bandwith = 2;
          break;
    }
    return bandwith;
}

uint8_t cfg_parm_get_air_sf(void)
{
    uint16_t sf ;
    //  0: 125 kHz,
    //  1: 250 kHz,
    //  2: 500 kHz,
    switch(stTmpCfgParm.speed.speedbit.radio_baud)
    {
        case 0:
          sf = 12;
          break;
        case 1:
          sf = 12;
          break;
        case 2:
          sf = 11;
          break;
        case 3:
          sf = 10;
          break;
        case 4:
          sf = 9;
          break;
        case 5:
          sf = 8;
          break;
        case 6:
          sf = 7;
          break;
        case 7:
          sf = 6;
          break;
        default:
          sf = 11;
          break;
    }
    return sf;
}

uint16_t cfg_parm_get_wakeup_time(void)
{
    uint16_t wakeuptime ;
    switch(stTmpCfgParm.option.optionbit.radio_wakeup_time)
    {
        case 0:
          wakeuptime = 250;
          break;
        case 1:
          wakeuptime = 500;
          break;
        case 2:
          wakeuptime = 750;
          break;
        case 3:
          wakeuptime = 1000;
          break;
        case 4:
          wakeuptime = 1250;
          break;
        case 5:
          wakeuptime = 1500;
          break;
        case 6:
          wakeuptime = 1750;
          break;
        case 7:
          wakeuptime = 2000;
          break;
        default:
          wakeuptime = 250;
          break;
    }
    return wakeuptime;
}