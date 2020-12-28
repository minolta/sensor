#include "ktimer.h"

void Ktimer::setSec(long s)
{
    sec = s;
}
void Ktimer::setMessage(String s)
{
    msg = s;
}
String Ktimer::getMessage()
{
    return msg;
}
void Ktimer::setLogic(int i)
{
    logic = i;
}
void Ktimer::start()
{
    running = true;
    pinMode(D5, OUTPUT);
    digitalWrite(D5, logic);
}
void Ktimer::stop()
{
    running = false;
    digitalWrite(D5, !digitalRead(D5));
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
