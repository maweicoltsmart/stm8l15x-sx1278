#ifndef __BOARD_H__
#define __BOARD_H__
#include "stm8l15x.h"

typedef enum{
    En_Normal_Mode         = (uint8_t)0x00,
    En_Wake_Up_Mode        = (uint8_t)0x01,
    En_Low_Power_Mode      = (uint8_t)0x02,
    En_Config_Mode         = (uint8_t)0x03,
    En_Test_Mode           = (uint8_t)0x04,
}Run_Mode_Type;

#define SPI_NSS_PORT            GPIOB
#define SPI_NSS_PIN             GPIO_Pin_4
#define SPI_SCK_PORT            GPIOB
#define SPI_SCK_PIN             GPIO_Pin_5
#define SPI_MOSI_PORT           GPIOB
#define SPI_MOSI_PIN            GPIO_Pin_6
#define SPI_MISO_PORT           GPIOB
#define SPI_MISO_PIN            GPIO_Pin_7
#define SX1278_NRST_PORT        GPIOD
#define SX1278_NRST_PIN         GPIO_Pin_4

#define SX1278_RF_SWITCH_PORT        GPIOB
#define SX1278_RF_SWITCH_PIN         GPIO_Pin_3

#define SX1278_DIO0_PORT                GPIOD
#define SX1278_DIO0_PIN                 GPIO_Pin_0
#define SX1278_DIO0_EXTI_PIN            EXTI_Pin_0
#define SX1278_DIO0_EXTI_IT_PIN         EXTI_IT_Pin0

#define SX1278_DIO1_PORT                GPIOA
#define SX1278_DIO1_PIN                 GPIO_Pin_4
#define SX1278_DIO1_EXTI_PIN            EXTI_Pin_4
#define SX1278_DIO1_EXTI_IT_PIN         EXTI_IT_Pin4

#define SX1278_DIO2_PORT                GPIOA
#define SX1278_DIO2_PIN                 GPIO_Pin_3
#define SX1278_DIO2_EXTI_PIN            EXTI_Pin_3
#define SX1278_DIO2_EXTI_IT_PIN         EXTI_IT_Pin3

#define SX1278_DIO3_PORT                GPIOA
#define SX1278_DIO3_PIN                 GPIO_Pin_2
#define SX1278_DIO3_EXTI_PIN            EXTI_Pin_2
#define SX1278_DIO3_EXTI_IT_PIN         EXTI_IT_Pin2

#define SX1278_DIO4_PORT                GPIOC
#define SX1278_DIO4_PIN                 GPIO_Pin_6
#define SX1278_DIO4_EXTI_PIN            EXTI_Pin_6
#define SX1278_DIO4_EXTI_IT_PIN         EXTI_IT_Pin6

#define SX1278_DIO5_PORT                GPIOC
#define SX1278_DIO5_PIN                 GPIO_Pin_5
#define SX1278_DIO5_EXTI_PIN            EXTI_Pin_5
#define SX1278_DIO5_EXTI_IT_PIN         EXTI_IT_Pin5


#define SX1278_M0_PORT        GPIOC
#define SX1278_M0_PIN         GPIO_Pin_0
#define SX1278_M1_PORT        GPIOC
#define SX1278_M1_PIN         GPIO_Pin_1
#define SX1278_RX_PORT        GPIOC
#define SX1278_RX_PIN         GPIO_Pin_2
#define SX1278_TX_PORT        GPIOC
#define SX1278_TX_PIN         GPIO_Pin_3
#define SX1278_AUX_PORT        GPIOC
#define SX1278_AUX_PIN         GPIO_Pin_4

#define SX1278_IO1_PORT        GPIOB
#define SX1278_IO1_PIN         GPIO_Pin_2
#define SX1278_IO2_PORT        GPIOB
#define SX1278_IO2_PIN         GPIO_Pin_1
#define SX1278_IO3_PORT        GPIOB
#define SX1278_IO3_PIN         GPIO_Pin_0
#define SX1278_IO4_PORT        GPIOD
#define SX1278_IO4_PIN         GPIO_Pin_3
#define SX1278_IO5_PORT        GPIOD
#define SX1278_IO5_PIN         GPIO_Pin_2
#define SX1278_IO6_PORT        GPIOD
#define SX1278_IO6_PIN         GPIO_Pin_1

#define SX1278_TEST_PORT        GPIOA
#define SX1278_TEST_PIN         GPIO_Pin_5

void BoardInitMcu( void );
Run_Mode_Type GetRunModePin(void);

#endif