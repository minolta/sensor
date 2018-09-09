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

    return (float)sensorValue / 16;
}

float KAnalog::readPsi(float vinit, float step)
{
    float volts = readVolts();
    float psi = (volts - vinit) * step;
    psi = fliter(psi); //หาค่าเฉลีย
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
    v = t/16;

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
    //for (int i = 0; i < 16; i++)
    // {
    float value = readA0();
    //  Serial.println(value);
    volts = (3.30 * value / 1023.00);
    //}
    return volts ;
}