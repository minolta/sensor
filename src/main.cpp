#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266httpUpdate.h>
#include <Wire.h>
#include "SHTSensor.h"
#include "KDs18b20.h"
#include "ktimer.h"
#include "SSD1306Wire.h"
#include "FS.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <RtcDS3231.h> //RTC library
#include <ESP8266Ping.h>
#include <Ticker.h>
#include "KAnalog.h"
#include <EEPROM.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"
#define xs 40
#define ys 15
#define pingPin D1
#define inPin D2
#define jsonbuffersize 1500
char jsonChar[jsonbuffersize];
long distance = 0;
//ntp

SoftwareSerial mySerial(D6, D5); // RX, TX
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;
int displaytmp = 0;
int oledok = 0;
int displaycounter = 0;

// #include <WiFiUdp.h>

const String version = "104";
RtcDS3231<TwoWire> rtcObject(Wire); //Uncomment for version 2.0.0 of the rtc library
//สำหรับบอกว่ามีการ run port io
long counttime = 0;
// #include "Timer.h"
// #include <max6675.h>

// #include <Q2HX711.h>

KDS ds(D3);
Ktimer kt;
SSD1306Wire display(0x3c, D2, D1);
#define ADDR 100
#define someofio 5
int canuseled = 1;
long uptime = 0;
long checkintime = 0;
long readdhttime = 0;
//run port ได้
long porttrick = 0;
long readdstime = 0;
long apmodetime = 0;
String message = "";
long reada0time = 0;
float tmpvalue = 0;
long rtctime = 0;
int ntptime = 0;
static char buf[100] = {'\0'};
long h, m, s, Y, M, d;
#define ioport 7
long load = 0;
long loadcount = 0;
double loadav = 0;
double loadtotal = 0;
double psi = 0;
int ledstatus = 0;
SHTSensor sht;
int readdistance = 0;
// int a0readcount = 0;
StaticJsonDocument<jsonbuffersize> doc;
int wifitimeout = 0;
int makestatuscount = 0;
boolean checkconnect();

class Wifidata
{
public:
    char ssid[50];
    char password[50];
};
Wifidata wifidata;

class DS18b20
{
public:
    float c;
    float f;
    float t;
};
struct
{
    int D5value = OUTPUT;
    int D5initvalue = 0;
    int D6value = OUTPUT;
    int D6initvalue = 0;
    int D7value = OUTPUT;
    int D7initvalue = 0;
    int D8value = OUTPUT;
    int D8initvalue = 0;

} portconfig;
extern "C"
{
#include "user_interface.h"
}

class Portio
{
public:
    int port;
    int value;
    int delay;
    int waittime;
    int run = 0;
    String closetime;
    String name;
    Portio *n;
    Portio *p;
};

Portio ports[ioport];
class Dhtbuffer
{
public:
    float h;
    float t;
    int count;
};
struct
{
    String save = "";
} saveconfig;
struct
{
    int readtmpvalue = 120;
    int a0readtime = 120;
    float va0 = 0.5;
    float sensorvalue = 42.5;
    boolean havedht = false;
    boolean haveds = false;
    boolean havea0 = false;
    boolean havetorestart = false; //สำหรับบอกว่าถ้าติดต่อ wifi ไม่ได้ให้ restart
    boolean havesht = false;
    boolean havertc = false;
    boolean havepmsensor = 0;
    int wifitimeout = 60;
    boolean havesonic = false;
    int haveoled = false;
    /*float volts = 3.02 * (float)sensorValue / 1023.00;
    float pressure_kPa = (volts - 0.532) / 4.0 * 1200.0;
    float pressure_psi = pressure_kPa * 0.14503773773020923;

    float psi = (volts - 0.50) * 42.5; //172/psi
    */
    // float psi = (volts - 0.433) * 3.75; // 15 psi
    //float psi = (volts - 0.48) * 37.5; // 15 psi

    //  float volts = analog.readVolts();
    // 42.5 = 172 psi  37.5 = 150 psi 3.75 = 15psi
    // float psi = analog.readPsi(0.42, 3.75);
} configdata;

struct
{
    unsigned int pm1 = 0;
    unsigned int pm2_5 = 0;
    unsigned int pm10 = 0;
} pmdata;
Dhtbuffer dhtbuffer;
long otatime = 0;
int readdhtstate = 0;
int apmode = 0;
String otahost = "fw1.pixka.me";
String type = "SENSOR";
String urlupdate = "/espupdate/nodemcu/" + version;
// OneWire  ds(D4);  // on pin D4 (a 4.7K resistor is necessary)

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP WIFI </title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    SSID: <input type="text" name="ssid">
    PASSWORD: <input type="password" name="password">
    <input type="submit" value="Submit">
  </form><br> contract ky@pixka.me 
</body></html>)rawliteral";

int watchdog = 0;
// Portio ports[someofio];
#define b_led 2 // 1 for ESP-01, 2 for ESP-12
const char *host = "endpoint.pixka.me:5002";
char *checkinhost = "http://fw1.pixka.me:2222/checkin";
// char *otahost = "fw1.pixka.me";
const char *token = "a09f802999d3a35610d5b4a11924f8fb";
int count = 0;
//WiFiServer server(80); //กำหนดใช้งาน TCP Server ที่ Port 80
ESP8266WebServer server(80);
//#define ONE_WIRE_BUS D4
// OneWire ds(D3); // on pin D4 (a 4.7K resistor is necessary)

#define DHTPIN D3 // Pin which is connected to the DHT sensor.
boolean haveportrun();
uint8_t deviceCount = 0;
float tempC;
// Timer t;
KAnalog analog;

#define cccc D6;
String name = "nodemcu";
const byte hx711_data_pin = D1;
const byte hx711_clock_pin = D2;
// Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);
// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

// DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_AM2320 am2320 = Adafruit_AM2320();
int countsend = 0;
uint32_t delayMS;
float pfDew, pfHum, pfTemp, pfVcc;
float a0value;
float rawvalue = 0;
int readshtcount = 0;
#define DHT

#ifdef DHT
DHT_Unified dht(DHTPIN, DHTTYPE);
#endif
// int ktcSO = D6;
// int ktcCS = D7;
// int ktcCLK = D5;

// MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
ESP8266WiFiMulti WiFiMulti;
float ktypevalue = 0;
Ticker flipper;

#define CONFIGADDRESS 500
#define PORTADDRESS 400
#define WIFIADDRESS 50

class Displayslot
{
public:
    String head;
    String description;
    String description1;
    String foot;
    String foot2;
} displayslot;
void dd()
{
    if (oledok)
    {
        display.clear();
        //print head
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setFont(ArialMT_Plain_16);
        display.drawString(xs + 22, ys, displayslot.head);

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(xs - 8, ys + 17, displayslot.description);

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(xs - 8, ys + 27, displayslot.description1);

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(xs - 8, ys + 39, displayslot.foot + displayslot.foot2);

        display.display();
    }
}
void saveEEPROM()
{
    Serial.println("SAVE ERRPROM");
    EEPROM.put(0, "savedeeprom");
    EEPROM.put(ADDR + WIFIADDRESS, wifidata);
    EEPROM.put(ADDR + PORTADDRESS, portconfig);
    EEPROM.put(ADDR + CONFIGADDRESS, configdata); //เอาไว้ท้ายสุด
    EEPROM.commit();
}
void readEEPROM()
{
    Serial.println("READ ERRPROM");
    EEPROM.get(ADDR + WIFIADDRESS, wifidata);
    EEPROM.get(ADDR + PORTADDRESS, portconfig);
    EEPROM.get(ADDR + CONFIGADDRESS, configdata);
}
void setEEPROM()
{

    EEPROM.begin(1000); // Use 1k for save value
    char eepromsaved[50];
    EEPROM.get(0, eepromsaved);
    Serial.print("EEPROM:");
    Serial.println(eepromsaved);
    String ee = String(eepromsaved);
    if (ee.indexOf("savedeeprom") != -1)
    {
        Serial.println("READ EEPROM----");
        readEEPROM();
        //ถ้าไม่อยู่ในช่วงนี้
        if (!(configdata.va0 > 0.1 && configdata.va0 < 1))
        {
            configdata.va0 = 0.5;
            saveEEPROM();
        }
    }
    else
    {
        Serial.println("Not save EEPROM");
        // EEPROM.put(0, "savedeeprom");
        saveEEPROM();
    }
}
void updateNTP()
{
    timeClient.update();
    formattedDate = timeClient.getFormattedDate();
    Serial.println(formattedDate);

    // Extract date
    int splitT = formattedDate.indexOf("T");
    dayStamp = formattedDate.substring(0, splitT);
    Serial.print("DATE: ");
    Serial.println(dayStamp);
    // Extract time
    timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
    Serial.print("HOUR: ");
    Serial.println(timeStamp);
}

