#ifndef ESPJOB_H
#define ESPJOB_H
#include "timeservice.h"

struct Espjob
{
    int id;
    long sdate = 0;
    long edate = 0;

    float hlow = 0.0;
    float hhigh = 0.0;

    int port = 0;
    int runtime = 0;
    int waittime = 0;
    int out = 1;

    bool enable = false;
    unsigned long st;
    unsigned long et;
    String stime;
    String etime;
    Espjob *p;
    Espjob *n;
    /* data */
};
struct Foundjob
{
    Espjob *job; // ใช้ชี้ไปงาน
    Foundjob *p;
    Foundjob *n;
};
class Job
{

    Espjob *data = NULL;
    Espjob *f = NULL;
    Espjob *l = NULL;
    int size = 0;
    int foundjobsize = 0;
    TimeService *ts = NULL;

public:
    void addJob(Espjob *job)
    {
        if (data == NULL)
        {
            data = job;
            data->p = NULL;
            data->n = NULL;
            f = data;
            size = 1;
        }
        else
        {
            data->n = job;
            job->p = data;
            job->n = NULL;
            data = job;
            l = data;
            size++;
        }
    }
    void addJob(Espjob job)
    {
        if (data == NULL)
        {
            data = &job;
            data->p = NULL;
            data->n = NULL;
            f = data;
            size = 1;
        }
        else
        {
            data->n = &job;
            job.p = data;
            job.n = NULL;
            data = &job;
            l = data;
            size++;
        }
    }
    int getsize()
    {
        return size;
    }
    Espjob *getfirst()
    {
        return f;
    }
    Espjob *last()
    {
        return l;
    }
    void printFound(String tag = "default:", Foundjob *ff = NULL)
    {
        Foundjob *f = ff;
        while (f != NULL)
        {
            Serial.println("------------------------------------------------------------------------------");
            Serial.printf("%s id: %d %f %f   Stime:%s Etime:%s  P:%x N:%x  runtime:%ld ,waittime:%ld \n", tag.c_str(), f->job->id, f->job->hlow, f->job->hhigh, f->job->stime,
                          f->job->etime, f->p, f->n, f->job->runtime, f->job->waittime);
            Serial.printf("\n###### %d #####\n", f->job->id);
            Serial.println("------------------------------------------------------------------------------");

            f = f->n;
        }
    }
    int printfound()
    {
        Serial.println("Found " + foundjobsize);
        return foundjobsize;
    }
    void setTimeService(TimeService *p)
    {
        ts = p;
    }
    /**
     * @brief เปลียน link list ไปเป็น String ชุดสำหรับเก็บลง file
     *
     * @return String
     */
    String toString()
    {
        String jobs = "";
        Espjob *t = f;
        for (; t != NULL;)
        {
            char b[512];
            // Serial.printf("\nData %d\n", t->id);
            sprintf(b, "%d,%.2f,%.2f,%d,%d,%d,%d,%d,%s,%s|", t->id, t->hlow, t->hhigh, t->port, t->runtime, t->waittime, t->enable, t->out, t->stime.c_str(), t->etime.c_str());
            jobs += String(b);

            t = t->n;
        }

        return jobs;
    }
    /**
     * @brief เปลียน link list ไปเป็น String ชุดสำหรับเก็บลง file
     *
     * @return String
     */
    String toStringln()
    {
        String jobs = "";
        Espjob *t = f;
        char b[1000];
        while (t != NULL)
        {

            // Serial.printf("\nData %d\n", t->id);
            sprintf(b, "%d,%.2f,%.2f,%d,%d,%d,%d,%d,%s,%s\n", t->id, t->hlow, t->hhigh, t->port, t->runtime, t->waittime, t->enable, t->out, t->stime, t->etime);
            jobs += String(b);
            t = t->n;
        }

        return jobs;
    }
    /**
     * @brief สำหรับสร้าง job จาก String
     *
     * @param d
     * @return Espjob*
     */
    Espjob *n(String d)
    {
        String ff = d + "T"; // ปิดท้ายด้วย T
        int size = ff.length();
        char buf[size];
        ff.toCharArray(buf, size);
#ifdef JOBDEBUG
        Serial.printf("\n new object in file  %s ]\n", buf);
#endif
        Espjob *t = new Espjob();
        t->p = NULL;
        t->n = NULL;
        char a[20], b[20];
        ///          %d,%.2f,%.2f,%d,%d,%d,%d,%d
        sscanf(buf, "%d,%f,%f,%d,%d,%d,%d,%d,%[^,],%[^T]",
               &t->id, &t->hlow, &t->hhigh, &t->port, &t->runtime, &t->waittime, &t->enable, &t->out, a, b);

        t->stime = a;
        t->etime = b;
        return t;
    }
    Espjob *newjob(int id, float hlow, float hhigh, int port, int runtime, int waittime, int enable,
                   int out, String stime, String etime)
    {
        Espjob *t = new Espjob();
        t->p = NULL;
        t->n = NULL;
        t->id = id;
        t->hlow = hlow;
        t->hhigh = hhigh;
        t->port = port;
        t->runtime = runtime;
        t->waittime = waittime;
        t->enable = enable;
        t->out = out;
        // if (stime.length() > 3)
        t->stime = stime.c_str();
        // if (etime.length() > 3)
        t->etime = etime.c_str();
        t->st = ts->StringtoTime(stime,"HH:MM");
        t->et = ts->StringtoTime(etime,"HH:MM");
        return t;
    }
    Espjob *findById(int id)
    {

        Espjob *index = f;

        while (index != NULL)
        {
            if (index->id == id)
                return index;

            index = index->n;
        }

        return NULL;
    }
    bool edit(Espjob *edit)
    {
        Espjob *found = findById(edit->id);

        if (found != NULL)
        {
            found->edate = edit->edate;
            found->enable = edit->enable;
            found->hhigh = edit->hhigh;
            found->hlow = edit->hlow;
            // found->n = edit->n;
            found->out = edit->out;
            // found->p = edit->p;
            found->port = edit->port;
            found->runtime = edit->runtime;
            found->waittime = edit->waittime;
            return true;
        }

        return false;
    }

