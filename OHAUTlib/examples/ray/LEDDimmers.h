#ifndef __LED_DIMMERS_H
#define __LED_DIMMERS_H

#define N_DIMMERS 3

#define DIMMER_STEP   0.05
#define DIMMER_PERIOD 0.02

class LEDDimmers {
private:
  float _gamma = 1.5;
  float _dimmers[N_DIMMERS];
  float _dimmers_val[N_DIMMERS];
  Ticker update_ticker;
public:
  void setup(float* boot_values=NULL);
  void update();
  void setGamma(float gamma);
  void setDimmer(int n, float value);  
  float getDimmer(int n);
  void halt();
  void restart();
};

#endif