void portcheck()
{
    for (int i = 0; i < ioport; i++)
    {
        if (ports[i].delay > 0)
        {
            ports[i].delay--;
            Serial.print("Port  ");
            Serial.println(ports[i].port);
            Serial.print(" Delay ");
            Serial.println(ports[i].delay);
            if (ports[i].delay == 0)
            {
                ports[i].run = 0;
                digitalWrite(ports[i].port, !ports[i].value);
                Serial.println("End job");
            }
        }

        if (ports[i].delay == 0)
            ports[i].run = 0;
    }
}
void readA0()
{
    //   int sensorValue = analog.readA0();

    Serial.print("ADC 10 bit = ");
    //   Serial.print(sensorValue); // print out the value you read:

    // (3.6 * val) / 4095;
    // float volts = 3.30 * (float)sensorValue / 1023.00;

    /*float volts = 3.02 * (float)sensorValue / 1023.00;
    float pressure_kPa = (volts - 0.532) / 4.0 * 1200.0;
    float pressure_psi = pressure_kPa * 0.14503773773020923;

    float psi = (volts - 0.50) * 42.5; //172/psi
    */
    // float psi = (volts - 0.433) * 3.75; // 15 psi
    //float psi = (volts - 0.48) * 37.5; // 15 psi

    //  float volts = analog.readVolts();
    // 42.5 = 172 psi  37.5 = 150 psi 3.75 = 15psi
    // float psi = analog.readPsi(0.42, 3.75);
    float psi = analog.readPsi(configdata.va0, configdata.sensorvalue);
    if (psi < 0)
        psi = 0;
    Serial.print(" , Voltage = ");
    // Serial.print(volts, 2);
    Serial.print(" V");
    Serial.print(", PSI:");
    Serial.println(psi);
    a0value = psi;
    // rawvalue = sensorValue;
}

void get()
{
    String ssd = server.arg("ssid");
    String password = server.arg("password");

    Serial.print("SSD ");
    Serial.println(ssd);
    Serial.print("password ");
    Serial.println(password);

    if (ssd != NULL)
        ssd.toCharArray(wifidata.ssid, 50);

    if (password != NULL)
        password.toCharArray(wifidata.password, 50);

    Serial.println("Set ok");

    saveEEPROM();
    String re = "<html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><h3>set WIFI TO " + ssd + " <h3><hr><a href='/setconfig'>back</a></html>";
    server.send(200, "text/html", re);
}

void setwifi()
{
    server.send(200, "text/html", index_html);
}
String intToPinmode(int i)
{
    if (i == 0)
    {
        return String("Input");
    }
    else
    {
        return String("Output");
    }
}
String intToLogic(int i)
{
    if (i == 0)
    {
        return String("Low");
    }
    else
    {
        return String("High");
    }
}
String intToEnable(int i)
{
    if (i == 0)
    {
        return String("Disable");
    }
    else
    {
        return String("Enable");
    }
}
void readPm()
{
    int index = 0;
    char value;
    char previousValue;
    int have = 0;
    while (mySerial.available())
    {
        have = 1;
        value = mySerial.read();
        if ((index == 0 && value != 0x42) || (index == 1 && value != 0x4d))
        {
            Serial.println("Cannot find the data header.");
            message = "Cannot find the data header.";
            break;
        }

        if (index == 4 || index == 6 || index == 8 || index == 10 || index == 12 || index == 14)
        {
            previousValue = value;
        }
        else if (index == 5)
        {
            pmdata.pm1 = 256 * previousValue + value;
        }
        else if (index == 7)
        {
            pmdata.pm2_5 = 256 * previousValue + value;
        }
        else if (index == 9)
        {
            pmdata.pm10 = 256 * previousValue + value;
        }
        else if (index > 15)
        {
            break;
        }
        index++;
    }

    if (have)
    {
        while (mySerial.available())
            mySerial.read();
        delay(1000); //ถ้ามีการอ่านให้
    }
}

