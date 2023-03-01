#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
#include "Apmode.h"
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "checkconnection.h"
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
#include "Configfile.h"
#include <TM1637Display.h>
#include <ESPAsyncTCP.h>
#include "scanwifi.h"
#include <ESPAsyncWebServer.h>
#define xs 40
#define ys 15
#define pingPin D1
#define inPin D2
#define jsonbuffersize 1500
#define TMCLK D7
#define TMDIO D6
#define REALYPORT D7 // สำหรับยกน้ำออก
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
int isDisconnect = false; // สำหรับบอกสถานะว่า wifi หลุด
char jsonChar[jsonbuffersize];
long distance = 0;
// ntp

// สำหรับนับ จำนวนน้ำที่ผ่าน
#define Warterinterruppin D5
TM1637Display tm1(TMCLK, TMDIO);
SoftwareSerial mySerial(D6, D5); // RX, TX
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
String formattedDate;
String dayStamp;
String timeStamp;
int displaytmp = 0;
int oledok = 0;
int displayshtcount = 0;
int displaycounter = 0;
int checkconnectiontime = 0;
Configfile cfg("/config.cfg");

// #include <WiFiUdp.h>

const String version = "126";
RtcDS3231<TwoWire> rtcObject(Wire); // Uncomment for version 2.0.0 of the rtc library
// สำหรับบอกว่ามีการ run port io
long counttime = 0;

volatile int wateruse = 0;       // สำหรับบอกว่าใช้น้ำไปเท่าไหรแล้ว
volatile int idlewaterlimit = 0; // บอกว่าไม่มีการใช้น้ำ
KDS ds(D3);
Ktimer kt;
SSD1306Wire display(0x3c, D2, D1);
AsyncWebServer server(80);
#define ADDR 100
#define someofio 5
int canuseled = 1;
long uptime = 0;
long checkintime = 0;
long readdhttime = 0;
// run port ได้
long porttrick = 0;
long readdstime = 0;
long apmodetime = 0;
String message = "";
long reada0time = 0;
float tmpvalue = 0;
long rtctime = 0;
int ntptime = 0;
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
// StaticJsonDocument<jsonbuffersize> doc;
int wifitimeout = 0;
int makestatuscount = 0;
// boolean checkconnect();
void readSht();
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
    int D3value = OUTPUT;
    int D3initvalue = 0;
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
    boolean havetorestart = false; // สำหรับบอกว่าถ้าติดต่อ wifi ไม่ได้ให้ restart
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
    // float psi = (volts - 0.48) * 37.5; // 15 psi

    //  float volts = analog.readVolts();
    // 42.5 = 172 psi  37.5 = 150 psi 3.75 = 15psi
    // float psi = analog.readPsi(0.42, 3.75);

    int havewater = 0;
    int checkintime = 60;
    int havewaterlimit = 0;          // สำหรับ limit
    int waterlimitvalue = 0;         // สำหรับบอกยอดจำนวนเต็ม
    int waterlimittime = 300;        // 5 นาทีสำหรับหยุดแบบชุดเล็ก
    int wateridletime = 60;          // เวลาที่ไม่มีการใช้น้ำจะ หยุดนับจำนวนน้ำ
    int wateroverlimit = 3;          // ถ้าตัดเกินตามที่กำหนดให้ตัดยาวเลย
    int wateroverlimitvalue = 28800; // ตัดยาวเลย
    int readdhttime = 5;
    int readdstime = 10;
    int readshttime = 5;
    int ntpupdatetime = 600;
    int rtctimeupdate = 600;
    int readdistancetime = 60;
    int reada0time = 60;
    int otatime = 60;
    int checkconnectiontime = 600;
    int maxconnecttimeout = 10;
    int jsonbuffer = 1500;
    int checkactivetimeout = 0;
    int apmodetimeout = 600;
    String checkinurl;

} configdata;

struct
{
    unsigned int pm1 = 0;
    unsigned int pm2_5 = 0;
    unsigned int pm10 = 0;
} pmdata;
/**
 * Load config data to ram
 *
 * */

