/**
 ******************************************************************************
 @file           : main.cpp
 @brief          : Main program body
 ******************************************************************************
 @attention

 Created on: 05 мая. 2023 г.
 Author: Oleg Volkov

 YouTube: https://www.youtube.com/channel/UCzZKTNVpcMSALU57G1THoVw
 GitHub: https://github.com/Solderingironspb/Lessons-Stm32/blob/master/README.md
 Группа ВК: https://vk.com/solderingiron.stm32

 ******************************************************************************
 */

#define WINDOWS_SENSOR                    0
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "uart_mqtt_lib.h"

#define VARIABLE_ID esp_rx_buffer[1]

#if (WINDOWS_SENSOR == 0)
#define name_mqtt_user "kitchen_windows_sensor"
#endif

#if (WINDOWS_SENSOR == 1)
#define name_mqtt_user "sleeping_windows_sensor"
#endif
/*-------------------------------Настройки Wi-fi------------------------------------------*/

#if (WINDOWS_SENSOR == 0)
uint8_t newMACAddress[] = { 0x32, 0xAE, 0x11, 0x07, 0x0D, 0x68 }; //Задаем MAC адрес устройства
#endif

#if (WINDOWS_SENSOR == 1)
uint8_t newMACAddress[] = { 0x32, 0xAE, 0x11, 0x07, 0x0D, 0x69 }; //Задаем MAC адрес устройства
#endif

const char *ssid = "*************";       // Имя Wi-fi точки доступа
const char *pass = "*************";  // Пароль от Wi-fi точки доступа

const char *mqtt_server = "192.168.0.104";  // Имя сервера MQTT
const int mqtt_port = 1883;                       // Порт для подключения к серверу MQTT
const char *mqtt_user = "************";                // Логин от сервера
const char *mqtt_pass = "************";            // Пароль от сервера

/*-------------------------------Настройки Wi-fi------------------------------------------*/

uint8_t esp_rx_buffer[20] = { 0, };  //входящий буфер
uint8_t esp_tx_buffer[20] = { 0, };  //исходящий буфер

float Temperature = 0.0f; //Температура с датчика HDC1080   Адрес: 0x02
float Humidity = 0.0f; //Влажность с датчика HDC1080        Адрес: 0x03
uint16_t CO2_ppm = 0.0f; //Концентрация углекислого газа    Адрес: 0x04

WiFiClient wclient;
PubSubClient client( wclient, mqtt_server, mqtt_port);

/* USER CODE BEGIN PFP */
void callback(const MQTT::Publish &pub);
void uart_parsing(void);
/* USER CODE END PFP */

// Функция получения данных от сервера
void callback(const MQTT::Publish &pub) {
  String payload = pub.payload_string();

  /*------------------Парсинг приходящих писем в топики-----------------------*/
#if (WINDOWS_SENSOR == 0)
  if (String(pub.topic()) == "kitchen_windows_sensor/Temperature")  //  проверяем из нужного ли нам топика пришли данные
#endif
#if (WINDOWS_SENSOR == 1)
  if (String(pub.topic()) == "sleeping_windows_sensor/Temperature")  //  проверяем из нужного ли нам топика пришли данные
#endif
  {
    Temperature = payload.toFloat();  //  преобразуем полученные данные в тип float(все типы данных, кроме float преобразуем в integer)
    UART_MQTT_Send_data_float(0x02, Temperature, esp_tx_buffer);
  }
#if (WINDOWS_SENSOR == 0)
  if (String(pub.topic()) == "kitchen_windows_sensor/Humidity")  //  проверяем из нужного ли нам топика пришли данные
#endif
#if (WINDOWS_SENSOR == 1)
  if (String(pub.topic()) == "sleeping_windows_sensor/Humidity")  //  проверяем из нужного ли нам топика пришли данные
#endif
  {
    Humidity = payload.toFloat();  //  преобразуем полученные данные в тип float(все типы данных, кроме float преобразуем в integer)
    UART_MQTT_Send_data_float(0x03, Humidity, esp_tx_buffer);
  }
#if (WINDOWS_SENSOR == 0)
  if (String(pub.topic()) == "kitchen_windows_sensor/CO2_ppm")  //  проверяем из нужного ли нам топика пришли данные
#endif

#if (WINDOWS_SENSOR == 1)
  if (String(pub.topic()) == "sleeping_windows_sensor/CO2_ppm")  //  проверяем из нужного ли нам топика пришли данные
#endif
  {
    CO2_ppm = payload.toInt();  //  преобразуем полученные данные в тип int(все типы данных, кроме float преобразуем в integer)
    UART_MQTT_Send_data_float(0x04, CO2_ppm, esp_tx_buffer);
  }
  /*------------------Парсинг приходящих писем в топики-----------------------*/
}

