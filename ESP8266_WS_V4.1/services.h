#ifndef SERVICES_H
#define SERVICES_H

#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include "webserver.h"

WiFiClient wifiClient;
PubSubClient mqttAPI(wifiClient);

String httpCodeStr(int code) {
  switch(code) {
    case -1:  return "CONNECTION REFUSED";
    case -2:  return "SEND HEADER FAILED";
    case -3:  return "SEND PAYLOAD FAILED";
    case -4:  return "NOT CONNECTED";
    case -5:  return "CONNECTION LOST";
    case -6:  return "NO STREAM";
    case -7:  return "NO HTTP SERVER";
    case -8:  return "TOO LESS RAM";
    case -9:  return "ENCODING";
    case -10: return "STREAM WRITE";
    case -11: return "READ TIMEOUT";
     default: return  http.codeTranslate(code);
  }
}

String mqttCodeStr(int code) {
  switch (code) {
    case -4: return "CONNECTION TIMEOUT";
    case -3: return "CONNECTION LOST";
    case -2: return "CONNECT FAILED";
    case -1: return "MQTT DISCONNECTED";
    case  0: return "CONNECTED";
    case  1: return "CONNECT BAD PROTOCOL";
    case  2: return "CONNECT BAD CLIENT ID";
    case  3: return "CONNECT UNAVAILABLE";
    case  4: return "CONNECT BAD CREDENTIALS";
    case  5: return "CONNECT UNAUTHORIZED";
    default: return String(code);
  }
}

bool mqttPublish(String topic, String data) {
  yield();
  if (conf.param("mqtt_path").length()) topic = conf.param("mqtt_path") + "/" + topic;
  return mqttAPI.publish(topic.c_str(), data.c_str(), true);
}
bool mqttPublish(String topic, float data) { return mqttPublish(topic, String(data)); }
bool mqttPublish(String topic, int32_t data) { return mqttPublish(topic, String(data)); }
bool mqttPublish(String topic, uint32_t data) { return mqttPublish(topic, String(data)); }

void restAPIsend(String host, uint16_t port, String query) {
  HTTPClient restAPI;
  restAPI.setUserAgent("weather station (www.it4it.club) " + WiFi.hostname());
  restAPI.setTimeout(3000);
  restAPI.begin(host, port, query);
  int code = restAPI.GET();
  #ifdef console
    console.printf("answer: %s\n", httpCodeStr(code).c_str());
  #endif
  restAPI.end();
  yield();
}

/* mqtt.it4it.club */
void sendDataToMQTT() {
  //char outstr[15];
  float f0,f1;
  //String str1,str2;
  if (wifi.transferDataPossible() and conf.param("mqtt_server").length()) {
    String server = conf.param("mqtt_server");
    #ifdef console
      console.print(F("services: send data to MQTT server :"));
      console.println(server);
    #endif
    // баг при прямой передаче значения (c_str) из конфига в setServer (не забыть поправить!)
    mqttAPI.setServer(server.c_str(), 1883);
    mqttAPI.connect(WiFi.hostname().c_str(),
      (conf.param("mqtt_login").length() ? conf.param("mqtt_login").c_str() : 0),
      (conf.param("mqtt_pass").length() ? conf.param("mqtt_pass").c_str() : 0) );
    if (mqttAPI.connected()) {
      //mqttPublish("light",  sensors.get("out_light"));
      //if (sensors.get("out_temperature")) 
      mqttPublish("out_temp", sensors.get("out_temperature"));
      //if (sensors.get("out_humidity"  ))  
      mqttPublish("out_hum",  sensors.get("out_humidity"));
      //if (sensors.get("out_pressure"  )) 
      mqttPublish("out_press",sensors.get("out_pressure"));
      //if (sensors.get("in_temperature"))
      mqttPublish("in_temp",  sensors.get("in_temperature"));
      //if (sensors.get("in_humidity"  )) 
      mqttPublish("in_hum",   sensors.get("in_humidity"));
      //if (sensors.get("in_pressure"  )) 
      mqttPublish("in_press", sensors.get("in_pressure"));
      f0 = sensors.get("ds18b20_s0");   
      f1 = sensors.get("ds18b20_s1");   
      //if (f0)
      mqttPublish("ds0",f0); 
      //if (f1) 
      mqttPublish("ds1",f1);
      //if (sensors.get("ds18b20_s1"   ))   mqttPublish("ds1",      sensors.get("ds18b20_s1"));
      String strT =  String(f0,1) + ":" + String(f1,1);
      if (strT) mqttPublish("ds2", strT);
      //if (sensors.get("out_co2"))  mqttPublish("co2",    sensors.get("out_co2"));
      #ifdef console
        console.println(F("answer: OK"));
      #endif
      mqttAPI.disconnect();
    } else {
      #ifdef console
        console.printf("answer: %s\n", mqttCodeStr(mqttAPI.state()).c_str());
      #endif
    }
  }
}

/* https://thingspeak.com/ */
void sendDataToThingSpeak() {
  if (wifi.transferDataPossible() and conf.param("thingspeak_key").length()) {
    #ifdef console
      console.println(F("services: send data to ThingSpeak"));
    #endif
    String query;
    //query += "&field1=" + String(sensors.get("out_light"));
    query += "&field1=" + String(sensors.get("out_temperature"));
    query += "&field2=" + String(sensors.get("out_humidity"));
    query += "&field3=" + String(sensors.get("out_pressure"));
    query += "&field4=" + String(sensors.get("in_temperature"));
    query += "&field5=" + String(sensors.get("in_humidity"));
    query += "&field6=" + String(sensors.get("in_pressure"));
    query += "&field7=" + String(sensors.get("ds18b20_s0"));
    query += "&field8=" + String(sensors.get("ds18b20_s1"));
    //query += "&field5=" + String(sensors.get("out_co2"));
    restAPIsend("api.thingspeak.com", 80, "/update?api_key=" + conf.param("thingspeak_key") + query);
  }
}
/* https://narodmon.ru/ */
void sendDataToNarodmon() {
  if (wifi.transferDataPossible() and conf.param("narodmon_id").length()) {
    #ifdef console
      console.println(F("services: send data to Narodmon"));
    #endif
    String query;
    query += "&L1="  + String(sensors.get("out_light"));
    query += "&T1="  + String(sensors.get("out_temperature"));
    query += "&H1="  + String(sensors.get("out_humidity"));
    query += "&P1="  + String(sensors.get("out_pressure"));
    //query += "&CO2=" + String(sensors.get("out_co2"));
    //query += "&H2="  + String(sensors.get("out_absoluteHumidity"));
    restAPIsend("narodmon.ru", 80, "/get?id=" + conf.param("narodmon_id") + query);
  }
}
#endif
