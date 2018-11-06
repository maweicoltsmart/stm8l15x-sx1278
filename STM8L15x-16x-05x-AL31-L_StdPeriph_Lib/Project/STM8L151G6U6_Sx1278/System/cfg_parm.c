#include "cfg_parm.h"

__eeprom st_cfg_pkg stNvCfgParm;
st_cfg_pkg stTmpCfgParm;

void cfg_parm_factory_reset(void)
{
    stNvCfgParm.addr_h = 0x00;
    stNvCfgParm.addr_l = 0x00;
    stNvCfgParm.channel.channelno = 0x17;
    stNvCfgParm.dest_transmit = 0x00;
    stNvCfgParm.fec = 1;
    stNvCfgParm.io_pushpull = 1;
    stNvCfgParm.radio_wakeup_time = 0x00;
    stNvCfgParm.speed.radio_baud = 2;
    stNvCfgParm.speed.uart_baud = 0x03;
    stNvCfgParm.speed.uart_parity = 0x00; // 8n1
    stNvCfgParm.tx_power = 0x00; // 20dbm
}

void cfg_parm_restore(void)
{
    stNvCfgParm.addr_h = stTmpCfgParm.addr_h;
    stNvCfgParm.addr_l = stTmpCfgParm.addr_l;
    stNvCfgParm.channel.channelno = stTmpCfgParm.channel.channelno;
    stNvCfgParm.dest_transmit = stTmpCfgParm.dest_transmit;
    stNvCfgParm.fec = stTmpCfgParm.fec;
    stNvCfgParm.io_pushpull = stTmpCfgParm.io_pushpull;
    stNvCfgParm.radio_wakeup_time = stTmpCfgParm.radio_wakeup_time;
    stNvCfgParm.speed.radio_baud = stTmpCfgParm.speed.radio_baud;
    stNvCfgParm.speed.uart_baud = stTmpCfgParm.speed.uart_baud;
    stNvCfgParm.speed.uart_parity = stTmpCfgParm.speed.uart_parity; // 8n1
    stNvCfgParm.tx_power = stTmpCfgParm.tx_power; // 20dbm
}

void cfg_parm_dump_to_ram(void)
{
    stTmpCfgParm.addr_h = stNvCfgParm.addr_h;
    stTmpCfgParm.addr_l = stNvCfgParm.addr_l;
    stTmpCfgParm.channel.channelno = stNvCfgParm.channel.channelno;
    stTmpCfgParm.dest_transmit = stNvCfgParm.dest_transmit;
    stTmpCfgParm.fec = stNvCfgParm.fec;
    stTmpCfgParm.io_pushpull = stNvCfgParm.io_pushpull;
    stTmpCfgParm.radio_wakeup_time = stNvCfgParm.radio_wakeup_time;
    stTmpCfgParm.speed.radio_baud = stNvCfgParm.speed.radio_baud;
    stTmpCfgParm.speed.uart_baud = stNvCfgParm.speed.uart_baud;
    stTmpCfgParm.speed.uart_parity = stNvCfgParm.speed.uart_parity; // 8n1
    stTmpCfgParm.tx_power = stNvCfgParm.tx_power; // 20dbm
}

uint8_t cfg_parm_get_tx_power(void)
{
    uint8_t tx_power;
    switch(stTmpCfgParm.tx_power)
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
    switch(stTmpCfgParm.speed.uart_baud)
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
    switch(stTmpCfgParm.tx_power)
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