#include <Arduino.h>
#include <ArduinoJson.h>
#define JOBFILE "/j1.job"

// #include <Adafruit_Sensor.h>
// #include <DHT.h>
// #include <DHT_U.h>
// #include <OneWire.h>
#include "Apmode.h"
#include "Configfile.h"
#include "FS.h"
#include "Hjob.h"
#include "KAnalog.h"
#include "KDs18b20.h"
#include "Runjob.h"
#include "SHTSensor.h"
#include "SSD1306Wire.h"
#include "checkconnection.h"
#include "config_desc.h"
#include "config_desc_json.h"
#include "gps.h"
#include "html.h"
#include "ktimer.h"
#include "memlog.h"
#include "moveavg.h"
#include "scanwifi.h"
#include "taskservice.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266httpUpdate.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <PZEM004Tv30.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <TM1637Display.h>
#include <Ticker.h>
#include <TinyGPSPlus.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <time.h>

int timezone = 25000;
void readSoivalue();
boolean dotState = false;
// เป็นความต่างเวลาของ diff กับ timestamp
unsigned long difftimevalue = 0;
// เป็นเวลาที่รับมาครั้งสุดท้าย
unsigned long timestamp = 0;
GPS *gps;
Job *js = new Job();
KDNSServer dnsServer;
TimeService *timeservice = new TimeService();
TaskService *taskservice = new TaskService();
static const int RXPin = D7, TXPin = D8;
// The TinyGPSPlus object
// Htask *htask = new Htask();
// Hdata *hdata = new Hdata();
GPS *gpsservice = new GPS();
Htask *hservice = new Htask();
// The serial connection to the GPS device
PZEM004Tv30 pzem(&Serial);
SoftwareSerial ss(RXPin, TXPin);
const String version = "218";
boolean findsoinow = false;
void findwetair();
#define xs 40
#define ys 15
#define pingPin D1
#define inPin D2
#define jsonbuffersize 1500
#define statusJsonBufSize 2800
#define TMCLK D6
#define TMDIO D7
#define REALYPORT D7 // สำหรับยกน้ำออก
// WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
int isDisconnect = false; // สำหรับบอกสถานะว่า wifi หลุด
char jsonChar[jsonbuffersize];
static char g_statusJsonBuf[statusJsonBufSize];
long distance = 0;
unsigned long nextreadsoi = 0;
// ntp
static const uint32_t GPSBaud = 9600;
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
int readpzemtime = 0;
int gpsdisplaytime = 0;
Configfile cfg("/config.cfg");
String fulldate();
void Converttime();
// #include <WiFiUdp.h>

// RtcDS3231<TwoWire> rtcObject(Wire); // Uncomment for version 2.0.0 of the rtc
// library สำหรับบอกว่ามีการ run port io
long counttime = 0;

volatile int wateruse = 0;       // สำหรับบอกว่าใช้น้ำไปเท่าไหรแล้ว
volatile int idlewaterlimit = 0; // บอกว่าไม่มีการใช้น้ำ
void findTDE();
// KDS ds(D3);
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
String errormessage = "";
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
int readdistance = 0;
int updatetimecounter = 0;
// int a0readcount = 0;
// StaticJsonDocument<jsonbuffersize> doc;
int wifitimeout = 0;
int makestatuscount = 0;
float v;
float tde;
float firstted = 0;
float i;
float p;
float e;
float f;
float pf;
// float s;
float q;

#define drySoil 590 // Example dry value (in air)
#define wetSoil 273 // Example wet value (in cup of water)

const int soisensorPin =
    A0; // Connect the sensor's AOUT pin to Arduino Analog pin A0
int AirValue = 840;

// WaterValue: The raw sensor reading when the probe is placed in a glass of
// water. This value is treated as 100% moisture.
int WaterValue = 470;
// boolean checkconnect();
void readSht();
class Wifidata {
public:
  char ssid[50];
  char password[50];
};
Wifidata wifidata;

