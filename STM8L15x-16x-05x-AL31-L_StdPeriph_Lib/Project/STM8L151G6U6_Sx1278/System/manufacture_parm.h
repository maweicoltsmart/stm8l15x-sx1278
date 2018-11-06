#ifndef __MANUFACTURE_PARM_H__
#define __MANUFACTURE_PARM_H__
#include "stm8l15x.h"

struct manufacture_parm{
    unsigned char *softwareversion; // ����汾
    float maxpower; // ����书��
    int16_t radio_sense; // ����������
    float tx_current; // ���߷������
    float rx_current; // ���߽��յ���
    float sleep_current; // ���ߵ���
};
typedef struct manufacture_parm st_manufacture_parm;
typedef struct manufacture_parm *pst_manufacture_parm;

#define VERSION_STR   "VERSION 1.2 ("__DATE__" "__TIME__")"
extern __eeprom st_manufacture_parm stManufactureParm;

#endif