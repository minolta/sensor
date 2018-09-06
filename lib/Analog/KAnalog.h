#ifndef KAnalog_h
#define KAnalog_h
#include <inttypes.h>
#include <Arduino.h>

class KAnalog
{
  public:
    float readA0(void);
    float readPsi(float,float);
    float readVolts(void);
};
#endif