    bool deletejob(Espjob *todelete)
    {

        if (todelete->n != NULL && todelete->p != NULL)
        { // node กลาง
            // ถ้า node ต่อไปไม่ใช่ NULL ต้องให้ n มาชี้ p
            todelete->n->p = todelete->p;
            todelete->p->n = todelete->n; // ให้node ก่อนข้ามไปชี้ n
            todelete->p = NULL;
            todelete->n = NULL;
            delete (todelete);
            size--;
            return true;
        }
        if (todelete->n != NULL && todelete->p == NULL)
        {
            // node แรก

            todelete->n->p = NULL; // ตัดตัวชี้ไปที่อื่น
            f = todelete->n;       // ย้าย f มาเป็นตัวแรก
            todelete->n = NULL;    // ตัดออก
            delete (todelete);
            size--;
            return true;
        }
        if (todelete->n == NULL && todelete->p != NULL)
        {
            todelete->p->n = NULL; // ตัดตัวลองสุดท้ายออก
            todelete->p = NULL;
            delete (todelete);
            size--;
            return true;
            // node สุดท้าย
        }
        if (todelete == f)
        {
            size = 0;
            delete (todelete);

            data = f = NULL;
            return true;
        }
        return false;
    }

    void clearjobs()
    {
        Espjob *ff = getfirst();
        while (ff != NULL)
        {
#ifdef JOBDEBUG
            Serial.printf("FREE JOB Link list: +++++ %x  next node%x \n", ff, ff->n);
#endif

            Espjob *n = ff->n;
            delete ff;
            ff = n;
            f = n;
        }
    }
    /**
     * @brief load job into linklist
     *
     * @param filename
     * @return boolean
     */
    boolean load(String filename)
    {
        if (!LittleFS.begin())
        {
            Serial.println("ERROR LITTLEFS");
            return false;
        }
        File file = LittleFS.open(filename.c_str(), "r");
        String to = file.readStringUntil('|');
#ifdef JOBDEBUG
        Serial.printf("TO: +++++ %s\n", to.c_str());
#endif

#ifdef JOBDEBUG
        Serial.println("Go loop: +++++ ");
#endif

        while (to != NULL)
        {
#ifdef JOBDEBUG
            Serial.printf("=======>%s\n", to.c_str());
#endif
            addJob(n(to));
            to = file.readStringUntil('|');
        }
        file.close();
        return true;
    }
    bool removefile(String fn)
    {

        if (!LittleFS.begin())
        {
            Serial.println("ERROR LITTLEFS");
            return false;
        }
        // File file = LittleFS.open(Fn.c_str(), "w");
        return LittleFS.remove(fn);
        // file.close();
    }
    bool savetofile(String Fn)
    {

        if (!LittleFS.begin())
        {
            Serial.println("ERROR LITTLEFS");
            return false;
        }
        File file = LittleFS.open(Fn.c_str(), "w");
        String tofile = toString();
        file.print(tofile);
        file.close();
        return true;
    }
    Foundjob *newFoudnjob(Foundjob *ff, Espjob *t)
    {
#ifdef JOBDEBUG
        if (ff != NULL)
            Serial.printf("new FROM %x  next:%x \n", ff, ff->n);
#endif

        Foundjob *nf = new Foundjob();
        nf->p = ff;
        nf->n = NULL;
        nf->job = t;

        // from = nf;

#ifdef JOBDEBUG

#endif
        return nf;
    }
    int getFoundsize()
    {
        return foundjobsize;
    }

