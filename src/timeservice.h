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
        return ts + ((updatetime + millis()) / 1000);
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

        // struct tm *ptm = localtime(&t);
        struct tm *ptm = gmtime(&t);
        char buffer[100];
        // Format: Mo, 15.06.2009 20:20:00
        strftime(buffer, 100, "%d-%m-%Y", ptm);
//    delete(ptm,sizeof(tm));
// time_t t = getEP
// struct tm *tt = gmtime(&t);
// char buf[100];
// strftime(buf, sizeof(buf), "%d-%m-%Y", tt);
// delete (tt);
#ifdef TIMEDEBUG
        Serial.printf("ToDAY:%s", buffer);
#endif
        return String(buffer);
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
        String today = String(gettodayString());
        String g = today + " " + s;
        struct tm t = {0};
        char buf[200];
        g.toCharArray(buf, 200);
#ifdef TIMEDEBUG
        Serial.printf("Today %s\n", g.c_str());
#endif
        strptime(buf, "%d-%m-%Y %H:%M", &t);
        unsigned long tt = mktime(&t);
        return tt;
    }
};

#endif