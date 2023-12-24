#ifndef TASKSERVICE_H
#define TASKSERVICE_H
#include "Espjob.h"
#include "timeservice.h"
#include "gps.h"
#include "hjob.h"
class Task
{
    Espjob *job;
    Foundjob *found;
    unsigned long exittime = 0;
    unsigned long runtime = 0;
    unsigned long waittime = 0;
    bool runok = false;
    bool end = false;
    Task *p = NULL;
    Task *n = NULL;

public:
    void setRuntime(unsigned long r)
    {
        runtime = r;
    }
    void setRunok(bool f)
    {
        runok = f;
    }
    bool getRunok()
    {
        return runok;
    }
    void setWaittime(unsigned long r)
    {
        waittime = r;
    }
    unsigned long getRuntime()
    {
        return runtime;
    }
    void setEnd(bool f)
    {
        end = f;
    }
    unsigned long getWaittime()
    {
        return waittime;
    }
    void setJob(Espjob *p)
    {
        job = p;
    }
    void setexittime(unsigned long i)
    {
        exittime = i;
    }
    Espjob *getJob()
    {
        return job;
    }
    Task *getp()
    {
        return p;
    }
    Task *getn()
    {
        return n;
    }
    bool getEnd()
    {
        return end;
    }
    int getID()
    {
        return job->id;
    }
    void setNext(Task *pp)
    {
        n = pp;
    }
    void setPrv(Task *pp)
    {
        p = pp;
    }
};
class TaskService
{

    unsigned long nextrun = 0;
    int timetorun;        // เวลาที่ run เป็น วินาที
    Task *runlist = NULL; // สำหรับ Run Job
    Task *firsttask = NULL;
    Job *jobservice = NULL;
    Htask *htask = NULL; // สำหรับอ่าน sht
    TimeService *timeService = NULL;
    GPS *gps;

public:
    Task *getF()
    {
        return firsttask;
    }
    void setGps(GPS *p)
    {
        gps = p;
    }
    void setTimeService(TimeService *p)
    {
        timeService = p;
        if (jobservice != NULL)
            jobservice->setTimeService(timeService);
    }

    // สำหรับทดสอบ
    void setTask(Task *nt)
    {
        firsttask = runlist = nt;
    }

    void freeTask()
    {

        Task *i = firsttask;
        Task *todelete = NULL;
        while (i != NULL)
        {
            todelete=NULL;
            if (i->getEnd())
            {
                todelete = i;
                Serial.printf("End %d\n", todelete->getJob()->id);
                if (todelete->getp() != NULL && todelete->getn() != NULL)
                {
                    Serial.println("last delete");
                    todelete->getp()->setNext(todelete->getn());
                    todelete->getn()->setPrv(todelete->getp());
                    // firsttask = NULL; //ตัดตัวสุดท้ายแล้ว

                }
                else if(todelete->getp()==NULL && todelete->getn()!=NULL)
                {
                    //เป็นตัวแรก
                    Serial.println("first delete");
                    todelete->getn()->setPrv(todelete->getp()); 
                    firsttask = todelete->getn();//ย้ายตัวชี้แรกไป node ต่อไป
                    
                }
                else if(todelete->getp()!=NULL && todelete->getn()==NULL)
                {
                    Serial.println("delete last");
                    //เป็นตัวสุดท้าย
                    todelete->getp()->setNext(todelete->getn()); //ตัดแล้ว
                    if(todelete->getp()->getp()==NULL)
                    {
                        //ตัวสุดท้ายแล้ว
                        firsttask = todelete->getn();
                    }
                }
                else if(todelete->getp()==NULL && todelete->getn()==NULL)
                {
                    firsttask = NULL;
                }
            }

            i = i->getn();

            if(todelete!=NULL)
            {
                delete todelete;

            }
        }
    }
    /**
     * @brief ถ้ามี run อยู่ ให้ return true ถ้าไม่เจอ return false
     *
     * @param p
     * @return true
     * @return false
     */
    bool checkrun(Foundjob *p)
    {
        Task *index = firsttask;
        while (index != NULL)
        {
            if (index->getID() == p->job->id && index->getEnd() == 0)
            {
#ifdef DEBUG
                Serial.printf("\nCan run this job %d \n", p->job->id);
#endif
                return true;
            }
            index = index->getn();
        }

        return false;
    }
    Task *newTask(Foundjob *p, Task *from)
    {
        if (from == NULL)
        {
            Serial.println("New first task");
            Task *t = new Task();
            t->setJob(p->job);
            t->setNext(NULL);
            t->setPrv(from);
            t->setexittime(millis() + ((p->job->runtime + p->job->waittime) * 1000));
            runlist = t;
            firsttask = t;
            return t;
        }
        else
        {
            Serial.println("New next task");

            Task *t = new Task();
            t->setJob(p->job);
            t->setNext(NULL);
            t->setPrv(from);
            t->setexittime(millis() + ((p->job->runtime + p->job->waittime) * 1000));
            from->setNext(t);

            return t;
        }
    }
    void exec(Task *p)
    {
        Serial.println(p->getJob()->id);
        Espjob *job = p->getJob();
        time_t base = timeService->getTime();
        unsigned long r = job->runtime + base;
        p->setRuntime(r); // เวลาที่จะหมด
        unsigned long waittime = job->waittime;
        p->setWaittime(job->waittime + r);
        digitalWrite(job->port, job->out);
    }
    void deleteTask(Task *p)
    {

        Serial.printf("\ncTO delete %x %x %x\n", p->getp(), p->getn(), p);
        if (p != NULL)
        {
            if (p->getn() != NULL)
            {
                p->getn()->setPrv(p->getp());
            }
            if (p->getp() != NULL)
            {
                p->getp()->setNext(p->getn());
            }
            delete p;
        }
    }

