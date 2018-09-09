#ifndef Kktype_h
#define Kktype_h
#include <inttypes.h>
#include <Arduino.h>

class Kktype
{
private:
  float av[16];
  int i = 0;
  float oldvalue = 0; //เป็นค่า ความต่างของ psi 

public:
  float readKtype(void);
  float fliter(float);
};
#endif