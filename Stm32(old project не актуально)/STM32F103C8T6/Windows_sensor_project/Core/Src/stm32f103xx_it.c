#include <main.h>

/*---------------------Переменные--------------------*/
//Флаги
extern bool flag_HDC1080_ready;  //Флаг готовности микросхемы HDC1080
extern bool flag_MH_Z19_ready; //Флаг готовности датчика MH-Z19B
extern bool MQTT_flag, WIFI_flag;  //Флаги для отслеживания подключения к Wi-fi и MQTT серверу
extern bool flag_I2C1_send; //Отправка данных по I2C1
extern bool flag_TIM3_HDC1080_start; //Запуск таймера для HDC1080
extern bool flag_TIM3_CO2_start; //Запуск таймера для MH-Z19
extern bool flag_USART1_send; //Отправка данных по USART1
extern bool flag_TIM3_MQTT_start; //Отправка данных по MQTT
extern uint8_t flag_MQTT_Send; //Битовая маска для разрешения отправки 8 переменных по MQTT
extern bool flag_TIM3_ESP8266_not_connect_start; //Запуск таймера для ситуации, когда устройство долго не может подключиться к сети(сброс esp8266)
extern bool flag_Reset_ESP8266; //Ресет ESP8266
//Для шины I2C1
extern uint8_t I2C1_tx_buffer[4];  //Исходящий буфер
extern uint8_t I2C1_rx_buffer[10];  //Входящий буфер
//Модуль HDC1080
extern uint16_t HDC1080_Configuration;  //Настройки конфигурации модуля
extern float Temperature;  //Температура с датчика HDC1080
extern float Humidity;     //Влажность с датчика HDC1080
extern uint8_t queue_I2C1; //Очередь для работы с данными от HDC1080
//Для шины USART1(MH-Z19B)
extern uint8_t CRC_MH_Z19; //CRC-8 для датчика MH-Z19B
extern uint16_t CO2_ppm;  //Концентрация углекислого газа
//Таймеры
uint16_t TIM3_HDC1080 = 0; //Таймер для модуля HDC1080
uint32_t TIM3_MH_Z19 = 0; //Таймер для модуля MH-Z19B
uint16_t TIM3_MQTT_Send = 0; //Таймер для отправки данных по MQTT
uint16_t TIM3_Wifi_OK = 0; //Таймер для индикатора подключения устройства к Wifi и MQTT
uint16_t TIM3_ESP8266_not_connect = 0; //Таймер для ситуации, когда устройство долго не может подключиться к сети

uint8_t TIM3_Wifi_OK_cnt = 0;
bool flag_TIM3_Wifi_OK = true;



extern struct USART_name husart1;
extern struct USART_name husart2;




/*--------------------Прерывание от USART1--------------------*/
void USART1_IRQHandler(void) {
	if (READ_BIT(USART1->SR, USART_SR_RXNE)) {
		//Если пришли данные по USART
		husart1.rx_buffer[husart1.rx_counter] = USART1->DR; //Считаем данные в соответствующую ячейку в rx_buffer
		husart1.rx_counter++; //Увеличим счетчик принятых байт на 1
	}
	if (READ_BIT(USART1->SR, USART_SR_IDLE)) {
		//Если прилетел флаг IDLE
		USART1->DR; //Сбросим флаг IDLE
		husart1.rx_len = husart1.rx_counter; //Узнаем, сколько байт получили

		/*----------Тут будем работать с приходящими данными----------*/
		if (CRC8_MH_Z19_Check()) {//Если CRC8 бьется
			CO2_ppm = husart1.rx_buffer[2] * 256 + husart1.rx_buffer[3]; //Рассчитаем количество CO2 в ppm
		}
		/*----------Тут будем работать с приходящими данными----------*/

		husart1.rx_counter = 0; //сбросим счетчик приходящих данных
	}
}
/*--------------------Прерывание от USART1--------------------*/

/*--------------------Прерывание от USART2--------------------*/
void USART2_IRQHandler(void) {
	if (READ_BIT(USART2->SR, USART_SR_RXNE)) {
		//Если пришли данные по USART
		husart2.rx_buffer[husart2.rx_counter] = USART2->DR; //Считаем данные в соответствующую ячейку в rx_buffer
		husart2.rx_counter++; //Увеличим счетчик принятых байт на 1
	}
	if (READ_BIT(USART2->SR, USART_SR_IDLE)) {
		//Если прилетел флаг IDLE
		USART2->DR; //Сбросим флаг IDLE
		husart2.rx_len = husart2.rx_counter; //Узнаем, сколько байт получили

		/*----------Тут будем работать с приходящими данными----------*/
		UART_MQTT_Check_net();//Проверка подключения к Wifi и MQTT брокеру
		/*----------Тут будем работать с приходящими данными----------*/

		husart2.rx_counter = 0; //сбросим счетчик приходящих данных
	}
}
/*--------------------Прерывание от USART2--------------------*/