void uart_parsing(void) {
  if (Serial.available() > 0) {
    Serial.readBytes(esp_rx_buffer, 9);
    switch (VARIABLE_ID) {
      /*----------------здесь будет выборка по номеру переменной-----------------------*/

      case (0x02):
      if (UART_MQTT_Checksumm_validation(esp_rx_buffer)) {
        Temperature = UART_MQTT_Receive_data_float(esp_rx_buffer);
#if (WINDOWS_SENSOR == 0)
        client.publish("kitchen_windows_sensor/Temperature", String(Temperature));
#endif

#if (WINDOWS_SENSOR == 1)
        client.publish("sleeping_windows_sensor/Temperature", String(Temperature));
#endif
      }
      break;
      case (0x03):
      if (UART_MQTT_Checksumm_validation(esp_rx_buffer)) {
        Humidity = UART_MQTT_Receive_data_float(esp_rx_buffer);
#if (WINDOWS_SENSOR == 0)
        client.publish("kitchen_windows_sensor/Humidity", String(Humidity));
#endif

#if (WINDOWS_SENSOR == 1)
        client.publish("sleeping_windows_sensor/Humidity", String(Humidity));
#endif
      }
      break;
      case (0x04):
      if (UART_MQTT_Checksumm_validation(esp_rx_buffer)) {
        CO2_ppm = UART_MQTT_Receive_data_int16_t(esp_rx_buffer);
#if (WINDOWS_SENSOR == 0)
        client.publish("kitchen_windows_sensor/CO2_ppm", String(CO2_ppm));
#endif

#if (WINDOWS_SENSOR == 1)
        client.publish("sleeping_windows_sensor/CO2_ppm", String(CO2_ppm));
#endif
      }
      break;

      /*----------------здесь будет выборка по номеру переменной-----------------------*/
    }
    memset(esp_rx_buffer, 0, 9);  //очистка входящего буффера
  }
}

void setup(void) {

  Serial.begin(9600);
  delay(100);
  Serial.setTimeout(5);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  wifi_set_macaddr(STATION_IF, &newMACAddress[0]);
  WiFi.hostname(name_mqtt_user);
  WiFi.begin(ssid, pass);

}

void loop(void) {
  if (client.connected()) {
    client.loop();
    uart_parsing();
    delay(1);
  } else {
    if (WiFi.status() != WL_CONNECTED)  //Проверяем статус подключения к Wi-fi точке
        {
      wifi_not_ok();  //отсылаем служебную команду Wifi != OK
      delay(10);

      WiFi.begin(ssid, pass); //Вводим имя точки доступа и пароль

      if (WiFi.waitForConnectResult() != WL_CONNECTED)
        return;
      wifi_ok();  //отсылаем служебную команду Wifi = OK
      delay(10);
    }

    if (WiFi.status() == WL_CONNECTED)  // Если подключились к Wi-fi точке
        {
      if (!client.connected())  //Проверяем статус подключения к MQTT серверу
      {
        mqtt_not_ok();
        delay(10);
        if (client.connect(MQTT::Connect(name_mqtt_user).set_auth(mqtt_user, mqtt_pass)))  //Если подключились к MQTT серверу, то авторизуемся
        {
          mqtt_ok();  //отсылаем служебную команду MQTT = OK
          delay(10);
          client.set_callback(callback);

          /*--------------------Указываем топики, на которые хотим подписаться-------------------------*/
#if (WINDOWS_SENSOR == 0)
          client.subscribe("kitchen_windows_sensor/Temperature");
          client.subscribe("kitchen_windows_sensor/Humidity");
          client.subscribe("kitchen_windows_sensor/CO2_ppm");
#endif

#if (WINDOWS_SENSOR == 1)
          client.subscribe("sleeping_windows_sensor/Temperature");
          client.subscribe("sleeping_windows_sensor/Humidity");
          client.subscribe("sleeping_windows_sensor/CO2_ppm");
#endif
          /*--------------------Указываем топики, на которые хотим подписаться-------------------------*/
        } else {
          mqtt_not_ok();  //отсылаем служебную команду MQTT = !OK
          delay(10);

        }
      }

    }
    delay(1000);
  }
}
/************************** (C) COPYRIGHT Soldering iron *******END OF FILE****/