class DS18b20 {
public:
  float c;
  float f;
  float t;
};
struct {
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
extern "C" {
#include "user_interface.h"
}

class Portio {
public:
  int port;
  int value;
  unsigned long delay;
  unsigned long flowchecktime;
  unsigned long flowfailcount = 0; // ตัวนับสูบไม่ขึ้น
  unsigned long flowfailtime = 0;  // เวลาที่จะหยุดการทำงาน
  int waittime;
  int run = 0;
  unsigned long endtime; // เวลาที่จะหยุด run
  int defaultvalue = 0;
  String closetime;
  String name;
  Portio *n;
  Portio *p;
};

Portio ports[ioport];
class Dhtbuffer {
public:
  float h;
  float t;
  int count;
};
struct {
  String save = "";
} saveconfig;

struct {
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
  String pinghost;
  int apmodetimeout = 600;
  String checkinurl;
  String checkintoken;
  int havepzem = 0;
  int readpzemtime = 1;
  int havegps = 0;
  int stanalone = 0;
  int flowlow = 10; // การไหลของน้ำ
  unsigned long flowchecktime = 5;
  int flowfaillimit = 5;             // จับว่าน้ำไม่มีกี่ครั้งให้หยุดตามเวลาที่กำหนด
  int flowfailtime = 60;             // เวลาหยุดการการดูดน้ำก่อน
  int flowlimittims = 3;             // จำนวนครั้งที่การไหลไม่เกินกำหนด
  int flowfailcount = 3;             // จำนวนครั้งที่น้ำไม่ไหล (default 3)
  int waterlimtwaitovertimes = 1200; // เวลาหยุดดูดน้ำเมื่อใช้น้ำไม่เกินกำหนด (วินาที)
  int flowwaittimes = 1200; // เวลาหยุดดูดน้ำเมื่อน้ำไม่ไหล (วินาที, default 1200)
  int havefastport = 0;
  int fastport1status = 0;
  int fastport0status = 0;
  // บอกว่าช่วงเวลาที่ต้อง check อีกรอบเวลามีคนมาเข้าใกล้ sensor check เท่ากับ วินานว่าอยู่ใน
  // sensor นานกี่วิ
  int fastport0check = 0;
  int fastport1check = 0;
  unsigned long fastport0nextcheck = 0;
  int fastport0statustime = 0; // เวลาในการแสดงสถานะ tig
  int fastport1statustime = 0;
  unsigned long fastport0statusendtime =
      0; // เป็นเวลาที่ใช้แสดง status ของ port ว่าจะให้แสดงนานเท่าไหร่
  unsigned long fastport1statusendtime = 0;
  unsigned long fastport1nextcheck = 0;
  int fastport0;
  int fastport1;
  int fastport0time;
  int fastport1time;
  int havesoisensor;
  int updatetime = 3600;
  int havetm = 0;
  int nextreadsoi;
  String description;
  String updatetimestampurl;
  int soienablepin = D5;
} configdata;

struct {
  unsigned int pm1 = 0;
  unsigned int pm2_5 = 0;
  unsigned int pm10 = 0;
} pmdata;
/**
 * Load config data to ram
 *
 * */
int getPort(String);
/**
 * @brief Set the Timestamp object สำหรับ ปรับเวลาให้ esp เมื่อรับ timestamp มาแล้วหา
 * diff กับ mills() แล้วเก็บไว้เวลาเรียกก็ใช้บวก Diff ด้วย
 *
 * @param request
 */
void setTimestamp(AsyncWebServerRequest *request) {
  char *endptr;
  const char *c_str = request->getParam("t")->value().c_str();
  unsigned long t = strtoul(c_str, &endptr, 10);
  if (*endptr == '\0' && endptr != c_str) {
    Serial.print("Conversion successful. The unsigned long value is: ");
    Serial.println(t);
    difftimevalue = t - (millis() / 1000);
    timestamp = t;
    timeClient.setEpochTime(t);
    String re =
        "{\"message\":\"Conversion successful. The unsigned long value is: " +
        t + String("\"}");
    request->send(200, "application/json", re);
  } else
    request->send(500, "application/json", "{\"message\":\"ERROR Convert\"}");

  // หาเวลา diff เวลาเรียก time stamp จะเอา diff ไปบวกกับ millis()
  // ทำให้ได้ค่าเวลาที่จริง
}

void finddry(AsyncWebServerRequest *request) {

  int dryvalue = analogRead(soisensorPin);
  String s = "{\"airvalue\":" + String(dryvalue) + String("}");
  cfg.addConfig("airvalue", dryvalue);
  AirValue = dryvalue;
  memlogAdd(MEMLOG_SOI, dryvalue, "cal air dry");
  request->send(200, "application/json", s);

  // หาเวลา diff เวลาเรียก time stamp จะเอา diff ไปบวกกับ millis()
  // ทำให้ได้ค่าเวลาที่จริง
}
void findwet(AsyncWebServerRequest *request) {

  int wetvalue = analogRead(soisensorPin);
  String s = "{\"wetvalue\":" + String(wetvalue) + String("}");
  cfg.addConfig("wetvalue", wetvalue);
  WaterValue = wetvalue;
  memlogAdd(MEMLOG_SOI, wetvalue, "cal wet");
  request->send(200, "application/json", s);

  // หาเวลา diff เวลาเรียก time stamp จะเอา diff ไปบวกกับ millis()
  // ทำให้ได้ค่าเวลาที่จริง
}

void loadconfigtoram() {
  Serial.println("Load config to ram");
  configdata.maxconnecttimeout =
      cfg.getIntConfig("maxconnecttimeout", 60); // 1 for test ap mode
  configdata.checkconnectiontime = cfg.getIntConfig("checkconnectiontime", 600);
  configdata.otatime = cfg.getIntConfig("otatime", 600);
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
  configdata.havefastport = cfg.getIntConfig("havefastport", 0);
  configdata.wifitimeout = cfg.getIntConfig("wifitimeout", 60);
  if (configdata.wifitimeout <= 0)
    configdata.wifitimeout = 60;
  configdata.checkintime = cfg.getIntConfig("checkintime", 600);
  if (configdata.checkintime <= 0)
    configdata.checkintime = 600;

  configdata.havewaterlimit = cfg.getIntConfig("havewaterlimit", 0);
  configdata.waterlimitvalue = cfg.getIntConfig(
      "waterlimitvalue", 100000); // ช่วงเวลาที่ไม่เกินกำหนดสำหรับการใช้น้ำ
  configdata.wateridletime = cfg.getIntConfig(
      "wateridletime", 60); // เวลาที่ปั็มไม่ทำงานแล้วระบบจะถือว่าปิดการทำงานแล้ว
  configdata.wateroverlimit =
      cfg.getIntConfig("wateroverlimit", 3); // เป็นจำนวนครั้งที่เกินแล้วตัดใหญ่เลย
  configdata.waterlimittime = cfg.getIntConfig("waterlimittime", 300);
  configdata.wateroverlimitvalue =
      cfg.getIntConfig("wateroverlimitvalue", 28800);
  wateruse = 0; // reset use water

  portconfig.D3value = cfg.getIntConfig("D3mode", 0);
  portconfig.D3initvalue = cfg.getIntConfig("D3initvalue", 0);
  portconfig.D5value = cfg.getIntConfig("D5mode", 0);
  portconfig.D5initvalue = cfg.getIntConfig("D5initvalue", 0);
  portconfig.D6value = cfg.getIntConfig("D6mode", 0);
  portconfig.D6initvalue = cfg.getIntConfig("D6initvalue", 0);
  portconfig.D7value = cfg.getIntConfig("D7mode", 0);
  portconfig.D7initvalue = cfg.getIntConfig("D7initvalue", 0);
  portconfig.D8value = cfg.getIntConfig("D8mode", 0);
  portconfig.D8initvalue = cfg.getIntConfig("D8initvalue", 0);
  configdata.checkinurl =
      cfg.getConfig("checkinurl", "http://192.168.88.5:888/rest/iot/checkin");
  configdata.checkintoken = cfg.getConfig("checkintoken", "");
  configdata.checkactivetimeout = cfg.getIntConfig("checkactivetimeout", 600);
  configdata.pinghost = cfg.getConfig("pinghost", "");
  configdata.apmodetimeout = cfg.getIntConfig("apmodetimeout", 60);
  configdata.havepzem = cfg.getIntConfig("havepzem", 0);
  configdata.readpzemtime = cfg.getIntConfig("readpzemtime", 1);
  configdata.havegps = cfg.getIntConfig("havegps", 0);
  configdata.stanalone =
      cfg.getIntConfig("stanalone", 0); // บอกให้ run stan alone
  configdata.flowlow = cfg.getIntConfig("flowlow", 10);
  configdata.fastport0 = getPort(cfg.getConfig("fastport0", "D5"));
  configdata.fastport1 = getPort(cfg.getConfig("fastport1", "D6"));
  configdata.fastport0check = cfg.getIntConfig("fastport0check", 1);
  configdata.fastport1check = cfg.getIntConfig("fastport1check", 1);

  configdata.flowchecktime = cfg.getIntConfig("flowchecktime", 10);
  configdata.flowfaillimit = cfg.getIntConfig("flowfaillimit", 5); // ครั้งที่สูบไม่ขึ้น
  configdata.flowfailtime = cfg.getIntConfig("flowfailtime", 60); // เวลาหยุดสูบน้ำ
  configdata.flowlimittims = cfg.getIntConfig("flowlimittims", 3);
  configdata.flowfailcount =
      cfg.getIntConfig("flowfailcount", configdata.flowlimittims);
  configdata.waterlimtwaitovertimes =
      cfg.getIntConfig("waterlimtwaitovertimes", 1200);
  configdata.flowwaittimes =
      cfg.getIntConfig("flowwaittimes", configdata.waterlimtwaitovertimes);
  configdata.fastport0statustime = cfg.getIntConfig("fastport0statustime", 5);
  configdata.fastport1statustime = cfg.getIntConfig("fastport1statustime", 5);
  configdata.fastport0time = cfg.getIntConfig("fastport0time", 3);
  configdata.fastport1time = cfg.getIntConfig("fastport1time", 3);
  configdata.description = cfg.getConfig("description");
  configdata.updatetimestampurl =
      cfg.getConfig("updatetimestampurl", "http://192.168.88.130/timestamp");
  configdata.updatetime = cfg.getIntConfig("updatetimestamp", 3600);
  configdata.havetm = cfg.getIntConfig("havetm", 0);
  configdata.havesoisensor = cfg.getIntConfig("havesoisensor", 0);
  AirValue = cfg.getIntConfig("airvalue", 850);
  WaterValue = cfg.getIntConfig("wetvalue", 536);
  configdata.nextreadsoi = cfg.getIntConfig("nextreadsoi", 1000 * 60 * 15);
  configdata.soienablepin = getPort(cfg.getConfig("soienablepin", "D5"));
  memlogSetSlots(cfg.getIntConfig("logslots", 16));
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

int watchdog = 0;
// Portio ports[someofio];
#define b_led 2 // 1 for ESP-01, 2 for ESP-12
const char *host = "endpoint.pixka.me:5002";
char *checkinhost = "http://fw1.pixka.me:2222/checkin";
// char *otahost = "fw1.pixka.me";
int count = 0;
// WiFiServer server(80); //กำหนดใช้งาน TCP Server ที่ Port 80
//  ESP8266WebServer server(80);
// #define ONE_WIRE_BUS D4
//  OneWire ds(D3); // on pin D4 (a 4.7K resistor is necessary)

#define DHTPIN D3 // Pin which is connected to the DHT sensor.
boolean haveportrun();
uint8_t deviceCount = 0;
TimeService *ts;
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
// Adafruit_AM2320 am2320 = Adafruit_AM2320();
int countsend = 0;
uint32_t delayMS;
float pfDew, pfHum, pfTemp, pfVcc;
float a0value;
float rawvalue = 0;

// สำหรับอ่านค่าน้ำ --------------

volatile int flow_frequency;
volatile unsigned long totalflow_frequency;
volatile int fordisplay = 0;
//----------------------------

int readshtcount = 0;
#define DHT

#ifdef DHT
// DHT_Unified dht(DHTPIN, DHTTYPE);
#endif
// int ktcSO = D6;
// int ktcCS = D7;
// int ktcCLK = D5;

// MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
static ESP8266WiFiMulti wifiMultiObj;
static ESP8266WiFiMulti *wifiMulti = &wifiMultiObj;
static const int MAX_WIFI_NETWORKS = 5;
static void loadWifiMultiFromConfig();
static void resetWifiMulti();
static String trimConfig(const String &s);
static bool connectOneWifi(const String &ssid, const String &pass,
                           int timeoutSec);
static bool connectAllConfiguredWifi();
Runjob *runservice;
float ktypevalue = 0;
Ticker flipper;

#define CONFIGADDRESS 500
#define PORTADDRESS 400
#define WIFIADDRESS 50

class Displayslot {
public:
  String head;
  String description;
  String description1;
  String foot;
  String foot2;
} displayslot;
void dd() {
  if (oledok) {
    display.clear();
    // print head
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);
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
ICACHE_RAM_ATTR void waterlimitinterrup() {

  wateruse++;         // นับจำนวนน้ำที่ไหลผ่าน Sensor
  idlewaterlimit = 0; // บอกว่ามีการใช้อยู่
}
// สำหรับนับจำนวนน้ำที่ไหลผ่าน

ICACHE_RAM_ATTR void flow() // Interrupt function
{

  flow_frequency++;
  totalflow_frequency++;
  // Serial.println(flow_frequency);
}

void displayTOTM(float d) { tm1.showNumberDec(d); }

// สำหรับจำนวนลิตรเข้ามาเพื่อเติมน้ำ
void openwater() {
  // if (server.hasArg("w"))
  // {
  //     int watertorefill = server.arg("w").toInt();
  //     fordisplay = watertorefill / 0.0022;
  //     //จะแสดงว่าระบบจะต้องเติมน้ำเข้าไปเท่าไหร่
  //     //เปิดน้ำเปิด Sonenoi
  //     digitalWrite(D1, 1);
  //     Serial.printf("Open water %d", fordisplay);

  //     displayTOTM((fordisplay * 0.0022) + 1);
  //     char buf[255];
  //     sprintf(buf, "{\"open\": %d}", fordisplay);
  //     server.send(200, "application/json", buf);
  // }
}
void updateRTC() {
  if (configdata.havertc) {
    // long t = timeClient.getEpochTime();
    // Serial.println(t);
    // // 946659600 = timestamp sine 2000 - 1 -1 - 0:0:0 - 25200 GTM+7
    // RtcDateTime manual = RtcDateTime(t - 946659600 - 25200);
    // rtcObject.SetDateTime(manual);
  }
}
int getPort(String p) {
  if (p == "D1") {
    return D1;
  } else if (p == "D2") {
    return D2;
  } else if (p == "D5") {
    return D5;
  } else if (p == "D6") {
    return D6;
  } else if (p == "D7") {
    return D7;
  } else if (p == "D8") {
    return D8;
  } else if (p == "D4") {
    return D4;
  }

  return -1;
}
void updateNTP() {
  if (timeClient.update()) {

    if (timeservice != NULL) {
      timeservice->setTime(timeClient.getEpochTime());
    }
    if (gps != NULL) {
      gps->setTime(timeClient.getEpochTime());
    }
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
    updateRTC();
  }
}

void portcheck() {

  for (int i = 0; i < ioport; i++) {

    unsigned long t = millis();
    if (configdata.havewater) {
      if (ports[i].run == 1 && ports[i].flowchecktime <= t) {
        memlogTask("flow", i);
        if (flow_frequency <=
            configdata.flowlow) // ดูค่าใน flow_frequency
                                // น้อยกว่าที่กำหนดหรือเปล่าถ้าน้อยปิดระบบเลย
        {
          // ถ่าไม่มีการไหลของน้ำเลยให้หยุด port เลย flow ต่อกับ D6
          ports[i].run = 0;
          ports[i].endtime = 0;
          digitalWrite(ports[i].port, ports[i].defaultvalue);
          ports[i].flowfailcount++;
          int limitTims =
              configdata.flowfailcount > 0
                  ? configdata.flowfailcount
                  : (configdata.flowlimittims > 0 ? configdata.flowlimittims
                                                  : 3);
          int waitOverSec = configdata.flowwaittimes > 0
                                ? configdata.flowwaittimes
                                : (configdata.waterlimtwaitovertimes > 0
                                       ? configdata.waterlimtwaitovertimes
                                       : 1200);
          if (ports[i].flowfailcount >= limitTims) {
            errormessage = "Flow fail count is  " +
                           String(ports[i].flowfailcount) + " spend " +
                           String(waitOverSec);
            memlogAdd(MEMLOG_ERROR, ports[i].flowfailcount, errormessage);
            ports[i].flowfailtime =
                t + ((unsigned long)waitOverSec * 1000); // กำหนดเวลาหยุดทำงาน
            ports[i].flowfailcount = 0; // reset count for next attempt
          } else {
            message = "Open pump but no flow off pump";
            errormessage = "Have flow " + String(flow_frequency) + " < " +
                           String(configdata.flowlow) + " off pump ";
            memlogAdd(MEMLOG_ERROR, flow_frequency, errormessage);
            Serial.println(message);
            flow_frequency = 0;
          }
        } else {
          message = "Have flow " + String(flow_frequency) + " > " +
                    String(configdata.flowlow) + " runok set next check ";
          errormessage = "";
          flow_frequency = 0;

          ports[i].flowfailcount = 0; // ถ้าน้ำมาแล้ว reset ใหม่
          ports[i].flowfailtime = 0;  // ถ้าน้ำมาแล้ว reset ใหม่

          ports[i].flowchecktime =
              t + (configdata.flowchecktime * 1000); // ปรับเวลาตรวจสอบรอบหน้า

          Serial.println(message);
        }
      }
    }
    // end job

    if (ports[i].run == 1 && ports[i].endtime <= t) {
      Serial.println("Check port");
      Serial.print("Port  ");
      Serial.println(ports[i].port);
      Serial.print(" Delay ");
      Serial.println(ports[i].delay);
      ports[i].run = 0;
      ports[i].endtime = 0;
      digitalWrite(ports[i].port, ports[i].defaultvalue);
      memlogTask("port", ports[i].port);
      Serial.println("End job");
    }
  }
}
void readA0() {
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
void updateTime() {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, configdata.updatetimestampurl);

  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.println("HTTP Response Code: " + String(httpResponseCode));
    Serial.println("Received JSON:");
    Serial.println(payload);

    DynamicJsonDocument ddd(200);
    Serial.print(" Play load:");
    deserializeJson(ddd, payload);
    String tt = ddd["timestamp"].as<String>();
    // --- Parse the JSON ---

    char *endptr;
    unsigned long number = strtoul(tt.c_str(), &endptr, 10);

    if (*endptr == '\0') {
      Serial.print("Conversion successful: ");
      Serial.println(number);
      timestamp = number;
      difftimevalue = number - (millis() / 1000);
      timeClient.setEpochTime(number);
    } else {
      Serial.print("Conversion failed. Found non-numeric character: ");
      Serial.println(*endptr);
    }
  } else {
    Serial.print("HTTP request failed, error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
void setwifi() {
  // server.send(200, "text/html", index_html);
}
String intToPinmode(int i) {
  if (i == 0) {
    return String("Input");
  } else {
    return String("Output");
  }
}
String intToLogic(int i) {
  if (i == 0) {
    return String("Low");
  } else {
    return String("High");
  }
}
String intToEnable(int i) {
  if (i == 0) {
    return String("Disable");
  } else {
    return String("Enable");
  }
}
void readPm() {
  Serial.println("Update PM");
  int index = 0;
  char value;
  char previousValue;
  int have = 0;
  while (mySerial.available()) {
    have = 1;
    value = mySerial.read();
    if ((index == 0 && value != 0x42) || (index == 1 && value != 0x4d)) {
      Serial.println("Cannot find the data header.");
      message = "Cannot find the data header.";
      break;
    }

    if (index == 4 || index == 6 || index == 8 || index == 10 || index == 12 ||
        index == 14) {
      previousValue = value;
    } else if (index == 5) {
      pmdata.pm1 = 256 * previousValue + value;
    } else if (index == 7) {
      pmdata.pm2_5 = 256 * previousValue + value;
    } else if (index == 9) {
      pmdata.pm10 = 256 * previousValue + value;
    } else if (index > 15) {
      break;
    }
    index++;
  }

  if (have) {
    memlogTask("pm", pmdata.pm2_5);
    while (mySerial.available())
      mySerial.read();
    delay(1000); // ถ้ามีการอ่านให้
  }
}

void setport() {

  pinMode(D1, OUTPUT); // เป็น output
  pinMode(D2, OUTPUT);

  pinMode(D3, portconfig.D3value);
  digitalWrite(D3, portconfig.D3initvalue);
  if (portconfig.D5value == 0)
    pinMode(D5, INPUT_PULLUP);
  else {
    pinMode(D5, portconfig.D5value);
    digitalWrite(D5, portconfig.D5initvalue);
  }
  if (portconfig.D6value == 0)
    pinMode(D6, INPUT_PULLUP);
  else {
    pinMode(D6, portconfig.D6value);
    digitalWrite(D6, portconfig.D6initvalue);
  }
  if (portconfig.D7value == 0)
    pinMode(D7, INPUT_PULLUP);
  else {
    pinMode(D7, portconfig.D7value);
    digitalWrite(D7, portconfig.D7initvalue);
  }
  pinMode(D8, portconfig.D8value);
  digitalWrite(D8, portconfig.D8initvalue);
  ports[0].port = D5;
  ports[0].name = "D5";
  ports[0].defaultvalue = portconfig.D5initvalue;
  ports[1].port = D6;
  ports[1].name = "D6";
  ports[1].defaultvalue = portconfig.D6initvalue;
  ports[2].port = D7;
  ports[2].name = "D7";
  ports[2].defaultvalue = portconfig.D7initvalue;
  ports[3].port = D1;
  ports[3].name = "D1";
  ports[3].defaultvalue = portconfig.D3initvalue;
  ports[4].port = D2;
  ports[4].name = "D2";
  //  ports[4].defaultvalue = portconfig.D2initvalue;
  ports[5].port = D8;
  ports[5].name = "D8";
  ports[5].defaultvalue = portconfig.D8initvalue;
  ports[6].port = D3;
  ports[6].name = "D3";
  ports[6].defaultvalue = portconfig.D6initvalue;
  pinMode(D4, OUTPUT);
}
void readDHT() {
  // readdhtstate = 1;

  // // Delay between measurements.
  // // delay(delayMS);
  // // Get temperature event and print its value.
  // sensors_event_t event;
  // dht.begin();
  // dht.temperature().getEvent(&event);
  // if (isnan(event.temperature))
  // {
  //     Serial.println("Error reading temperature!");
  //     message = "ERROR reading temperature";
  // }
  // else
  // {
  //     Serial.print("Temperature: ");
  //     Serial.print(event.temperature);
  //     Serial.println(" *C");
  //     pfTemp = event.temperature;
  //     dhtbuffer.t = pfTemp;
  //     dhtbuffer.count = 120; // update buffer life time
  // }
  // // Get humidity event and print its value.
  // dht.humidity().getEvent(&event);
  // if (isnan(event.relative_humidity))
  // {
  //     Serial.println("Error reading humidity!");
  // }
  // else
  // {
  //     Serial.print("Humidity: ");
  //     Serial.print(event.relative_humidity);
  //     Serial.println("%");
  //     pfHum = event.relative_humidity;
  //     dhtbuffer.h = pfHum;
  //     dhtbuffer.count = 120; // update buffer life time
  //     message = "Read DHT T:" + String(pfTemp) + " H: " + String(pfHum);
  // }

  // readdhtstate = 0;
}
time_t realtime() {
  return (time_t)(millis() / 1000) + difftimevalue + timezone;
}
size_t makeStatusJson(char *buf, size_t cap);
String makeStatus() {
  makeStatusJson(g_statusJsonBuf, sizeof(g_statusJsonBuf));
  return String(g_statusJsonBuf);
}

size_t makeStatusJson(char *buf, size_t cap) {
  if (buf == nullptr || cap < 128)
    return 0;

  const int buffersize = (cap > 2600) ? 2600 : (int)cap - 1;
  cfg.setbuffer(configdata.jsonbuffer);
  DynamicJsonDocument doc(buffersize);
  doc["description"] = configdata.description;
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

  // if (configdata.havesht)
  //     readSht();
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
  doc["havefastport"] = configdata.havefastport;
  doc["distance"] = distance;
  doc["config.wifitimeout"] = configdata.wifitimeout;
  doc["config.checkintime"] = configdata.checkintime;
  doc["fordisplay"] = fordisplay;
  doc["wateruse"] = wateruse;
  if (configdata.havesonic)
    doc["distance"] = distance;
  else {
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
  doc["fastport0status"] = configdata.fastport0status;
  doc["fastport1status"] = configdata.fastport1status;
  doc["fulltime"] = fulldate();
  doc["datetime"] = formattedDate;
  doc["date"] = dayStamp;
  doc["time"] = timeStamp;
  doc["currentflow"] = flow_frequency;
  doc["totalflow"] = totalflow_frequency;
  doc["errormessage"] = errormessage;
  if (cfg.getIntConfig("havepm25")) {
    doc["pm25"] = pmdata.pm2_5;
    doc["pm1"] = pmdata.pm1;
    doc["pm10"] = pmdata.pm10;
  }
  if (cfg.getIntConfig("havertc")) {
    // RtcDateTime currentTime = rtcObject.GetDateTime(); // get the time from
    // the RTC

    // if (currentTime.Year() != 2000)
    // {
    //     char str[20]; // declare a string as an array of chars

    //     sprintf(str, "%d/%d/%d %d:%d:%d", //%d allows to print an integer to
    //     the string
    //             currentTime.Year(),       // get year method
    //             currentTime.Month(),      // get month method
    //             currentTime.Day(),        // get day method
    //             currentTime.Hour(),       // get hour method
    //             currentTime.Minute(),     // get minute method
    //             currentTime.Second()      // get second method
    //     );
    //     doc["rtctime"] = str;
    // }
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
  doc["v"] = v;
  doc["i"] = i;
  doc["e"] = e;
  doc["p"] = p;
  doc["tde"] = tde;
  doc["firsttde"] = firstted;
  doc["mills"] = millis();
  doc["flow"] = flow_frequency;
  doc["totalflow"] = totalflow_frequency;
  doc["localtimestamp"] = realtime();
  doc["lasttimestart"] = timestamp;
  doc["fulldate"] = fulldate();
  //  Serial.print(gps.location.lat(), 6);
  // Serial.print(F(","));
  // Serial.print(gps.location.lng(), 6);
  if (configdata.havegps) {
  }
  const size_t n = serializeJson(doc, buf, cap);
  return (n > 0) ? n : 0;
}

boolean addTorun(int port, int delay, int value, int wait) {
  unsigned long t = millis();
  if (delay > counttime)
    counttime = delay;
  for (int i = 0; i < ioport; i++) {
    if (ports[i].port == port) {
      if (ports[i].flowfailtime > 0 && t >= ports[i].flowfailtime) {
        // Expiration of wait period: reset flowfailtime and flowfailcount to
        // retry
        ports[i].flowfailtime = 0;
        ports[i].flowfailcount = 0;
      }
      if (ports[i].flowfailtime <=
          t) // ถ้า flowfailtime น้อยกว่าหรือเท่ากับ t ให้ set
             // port ได้แต่ถ้ายังไม่ครบกำหนดให้หยุด set port ก่อน
      {
        unsigned long t = millis();
        ports[i].value = value;
        // if (ports[i].delay < delay)
        ports[i].delay = delay;
        ports[i].endtime = t + (delay * 1000); // บอกเวลาหยุดทำงาน
        ports[i].flowchecktime =
            t + (configdata.flowchecktime * 1000); // บอกเวลาให้ตรวจสอบน้ำไหล
        ports[i].waittime = wait;
        ports[i].run = 1;
        digitalWrite(ports[i].port, value);
        Serial.printf("Set port %d %ld\n", ports[i].run, ports[i].endtime);
        char logmsg[28];
        snprintf(logmsg, sizeof(logmsg), "run ON p%d %ds", port, delay);
        memlogAdd(MEMLOG_TASK, value, logmsg);
        return true;
      }
    }
  }

  char logmsg[28];
  snprintf(logmsg, sizeof(logmsg), "run block p%d", port);
  memlogAdd(MEMLOG_TASK, -1, logmsg);
  return false;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}

long ma() {
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
  if (oledok) {
    display.clear();
    display.print("distance:");
    display.println(cm);
    display.display();
  }
  return cm;
}
void stopfill() {
  // doc["fillwater"] = fordisplay;
  // fordisplay = 0;
  // doc["stopfill"] = "ok";
  // serializeJsonPretty(doc, jsonChar, jsonbuffersize);
  // server.send(200, "application/json", jsonChar);
}

void ota() {
  Serial.println("OTA NOW ===================+++++++++++------------------");
  if (WiFi.status() != WL_CONNECTED) {
    message = "OTA skipped (no WiFi)";
    memlogAdd(MEMLOG_OTA, -1, message);
    return;
  }
  if (oledok) {
    displayslot.description = "OTA";
    dd();
  }
  WiFiClient client;
  Serial.print("OTAURL:");

  String u =
      cfg.getConfig("otaurl", "http://192.168.88.5:888/rest/fw/update/sensor/");
  String url = u + version;
  Serial.println(url);
  const uint32_t otaSpace = ESP.getFreeSketchSpace();
  Serial.printf("OTA free space: %u bytes\n", otaSpace);
  if (otaSpace > 0 && otaSpace < 490000) {
    message = String("OTA skip: only ") + otaSpace + "B free";
    memlogAdd(MEMLOG_OTA, -2, message);
    Serial.println(F("OTA: not enough partition space (use 4M1M or 1M "
                     "ldscript, USB flash once)"));
    return;
  }
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
  String error = ESPhttpUpdate.getLastErrorString();

  Serial.println("return #################### " + error +
                 "###########################3");
  switch (ret) {
  case HTTP_UPDATE_FAILED:
    Serial.println("[update] Update failed. : " +
                   ESPhttpUpdate.getLastErrorString());
    message = "update ERROR " + ESPhttpUpdate.getLastErrorString();
    memlogAdd(MEMLOG_OTA, ESPhttpUpdate.getLastError(), message);
    break;
  case HTTP_UPDATE_NO_UPDATES:
    if (oledok) {
      displayslot.description = "No update";
      displayslot.foot2 = " No update";
      dd();
    }
    Serial.println("[update] Update no Update.");
    message = "OTA no update";
    memlogAdd(MEMLOG_OTA, 0, message);
    break;
  case HTTP_UPDATE_OK:
    if (oledok) {
      displayslot.description = "New FW";
      displayslot.foot2 = " New update";
      dd();
    }
    Serial.println("[update] Update ok."); // may not called we reboot the ESP
    memlogAdd(MEMLOG_OTA, 200, "OTA ok reboot");
    break;
  }
}

void checkin() {
  static int checkinFails = 0;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("checkin: WiFi not connected"));
    message = "Checkin skipped (no WiFi)";
    memlogAdd(MEMLOG_CHECKIN, -1, message);
    reconnectWifiNow();
    return;
  }
  if (configdata.checkinurl.length() == 0) {
    Serial.println(F("checkin: checkinurl empty"));
    message = "Checkin skipped (no URL)";
    memlogAdd(MEMLOG_CHECKIN, -2, message);
    return;
  }

  if (oledok) {
    displayslot.description = "checkin";
    dd();
  }

  DynamicJsonDocument doc(768);
  doc["freemem"] = system_get_free_heap_size();
  doc["version"] = version;
  doc["name"] = name;
  doc["ip"] = WiFi.localIP().toString();
  doc["mac"] = WiFi.macAddress();
  doc["ssid"] = WiFi.SSID();
  doc["password"] = "";

  char body[768];
  const size_t n = serializeJson(doc, body, sizeof(body));
  if (n >= sizeof(body)) {
    Serial.println(F("checkin: request JSON too large for buffer"));
    message = "Checkin payload buffer overflow";
    memlogAdd(MEMLOG_CHECKIN, -3, message);
    return;
  }

  WiFiClient client;
  HTTPClient http;

  Serial.printf("checkin POST (%u bytes) %s\n", (unsigned)n,
                configdata.checkinurl.c_str());
  http.begin(client, configdata.checkinurl);
  http.addHeader(F("Content-Type"), F("application/json"));
  if (configdata.checkintoken.length() > 0)
    http.addHeader(F("Authorization"), "Bearer " + configdata.checkintoken);
  const int httpCode = http.POST(reinterpret_cast<uint8_t *>(body), n);
  (void)http.getString(); // drain response (required before end / reuse)

  if (httpCode == HTTP_CODE_OK) {
    checkinFails = 0;
    if (oledok) {
      displayslot.foot2 = "checkin ok";
      dd();
    }
    message = "Check in ok";
    memlogAdd(MEMLOG_CHECKIN, httpCode, message);
  } else {
    message = "Checkin failed HTTP " + String(httpCode);
    Serial.printf("checkin: HTTP error %d\n", httpCode);
    memlogAdd(MEMLOG_CHECKIN, httpCode, message);
    checkinFails++;
    if (checkinFails >= 2) {
      checkinFails = 0;
      reconnectWifiNow();
    }
  }

  http.end();
}

void writeResponse(WiFiClient &client, JsonObject &json) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  serializeJsonPretty(json, client);
}
bool readRequest(WiFiClient &client) {
  bool currentLineIsBlank = true;
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      if (c == '\n' && currentLineIsBlank) {
        return true;
      } else if (c == '\n') {
        currentLineIsBlank = true;
      } else if (c != '\r') {
        currentLineIsBlank = false;
      }
    }
  }
  return false;
}

void reada0() {

  a0value = analog.readA0();
  psi = analog.readPsi(cfg.getDobuleConfig("va0"),
                       cfg.getDobuleConfig("sensorvalue"));
  if (cfg.getIntConfig("haveoled") && otatime % 30 == 0) {
    Serial.println("READ A0");
    Serial.print("A0 value:");
    Serial.println(a0value);
  }
}

void readTmp() {
  Serial.println("READ DS");
  // tmpvalue = ds.readDs();
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

void readAm2320() {
  // pfTemp = am2320.readTemperature();
  // pfHum = am2320.readHumidity();
  // dhtbuffer.t = pfTemp;
  // dhtbuffer.h = pfHum;
  // dhtbuffer.count = 120; // update buffer life time
}
void readam() { readAm2320(); }
void senddata() {

  if (WiFi.status() == WL_CONNECTED) { // Check WiFi connection status

    digitalWrite(b_led, 1);
    checkin();
    ota();
    digitalWrite(b_led, 0);
  } else {
    // if (!WiFi.reconnect())
    // {
    //     ESP.restart();
    // }
  }
}
void getRtc() {
  // Serial.println("Update RTC");
  // RtcDateTime currentTime = rtcObject.GetDateTime(); // get the time from the
  // RTC

  // char str[20]; // declare a string as an array of chars

  // sprintf(str, "%d/%d/%d %d:%d:%d", //%d allows to print an integer to the
  // string
  //         currentTime.Year(),       // get year method
  //         currentTime.Month(),      // get month method
  //         currentTime.Day(),        // get day method
  //         currentTime.Hour(),       // get hour method
  //         currentTime.Minute(),     // get minute method
  //         currentTime.Second()      // get second method
  // );
  // Serial.println(str); // print the string to the serial port

  // m = currentTime.Minute();
  // h = currentTime.Hour();
  // s = currentTime.Second();
  // Y = currentTime.Year();
  // M = currentTime.Month();
  // d = currentTime.Day();
}
void readRTC() {
  if (cfg.getIntConfig("havertc")) {
    getRtc();
  }
}
void inden() {
  displaycounter++;
  displayshtcount++;
  displaytmp++;
  gpsdisplaytime++;
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
  readpzemtime++;
  checkconnectiontime++;
  updatetimecounter++;
  dotState = !dotState;
  if (apmode) {
    apmodetime++;
  }
  if (counttime > 0)
    counttime--;

  if (!readdhtstate && canuseled) {
    ledstatus = !digitalRead(b_led);
    digitalWrite(b_led, ledstatus);
  }
  // ถ้ามีการหยุดปั๊มให้
  if (configdata.havewaterlimit) {
    idlewaterlimit++;
  }
  if (waterlimitime > 0)
    waterlimitime--;

  kt.run();
}

void setAPMode() {
  if (WiFi.status() != WL_CONNECTED) {
    String mac = WiFi.macAddress();
    WiFi.softAP("ESP-Sensor" + mac, "12345678");
    IPAddress IP = WiFi.softAPIP();
    Serial.println(IP.toString());
    apmode = 1;
  } else {
  }
}

void printIPAddressOfHost(const char *host) {
  IPAddress resolvedIP;
  if (!WiFi.hostByName(host, resolvedIP)) {
    Serial.println("DNS lookup failed.  Count..." + String(wifitimeout));
    Serial.flush();
    wifitimeout++;
    if (wifitimeout > 5 && cfg.getIntConfig("havetorestart")) {
      reconnectWifiNow();
    }
  }
  Serial.print(host);
  Serial.print(" IP: ");
  Serial.println(resolvedIP);
}

static void sendConfigJson(AsyncWebServerRequest *request) {
  File f = LittleFS.open("/config.cfg", "r");
  if (!f) {
    request->send(500, "application/json", "{}");
    return;
  }
  if (f.size() == 0) {
    f.close();
    request->send(200, "application/json", "{}");
    return;
  }

  AsyncResponseStream *response =
      request->beginResponseStream("application/json");
  uint8_t buf[256];
  while (f.available()) {
    const size_t n = f.read(buf, sizeof(buf));
    if (n == 0)
      break;
    response->write(buf, n);
  }
  f.close();
  request->send(response);
}

static void sendConfigDescJson(AsyncWebServerRequest *request) {
  request->send_P(200, "application/json", CONFIG_DESC_JSON);
}

static void handlePingRequest(AsyncWebServerRequest *request);

void setHttp() {

  if (WiFi.status() != WL_CONNECTED)
    return; // ออกเลยถ้าไม่ต่อ wifi

  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", logs_html);
  });
  server.on("/logs.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    char *buf = memlogJsonBuffer();
    const size_t cap = memlogJsonBufferSize();
    if (memlogWriteJson(buf, cap, version.c_str()) == 0) {
      request->send(
          500, "application/json",
          "{\"logs\":[],\"count\":0,\"max\":0,\"heap\":0,\"error\":\"json\"}");
      return;
    }
    request->send(200, "application/json", buf);
  });
  server.on("/logs/clear", HTTP_GET, [](AsyncWebServerRequest *request) {
    memlogClear();
    request->send(200, "application/json", "{\"ok\":1}");
  });
  // server.on("/dht", DHTtoJSON);
  // server.on("/pressure", PressuretoJSON);
  // server.on("/ktype", KtypetoJSON);
  // server.on("/info", info);
  // server.on("/ds18b20", readDS);
  // server.on("/run", run);
  // server.on("/a0", a0);
  // server.on("/setwifi", setwifi);

