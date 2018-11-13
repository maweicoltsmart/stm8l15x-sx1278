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

uint16_t cfg_parm_get_air_baud(void)
{
    uint16_t airbaud ;
    switch(stTmpCfgParm.speed.speedbit.radio_baud)
    {
        case 0:
          airbaud = 300; // 125k sf = 12
          break;
        case 1:
          airbaud = 1200; // 250k sf = 
          break;
        case 2:
          airbaud = 2400; // 500k sf = 
          break;
        case 3:
          airbaud = 4800; // 250k sf = 
          break;
        case 4:
          airbaud = 9600; // 500k sf = 
          break;
        case 5:
          airbaud = 19200; // 500k sf = 
          break;
        case 6:
          airbaud = 19200; // 500k sf = 
          break;
        case 7:
          airbaud = 19200; // 500k sf = 
          break;
        default:
          airbaud = 2400;
          break;
    }
    return airbaud;
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