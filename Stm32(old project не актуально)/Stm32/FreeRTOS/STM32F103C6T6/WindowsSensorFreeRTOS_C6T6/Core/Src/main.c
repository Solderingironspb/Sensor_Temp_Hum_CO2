#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

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

bool start_os = false;

QueueHandle_t xQueue1;
struct USART_rx_data usart1_RX;

int main(void) {
	CMSIS_Debug_init(); //Настройка дебага
	CMSIS_RCC_SystemClock_72MHz(); //Настроим МК на 72 МГц
	CMSIS_SysTick_Timer_init(); //Настроим системный таймер
	CMSIS_TIM3_init(); //Таймер для индикации подключения к сети.
	GPIO_init(); //Настроим GPIO
	CMSIS_USART1_Init(); //общение с модулем MH19Z (9600 8N1)
	CMSIS_USART2_Init(); //общение с модулем ESP8266 (9600 8N1)
	CMSIS_I2C1_Init(); //общение с модулем HDC1080 (standart mode)
	HDC1080_init(); //Проинициализируем модуль HDC1080

#if (USE_DEBUG == 1)
	CMSIS_GPIO_init(GPIOC, 13, GPIO_GENERAL_PURPOSE_OUTPUT, GPIO_OUTPUT_PUSH_PULL, GPIO_SPEED_50_MHZ);
#endif

	/*Создадим задачи*/
	xTaskCreate((void*)vTask1_MH19Z_Request, "Task1", 30, NULL, 0, NULL);
	xTaskCreate((void*)vTask2_HDC1080_Request, "Task2", 30, NULL, 0, NULL);
	xTaskCreate((void*)vTask3_MQTT_Send, "Task3", 30, NULL, 0, NULL);

	/*Создадим очереди*/
	xQueue1 = xQueueCreate(3, sizeof(usart1_RX)); //Создадим очередь на 5 элементов

	start_os = 1; //Мы готовы к запуску FreeRTOS
	vTaskStartScheduler();

	for (;;) {
		/*Если провалимся сюда - ошибка FreeRTOS. Возможно StackOverflow*/
	}
}
