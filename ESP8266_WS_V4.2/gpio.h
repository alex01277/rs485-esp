#ifndef GPIO_H
#define GPIO_H

/* Выбираем через какой логический уровень будем управлять нагрузкой */
bool gpio_enable = LOW; // HIGH

/*
   Управление нагрузкой при превышении определенных показаний:
    GPIO 12 (D6) - превышение установленной температуры
    GPIO 13 (D7) - превышение установленной влажности
*/
void gpio_12_13() {/* Все будет работать если определен датчик температуры и влажности */
  cron.add(cron::time_1d, [&]() { /* Контроль отправки СМС */
    if (wifi.transferDataPossible()) {
#ifdef console
      console.println("testing SMS_gate function");
#endif
      float temperature0 = sensors.get("ds18b20_s0");
      float temperature1 = sensors.get("out_temperature");  // float temperature1 = sensors.get("ds18b20_s1");
      int t0_max = conf.param("gpio12").toInt();
      int t1_max = conf.param("gpio13").toInt();
      String sMsg = "Test:" + String(temperature0) + ":" + String(temperature1);
#ifdef console
      console.print(sMsg); console.print(conf.param("sms_gate01"));
      console.print(t0_max); console.print(t1_max);
#endif
      sendSMS("294001234", sMsg);
    }
  });
  cron.add(cron::time_30m, [&]() { /* Контроль превышения температуры */
    if (wifi.transferDataPossible() and conf.param("sms_gate01").length()) {
#ifdef console
      console.println("testing temperature indication");
#endif
      float temperature0 = sensors.get("ds18b20_s0");
      float temperature1 = sensors.get("out_temperature");  // float temperature1 = sensors.get("ds18b20_s1");
      int t0_max = conf.param("gpio12").toInt();
      int t1_max = conf.param("gpio13").toInt();
      if (temperature0 > t0_max  || temperature1 > t1_max)  {
        String sMsg = String(temperature0) + ":" + String(temperature1);
#ifdef console
        console.print(sMsg); console.print(conf.param("sms_gate01"));
        console.print(t0_max); console.print(t1_max);
#endif
        if (conf.param("sms_gate01")) sendSMS(conf.param("sms_gate01"), sMsg);
        //  delay(5000);
        //  if (conf.param("sms_gate02")) sendSMS(conf.param("sms_gate02"),sMsg);
      }
    }
  });
}
/*
   Управление нагрузкой при расхождении расчетной абсолютной влажности между двух датчиков
    GPIO 14 (D5) - не регулируется через WEB интерфейс т.к регулировать по сути нечего, есть только гистерезис
*/
void gpio_14() {
  /* Необходимо наличие данных о температуре и влажности внутри и снаружи помещения */
  if (sensors.find("out_temperature") and sensors.find("out_humidity") and sensors.find("in_temperature") and sensors.find("in_humidity")) {
    pinMode(14, OUTPUT); digitalWrite(14, !gpio_enable);
    /* Добавление задачи в планировщик */
    cron.add(cron::time_5s, [&]() {
      if (sensors.status("out_temperature") and sensors.status("in_temperature") and sensors.status("out_humidity") and sensors.status("in_humidity")) {
        int out_hum = (int)absoluteHumidity(sensors.get("out_temperature"), sensors.get("out_humidity"));
        int in_hum  = (int)absoluteHumidity(sensors.get("in_temperature"), sensors.get("in_humidity"));
        /* Разница в показаниях должна быть больше 2 грамм на кубический сантиметр */
        if ((in_hum - out_hum) > 2 and digitalRead(14) != gpio_enable) digitalWrite(14, gpio_enable);
        else if (in_hum <= out_hum and digitalRead(14) == gpio_enable) digitalWrite(14, !gpio_enable);
      } else if (digitalRead(14) == gpio_enable) digitalWrite(14, !gpio_enable);
    });
  }
}

#endif
