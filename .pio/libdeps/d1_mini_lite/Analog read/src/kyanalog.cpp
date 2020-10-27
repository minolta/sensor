#include "KAnalog.h"
#include <Arduino.h>

float KAnalog::readA0(void)
{
    return (float)analogRead(A0);
}
float KAnalog::getRawvalue()
{
    return rawvalue;
}
float KAnalog::getReadVolts()
{
    return readvolts;
}

float KAnalog::readPsi(float vinit, float step)
{
    float volts = readVolts();
    readvolts = volts;
    float psi = (volts - vinit) * step;
    return psi;
}

float KAnalog::fliter(float v)
{

    av[i] = v;

    if (i > 16)
    {
        i = 0;
    }
    else
        i++;

    float t = 0;
    for (int j = 0; j < 16; j++)
    {
        t = t + av[j];
        //  Serial.println(av[j]);
    }
    v = t / 16;

    Serial.println(v);
    return v;
}
//สำหรับดูว่าค่าต่างกันมากหรือเปล่าถ้าต่างมาก return 1 ถ้าไม่ return 0
int KAnalog::checkOldvalue(float old, float newvalue, long tr)
{

    long v = 0;
    if (old < newvalue)
    {
        v = newvalue - old;
    }
    else
    {
        v = old - newvalue;
    }

    if (v >= tr)
        return 1;

    return 0;
}

float KAnalog::readVolts(void)
{

    float volts = 0;

    float value = 0;

    //อ่านแบบหาค่าเฉลี่ย
    for (i = 0; i < 16; i++)
    {
        value += readA0();
    }
    value = value/16;


    Serial.print("RAW : ");
    rawvalue = value;
    Serial.println(value);
    volts = (3.30 * value / 1023.00);
    return volts;
}