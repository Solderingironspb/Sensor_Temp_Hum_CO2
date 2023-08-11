#include "HDC1080.h"


uint8_t I2C1_tx_buffer[4] = { 0, };  //Исходящий буфер
uint8_t I2C1_rx_buffer[10] = { 0, };  //Входящий буфер
float Temperature, Humidity;


void HDC1080_init(void){
	/*-------------------Просмотр настроек для модуля HDC1080-------------------*/
	I2C1_tx_buffer[0] = 0x02;  //Команда конфигурация
	I2C1_tx_buffer[1] = 0x10;
	I2C1_tx_buffer[2] = 0x00;
	//Скажем модулю, что хотим записать данные на адрес "Configuration" со значением 0x0000
	if (CMSIS_I2C_Data_Transmit(I2C1, HDC1080ADDR, I2C1_tx_buffer, 3, 100)) {
		ERROR_LED_OFF();  //Неисправностей нет
	} else {
		ERROR_LED_ON();//Появится сигнализация о неисправности
	}
	Delay_ms(20);
	//Скажем модулю, что хотим просмотреть данные с адреса "Configuration"
	if (CMSIS_I2C_Data_Transmit(I2C1, HDC1080ADDR, I2C1_tx_buffer, 1, 100)) {
		ERROR_LED_OFF();  //Неисправностей нет
	} else {
		ERROR_LED_ON();//Появится сигнализация о неисправности
	}
	Delay_ms(20);

	//Сохраним конфигурацию в переменную
	if (CMSIS_I2C_Data_Receive(I2C1, HDC1080ADDR, I2C1_rx_buffer, 2, 100)) {
		ERROR_LED_OFF();  //Неисправностей нет
	} else {
		ERROR_LED_ON();//Появится сигнализация о неисправности
	}
	/*-------------------Просмотр настроек для модуля HDC1080-------------------*/
}