void setconfig()
{

    // String html = " <!DOCTYPE html> <style> table { font-family: \"Trebuchet MS\", Arial, Helvetica, sans-serif; border-collapse: collapse; width: 100%; } td, th { border: 1px solid #ddd; padding: 8px; } tr:nth-child(even) { background-color: #f2f2f2; } tr:hover { background-color: #ddd; } th { padding-top: 12px; padding-bottom: 12px; text-align: left; background-color: #4CAF50; color: white; } button { /* width: 100%; */ background-color: #4CAF50; color: white; padding: 10px 15px; /* margin: 8px 0; */ border: none; border-radius: 4px; cursor: pointer; } .button3 { background-color: #f44336; } </style> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <meta charset=\"UTF-8\"> </head> Config in " + String(name) + " version:" + String(version) + " SSID:" + WiFi.SSID() + " Signel: " + String(WiFi.RSSI()) + " type: " + String(type) + "<table id=\"customres\"> <tr> <td>Parameter</td> <td>value</td> <td>Option</td> </tr> <tr> <td>DHT</td> <td>" + intToEnable(configdata.havedht) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havedht\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havedht\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\" value=\"1\">Disable</button> </form> </td> </tr> <tr> <td>SHT</td> <td>" + intToEnable(configdata.havesht) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havesht\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havesht\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>DS</td> <td>" + intToEnable(configdata.haveds) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"haveds\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"haveds\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have A0</td> <td>" + intToEnable(configdata.havea0) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havea0\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havea0\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have to restart</td> <td>" + intToEnable(configdata.havetorestart) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havetorestart\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havetorestart\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>D5</td> <td>" + intToPinmode(portconfig.D5value) + " /init " + intToLogic(portconfig.D5initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D5\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D6</td> <td>" + intToPinmode(portconfig.D6value) + " /init " + intToLogic(portconfig.D6initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D6\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D7</td> <td>" + intToPinmode(portconfig.D7value) + " /init " + intToLogic(portconfig.D7initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D7\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D8</td> <td>" + intToPinmode(portconfig.D8value) + " /init " + intToLogic(portconfig.D8initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D8\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>Sensor value </td> <td>" + String(configdata.sensorvalue) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"sensorvalue\"> <input type=\"number\" name=\"value\" value=\"0\"> <button type=\"submit\" value=\"1\">Save</button> </form> </td> </tr> <tr> <td>สำหรับปรับค่าแรงดัน VA0</td> <td>" + String(configdata.va0) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"va0\" > <input type=\"number\" name=\"value\" step='0.01' value=\"" + String(configdata.va0) + "\" > <button type=\"submit\" >Save</button> </form> </td> </tr> <tr> <td>Auto restart value </td> <td>" + String(configdata.wifitimeout) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"wifitimeout\"> <input type=\"number\" name=\"value\" value=\"0\"> <button type=\"submit\" value=\"1\">Save</button> </form> </td> </tr> </table> <hr> <form action=\"/get\"> <table> <tr> <td colspan=\"3\">WIFI information</td> </tr> <tr> <td>SSID</td> <td>" + String(wifidata.ssid) + "</td> <td><input type=\"text\" name=\"ssid\"></td> </tr> <tr> <td>PASSWORD</td> <td>********</td> <td><input type=\"password\" name=\"password\"></td> </tr> <tr> <td colspan=\"3\" align=\"right\"><button type=\"submit\">Set wifi</button></td> </tr> </table> </form> <form action=\"/restart\"> <button type=\"submit\" class=\"button3\" value=\"Restart\">Restart</button> </form> ky@pixka.me 2020 </html>";
    // String html = " <!DOCTYPE html> <style> table { font-family: \"Trebuchet MS\", Arial, Helvetica, sans-serif; border-collapse: collapse; width: 100%; } td, th { border: 1px solid #ddd; padding: 8px; } tr:nth-child(even) { background-color: #f2f2f2; } tr:hover { background-color: #ddd; } th { padding-top: 12px; padding-bottom: 12px; text-align: left; background-color: #4CAF50; color: white; } button { /* width: 100%; */ background-color: #4CAF50; color: white; padding: 10px 15px; /* margin: 8px 0; */ border: none; border-radius: 4px; cursor: pointer; } .button3 { background-color: #f44336; } </style> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <meta charset=\"UTF-8\"> </head> Config in " + String(name) + " version:" + String(version) + " SSID:" + WiFi.SSID() + " Signel: " + String(WiFi.RSSI()) + " type: " + String(type) + "<table id=\"customres\"> <tr> <td>Parameter</td> <td>value</td> <td>Option</td> </tr> <tr> <td>DHT</td> <td>" + intToEnable(configdata.havedht) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havedht\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havedht\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\" value=\"1\">Disable</button> </form> </td> </tr> <tr> <td>SHT</td> <td>" + intToEnable(configdata.havesht) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havesht\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havesht\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>DS</td> <td>" + intToEnable(configdata.haveds) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"haveds\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"haveds\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have A0</td> <td>" + intToEnable(configdata.havea0) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havea0\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havea0\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have to restart</td> <td>" + intToEnable(configdata.havetorestart) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havetorestart\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havetorestart\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have PM Sensor</td> <td>" + intToEnable(configdata.havepmsensor) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havepmsensor\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havepmsensor\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>D5</td> <td>" + intToPinmode(portconfig.D5value) + " /init " + intToLogic(portconfig.D5initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D5\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D6</td> <td>" + intToPinmode(portconfig.D6value) + " /init " + intToLogic(portconfig.D6initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D6\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D7</td> <td>" + intToPinmode(portconfig.D7value) + " /init " + intToLogic(portconfig.D7initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D7\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D8</td> <td>" + intToPinmode(portconfig.D8value) + " /init " + intToLogic(portconfig.D8initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D8\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>Sensor value </td> <td>" + String(configdata.sensorvalue) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"sensorvalue\"> <input type=\"number\" name=\"value\" value=\"0\"> <button type=\"submit\" value=\"1\">Save</button> </form> </td> </tr> <tr> <td>สำหรับปรับค่าแรงดัน VA0</td> <td>" + String(configdata.va0) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"va0\"> <input type=\"number\" name=\"value\" step='0.01' value=\"" + String(configdata.va0) + "\"> <button type=\"submit\">Save</button> </form> </td> </tr> <tr> <td>Auto restart value </td> <td>" + String(configdata.wifitimeout) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"wifitimeout\"> <input type=\"number\" name=\"value\" value=\"0\"> <button type=\"submit\" value=\"1\">Save</button> </form> </td> </tr> </table> <hr> <form action=\"/get\"> <table> <tr> <td colspan=\"3\">WIFI information</td> </tr> <tr> <td>SSID</td> <td>" + String(wifidata.ssid) + "</td> <td><input type=\"text\" name=\"ssid\"></td> </tr> <tr> <td>PASSWORD</td> <td>********</td> <td><input type=\"password\" name=\"password\"></td> </tr> <tr> <td colspan=\"3\" align=\"right\"><button type=\"submit\">Set wifi</button></td> </tr> </table> </form> <form action=\"/restart\"> <button type=\"submit\" class=\"button3\" value=\"Restart\">Restart</button> </form> ky@pixka.me 2020 </html>";

    String html = " <!DOCTYPE html> <style> table { font-family: \"Trebuchet MS\", Arial, Helvetica, sans-serif; border-collapse: collapse; width: 100%; } td, th { border: 1px solid #ddd; padding: 8px; } tr:nth-child(even) { background-color: #f2f2f2; } tr:hover { background-color: #ddd; } th { padding-top: 12px; padding-bottom: 12px; text-align: left; background-color: #4CAF50; color: white; } button { /* width: 100%; */ background-color: #4CAF50; color: white; padding: 10px 15px; /* margin: 8px 0; */ border: none; border-radius: 4px; cursor: pointer; } .button3 { background-color: #f44336; } </style> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> <meta charset=\"UTF-8\"> </head> Config in " + String(name) + " version:" + String(version) + " SSID:" + WiFi.SSID() + " Signel: " + String(WiFi.RSSI()) + " type: " + String(type) + " <table id=\"customres\"> <tr> <td>Parameter</td> <td>value</td> <td>Option</td> </tr> <tr> <td>DHT</td> <td>" + intToEnable(configdata.havedht) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havedht\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havedht\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\" value=\"1\">Disable</button> </form> </td> </tr> <tr> <td>SHT</td> <td>" + intToEnable(configdata.havesht) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havesht\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havesht\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>DS</td> <td>" + intToEnable(configdata.haveds) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"haveds\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"haveds\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Sonic</td> <td>" + intToEnable(configdata.havesonic) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havesonic\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havesonic\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have A0</td> <td>" + intToEnable(configdata.havea0) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havea0\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havea0\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have to RTC</td> <td>" + intToEnable(configdata.havertc) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havertc\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havertc\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have to restart</td> <td>" + intToEnable(configdata.havetorestart) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havetorestart\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havetorestart\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have PM Sensor</td> <td>" + intToEnable(configdata.havepmsensor) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havepmsensor\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"havepmsensor\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>Have OLED</td> <td>" + intToEnable(configdata.haveoled) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"haveoled\"> <input type=\"hidden\" name=\"value\" value=\"1\"> <button type=\"submit\" value=\"1\">Enable</button> </form> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"haveoled\"> <input type=\"hidden\" name=\"value\" value=\"0\"> <button type=\"submit\" class=\"button3\">Disable</button> </form> </td> </tr> <tr> <td>D5</td> <td>" + intToPinmode(portconfig.D5value) + " /init " + intToLogic(portconfig.D5initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D5\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D6</td> <td>" + intToPinmode(portconfig.D6value) + " /init " + intToLogic(portconfig.D6initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D6\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D7</td> <td>" + intToPinmode(portconfig.D7value) + " /init " + intToLogic(portconfig.D7initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D7\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>D8</td> <td>" + intToPinmode(portconfig.D8value) + " /init " + intToLogic(portconfig.D8initvalue) + "</td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"D8\"> <td> <select name=\"value\"> <option value=\"1\">OUTPUT</option> <option value=\"0\">INPUT</option> </select> <select name=\"value2\"> <option value=\"1\">High</option> <option value=\"0\">Low</option>s </select> <button type=\"submit\">Save</button> </td> </td> </form> </tr> <tr> <td>Sensor value </td> <td>" + String(configdata.sensorvalue) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"sensorvalue\"> <input type=\"number\" name=\"value\" value=\"0\"> <button type=\"submit\" value=\"1\">Save</button> </form> </td> </tr> <tr> <td>สำหรับปรับค่าแรงดัน VA0</td> <td>" + String(configdata.va0) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"va0\"> <input type=\"number\" name=\"value\" step='0.01' value=\"" + String(configdata.va0) + "\"> <button type=\"submit\">Save</button> </form> </td> </tr> <tr> <td>Auto restart value </td> <td>" + String(configdata.wifitimeout) + "</td> <td> <form action=\"/setp\"> <input type=\"hidden\" name=\"p\" value=\"wifitimeout\"> <input type=\"number\" name=\"value\" value=\"0\"> <button type=\"submit\" value=\"1\">Save</button> </form> </td> </tr> </table> <hr> <form action=\"/get\"> <table> <tr> <td colspan=\"3\">WIFI information</td> </tr> <tr> <td>SSID</td> <td>" + String(wifidata.ssid) + "</td> <td><input type=\"text\" name=\"ssid\"></td> </tr> <tr> <td>PASSWORD</td> <td>********</td> <td><input type=\"password\" name=\"password\"></td> </tr> <tr> <td colspan=\"3\" align=\"right\"><button type=\"submit\">Set wifi</button></td> </tr> </table> </form> <form action=\"/restart\"> <button type=\"submit\" class=\"button3\" value=\"Restart\">Restart</button> </form> ky@pixka.me 2020 </html>"; // ;
    server.send(200, "text/html", html);
}
void setReadtmplimit()
{
    String v = server.arg("value");
    if (v != NULL)
        configdata.readtmpvalue = v.toInt();
    saveEEPROM();
    server.send(200, "text/html", "OK:" + v);
}
void setReada0limit()
{
    String v = server.arg("value");
    if (v != NULL)
        configdata.a0readtime = v.toInt();
    saveEEPROM();
    server.send(200, "text/html", "OK:" + v);
}
void setport()
{

    pinMode(D1, OUTPUT);
    pinMode(D2, OUTPUT);
    pinMode(D5, portconfig.D5value);
    digitalWrite(D5, portconfig.D5initvalue);
    pinMode(D6, portconfig.D6value);
    digitalWrite(D6, portconfig.D6initvalue);

    pinMode(D7, portconfig.D7value);
    digitalWrite(D7, portconfig.D7initvalue);
    pinMode(D8, portconfig.D8value);
    digitalWrite(D8, portconfig.D8initvalue);
    ports[0].port = D5;
    ports[0].name = "D5";
    ports[1].port = D6;
    ports[1].name = "D6";
    ports[2].port = D7;
    ports[2].name = "D7";
    ports[3].port = D1;
    ports[3].name = "D1";
    ports[4].port = D2;
    ports[4].name = "D2";
    ports[5].port = D8;
    ports[5].name = "D8";
}
void readDHT()
{
    readdhtstate = 1;

    // Delay between measurements.
    // delay(delayMS);
    // Get temperature event and print its value.
    sensors_event_t event;
    dht.begin();
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature))
    {
        Serial.println("Error reading temperature!");
        message = "ERROR reading temperature";
    }
    else
    {
        Serial.print("Temperature: ");
        Serial.print(event.temperature);
        Serial.println(" *C");
        pfTemp = event.temperature;
        dhtbuffer.t = pfTemp;

        dhtbuffer.count = 120; //update buffer life time
    }
    // Get humidity event and print its value.
    dht.humidity().getEvent(&event);
    if (isnan(event.relative_humidity))
    {
        Serial.println("Error reading humidity!");
    }
    else
    {
        Serial.print("Humidity: ");
        Serial.print(event.relative_humidity);
        Serial.println("%");
        pfHum = event.relative_humidity;
        dhtbuffer.h = pfHum;
        dhtbuffer.count = 120; //update buffer life time
        message = "Read DHT T:" + String(pfTemp) + " H: " + String(pfHum);
    }

    readdhtstate = 0;
}
void makeStatus()
{
    doc.clear();
    doc["heap"] = system_get_free_heap_size();
    doc["version"] = version;
    doc["name"] = name;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["ssid"] = WiFi.SSID();
    doc["wifitimeout"] = wifitimeout;
    doc["ssid"] = WiFi.SSID();
    doc["signal"] = WiFi.RSSI();
    doc["sensorvalue"] = configdata.sensorvalue;
    doc["rawvalue"] = analog.getRawvalue();
    doc["pressurevalue"] = a0value;
    doc["psi"] = psi;
    doc["bar"] = psi / 14.504;
    doc["volts"] = analog.getReadVolts();
    doc["a0"] = a0value;
    // doc["VA0"] = configdata.va0;
    doc["va0"] = configdata.va0;
    // readDHT();
    doc["h"] = pfHum;
    doc["t"] = pfTemp;
    doc["uptime"] = uptime;
    doc["dhtbuffer.time"] = dhtbuffer.count;
    doc["type"] = type;
    doc["message"] = message;
    doc["havedht"] = configdata.havedht;
    doc["haveds"] = configdata.haveds;
    doc["havea0"] = configdata.havea0;
    doc["havetorestart"] = configdata.havetorestart;
    doc["havetosht"] = configdata.havesht;
    doc["havetortc"] = configdata.havertc;
    doc["havesonic"] = configdata.havesonic;
    doc["distance"] = distance;
    doc["config.wifitimeout"] = configdata.wifitimeout;

    if (configdata.havesonic)
        doc["distance"] = distance;
    doc["tmp"] = tmpvalue;
    //  int readtmpvalue = 120;
    // int a0readtime = 120;
    doc["reada0time"] = configdata.a0readtime;
    doc["readtmptime"] = configdata.readtmpvalue;

    doc["D5config"] = portconfig.D5value;
    doc["D5init"] = portconfig.D5initvalue;

    doc["D6config"] = portconfig.D6value;
    doc["D6init"] = portconfig.D6initvalue;
    doc["D7config"] = portconfig.D7value;
    doc["D7init"] = portconfig.D7initvalue;

    doc["d1"] = digitalRead(D1);
    doc["d2"] = digitalRead(D2);
    doc["d3"] = digitalRead(D3);
    doc["d4"] = digitalRead(D4);
    doc["d5"] = digitalRead(D5);
    doc["d6"] = digitalRead(D6);
    doc["d7"] = digitalRead(D7);
    doc["d8"] = digitalRead(D8);
    // updateNTP();
    doc["datetime"] = formattedDate;
    doc["date"] = dayStamp;
    doc["time"] = timeStamp;

    if (configdata.havepmsensor)
    {
        doc["pm25"] = pmdata.pm2_5;
        doc["pm1"] = pmdata.pm1;
        doc["pm10"] = pmdata.pm10;
    }
    if (configdata.havertc)
    {
        RtcDateTime currentTime = rtcObject.GetDateTime(); //get the time from the RTC

        if (currentTime.Year() != 2000)
        {
            char str[20]; //declare a string as an array of chars

            sprintf(str, "%d/%d/%d %d:%d:%d", //%d allows to print an integer to the string
                    currentTime.Year(),       //get year method
                    currentTime.Month(),      //get month method
                    currentTime.Day(),        //get day method
                    currentTime.Hour(),       //get hour method
                    currentTime.Minute(),     //get minute method
                    currentTime.Second()      //get second method
            );
            doc["rtctime"] = str;
        }
    }

    doc["ntptime"] = timeClient.getFormattedTime();
    doc["ntptimelong"] = timeClient.getEpochTime();
    doc["load"] = load;
    doc["loadav"] = loadav;
    doc["timer.message"] = kt.getMessage();

    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
}
void status()
{
    // makeStatus();
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    server.sendHeader("Access-Control-Allow-Headers", "application/json");
    // 'Access-Control-Allow-Headers':'application/json'
    server.send(200, "application/json", jsonChar);
}
boolean addTorun(int port, int delay, int value, int wait)
{

    if (delay > counttime)
        counttime = delay;
    for (int i = 0; i < ioport; i++)
    {
        if (ports[i].port == port)
        {

            ports[i].value = value;
            ports[i].delay = delay;
            ports[i].waittime = wait;
            ports[i].run = 1;
            digitalWrite(ports[i].port, value);
            Serial.println("Set port");
            return true;
        }
    }

    return false;
}
int getPort(String p)
{
    if (p == "D1")
    {
        return D1;
    }
    else if (p == "D2")
    {
        return D2;
    }
    else if (p == "D5")
    {
        return D5;
    }
    else if (p == "D6")
    {
        return D6;
    }
    else if (p == "D7")
    {
        return D7;
    }
    else if (p == "D8")
    {
        return D8;
    }
}

