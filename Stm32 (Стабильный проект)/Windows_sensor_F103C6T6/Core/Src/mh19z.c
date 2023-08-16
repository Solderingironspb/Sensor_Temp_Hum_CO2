#include "mh19z.h"

uint16_t CO2_ppm; //Уровень углекислого газа в ppm

extern struct USART_name husart1; //Объявляем структуру по USART.(см. stm32f103xx_CMSIS.h)

/*------------------------Проверка CRC-8 от MH-Z19B------------------------*/
bool CRC8_MH_Z19_Check(void) {
	uint8_t CRC_MH_Z19;
	CRC_MH_Z19 = (~(husart1.rx_buffer[1] + husart1.rx_buffer[2] + husart1.rx_buffer[3] + husart1.rx_buffer[4] + husart1.rx_buffer[5] + husart1.rx_buffer[6] + husart1.rx_buffer[7])) + 1;
	if (CRC_MH_Z19 == husart1.rx_buffer[8]) {
		return true;
	} else {
		return false;
	}
}
/*------------------------Проверка CRC-8 от MH-Z19B------------------------*/