  // server.on("/scanwifi", scanwifi);

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      AsyncWebServerResponse *response =
          request->beginResponse(204, "text/html", "not found");
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Max-Age", "10000");
      response->addHeader("Access-Control-Allow-Methods",
                          "PUT,POST,GET,OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "*");
      request->send(response);
    } else {
      request->send(404, "text/plain", "");
    }
    request->send(404);
  });
  server.on("/setconfigwww", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", configfile_html);
  });
  server.on("/configdesc.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendConfigDescJson(request);
  });

  //-------------------------------------------------------------------------------------------------------------------------
  server.on("/resetconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
    cfg.resettodefault();
    loadconfigtoram();
    request->send(200, "application/json",
                  "{\"setconfig\":\"ok\",\"value\":\"ok\"}");
    ESP.restart();
  });
  server.on("/setconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
    String v = request->arg("configname");
    String value = request->arg("value");
    cfg.addConfig(v, value);
    loadconfigtoram();
    request->send(200, "application/json",
                  "{\"setconfig\":\"" + v + "\",\"value\":\"" + value + "\"}");
  });

  server.on("/removeconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
    String v = request->arg("configname");
    cfg.remove(v);
    loadconfigtoram();
    request->send(200, "application/json", "{\"remove\":\"" + v + "\"}");
  });
  //-------------------------------------------------------------------------------------------------------------------------
  server.on("/config", HTTP_GET,
            [](AsyncWebServerRequest *request) { sendConfigJson(request); });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    makeStatusJson(g_statusJsonBuf, sizeof(g_statusJsonBuf));
    AsyncWebServerResponse *response = request->beginResponse(
        200, "application/json; charset=utf-8", g_statusJsonBuf);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Max-Age", "10000");
    response->addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "*");
    request->send(response);
  });
  //------------------------------------------------------------------------------------------------------------------------

  server.on("/scanwifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument dy(jsonbuffersize);
    char buf[jsonbuffersize];
    int n = WiFi.scanNetworks();
    for (int i = 0; i < n; i++) {
      String s = "wifi:" + String(i);
      String ss = "wifisignel:" + String(i);
      dy[s] = WiFi.SSID(i);
      dy[ss] = WiFi.RSSI(i);
    }
    serializeJsonPretty(dy, buf, jsonbuffersize);
    request->send(200, "application/json", buf);
  });
  //------------------------------------------------------------------------------------------------------------------------
  server.on("/run", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Run");
    String p = request->arg("port");
    char b[jsonbuffersize];
    DynamicJsonDocument dy(jsonbuffersize);
    if (p.equals("test")) {
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
      // request->send(200, "application/json", b);
      AsyncWebServerResponse *response =
          request->beginResponse(200, "application/json", b);
      response->addHeader("Access-Control-Allow-Origin", "*");
      response->addHeader("Access-Control-Max-Age", "10000");
      response->addHeader("Access-Control-Allow-Methods",
                          "PUT,POST,GET,OPTIONS");
      response->addHeader("Access-Control-Allow-Headers", "*");
      request->send(response);
      for (int i = 0; i < 40; i++) {
        digitalWrite(2, !digitalRead(2));
        delay(200);
      }
      canuseled = 1;
      return;
    }
    String v = request->arg("value");
    String d = request->arg("delay");
    String w = request->arg("wait");
    Serial.println("Port: " + p + " value : " + v + " delay: " + d);
    message = String("run port ") + String(p) + String(" value") +
              String(" delay ") + String(d) + " " +
              timeClient.getFormattedDate();
    int value = v.toInt();
    int port = getPort(p);
    if (p == NULL) {
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
    AsyncWebServerResponse *response =
        request->beginResponse(200, "application/json", b);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Max-Age", "10000");
    response->addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "*");
    request->send(response);
  });
  //------------------------------------------------------------------------------------------------------------------------

  // server.on("/setconfig", setconfig);
  // server.on("/inconfigfile", allinfile);
  // server.on("/get", get);
  // server.on("/updatecheckin", updateCheckin);
  // server.on("/readam", readam);
  // server.on("/status", status);
  // server.on("/reset", reset);
  server.on("/checkin", HTTP_GET, [](AsyncWebServerRequest *request) {
    checkintime = configdata.checkintime + 1;
    request->send(200, "application/json", "{\"Check in\":\"ok\"}");
  });

  server.on("/ping", HTTP_GET,
            [](AsyncWebServerRequest *request) { handlePingRequest(request); });

  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"reset\":\"ok\"}");
    ESP.restart();
  });

  server.on("/findsoi", HTTP_GET, [](AsyncWebServerRequest *request) {
    findsoinow = true;
    request->send(200, "application/json",
                  "{\"findsoi\":\"ok\",\"date\":\"" + fulldate() + "\"}");
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"reset\":\"ok\"}");
    ESP.restart();
  });
  //-------------------------------------------------------------------------------------------------------------------------
  // server.on("/restart", reset);
  // server.on("/setp", setValue2);
  // server.on("/update", ota);
  // server.on("/timer", runtimer);
  server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(jsonbuffersize);
    String time = request->arg("time"); // เวลาที่ทำงานงาน
    String message = request->arg("closetime");
    String l = request->arg("l");
    if (oledok) {
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
    AsyncWebServerResponse *response =
        request->beginResponse(200, "application/json", buf);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Max-Age", "10000");
    response->addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "*");
    request->send(response);
  });

  if (configdata.havewater) {
    Serial.println("Set http for function /openwater");
    server.on("/openwater", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (request->hasArg("w")) {
        int watertorefill = request->arg("w").toInt();
        fordisplay =
            watertorefill / 0.0022; // จะแสดงว่าระบบจะต้องเติมน้ำเข้าไปเท่าไหร่
        // เปิดน้ำเปิด Sonenoi
        digitalWrite(D1, 1);
        Serial.printf("Open water %d", fordisplay);
        displayTOTM((fordisplay * 0.0022) + 1);
        char buf[255];
        sprintf(buf, "{\"open\": %d}", fordisplay);
        request->send(200, "application/json", buf);
      }
    });
    //------------------------------------------------------------------------------------------------------------------------

    server.on("/stopfill", HTTP_GET, [](AsyncWebServerRequest *request) {
      DynamicJsonDocument doc(jsonbuffersize);
      doc["fillwater"] = fordisplay;
      fordisplay = 0;
      doc["stopfill"] = "ok";
      char buf[jsonbuffersize];
      serializeJsonPretty(doc, buf, jsonbuffersize);
      request->send(200, "application/json", buf);
    });
  }
  // // closetime parameter have to show on oled
  // server.on("/setclosetime", runtimer); // time parameter to count

  server.on("/settimestamp", setTimestamp);
  server.on("/findair", finddry);
  server.on("/findwet", findwet);
  server.begin(); // เปิด TCP Server
  Serial.println("Server started");
  if (oledok) {
    displayslot.description = "Http started";
  }
}

