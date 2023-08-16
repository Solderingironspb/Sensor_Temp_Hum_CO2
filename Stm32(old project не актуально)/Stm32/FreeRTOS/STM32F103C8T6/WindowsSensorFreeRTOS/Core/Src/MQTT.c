#include "MQTT.h"

//bool WIFI_connected = false;
//bool MQTT_connected = false;
uint8_t NETWORK_Status = WIFI_STATUS_NOT_CONNECTED;
extern struct USART_name husart2; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)


/*----------Проверка подключения ESP8266 к Wifi и MQTT брокеру-----------------*/
/// Проверка подключения ESP8266 к Wifi и MQTT брокеру
void UART_MQTT_Check_net(void) {
	if (UART_MQTT_Checksumm_validation(husart2.rx_buffer)) {
		if ((husart2.rx_buffer[0] == 0xFF) && (husart2.rx_buffer[1] == 0xFF) && (husart2.rx_buffer[2] == 0xFF) && (husart2.rx_buffer[3] == 0xBB) && (husart2.rx_buffer[4] == 0xBB) && (husart2.rx_buffer[5] == 0xBB) && (husart2.rx_buffer[6] == 0xBB)) {
			//WIFI_connected = false; //Wifi еще не подключился
			NETWORK_Status = WIFI_STATUS_NOT_CONNECTED;
			TIM3->PSC = 7200-1;
			SET_BIT(TIM3->CR1, TIM_CR1_CEN);
		} else if ((husart2.rx_buffer[0] == 0xFF) && (husart2.rx_buffer[1] == 0xFF) && (husart2.rx_buffer[2] == 0xFF) && (husart2.rx_buffer[3] == 0xAA) && (husart2.rx_buffer[4] == 0xAA) && (husart2.rx_buffer[5] == 0xAA) && (husart2.rx_buffer[6] == 0xAA)) {
			//WIFI_connected = true; //Wifi подключился
			NETWORK_Status = WIFI_STATUS_CONNECTED;
			TIM3->PSC = 1800-1;
			SET_BIT(TIM3->CR1, TIM_CR1_CEN);
		} else if ((husart2.rx_buffer[0] == 0xFF) && (husart2.rx_buffer[1] == 0xFF) && (husart2.rx_buffer[2] == 0xFF) && (husart2.rx_buffer[3] == 0xDD) && (husart2.rx_buffer[4] == 0xDD) && (husart2.rx_buffer[5] == 0xDD) && (husart2.rx_buffer[6] == 0xDD)) {
			//MQTT_connected = false; //Wifi уже работает, но MQTT еще не подключился
			NETWORK_Status = MQTT_STATUS_NOT_CONNECTED;
			TIM3->PSC = 450-1;
			SET_BIT(TIM3->CR1, TIM_CR1_CEN);
		} else if ((husart2.rx_buffer[0] == 0xFF) && (husart2.rx_buffer[1] == 0xFF) && (husart2.rx_buffer[2] == 0xFF) && (husart2.rx_buffer[3] == 0xCC) && (husart2.rx_buffer[4] == 0xCC) && (husart2.rx_buffer[5] == 0xCC) && (husart2.rx_buffer[6] == 0xCC)) {
			//MQTT_connected = true; //MQTT подключился
			NETWORK_Status = MQTT_STATUS_CONNECTED;
			TIM3->PSC = 7200-1;
			CLEAR_BIT(TIM3->CR1, TIM_CR1_CEN);
			WIFI_LED_ON();
		}
	}
}
/*----------Проверка подключения ESP8266 к Wifi и MQTT брокеру-----------------*/

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
