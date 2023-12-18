#ifndef RUNJOB_H
#define RUNJOB_H
#include "Espjob.h"
#include "Hjob.h"
#include "timeservice.h"
#include <Arduino.h>
// สำหรับเก็บ job ที่กำลังทำงานอยู่
class Runtask
{
    Espjob *run;
    int id = 0; // บอกว่า job ไหน run อยู่
    long runtime = 0;
    long waittime = 0;
    unsigned long startrun = 0;
    unsigned long endrun = 0;
    unsigned long output;
    int port = 0;
};
class Runjob
{

    Espjob *job = NULL;
    Htask *htask = NULL;
    TimeService *ts = NULL;
    int runstatus = 0; // บอกว่าตอนนี้กำลังทำงานอะไรอยู่ถ้า 0 แสดงว่าว่า
    int runtime, waittime;
    int port;
    unsigned long r, w;
    unsigned long base = 0;

public:
    void setHtask(Htask *p)
    {
        htask = p;
    }
    /**
     * @brief run now
     *
     * @param p
     */
    int run(Espjob *p)
    {
        job = p;
        runtime = waittime = 0;
        r = w = 0;
        base = millis();
        if (checkh(p))
        {
            runtime = job->runtime;
            r = (job->runtime * 1000) + base; // เวลาที่จะหมด

            waittime = job->waittime;
            w = (job->waittime * 1000) + r;

            digitalWrite(job->port, job->out);
            return 1;
        }

        return 0;
    }
    int directrun(Espjob *torun)
    {
        r = w = 0;
        base = millis();
        job = torun;
        runtime = torun->runtime;
        waittime = torun->waittime;
        runstatus = 1;
        r = (job->runtime * 1000) + base; // เวลาที่จะหมด
        w = (job->waittime * 1000) + r;

        digitalWrite(torun->port, torun->out);
        return 1;
    }
    bool isRun()
    {
        return runstatus;
    }
    /**
     * @brief countdown runtime
     *
     */
    void update()
    {
        if (millis() <= r)
        {
            // Serial.printf("Runtime %d", runtime);
            runstatus = 1;
        }
        else if (millis() <= w)
        {
            // Serial.printf("Waittime %d",waittime);

            digitalWrite(job->port, !job->out);
            waittime--;
            runstatus = 1;
        }
        else
        {
            runstatus = 0;
            digitalWrite(job->port, !job->out);
        }
    }
    void setport()
    {

        port = job->port;
    }
    int checkh(Espjob *p)
    {
        if (job == NULL)
            job = p;

        float h = htask->geth();
        // printf(" hl %f <= %f <= %f",job->hlow,h,job->hhigh);
        if (job->hlow <= h && h <= job->hhigh)
            return 1;
        return 0;
    }

    int checktime(Espjob *p)
    {

        return false;
    }
    Espjob *getJob()
    {
        return job;
    }
};
#endif