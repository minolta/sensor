#ifndef ESPJOB_H
#define ESPJOB_H
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
    String stime;
    String etime;
    Espjob *p;
    Espjob *n;
    /* data */
};

class Job
{

    Espjob *data = NULL;
    Espjob *f = NULL;
    Espjob *l = NULL;
    int size = 0;

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
            sprintf(b, "%d,%.2f,%.2f,%d,%d,%d,%d,%d,%s,%s|", t->id, t->hlow, t->hhigh, t->port, t->runtime, t->waittime, t->enable, t->out,t->stime,t->etime);
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
        int size = d.length();
        char buf[size];
        d.toCharArray(buf, size);
        Serial.println(buf);
        Espjob *t = new Espjob();
        t->p = NULL;
        t->n = NULL;
        ///          %d,%.2f,%.2f,%d,%d,%d,%d,%d
        sscanf(buf, "%d,%f,%f,%d,%d,%d,%d,%d,%s,%s",
         &t->id, &t->hlow, &t->hhigh, &t->port, &t->runtime, &t->waittime, &t->enable, &t->out,&t->stime,&t->etime);
        return t;
    }
    Espjob * newjob(int id,float hlow,float hhigh,int port,int runtime,int waittime,int enable,int out,String stime,String etime)
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
        t->stime = stime.c_str();
        t->etime = etime.c_str();
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
        if(todelete == f)
        {
            size = 0;
            delete(todelete);
            
            data = f = NULL;
            return true;
        }
        return false;
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
        Serial.printf("TO:%s", to.c_str());
        while (to != NULL)
        {
            // Serial.printf("=======>%s\n", to.c_str());
            addJob(n(to));
            to = file.readStringUntil('|');
        }
        file.close();
        return true;
    }

    bool savetofile(String Fn)
    {

        if (!LittleFS.begin())
        {
            Serial.println("ERROR LITTLEFS");
            return false;
        }
        File file = LittleFS.open(Fn.c_str(), "w");

        file.print(toString());
        file.close();
        return true;
    }
};

#endif