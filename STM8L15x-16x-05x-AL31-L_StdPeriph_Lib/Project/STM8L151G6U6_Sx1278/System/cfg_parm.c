#include "cfg_parm.h"
#include <string.h>
#include <stdio.h>

__eeprom st_cfg_pkg stNvCfgParm;
st_cfg_pkg stTmpCfgParm;

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
    
    
    stNvCfgParm.addr_h = stTmpCfgParm.addr_h;
    stNvCfgParm.addr_l = stTmpCfgParm.addr_l;
    stNvCfgParm.speed.speed = stTmpCfgParm.speed.speed;
    stNvCfgParm.channel.channel = stTmpCfgParm.channel.channel;
    stNvCfgParm.option.option = stTmpCfgParm.option.option;
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
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stTmpCfgParm.addr_h,stTmpCfgParm.addr_l,stTmpCfgParm.speed.speed,stTmpCfgParm.channel.channel,stTmpCfgParm.option.option);
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stNvCfgParm.addr_h,stNvCfgParm.addr_l,stNvCfgParm.speed.speed,stNvCfgParm.channel.channel,stNvCfgParm.option.option);
}

void cfg_parm_dump_to_ram(void)
{
    stTmpCfgParm.addr_h = stNvCfgParm.addr_h;
    stTmpCfgParm.addr_l = stNvCfgParm.addr_l;
    stTmpCfgParm.speed.speed = stNvCfgParm.speed.speed;
    stTmpCfgParm.channel.channel = stNvCfgParm.channel.channel;
    stTmpCfgParm.option.option = stNvCfgParm.option.option;
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stTmpCfgParm.addr_h,stTmpCfgParm.addr_l,stTmpCfgParm.speed.speed,stTmpCfgParm.channel.channel,stTmpCfgParm.option.option);
    //printf("%s : %02X %02X %02X %02X %02X \r\n",__func__,stNvCfgParm.addr_h,stNvCfgParm.addr_l,stNvCfgParm.speed.speed,stNvCfgParm.channel.channel,stNvCfgParm.option.option);
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
          airbaud = 292.96875; //300 125k sf = 12
          break;
        case 1:
          airbaud = 976.5625; //1200 125k sf = 10
          break;
        case 2:
          airbaud = 1953.125; //2400 250k sf = 10
          break;
        case 3:
          airbaud = 3515.625; //4800 250k sf = 9
          break;
        case 4:
          airbaud = 6250; //7200 250k sf = 8
          break;
        case 5:
          airbaud = 7031.25; //9600 500k sf = 9
          break;
        case 6:
          airbaud = 12500; //14400 500k sf = 8
          break;
        case 7:
          airbaud = 21875; //19200 500k sf = 7
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
          bandwith = 0;
          break;
        case 1:
          bandwith = 0;
          break;
        case 2:
          bandwith = 1;
          break;
        case 3:
          bandwith = 1;
          break;
        case 4:
          bandwith = 1;
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
          bandwith = 1;
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
          sf = 10;
          break;
        case 2:
          sf = 10;
          break;
        case 3:
          sf = 9;
          break;
        case 4:
          sf = 8;
          break;
        case 5:
          sf = 9;
          break;
        case 6:
          sf = 8;
          break;
        case 7:
          sf = 7;
          break;
        default:
          sf = 10;
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