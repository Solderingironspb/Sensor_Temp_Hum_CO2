#ifndef INC_GPIO_H_
#define INC_GPIO_H_

#include "main.h"

/*-----------------------Распиновка-----------------------*/
//PA2 USART2 Tx
//PA3 USART2 Rx
//PA4 Wifi LED
//PA5 USART1 LED
//PA6 I2C1 LED
//PA7 Error LED
//PA8 ESP8266 Reset
//PA9 USART1 Tx
//PA10 USART1 Rx
//PB6 SCL (I2C1 clock) Alternate function open drain
//PB7 SDA (I2C1 Data I/O) Alternate function open drain
/*-----------------------Распиновка-----------------------*/

/*---------------------------------------Макросы для GPIO-------------------------------------------------*/
#define WIFI_LED_ON()            GPIOA->BSRR = GPIO_BSRR_BS4; //Wifi LED On
#define WIFI_LED_OFF()           GPIOA->BSRR = GPIO_BSRR_BR4; //Wifi LED Off

#define USART1_LED_ON()          GPIOA->BSRR = GPIO_BSRR_BS5; //USART1 LED On
#define USART1_LED_OFF()         GPIOA->BSRR = GPIO_BSRR_BR5; //USART1 LED Off

#define I2C1_LED_ON()            GPIOA->BSRR = GPIO_BSRR_BS6; //I2C1 LED On
#define I2C1_LED_OFF()           GPIOA->BSRR = GPIO_BSRR_BR6; //I2C1 LED Off

#define ERROR_LED_ON()           GPIOA->BSRR = GPIO_BSRR_BS7; //Error LED On
#define ERROR_LED_OFF()          GPIOA->BSRR = GPIO_BSRR_BR7; //Error LED Off

#define ESP8266_RESET_ON()       GPIOA->BSRR = GPIO_BSRR_BS8; //Нажать кнопку ресет у ESP8266
#define ESP8266_RESET_OFF()      GPIOA->BSRR = GPIO_BSRR_BR8; //Отпустить кнопку ресет у ESP8266
/*---------------------------------------Макросы для GPIO-------------------------------------------------*/

void GPIO_init(void);
void ESP8266_Reset(void);
void LED_Indication_Test(void);


#endif /* INC_GPIO_H_ */
