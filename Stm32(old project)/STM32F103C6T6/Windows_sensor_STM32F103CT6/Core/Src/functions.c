#include <main.h>
/*---------------------Переменные--------------------*/
//Флаги
extern bool flag_HDC1080_ready;  //Флаг готовности микросхемы HDC1080
extern bool flag_MH_Z19_ready; //Флаг готовности датчика MH-Z19B
extern bool MQTT_flag, WIFI_flag;  //Флаги для отслеживания подключения к Wi-fi и MQTT серверу
//Для шины I2C1
extern uint8_t I2C1_tx_buffer[4];  //Исходящий буфер
extern uint8_t I2C1_rx_buffer[10];  //Входящий буфер
//Модуль HDC1080
extern uint16_t HDC1080_Configuration;  //Настройки конфигурации модуля
extern float Temperature;  //Температура с датчика HDC1080
extern float Humidity;     //Влажность с датчика HDC1080
//Для шины USART1(MH-Z19B)
extern uint8_t CRC_MH_Z19; //CRC-8 для датчика MH-Z19B
extern uint16_t CO2_ppm;  //Концентрация углекислого газа


extern uint16_t TIM3_Wifi_OK;
extern uint8_t TIM3_Wifi_OK_cnt;
extern bool flag_MQTT_Send;


extern uint16_t TIM3_HDC1080;
extern uint32_t TIM3_MH_Z19;


/*---------------------Переменные--------------------*/

/*---------------------Структуры--------------------*/
extern struct USART_name husart1;
extern struct USART_name husart2;
/*---------------------Структуры--------------------*/


/*-----------------------Функция настроек ножек GPIO-----------------------*/
void GPIO_Init(void) {
	RCC->APB2ENR = RCC_APB2ENR_IOPAEN; //Запуск тактирование порта A
	/*--------------------Настройка светодиодной индикации--------------------*/
	// PA4-Wifi_OK
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE4, 0b10 << GPIO_CRL_MODE4_Pos);
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF4, 0b00 << GPIO_CRL_CNF4_Pos);
	// PA5-USART1_OK
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE5, 0b10 << GPIO_CRL_MODE5_Pos);
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF5, 0b00 << GPIO_CRL_CNF5_Pos);
	// PA6-I2C1_OK
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE6, 0b10 << GPIO_CRL_MODE6_Pos);
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF6, 0b00 << GPIO_CRL_CNF6_Pos);
	// PA7-Error
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_MODE7, 0b10 << GPIO_CRL_MODE7_Pos);
	MODIFY_REG(GPIOA->CRL, GPIO_CRL_CNF7, 0b00 << GPIO_CRL_CNF7_Pos);
	// PA8-ESP8266_Reset
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_MODE8, 0b10 << GPIO_CRH_MODE8_Pos);
	MODIFY_REG(GPIOA->CRH, GPIO_CRH_CNF8, 0b00 << GPIO_CRH_CNF8_Pos);
	/*--------------------Настройка светодиодной индикации--------------------*/

	Reset_ESP8266(); //Перезагрузим ESP8266

	/*--------Проверка светодиодной индикации--------*/
	GPIOA->BSRR = GPIO_BSRR_BS4;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BS5;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BS6;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BS7;
	Delay_ms(100);

	GPIOA->BSRR = GPIO_BSRR_BR4;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BR5;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BR6;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BR7;
	Delay_ms(100);

	GPIOA->BSRR = GPIO_BSRR_BS7;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BS6;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BS5;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BS4;
	Delay_ms(100);

	GPIOA->BSRR = GPIO_BSRR_BR7;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BR6;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BR5;
	Delay_ms(100);
	GPIOA->BSRR = GPIO_BSRR_BR4;
	Delay_ms(100);
	/*--------Проверка светодиодной индикации--------*/
}
/*-----------------------Функция настроек ножек GPIO-----------------------*/