void run()
{
    Serial.println("Run");
    String p = server.arg("port");
    String v = server.arg("value");
    String d = server.arg("delay");
    String w = server.arg("wait");
    Serial.println("Port: " + p + " value : " + v + " delay: " + d);

    if (p.equals("test"))
    {
        message = "test port ";
        canuseled = 0;
        // makeStatus();
        doc["status"] = "ok";
        doc["port"] = p;
        doc["mac"] = WiFi.macAddress();
        doc["ip"] = WiFi.localIP().toString();
        // char jsonChar[jsonbuffersize];
        serializeJsonPretty(doc, jsonChar, jsonbuffersize);
        server.send(200, "application/json", jsonChar);
        for (int i = 0; i < 40; i++)
        {
            digitalWrite(2, !digitalRead(2));
            delay(200);
        }
        canuseled = 1;

        return;
    }

    int port = getPort(p);
    if (!addTorun(port, d.toInt(), v.toInt(), w.toInt()))
    {
        message = "Run Port ERROR";
        // makeStatus();
        doc["port"] = p;
        doc["value"] = v;
        doc["delay"] = d;
        doc["status"] = "error";
        // char jsonChar[jsonbuffersize];
        serializeJsonPretty(doc, jsonChar, jsonbuffersize);
        server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
        server.sendHeader("Access-Control-Allow-Headers", "application/json");
        // 'Access-Control-Allow-Headers':'application/json'
        server.send(500, "application/json", jsonChar);
    }
    else
    {
        message = "Run port ok Port:" + String(p) + " Value:" + String(v) + " Delay:" + String(d);
        // makeStatus();

        doc["port"] = p;
        doc["value"] = v;
        doc["delay"] = d;
        doc["status"] = "ok";
        // char jsonChar[jsonbuffersize];
        serializeJsonPretty(doc, jsonChar, jsonbuffersize);
        server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
        server.sendHeader("Access-Control-Allow-Headers", "application/json");
        // 'Access-Control-Allow-Headers':'application/json'
        server.send(200, "application/json", jsonChar);
    }
    // server.send(200, "application/json", "ok :" + p + " setto:" + v + " delay:" + d);
}
void updateCheckin()
{
    String s = server.arg("newurl");
    s.toCharArray(checkinhost, s.length());
}

