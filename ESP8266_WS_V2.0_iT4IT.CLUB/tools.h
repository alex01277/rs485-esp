#ifndef TOOLS_H
#define TOOLS_H

//#define NO_GLOBAL_TWOWIRE
#include <Wire.h>

/* Измерение напряжения питания */
ADC_MODE(ADC_VCC);

/* Индикация состояния контроллера */
class smartBlink {
  public:
    /* доступные порты */
    typedef enum {
      gpio0 = 0,
      gpio2 = 2, // NodeMCU
    } port_t;
    /* человеческие имена для режимов */
    typedef enum {
      mode_off    = 0, // Светодиод выключен
      mode_flash1 = 1, // Короткая вспышка раз в секунду
      mode_flash2 = 2, // Две короткие вспышки раз в секунду
      mode_flash3 = 3, // Три короткие вспышки раз в секунду
      mode_flash4 = 4, // Частые короткие вспышки (4 раза в секунду)
      mode_burn   = 5, // Горит постоянно
      mode_inhalf = 6  // Мигание по 0.5 сек
    } mode_t;
    /* инициализация объекта */
    smartBlink(port_t gpio = smartBlink::gpio2, bool on = LOW): port(gpio), loadON(on) {
      this->setPort();
      os_timer_setfn(this->timer, reinterpret_cast<ETSTimerFunc*>(&smartBlink::loop), reinterpret_cast<void*>(this));
      os_timer_arm(this->timer, 125, 1);
    }
    ~smartBlink() {
      os_timer_disarm(this->timer);
      delete this->timer;
    }
    /* смена режима */
    void setMode(mode_t mode) {
      this->step = 0;
      this->prev = this->mode;
      this->mode = mode;
    }
    /* откат к предыдущему режиму */
    void previous() {
      this->step = 0;
      this->mode = this->prev;
    }
    
  private:
    ETSTimer *timer = new ETSTimer;
    /* проход по маске */
    static void loop(smartBlink *self) { 
      if (self->mode) {
        if (self->step > 7) self->step = 0;
        digitalWrite(self->port, (self->list[self->mode] >> self->step++ & 1) ? self->loadON : !self->loadON);
      }
    }
    /* установка состояния порта */
    void setPort() {
      pinMode(this->port, OUTPUT);
      digitalWrite(this->port, !this->loadON);
    }
    /* порт и сигнал включения нагрузки (LOW/HIGH) */
    port_t port;
    bool loadON;
    /* все по режимам */
    mode_t mode  = mode_off;
    mode_t prev  = mode_off;
    byte step    = 0;
    byte list[7] = {
      0B00000000,
      0B00000001,
      0B00000101,
      0B00010101,
      0B01010101,
      0B11111111,
      0B00001111
    };
} blink;

/*
  Медианный фильтр
  https://ru.wikipedia.org/wiki/%D0%9C%D0%B5%D0%B4%D0%B8%D0%B0%D0%BD%D0%BD%D1%8B%D0%B9_%D1%84%D0%B8%D0%BB%D1%8C%D1%82%D1%80
  Экспериментальный вариант фильтра, маскирующегося под числовую переменную float для удобства внедрения в проекты:
    float A = 1;
    medianFilter_t B = 2;
    B = 2;
    B = A;
    B = 3;
    B = 3 + 1;
    B = 0;
    Serial.println(B); // Вернет отфильтрованное значение, полученное в ходе накопления значений [2, 1, 3, 4, 0] то есть 2
*/
class medianFilter_t {
  public:
    medianFilter_t(byte size = 5) {
      if (size < 3) this->size = 3;
      else this->size = (size % 2 != 0) ? size : ++size;
      this->buffer = new float[this->size]{0};
      this->position = 0;
    }
    ~medianFilter_t() {
      delete [] this->buffer;
    }
    /* Маскируемся под переменную float, но изменяем поведение обычных операторов сложения и присвоения */
    operator float () { return this->val; }
    float operator = (float val) { (*this) += val; }
    float operator += (float val) {
      this->buffer[this->position++] = val;
      if (this->position == this->size) this->position = 0;
      while(true) {
        bool filtered = true;
        for (byte i = 0; i < this->size - 1; i++) {
          if (this->buffer[i] < this->buffer[i + 1]) {
            float tmp = this->buffer[i];
            this->buffer[i] = this->buffer[i + 1];
            this->buffer[i + 1] = tmp;
            filtered = false;
          }
        } yield();
        if (filtered) break;
      }
      this->val = this->buffer[(this->size - 1) / 2];
      return this->val;
    }
    /* Вывод значения в виде целого числа */
    int toInt() { return (int)this->val; }

  private: 
    byte position;
    byte size = 0;
    float val = 0;
    float *buffer;
};

/*
   https://habrahabr.ru/post/140274
*/
class kalman_t {  
  public:
    kalman_t(float q, float r, float p, float value) {
      this->q = q;
      this->r = r;
      this->p = p;
      this->x = value;
    }
    
    float get(float data) {
      this->update(data);
      return x;
    }
    
  private:
    float q; //process noise covariance
    float r; //measurement noise covariance
    float x; //value
    float p; //estimation error covariance
    float k; //kalman gain
    
    void update(float data) {
      p = p + q;
      
      //measurement update
      k = p / (p + r);
      x = x + k * (data - x);
      p = (1 - k) * p;
    }
};

/*
  Функция расчета абсолютной влажности воздуха (грамм воды на один кубический метр воздуха)
  https://carnotcycle.wordpress.com/2012/08/04/how-to-convert-relative-humidity-to-absolute-humidity/
*/
float absoluteHumidity(float t, float h) {
  return (6.112 * pow(2.718281828, (17.67 * t) / (t + 243.5)) * h * 2.1674) / (273.15 + t);
}

#endif
