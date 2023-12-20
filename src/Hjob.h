#ifndef HJOB_H
#define HJOB_H
#include "SHTSensor.h"
// เป็น class สำหรับ run
struct Hdata
{
    float t = 0;
    float h = 0;
    /* data */
};

class Htask
{

    SHTSensor *sht;
    float t, h;
    bool readok = false;
    unsigned long nextreadtime = 0; // บอกว่าเวลาในการอ่านกี่นาที
    int intervalread = 0;

public:
    void setSensor(SHTSensor *p)
    {
        sht = p;
    }
    int init()
    {
        Wire.begin();
        if (sht == NULL)
            sht = new SHTSensor();
        if (sht->init())
        {
            sht->setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM); // only supported by SHT3x
            return true;
        }
        else
        {
            Serial.println("Sht ERROR");
        }

        return false;
    }
    bool readstatus()
    {
        return readok;
    }
    void setreadNext(int nextreadsec)
    {
        intervalread = nextreadsec;
        nextreadtime = intervalread * 1000 + millis(); // เวลาอ่านต่อไป
    }
    void readInterval(Hdata *p)
    {
        if (millis() > nextreadtime)
        {
            nextreadtime = intervalread * 1000 + millis();
            return read(p);
        }

        p = NULL;
    }
    void read()
    {
        if (sht->readSample())
        {
            readok = true;
            h = sht->getHumidity();
            t = sht->getTemperature();
        }
        else
        {
            h = -200;
            t = -200;
            readok = false;
        }
    }
    void read(Hdata *d)
    {
        if (sht->readSample())
        {
            readok = true;
            h = sht->getHumidity();
            t = sht->getTemperature();
            d->h = h;
            d->t = t;
        }
        else
        {
            h = -200;
            t = -200;
            readok = false;
            d = NULL;
        }
    }

    float geth()
    {
        return h;
    }

    float gett()
    {
        return t;
    }
};
#endif