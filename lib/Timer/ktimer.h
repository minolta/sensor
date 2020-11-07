#ifndef KTIMER_H
#define KTIMER_H
#include <Arduino.h>
class Ktimer
{
private:
    long sec;
    String msg;
    boolean running = false;

public:
    void setSec(long);
    boolean run(void);
    void start();
    void stop();
    void setMessage(String);
    long getSec();
};
#endif