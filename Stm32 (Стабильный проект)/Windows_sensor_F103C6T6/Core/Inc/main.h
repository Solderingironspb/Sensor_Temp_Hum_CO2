#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f1xx.h>
#include <stdbool.h>
#include "stm32f103xx_CMSIS.h"
#include "stm32f103xx_it.h"
#include "SoftwareTimer.h"
#include "gpio.h"
#include "mh19z.h"
#include "HDC1080.h"
#include "MQTT.h"

enum {
	WIFI_STATUS_NOT_CONNECTED,
	WIFI_STATUS_CONNECTED,
	MQTT_STATUS_NOT_CONNECTED,
	MQTT_STATUS_CONNECTED
};




#ifdef __cplusplus

}
#endif

#endif /* __MAIN_H */