    void check(Task *t)
    {
        if (t->getWaittime() <= timeService->getTime() && !t->getEnd())
        {
            t->setEnd(true);
            Serial.println("End task");
        }
        else if (t->getRuntime() <= timeService->getTime() && t != NULL)
        {
            digitalWrite(t->getJob()->port, !t->getJob()->out);
            t->setRunok(true);
        }
    }

    // สำหรับ loop
    void updateExce()
    {
        Task *f = firsttask;
        while (f != NULL)
        {

#ifdef DEBUG
            printTask(f);
#endif

            check(f);
            if (f != NULL)
                f = f->getn();
        }
    }
    /**
     * @brief รับ job เข้ามาในระบบ
     *
     * @param foundjobs
     */
    void run(Foundjob *foundjobs)
    {
        Foundjob *i = foundjobs;
        while (foundjobs != NULL)
        {

#ifdef DEBUG
            Serial.println(foundjobs->job->id);
#endif

            if (!checkrun(foundjobs))
            {
                Task *p = newTask(foundjobs, firsttask);
                exec(p);
            }
            else
            {
#ifdef DEBUG
                Serial.printf("Not run this job %d\n", foundjobs->job->id);
#endif
            }
            foundjobs = foundjobs->n;
        }
    }
    void printTask()
    {
        Task *fp = firsttask;
        Serial.print("===============Task===============\n");
        while (fp != NULL)
        {
            Espjob *p = fp->getJob();
            Serial.printf("ID:%d HL:%f - HH:%f\n RUN Time:%d  Waittime:%d runtime:%ld - waittime:%ld  %ld", p->id, p->hlow, p->hhigh,
                          p->runtime, p->waittime, fp->getRuntime(), fp->getWaittime(), timeService->getTime());
            fp = fp->getn();
        }
        Serial.print("==================================\n");
    }

    void printTask(Task *fp)
    {
        Serial.print("===============Task===============\n");
        Espjob *p = fp->getJob();
        Serial.printf("ID:%d HL:%f - HH:%f\n RUN Time:%d  Waittime:%d runtime:%ld - waittime:%ld  NOW:%ld  Next:%x Prv:%x end:%d\n", p->id, p->hlow, p->hhigh,
                      p->runtime, p->waittime, fp->getRuntime(), fp->getWaittime(), timeService->getTime(),
                      fp->getn(), fp->getp(), (void *)fp->getn(), fp->getEnd());
        Serial.print("==================================\n");
    }
    void setHtask(Htask *p)
    {
        htask = p;
    }
    Foundjob *loadjob()
    {

        float hnow = htask->geth();
        Serial.printf("H: now %.2f", hnow);
        return NULL;
    }
    Foundjob *loadbyh()
    {
        return jobservice->loadjobByh(readH());
    }
    Foundjob *loadbyh(float h)
    {
        return jobservice->loadjobByh(h);
    }
    Foundjob *loadbytime()
    {

        return jobservice->loadjobByt();
    }

    Foundjob *loadalljob()
    {
        // Foundjob *found = loadbytime();
        return jobservice->loadalljob(readH());
    }
    float readH()
    {
        htask->read();
        return htask->geth();
    }
    void setJobService(Job *p)
    {
        jobservice = p;
    }
};

#endif