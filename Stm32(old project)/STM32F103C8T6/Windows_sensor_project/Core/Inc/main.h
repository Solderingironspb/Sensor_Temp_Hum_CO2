#include <stm32f1xx.h>
#include <stm32f103xx_CMSIS.h>
#include <stdbool.h>

/*---------------Распиновка---------------*/
//PA2 USART2 Tx
//PA3 USART2 Rx
//PA4 Wifi LED
//PA5 USART1 LED
//PB6 I2C1 LED
//PB7 Error LED
//PB8 ESP8266 Reset
//PA9 USART1 Tx
//PA10 USART1 Rx
//PB6 SCL (I2C clock) Alternate function open drain
//PB7 SDA (I2C Data I/O) Alternate function open drain

/*---------------Распиновка---------------*/

/*----------------------Макросы----------------------*/
#define HDC1080ADDR 0x40
#define Wifi_LED_ON GPIOA->BSRR = GPIO_BSRR_BS4; //Wifi LED On
#define Wifi_LED_OFF GPIOA->BSRR = GPIO_BSRR_BR4; //Wifi LED Off
#define USART1_LED_ON GPIOA->BSRR = GPIO_BSRR_BS5; //USART1 LED On
#define USART1_LED_OFF GPIOA->BSRR = GPIO_BSRR_BR5; //USART1 LED Off
#define I2C1_LED_ON GPIOA->BSRR = GPIO_BSRR_BS6; //I2C1 LED On
#define I2C1_LED_OFF GPIOA->BSRR = GPIO_BSRR_BR6; //I2C1 LED Off
#define ERROR_LED_ON GPIOA->BSRR = GPIO_BSRR_BS7; //Error LED On
#define ERROR_LED_OFF GPIOA->BSRR = GPIO_BSRR_BR7; //Error LED Off
#define ESP8266_RESET_ON GPIOA->BSRR = GPIO_BSRR_BS8; //Нажать кнопку ресет у ESP8266
#define ESP8266_RESET_OFF GPIOA->BSRR = GPIO_BSRR_BR8; //Отпустить кнопку ресет у ESP8266
#define MQTT_SEND_TEMPERATURE (1 << 0)
#define MQTT_SEND_HUMIDITY    (1 << 1)
#define MQTT_SEND_CO2PPM      (1 << 2)
/*----------------------Макросы----------------------*/

/*------------------------Прототипы функций------------------------*/
void GPIO_Init(void); //Функция настроек ножек GPIO
void UART_MQTT_Send_data_uint16_t(uint8_t VARIABLE_ID, uint16_t data, uint8_t *tx_buffer); //Отправка данных типа uint16_t на ESP8266 по UART2
void UART_MQTT_Send_data_float(uint8_t VARIABLE_ID, float data, uint8_t *tx_buffer); //Отправка данных типа float на ESP8266 по UART2
bool UART_MQTT_Checksumm_validation(uint8_t* rx_buffer); //Проверка контрольной суммы входящего сообщения
void UART_MQTT_Check_net(void); //Проверка подключения ESP8266 к Wifi и MQTT брокеру
bool HDC1080_Transmit(void); //Считывание данных с HDC1080
bool HDC1080_Receive(void); //Считывание данных с HDC1080
void MH_Z19B_Check(void); //Функция запроса датчику MH-Z19B
void Reset_ESP8266(void); //Функция перезагрузки ESP8266
bool CRC8_MH_Z19_Check(void); //Проверка CRC-8 от MH-Z19B