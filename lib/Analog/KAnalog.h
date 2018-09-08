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

public:
  float readA0(void);
  float readPsi(float, float);
  float readVolts(void);
  int checkOldvalue(float, float, long);
  float fliter(float);
};
#endif