void loadconfigtoram()
{
    Serial.println("Load config to ram");
    configdata.maxconnecttimeout = cfg.getIntConfig("maxconnecttimeout", 60); // 1 for test ap mode
    configdata.checkconnectiontime = cfg.getIntConfig("checkconnectiontime", 600);
    configdata.otatime = cfg.getIntConfig("otatime", 60);
    configdata.reada0time = cfg.getIntConfig("reada0time", 60);
    configdata.readdistancetime = cfg.getIntConfig("readdistancetime", 60);
    configdata.rtctimeupdate = cfg.getIntConfig("rtctimeupdate", 600);
    configdata.ntpupdatetime = cfg.getIntConfig("ntpupdatetime", 600);
    configdata.readshttime = cfg.getIntConfig("readshttime", 60);
    configdata.readdhttime = cfg.getIntConfig("readdhttime", 600);
    configdata.readdstime = cfg.getIntConfig("readdstime", 60);
    configdata.va0 = cfg.getConfig("va0").toFloat();
    configdata.sensorvalue = cfg.getConfig("senservalue").toDouble();
    configdata.jsonbuffer = cfg.getIntConfig("jsonbuffer", 1024);
    configdata.havedht = cfg.getIntConfig("havedht", 0);
    configdata.havewater = cfg.getIntConfig("havewater", 0);
    configdata.havea0 = cfg.getIntConfig("havea0", 0);
    configdata.haveds = cfg.getIntConfig("haveds", 0);
    configdata.haveoled = cfg.getIntConfig("haveoled", 0);
    configdata.havepmsensor = cfg.getIntConfig("havepmsensor", 0);
    configdata.havertc = cfg.getIntConfig("havertc", 0);
    configdata.havesht = cfg.getIntConfig("havesht", 0);
    configdata.havetorestart = cfg.getIntConfig("havetorestart", 0);
    configdata.havesonic = cfg.getIntConfig("havesonic", 0);
    configdata.wifitimeout = cfg.getIntConfig("wifitimeout", 60);
    if (configdata.wifitimeout <= 0)
        configdata.wifitimeout = 60;
    configdata.checkintime = cfg.getIntConfig("checkintime", 600);
    if (configdata.checkintime <= 0)
        configdata.checkintime = 600;

    configdata.havewaterlimit = cfg.getIntConfig("havewaterlimit", 0);
    configdata.waterlimitvalue = cfg.getIntConfig("waterlimitvalue", 100000); // ช่วงเวลาที่ไม่เกินกำหนดสำหรับการใช้น้ำ
    configdata.wateridletime = cfg.getIntConfig("wateridletime", 60);         // เวลาที่ปั็มไม่ทำงานแล้วระบบจะถือว่าปิดการทำงานแล้ว
    configdata.wateroverlimit = cfg.getIntConfig("wateroverlimit", 3);        // เป็นจำนวนครั้งที่เกินแล้วตัดใหญ่เลย
    wateruse = 0;                                                             // reset use water

    portconfig.D3value = cfg.getIntConfig("D3mode", 1);
    portconfig.D3initvalue = cfg.getIntConfig("D3initvalue", 0);
    portconfig.D5value = cfg.getIntConfig("D5mode");
    portconfig.D5initvalue = cfg.getIntConfig("D5initvalue");
    portconfig.D6value = cfg.getIntConfig("D6mode");
    portconfig.D6initvalue = cfg.getIntConfig("D6initvalue");
    portconfig.D7value = cfg.getIntConfig("D7mode");
    portconfig.D7initvalue = cfg.getIntConfig("D7initvalue");
    portconfig.D8value = cfg.getIntConfig("D8mode");
    portconfig.D8initvalue = cfg.getIntConfig("D8initvalue");
    configdata.checkinurl = cfg.getConfig("checkinurl", "http://pi.pixka.me:3336");
    configdata.checkactivetimeout = cfg.getIntConfig("checkactivetimeout", 600);
    configdata.apmodetimeout = cfg.getIntConfig("apmodetimeout", 60);
}

// water  limit
int waterlimitime = 0;         // เป็นเวลาที่หยุดใช้น้ำ
int waterlimitport = D7;       // สำหรับตัดอ่าน ตัววัดน้ำไหลผ่าน
int currentwateroverlimit = 0; // เป็นตัวนับว่าใช้น้ำเกินกี่รอบแล้ว
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
const char configfile_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Config</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Arial; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}

#customers {
    /* font-family: 'Karla', Tahoma, Varela, Arial, Helvetica, sans-serif; */
    border-collapse: collapse;
    width: 100%%;
    /* font-size: 12px; */
}
#btn {
  border: 1px solid #777;
  background: #6e9e2d;
  color: #fff;
  font: bold 11px 'Trebuchet MS';
  padding: 4px;
  cursor: pointer;
  -moz-border-radius: 4px;
  -webkit-border-radius: 4px;
}
.button {
  background-color: #4CAF50; /* Green */
  border: none;
  color: white;
  padding: 15px 32px;
  text-align: center;
  text-decoration: none;
  display: inline-block;
  font-size: 16px;
}
#customers td,
#customers th {
    border: 1px solid #ddd;
    padding: 8px;
}


/* #customers tr:nth-child(even){background-color: #f2f2f2;} */

#customers tr:hover {
    background-color: #ddd;
}

#customers th {
    padding-top: 12px;
    padding-bottom: 12px;
    text-align: left;
    background-color: #4CAF50;
    color: white;
}
</style>

<script>
function deleteallconfig()
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/resetconfig", true); 
    xhr.send();
}
function remove(config)
{
    var xhr = new XMLHttpRequest();
    xhr.open("GET", "/removeconfig?configname="+config, true); 

     xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
     location.reload();
     }
 });
    xhr.send();
}
function add()
{
  var xhr = new XMLHttpRequest();
  var input = document.getElementById('newconfigname');
  var value = document.getElementById('newvalue');
  xhr.open("GET", "/setconfig?configname="+input.value+"&value="+value.value, true); 
  xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
     var o =  JSON.parse(xhr.responseText);
     var t = document.getElementById('customers');
     var row = t.insertRow();
     row.innerHTML = "<td>"+o.setconfig+"</td><td>"+o.value+"</td><td><input value="+o.value+"></td>";
     }
 });
  xhr.send();
}
function setvalue(element,configname,value) {
  console.log("Call",element);
  var xhr = new XMLHttpRequest();
  var input = document.getElementById(configname);

  xhr.open("GET", "/setconfig?configname="+configname+"&value="+input.value, true); 
  xhr.addEventListener("readystatechange", () => {
     console.log(xhr.readystate);
    if (xhr.readyState === 4 && xhr.status === 200) {
     console.log(xhr.responseText);
    var o =  JSON.parse(xhr.responseText);
  var showvalue = document.getElementById(configname+'value');  
  console.log('Showvalue',showvalue);
  console.log('O',o);
  showvalue.innerHTML = o.value
    } else if (xhr.readyState === 4) {
     console.log("could not fetch the data");
     }
        });
  xhr.send();
}
</script>
  </head><body>
 <table id="customers">
  <tr>
  <td>Config</td><td>value</td><td>Set</td><td>#</td><td>x</td>
  </tr>
  %CONFIG%
 </table>