void Apmoderun() {

  ApMode ap("/config.cfg");
  ap.setapmodetime(cfg.getIntConfig("apmodetimeout", 3));
  ap.setApname("AP MODE D1 " + WiFi.macAddress());
  ap.run();
}
int disconnecttimeout = 0;

static String trimConfig(const String &s) {
  String t = s;
  t.trim();
  return t;
}

static void resetWifiMulti() {
  wifiMulti->cleanAPlist();
  wifiMulti->~ESP8266WiFiMulti();
  new (&wifiMultiObj) ESP8266WiFiMulti();
  wifiMulti = &wifiMultiObj;
}

static void loadWifiMultiFromConfig() {
  resetWifiMulti();
  for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
    String ssidKey = (i == 0) ? String("ssid") : String("ssid") + String(i + 1);
    String passKey =
        (i == 0) ? String("password") : String("password") + String(i + 1);
    String ssid = trimConfig(cfg.getConfig(ssidKey, (i == 0) ? "forpi" : ""));
    if (ssid.length() == 0 || ssid == "0")
      continue;
    String pass =
        trimConfig(cfg.getConfig(passKey, (i == 0) ? "04qwerty" : ""));
    if (wifiMulti->addAP(ssid.c_str(), pass.c_str()))
      Serial.printf("WiFi list %d: %s\n", i + 1, ssid.c_str());
    else
      Serial.printf("WiFi skip %d: %s\n", i + 1, ssid.c_str());
  }
}

