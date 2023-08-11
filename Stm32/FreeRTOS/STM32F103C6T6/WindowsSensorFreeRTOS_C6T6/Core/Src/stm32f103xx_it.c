#include "stm32f103xx_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

extern bool start_os;
extern struct USART_name husart1; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)
extern struct USART_name husart2; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)
extern struct USART_rx_data usart1_RX;
extern QueueHandle_t xQueue1;
bool LED_Network = false; //Помигаем светодиодом
extern uint8_t NETWORK_Status;

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

		/*----Подготовим данные для очереди----*/
		usart1_RX.len = husart1.rx_len;
		for (uint8_t i = 0; i < usart1_RX.len; i++) {
			usart1_RX.rx_buffer[i] = husart1.rx_buffer[i];
		}
		/*----Подготовим данные для очереди----*/
		BaseType_t StatusSend;
		if (xQueue1 != NULL) {
			StatusSend = xQueueSendFromISR(xQueue1, &usart1_RX, NULL);
			if (StatusSend == errQUEUE_FULL) {
				//Переполнение очереди
				ERROR_LED_ON();
			}
		}

		husart1.rx_counter = 0; //сбросим счетчик приходящих данных

	}
}

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
		UART_MQTT_Check_net(); //Проверка подключения к Wifi и MQTT брокеру
		/*----------Тут будем работать с приходящими данными----------*/

		husart2.rx_counter = 0; //сбросим счетчик приходящих данных
	}
}

void TIM3_IRQHandler(void) {
	if (READ_BIT(TIM3->SR, TIM_SR_UIF)) {
		CLEAR_BIT(TIM3->SR, TIM_SR_UIF); //Сбросим флаг прерывания
	}

	if (start_os) {
		if (NETWORK_Status != MQTT_STATUS_CONNECTED) {
			LED_Network = !LED_Network;
			if (LED_Network) {
				WIFI_LED_ON();
			} else {
				WIFI_LED_OFF();
			}
		}
	}
}