void readDS()
{
    // DS18b20 v = DS();
    doc.clear();
    // StaticJsonDocument<jsonbuffersize> doc;
    doc["c"] = ds.readDs();
    doc["f"] = ds.readDs();
    doc["t"] = ds.readDs();
    // char jsonChar[jsonbuffersize];
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
}
long microsecondsToCentimeters(long microseconds)
{
    // The speed of sound is 340 m/s or 29 microseconds per centimeter.
    // The ping travels out and back, so to find the distance of the
    // object we take half of the distance travelled.
    return microseconds / 29 / 2;
}

long ma()
{
    long duration, cm;
    pinMode(pingPin, OUTPUT);
    digitalWrite(pingPin, LOW);
    delayMicroseconds(2);
    digitalWrite(pingPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(pingPin, LOW);
    pinMode(inPin, INPUT);
    duration = pulseIn(inPin, HIGH);
    cm = microsecondsToCentimeters(duration);
    if (oledok)
    {
        display.clear();
        display.print("distance:");
        display.println(cm);
        display.display();
    }
    return cm;
}
// void read40()
// {
//     // float v = hx711.read() / 100.0;
//     // StaticJsonDocument<500> doc;
//     // doc["value"] = v;
//     // char jsonChar[100];
//     // serializeJsonPretty(doc, jsonChar, 100);
//     // server.send(200, "application/json", jsonChar);
// }
void reset()
{
    // StaticJsonDocument<jsonbuffersize> doc;
    doc.clear();
    doc["name"] = name;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["ssid"] = WiFi.SSID();
    doc["version"] = version;
    doc["h"] = pfHum;
    doc["t"] = pfTemp;
    doc["uptime"] = uptime;
    doc["type"] = type;
    doc["reset"] = "OK";
    // char jsonChar[jsonbuffersize];
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    server.sendHeader("Access-Control-Allow-Headers", "application/json");
    // 'Access-Control-Allow-Headers':'application/json'
    server.send(200, "application/json", jsonChar);
    delay(5000);
    ESP.restart();
}
void ota()
{
    if (oledok)
    {
        displayslot.description = "OTA";
        dd();
    }
    Serial.println("OTA");
    Serial.println(urlupdate);
    t_httpUpdate_return ret = ESPhttpUpdate.update(otahost, 8080, urlupdate);
    Serial.println("return " + ret);
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
        Serial.println("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES:
        if (oledok)
        {
            displayslot.description = "No update";
            displayslot.foot2 = " No update";
            dd();
        }
        Serial.println("[update] Update no Update.");
        break;
    case HTTP_UPDATE_OK:
        if (oledok)
        {
            displayslot.description = "New FW";
            displayslot.foot2 = " New update";
            dd();
        }
        Serial.println("[update] Update ok."); // may not called we reboot the ESP
        break;
    }
}

void checkin()
{
    if (oledok)
    {
        displayslot.description = "checkin";
        dd();
    }
    doc["freemem"] = system_get_free_heap_size();
    doc["version"] = version;
    doc["name"] = name;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["ssid"] = WiFi.SSID();
    doc["password"] = "";

    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    // put your main code here, to run repeatedly:
    HTTPClient http; //Declare object of class HTTPClient

    http.begin(checkinhost); //Specify request destination
    Serial.println(checkinhost);
    http.addHeader("Content-Type", "application/json"); //Specify content-type header
    // http.addHeader("Authorization", "Basic VVNFUl9DTElFTlRfQVBQOnBhc3N3b3Jk");

    int httpCode = http.POST(jsonChar); //Send the request
    String payload = http.getString();  //Get the response payload
    Serial.print(" Http Code:");
    Serial.println(httpCode); //Print HTTP return code
    if (httpCode == 200)
    {
        Serial.print(" Play load:");
        Serial.println(payload); //Print request response payload
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();
        name = obj["pidevice"]["name"].as<String>();
        if (oledok)
        {
            displayslot.foot2 = "checkin ok";
            dd();
        }
    }

    Serial.print(" Play load:");
    Serial.println(payload); //Print request response payload

    http.end(); //Close connection
}

void writeResponse(WiFiClient &client, JsonObject &json)
{
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Access-Control-Allow-Origin: *");
    client.println("Connection: close");
    client.println();
    serializeJsonPretty(json, client);
}
bool readRequest(WiFiClient &client)
{
    bool currentLineIsBlank = true;
    while (client.connected())
    {
        if (client.available())
        {
            char c = client.read();
            if (c == '\n' && currentLineIsBlank)
            {
                return true;
            }
            else if (c == '\n')
            {
                currentLineIsBlank = true;
            }
            else if (c != '\r')
            {
                currentLineIsBlank = false;
            }
        }
    }
    return false;
}

void runCommand()
{
    String s = server.arg("port");
    String d = server.arg("delay");
    int p = getPort(s);

    String set = server.arg("set");
    digitalWrite(p, set.toInt());
    addTorun(p, d.toInt(), set.toInt(), 0);

    //StaticJsonDocument<jsonbuffersize> doc;
    doc.clear();
    doc["add"] = "ok";
    doc["port"] = s;
    doc["set"] = set;
    char jsonChar[jsonbuffersize];
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
    Serial.println("Set port " + s + " to " + set);
}

void reada0()
{

    a0value = analog.readA0();
    psi = analog.readPsi(configdata.va0, configdata.sensorvalue);
    if (configdata.haveoled && otatime % 30 == 0)
    {
        Serial.println("READ A0");
        Serial.print("A0 value:");
        Serial.println(a0value);
    }
}
void a0()
{
    // StaticJsonDocument<jsonbuffersize> doc;
    // doc.clear();
    doc["a0"] = a0value;
    JsonObject device = doc.createNestedObject("device");
    device["mac"] = WiFi.macAddress();
    // char jsonChar[jsonbuffersize];
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
}
void runtimer()
{
    String time = server.arg("time"); //เวลาที่ทำงานงาน
    String message = server.arg("closetime");
    String l = server.arg("l");
    if (oledok)
    {
        displayslot.head = "Running";
        displayslot.description = "start timer";
        displayslot.description1 = time;
        dd();
    }
    if (l != NULL)
        kt.setLogic(l.toInt());
    kt.setSec(time.toInt());
    kt.setMessage(message);
    kt.start();
    doc["runtimer"] = time;
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
}

void DHTtoJSON()
{
    //  StaticJsonDocument<jsonbuffersize> doc;
    // doc.clear();
    doc["t"] = pfTemp;
    doc["h"] = pfHum;
    // char jsonChar[jsonbuffersize];
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
}
void PressuretoJSON()
{
    digitalWrite(LED_BUILTIN, LOW);
    readA0();
    digitalWrite(LED_BUILTIN, HIGH);
    // StaticJsonDocument<jsonbuffersize> doc;
    // doc.clear();
    doc["rawvalue"] = analog.getRawvalue();
    doc["pressurevalue"] = a0value;
    doc["psi"] = a0value;
    doc["bar"] = a0value / 14.504;
    doc["volts"] = analog.getReadVolts();
    JsonObject device = doc.createNestedObject("device");
    device["mac"] = WiFi.macAddress();
    // char jsonChar[jsonbuffersize];
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
}
void readTmp()
{
    tmpvalue = ds.readDs();
}
void KtypetoJSON()
{
    // doc.clear();
    doc["t"] = tmpvalue;

    JsonObject pidevice = doc.createNestedObject("pidevice");
    pidevice["mac"] = WiFi.macAddress();

    JsonObject ds18sensor = doc.createNestedObject("ds18sensor");
    ds18sensor["name"] = WiFi.macAddress();
    ds18sensor["callname "] = WiFi.macAddress();

    JsonObject device = doc.createNestedObject("device");
    device["mac"] = WiFi.macAddress();

    // char jsonChar[jsonbuffersize];
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
}
void info()
{
    Serial.print("Mac:");
    Serial.println(WiFi.macAddress());
    Serial.print("IP:");
    Serial.println(WiFi.localIP());
    Serial.print("Version:");
    Serial.println(version);
    // doc.clear();
    doc["mac"] = WiFi.macAddress();
    doc["IP"] = WiFi.localIP().toString();
    doc["version"] = version;
    // char jsonChar[jsonbuffersize];
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
}
void readAm2320()
{
    pfTemp = am2320.readTemperature();
    pfHum = am2320.readHumidity();
    dhtbuffer.t = pfTemp;
    dhtbuffer.h = pfHum;
    dhtbuffer.count = 120; //update buffer life time
}
void readam()
{
    readAm2320();
    status();
}
void senddata()
{

    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status

        digitalWrite(b_led, 1);
        checkin();
        ota();
        digitalWrite(b_led, 0);
    }
    else
    {
        if (!WiFi.reconnect())
        {
            ESP.restart();
        }
    }

    // readKtype();
}
void readRTC()
{
    if (configdata.havertc)
    {
        RtcDateTime currentTime = rtcObject.GetDateTime(); //get the time from the RTC

        char str[20]; //declare a string as an array of chars

        sprintf(str, "%d/%d/%d %d:%d:%d", //%d allows to print an integer to the string
                currentTime.Year(),       //get year method
                currentTime.Month(),      //get month method
                currentTime.Day(),        //get day method
                currentTime.Hour(),       //get hour method
                currentTime.Minute(),     //get minute method
                currentTime.Second()      //get second method
        );
        Serial.println(str); //print the string to the serial port

        m = currentTime.Minute();
        h = currentTime.Hour();
        s = currentTime.Second();
        Y = currentTime.Year();
        M = currentTime.Month();
        d = currentTime.Day();
    }
}
void inden()
{
    displaycounter++;
    displaytmp++;
    makestatuscount++;
    uptime++;
    wifitimeout++;
    checkintime++;
    otatime++;
    readdhttime++; //บอกเวลา สำหรับอ่าน DHT
    readdstime++;
    dhtbuffer.count--;
    reada0time++;
    readshtcount++;
    porttrick++; //บอกว่า 1 วิละ
    ntptime++;
    rtctime++;
    readdistance++;
    if (apmode)
    {
        apmodetime++;
    }
    if (counttime > 0)
        counttime--;

    if (!readdhtstate && canuseled)
    {
        ledstatus = !digitalRead(b_led);
        digitalWrite(b_led, ledstatus);
    }
    kt.run();
}

