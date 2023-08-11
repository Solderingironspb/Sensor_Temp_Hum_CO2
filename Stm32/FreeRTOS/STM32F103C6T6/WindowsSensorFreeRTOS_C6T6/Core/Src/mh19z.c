#include "mh19z.h"

uint16_t CO2_ppm; //Уровень углекислого газа в ppm

extern struct USART_rx_data usart1_rx_data;

/*------------------------Проверка CRC-8 от MH-Z19B------------------------*/
bool CRC8_MH_Z19_Check(void) {
	uint8_t CRC_MH_Z19;
	CRC_MH_Z19 = (~(usart1_rx_data.rx_buffer[1] + usart1_rx_data.rx_buffer[2] + usart1_rx_data.rx_buffer[3] + usart1_rx_data.rx_buffer[4] + usart1_rx_data.rx_buffer[5] + usart1_rx_data.rx_buffer[6] + usart1_rx_data.rx_buffer[7])) + 1;
	if (CRC_MH_Z19 == usart1_rx_data.rx_buffer[8]) {
		return true;
	} else {
		return false;
	}
}
/*------------------------Проверка CRC-8 от MH-Z19B------------------------*/
