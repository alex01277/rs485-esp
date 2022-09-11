/*
   Метеостанция на ESP8266 от it4it.club (v2.0 Beta)
   Тема проекта: https://it4it.club/topic/55-meteostantsiya-na-esp8266-ot-it4itclub/
выкл мигание диода
   Задействованные пины
   ---------------------------------------
   NodeMCU Pins | ESP 07/12 Pins | Датчики
   ---------------------------------------
        D2      |      GPIO 4    |   SDA
        D1      |      GPIO 5    |   SCL
        3.3V    |      VCC       |   VCC
        GND     |      GND       |   GND
*/

/* Консоль */
//#define console Serial    // Обязательно закомментируйте эту строку перед финальной загрузкой программы
#define consoleSpeed 115200

/* Библиотеки которые необходимо обязательно скачать */
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>  // https://github.com/bblanchon/ArduinoJson
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient

/* Файлы проекта (последовательность загрузки имеет значение) */
#include "config.h"       // Описание системы работающей с фалом конфигурации 
#include "tools.h"        // Вспомогательные утилиты
#include "cron.h"         // Планировщик задач
#include "wifi.h"         // Обслуживание режимов работы беспроводной сети
#include "sensors.h"      // Обслуживание датчиков
#include "webserver.h"    // http сервер
#include "services.h"     // Описание взаимодействия с внешними сервисами
#include "gpio.h"         // Обслуживание GPIO
WiFiServer srv81 (8081);
//#include "users_auto.h";  // Пользовательская конфигурация датчиков, именно тут описывается с какими датчиками работать
#include "users_bme280_x2.h"; // Пример для двух датчиков BME280 (несовместим с users_auto.h)

void setup() {
  /* Инициализация терминала */
#ifdef console
  console.begin(consoleSpeed);
  console.println();
#endif

  /* ВАЖНО!: Инициализация параметров конфига (НЕ требует внесения изменений, ВСЕ правится через web интерфейс) */
  conf.add("client_ssid");
  conf.add("client_pass");
  conf.add("client_bmac");
  conf.add("mdns_hostname", "espws");
  conf.add("softap_ssid",   "WeatherStation");
  conf.add("softap_pass");
  conf.add("admin_login",   "admin");
  conf.add("admin_pass",    "admin");
  conf.add("sms_gate01",    "294401401");
  conf.add("sms_gate02",    "294402402");
  conf.add("mqtt_server");
  conf.add("mqtt_login");
  conf.add("mqtt_pass");
  conf.add("mqtt_path");
  conf.add("thingspeak_key");
  conf.add("narodmon_id");
  
  conf.add("gpio12", "35"); // превышение температуры
  conf.add("gpio13", "75"); // превышение влажности
  
  conf.read();
  conf.print();
  
  /* Инициализация датчиков */
  sensors_config();
  
  /* Инициализация GPIO для управления внешней нагрузкой */
  gpio_12_13(); // Простое превышение температуры или влажности (выставляется в WEB интерфейсе)
  //gpio_14();    // Расхождение расчетной абсолютной влажности между показаниями с двух датчиков, например, BME280
  
  /* Добавление в планировщик заданий по отправке данных на внешнии ресурсы */
  cron.add(cron::time_1m, sendDataToMQTT);       // Отправка данных MQTT брокеру
  cron.add(cron::time_5m, sendDataToThingSpeak); // Отправка данных на сервер "ThingSpeak"
  cron.add(cron::time_5m + cron::minute, sendDataToNarodmon); // Отправка данных на севрер "Народный мониторинг"

  /* Добавление в планировщик заданий по контролю датчиков (холодный старт) */
  cron.add(cron::time_1m,  [&](){ sensors.checkLine();  }, true); // Проверка шины и инициализация датчиков при необходимости
  cron.add(cron::time_10s, [&](){ sensors.dataUpdate(); }, true); // Сбор данных с датчиков
  /* Добавление в планировщик задания (горячий старт) */
  cron.add(cron::time_10m, [&](){ sensors.logUpdate();  }, "httpSensorsLog"); // Обновление журнала (httpSensorsLog - не обязательный уникальный ID для быстрого поиска задания другими программными модулями)
  srv81.begin();
}

void loop() {
  /* Обработчики */
  wifi.handleEvents();
  http.handleClient();
  cron.handleEvents();
  WiFiClient client81 = srv81.available();  // send reqest to the client from port 8081
  if (!client81) {  return; }
          else {  String request = readRequest(&client81);
                      executeRequest(&client81, &request);      }
}