void setAPMode()
{
    if (WiFiMulti.run() != WL_CONNECTED)
    {
        String mac = WiFi.macAddress();
        WiFi.softAP("ESP-Sensor" + mac, "12345678");
        IPAddress IP = WiFi.softAPIP();
        Serial.println(IP.toString());
        apmode = 1;
    }
    else
    {
    }
}
void setvalue()
{
    String v = server.arg("p");
    String value = server.arg("value");
    String value2 = server.arg("value2");
    if (v.equals("va0"))
    {
        configdata.va0 = value.toFloat();
    }
    else if (v.equals("D5"))
    {

        portconfig.D5value = value.toInt(); // mode of port
        pinMode(D5, portconfig.D5value);
        if (value2 != NULL)
        {
            portconfig.D5initvalue = value2.toInt(); //init port
            digitalWrite(D5, portconfig.D5initvalue);
        }
    }
    else if (v.equals("D8"))
    {

        portconfig.D8value = value.toInt(); // mode of port
        pinMode(D8, portconfig.D8value);
        if (value2 != NULL)
        {
            portconfig.D8initvalue = value2.toInt(); //init port
            digitalWrite(D8, portconfig.D8initvalue);
        }
    }
    else if (v.equals("wifitimeout"))
    {

        configdata.wifitimeout = value.toInt(); // mode of port
    }
    else if (v.equals("D6"))
    {
        portconfig.D6value = value.toInt();
        pinMode(D6, portconfig.D6value);
        if (value2 != NULL)
        {
            portconfig.D6initvalue = value2.toInt();
            digitalWrite(D6, portconfig.D6initvalue);
        }
    }
    else if (v.equals("D7"))
    {
        portconfig.D7value = value.toInt();
        pinMode(D7, portconfig.D7value);
        if (value2 != NULL)
        {
            portconfig.D7initvalue = value2.toInt();
            digitalWrite(D7, portconfig.D7initvalue);
        }
    }
    else if (v.equals("sensorvalue"))
    {
        configdata.sensorvalue = value.toFloat();
    }
    else if (v.equals("havedht"))
    {
        configdata.havedht = value.toInt();
    }
    else if (v.equals("havesonic"))
    {
        configdata.havesonic = value.toInt();
    }
    else if (v.equals("haveoled"))
    {
        configdata.haveoled = value.toInt();
        if (configdata.haveoled)
        {
            if (display.init())
            {
                Serial.println("Display ok");
            }
        }
    }
    else if (v.equals("havepmsensor"))
    {
        configdata.havepmsensor = value.toInt();
        if (value.toInt())
            mySerial.begin(9600);
        else
        {
            mySerial.end();
        }
    }
    else if (v.equals("haveds"))
    {
        configdata.haveds = value.toInt();
    }
    else if (v.equals("havertc"))
    {
        configdata.havertc = value.toInt();
    }
    else if (v.equals("havea0"))
    {
        configdata.havea0 = value.toInt();
    }
    else if (v.equals("havetorestart"))
    {
        configdata.havetorestart = value.toInt();
    }
    else if (v.equals("havesht"))
    {
        if (value.toInt())
        {

            if (sht.init()) //ถ้า init sht ได้ ค่อย Save จะทำให้ระบบทำงานได้
            {
                configdata.havesht = value.toInt();
            }
            else
                configdata.havesht = 0;
        }
        else
        {
            configdata.havesht = 0;
        }
    }
    saveEEPROM();
    String re = "<html> <head> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head><h3>set " + v + " TO " + value + " <h3><hr><a href='/setconfig'>back</a> <script type=\"text/JavaScript\"> redirectTime = \"1500\"; redirectURL = \"/setconfig\"; function timedRedirect() { setTimeout(\"location.href = redirectURL;\",redirectTime); } </script></html>";
    server.send(200, "text/html", re);
}

