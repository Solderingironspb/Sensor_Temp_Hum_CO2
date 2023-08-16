#include <main.h>

/*---------------------Переменные--------------------*/
//Флаги
bool flag_HDC1080_ready = false;  //Флаг готовности микросхемы HDC1080
bool flag_MH_Z19_ready = false; //Флаг готовности датчика MH-Z19B
bool MQTT_flag, WIFI_flag; //Флаги для отслеживания подключения к Wi-fi и MQTT серверу
bool flag_I2C1_send = false; //Отправка данных по I2C1
bool flag_TIM3_HDC1080_start = false; //Запуск таймера для HDC1080
bool flag_TIM3_CO2_start = false; //Запуск таймера для MH-Z19
bool flag_USART1_send = false; //Отправка данных по USART1
bool flag_TIM3_MQTT_start = false; //Запуск таймера для MQTT
uint8_t flag_MQTT_Send = 0; //Битовая маска для разрешения отправки 8 переменных по MQTT
bool flag_TIM3_ESP8266_not_connect_start = 0; //Запуск таймера для ситуации, когда устройство долго не может подключиться к сети(сброс esp8266)
bool flag_Reset_ESP8266 = false; //Ресет ESP8266
//Для шины I2C1
uint8_t I2C1_tx_buffer[4] = { 0, };  //Исходящий буфер
uint8_t I2C1_rx_buffer[10] = { 0, };  //Входящий буфер
//Модуль HDC1080
uint16_t HDC1080_Configuration = 0;  //Настройки конфигурации модуля
float Temperature = 0.0f;  //Температура с датчика HDC1080
float Humidity = 0.0f;     //Влажность с датчика HDC1080
uint8_t queue_I2C1 = 0; //Очередь для работы с данными от HDC1080
//Для шины USART1(MH-Z19B)
uint8_t CRC_MH_Z19 = 0; //CRC-8 для датчика MH-Z19B
uint16_t CO2_ppm = 0;  //Концентрация углекислого газа
//Таймеры
extern uint16_t TIM3_HDC1080;
extern uint32_t TIM3_MH_Z19;
extern uint16_t TIM3_MQTT_Send;
extern uint16_t TIM3_ESP8266_not_connect; //Таймер для ситуации, когда устройство долго не может подключиться к сети

/*---------------------Структуры--------------------*/
extern struct USART_name husart1;
extern struct USART_name husart2;
/*---------------------Структуры--------------------*/

/*---------------------Переменные--------------------*/