static bool connectOneWifi(const String &ssid, const String &pass,
                           int timeoutSec) {
  if (ssid.length() == 0)
    return false;
  Serial.printf("WiFi try: %s\n", ssid.c_str());
  WiFi.disconnect();
  delay(200);
  WiFi.begin(ssid.c_str(), pass.c_str());
  int n = 0;
  const int maxTry = timeoutSec * 2;
  while (WiFi.status() != WL_CONNECTED && n < maxTry) {
    delay(500);
    n++;
    yield();
  }
  if (WiFi.status() == WL_CONNECTED &&
      WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
    Serial.print(F("Connected IP: "));
    Serial.println(WiFi.localIP());
    return true;
  }
  return false;
}

static bool connectAllConfiguredWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.disconnect(true);
  delay(300);

  loadWifiMultiFromConfig();

  if (wifiMulti->run(20000) == WL_CONNECTED &&
      WiFi.localIP() != IPAddress(0, 0, 0, 0)) {
    Serial.print(F("WiFiMulti connected: "));
    Serial.println(WiFi.SSID());
    return true;
  }

  Serial.println(F("WiFiMulti failed, try each SSID in order"));
  for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
    String ssidKey = (i == 0) ? String("ssid") : String("ssid") + String(i + 1);
    String passKey =
        (i == 0) ? String("password") : String("password") + String(i + 1);
    String ssid = trimConfig(cfg.getConfig(ssidKey, (i == 0) ? "forpi" : ""));
    if (ssid.length() == 0 || ssid == "0")
      continue;
    String pass =
        trimConfig(cfg.getConfig(passKey, (i == 0) ? "04qwerty" : ""));
    if (connectOneWifi(ssid, pass, 25))
      return true;
  }
  return false;
}

bool reconnectWifiNow() {
  memlogTask("reconn");
  Serial.println(F("reconnectWifiNow"));
  if (connectAllConfiguredWifi()) {
    isDisconnect = false;
    wifitimeout = 0;
    memlogAdd(MEMLOG_CHECKIN, 0, "WiFi reconnected");
    return true;
  }
  isDisconnect = true;
  memlogAdd(MEMLOG_ERROR, -1, "WiFi reconnect fail");
  return false;
}

static String connectionPingTarget() {
  if (configdata.pinghost.length() > 0)
    return configdata.pinghost;
  IPAddress gw = WiFi.gatewayIP();
  if (gw != IPAddress(0, 0, 0, 0))
    return gw.toString();
  return String("192.168.88.1");
}

static void handlePingRequest(AsyncWebServerRequest *request) {
  String target = connectionPingTarget();
  if (request->hasParam("host"))
    target = request->getParam("host")->value();
  const bool doReconnect = request->hasParam("reconnect") &&
                           request->getParam("reconnect")->value() == "1";
  const bool ok = doReconnect ? checkAndReconnectToIP(target, 8000)
                              : pingTarget(target, 8000);

  char buf[320];
  snprintf(buf, sizeof(buf),
           "{\"ok\":%d,\"target\":\"%s\",\"reconnect\":%d,\"wifi\":%d,\"ssid\":"
           "\"%s\",\"ip\":\"%s\",\"gateway\":\"%s\",\"rssi\":%d}",
           ok ? 1 : 0, target.c_str(), doReconnect ? 1 : 0, WiFi.status(),
           WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(),
           WiFi.gatewayIP().toString().c_str(), WiFi.RSSI());
  request->send(200, "application/json", buf);
}

void connect() {

  Serial.println();
  Serial.println("-----------------------------------------------");
  if (oledok) {
    displayslot.description = "Connect WiFi";
    displayslot.description1 = cfg.getConfig("ssid");
    dd();
  }
  Serial.print("connect.");
  int ft = 0;
  bool connected = false;
  while (!connected && ft <= configdata.maxconnecttimeout) {
    if (oledok) {
      displayslot.foot = "connect";
      displayslot.foot2 = (ft & 1) ? "/" : "\\";
      dd();
    }
    connected = connectAllConfiguredWifi();
    if (!connected) {
      ft++;
      delay(1000);
      Serial.print(".");
    }
  }

  if (!connected) {
    Serial.println("Connect main wifi timeout");
    apmode = 1;
  }

  if (apmode) {
    Apmoderun();
  } else {

    WiFi.setSleepMode(WIFI_NONE_SLEEP);
    Serial.println(WiFi.localIP()); // แสดงหมายเลข IP ของ Server
    Serial.print("Connected SSID: ");
    Serial.println(WiFi.SSID());
    String ip = WiFi.localIP().toString();
    String mac = WiFi.macAddress();
    Serial.println(mac); // แสดงหมายเลข IP ของ Server
    if (oledok) {
      displayslot.description = ip;
      displayslot.foot = mac;
      dd();
      delay(1000);
    }

    printIPAddressOfHost("fw1.pixka.me");
  }
}
MoveAvg havg(16);
MoveAvg tavg(16);

void readSht() {
  if (hservice == NULL)
    return;
  if (!hservice->isReady() && !hservice->init()) {
    memlogAdd(MEMLOG_SHT, -1, "SHT init fail");
    return;
  }
  hservice->read();
  if (!hservice->readstatus()) {
    memlogAdd(MEMLOG_SHT, -1, "SHT read fail");
    return;
  }
  havg.pushValue(hservice->geth());
  // pfHum = havg.getTotal() / havg.getSize();
  pfHum = hservice->geth();
  tavg.pushValue(hservice->gett());
  // pfTemp = tavg.getTotal() / tavg.getSize();
  pfTemp = hservice->gett();

  char logmsg[28];
  snprintf(logmsg, sizeof(logmsg), "T=%d H=%d", (int)pfTemp, (int)pfHum);
  memlogAdd(MEMLOG_SHT, (int)pfHum, logmsg);
}
void setSht() {
  if (hservice == NULL)
    hservice = new Htask();

  if (hservice->init()) {
    Serial.println(F("SHT init ok"));
    memlogAdd(MEMLOG_SHT, 0, "SHT init ok");
    readSht();
  } else {
    Serial.println(F("SHT init failed"));
    memlogAdd(MEMLOG_SHT, -1, "SHT init fail");
  }
}
time_t timeSinceEpoch;
unsigned long updatetime; // เป็นตัวบอกว่าเวลาที่รับ timestamp จาก gps
String gpsdate = "";
String gpstime = "";
String gpsdatetime = "";
String gpsloc = "";
String gpsraw = "";

