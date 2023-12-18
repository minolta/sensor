#ifndef TIMESERVICE_H
#define TIMESERVICE_H
#include <time.h>
#include <Arduino.h>
class TimeService
{
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
    /**
     *
     * เปลียน String d-m-yyyy H:M:S ไปเป็น long
     */
    time_t getTimestamp(String s)
    {
        struct tm t;
        char buf[200];
        s.toCharArray(buf, s.length());
        strptime(buf, "%d-%m-%Y %H:%M:%S", &t);
        return mktime(&t);
    }
};

#endif