/*----------------------------------------Прерывание от TIM3----------------------------------------*/
void TIM3_IRQHandler(void) {
	if (READ_BIT(TIM3->SR, TIM_SR_UIF)) {
		CLEAR_BIT(TIM3->SR, TIM_SR_UIF); //Сбросим флаг прерывания

		/*----------------Индикатор для Wifi_OK----------------*/
		TIM3_Wifi_OK++;
		if (!WIFI_flag && !MQTT_flag) {
			if (TIM3_Wifi_OK == 500) {
				GPIOA->BSRR = GPIO_BSRR_BS4; //Wifi_OK_on
			} else if (TIM3_Wifi_OK == 1000) {
				GPIOA->BSRR = GPIO_BSRR_BR4; //Wifi_OK_off	
			} else if (TIM3_Wifi_OK > 1000) {
				TIM3_Wifi_OK = 0;
			}
		} else if (WIFI_flag && !MQTT_flag) {
			if (TIM3_Wifi_OK == 100) {
				GPIOA->BSRR = GPIO_BSRR_BS4; //Wifi_OK_on
			} else if (TIM3_Wifi_OK == 200) {
				GPIOA->BSRR = GPIO_BSRR_BR4; //Wifi_OK_off	
			} else if (TIM3_Wifi_OK > 200) {
				TIM3_Wifi_OK = 0;
			}
		} else if (WIFI_flag && MQTT_flag) {
			GPIOA->BSRR = GPIO_BSRR_BS4; //Wifi_OK_on
		}
		/*----------------Индикатор для Wifi_OK----------------*/

		/*-------------------Опрос модуля HDC10080-------------------*/
		if (flag_HDC1080_ready && flag_TIM3_HDC1080_start) {
			if (TIM3_HDC1080 == 0) {
				queue_I2C1 = 0;
				flag_I2C1_send = true;
			} else if (TIM3_HDC1080 == 20) {
				queue_I2C1 = 1;
				flag_I2C1_send = true;
			}
			TIM3_HDC1080++;

			if (TIM3_HDC1080 == 10020) {
				TIM3_HDC1080 = 0;
				queue_I2C1 = 0;
			}
		}
		/*-------------------Опрос модуля HDC10080-------------------*/

		/*-------------------Опрос датчика MH-Z19B-------------------*/
		if (flag_TIM3_CO2_start) { //Если таймер запущен
			if (!flag_MH_Z19_ready) {//А 3 минут от падачи питания еще не прошло
				TIM3_MH_Z19++;
				if (TIM3_MH_Z19 == 180000) {
					flag_MH_Z19_ready = true; //3 минуты прошло. Датчик прогрелся. Можно опрашивать.
					TIM3_MH_Z19 = 0;
				}
			} else {//Если 3 минуты прошло и датчик прогрелся.
				if (TIM3_MH_Z19 == 0) {
					flag_USART1_send = true;
				}
				TIM3_MH_Z19++;
				if (TIM3_MH_Z19 == 10000) {
					TIM3_MH_Z19 = 0;
				}
			}
		}
		/*-------------------Опрос датчика MH-Z19B-------------------*/

		/*-------------------------Отправка данных на ESP8266-------------------------*/
		if (flag_TIM3_MQTT_start) {
			if (TIM3_MQTT_Send == 0) {
				if (flag_HDC1080_ready) {
					SET_BIT(flag_MQTT_Send, MQTT_SEND_TEMPERATURE); //Разрешим отправку температуры по MQTT
					SET_BIT(flag_MQTT_Send, MQTT_SEND_HUMIDITY);  //Разрешим отправку влажности по MQTT
				}
				if (flag_MH_Z19_ready) {
					SET_BIT(flag_MQTT_Send, MQTT_SEND_CO2PPM);  //Разрешим отправку CO2_ppm по MQTT
				}
			}
			TIM3_MQTT_Send++;
			if (TIM3_MQTT_Send == 10000) {
				TIM3_MQTT_Send = 0;
			}
		}
		/*-------------------------Отправка данных на ESP8266-------------------------*/

		/*--------------------------Сброс ESP8266, если устройство долго не подключается к сети--------------------------*/
		if (flag_TIM3_ESP8266_not_connect_start) {
			TIM3_ESP8266_not_connect++;
			if (TIM3_ESP8266_not_connect == 60000) {
				TIM3_ESP8266_not_connect = 0; //Сбросим таймер
				flag_Reset_ESP8266 = true; //Перезагрузим ESP8266
			}
		}
		/*--------------------------Сброс ESP8266, если устройство долго не подключается к сети--------------------------*/

	}

}
/*----------------------------------------Прерывание от TIM3----------------------------------------*/