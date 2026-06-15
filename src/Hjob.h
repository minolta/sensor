#ifndef HJOB_H
#define HJOB_H
#include "SHTSensor.h"

// SHT on D1=SCL, D2=SDA (same bus as SSD1306 OLED)
#ifndef SHT_SDA_PIN
#define SHT_SDA_PIN D2
#endif
#ifndef SHT_SCL_PIN
#define SHT_SCL_PIN D1
#endif

struct Hdata
{
    float t = 0;
    float h = 0;
};

class Htask
{
    SHTSensor *sht = NULL;
    float t = 0;
    float h = 0;
    bool readok = false;
    bool inited = false;
    unsigned long nextreadtime = 0;
    int intervalread = 0;

public:
    void setSensor(SHTSensor *p)
    {
        sht = p;
    }

    bool isReady() const
    {
        return inited;
    }

    int init()
    {
        if (inited)
            return true;

        Wire.begin(SHT_SDA_PIN, SHT_SCL_PIN);
        Wire.setClock(100000);
        delay(20);

        if (sht == NULL)
            sht = new SHTSensor();

        if (sht->init())
        {
            sht->setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM);
            inited = true;
            readok = true;
            h = sht->getHumidity();
            t = sht->getTemperature();
#ifdef HDEBUG
            Serial.println("\n SHT sensor ok");
#endif
            return true;
        }

        inited = false;
        readok = false;
        Serial.println(F("SHT init fail (check D1/D2 wiring, havesht=1)"));
        return false;
    }

    bool readstatus()
    {
        return readok;
    }

    void setreadNext(int nextreadsec)
    {
        intervalread = nextreadsec;
        nextreadtime = (unsigned long)intervalread * 1000UL + millis();
    }

    void readInterval(Hdata *p)
    {
        if (millis() > nextreadtime)
        {
            nextreadtime = (unsigned long)intervalread * 1000UL + millis();
            read(p);
        }
        p = NULL;
    }

    float readInterval()
    {
        if (!inited)
            return -100;
        if (millis() > nextreadtime)
        {
            nextreadtime = (unsigned long)intervalread * 1000UL + millis();
            return read();
        }
        return -100;
    }

    float read()
    {
        if (!inited || sht == NULL)
        {
            readok = false;
            return -200;
        }

        if (sht->readSample())
        {
            readok = true;
            h = sht->getHumidity();
            t = sht->getTemperature();
#ifdef HDEBUG
            Serial.printf("\n H:%f T:%f ", h, t);
#endif
        }
        else
        {
            h = -200;
            t = -200;
            readok = false;
#ifdef HDEBUG
            Serial.println(F("\n SHT read fail"));
#endif
        }

        return h;
    }

    void read(Hdata *d)
    {
        read();
        if (d != NULL && readok)
        {
            d->h = h;
            d->t = t;
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