void readGps() {
  if (configdata.havegps) {
    if (gps != NULL) {
      gps->read();
      static unsigned long lastGpsLog = 0;
      if (millis() - lastGpsLog >= 10000) {
        memlogTask("gpsRd");
        lastGpsLog = millis();
      }
    }
  }
}
void displayGpsData() {
  if (gpsdisplaytime > 10 && configdata.havegps) {
    memlogTask("gps");
    Serial.print("GPS:");
    gpsdisplaytime = 0;
    if (gps != NULL) {
      Serial.println(gps->getData());
      Serial.println(gps->timeEpoch());
    }
  }
}
void settime() {

  if (timeClient.update()) {
    Serial.println("Update time");
    message = "Update Time";
    long t = timeClient.getEpochTime();

    h = timeClient.getHours();
    m = timeClient.getMinutes();
    s = timeClient.getSeconds();

    Serial.print("T: ");
    Serial.println(t);
    // if (cfg.getIntConfig("havertc"))
    // {
    //     RtcDateTime currentTime = RtcDateTime(t - 946684800); // define date
    //     and time object rtcObject.SetDateTime(currentTime); // configure the
    //     RTC with object
    // }
  }
}

void setRTC() {
  //     if (cfg.getIntConfig("havertc"))
  //         rtcObject.Begin(); // Starts I2C
}

void setupoled() {
  if (display.init()) {
    oledok = 1;
    displayslot.head = "Sirifarm";
    displayslot.description = "start up";
    displayslot.description1 = "display ok";
    displayslot.foot = "version:";
    displayslot.foot2 = version;
    display.flipScreenVertically();
    dd();
    delay(1000);
  } else {
    Serial.println("Display not ok");
    oledok = 0;
  }
}

/**
 * ใช้สำหรับ run เมื่อ สร้าง file ครั้งแรก
 * */
void initConfig() {
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
void setupwater() {
  String p = cfg.getConfig("flowinterrrupport", "D6");
  int pt = getPort(p);
  pinMode(pt, INPUT);
  if (configdata.havewaterlimit) {
    attachInterrupt(pt, waterlimitinterrup, RISING);
  } else {
    attachInterrupt(pt, flow, RISING); // Setup Interrupt
  }
  pinMode(waterlimitport, OUTPUT);
  digitalWrite(waterlimitport, 0);
  pinMode(REALYPORT, OUTPUT);
  digitalWrite(REALYPORT, 0);
  Serial.println("Setup Interrup  for water ...");
}
void setupntp() {
  timeClient.begin();
  timeClient.setTimeOffset(25200); // Thailand +7 = 25200
}

void setStanaloneWifi() {
  String espname = cfg.getConfig("stanalonename", "ESP-");
  String pass = cfg.getConfig("stanalonepassword", "1234567890");
  const byte DNS_PORT = 53;
  IPAddress apIP(10, 10, 10, 1); // Private network for server
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  String wifi = espname + " " + WiFi.macAddress().c_str();
  WiFi.softAP(wifi, pass); // WiFi name
  dnsServer.start(DNS_PORT, "*", apIP);
}

void runstanalone() { setStanaloneWifi(); }

void displaySht() {
  char buf[16];
  int h10 = (int)(pfHum * 10 + 0.5f);
  int t10 = (int)(pfTemp * 10 + 0.5f);
  snprintf(buf, sizeof(buf), "H:%d.%d", h10 / 10, h10 % 10);
  displayslot.description1 = String(buf);
  snprintf(buf, sizeof(buf), "T:%d.%d", t10 / 10, t10 % 10);
  displayslot.description = String(buf);
}
void checkintask() {

  if (checkintime > configdata.checkintime) {
    memlogTask("checkin");
    checkintime = 0;
    checkin();
  }
}
void displaytmptask() {
  if (displaytmp > 30 && configdata.haveds) {
    memlogTask("dispTmp", (int)tmpvalue);
    if (oledok) {
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
void apmodetask() {
  if (configdata.havetorestart && apmodetime > configdata.apmodetimeout &&
      fordisplay <= 0) {
    memlogTask("apRestart");
    if (oledok) {
      display.clear();
      display.println("Restart");
      display.drawLogBuffer(0, 0);
      display.display();
    }

    ESP.restart();
  }
  apmodetime = 0;
}
void checkconnectiontask() {
  static int connHealthFails = 0;

  if (checkconnectiontime > configdata.checkconnectiontime) {
    memlogTask("conn", WiFi.status());
    Serial.println("Check connection");
    checkconnectiontime = 0;

    const String target = connectionPingTarget();
    Serial.printf("Ping target: %s\n", target.c_str());
    const bool ok = checkAndReconnectToIP(target, 8000);
    if (ok) {
      connHealthFails = 0;
      isDisconnect = false;
    } else {
      connHealthFails++;
      isDisconnect = true;
      char msg[32];
      snprintf(msg, sizeof(msg), "conn fail x%d", connHealthFails);
      memlogAdd(MEMLOG_ERROR, connHealthFails, msg);
      if (connHealthFails >= 3 && configdata.havetorestart)
        ESP.restart();
    }
  }
}
void otatask() {
  if (otatime > configdata.otatime) {
    memlogTask("ota");
    otatime = 0;
    ota();
    settime();
  }
}
void dhttask() {
  if (readdhttime > configdata.readdhttime && configdata.havedht) {
    memlogTask("dht");
    Serial.println("Read DHT");
    readdhttime = 0;
    message = "Read DHT";
    readDHT();
  }
}
void dsreadtask() {
  if (readdstime > configdata.readdstime && configdata.haveds) {
    memlogTask("ds18");
    readdstime = 0;
    readTmp();
  }
}
void shtreadtask() {
  if (configdata.havesht && readshtcount > configdata.readshttime) {
    memlogTask("sht");
    readshtcount = 0;
    readSht();

    if (displayshtcount > 20) {
      displayshtcount = 0;
      memlogTask("shtDisp");
      displaySht();
    }
  }
}
void porttask() { portcheck(); }
void ntptask() {
  if (ntptime > configdata.ntpupdatetime) {
    memlogTask("ntp");
    updateNTP();
    ntptime = 0;
    Serial.print("Update time:");
    Serial.println(timeClient.getFormattedTime());
    if (oledok) {
      displayslot.description = "update time";
      displayslot.description1 = "ok";
      dd();
    }
  }
}
void rtctask() {
  if (configdata.havertc && rtctime > configdata.rtctimeupdate &&
      fordisplay <= 0) {
    memlogTask("rtc");
    readRTC();
    rtctime = 0;
  }
}
void pmtask() {
  if (configdata.havepmsensor && fordisplay <= 0) {
    memlogTask("pmTsk");
    readPm();
  }
}
void distancetask() {
  if (configdata.havesonic && readdistance > configdata.readdistancetime &&
      fordisplay <= 0) {
    readdistance = 0;
    Serial.println("Update Distance");
    distance = ma();
    memlogTask("sonic", distance);
    if (oledok) {
      displayslot.description = "Distance";
      displayslot.description1 = String(distance);
      dd();
    }
  }
}
void countertask() {
  if (kt.getSec() >= 1 && displaycounter > 0 && fordisplay <= 0) {
    memlogTask("timer", kt.getSec());
    Serial.print("Count:");
    Serial.println(kt.getSec());
    if (oledok) {
      displayslot.description = kt.getMessage();
      displayslot.description1 = String(kt.getSec());
      dd();
    }

    displaycounter = 0;
  }
  if (kt.getSec() == 1) {
    memlogTask("tmrEnd");
    if (oledok) {
      displayslot.head = "SiriFarm";
    }
  }
}
void makestatustask() {
  if (makestatuscount > 15000 && fordisplay <= 0) {
    makestatuscount = 0;
  }
}
void a0task() {
  if (configdata.havea0 && reada0time > configdata.reada0time) {
    memlogTask("a0");
    reada0time = 0;
    Serial.println("Update A0");
    reada0();
  }
  if (configdata.havea0 && checkintime % 30 == 0) {
    memlogTask("a0Dsp", (int)a0value);
    if (oledok) {
      displayslot.description = "A0";
      displayslot.description1 = String(a0value) + "PSI";
      displayslot.foot2 = String(a0value) + "PSI";
      dd();
    }
  }
}
void watertask() {
  if (configdata.havewater && flow_frequency > 0) {
    static int lastFlow = -1;
    if (flow_frequency != lastFlow) {
      memlogTask("water", flow_frequency);
      lastFlow = flow_frequency;
    }
  }
}
void loadtask() {
  load = millis() - s;
  loadcount++;
  loadtotal += load;
  loadav = loadtotal / loadcount;
}
void oledtask() {
  if (oledok) {
    static unsigned long lastOledLog = 0;
    if (millis() - lastOledLog >= 30000) {
      memlogTask("oled");
      lastOledLog = millis();
    }
    if (!ledstatus) {
      displayslot.foot = ".";
    } else {
      displayslot.foot = " ";
    }
    dd();
  }
}
// สำหรับตรวจสอบว่ามีการใช้น้ำเกินกำหนดเปล่า
void waterlimittask() {
  if (configdata.havewaterlimit) {
    static unsigned long lastWLmtLog = 0;
    if (millis() - lastWLmtLog >= 60000) {
      memlogTask("wLmt", wateruse);
      lastWLmtLog = millis();
    }
    if (wateruse >= configdata.waterlimitvalue) {
      memlogTask("wLimit", wateruse);
      waterlimitime = configdata.waterlimittime; // ให้ทำการตัดปั๊มออกจากระบบ
      wateruse = 0;                              // หยุดรอไม่ใช้งานละ
      currentwateroverlimit++;                   // เพิ่มจำนวนการใช้น้ำเกินเข้าระบบ
    }

    if (waterlimitime > 0) {
      digitalWrite(waterlimitport,
                   1); // เปิดระบบตัดน้ำแล้วระบบจะทำการลดค่า limit ไปเรื่อยๆ
    } else {
      digitalWrite(waterlimitport, 0); // ปิดถ้าค่า หยุดรอหมด
    }

    // ถ้ามีการใช้น้ำเกินกำหนดหรือว่าท่อแตกหรืออะไรซํกอย่างระบบจะตัดหรือยก relay
    static bool overLimitTripped = false;
    if (currentwateroverlimit >= configdata.wateroverlimitvalue &&
        !overLimitTripped) {
      overLimitTripped = true;
      memlogTask("wOver", currentwateroverlimit);
      waterlimitime = configdata.waterlimittime;
      digitalWrite(REALYPORT, 1); // สั่งระบบยก relay
    }
    if (idlewaterlimit >= configdata.wateridletime && waterlimitime <= 0) {
      if (wateruse > 0 || currentwateroverlimit > 0)
        memlogTask("wIdle");
      wateruse = 0;              // ไม่มีการใช้น้ำแล้ว
      currentwateroverlimit = 0; // ถ้ามีการหยุดใช้น้ำแล้วก็ยกเลิกการน้ำใช้น้ำเกิน
      overLimitTripped = false;
    }
  }
}
unsigned long getUptime() { return millis() / 1000; }
void havekey() {
  if (Serial.available()) {
    char k = Serial.read();
    memlogTask("key", k);
    Serial.printf("Key is %c\n", k);
    if (k == 'w') {
      scanwifi();
    } else if (k == 'h') {
      int re = talktoServer(WiFi.localIP().toString(), name, uptime, &cfg);

      Serial.printf("Result hello to %s  = %d \n",
                    cfg.getConfig("talkurl").c_str(), re);
    } else if (k == 'f') {
      bool ok = reconnectWifiNow();
      Serial.printf("REconnect %s\n", ok ? WiFi.SSID().c_str() : "fail");
    } else if (k == 'p') {
      const String target = connectionPingTarget();
      const bool ok = pingTarget(target, 8000);
      Serial.printf("Ping %s = %s (gw %s ip %s)\n", target.c_str(),
                    ok ? "OK" : "FAIL", WiFi.gatewayIP().toString().c_str(),
                    WiFi.localIP().toString().c_str());
    } else if (k == 'c') {
      checkin();
    } else if (k == 'o') {
      ota();
    } else if (k == 't' || k == 'T') {
      Serial.println("Update time from ip ");
      updateTime();
    } else if (k == 'w' || k == 'a') {
      Serial.println("Find wet and air value ");
      findwetair();
    }
  }
}
void readpzem() {
  if (configdata.havepzem && readpzemtime >= configdata.readpzemtime) {
    v = pzem.voltage();
    i = pzem.current();
    p = pzem.power();
    e = pzem.energy();
    f = pzem.frequency();
    findTDE();
    pf = pzem.pf();
    float s = v * i;
    q = sqrt(pow(s, 2) - pow(p, 2));
    readpzemtime = 0;
    memlogTask("pzem", (int)v);
  }
}
String filllist(const String &var) {
  // Serial.println(var);
  char buf[2024];
  String tr = "";
  if (var == "list") {
    Serial.println("Fill list");
    Espjob *index = js->getfirst();
    // tr += "<tr><td>" + String(js->getsize()) + "</td></tr>";
    while (index != NULL) {
      Serial.println(index->id);
      sprintf(
          buf,
          "<tr><td>%d</td><td>%.2f</td><td>%.2f</td><td>%d</td><td>%d</"
          "td><td>%d</td><td>%d</td><td>%d</td><td>%s</td><td>%s</"
          "td><td><button onclick='deletejob(%d)'>delete</button></td></tr>\n",
          index->id, index->hlow, index->hhigh, index->port, index->runtime,
          index->waittime, index->out, index->enable, index->stime,
          index->etime, index->id);
      tr += String(buf);

      index = index->n;
    }
  }
  if (var == "CONFIG") {
    return String("<tr><td colspan=\"11\"><a href=\"/setconfigwww\">Open "
                  "config page</a></td></tr>");
  }

  return tr;
}
/**
 * @brief
 *
 * @return unsigned long
 */
unsigned long getEP() {
  return timeSinceEpoch + ((millis() - updatetime) / 1000);
}
int getDay() {
  return (((getEP() / 86400L) + 4) % 7); // 0 is Sunday
}
int getHours() { return ((getEP() % 86400L) / 3600); }
int getMinutes() { return ((getEP() % 3600) / 60); }
int getSeconds() { return (getEP() % 60); }
void setstanalonehttp() {

  server.on("/logs", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", logs_html);
  });
  server.on("/logs.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    char *buf = memlogJsonBuffer();
    const size_t cap = memlogJsonBufferSize();
    if (memlogWriteJson(buf, cap, version.c_str()) == 0) {
      request->send(
          500, "application/json",
          "{\"logs\":[],\"count\":0,\"max\":0,\"heap\":0,\"error\":\"json\"}");
      return;
    }
    request->send(200, "application/json", buf);
  });
  server.on("/logs/clear", HTTP_GET, [](AsyncWebServerRequest *request) {
    memlogClear();
    request->send(200, "application/json", "{\"ok\":1}");
  });

  server.on("/setconfigwww", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", configfile_html);
  });
  server.on("/configdesc.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    sendConfigDescJson(request);
  });
  //-------------------------------------------------------------------------------------------------------------------------
  server.on("/resetconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
    cfg.resettodefault();
    loadconfigtoram();
    request->send(200, "application/json",
                  "{\"setconfig\":\"ok\",\"value\":\"ok\"}");
    ESP.restart();
  });
  server.on("/setconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
    String v = request->arg("configname");
    String value = request->arg("value");
    cfg.addConfig(v, value);
    loadconfigtoram();
    request->send(200, "application/json",
                  "{\"setconfig\":\"" + v + "\",\"value\":\"" + value + "\"}");
  });

  server.on("/removeconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
    String v = request->arg("configname");
    cfg.remove(v);
    loadconfigtoram();
    request->send(200, "application/json", "{\"remove\":\"" + v + "\"}");
  });
  //-------------------------------------------------------------------------------------------------------------------------
  server.on("/config", HTTP_GET,
            [](AsyncWebServerRequest *request) { sendConfigJson(request); });

  server.on("/deletejob", HTTP_GET, [](AsyncWebServerRequest *request) {
    int id = request->getParam("id")->value().toInt();
    Espjob *p = js->findById(id);
    js->deletejob(p);
    js->savetofile("/job.job");
    request->send(200, "text/html", js->toString());
  });
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request) {
    unsigned long t = getEP();
    //  unsigned long t1 = timeSinceEpoch;
    //  String b = "<i>"+bb+"</i> <b> "+String(t)+" </b> | GPS datetime:
    //  "+gpsdatetime +" Gps date:"+gpsdate+" Gps time:"+gpstime
    +" " + gpsloc;
    unsigned long heap = system_get_free_heap_size();
    String j = "{\"datetime\":\"" + gpsdatetime + "+\",\"timestamp\":" + t +
               ",\"uptime\":" + getUptime() + ",\"t\":" + hservice->gett() +
               ",\"h\":" + hservice->geth() + ",\"heap\":" + heap + "}";
    request->send(200, "application/json", j);
  });
  server.on("/savejob", HTTP_GET, [](AsyncWebServerRequest *request) {
    String port = request->getParam("port")->value();

    int intport = getPort(port);
    String hlow = request->getParam("hlow")->value();
    String hhigh = request->getParam("hhigh")->value();
    String runtime = request->getParam("runtime")->value();
    String waittime = request->getParam("waittime")->value();
    String out = request->getParam("output")->value();
    String enable = request->getParam("enable")->value();
    String stime = request->getParam("stime")->value();
    String etime = request->getParam("etime")->value();
    // Serial.println(request->getParam("port")->value());
    // char buf [2024];
    // sprintf(buf, "%d,%.2f,%.2f,%d,%d,%d,%d,%d", js->getsize()+1, hlow, hhigh,
    // port, runtime, waittime, 1, out); Serial.printf("\nSave job =>
    // %s\n",buf);
    js->addJob(js->newjob(js->getsize() + 1, hlow.toFloat(), hhigh.toFloat(),
                          intport, runtime.toInt(), waittime.toInt(),
                          enable.toInt(), out.toInt(), stime, etime));
    js->savetofile("/job.job");
    // sprintf(buf,"size:%d Port:%s runttime: %s waittime: %s OUT:%s",
    // js->getsize() ,port,runtime,waittime,out);
    request->send(200, "text/html", "ok");
  });
  server.on("/addjob", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", addjob_html, filllist);
  });

  server.on("/jobs", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", js->toString());
  });
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", indexstanalone_html);
  });
  server.begin();
}