int main(void) {
	CMSIS_Debug_init();             //Настройка дебага
	CMSIS_RCC_SystemClock_72MHz();  //Настройка МК на частоту 72 МГц
	CMSIS_SysTick_Timer_init();  //Инициализация системного таймера
	GPIO_Init();                 //Настройка ножек GPIO
	CMSIS_I2C1_Init();    //Инициализация I2C1 для HDC1080
	CMSIS_USART1_Init();  //Инициализация USART1 для MH-Z19
	CMSIS_USART2_Init();  //Инициализация USART2 для ESP8266
	CMSIS_TIM3_init();    //Инициализация таймера 3.


	flag_TIM3_CO2_start = true; //Запустим таймер для MH-Z19B

	/*-------------------Заполним Tx буфер для опроса датчика углекислого газа------------------- */
	husart1.tx_buffer[0] = 0xFF;
	husart1.tx_buffer[1] = 0x01;
	husart1.tx_buffer[2] = 0x86;
	husart1.tx_buffer[3] = 0x00;
	husart1.tx_buffer[4] = 0x00;
	husart1.tx_buffer[5] = 0x00;
	husart1.tx_buffer[6] = 0x00;
	husart1.tx_buffer[7] = 0x00;
	husart1.tx_buffer[8] = 0x79;
	/*-------------------Заполним Tx буфер для опроса датчика углекислого газа------------------- */

	/*-------------------Проверим готовность модуля HDC1080-------------------*/
	if (CMSIS_I2C_Adress_Device_Scan(I2C1, HDC1080ADDR, 100)) {
		flag_HDC1080_ready = true;
		ERROR_LED_OFF;  //Неисправностей нет
	} else {
		flag_HDC1080_ready = false;
		ERROR_LED_ON;  //Появится сигнализация о неисправности
	}
	/*-------------------Проверим готовность модуля HDC1080-------------------*/

	Delay_ms(5);

	/*-------------------Просмотр настроек для модуля HDC1080-------------------*/
	I2C1_tx_buffer[0] = 0x02;  //Команда конфигурация
	I2C1_tx_buffer[1] = 0x10;
	I2C1_tx_buffer[2] = 0x00;
	//Скажем модулю, что хотим записать данные на адрес "Configuration" со значением 0x0000
	if (CMSIS_I2C_Data_Transmit(I2C1, HDC1080ADDR, I2C1_tx_buffer, 3, 100)) {
		ERROR_LED_OFF;  //Неисправностей нет
	} else {
		ERROR_LED_ON//Появится сигнализация о неисправности
	}
	Delay_ms(20);
	//Скажем модулю, что хотим просмотреть данные с адреса "Configuration"
	if (CMSIS_I2C_Data_Transmit(I2C1, HDC1080ADDR, I2C1_tx_buffer, 1, 100)) {
		ERROR_LED_OFF;  //Неисправностей нет
	} else {
		ERROR_LED_ON//Появится сигнализация о неисправности
	}
	Delay_ms(20);

	//Сохраним конфигурацию в переменную
	if (CMSIS_I2C_Data_Receive(I2C1, HDC1080ADDR, I2C1_rx_buffer, 2, 100)) {
		ERROR_LED_OFF;  //Неисправностей нет
	} else {
		ERROR_LED_ON//Появится сигнализация о неисправности
	}
	/*-------------------Просмотр настроек для модуля HDC1080-------------------*/

	Delay_ms(1000);

	flag_TIM3_HDC1080_start = true; //Запустим таймер для работы c HDC1080
	

	while (1) {

		/*----------------------Опрос модуля HDC1080----------------------*/
		if (flag_HDC1080_ready && flag_I2C1_send) {
			switch (queue_I2C1) {
			case 0:
				HDC1080_Transmit();
				flag_I2C1_send = false;
				break;
			case 1:
				HDC1080_Receive();
				flag_I2C1_send = false;
				break;
			}
		}
		/*----------------------Опрос модуля HDC1080----------------------*/


		/*----------------------Опрос модуля MH-Z19B----------------------*/
		if (flag_MH_Z19_ready && flag_USART1_send) {
			MH_Z19B_Check();
			flag_USART1_send = false;
		}
		/*----------------------Опрос модуля MH-Z19B----------------------*/

	   /*-------------------Если подключение к сети осуществилось-------------------*/
		if (WIFI_flag && MQTT_flag) {
			flag_TIM3_MQTT_start = true;
			/*---------------------------Отправка данных по MQTT---------------------------*/
			if (READ_BIT(flag_MQTT_Send, MQTT_SEND_TEMPERATURE)) {
				UART_MQTT_Send_data_float(0x02, Temperature, husart2.tx_buffer);
				CLEAR_BIT(flag_MQTT_Send, MQTT_SEND_TEMPERATURE);
			}
			if (READ_BIT(flag_MQTT_Send, MQTT_SEND_HUMIDITY)) {
				UART_MQTT_Send_data_float(0x03, Humidity, husart2.tx_buffer);
				CLEAR_BIT(flag_MQTT_Send, MQTT_SEND_HUMIDITY);
			}
			if (READ_BIT(flag_MQTT_Send, MQTT_SEND_CO2PPM)) {
				UART_MQTT_Send_data_uint16_t(0x04, CO2_ppm, husart2.tx_buffer);
				CLEAR_BIT(flag_MQTT_Send, MQTT_SEND_CO2PPM);
			}
			/*---------------------------Отправка данных по MQTT---------------------------*/
			flag_TIM3_ESP8266_not_connect_start = false; //Остановим таймер для ситуации, когда устройство долго не может подключиться к сети
			TIM3_ESP8266_not_connect = 0;
		} else {
			flag_TIM3_MQTT_start = false;
			TIM3_MQTT_Send = 0;
			if (flag_Reset_ESP8266) {
				Reset_ESP8266();//Перезагрузим ESP8266
				flag_Reset_ESP8266 = false;
			}
			flag_TIM3_ESP8266_not_connect_start = true; //Запустим таймер для ситуации, когда устройство долго не может подключиться к сети
		}
		/*-------------------Если подключение к сети осуществилось-------------------*/


	}

}
