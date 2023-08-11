#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f1xx.h>
#include <stdbool.h>
#include "stm32f103xx_CMSIS.h"
#include "tasks_list.h"
#include "gpio.h"
#include "mh19z.h"
#include "HDC1080.h"
#include "MQTT.h"

/*Структура для входящих данных по UART. Используем для очередей*/
struct USART_rx_data {
	uint8_t rx_buffer[20]; //буфер для входящих данных
	uint8_t len; //сколько байт данных приняли
};

enum {
	WIFI_STATUS_NOT_CONNECTED, WIFI_STATUS_CONNECTED, MQTT_STATUS_NOT_CONNECTED, MQTT_STATUS_CONNECTED
};

#ifdef __cplusplus

}
#endif

#endif /* __MAIN_H */