void setup() {

  Serial.begin(9600);
  flipper.attach(1, inden);
  // kt.run();
  pinMode(b_led, OUTPUT); // On Board LED
                          //   pinMode(D4, OUTPUT);
                          //  pinMode(LED_BUILTIN, OUTPUT);
                          // pinMode(D6, OUTPUT);
                          // pinMode(D5, OUTPUT);
                          // pinMode(D1, OUTPUT);
                          //   pinMode(D3, OUTPUT);
                          //  digitalWrite(D3,0);
  cfg.setbuffer(configdata.jsonbuffer);
  if (!cfg.openFile()) {
    Serial.println("Init file");
    initConfig();
  }
  loadconfigtoram();
  if (configdata.havesht || configdata.haveoled) {
    Wire.begin(D2, D1);
    Wire.setClock(100000);
  }
  setport();
  if (cfg.getIntConfig("haveoled")) {
    setupoled();
  }

  if (configdata.havewater) {
    setupwater();
  }
  // if (configdata.havedht)
  // {
  //     dht.begin();
  //     readDHT();
  // }

  if (configdata.havesht) {
    setSht();
    hservice->setreadNext(configdata.readshttime > 0 ? configdata.readshttime
                                                     : 60);
  }

  if (configdata.havefastport) {

    // pinMode(D3, OUTPUT);
    // digitalWrite(D3,0);
    Serial.println("######## check fast port ##########");
  }

  // ถ้ามีการต่อ Pm Sensor
  if (configdata.havepmsensor) {
    Serial.println("######## Have Pm sensor check ##########");

    while (!Serial)
      ;
    mySerial.begin(9600);
  }
  if (configdata.havepzem) {
    Serial.println("Setup Pzem");
  } else {
    Serial.begin(9600);
  }

  if (!configdata.stanalone) {
    connect();
    setupntp();
    setHttp();
    settime();
    ota();
    checkin();
  } else {
    ss.begin(GPSBaud);
    gpsservice->start();
    gpsservice->settimezone(7);
    timeservice->setGps(gpsservice);
    if (configdata.havesht) {
      hservice->setreadNext(15);
      taskservice->setHtask(hservice);
    }
    taskservice->setJobService(js);
    taskservice->setTimeService(timeservice);
    js->setTimeService(timeservice);
    js->load(JOBFILE);
  }
  if (configdata.havegps) {
    if (gps == NULL)
      gps = new GPS();

    gps->start();
  }
  if (configdata.havetm) {
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
    tm1.setBrightness(0x05); // 0-7
    tm1.clear();
  }
  updateTime();
  if (configdata.havesoisensor) {
    pinMode(configdata.soienablepin, OUTPUT);
    readSoivalue();
    nextreadsoi = millis() + configdata.nextreadsoi;
  }
  // setWiFiEvent();
}
void runs() {
  static unsigned long lastRunLog = 0;
  if (millis() - lastRunLog >= 60000) {
    memlogTask("runs");
    lastRunLog = millis();
  }

  Foundjob *timejobs = js->loadjobByt();
  js->printFound("Time jobs:", timejobs);

  if (timejobs != NULL && configdata.havesht && hservice != NULL &&
      hservice->isReady()) {
    memlogTask("runJob", js->getsize());
    Foundjob *alljob = js->loadjobByh(hservice->read(), timejobs);
    js->printFound("All jobs:", alljob);
    taskservice->run(alljob);
    taskservice->updateExce();
    js->freeFoundjobs(alljob);
    js->freeFoundjobs(timejobs);
  } else if (timejobs != NULL) {
    js->freeFoundjobs(timejobs);
  }

  gpsservice->read();
  if (configdata.havesht && hservice != NULL && hservice->isReady())
    hservice->readInterval();
  // run stan alone
}
// มีการเข้ามาใน haveinsensor true แสดงว่าเกิด interrup sensor 1
volatile bool haveinsensor1 = false;
volatile unsigned long timenextcheck1; // บอกว่าระบบจะต้องตรวจสอบสถานะอีกครั้งเมื่อไหร่
/**
 * @brief ทำงานสำหรับ sensor จับการเคลือนไหวช่องแรก
 *
 */
