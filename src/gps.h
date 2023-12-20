#ifndef GPS_H
#define GPS_H
#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
/**
 * @brief สำหรับจัดการ GPS
 *
 */
class GPS
{
    TinyGPSPlus gps;
    int RXPin = D7, TXPin = D8;
    uint32_t GPSBaud = 9600;
    SoftwareSerial *ss;
    String gpsdate;
    String gpsloc;
    String gpstime;
    String gpsdatetime;
    unsigned long timeSinceEpoch;
    unsigned long updatetime;

public:
    void start()
    {
        ss = new SoftwareSerial(RXPin, TXPin);
        ss->begin(GPSBaud);
    }
    void setRx(int pin)
    {
        RXPin = pin;
    }
    void setTx(int pin)
    {
        TXPin = pin;
    }

    void read()
    {
        while (ss->available() > 0)
            if (gps.encode(ss->read()))
                displayInfo();

        if (millis() > 15000 && gps.charsProcessed() < 10)
        {
            Serial.println(F("No GPS detected: check wiring."));
        }
    }
    String getData()
    {
        return gpsloc+" "+gpsdatetime;
    }
    void displayInfo()
    {
        if (gps.location.isValid())
        {
            gpsloc = "";
            gpsloc = "Location: LAT:" + String(gps.location.lat()) + " LNG:" + String(gps.location.lng());
        }
        else
        {
        }


        if (gps.date.isValid())
        {
            char b[100];
            sprintf(b, "%d-%d-%d", gps.date.day(), gps.date.month(), gps.date.year());
            gpsdate = String(b);
        }
        else
        {
            gpsdate = NULL;
        }

        if (gps.time.isValid())
        {
            char b[100];
            sprintf(b, "%d:%d:%d", gps.time.hour(), gps.time.minute(), gps.time.second());
            gpstime = String(b);
        }
        else
        {
            gpstime = NULL;
        }

        if (gpsdate != NULL && gpstime != NULL)
        {
            char buf[500];
            sprintf(buf, "%d-%d-%d %d:%d:%d", gps.date.day(), gps.date.month(), gps.date.year(), gps.time.hour(), gps.time.minute(),
                    gps.time.second());
            gpsdatetime = String(buf);

            Converttime(gpsdatetime);
        }
    }
    void Converttime(String v)
    {
        struct tm tm = {0};
        char buf[100];
        strptime(v.c_str(), "%d-%m-%Y %H:%M:%S", &tm);
        timeSinceEpoch = mktime(&tm);
        updatetime = millis();
    }
    unsigned long timeEpoch()
    {
        return timeSinceEpoch + (updatetime/1000);
    }
};

#endif