<hr>
New Config <input id=newconfigname> <input id=newvalue> <button  id=btn onClick="add()">add </button>
<hr>
<button id=btn onClick="deleteallconfig()">Reset Config</button>

</body></html>)rawliteral";

int watchdog = 0;
// Portio ports[someofio];
#define b_led 2 // 1 for ESP-01, 2 for ESP-12
const char *host = "endpoint.pixka.me:5002";
char *checkinhost = "http://fw1.pixka.me:2222/checkin";
// char *otahost = "fw1.pixka.me";
const char *token = "a09f802999d3a35610d5b4a11924f8fb";
int count = 0;
// WiFiServer server(80); //กำหนดใช้งาน TCP Server ที่ Port 80
//  ESP8266WebServer server(80);
// #define ONE_WIRE_BUS D4
//  OneWire ds(D3); // on pin D4 (a 4.7K resistor is necessary)

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
// #define DHTTYPE           DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
// #define DHTTYPE           DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

// DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_AM2320 am2320 = Adafruit_AM2320();
int countsend = 0;
uint32_t delayMS;
float pfDew, pfHum, pfTemp, pfVcc;
float a0value;
float rawvalue = 0;

// สำหรับอ่านค่าน้ำ --------------
volatile int flow_frequency;
volatile int fordisplay = 0;
//----------------------------

int readshtcount = 0;
#define DHT

#ifdef DHT
DHT_Unified dht(DHTPIN, DHTTYPE);
#endif
// int ktcSO = D6;
// int ktcCS = D7;
// int ktcCLK = D5;

// MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
// ESP8266WiFiMulti WiFiMulti;
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
        // print head
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
ICACHE_RAM_ATTR void waterlimitinterrup()
{

    wateruse++;         // นับจำนวนน้ำที่ไหลผ่าน Sensor
    idlewaterlimit = 0; // บอกว่ามีการใช้อยู่
}
// สำหรับนับจำนวนน้ำที่ไหลผ่าน

ICACHE_RAM_ATTR void flow() // Interrupt function
{

    flow_frequency++;
    if (fordisplay > 0)
    {
        fordisplay--;
    }
    else
    {
        digitalWrite(D1, 0); // ปิดน้ำไม่มีการจ่ายน้ำอีกแล้ว
    }

    Serial.println(flow_frequency);
}

void displayTOTM(float d)
{
    tm1.showNumberDec(d);
}

// สำหรับจำนวนลิตรเข้ามาเพื่อเติมน้ำ
void openwater()
{
    // if (server.hasArg("w"))
    // {
    //     int watertorefill = server.arg("w").toInt();
    //     fordisplay = watertorefill / 0.0022; //จะแสดงว่าระบบจะต้องเติมน้ำเข้าไปเท่าไหร่
    //     //เปิดน้ำเปิด Sonenoi
    //     digitalWrite(D1, 1);
    //     Serial.printf("Open water %d", fordisplay);

    //     displayTOTM((fordisplay * 0.0022) + 1);
    //     char buf[255];
    //     sprintf(buf, "{\"open\": %d}", fordisplay);
    //     server.send(200, "application/json", buf);
    // }
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
    // float psi = (volts - 0.48) * 37.5; // 15 psi

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

void setwifi()
{
    // server.send(200, "text/html", index_html);
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
        delay(1000); // ถ้ามีการอ่านให้
    }
}

