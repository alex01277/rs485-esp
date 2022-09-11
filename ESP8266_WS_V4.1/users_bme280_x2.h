#ifndef USERS_H
#define USERS_H
#define ON  true
#define OFF false
#define SENSOR_BH1750 ON // https://github.com/claws/BH1750

#if SENSOR_BH1750
  #include <BH1750.h>
  BH1750 BH1750;
#endif
  
#include <BME280I2C.h> // https://github.com/finitespace/BME280
#include <OneWire.h>
#include <DallasTemperature.h> 

//DallasTemperature ds18b20(new OneWire(16)); // GPIO16 (D0)
 DallasTemperature ds18b20(new OneWire(14)); // GPIO14 (D5)
 DeviceAddress sensorAddress;
BME280I2C::Settings
settings_out(
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::Mode_Forced,
  BME280::StandbyTime_1000ms,
  BME280::Filter_Off,
  BME280::SpiEnable_False,
  BME280I2C::I2CAddr_0x76
),
settings_in(
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::OSR_X1,
  BME280::Mode_Forced,
  BME280::StandbyTime_1000ms,
  BME280::Filter_Off,
  BME280::SpiEnable_False,
  BME280I2C::I2CAddr_0x77
);

BME280I2C BME_OUT(settings_out), BME_IN(settings_in);
/* Параметры индикаторов web интерфейса для плагина Knob
                       Мин  Макс   Шаг    Заголовок      Ед. измер.
|---------------------|----|------|------|--------------|---------| */
knob_t *T = new knob_t( -40,   125,  ".1", "Zone 1", "°C");
knob_t *T1 =new knob_t( -40,   125,  ".1", "Camera", "°C");
knob_t *T2 =new knob_t( -40,   125,  ".1", "Zone 2", "°C");
knob_t *P = new knob_t( 700,   800,   "1", "Press ", "mm");
knob_t *H = new knob_t(   0,   100, ".01", "Hum   ", "%" );
knob_t *L = new knob_t(   0, 65000,   "1", "Light ", "lx");

/* Функции, описывающие инициализацию датчиков */
void out_init() { BME_OUT.begin(); }
void in_init()  { BME_IN.begin(); }
/* Функции, описывающие как получить от внешнего датчика те или иные данные */
float out_temp() { return BME_OUT.temp(BME280::TempUnit_Celsius); } 
float out_hum()  { return BME_OUT.hum(); }
float out_pres() { return BME_OUT.pres(BME280::PresUnit_torr); }

/* Функции, описывающие как получить от внутреннего датчика те или иные данные */
float in_temp()  { return BME_IN.temp(BME280::TempUnit_Celsius); }
float in_hum()   { return BME_IN.hum(); }
float in_pres()  { return BME_IN.pres(BME280::PresUnit_torr); }
float in_temp18b20_0() { int iii=0;
    label_01: float rez=ds18b20.getTempCByIndex(0);
              if (rez==-127.0) {delay(100);iii++; // error... try again
                                ds18b20.requestTemperatures();delay(300);
                                if (iii>10) return 0;
                                goto label_01;}
              else return rez; 
 }
float in_temp18b20_1() { int iii=0;
    label_01: float rez=ds18b20.getTempCByIndex(1);
              if (rez==-127.0) {delay(100);iii++; // error... try again
                                ds18b20.requestTemperatures();delay(300);
                                if (iii>10) return 0;
                                goto label_01;}
              else return rez; 
 }
/* Добавление датчиков в систему */
void sensors_config() {
  Wire.begin(4, 5);
  sensors.add(new knob_t(-100, 0, "1", "RSSI", "dbm"), device::out, "rssi",[&](){ return wifi.isConnected() ? WiFi.RSSI() : 0;    },false);
#if SENSOR_BH1750
  sensors.add(L, device::out, 0x23, "out_light", [&](){ BH1750.begin(); }, [&](){ return BH1750.readLightLevel(); }, true);  
#endif
  /* Внешний датчик */
  sensors.add(P, device::out, 0x76, "out_pressure",    out_pres, false);
  sensors.add(H, device::out, 0x76, "out_humidity",    out_hum,  false);  
  
  /* Внутренний датчик */
 // sensors.add(P, device::in, 0x77, "in_pressure",    in_pres, true);
 // sensors.add(H, device::in, 0x77, "in_humidity",    in_hum,  true);
 // sensors.add(T, device::in, 0x77, "in_temperature", in_init, in_temp, true);

 // sensors.add(new knob_t(0, 5, ".01", "Питание", "V"), device::in, "vcc", [&](){  return ESP.getVcc() * 0.001;   },false);
 // sensors.add(new knob_t(0, 81920, "1", "RAM", "Byte"), device::in, "ram", [&](){ return 81920 - ESP.getFreeHeap(); },false);
   /* temperatures */
  ds18b20.begin(); 
  ds18b20.getAddress(sensorAddress, 0);ds18b20.setResolution(sensorAddress, 11);
  ds18b20.getAddress(sensorAddress, 1);ds18b20.setResolution(sensorAddress, 11);

  //sensors.add(T1, device::in, "ds18b20_s0", [&](){ return ds18b20.getTempCByIndex(0); });
  //sensors.add(T2, device::in, "ds18b20_s1", [&](){ return ds18b20.getTempCByIndex(1); });

 
  sensors.add(T2, device::out, "ds18b20_s1", in_temp18b20_1,  false);             // 3 
  sensors.add(T, device::out, 0x76, "out_temperature", out_init, out_temp, true); // 2  
  sensors.add(T1, device::out, "ds18b20_s0", in_temp18b20_0,  true);              // 1
  
  cron.add(cron::time_30s, [&](){ ds18b20.requestTemperatures(); }, true);    
    /* тестовый вывод */
    
  cron.add(cron::time_10m + cron::minute, [&](){
    #ifdef console
      console.println(sensors.list());
      console.println(sensors.status());
      console.println(sensors.get());
      //console.println(sensors.log());
    #endif
  });
}

#endif
