#ifndef TIMESERVICE_H
#define TIMESERVICE_H
#include <time.h>
#include <Arduino.h>
#include "gps.h"
class TimeService
{
    GPS *gps;
    time_t ts;
    unsigned long updatetime;

public:
    void setGps(GPS *p)
    {
        gps = p;
    }

    void setTime(time_t t)
    {
        ts = t;
        updatetime = millis();
    }
    time_t getTime()
    {
        return ts + (updatetime / 1000);
    }
    String getTimeString(time_t t)
    {
        return ctime(&t);
    }

    String getTimeString()
    {
        time_t t = getTime();
        return ctime(&t);
    }
    unsigned long getTimeNow()
    {
        if (gps != NULL)
            return gps->timeEpoch();

        return ts;
    }
    /**
     * @brief เอาสติงของวันนี้ไปเป็นออกไปบวกกับเวลาของ job
     *
     *
     * @return String
     */
    String gettodayString(time_t t)
    {
        // time_t t = getEP
        struct tm *tt = gmtime(&t);
        char buf[100];
        strftime(buf, sizeof(buf), "%d-%m-%Y", tt);
        return buf;
    }
    String gettodayString()
    {
        return gettodayString(ts);
    }
    /**
     *
     * เปลียน String d-m-yyyy H:M:S ไปเป็น long
     */
    time_t getTimestamp(String s)
    {
        struct tm t;
        char buf[200];
        s.toCharArray(buf, s.length() + 1);
        Serial.printf("\n+++++++++++ %s\n", buf);
        strptime(buf, "%d-%m-%Y %H:%M:%S", &t);
        return mktime(&t);
    }

    /**
     * @brief เปลียนเวลา job ให้เป็น time stamp
     *
     * @param s
     * @return time_t
     */
    time_t getjobTimestamp(String s)
    {
        String today = gettodayString();
        String g = today + " " + s;

        struct tm t = {0};
        char buf[200];
        g.toCharArray(buf, g.length() + 1);
        Serial.printf("\n+++++++++++ %s\n", buf);
        strptime(buf, "%d-%m-%Y %H:%M", &t);
        unsigned long tt = mktime(&t);
        Serial.println(buf);
        Serial.println(tt);
        return tt;
    }
};

#endif