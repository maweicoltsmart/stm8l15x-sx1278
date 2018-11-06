#ifndef __MANUFACTURE_PARM_H__
#define __MANUFACTURE_PARM_H__
#include "stm8l15x.h"

struct manufacture_parm{
    unsigned char *softwareversion; // 软件版本
    float maxpower; // 最大发射功率
    int16_t radio_sense; // 接收灵敏度
    float tx_current; // 无线发射电流
    float rx_current; // 无线接收电流
    float sleep_current; // 休眠电流
};
typedef struct manufacture_parm st_manufacture_parm;
typedef struct manufacture_parm *pst_manufacture_parm;

#define VERSION_STR   "VERSION 1.2 ("__DATE__" "__TIME__")"
extern __eeprom st_manufacture_parm stManufactureParm;

#endif