void setport()
{

    pinMode(D1, OUTPUT); // เป็น output
    pinMode(D2, OUTPUT);

    pinMode(D3, portconfig.D3value);
    digitalWrite(D3, portconfig.D3initvalue);
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
    ports[6].port = D3;
    ports[6].name = "D3";
    pinMode(D4, OUTPUT);
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
        dhtbuffer.count = 120; // update buffer life time
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
        dhtbuffer.count = 120; // update buffer life time
        message = "Read DHT T:" + String(pfTemp) + " H: " + String(pfHum);
    }

    readdhtstate = 0;
}
String makeStatus()
{
    cfg.setbuffer(configdata.jsonbuffer);
    DynamicJsonDocument doc(jsonbuffersize);
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
    doc["va0"] = configdata.va0;

    if (configdata.havesht)
        readSht();
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
    doc["config.checkintime"] = configdata.checkintime;
    doc["fordisplay"] = fordisplay;
    doc["wateruse"] = wateruse;
    if (configdata.havesonic)
        doc["distance"] = distance;
    else
    {
        doc["distance"] = "-1";
    }
    doc["tmp"] = tmpvalue;
    doc["D5config"] = portconfig.D5value;
    doc["D5init"] = portconfig.D5initvalue;
    doc["D3init"] = portconfig.D3initvalue;
    doc["D3config"] = portconfig.D5value;
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
    doc["datetime"] = formattedDate;
    doc["date"] = dayStamp;
    doc["time"] = timeStamp;

    if (cfg.getIntConfig("havepm25"))
    {
        doc["pm25"] = pmdata.pm2_5;
        doc["pm1"] = pmdata.pm1;
        doc["pm10"] = pmdata.pm10;
    }
    if (cfg.getIntConfig("havertc"))
    {
        RtcDateTime currentTime = rtcObject.GetDateTime(); // get the time from the RTC

        if (currentTime.Year() != 2000)
        {
            char str[20]; // declare a string as an array of chars

            sprintf(str, "%d/%d/%d %d:%d:%d", //%d allows to print an integer to the string
                    currentTime.Year(),       // get year method
                    currentTime.Month(),      // get month method
                    currentTime.Day(),        // get day method
                    currentTime.Hour(),       // get hour method
                    currentTime.Minute(),     // get minute method
                    currentTime.Second()      // get second method
            );
            doc["rtctime"] = str;
        }
    }
    doc["ntptime"] = timeClient.getFormattedTime();
    doc["ntptimelong"] = timeClient.getEpochTime();
    doc["load"] = load;
    doc["loadav"] = loadav;
    doc["timer.message"] = kt.getMessage();
    doc["checkinconnectime"] = checkconnectiontime;
    doc["otatime"] = otatime;
    // doc["ntpupdatetime"] = ntp;
    doc["readshtcount"] = readshtcount;
    doc["rtctime"] = rtctime;
    doc["readdhttime"] = readdhttime;
    doc["configdata.checkconnectiontime"] = configdata.checkconnectiontime;
    doc["configdata.checkinurl"] = configdata.checkinurl;
    char buf[jsonbuffersize];
    serializeJsonPretty(doc, buf, jsonbuffersize);
    return String(buf);
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
            if (ports[i].delay < delay)
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

    return -1;
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
void stopfill()
{
    // doc["fillwater"] = fordisplay;
    // fordisplay = 0;
    // doc["stopfill"] = "ok";
    // serializeJsonPretty(doc, jsonChar, jsonbuffersize);
    // server.send(200, "application/json", jsonChar);
}

void ota()
{
    if (oledok)
    {
        displayslot.description = "OTA";
        dd();
    }
    WiFiClient client;
    Serial.println("OTA");
    Serial.println(urlupdate);
    String u = cfg.getConfig("otaurl", "http://192.168.88.21:2000/rest/fw/update/sersor/");
    String url = u + version;

    t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
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
    Serial.println(" +++++++++++++++++++++ Check in now ++++++++++++++++++++++++++++++++++++");
    DynamicJsonDocument dy(1024);
    if (oledok)
    {
        displayslot.description = "checkin";
        dd();
    }
    dy["freemem"] = system_get_free_heap_size();
    dy["version"] = version;
    dy["name"] = name;
    dy["ip"] = WiFi.localIP().toString();
    dy["mac"] = WiFi.macAddress();
    dy["ssid"] = WiFi.SSID();
    dy["password"] = "";
    char buf[1024];
    serializeJsonPretty(dy, buf, 1024);
    Serial.println(buf);
    WiFiClient client;
    // put your main code here, to run repeatedly:
    HTTPClient http; // Declare object of class HTTPClient
    // http.begin(client, configdata.checkinurl); // Specify request destination
    http.begin(client, configdata.checkinurl); // Specify request destination
    Serial.println(configdata.checkinurl);
    http.addHeader("Content-Type", "application/json"); // Specify content-type header
    int httpCode = http.POST(buf);                      // Send the request
    String payload = http.getString();                  // Get the response payload
    Serial.print(" Http Code:");
    Serial.println(httpCode); // Print HTTP return code
    if (httpCode == 200)
    {
        DynamicJsonDocument ddd(2048);
        Serial.print(" Play load:");
        Serial.println(payload); // Print request response payload
        deserializeJson(ddd, payload);
        JsonObject obj = ddd.as<JsonObject>();
        Serial.print("---------------------------------------------------------------");
        Serial.println(obj);
        Serial.print("---------------------------------------------------------------");

        name = obj["name"].as<String>();
        if (oledok)
        {
            displayslot.foot2 = "checkin ok";
            dd();
        }
    }
    // Serial.print(" Play load:");
    // Serial.println(payload); // Print request response payload
    http.end(); // Close connection
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

void reada0()
{

    a0value = analog.readA0();
    psi = analog.readPsi(cfg.getDobuleConfig("va0"), cfg.getDobuleConfig("sensorvalue"));
    if (cfg.getIntConfig("haveoled") && otatime % 30 == 0)
    {
        Serial.println("READ A0");
        Serial.print("A0 value:");
        Serial.println(a0value);
    }
}

void readTmp()
{
    tmpvalue = ds.readDs();
}
// void KtypetoJSON()
// {
//     // doc["t"] = tmpvalue;

//     // JsonObject pidevice = doc.createNestedObject("pidevice");
//     // pidevice["mac"] = WiFi.macAddress();

//     // JsonObject ds18sensor = doc.createNestedObject("ds18sensor");
//     // ds18sensor["name"] = WiFi.macAddress();
//     // ds18sensor["callname "] = WiFi.macAddress();

//     // JsonObject device = doc.createNestedObject("device");
//     // device["mac"] = WiFi.macAddress();

//     // serializeJsonPretty(doc, jsonChar, jsonbuffersize);
//     // server.send(200, "application/json", jsonChar);
// }

void readAm2320()
{
    pfTemp = am2320.readTemperature();
    pfHum = am2320.readHumidity();
    dhtbuffer.t = pfTemp;
    dhtbuffer.h = pfHum;
    dhtbuffer.count = 120; // update buffer life time
}
void readam()
{
    readAm2320();
}
void senddata()
{

    if (WiFi.status() == WL_CONNECTED)
    { // Check WiFi connection status

        digitalWrite(b_led, 1);
        checkin();
        ota();
        digitalWrite(b_led, 0);
    }
    else
    {
        // if (!WiFi.reconnect())
        // {
        //     ESP.restart();
        // }
    }
}
void readRTC()
{
    if (cfg.getIntConfig("havertc"))
    {
        RtcDateTime currentTime = rtcObject.GetDateTime(); // get the time from the RTC

        char str[20]; // declare a string as an array of chars

        sprintf(str, "%d/%d/%d %d:%d:%d", //%d allows to print an integer to the string
                currentTime.Year(),       // get year method
                currentTime.Month(),      // get month method
                currentTime.Day(),        // get day method
                currentTime.Hour(),       // get hour method
                currentTime.Minute(),     // get minute method
                currentTime.Second()      // get second method
        );
        Serial.println(str); // print the string to the serial port

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
    displayshtcount++;
    displaytmp++;
    makestatuscount++;
    uptime++;
    if (isDisconnect)
        wifitimeout++;
    checkintime++;
    otatime++;
    readdhttime++; // บอกเวลา สำหรับอ่าน DHT
    readdstime++;
    dhtbuffer.count--;
    reada0time++;
    readshtcount++;
    porttrick++; // บอกว่า 1 วิละ
    ntptime++;
    rtctime++;
    readdistance++;
    checkconnectiontime++;
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
    // ถ้ามีการหยุดปั๊มให้
    if (waterlimitime >= 0)
        waterlimitime--;
    // else
    // {
    //     //สำหรับปิด
    //     digitalWrite(REALYPORT,0);//ปิดระบบ ตัดน้ำ
    // }

    kt.run();
}

void setAPMode()
{
    if (WiFi.status() != WL_CONNECTED)
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

void printIPAddressOfHost(const char *host)
{
    IPAddress resolvedIP;
    if (!WiFi.hostByName(host, resolvedIP))
    {
        Serial.println("DNS lookup failed.  Count..." + String(wifitimeout));
        Serial.flush();
        wifitimeout++;
        if (wifitimeout > 5 && cfg.getIntConfig("havetorestart"))
            ESP.reset();
    }
    Serial.print(host);
    Serial.print(" IP: ");
    Serial.println(resolvedIP);
}

String fillconfig(const String &var)
{
    // Serial.println(var);
    if (var == "CONFIG")
    {
        DynamicJsonDocument dy = cfg.getAll();
        JsonObject documentRoot = dy.as<JsonObject>();
        String tr = "";
        for (JsonPair keyValue : documentRoot)
        {
            String v = dy[keyValue.key()];
            String k = keyValue.key().c_str();
            tr += "<tr><td>" + k + "</td><td> <label id=" + k + "value>" + v + "</label> </td> <td> <input id = " + k + " value =\"" + v + "\"></td><td><button id=btn onClick=\"setvalue(this,'" + k + "','" + v + "')\">Set</button></td><td><button id=btn onClick=\"remove('" + k + "')\">Remove</button></td></tr>";
        }
        tr += "<tr><td>heap</td><td colspan=4>" + String(ESP.getFreeHeap()) + "</td></tr>";

        return tr;
    }
    return String();
}
void setHttp()
{

    if (WiFi.status() != WL_CONNECTED)
        return; // ออกเลยถ้าไม่ต่อ wifi
    // server.on("/dht", DHTtoJSON);
    // server.on("/pressure", PressuretoJSON);
    // server.on("/ktype", KtypetoJSON);
    // server.on("/info", info);
    // server.on("/ds18b20", readDS);
    // server.on("/run", run);
    // server.on("/a0", a0);
    // server.on("/setwifi", setwifi);
    // server.on("/scanwifi", scanwifi);

    server.on("/setconfigwww", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", configfile_html, fillconfig); });
    //-------------------------------------------------------------------------------------------------------------------------
    server.on("/resetconfig", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
        cfg.resettodefault();
        loadconfigtoram();
  request->send(200, "application/json", "{\"setconfig\":\"ok\",\"value\":\"ok\"}");
  ESP.restart(); });
    server.on("/setconfig", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
        String v = request->arg("configname");
        String value = request->arg("value");
        cfg.addConfig(v, value);
        loadconfigtoram();
  request->send(200, "application/json", "{\"setconfig\":\"" + v + "\",\"value\":\"" + value + "\"}"); });

    server.on("/removeconfig", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
        String v = request->arg("configname");
        cfg.remove(v);
        loadconfigtoram();
  request->send(200, "application/json", "{\"remove\":\"" + v + "\"}"); });
    //-------------------------------------------------------------------------------------------------------------------------
    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                DynamicJsonDocument dd  = cfg.getAll();
                char buf[jsonbuffersize];
                serializeJsonPretty(dd,buf,jsonbuffersize);
                request->send(200, "application/json", buf); });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                String status = makeStatus();
                request->send(200, "application/json", status); });
    //------------------------------------------------------------------------------------------------------------------------

    server.on("/scanwifi", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                 DynamicJsonDocument dy(jsonbuffersize);
                 char buf[jsonbuffersize];
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++)
    {
        String s = "wifi:" + String(i);
        String ss = "wifisignel:" + String(i);
        dy[s] = WiFi.SSID(i);
        dy[ss] = WiFi.RSSI(i);
    }
    serializeJsonPretty(dy, buf, jsonbuffersize);
                request->send(200, "application/json", buf); });
    //------------------------------------------------------------------------------------------------------------------------
    server.on("/run", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                Serial.println("Run");
  String p = request->arg("port");
  char b[jsonbuffersize];
  DynamicJsonDocument dy(jsonbuffersize);
  if (p.equals("test"))
  {
    message = "test port ";
    canuseled = 0;
    // doc.clear();
    dy["status"] = "ok";
    dy["port"] = p;
    dy["mac"] = WiFi.macAddress();
    dy["ip"] = WiFi.localIP().toString();
    dy["name"] = name;
    dy["uptime"] = uptime;
    dy["ntptime"] = timeClient.getFormattedTime();
    dy["ntptimelong"] = timeClient.getEpochTime();

    serializeJson(dy, b, jsonbuffersize);
    request->send(200, "application/json", b);
    for (int i = 0; i < 40; i++)
    {
      digitalWrite(2, !digitalRead(2));
      delay(200);
    }
    canuseled = 1;
    return;
  }
  String v =  request->arg("value");
  String d =  request->arg("delay");
  String w =  request->arg("wait");
  Serial.println("Port: " + p + " value : " + v + " delay: " + d);
  message = String("run port ") + String(p) + String(" value") + String(" delay ") + String(d) + " " + timeClient.getFormattedDate();
  int value = v.toInt();
  int port = getPort(p);
  if (p == NULL)
  {
    port = D5;
    v = "1";
  }
  addTorun(port, d.toInt(), v.toInt(), w.toInt());
  dy["status"] = "ok";
  dy["port"] = p;
  dy["runtime"] = d;
  dy["value"] = value;
  dy["mac"] = WiFi.macAddress();
  dy["ip"] = WiFi.localIP().toString();
  dy["uptime"] = uptime;
  serializeJson(dy, b, jsonbuffersize);
   request->send(200, "application/json", b); });
    //------------------------------------------------------------------------------------------------------------------------

    // server.on("/setconfig", setconfig);
    // server.on("/inconfigfile", allinfile);
    // server.on("/get", get);
    // server.on("/updatecheckin", updateCheckin);
    // server.on("/readam", readam);
    // server.on("/status", status);
    // server.on("/reset", reset);
    server.on("/checkin", HTTP_GET, [](AsyncWebServerRequest *request)
              { checkin();
                request->send(200, "application/json", "{\"Check in\":\"ok\"}"); });

    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        request->send(200, "application/json", "{\"reset\":\"ok\"}");
        ESP.restart(); });
    server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        request->send(200, "application/json", "{\"reset\":\"ok\"}");
        ESP.restart(); });
    //-------------------------------------------------------------------------------------------------------------------------
    // server.on("/restart", reset);
    // server.on("/setp", setValue2);
    // server.on("/update", ota);
    // server.on("/timer", runtimer);
    server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request)
              { 
                DynamicJsonDocument doc(jsonbuffersize);
                String time = request->arg("time"); //เวลาที่ทำงานงาน
                String message = request->arg("closetime");
                String l = request->arg("l");
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
        char buf[jsonbuffersize];
        serializeJsonPretty(doc, buf, jsonbuffersize);
                request->send(200, "application/json", buf); });

    if (configdata.havewater)
    {
        Serial.println("Set http for function /openwater");
        server.on("/openwater", HTTP_GET, [](AsyncWebServerRequest *request)
                  { 
    if (request->hasArg("w"))
    {
        int watertorefill = request->arg("w").toInt();
        fordisplay = watertorefill / 0.0022; //จะแสดงว่าระบบจะต้องเติมน้ำเข้าไปเท่าไหร่
        //เปิดน้ำเปิด Sonenoi
        digitalWrite(D1, 1);
        Serial.printf("Open water %d", fordisplay);
        displayTOTM((fordisplay * 0.0022) + 1);
        char buf[255];
        sprintf(buf, "{\"open\": %d}", fordisplay);
        request->send(200, "application/json", buf);
    } });
        //------------------------------------------------------------------------------------------------------------------------

        server.on("/stopfill", HTTP_GET, [](AsyncWebServerRequest *request)
                  { 
                    DynamicJsonDocument doc(jsonbuffersize);                
                    doc["fillwater"] = fordisplay;
                    fordisplay = 0;
                    doc["stopfill"] = "ok";
                    char buf[jsonbuffersize];
                    serializeJsonPretty(doc, buf, jsonbuffersize);
                    request->send(200, "application/json", buf); });
    }
    // // closetime parameter have to show on oled
    // server.on("/setclosetime", runtimer); // time parameter to count

    server.begin(); // เปิด TCP Server
    Serial.println("Server started");
    if (oledok)
    {
        displayslot.description = "Http started";
    }
}