void printIPAddressOfHost(const char *host)
{
    IPAddress resolvedIP;
    if (!WiFi.hostByName(host, resolvedIP))
    {
        Serial.println("DNS lookup failed.  Count..." + String(wifitimeout));
        Serial.flush();
        wifitimeout++;
        if (wifitimeout > 5 && configdata.havetorestart)
            ESP.reset();
    }
    Serial.print(host);
    Serial.print(" IP: ");
    Serial.println(resolvedIP);
}
void scanwifi()
{
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++)
    {
        String s = "wifi:" + String(i);
        String ss = "wifisignel:" + String(i);
        doc[s] = WiFi.SSID(i);
        doc[ss] = WiFi.RSSI(i);
    }
    serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    server.send(200, "application/json", jsonChar);
}
void setHttp()
{
    server.on("/dht", DHTtoJSON);
    server.on("/pressure", PressuretoJSON);
    server.on("/ktype", KtypetoJSON);
    server.on("/info", info);
    // server.on("/read40", read40);
    server.on("/ds18b20", readDS);
    server.on("/run", run);
    server.on("/a0", a0);
    server.on("/setwifi", setwifi);
    server.on("/scanwifi", scanwifi);
    server.on("/setconfig", setconfig);
    server.on("/setreadtmp", setReadtmplimit);
    server.on("/setreada0", setReada0limit);
    server.on("/get", get);
    server.on("/updatecheckin", updateCheckin);
    server.on("/readam", readam);
    server.on("/status", status);
    server.on("/reset", reset);
    server.on("/restart", reset);
    server.on("/setp", setvalue);
    server.on("/update", ota);
    server.on("/timer", runtimer);

    //closetime parameter have to show on oled
    server.on("/setclosetime", runtimer); //time parameter to count

    server.on("/", status);
    server.begin(); //เปิด TCP Server
    Serial.println("Server started");
    if (oledok)
    {
        displayslot.description = "Http started";
    }
}
void connect()
{
    WiFi.mode(WIFI_STA);
    Serial.println();
    Serial.println("-----------------------------------------------");
    Serial.println(wifidata.ssid);
    Serial.println(wifidata.password);
    Serial.println("-----------------------------------------------");
    if (oledok)
    {
        displayslot.description = "Connect to ";
        displayslot.description1 = wifidata.ssid;
        dd();
    }
    WiFiMulti.addAP(wifidata.ssid, wifidata.password);

    int ft = 0;
    // display.clear();
    while (WiFiMulti.run() != WL_CONNECTED) //รอการเชื่อมต่อ
    {

        delay(250);
        if (oledok)
        {
            displayslot.foot = "connect";
            displayslot.foot2 = "/";
            dd();
        }
        delay(250);
        if (oledok)
        {
            displayslot.foot = "connect";
            displayslot.foot2 = "\\";
            dd();
        }

        ft++;
        if (ft > 10)
        {
            Serial.println("Connect main wifi timeout");
            apmode = 1;
            break;
        }
    }
    WiFiMulti.addAP("forpi", "04qwerty");
    WiFiMulti.addAP("forpi2", "04qwerty");
    WiFiMulti.addAP("forpi4", "04qwerty");
    WiFiMulti.addAP("forpi5", "04qwerty");
    WiFiMulti.addAP("forpi6", "04qwerty");
    WiFiMulti.addAP("forpi7", "04qwerty");
    // WiFiMulti.addAP("forpi3", "04qwerty");
    WiFiMulti.addAP("Sirifarm", "0932154741");
    WiFiMulti.addAP("test", "12345678");

    // WiFiMulti.addAP("forgame", "0894297443");
    WiFiMulti.addAP("pksy", "04qwerty");
    WiFiMulti.addAP("test", "12345678");
    // WiFiMulti.addAP("SP", "04qwerty");
    // WiFiMulti.addAP("SP3", "04qwerty");

    int co = 0;
    while (WiFiMulti.run() != WL_CONNECTED) //รอการเชื่อมต่อ
    {

        Serial.print(".");
        delay(250);
        if (oledok)
        {
            displayslot.foot = "next ssid";
            displayslot.foot2 = "+";
            dd();
        }
        delay(250);
        if (oledok)
        {
            displayslot.foot = "connect";
            displayslot.foot2 = "-";
            dd();
        }
        co++;
        if (co > 200)
        {
            setAPMode();
            break;
        }
    }
    if (WiFiMulti.run() == WL_CONNECTED)
        apmode = 0;
    if (!apmode)
    {

        Serial.println(WiFi.localIP()); // แสดงหมายเลข IP ของ Server
        String ip = WiFi.localIP().toString();
        String mac = WiFi.macAddress();
        Serial.println(mac); // แสดงหมายเลข IP ของ Server
        if (oledok)
        {
            displayslot.description = ip;
            displayslot.foot = mac;
            dd();
            delay(1000);
            // display.clear();
            // display.print("IP:");
            // display.println(ip);
            // display.print("MAC:");
            // display.println(mac);
            // display.drawLogBuffer(0, 0);
            // display.display();
        }
    }
}

void readSht()
{
    if (sht.readSample())
    {
        pfHum = sht.getHumidity();
        pfTemp = sht.getTemperature();
        message = "Read SHT H:" + String(pfHum) + " T:" + String(pfTemp);
    }
    else
    {
        pfTemp = pfHum = 0;
        Serial.println("SHT ERROR");
        message = "Sht Error";
    }
}
void setSht()
{
    //D1,D2
    Wire.begin();
    if (sht.init())
    {
        Serial.print("SHT init(): success\n");
    }
    else
    {
        Serial.print("SHT init(): failed\n");
    }
    sht.setAccuracy(SHTSensor::SHT_ACCURACY_HIGH); // only supported by SHT3x
}
void settime()
{

    if (timeClient.update())
    {
        Serial.println("Update time");
        message = "Update Time";
        long t = timeClient.getEpochTime();

        h = timeClient.getHours();
        m = timeClient.getMinutes();
        s = timeClient.getSeconds();

        Serial.print("T: ");
        Serial.println(t);
        if (configdata.havertc)
        {
            RtcDateTime currentTime = RtcDateTime(t - 946684800); //define date and time object
            rtcObject.SetDateTime(currentTime);                   //configure the RTC with object
        }
    }
}

