#include "main.h"
#include "stm32f103xx_CMSIS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "HDC1080.h"

extern QueueHandle_t xQueue1;
struct USART_rx_data usart1_rx_data;
extern uint16_t CO2_ppm; //Уровень углекислого газа в ppm
extern uint8_t I2C1_tx_buffer[4];  //Исходящий буфер
extern uint8_t I2C1_rx_buffer[10];  //Входящий буфер HDC1080
extern float Temperature, Humidity;
bool MH19Z_data_ready = false; //Разрешающий флаг на отправку полученных данных
bool HDC1080_data_ready = false; //Разрешающий флаг на отправку полученных данных
extern struct USART_name husart2; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)
extern uint8_t NETWORK_Status; //Cтатус подключения к сети

/**
 ***************************************************************************************
 *  @breif Отдать запрос датчику и обработать входящие данные MH19Z
 ***************************************************************************************
 */
void vTask1_MH19Z_Request(void *pvParameters) {
	BaseType_t StatusReceieve;
	uint8_t tx_buffer[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 }; //Буфер запроса данных
	for (;;) {
		CMSIS_USART_Transmit(USART1, tx_buffer, sizeof(tx_buffer), 100); //Отправим запрос датчику CO2
		/*Мигнем светодиодом*/
		USART1_LED_ON();
		vTaskDelay(100);
		USART1_LED_OFF();
		/*Проверим очередь, если что - обработаем входящие данные*/
		if (xQueue1 != NULL) {
			StatusReceieve = xQueueReceive(xQueue1, &usart1_rx_data, 0);
			if (StatusReceieve == pdPASS) {
				if (CRC8_MH_Z19_Check()) {
					//Рассчитаем количество CO2 в ppm
					CO2_ppm = usart1_rx_data.rx_buffer[2] * 256 + usart1_rx_data.rx_buffer[3];
					MH19Z_data_ready = true;
				}
			}
		}
		vTaskDelay(4900);
	}
}

/**
 ***************************************************************************************
 *  @breif Отдать запрос датчику и обработать входящие данные MH19Z
 ***************************************************************************************
 */
void vTask2_HDC1080_Request(void *pvParameters) {
	for (;;) {
		//Команда опроса температуры и влажности
		I2C1_tx_buffer[0] = 0;
		I2C1_tx_buffer[1] = 0;
		if (CMSIS_I2C_Data_Transmit(I2C1, HDC1080ADDR, I2C1_tx_buffer, 1, 100)) {
			I2C1_LED_ON(); //Покажем активность шины I2C1
			vTaskDelay(100);
			I2C1_LED_OFF();
		} else {
			ERROR_LED_ON();
			vTaskDelay(100);
			ERROR_LED_OFF();
			vTaskDelay(100);
		}

		//Cчитаем данные
		if (CMSIS_I2C_Data_Receive(I2C1, HDC1080ADDR, I2C1_rx_buffer, 4, 100)) {
			Temperature = (((float) ((uint16_t) I2C1_rx_buffer[0] << 8 | I2C1_rx_buffer[1]) * 165) / 65536) - 40;
			Temperature = Temperature - 1;
			Humidity = ((float) ((uint16_t) I2C1_rx_buffer[2] << 8 | I2C1_rx_buffer[3]) * 100) / 65536;
			Humidity = Humidity + 5;
			HDC1080_data_ready = true;
		} else {
			ERROR_LED_ON();
			vTaskDelay(100);
			ERROR_LED_OFF();
			vTaskDelay(100);
		}
		vTaskDelay(4900);
	}
}

/**
 ***************************************************************************************
 *  @breif Отправка данных по MQTT на сервер
 ***************************************************************************************
 */
void vTask3_MQTT_Send(void *pvParameters) {
	for (;;) {
		if (NETWORK_Status == MQTT_STATUS_CONNECTED) {
			if (MH19Z_data_ready) {
				UART_MQTT_Send_data_uint16_t(MQTT_ID_CO2PPM, CO2_ppm, husart2.tx_buffer);
				MH19Z_data_ready = false;

			}
			if (HDC1080_data_ready) {
				UART_MQTT_Send_data_float(MQTT_ID_TEMPERATURE, Temperature, husart2.tx_buffer);
				UART_MQTT_Send_data_float(MQTT_ID_HUMIDITY, Humidity, husart2.tx_buffer);
				HDC1080_data_ready = false;
			}
		}
		vTaskDelay(100);
	}
}

/**
 ***************************************************************************************
 *  @breif vApplicationIdleHook
 *  Задача бездействия
 *  Задача бездействия создается автоматически при запуске планировщика RTOS,
 *  чтобы гарантировать, что всегда есть хотя бы одна задача, которая может быть запущена.
 *  Она создается с самым низким возможным приоритетом , чтобы гарантировать, что он
 *  не использует процессорное время, если есть задачи приложения
 *  с более высоким приоритетом в состоянии готовности.
 ***************************************************************************************
 */
#if (USE_DEBUG == 1)
uint32_t HeapSize; //Свободное место в куче
#endif

#if (configUSE_IDLE_HOOK==1)
void vApplicationIdleHook(void) {

	/*Проверка свободного места в куче, выделенной под FreeRTOS*/
#if (USE_DEBUG == 1)
	GPIOC->BSRR = GPIO_BSRR_BS13;
	GPIOC->BSRR = GPIO_BSRR_BR13;
	HeapSize = xPortGetFreeHeapSize();
#endif

}
#endif

/**
 ***************************************************************************************
 *  @breif vApplicationGetIdleTaskMemory
 *  Если configSUPPORT_STATIC_ALLOCATION == 1, то объекты RTOS могут создаваться
 *  с использованием ОЗУ, предоставленного автором приложения.
 *  Если для configSUPPORT_STATIC_ALLOCATION установлено значение 1, то автор приложения
 *  должен также предоставить две функции обратного вызова:
 *  vApplicationGetIdleTaskMemory(), чтобы предоставить память для использования
 *  задачей бездействия RTOS, и (если для configUSE_TIMERS установлено значение 1)
 *  vApplicationGetTimerTaskMemory(), чтобы предоставить память
 *  для использования задача службы демона/таймера RTOS
 ***************************************************************************************
 */
#if (configSUPPORT_STATIC_ALLOCATION == 1)
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	/* If the buffers to be provided to the Idle task are declared inside this
	 function then they must be declared static - otherwise they will be allocated on
	 the stack and so not exists after this function exits. */
	static StaticTask_t xIdleTaskTCB;
	static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	 state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	 Note that, as the array is necessarily of type StackType_t,
	 configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
#endif