void Apmoderun()
{

    ApMode ap("/config.cfg");
    ap.setApname("ESP Sensor AP Mode");
    ap.run();
}
int disconnecttimeout = 0;
void connect()

{

    // WiFi.mode(WIFI_STA);
    Serial.println();
    Serial.println("-----------------------------------------------");
    Serial.println(cfg.getConfig("ssid", "forpi"));
    Serial.println(cfg.getConfig("password", "04qwerty"));
    Serial.println("-----------------------------------------------");
    if (oledok)
    {
        displayslot.description = "Connect to ";
        displayslot.description1 = cfg.getConfig("ssid");
        dd();
    }
    // gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &event)
    //                                             {
    // Serial.print("Station connected, IP: ");
    // Serial.println(WiFi.localIP()); });

    // disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event)
    //                                                           { Serial.println("Station disconnected");
    //                                                             disconnecttimeout++;
    //                                                             if(disconnecttimeout > cfg.getIntConfig("disconnectiontimeout",600))
    //                                                             {
    //                                                                 ESP.restart();
    //                                                             } });
    WiFi.begin(cfg.getConfig("ssid", "forpi").c_str(), cfg.getConfig("password", "04qwerty").c_str());
    // WiFiMulti.addAP(cfg.getConfig("ssid", "forpi").c_str(), cfg.getConfig("password", "04qwerty").c_str());
    Serial.print("connect.");
    int ft = 0;
    // display.clear();
    while (WiFi.status() != WL_CONNECTED) // รอการเชื่อมต่อ
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
        if (ft > configdata.maxconnecttimeout)
        {
            Serial.println("Connect main wifi timeout");
            apmode = 1;
            break;
        }
        Serial.print(".");
    }

    if (apmode)
    {
        Apmoderun();
    }
    else
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
        }

        printIPAddressOfHost("fw1.pixka.me");
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
        // pfTemp = pfHum = 0;
        Serial.println("SHT ERROR");
        message = "Sht Error";
    }
}
void setSht()
{
    // D1,D2
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
        if (cfg.getIntConfig("havertc"))
        {
            RtcDateTime currentTime = RtcDateTime(t - 946684800); // define date and time object
            rtcObject.SetDateTime(currentTime);                   // configure the RTC with object
        }
    }
}

