#include <Arduino.h>
#include <unity.h>
#include "Configfile.h"
Configfile test("/testconfig.cfg");
void testSetConfig(void)
{

    test.addConfig("a", "x");
    String a = test.getConfig("a", "y");
    TEST_ASSERT_EQUAL_STRING("x", a.c_str());
}

void testRead(void)
{
    String a = test.getConfig("a", "y");
    TEST_ASSERT_EQUAL_STRING("x", a.c_str());
}
void setup()
{

    test.openFile();
    Serial.begin(9600);
    pinMode(2, OUTPUT);
    // NOTE!!! Wait for >2 secs
    // if board doesn't support software reset via Serial.DTR/RTS
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(testSetConfig);
    RUN_TEST(testRead);
    UNITY_END();
}

void loop()
{
    digitalWrite(2, HIGH);
    delay(500);
    digitalWrite(2, LOW);
    delay(500);
}