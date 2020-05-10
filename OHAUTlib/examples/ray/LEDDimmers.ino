#include <Arduino.h>
#include "LEDDimmers.h"


#define ANALOG_RANGE 1000
#define BOOT_VALUE 1000
#define BOOT_VALUE_F 1.0

int DIMMER_PIN[] = {14, 5, 15};

void ticker_update(LEDDimmers *cls) {
  cls->update();
}

void LEDDimmers::setup(float* boot_values) {

  analogWriteRange(ANALOG_RANGE);
  for (int i=0; i<N_DIMMERS; i++) {
    float boot_value = boot_values?boot_values[i]:1.0;
    float analog_val = pow(boot_value,
                           _gamma)*float(ANALOG_RANGE);
    pinMode(DIMMER_PIN[i], OUTPUT);
    analogWrite(DIMMER_PIN[i], analog_val);
   _dimmers[i] = boot_value;
   _dimmers_val[i] = boot_value;
  }
  update_ticker.attach(DIMMER_PERIOD, ticker_update, this);
}

void LEDDimmers::halt()
{
  for (int i=0; i<N_DIMMERS; i++) {
    analogWrite(DIMMER_PIN[i], 0);
  }
  update_ticker.detach();
}

void LEDDimmers::restart() {
  for (int i=0; i<N_DIMMERS; i++) {
    setDimmer(i, _dimmers[i]);
  }
  update_ticker.attach(DIMMER_PERIOD, ticker_update, this);
}

void LEDDimmers::setGamma(float gamma) {
  _gamma = gamma;
}

void LEDDimmers::update() {

  for (int n=0; n<N_DIMMERS; n++) {
    float step = (_dimmers[n]-_dimmers_val[n])*DIMMER_STEP;
     if (_dimmers_val[n]!=_dimmers[n]) {
        if (fabs(_dimmers_val[n] - _dimmers[n])<=step)
          _dimmers_val[n] = _dimmers[n];
        else
          _dimmers_val[n] += step;
        analogWrite(DIMMER_PIN[n],
                    pow(_dimmers_val[n], _gamma)*float(ANALOG_RANGE));
     }
  }
}

void LEDDimmers::setDimmer(int n, float value) {

   if (!(n>=0 && n<N_DIMMERS))
      return;

   if (value<0.0) value = 0.0;
   if (value>1.0) value = 1.0;

   _dimmers[n] = value;
}

float LEDDimmers::getDimmer(int n) {
  if (n>=0 && n<N_DIMMERS)
    return _dimmers[n];

  return -1;
}
