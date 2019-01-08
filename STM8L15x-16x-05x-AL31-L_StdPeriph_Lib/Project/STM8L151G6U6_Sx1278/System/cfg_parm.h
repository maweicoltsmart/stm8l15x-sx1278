#ifndef __CFG_PARM_H__
#define __CFG_PARM_H__
#include "stm8l15x.h"
#include "stm8l15x_usart.h"

/*struct cfg_parm{
    unsigned char *model_type;
    uint32_t radio_freq;
    uint8_t local_addr_h;
    uint8_t local_addr_l;
    uint8_t channel;
    uint32_t radio_rate;
    uint32_t radio_power;
    uint32_t uart_baud;
    USART_WordLength_TypeDef databit;
    USART_StopBits_TypeDef stopbit;
    USART_Parity_TypeDef parity;
};*/

#define IS_BROADCAST_ADDR(addr)     ((addr == 0xffff) || (addr == 0x0000))?1:0

typedef struct cfg_parm st_cfg_parm;
typedef struct cfg_parm *pst_cfg_parm;

struct cfg_pkg{
    //unsigned char head; // 0xC0 所设置的参数会掉电保存。; 0xC2 所设置的参数不会掉电保存。
    unsigned char addr_h;
    unsigned char addr_l;
    union{
        unsigned char speed;
        struct{
            unsigned char radio_baud:3; // 0x00 300; 0x01 1200; 0x02 2400; 0x03 4800; 0x04 9600; 0x05 19200; 0x06 19200; 0x07 19200
            unsigned char uart_baud:3; // 0x00 1200; 0x01 2400; 0x02 4800; 0x03 9600; 0x04 19200; 0x05 38400; 0x06 57600; 0x07 115200
            unsigned char uart_parity:2; // 0x00 8N1; 0x01 8O1; 0x02 8E1; 0x03 8N1
        }speedbit;
    }speed;
    union{
        unsigned char channel;
        struct{
            unsigned char channelno:5; // 对应（410MHz+CHAN * 1MHz），默认17H（433MHz） 
            unsigned char reserved:3;
        }channelbit;
    }channel;
    union{
        unsigned char option;
        struct{
            unsigned char tx_power:2; // 0x00 20dBm（默认）; 0x01 17dBm; 0x02 14dBm; 0x03 10dBm
            unsigned char fec:1; // 0x00 关闭FEC纠错 ; 0x01 打开FEC纠错（默认）
            unsigned char radio_wakeup_time:3; // 无线唤醒时间 ; 0x00 250ms（默认）; 0x01 500ms; 0x02 750ms; 0x03 1000ms; 0x04 1250ms; 0x05 1500ms; 0x06 1750ms; 0x07 2000ms
            unsigned char io_pushpull:1; // IO 驱动方式（默认1）; 0: TXD、AUX开路输出，RXD开路输入; 1: TXD、AUX推挽输出，RXD上拉输入 
            unsigned char dest_transmit:1; // 定点发送使能位（类MODBUS）0: 透明传输模式 ; 1: 定点传输模式,为1时，每个用户数据帧的前3个字节作为高、低地址、信道。发射时，模块改变自身地址和信道，完毕后，恢复原有设置。
        }optionbit;
    }option;
    unsigned char inNetMode;
};
typedef struct cfg_pkg st_cfg_pkg;
typedef struct cfg_pkg* pst_cfg_pkg;

extern __eeprom st_cfg_pkg stNvCfgParm;
extern st_cfg_pkg stTmpCfgParm;

void cfg_parm_factory_reset(void);
void cfg_parm_restore(void);
void cfg_parm_dump_to_ram(void);
uint8_t cfg_parm_get_tx_power(void);
uint32_t cfg_parm_get_uart_baud(void);
USART_Parity_TypeDef cfg_parm_get_uart_parity(void);
float cfg_parm_get_air_baud(void);
uint16_t cfg_parm_get_wakeup_time(void);
uint8_t cfg_parm_get_air_bandwith(void);
uint8_t cfg_parm_get_air_sf(void);

#endif