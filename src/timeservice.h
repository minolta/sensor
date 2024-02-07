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
    /**
     * @brief Set the Gps objectสำหรับรับเวลาจาก gps
     *
     * @param p
     */
    void setGps(GPS *p)
    {
        gps = p;
    }

    void setTime(time_t t)
    {
        ts = t;
        updatetime = millis();
    }
    void printFreemem()
    {

        Serial.printf("\n %s  = %ld\n", toString().c_str(), system_get_free_heap_size());
    }
    time_t getTime()
    {
        if (gps != NULL)
        {
            unsigned long gpstime = gps->timeEpoch();
            if (gpstime > ts)
            {
                ts = gpstime;
                updatetime = millis();
            }
        }
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

    String toString()
    {
        time_t t = getTime();
        struct tm *ptm = gmtime(&t);
        char buffer[100];
        // Format: Mo, 15.06.2009 20:20:00
        strftime(buffer, 100, "%d-%m-%Y %H:%M", ptm);

        return buffer;
    }
    unsigned long getTimeNow()
    {
        if (gps != NULL)
            return gps->timeEpoch();

        return ts;
    }
    time_t StringtoTime(String str, String format)
    {
        char buf[500];
        tm t;
        // t.tm_year = 2019 - 1900; // Year - 1900
        // t.tm_mon = 7;            // Month, where 0 = jan
        // t.tm_mday = 8;           // Day of the month
        // t.tm_hour = 16;
        // t.tm_min = 11;
        // t.tm_sec = 42;
        // t.tm_isdst = -1;
        str.toCharArray(buf, str.length() + 1);
        Serial.println(str);
        strptime(buf, format.c_str(), &t);
        return mktime(&t);
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
#ifdef TIMEDEBUG
        Serial.printf("\n+++++++++++ %s\n", buf);
#endif
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
    /**
     * @brief เปลียน
     *
     * @param s
     * @return time_t
     */
    time_t stl(String s)
    {
        struct tm t = {0};
        char buf[20];
        s.toCharArray(buf, 20);
#ifdef TIMEDEBUG
        Serial.printf("Today %s\n", s.c_str());
#endif
        strptime(buf, "%H:%M", &t);
        unsigned long tt = mktime(&t);
        return tt;
    }
    unsigned long uptime()
    {
        return millis() / 1000;
    }
};

#endif