/*----------Отправка uint16_t в массив-----------------*/
/// Отправка uint16_t в tx_buffer(принимающему устройству)
/// \param VARIABLE_ID - номер переменной(должен совпадать на передающем и
/// принимающем устройстве) \param data - переменная, которую следует отправить
/// \param *tx_buffer - буфер, который используется для отправки
void UART_MQTT_Send_data_uint16_t(uint8_t VARIABLE_ID, uint16_t data, uint8_t* tx_buffer) {
	tx_buffer[0] = 0xFF;  // Start byte
	tx_buffer[1] = VARIABLE_ID;
	tx_buffer[2] = 0x05;
	tx_buffer[3] = data >> 16;
	tx_buffer[4] = data >> 16;
	tx_buffer[5] = data >> 8;
	tx_buffer[6] = data;
	uint16_t CRC_tx_buffer = (~(tx_buffer[1] + tx_buffer[2] + tx_buffer[3] + tx_buffer[4] + tx_buffer[5] + tx_buffer[6])) + 1;
	tx_buffer[7] = CRC_tx_buffer >> 8;
	tx_buffer[8] = CRC_tx_buffer;

	CMSIS_USART_Transmit(USART2, tx_buffer, 9, 100);
}
/*----------Отправка uint16_t в массив-----------------*/

/*----------Отправка float в массив-----------------*/
/// Отправка float в tx_buffer(принимающему устройству)
/// \param VARIABLE_ID - номер переменной(должен совпадать на передающем и
/// принимающем устройстве) \param data - переменная, которую следует отправить
/// \param *tx_buffer - буфер, который используется для отправки
void UART_MQTT_Send_data_float(uint8_t VARIABLE_ID, float data, uint8_t* tx_buffer) {
	tx_buffer[0] = 0xFF;
	tx_buffer[1] = VARIABLE_ID;
	tx_buffer[2] = 0x08;
	tx_buffer[3] = *((uint32_t*)&data) >> 24;
	tx_buffer[4] = *((uint32_t*)&data) >> 16;
	tx_buffer[5] = *((uint32_t*)&data) >> 8;
	tx_buffer[6] = *((uint32_t*)&data);
	uint16_t CRC_tx_buffer = (~(tx_buffer[1] + tx_buffer[2] + tx_buffer[3] + tx_buffer[4] + tx_buffer[5] + tx_buffer[6])) + 1;
	tx_buffer[7] = CRC_tx_buffer >> 8;
	tx_buffer[8] = CRC_tx_buffer;

	CMSIS_USART_Transmit(USART2, tx_buffer, 9, 100);
}
/*----------Отправка float в массив-----------------*/

/*----------Проверка контрольной суммы входящего сообщения-----------------*/
/// Проверка контрольной суммы входящего сообщения
/// \param *rx_buffer - буфер, используемый для приема данных
bool UART_MQTT_Checksumm_validation(uint8_t* rx_buffer) {
	bool result;
	uint16_t CRC_rx_buffer = 0x0000;
	uint16_t CRC_check = 0x0000;
	CRC_rx_buffer = (~(rx_buffer[1] + rx_buffer[2] + rx_buffer[3] + rx_buffer[4] + rx_buffer[5] + rx_buffer[6])) + 1;
	CRC_check = (CRC_check | rx_buffer[7] << 8) | (CRC_check | rx_buffer[8]);
	//CRC_rx_buffer == CRC_check ? result = true : result = false;
	if (CRC_rx_buffer == CRC_check) {
		result = true;
	} else {
		result = false;
	}
	return result;
}
/*----------Проверка контрольной суммы входящего сообщения-----------------*/