    /**
     * @brief ใส่เวลาเข้ามาระบบจะค้นหางานที่ต้องทำในช้วงนี้
     *
     * @param t
     * @return Foundjob*
     */
    Foundjob *loadjobByt(unsigned long ctime)
    {
        foundjobsize = 0;
        Espjob *index = f;
        Foundjob *fo = NULL;
        Foundjob *fi = NULL;
        long stime;
        long etime;
        while (index != NULL)
        {

            stime = ts->getjobTimestamp(index->stime);
            etime = ts->getjobTimestamp(index->etime);
            // // Serial.printf("###### %s %s", index->stime.c_str(), index->etime.c_str());
            if (stime == etime)
            { // ไม่กำหนดเวลา
#ifdef JOBDEBUG
                Serial.println("not set time");
#endif
                if (fo == NULL)
                {
                    fo = new Foundjob();
                    fo->p = NULL;
                    fo->n = NULL;
                    fo->job = index;
                    fi = fo; // อันแรก
#ifdef JOBDEBUG
                    Serial.printf("FFF1: %x %x %x\n", fo, fo->p, fo->n);
#endif
                    foundjobsize++;
                }
                else
                {
                    Foundjob *nf = new Foundjob();
                    nf->p = fo;
                    nf->n = NULL;
                    nf->job = index;
                    fo->n = nf;
                    fo = nf;
                    foundjobsize++;
#ifdef JOBDEBUG
                    Serial.printf("FFF2: %x %x %x\n", fo, fo->p, fo->n);
#endif
                }
            }
            else
            {

#ifdef JOBDEBUG
                Serial.printf("\nid:%d  %s <=  %s <= %s \n", index->id, ts->getTimeString(stime).c_str(), ts->getTimeString(ctime).c_str(), ts->getTimeString(etime).c_str());
#endif
                if (stime <= ctime && ctime <= etime)
                {
#ifdef JOBDEBUG
                    Serial.println("IN");
#endif
                    if (fo == NULL)
                    {

                        fo = new Foundjob();
                        fo->p = NULL;
                        fo->n = NULL;
                        fo->job = index;
#ifdef JOBDEBUG
                        Serial.printf("FFF3: %x %x", fo->p, fo->n);
#endif
                        fi = fo; // อันแรก
                        foundjobsize++;
                    }
                    else
                    {
                        Foundjob *nf = new Foundjob();
                        nf->p = fo;
                        nf->n = NULL;
                        nf->job = index;
#ifdef JOBDEBUG
                        Serial.printf("FFF4: %x %x", fo->p, fo->n);
#endif
                        fo->n = nf;
                        fo = nf;
                        foundjobsize++;
                    }
                }
            }

            index = index->n;
        }
#ifdef JOBDEBUG
        Serial.print("Load TIME JOB");
        printFound("LOAD TIME JOB", fi);
#endif
        return fi;
    }
    /**
     * @brief ใส่เวลาเข้ามาระบบจะค้นหางานที่ต้องทำในช้วงนี้
     *
     * @param t
     * @return Foundjob*
     */
    Foundjob *loadjobByt()
    {

        return loadjobByt(ts->getTime());
    }
    /**
     * @brief ค้นหา  job จากการ รับ h เข้ามาถ้า hlow == hhigh ทำตลอด
     *
     * @param h
     * @return Foundjob*
     */
    Foundjob *loadjobByh(float h)
    {

        foundjobsize = 0;
        Foundjob *found = NULL;
        Foundjob *fi = NULL;
        Espjob *t = f;
        while (t != NULL)
        {

            if ((t->hlow <= h && h <= t->hhigh) || (t->hlow == t->hhigh)) // t->l == t->h ถ้าไม่ได้ set ก็ออกไปเลย
            {

                // Serial.println(t->id);
                if (found == NULL)
                {
                    found = new Foundjob();
                    found->p = found->n = NULL;
                    found->job = t;
                    fi = found; // อันแรก
                    foundjobsize++;
                }
                else
                {
                    found = newFoudnjob(found, t);
                    foundjobsize++;
                }
            }

            t = t->n;
        }

        return fi;
    }
    void freeFound(Foundjob *p)
    {
        if (p != NULL)
        {
            if (p->p != NULL && p->n != NULL)
            { // ตัวกลาง
                p->p->n = p->n;
                p->n->p = p->p;
            }
            else if (p->p == NULL && p->n != NULL)
            {
                // ตัวแรก
                p->n->p = p->p;
                p->n = NULL; // ไม่ชี้ใครแล้ว
            }
            else if (p->p != NULL && p->n == NULL)
            {
                // ตัวสุดท้าย
                p->p->n = NULL;
                p->p = NULL;
            }
        }

        delete p;
    }
    /**
     * @brief ค้นหา  job จากการ รับ h เข้ามาถ้า hlow == hhigh ทำตลอด
     * ตัวนี้จะกรองจาก by time
     * @param h
     * @return Foundjob*
     */
    Foundjob *loadjobByh(float h, Foundjob *p)
    {
        Foundjob *f = p; // backup
        Foundjob *hf = NULL;
#ifdef JOBDEBUG
        Serial.print("GET JOB FROM");
        printFound("In load by h fun:", f);
#endif
        foundjobsize = 0;
        Espjob *t;
        while (p != NULL)
        {
            t = p->job;
#ifdef JOBDEBUG
            Serial.printf("jobID:%d %f <= %f <= %f\n", t->id, t->hlow, h, t->hhigh);
#endif
            if ((t->hlow <= h && h <= t->hhigh) || (t->hlow == t->hhigh)) // t->l == t->h ถ้าไม่ได้ set ก็ออกไปเลย
            {
                foundjobsize++;
                hf = newFoudnjob(hf, t);
            }

            p = p->n;
        }

        return hf;
    }
    Foundjob *loadalljob(float h)
    {
        Foundjob *found = loadjobByt();

        Foundjob *final = loadjobByh(h, found);

        return final;
    }
    int freeFoundjobs(Foundjob *p)
    {
        int deletecount = 0;

        Foundjob *index;
        while (p != NULL)
        {
            deletecount++;
            index = p;
            p = p->n;
            freeFound(index);
        }

        return deletecount;
    }
};

#endif