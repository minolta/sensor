#ifndef KTIMER_H
#define KTIMER_H
#include <Arduino.h>
class Ktimer
{
private:
    long sec;
    String msg;
    boolean running = false;
    int logic = 0;

public:
    void setSec(long);
    boolean run(void);
    void start();
    void stop();
    void setMessage(String);
    void setLogic(int);
    String getMessage();
    long getSec();
};
#endif