/*----------Проверка подключения ESP8266 к Wifi и MQTT брокеру-----------------*/
/// Проверка подключения ESP8266 к Wifi и MQTT брокеру
void UART_MQTT_Check_net(void) {
	if (UART_MQTT_Checksumm_validation(husart2.rx_buffer)) {
		if ((husart2.rx_buffer[0] == 0xFF) && (husart2.rx_buffer[1] == 0xFF) && (husart2.rx_buffer[2] == 0xFF) && (husart2.rx_buffer[3] == 0xBB) && (husart2.rx_buffer[4] == 0xBB) && (husart2.rx_buffer[5] == 0xBB) && (husart2.rx_buffer[6] == 0xBB)) {
			WIFI_flag = 0; //Wifi еще не подключился
		} else if ((husart2.rx_buffer[0] == 0xFF) && (husart2.rx_buffer[1] == 0xFF) && (husart2.rx_buffer[2] == 0xFF) && (husart2.rx_buffer[3] == 0xAA) && (husart2.rx_buffer[4] == 0xAA) && (husart2.rx_buffer[5] == 0xAA) && (husart2.rx_buffer[6] == 0xAA)) {
			WIFI_flag = 1; //Wifi подключился
		} else if ((husart2.rx_buffer[0] == 0xFF) && (husart2.rx_buffer[1] == 0xFF) && (husart2.rx_buffer[2] == 0xFF) && (husart2.rx_buffer[3] == 0xDD) && (husart2.rx_buffer[4] == 0xDD) && (husart2.rx_buffer[5] == 0xDD) && (husart2.rx_buffer[6] == 0xDD)) {
			MQTT_flag = 0; //Wifi уже работает, но MQTT еще не подключился
		} else if ((husart2.rx_buffer[0] == 0xFF) && (husart2.rx_buffer[1] == 0xFF) && (husart2.rx_buffer[2] == 0xFF) && (husart2.rx_buffer[3] == 0xCC) && (husart2.rx_buffer[4] == 0xCC) && (husart2.rx_buffer[5] == 0xCC) && (husart2.rx_buffer[6] == 0xCC)) {
			MQTT_flag = 1; //MQTT подключился
		}
	}
}
/*----------Проверка подключения ESP8266 к Wifi и MQTT брокеру-----------------*/


/*---------------------Опрос модуля HDC1080---------------------*/
bool HDC1080_Transmit(void) {
	//Команда опроса температуры и влажности
	I2C1_tx_buffer[0] = 0;
	I2C1_tx_buffer[1] = 0;
	if (CMSIS_I2C_Data_Transmit(I2C1, HDC1080ADDR, I2C1_tx_buffer, 1, 100)) {
		I2C1_LED_ON; //Покажем активность шины I2C1
		ERROR_LED_OFF; //Ошибок нет
		return true;
	} else {
		I2C1_LED_ON; //Покажем активность шины I2C1
		ERROR_LED_ON; //Ошибка общения
		return false;
	}
	//Delay_ms(100);
}

bool HDC1080_Receive(void) {
	//Cчитаем данные
	if (CMSIS_I2C_Data_Receive(I2C1, HDC1080ADDR, I2C1_rx_buffer, 4, 100)) {
		I2C1_LED_OFF; //Покажем активность шины I2C1
		ERROR_LED_OFF; //Ошибок нет
		Temperature = (((float)((uint16_t)I2C1_rx_buffer[0] << 8 | I2C1_rx_buffer[1]) * 165) / 65536) - 40;
		Temperature = Temperature - 1;
		Humidity = ((float)((uint16_t)I2C1_rx_buffer[2] << 8 | I2C1_rx_buffer[3]) * 100) / 65536;
		Humidity = Humidity + 5;
		return true;
	} else {
		I2C1_LED_OFF; //Покажем активность шины I2C1
		ERROR_LED_ON; //Ошибка общения
		return false;
	}
}

/*-----------------------Функция запроса датчику MH-Z19B-----------------------*/
void MH_Z19B_Check(void) {
	USART1_LED_ON; //Покажем активность шины USART1
	CMSIS_USART_Transmit(USART1, husart1.tx_buffer, 9, 100);
	USART1_LED_OFF; //Покажем активность шины USART1
}
/*-----------------------Функция запроса датчику MH-Z19B-----------------------*/

/*-----------------------Функция перезагрузки ESP8266-----------------------*/
void Reset_ESP8266(void) {
	ESP8266_RESET_ON; //Нажмем кнопку ресет на ESP8266
	Delay_ms(500);
	ESP8266_RESET_OFF; //Отпустим кнопку ресет на ESP8266
	Delay_ms(200);
}
/*-----------------------Функция перезагрузки ESP8266-----------------------*/


/*------------------------Проверка CRC-8 от MH-Z19B------------------------*/
bool CRC8_MH_Z19_Check(void) {
	CRC_MH_Z19 = (~(husart1.rx_buffer[1] + husart1.rx_buffer[2] + husart1.rx_buffer[3] + husart1.rx_buffer[4] + husart1.rx_buffer[5] + husart1.rx_buffer[6] + husart1.rx_buffer[7])) + 1;
	if (CRC_MH_Z19 == husart1.rx_buffer[8]) {
		return true;
	} else {
		return false;
	}
}
/*------------------------Проверка CRC-8 от MH-Z19B------------------------*/