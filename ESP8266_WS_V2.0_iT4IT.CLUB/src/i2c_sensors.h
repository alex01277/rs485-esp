#ifndef IT4IT_CLUB_SENSORS_H
#define IT4IT_CLUB_SENSORS_H

/*
0:success
1:data too long to fit in transmit buffer
2:received NACK on transmit of address
3:received NACK on transmit of data
4:other error
*/

//#define NO_GLOBAL_TWOWIRE
#include <Wire.h>

/* описание способа общения */
class wire_io {
  public:
    void setWire(TwoWire &wire) { this->wire = &wire; }
    /* */
    void setAddress(byte address) { this->address = address; this->find(); }
    byte getAddress() { return this->address; }
    /* */
    bool isExists() { return exists; }
    bool find() {
      if (this->wire) {
        this->wire->beginTransmission(this->address);
        return this->exists = (this->wire->endTransmission() == 0);
      } return false;
    }
    /* */
    bool read(byte reg, byte *data, byte length) {
      byte ord = 0;
      this->wire->beginTransmission(this->address);
      this->wire->write(reg);
      this->wire->endTransmission();
      this->wire->requestFrom(this->address, length);
      while (this->wire->available() and ord < length) data[ord++] = this->wire->read();
      return ord == length;
    }
    bool write(byte reg, byte data) {
      this->wire->beginTransmission(this->address);
      this->wire->write(reg);
      this->wire->write(data);
      return (this->wire->endTransmission() == 0);
    }


  private:
    byte address = 0;
    byte length  = 0;
    bool exists  = false;

    TwoWire *wire = 0;
};

/* ************************************************************************************************************************* *
 * универсальный датчик BME280 
 * ************************************************************************************************************************* */
class bme280: public wire_io {
  public:
    /* возможные адреса датчика */
    typedef enum {
      ADDR_NAN = 0,
      ADDR_76  = 0x76,
      ADDR_77  = 0x77
    } i2c_addr_t;
    /* */
    typedef enum {
      CHIP_NAN = 0,
      BMP280   = 0x58,
      BME280   = 0x60
    } chip_t;
    /* */
    typedef enum {
      ADDR_CTRL_HUM  = 0xF2,
      ADDR_CTRL_MEAS = 0xF4,
      ADDR_CONFIG    = 0xF5,
      ADDR_PRESS     = 0xF7,
      ADDR_TEMP      = 0xFA,
      ADDR_HUM       = 0xFD,
      ADDR_DIG_TEMP  = 0x88,
      ADDR_DIG_PRESS = 0x8E,
      ADDR1_HUM_DIG  = 0xA1,
      ADDR2_HUM_DIG  = 0xE1,
      ADDR_ID        = 0xD0
    } addr_t;
    /* */
    typedef enum {
      LENGTH_DIG_TEMP      = 6,
      LENGTH_DIG_PRESS     = 18,
      LENGTH_DIG_HUM_ADDR1 = 1,
      LENGTH_DIG_HUM_ADDR2 = 7,
      LENGTH_DIG           = 32,
      LENGTH_SENSOR_DATA   = 8
    } length_t;
    /* */
    typedef enum {
      OSR_X1 = 1,
      OSR_X2,
      OSR_X4,
      OSR_X8,
      OSR_X16
    } osr_t;
    typedef enum {
      MODE_SLEEP,
      MODE_FORCED,
      MODE_NORMAL
    } mode_t;
    typedef enum {
      standbyTime_500us,
      standbyTime_62500us,
      standbyTime_125ms,
      standbyTime_250ms,
      standbyTime_50ms,
      standbyTime_1000ms,
      standbyTime_10ms,
      standbyTime_20ms
    } standbyTime_t;
    /* */
    typedef enum {
      filter_Off,
      filter_2,
      filter_4,
      filter_8,
      filter_16
    } filter_t;
    /* */
    typedef enum spiEnable {
      spiEnable_False,
      spiEnable_True
    } spiEnable_t;
    /* */
    struct settings {
      osr_t tempOSR = OSR_X1;
      osr_t humOSR  = OSR_X1;
      osr_t presOSR = OSR_X1;
      mode_t mode = MODE_FORCED;
      standbyTime_t standbyTime = standbyTime_1000ms;
      filter_t filter = filter_Off;
      spiEnable_t spiEnable = spiEnable_False;
   } m_settings;
    /* устанавливаем адрес и длинну данных */
    bme280(i2c_addr_t address = ADDR_NAN) {
      this->setAddress(address?:ADDR_76);
    }
    void setAddress(i2c_addr_t address) { wire_io::setAddress(address); }
    bool begin() {
      if (this->find()) {
        byte model;
        /* тип датчика */
        if (this->read(ADDR_ID, &model, 1)) {
          switch (model) {
            case BMP280: this->model = BMP280; break;
            case BME280: this->model = BME280; break;
            default: return false;
          }
          /* читаем текущие значения регистров */
          if (!this->read(ADDR_DIG_TEMP,  &this->dig[0],  LENGTH_DIG_TEMP) or
              !this->read(ADDR_DIG_PRESS, &this->dig[6],  LENGTH_DIG_PRESS) or
              !this->read(ADDR1_HUM_DIG,  &this->dig[24], LENGTH_DIG_HUM_ADDR1) or
              !this->read(ADDR2_HUM_DIG,  &this->dig[25], LENGTH_DIG_HUM_ADDR2)
          ) return false;
          /* */
          for (byte i = 0; i < 32; i++) console.printf("%x ", this->dig[i]);
          console.println();
          /* */
          uint8_t ctrlHum, ctrlMeas, config;
          // ctrl_hum register. (ctrl_hum[2:0] = Humidity oversampling rate.)
          ctrlHum = (uint8_t)m_settings.humOSR;
          // ctrl_meas register. (ctrl_meas[7:5] = temperature oversampling rate, ctrl_meas[4:2] = pressure oversampling rate, ctrl_meas[1:0] = mode.)
          ctrlMeas = ((uint8_t)m_settings.tempOSR << 5) | ((uint8_t)m_settings.presOSR << 2) | (uint8_t)m_settings.mode;
          // config register. (config[7:5] = standby time, config[4:2] = filter, ctrl_meas[0] = spi enable.)
          config = ((uint8_t)m_settings.standbyTime << 5) | ((uint8_t)m_settings.filter << 2) | (uint8_t)m_settings.spiEnable;
        }
      }
      return false;
    }
    
  private:
    byte dig[32];
    chip_t model = CHIP_NAN;
} bme280;

/* ************************************************************************************************************************* *
 * 
 * ************************************************************************************************************************* */

#endif
