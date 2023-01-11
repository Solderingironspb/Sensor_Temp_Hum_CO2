/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.cpp
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 *  Created on: 25 дек. 2020 г.
 *      Author: Oleg Volkov
 *
 *  YouTube: https://www.youtube.com/channel/UCzZKTNVpcMSALU57G1THoVw
 *  GitHub: https://github.com/Solderingironspb/Lessons-Stm32/blob/master/README.md
 *  Группа ВК: https://vk.com/solderingiron.stm32
 * 
 ******************************************************************************
 */
/* USER CODE END Header */
/* USER CODE BEGIN Includes */
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "uart_mqtt_lib.h"

#define VARIABLE_ID esp_rx_buffer[1]
#define name_mqtt_user "Windows_sensor_1"
/*-------------------------------Настройки Wi-fi------------------------------------------*/

const char *ssid = "Solderingiron";       // Имя Wi-fi точки доступа
const char *pass = "Service5H4e3l2l1O";  // Пароль от Wi-fi точки доступа

const char *mqtt_server = "192.168.0.104";  // Имя сервера MQTT
const int mqtt_port = 1883;                       // Порт для подключения к серверу MQTT
const char *mqtt_user = "dcdjpkvp";                // Логин от сервера
const char *mqtt_pass = "d0wyfc9x25Ce";            // Пароль от сервера

/*-------------------------------Настройки Wi-fi------------------------------------------*/

uint8_t esp_rx_buffer[9] = {
    0,
};  //входящий буфер
uint8_t esp_tx_buffer[9] = {
    0,
};  //исходящий буфер

float Temperature = 0.0f; //Температура с датчика HDC1080   Адрес: 0x02
float Humidity = 0.0f; //Влажность с датчика HDC1080        Адрес: 0x03
uint16_t CO2_ppm = 0.0f; //Концентрация углекислого газа    Адрес: 0x04


/* USER CODE END Includes */

WiFiClient wclient;
PubSubClient client(wclient, mqtt_server, mqtt_port);

/* USER CODE BEGIN PFP */
void callback(const MQTT::Publish &pub);
void uart_parsing(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void callback(const MQTT::Publish &pub)  // Функция получения данных от сервера
{
  String payload = pub.payload_string();

  /*------------------Парсинг приходящих писем в топики-----------------------*/

 
  if (String(pub.topic()) == "Windows_sensor_1/Temperature")  //  проверяем из нужного ли нам топика пришли данные
  {
    Temperature = payload.toFloat();  //  преобразуем полученные данные в тип float(все типы данных, кроме float преобразуем в integer)
    UART_MQTT_Send_data_float(0x02, Temperature, esp_tx_buffer);
  }
  if (String(pub.topic()) == "Windows_sensor_1/Humidity")  //  проверяем из нужного ли нам топика пришли данные
  {
    Humidity = payload.toFloat();  //  преобразуем полученные данные в тип float(все типы данных, кроме float преобразуем в integer)
    UART_MQTT_Send_data_float(0x03, Humidity, esp_tx_buffer);
  }
  if (String(pub.topic()) == "Windows_sensor_1/CO2_ppm")  //  проверяем из нужного ли нам топика пришли данные
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
          client.publish("Windows_sensor_1/Temperature", String(Temperature));
        }
        break;
      case (0x03):
        if (UART_MQTT_Checksumm_validation(esp_rx_buffer)) {
          Humidity = UART_MQTT_Receive_data_float(esp_rx_buffer);
          client.publish("Windows_sensor_1/Humidity", String(Humidity));
        }
        break;
       case (0x04):
        if (UART_MQTT_Checksumm_validation(esp_rx_buffer)) {
          CO2_ppm = UART_MQTT_Receive_data_int16_t(esp_rx_buffer);
          client.publish("Windows_sensor_1/CO2_ppm", String(CO2_ppm));
        }
        break;
        

        /*----------------здесь будет выборка по номеру переменной-----------------------*/
    }
    memset(esp_rx_buffer, 0, 9);  //очистка входящего буффера
  }
}

/* USER CODE END 0 */

void setup(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Initialize all configured peripherals */
  Serial.begin(9600);
  delay(100);
  Serial.setTimeout(5);

  /* USER CODE BEGIN 2 */

}
/* USER CODE END 2 */

/* Infinite loop */
/* USER CODE BEGIN WHILE */
void loop(void)
/* USER CODE END WHILE */
/* USER CODE BEGIN 3 */
{
  if (WiFi.status() != WL_CONNECTED)  //Проверяем статус подключения к Wi-fi точке
  {
    wifi_not_ok();  //отсылаем служебную команду Wifi != OK
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    wifi_ok();  //отсылаем служебную команду Wifi = OK
  }

  if (WiFi.status() == WL_CONNECTED)  // Если подключились к Wi-fi точке
  {
    if (!client.connected())  //Проверяем статус подключения к MQTT серверу
    {
      mqtt_not_ok();                                                                     //отсылаем служебную команду MQTT = !OK
      if (client.connect(MQTT::Connect(name_mqtt_user).set_auth(mqtt_user, mqtt_pass)))  //Если подключились к MQTT серверу, то авторизуемся
      {
        mqtt_ok();  //отсылаем служебную команду MQTT = OK
        client.set_callback(callback);

        /*--------------------Указываем топики, на которые хотим подписаться-------------------------*/
        client.subscribe("Windows_sensor_1/Temperature");
        client.subscribe("Windows_sensor_1/Humidity");
        client.subscribe("Windows_sensor_1/CO2_ppm");
        /*--------------------Указываем топики, на которые хотим подписаться-------------------------*/
      } else {
        mqtt_not_ok();  //отсылаем служебную команду MQTT = !OK
      }
    }

    if (client.connected()) {
      client.loop();
      uart_parsing();
    }
  }
}
/* USER CODE END 3 */
/************************** (C) COPYRIGHT Soldering iron *******END OF FILE****/