void setRTC()
{
    if (cfg.getIntConfig("havertc"))
        rtcObject.Begin(); // Starts I2C
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

/**
 * ใช้สำหรับ run เมื่อ สร้าง file ครั้งแรก
 * */
void initConfig()
{
    cfg.openFile();
    cfg.load();
    Serial.printf("\n***** Init config **** \n");
    delay(1000);

    cfg.addConfig("ssid", "forpi");
    cfg.addConfig("password", "04qwerty");

    cfg.addConfig("va0", 0.5);
    cfg.addConfig("sensorvalue", 42.5);
    cfg.addConfig("wifitimeout", 60);
    cfg.addConfig("checkintime", 600);

    cfg.addConfig("D5mode", OUTPUT);
    cfg.addConfig("D5initvalue", 0);
    cfg.addConfig("D6mode", OUTPUT);
    cfg.addConfig("D6initvalue", 0);
    cfg.addConfig("D7mode", OUTPUT);
    cfg.addConfig("D7initvalue", 0);
    cfg.addConfig("D8mode", OUTPUT);
    cfg.addConfig("D8initvalue", 0);

    cfg.addConfig("havedht", 0);
    cfg.addConfig("havea0", 0);
    cfg.addConfig("haveds", 0);
    cfg.addConfig("havertc", 0);
    cfg.addConfig("havertc", 0);
    cfg.addConfig("havepmsensor", 0);
    cfg.addConfig("havesht", 0);
    cfg.addConfig("havesonic", 0);
    cfg.addConfig("haveoled", 0);
    cfg.addConfig("havewaterlimit", 0);
}
void setupwater()
{
    tm1.setBrightness(0x0f);
    tm1.showNumberDec(0000);
    pinMode(Warterinterruppin, INPUT);
    pinMode(D1, OUTPUT);
    digitalWrite(D1, 0);
    attachInterrupt(Warterinterruppin, flow, RISING); // Setup Interrupt
    Serial.println("Setup Interrup  for water ...");
}
void setupntp()
{
    timeClient.begin();
    timeClient.setTimeOffset(25200); // Thailand +7 = 25200
}
void setWiFiEvent()
{
    gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP &event)
                                                {
    Serial.print("Station connected, IP: ");
    Serial.println(WiFi.localIP()); 
    isDisconnect=false; 
    wifitimeout = 0; });

    disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected &event)
                                                              { 
                                                                Serial.println("Station disconnected");
                                                               isDisconnect = true; });
}
void setup()
{

    Serial.begin(9600);

    Serial.println();
    Serial.println();
    kt.run();
    pinMode(b_led, OUTPUT); // On Board LED
                            //   pinMode(D4, OUTPUT);
                            //  pinMode(LED_BUILTIN, OUTPUT);
                            // pinMode(D6, OUTPUT);
                            // pinMode(D5, OUTPUT);
                            // pinMode(D1, OUTPUT);
                            //   pinMode(D3, OUTPUT);
                            //  digitalWrite(D3,0);
    cfg.setbuffer(configdata.jsonbuffer);
    if (!cfg.openFile())
    {
        Serial.println("Init file");
        initConfig();
    }
    loadconfigtoram();
    setport();
    if (cfg.getIntConfig("haveoled"))
    {
        setupoled();
    }
    connect();
    setupntp();
    // checkconnect();
    setHttp();

    flipper.attach(1, inden);

    if (configdata.havewater)
    {
        setupwater();
    }
    if (configdata.havedht)
    {
        dht.begin();
        readDHT();
    }

    if (configdata.havesht)
    {
        setSht();
    }

    // ถ้ามีการต่อ Pm Sensor
    if (configdata.havepmsensor)
    {
        while (!Serial)
            ;
        mySerial.begin(9600);
    }

    settime();
    ota();
    checkin();
    // setWiFiEvent();
}

