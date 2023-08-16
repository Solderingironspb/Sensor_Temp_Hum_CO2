#include "main.h"

/*----------------------Описание проекта----------------------------*/
//USART1 - общение с модулем MH19Z (9600 8N1)
//USART2 - общение с модулем ESP8266(мой протокол для работы с MQTT) (9600 8N1)
//I2C1   - общение с модулем HDC1080 (standart mode)
/*----------------------Описание проекта----------------------------*/

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

extern uint8_t NETWORK_Status;
bool MH19Z_data_ready = false; //Разрешающий флаг на отправку полученных данных
bool HDC1080_data_ready = false; //Разрешающий флаг на отправку полученных данных
extern struct USART_name husart1; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)
extern struct USART_name husart2; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)

uint8_t USART1_tx_buffer[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 }; //Буфер запроса данных
uint8_t USART2_tx_buffer[20] = { 0, }; //Буфер для MQTT запросов
extern uint16_t CO2_ppm; //Уровень углекислого газа в ppm

extern uint8_t I2C1_tx_buffer[4];  //Исходящий буфер
extern uint8_t I2C1_rx_buffer[10];  //Входящий буфер
extern float Temperature, Humidity;

bool Task1 = false;
uint32_t USART1_Polling_Timer; //Таймер для опроса USART1
uint32_t USART1_Timer_LED; //Таймер для светодиода USART1
bool USART1_Data_ready = false;

bool Task2 = false;
uint32_t I2C1_Polling_Timer; //Таймер для опроса HDC1080
uint32_t I2C1_Timer_LED; //Таймер для светодиода I2C1

bool Task3 = false;
uint32_t MQTT_Send_Timer; //Таймер для отправки MQTT

uint32_t Error_Timer_LED; //Таймер для ошибок

bool flag_programm_start = false;

void Task1_MH_Z19_Polling(void) {
	CMSIS_USART_Transmit(USART1, USART1_tx_buffer, sizeof(USART1_tx_buffer), 100); //Отправим запрос датчику CO2
	/*Мигнем светодиодом*/
	USART1_LED_ON();
	Software_timer((uint32_t*) &USART1_Timer_LED, 100);
}

void Task2_HDC1080_Polling(void) {
	//Команда опроса температуры и влажности
	I2C1_tx_buffer[0] = 0;
	I2C1_tx_buffer[1] = 0;
	if (CMSIS_I2C_Data_Transmit(I2C1, HDC1080ADDR, I2C1_tx_buffer, 1, 100)) {
		I2C1_LED_ON(); //Покажем активность шины I2C1
		Software_timer((uint32_t*) &I2C1_Timer_LED, 100);
	} else {
		ERROR_LED_ON();
		Software_timer((uint32_t*) &Error_Timer_LED, 100);
	}
	Delay_ms(100);

	//Cчитаем данные
	if (CMSIS_I2C_Data_Receive(I2C1, HDC1080ADDR, I2C1_rx_buffer, 4, 100)) {
		Temperature = (((float) ((uint16_t) I2C1_rx_buffer[0] << 8 | I2C1_rx_buffer[1]) * 165) / 65536) - 40;
		Temperature = Temperature - 1;
		Humidity = ((float) ((uint16_t) I2C1_rx_buffer[2] << 8 | I2C1_rx_buffer[3]) * 100) / 65536;
		Humidity = Humidity + 5;
		HDC1080_data_ready = true;
	} else {
		ERROR_LED_ON();
		Software_timer((uint32_t*) &Error_Timer_LED, 100);
	}
}

void Task3_MQTT_Send(void) {
	if (NETWORK_Status == MQTT_STATUS_CONNECTED) {
		if (MH19Z_data_ready) {
			UART_MQTT_Send_data_uint16_t(MQTT_ID_CO2PPM, CO2_ppm, USART2_tx_buffer);
			Delay_ms(5);
			MH19Z_data_ready = false;

		}
		if (HDC1080_data_ready) {
			UART_MQTT_Send_data_float(MQTT_ID_TEMPERATURE, Temperature, USART2_tx_buffer);
			Delay_ms(5);
			UART_MQTT_Send_data_float(MQTT_ID_HUMIDITY, Humidity, USART2_tx_buffer);
			Delay_ms(5);
			HDC1080_data_ready = false;
		}
	}
}

int main(void) {
	CMSIS_Debug_init();
	CMSIS_RCC_SystemClock_72MHz();
	CMSIS_SysTick_Timer_init();
	CMSIS_TIM3_init(); //Таймер для индикации подключения к сети.
	GPIO_init(); //Настроим GPIO
	CMSIS_USART1_Init();
	CMSIS_USART2_Init();
	CMSIS_I2C1_Init();
	HDC1080_init();

	Software_timer((uint32_t*) &USART1_Polling_Timer, 5);
	Software_timer((uint32_t*) &I2C1_Polling_Timer, 5);
	Software_timer((uint32_t*) &MQTT_Send_Timer, 5);
	flag_programm_start = true;

	while (1) {
		/*Опрос датчика MH-Z19*/
		if (Task1) {
			Task1_MH_Z19_Polling();
			Task1 = false;
			Software_timer((uint32_t*) &USART1_Polling_Timer, 5000);
		}

		/*Опрос датчика HDC1080*/
		if (Task2) {
			Task2_HDC1080_Polling();
			Task2 = false;
			Software_timer(&I2C1_Polling_Timer, 5000);
		}

		/*Отправка данных по MQTT*/
		if (Task3) {
			Task3_MQTT_Send();
			Task3 = false;
			Software_timer((uint32_t*) &MQTT_Send_Timer, 5000);
		}

		/*Если в USART1 что-то есть - Обработаем данные*/
		if (USART1_Data_ready && husart1.rx_len > 0) {
			if (CRC8_MH_Z19_Check()) {
				//Рассчитаем количество CO2 в ppm
				CO2_ppm = husart1.rx_buffer[2] * 256 + husart1.rx_buffer[3];
				MH19Z_data_ready = true;
			}
		}

	}
}
