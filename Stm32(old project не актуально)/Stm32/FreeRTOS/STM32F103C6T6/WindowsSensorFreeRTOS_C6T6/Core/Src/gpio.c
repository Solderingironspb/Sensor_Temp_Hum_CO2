#include "gpio.h"

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

void GPIO_init(void) {
	/*Настройка PA2 PA3 PA9 PA10 производятся при инициализации USART1 и USART2(см. main.c)*/
	/*Настройка PB6 PB7 производятся при инициализации I2C1 (см. main.c)*/

	/*Инициализация PA4 Wifi LED*/
	CMSIS_GPIO_init(GPIOA, 4, GPIO_GENERAL_PURPOSE_OUTPUT, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_50_MHZ);

	/*Инициализация PA5 USART1 LED*/
	CMSIS_GPIO_init(GPIOA, 5, GPIO_GENERAL_PURPOSE_OUTPUT, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_50_MHZ);

	/*Инициализация PBA I2C1 LED*/
	CMSIS_GPIO_init(GPIOA, 6, GPIO_GENERAL_PURPOSE_OUTPUT, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_50_MHZ);

	/*Инициализация PA7 Error LED*/
	CMSIS_GPIO_init(GPIOA, 7, GPIO_GENERAL_PURPOSE_OUTPUT, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_50_MHZ);

	/*Инициализация PA8 ESP8266 Reset*/
	CMSIS_GPIO_init(GPIOA, 8, GPIO_GENERAL_PURPOSE_OUTPUT, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_50_MHZ);

	ESP8266_Reset(); //Сбросим ESP8266

	LED_Indication_Test(); //Тест световой индикации

}

/**
 ***************************************************************************************
 *  @breif Сброс ESP8266
 *  При запуске МК - сбросим ESP8266, чтоб инициализация прошла синхронно
 ***************************************************************************************
 */
void ESP8266_Reset(void) {
	ESP8266_RESET_ON(); //Нажмем кнопку ресет на ESP8266
	Delay_ms(500);
	ESP8266_RESET_OFF(); //Отпустим кнопку ресет на ESP8266
	Delay_ms(200);
}

/**
 ***************************************************************************************
 *  @breif Проверка индикации диодов
 *  При запуске МК красиво пробегут огоньки сверху вниз и обратно
 ***************************************************************************************
 */
void LED_Indication_Test(void) {
	WIFI_LED_ON();
	Delay_ms(100);
	USART1_LED_ON();
	Delay_ms(100);
	I2C1_LED_ON();
	Delay_ms(100);
	ERROR_LED_ON();
	Delay_ms(100);

	WIFI_LED_OFF();
	Delay_ms(100);
	USART1_LED_OFF();
	Delay_ms(100);
	I2C1_LED_OFF();
	Delay_ms(100);
	ERROR_LED_OFF();
	Delay_ms(100);

	ERROR_LED_ON();
	Delay_ms(100);
	I2C1_LED_ON();
	Delay_ms(100);
	USART1_LED_ON();
	Delay_ms(100);
	WIFI_LED_ON();
	Delay_ms(100);

	ERROR_LED_OFF();
	Delay_ms(100);
	I2C1_LED_OFF();
	Delay_ms(100);
	USART1_LED_OFF();
	Delay_ms(100);
	WIFI_LED_OFF();
	Delay_ms(100);
}
