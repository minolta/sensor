#ifndef KAnalog_h
#define KAnalog_h
#include <inttypes.h>
#include <Arduino.h>

class KAnalog
{
private:
  float av[16];
  int i = 0;
  float oldvalue = 0; //เป็นค่า ความต่างของ psi
  float rawvalue = 0;
  float readvolts = 0;

public:
  float readA0(void);
  float readPsi(float, float);
  float readVolts(void);
  int checkOldvalue(float, float, long);
  float fliter(float);
  float getRawvalue(void);
  float getReadVolts(void);
};
#endif