void displaySht()
{
    char buf[255];
    sprintf(buf, "H:%.2f", pfHum);
    displayslot.description1 = String(buf);
    sprintf(buf, "T:%.2f", pfTemp);
    displayslot.description = String(buf);
}
void checkintask()
{

    if (checkintime > configdata.checkintime && fordisplay <= 0)
    {
        Serial.println(" +++++++++++++++++++++ Check in now ++++++++++++++++++++++++++++++++++++");
        checkintime = 0;
        checkin();
    }
}
void displaytmptask()
{
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
}
void apmodetask()
{
    if (apmodetime > configdata.apmodetimeout && fordisplay <= 0)
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
}
void checkconnectiontask()
{

    // if (wifitimeout > configdata.wifitimeout && configdata.havetorestart)
    // {
    //     ESP.restart();
    // }
    if (checkconnectiontime > configdata.checkconnectiontime)
    {
        int re = talktoServer(WiFi.localIP().toString(), name, uptime, &cfg);

        if (re != 200 && configdata.havetorestart)
        {
            ESP.restart();
        }
        else
        {
            WiFi.reconnect();
        }
        checkconnectiontime = 0;
    }
}
void otatask()
{
    if (otatime > configdata.otatime)
    {
        wifitimeout = 0;
        otatime = 0;
        ota();
        settime();
    }
}
void dhttask()
{
    if (readdhttime > configdata.readdhttime && configdata.havedht)
    {
        readdhttime = 0;
        message = "Read DHT";
        readDHT();
    }
}
void dsreadtask()
{
    if (readdstime > configdata.readdstime && configdata.haveds)
    {
        readdstime = 0;
        readTmp();
    }
}
void shtreadtask()
{
    if (configdata.havesht && readshtcount > configdata.readshttime)
    {
        Serial.println("Read sht");
        readshtcount = 0;
        readSht();

        if (displayshtcount > 20)
        {
            displayshtcount = 0;
            displaySht();
        }
    }
}
void porttask()
{
    if (porttrick > 0 && counttime >= 0)
    {
        porttrick = 0;
        portcheck();
    }
}
void ntptask()
{
    if (ntptime > configdata.ntpupdatetime)
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
}
void rtctask()
{
    if (configdata.havertc && rtctime > configdata.rtctimeupdate && fordisplay <= 0)
    {
        readRTC();
        rtctime = 0;
    }
}
void pmtask()
{
    if (configdata.havepmsensor && fordisplay <= 0)
    {
        readPm();
    }
}
void distancetask()
{
    if (configdata.havesonic && readdistance > configdata.readdistancetime && fordisplay <= 0)
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
}
void countertask()
{
    if (kt.getSec() >= 1 && displaycounter > 0 && fordisplay <= 0)
    {
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
}
void makestatustask()
{
    if (makestatuscount > 15000 && fordisplay <= 0)
    {
        makeStatus();
        makestatuscount = 0;
    }
}
void a0task()
{
    if (configdata.havea0 && reada0time > configdata.reada0time && fordisplay <= 0)
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
}
void watertask()
{
    if (configdata.havewater && displaytmp > 1)
    {
        Serial.printf("Water %d\n", fordisplay);
        displaytmp = 0;
        int a = 1;
        if (fordisplay <= 0)
            a = 0;
        displayTOTM((fordisplay * 0.0022) + a);
    }
}
void loadtask()
{
    load = millis() - s;
    loadcount++;
    loadtotal += load;
    loadav = loadtotal / loadcount;
}
void oledtask()
{
    if (oledok)
    {

        if (!ledstatus)
        {
            displayslot.foot = ".";
        }
        else
        {
            displayslot.foot = " ";
        }
        dd();
    }
}
// สำหรับตรวจสอบว่ามีการใช้น้ำเกินกำหนดเปล่า
void waterlimittask()
{
    if (configdata.havewaterlimit)
    {
        if (wateruse >= configdata.waterlimitvalue)
        {
            waterlimitime = configdata.waterlimittime; // ให้ทำการตัดปั๊มออกจากระบบ
            wateruse = 0;                              // หยุดรอไม่ใช้งานละ
            currentwateroverlimit++;                   // เพิ่มจำนวนการใช้น้ำเกินเข้าระบบ
        }

        if (waterlimitime >= 0)
        {
            digitalWrite(waterlimitport, 1); // เปิดระบบตัดน้ำแล้วระบบจะทำการลดค่า limit ไปเรื่อยๆ
        }
        else
        {
            digitalWrite(waterlimitport, 0); // ปิดถ้าค่า หยุดรอหมด
        }

        // ถ้ามีการใช้น้ำเกินกำหนดหรือว่าท่อแตกหรืออะไรซํกอย่างระบบจะตัดหรือยก relay
        if (currentwateroverlimit >= configdata.wateroverlimitvalue)
        {
            waterlimitime = configdata.waterlimittime;
            digitalWrite(REALYPORT, 1); // สั่งระบบยก relay
        }
        if (idlewaterlimit >= configdata.wateridletime && waterlimitime <= 0)
        {
            wateruse = 0;              // ไม่มีการใช้น้ำแล้ว
            currentwateroverlimit = 0; // ถ้ามีการหยุดใช้น้ำแล้วก็ยกเลิกการน้ำใช้น้ำเกิน
        }
    }
}
void loop()
{

    if (Serial.available())
    {
        char k = Serial.read();
        if (k == 'w')
        {
            scanwifi();
        }
        else if (k == 'h')
        {
            int re = talktoServer(WiFi.localIP().toString(), name, uptime, &cfg);

            Serial.printf("Result hello to %s  = %d \n", cfg.getConfig("talkurl").c_str(), re);
        }
        else if (k == 'f')
        {
            int r = WiFi.reconnect();
            Serial.printf("REconnect %s  = %d \n", cfg.getConfig("talkurl").c_str(), r);
        }
    }

    // long s = millis();
    // server.handleClient();
    checkintask();
    displaytmptask();
    apmodetask();
    checkconnectiontask();
    otatask();
    dhttask();
    dsreadtask();
    shtreadtask();
    porttask();
    ntptask();
    rtctask();
    pmtask();
    distancetask();
    countertask();
    // makestatustask();
    // loadtask();

    a0task();
    oledtask();

    watertask();
    waterlimittask();
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