void setRTC()
{
    if (configdata.havertc)
        rtcObject.Begin(); //Starts I2C
}

void setupoled()
{
    if (display.init())
    {
        oledok = 1;
        displayslot.head = "Sirifarm";
        displayslot.description = "start up";
        displayslot.description1 = "display ok";
        displayslot.foot = "version:";
        displayslot.foot2 = version;
        display.flipScreenVertically();
        dd();
        delay(1000);
    }
    else
    {
        Serial.println("Display not ok");
        oledok = 0;
    }
}
void setup()
{

    Serial.begin(9600);

    Serial.println();
    Serial.println();
    kt.run();
    setEEPROM();
    setport();
    if (configdata.haveoled)
    {
        setupoled();
    }
   

    pinMode(b_led, OUTPUT); //On Board LED
                            //  pinMode(D4, OUTPUT);
                            // pinMode(LED_BUILTIN, OUTPUT);
                            //pinMode(D6, OUTPUT);
                            //pinMode(D5, OUTPUT);
                            //pinMode(D1, OUTPUT);
                            //  pinMode(D3, OUTPUT);
                            // digitalWrite(D3,0);

    connect();
    timeClient.begin();
    timeClient.setTimeOffset(25200); //Thailand +7 = 25200
    checkconnect();
    setHttp();

    Serial.println(WiFi.localIP()); // แสดงหมายเลข IP ของ Server
    String mac = WiFi.macAddress();
    Serial.println(mac); // แสดงหมายเลข IP ของ Server
    // Initialize device.
    Serial.print("DNS");
    Serial.println(WiFi.dnsIP().toString());
    printIPAddressOfHost("fw1.pixka.me");
    pinMode(D4, OUTPUT);
    flipper.attach(1, inden);
    if (configdata.havedht)
    {
        dht.begin();
        readDHT();
    }

    if (configdata.havesht)
    {
        setSht();
    }

    //ถ้ามีการต่อ Pm Sensor
    if (configdata.havepmsensor)
    {
        while (!Serial)
            ;
        mySerial.begin(9600);
    }

    settime();
    ota();
    // ota();
    checkin();
}

void loop()
{
    long s = millis();
    // t.update();
    server.handleClient();
    //  delay(100);
    if (checkintime > 60)
    {
        checkintime = 0;
        checkin();
    }
    if (displaytmp > 30 && configdata.haveds)
    {
        if (oledok)
        {
            displayslot.description = "Tmp now";
            displayslot.description1 = String(tmpvalue);
            displayslot.foot2 = " tmp:" + String(tmpvalue);
            dd();
        }
        Serial.print("TMP:");
        Serial.println(tmpvalue);
        displaytmp = 0;
    }

    if (apmodetime > 120)
    {
        if (oledok)
        {
            display.clear();
            display.println("Restart");
            display.drawLogBuffer(0, 0);
            display.display();
        }
        ESP.restart();
    }
    if (otatime % 30 == 0)
    {
        if (!checkconnect())
        {
            if (oledok)
            {

                displayslot.description = "Reconnect";
                dd();
                // display.drawString(xs, ys + 10, "Reconnect");
                // display.display();
            }
            WiFi.mode(WIFI_STA);
            WiFi.disconnect();
            delay(100);
            WiFi.reconnect();
            int trytoconnect = 0;
            while (WiFiMulti.run() != WL_CONNECTED) //รอการเชื่อมต่อ
            {

                delay(500);
                Serial.print(trytoconnect);
                if (oledok)
                {
                    displayslot.description = "reconnect";
                    displayslot.description1 = String(trytoconnect);
                    dd();
                }
                trytoconnect++;
                //ต้องมี have run เพราะจะทำให้งานที่ยังทำอยู่เสร็จก่อน
                if (trytoconnect > 50 && !haveportrun())
                    ESP.restart();

                if (trytoconnect > 50)
                    break;
            }
        }
    }
    if (otatime > 60)
    {
        wifitimeout = 0;
        otatime = 0;
        ota();
        settime();
    }
    if (readdhttime > 5 && configdata.havedht)
    {
        readdhttime = 0;
        message = "Read DHT";
        readDHT();
    }
    if (readdstime > 1 && configdata.haveds)
    {
        readdstime = 0;
        readTmp();
    }
    if (configdata.havesht && readshtcount > 2)
    {
        readshtcount = 0;
        readSht();
    }
    if (porttrick > 0 && counttime >= 0)
    {
        porttrick = 0;
        portcheck();
    }
    if (ntptime > 600)
    {

        updateNTP();
        ntptime = 0;
        Serial.print("Update time:");
        Serial.println(timeClient.getFormattedTime());
        if (oledok)
        {
            displayslot.description = "update time";
            displayslot.description1 = "ok";
            dd();
        }
    }
    if (configdata.havertc && rtctime > 10)
    {
        readRTC();
        rtctime = 0;
    }
    if (configdata.havepmsensor)
    {
        readPm();
    }

    load = millis() - s;
    loadcount++;
    loadtotal += load;
    loadav = loadtotal / loadcount;

    if (configdata.havesonic && readdistance > 2)
    {
        readdistance = 0;
        distance = ma();
        if (oledok)
        {
            displayslot.description = "Distance";
            displayslot.description1 = String(distance);
            dd();
        }
    }
    if (makestatuscount > 0)
    {
        makeStatus();
        makestatuscount = 0;
    }

    if (kt.getSec() >= 1 && displaycounter > 0)
    {
        // display.setLogBuffer(1, 30);
        Serial.print("Count:");
        Serial.println(kt.getSec());
        if (oledok)
        {
            displayslot.description = kt.getMessage();
            displayslot.description1 = String(kt.getSec());
            dd();
        }

        displaycounter = 0;
    }
    if (kt.getSec() == 1)
    {
        if (oledok)
        {
            displayslot.head = "SiriFarm";
        }
    }

    if (configdata.havea0 && reada0time > 1)
    {
        reada0time = 0;
        reada0();
    }
    if (configdata.havea0 && checkintime % 30 == 0)
    {
        if (oledok)
        {
            displayslot.description = "A0";
            displayslot.description1 = String(a0value) + "PSI";
            displayslot.foot2 = String(a0value) + "PSI";
            dd();
        }
    }
    if (oledok)
    {

        if (!ledstatus)
        {
            displayslot.foot = ".";
        }
        else
        {
            // display.clear();
            displayslot.foot = " ";
        }
        dd();
    }
    // Serial.println("END loop");
}

boolean checkconnect()
{
    // IPAddress ip(192, 168, 88, 1); // The remote ip to ping
    bool ret = Ping.ping(WiFi.gatewayIP());
    if (ret)
    {
        Serial.println("Success!!");
        message = "Connect is ok";
        if (oledok)
        {
            displayslot.description = "Connect ok";
            // display.clear();
            // display.println("Connect ok");
            // display.drawLogBuffer(0, 0);
            // display.display();
        }
        return true;
    }
    else
    {
        Serial.println("Error :(");
        message = "Connect have problem";
        if (oledok)
        {
            displayslot.description = "connection error";
            // display.clear();
            // display.println("Connect ERROR");
            // display.drawLogBuffer(0, 0);
            // display.display();
        }
    }
    return false;
}

boolean haveportrun()
{
    for (int i = 0; i < ioport; i++)
    {
        if (ports[i].delay > 0)
            return true;
    }
    return false;
}