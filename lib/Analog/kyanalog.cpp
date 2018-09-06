#include "KAnalog.h"
#include <Arduino.h>

float KAnalog::readA0(void)
{
    int sensorValue = 0;

    //อ่าน 16 ครั้งเพื่อให้ ละเอียดขึ้น
    for (int i = 0; i < 16; i++)
    {
        sensorValue += analogRead(A0);
    }

    return (float)sensorValue;
}

float KAnalog::readPsi(float vinit, float step)
{

    float volts = readVolts();
    float psi = (volts - vinit) * step;
    return psi;
}

float KAnalog::readVolts(void)
{
    float value = readA0();
    float volts = 3.02 * value / 1023.00;

    return volts;
}