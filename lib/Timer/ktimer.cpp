#include "ktimer.h"

void Ktimer::setSec(long s)
{
    sec = s;
}
void Ktimer::setMessage(String s)
{
    msg = s;
}

void Ktimer::start()
{
    running = true;
    pinMode(D5, OUTPUT);
    digitalWrite(D5, 1);
}
void Ktimer::stop()
{
    running = false;
    digitalWrite(D5, 0);
}
long Ktimer::getSec()
{
    return sec;
}
boolean Ktimer::run()
{
    if (running)
    {
        sec--;

        if (sec <= 0)
        {
            stop();
            return false;
        }
        return true;
    }
    return false;
}
