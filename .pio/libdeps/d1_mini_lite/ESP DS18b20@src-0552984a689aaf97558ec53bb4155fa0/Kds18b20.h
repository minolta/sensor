#include <Arduino.h>
#include "OneWire.h"
#ifndef KDS18B20_H
#define KDS18B20_H
class Kds18b20
{
private:
    int pin = 0;
    int have = 0;
    OneWire ds;

public:
    Kds18b20(int);
    float readDs();
    
};

#endif