void ICACHE_RAM_ATTR insensor1() {

  if (!haveinsensor1) {
    Serial.println("Have some one in sesnor");
    haveinsensor1 = true; // เข้ากระบวนการรอ sensor
    timenextcheck1 =
        millis() + (configdata.fastport0check * 1000); // เวลาที่รอการตรวจสอบอีกครั้ง
  }
}

/**
 * @brief สำหรับบอกว่าว่า port ไปตามที่กำหนดตามเวาลหรืไม่หรือไม่
 *
 * เช่นกำหนด D6 == 1 สองวิ  ถ้า D6 เป็น วิเดียวก็จะไม่แสดงไปที่ portstatus0 ใน make
 *
 */
void fastcheckport() {

  // เอาไว้ตรวจสอบว่าเป็น
  if (configdata.fastport0 != 0) {

    if (digitalRead(configdata.fastport0) == configdata.fastport0check &&
        configdata.fastport0nextcheck == 0) {
      // ถ้าเป็นไปตามที่กำหนดให้รอตามเวลา
      // delay(fastport0time * 1000);
      unsigned long now = millis();
      configdata.fastport0nextcheck =
          now + (configdata.fastport0time * 1000); // เวลาที่จะมา check อีกรอบ
      Serial.print("port " + String(configdata.fastport0) + " check time " +
                   String(configdata.fastport0time) + " Check logic is " +
                   String(configdata.fastport0check));
      Serial.println(" Now :" + String(now) +
                     " Next check : " + String(configdata.fastport0nextcheck));
    }

    if (configdata.fastport0nextcheck > 0 &&
        configdata.fastport0nextcheck <= millis() &&
        digitalRead(configdata.fastport0) ==
            configdata
                .fastport0check) { // ถ้าเวลาที่กำหนด น้อยกว่าหรือเท่ากับเวลาตรวจสอบจริง
                                   // ละ อ่านค่ายังได้เท่าเดิมหรือมีข้อมูลเข้ามาใช้เปลียน
                                   // status เป็น 1

      configdata.fastport0status = 1;
      memlogTask("fp0", 1);
      // digitalWrite(D3, 1);
      configdata.fastport0nextcheck = 0; // reset ไปเลยเพื่อนให้เข้า start loop ใหม่
      configdata.fastport0statusendtime =
          millis() +
          (configdata.fastport0statustime * 1000); // เวลาแสดงต่อไปจะกลับไปเป็น 0
      Serial.print(
          " In condition port  set fastport0 status to 1 and next 0 in " +
          String(configdata.fastport0statusendtime));
      Serial.println(" Status time " + String(configdata.fastport0statustime) +
                     " next end " + String(configdata.fastport0statusendtime));
    }

    if (configdata.fastport0nextcheck > 0 &&
        digitalRead(configdata.fastport0) != configdata.fastport0check) {

      configdata.fastport0nextcheck = 0;
      Serial.println("not in rang " + String(configdata.fastport0time));
    }

    if (configdata.fastport0statusendtime > 0 &&
        configdata.fastport0statusendtime <= millis()) {
      // digitalWrite(D3, 0);
      Serial.println("End fast status job");
      configdata.fastport0status = 0; // เปลียน status
      configdata.fastport0statusendtime = 0;
    }
  }

  if (configdata.fastport1 != 0) {

    if (digitalRead(configdata.fastport1) == configdata.fastport1check &&
        configdata.fastport1nextcheck == 0) {
      // ถ้าเป็นไปตามที่กำหนดให้รอตามเวลา
      // delay(fastport0time * 1000);
      configdata.fastport1nextcheck =
          millis() + (configdata.fastport1time * 1000); // เวลาที่จะมา check อีกรอบ
      Serial.print("port check time " + String(configdata.fastport1time));
      Serial.println(" Next check : " + String(configdata.fastport1nextcheck));
    }

    if (configdata.fastport1nextcheck > 0 &&
        configdata.fastport1nextcheck <= millis() &&
        digitalRead(p) ==
            configdata
                .fastport1check) { // ถ้าเวลาที่กำหนด น้อยกว่าหรือเท่ากับเวลาตรวจสอบจริง
                                   // ละ อ่านค่ายังได้เท่าเดิมหรือมีข้อมูลเข้ามาใช้เปลียน
                                   // status เป็น 1

      configdata.fastport1status = true;
      memlogTask("fp1", 1);
      configdata.fastport1nextcheck = 0; // reset ไปเลยเพื่อนให้เข้า start loop ใหม่
      configdata.fastport1statusendtime =
          millis() +
          (configdata.fastport1statustime * 1000); // เวลาแสดงต่อไปจะกลับไปเป็น 0
      Serial.print(
          " In condition port  set fastport 0 status to 1 and next 0 in " +
          String(configdata.fastport1statusendtime));
      Serial.println(" Status time " + String(configdata.fastport1statustime) +
                     " next end " + String(configdata.fastport1statusendtime));
    }

    if (configdata.fastport1nextcheck > 0 &&
        digitalRead(configdata.fastport1) != configdata.fastport1check) {

      configdata.fastport1nextcheck = 0;
      Serial.println("not in rang " + String(configdata.fastport1time));
    }

    if (configdata.fastport1statusendtime > 0 &&
        configdata.fastport1statusendtime <= millis()) {
      Serial.println("End fast status job");
      configdata.fastport1status = 0; // เปลียน status
      configdata.fastport1statusendtime = 0;
    }
  }
}

String fulldate() {
  time_t t = realtime();
  struct tm *timeinfo = localtime(&t);

  char buffer[80];

  // Format the time as "Thursday, August 28, 2025 16:33:25"
  // %A = Full weekday name
  // %B = Full month name
  // %d = Day of the month (padded)
  // %Y = Year with century
  // %H = Hour (24-hour)
  // %M = Minute (padded)
  // %S = Second (padded)
  strftime(buffer, sizeof(buffer), "%A, %B %d, %Y %H:%M:%S", timeinfo);
  return String(buffer);
}
void havefp() {
  if (configdata.havefastport) {
    static unsigned long lastFpLog = 0;
    if (millis() - lastFpLog >= 30000) {
      memlogTask("fp");
      lastFpLog = millis();
    }
    fastcheckport();
  }
}
/**
 * @brief จะ return ture เมืออยู่ในช่วง 6 เช้าถึง 6 โมงเย็น
 *
 * @return boolean
 */
boolean checkDaytime() {
  time_t now = realtime();

  struct tm *timeinfo = localtime(&now);

  // จะส่งค่าออกมาถ้าเป็นช่วงกลางวัน
  int h = timeinfo->tm_hour;
  if (h >= 6 && h <= 18) {
    return true;
  }

  return false;
}
void resetTed() {
  // tde = 0; // เอาออกไป reset ตอนเช้า
  firstted = 0;
}
// find TDE
void findTDE() {

  // ทำงานช่วง 6 - 18.00
  if (!checkDaytime()) {
    resetTed();
  } else {
    // ถ้า TDE  == 0
    if (firstted == 0) {
      firstted = e; // กำหนด e เป็นตัวแรก
      tde = 0;      // reset ตอนเช้า
    }

    tde = e - firstted;
  }
}
void updatetimefn() {
  if (updatetimecounter > configdata.updatetime) {
    memlogTask("timeUp");
    updatetimecounter = 0;
    updateTime();
  }
}

// Standalone function to check connectivity to 192.168.88.1 and reconnect if
// needed
bool checkConnectivityToGateway() {
  return checkAndReconnectToIP("192.168.88.1", 10000);
}

// Function to manually trigger connection check and reconnection
void manualConnectionCheck() {
  Serial.println("Manual connection check triggered");
  if (checkConnectivityToGateway()) {
    Serial.println("Manual connection check: SUCCESS");
  } else {
    Serial.println("Manual connection check: FAILED");
  }
}

void timetotm() {
  if (configdata.havetm) {
    time_t t = realtime();
    struct tm *timeinfo = localtime(&t);

    int hh = timeinfo->tm_hour;
    int mm = timeinfo->tm_min;
    static int lastMin = -1;
    if (mm != lastMin) {
      memlogTask("tm", hh * 100 + mm);
      lastMin = mm;
    }

    // แปลงเป็นเลข 4 หลัก HHMM
    int displayTime = hh * 100 + mm;

    // กะพริบจุดกลาง (ทุกครั้งที่เข้า loop ให้สลับ)

    uint8_t dots = dotState ? 0b01000000 : 0b00000000;

    tm1.showNumberDecEx(displayTime, dots, true);
    Serial.printf("%02d:%02d\n", hh, mm);
  }
}
void readSoivalue() {
  digitalWrite(configdata.soienablepin, HIGH);
  delay(500);
  int moisture = analogRead(soisensorPin);
  digitalWrite(configdata.soienablepin, LOW);
  a0value = moisture;
  Serial.print("Analog output: ");
  Serial.println(moisture);
  int moisturePercent = map(moisture, AirValue, WaterValue, 0, 100);
  Serial.print("H ");
  Serial.print(moisturePercent);
  pfHum = moisturePercent;
  Serial.println("%");

  char logmsg[28];
  snprintf(logmsg, sizeof(logmsg), "pct=%d raw=%d", moisturePercent, moisture);
  memlogAdd(MEMLOG_SOI, moisturePercent, logmsg);
}
void havesoi() {
  if (configdata.havesoisensor && millis() >= nextreadsoi) {
    memlogTask("soi");
    readSoivalue();
    nextreadsoi = millis() + configdata.nextreadsoi;
  }
}

void findwetair() {

  if (findsoinow) {
    memlogTask("findSoi");
    memlogAdd(MEMLOG_SOI, 0, "cal findsoi start");
    canuseled = 0;
    MoveAvg wet(15);
    MoveAvg air(15);

    digitalWrite(2, 0);
    for (int i = 0; i < 60; i++) {
      digitalWrite(2, !digitalRead(2));
      delay(200);
    }
    for (int i = 0; i < 15; i++) {
      wet.pushValue(analogRead(A0));
      delay(2000);
      digitalWrite(2, !digitalRead(2));
    }
    WaterValue = wet.av();
    cfg.addConfig("wetvalue", WaterValue);
    digitalWrite(2, 0);
    for (int i = 0; i < 60; i++) {
      digitalWrite(2, !digitalRead(2));
      delay(200);
    }
    for (int i = 0; i < 15; i++) {
      air.pushValue(analogRead(A0));
      delay(2000);
      digitalWrite(2, !digitalRead(2));
    }

    AirValue = air.av();
    cfg.addConfig("airvalue", AirValue);
    canuseled = 1;
    findsoinow = false;
    char logmsg[28];
    snprintf(logmsg, sizeof(logmsg), "cal wet=%d air=%d", WaterValue, AirValue);
    memlogAdd(MEMLOG_SOI, WaterValue, logmsg);
  }
}
void loop() {
  if (!configdata.stanalone) {
    havekey();
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
    a0task();
    oledtask();
    watertask();
    waterlimittask();
    readpzem();
    readGps();
    displayGpsData();
    havefp();
    updatetimefn();
    timetotm();
    havesoi();
    findwetair();
  } else {
    runs();
  }
}

boolean haveportrun() {
  for (int i = 0; i < ioport; i++) {
    if (ports[i].delay > 0)
      return true;
  }
  return false;
}