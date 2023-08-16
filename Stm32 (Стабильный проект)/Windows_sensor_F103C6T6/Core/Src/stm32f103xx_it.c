#include "stm32f103xx_it.h"

extern struct USART_name husart1; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)
extern struct USART_name husart2; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)
extern volatile uint32_t SysTimer_ms; //Переменная, аналогичная HAL_GetTick()
extern volatile uint32_t Delay_counter_ms; //Счетчик для функции Delay_ms
extern volatile uint32_t Timeout_counter_ms; //Переменная для таймаута функций
extern bool USART1_Data_ready;

void USART1_IRQHandler(void) {
	if (READ_BIT(USART1->SR, USART_SR_RXNE)) {
		//Если пришли данные по USART
		if (husart1.rx_counter < USART_MAX_LEN_RX_BUFFER) { //Если байт прилетело меньше, чем размер буфера
			husart1.rx_buffer[husart1.rx_counter] = USART1->DR; //Считаем данные в соответствующую ячейку в rx_buffer
			husart1.rx_counter++; //Увеличим счетчик принятых байт на 1
		} else {
			husart1.rx_counter = 0; //Если больше - сбросим счетчик.
		}
	}
	if (READ_BIT(USART1->SR, USART_SR_IDLE)) {
		//Если прилетел флаг IDLE
		USART1->DR; //Сбросим флаг IDLE
		husart1.rx_len = husart1.rx_counter; //Узнаем, сколько байт получили
		husart1.rx_counter = 0; //сбросим счетчик приходящих данных
		USART1_Data_ready = true; //Сообщим о том, что получили данные

	}
}

void USART2_IRQHandler(void) {
	if (READ_BIT(USART2->SR, USART_SR_RXNE)) {
		//Если пришли данные по USART
		if (husart2.rx_counter < USART_MAX_LEN_RX_BUFFER) { //Если байт прилетело меньше, чем размер буфера
			husart2.rx_buffer[husart2.rx_counter] = USART2->DR; //Считаем данные в соответствующую ячейку в rx_buffer
			husart2.rx_counter++; //Увеличим счетчик принятых байт на 1
		} else {
			husart2.rx_counter = 0; //Если больше - сбросим счетчик
		}
	}
	if (READ_BIT(USART2->SR, USART_SR_IDLE)) {
		//Если прилетел флаг IDLE
		USART2->DR; //Сбросим флаг IDLE
		husart2.rx_len = husart2.rx_counter; //Узнаем, сколько байт получили
		husart2.rx_counter = 0; //сбросим счетчик приходящих данных

		/*----------Тут будем работать с приходящими данными----------*/
		UART_MQTT_Check_net(); //Проверка подключения к Wifi и MQTT брокеру
		/*----------Тут будем работать с приходящими данными----------*/
	}
}

extern bool flag_programm_start;

extern bool Task1;
extern uint32_t USART1_Polling_Timer; //Таймер для опроса USART1
extern uint32_t USART1_Timer_LED; //Таймер для светодиода USART1

extern bool Task2;
extern uint32_t I2C1_Polling_Timer; //Таймер для опроса HDC1080
extern uint32_t I2C1_Timer_LED; //Таймер для светодиода I2C1

extern bool Task3;
extern uint32_t MQTT_Send_Timer; //Таймер для отправки MQTT

uint32_t Error_Timer_LED; //Таймер для ошибок

void SysTick_Handler(void) {

	SysTimer_ms++;

	if (Delay_counter_ms) {
		Delay_counter_ms--;
	}
	if (Timeout_counter_ms) {
		Timeout_counter_ms--;
	}

	if (flag_programm_start) {

		if (!Software_timer_check((uint32_t*) &USART1_Timer_LED)) {
			USART1_LED_OFF();
		}

		if (!Software_timer_check((uint32_t*) &I2C1_Timer_LED)) {
			I2C1_LED_OFF();
		}

		if (!Software_timer_check((uint32_t*) &Error_Timer_LED)) {
			ERROR_LED_OFF();
		}

		if (!Software_timer_check((uint32_t*) &USART1_Polling_Timer)) {
			Task1 = true;
		}

		if (!Software_timer_check((uint32_t*) &I2C1_Polling_Timer)) {
			Task2 = true;
		}
		if (!Software_timer_check((uint32_t*) &MQTT_Send_Timer)) {
			Task3 = true;
		}

	}

#if defined FreeRTOS_USE
	GPIOC->BSRR = GPIO_BSRR_BS13;
    GPIOC->BSRR = GPIO_BSRR_BR13;
	xPortSysTickHandler();
#endif
}

bool LED_Network = false; //Помигаем светодиодом
extern uint8_t NETWORK_Status;

void TIM3_IRQHandler(void) {
	if (READ_BIT(TIM3->SR, TIM_SR_UIF)) {
		CLEAR_BIT(TIM3->SR, TIM_SR_UIF); //Сбросим флаг прерывания
	}

	if (flag_programm_start) {
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
