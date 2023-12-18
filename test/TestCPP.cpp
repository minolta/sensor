#include <Arduino.h>
#include <unity.h>
#include "Configfile.h"
#include "./DNSServer.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
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
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include "checkconnection.h"
#include <ESP8266httpUpdate.h>
#include "Espjob.h"
#include <TinyGPSPlus.h>
// #include <NMEAGPS.h>
#include <SoftwareSerial.h>
#include "Hjob.h"
#include "Runjob.h"
#include "html.h"
#include <time.h>
#include <stdio.h>
static const int RXPin = D7, TXPin = D8;
static const uint32_t GPSBaud = 9600;
unsigned long updatetime; // เป็นตัวบอกว่าเวลาที่รับ timestamp จาก gps
// The TinyGPSPlus object
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);
time_t timeSinceEpoch;
void displayInfo();
void testConverttime();
// #include <ESP8266WebServer.h>
Configfile test("/testconfig.cfg");
String responseHTML = "<!DOCTYPE html>"
                      "<html lang=\"en\">"
                      "<head>"
                      "<meta charset=\"utf-8\">"
                      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                      "<title>Internet of Bottles</title>"
                      "</head>"
                      "<body>"
                      "<p>I'm just a stupid bottle with WiFi.</p>"
                      "</body>"
                      "</html>";

const char getpassword_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Config file wifi</h2>
  <form action="/setpassword" method="POST" enctype="application/x-www-form-urlencoded">
  <input type="text" name="ssid" >
  <input type="password" name="password">
   <input type="submit" value="Send">
</form>
</body>
</html>)rawliteral";
void connect()
{
  WiFi.begin("forpi", "04qwerty");
  while (WiFi.status() != WL_CONNECTED) // รอการเชื่อมต่อ
  {
    Serial.print(".");
    delay(1000);
  }
}
void testEspjobtofile()
{
  int buffersize = 2024;
  Job o;
  if (!LittleFS.begin())
  {
    Serial.println("ERROR LITTLEFS");
  }
  File configfile = LittleFS.open("/j.job", "w");

  Espjob *j = new Espjob();
  j->id = 1;
  j->hlow = 10;
  j->hhigh = 80;

  o.addJob(j);

  j = new Espjob();
  j->id = 2;
  j->hlow = 10;
  j->hhigh = 80;

  o.addJob(j);
  TEST_ASSERT_EQUAL(o.getsize(), 2);

  j = new Espjob();
  j->id = 3;
  j->hlow = 10;
  j->hhigh = 80;

  o.addJob(j);

  Serial.println(o.toString());
  if (configfile)
  {
    String k = o.toString();
    Serial.println("Save");
    Serial.println(k);
    configfile.print(k);
    delay(1);
    configfile.close();
  }
  else
  {
    Serial.println("Can not open file");
  }

  File file = LittleFS.open("/j.job", "r");
  String re = file.readString();
  Serial.println("=============");
  Serial.println(re);
}
void split(String data)
{
  const char s[2] = ",";
  char *token;
  int size = data.length();
  char str[size];
  data.toCharArray(str, size);

  Serial.printf("have %s", str);
  // token = strtok(str,s);

  // while(token!=NULL)
  // {
  //   Serial.printf("%s",String(token));
  //   token = strtok(NULL,s);
  // }

  int id = 0;
  // Espjob *p = new Espjob();
  // sscanf(str,"%d,",id);
  // Serial.println(id);

  char *pp = "1";
  sscanf(pp, "%d", id);
  Serial.println(id);
}
void testSscanf()
{
  String ss = "1,20.22,22.22,4,1000";

  char *data = "[08:30:15]1.2.3.4=99";
  // char p[] ="[08:30:15]1.2.3.4=99";
  char p[ss.length()];

  ss.toCharArray(p, ss.length());

  // Serial.println(data);
  int id;
  float hl, hh;
  int port, runtime;
  Espjob *ej = new Espjob();
  sscanf(p, "%d,%f,%f,%d,%d", &ej->id, &ej->hlow, &ej->hhigh, &ej->p, &runtime);
  Serial.printf("%d= %.2f= %.2f = %d = %d\n", ej->id, hl, hh, port, runtime);
}
void testConvert()
{
  Job j = Job();

  j.load("j.job");

  Serial.println(j.toString());

  /* get the first token */

  /* walk through other tokens */
}
void testEspjob()
{

  Espjob *data;
  data = NULL;

  for (int i = 0; i < 10; i++)
  {
    Espjob *n = new Espjob();
    n->id = i;
    n->runtime = 10 * i;
    if (data == NULL)
    {
      data = n;
      data->p = NULL;
    }
    else
    {
      data->n = n;
      n->p = data;
      data = n;
    }
  }

  for (; data != NULL;)
  {

    Serial.println(data->id);
    data = data->p;
  }
}
void testSetConfig(void)
{

  test.addConfig("a", "x");
  String a = test.getConfig("a", "y");
  TEST_ASSERT_EQUAL_STRING("x", a.c_str());
}

void updateRTC()
{
  connect();
  RtcDS3231<TwoWire> rtcObject(Wire);
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP);
  timeClient.begin();
  timeClient.setTimeOffset(25200); // Thailand +7 = 25200
  timeClient.forceUpdate();
  rtcObject.Begin();

  String formattedDate = timeClient.getFormattedDate();
  Serial.println(formattedDate);

  // Extract date
  int splitT = formattedDate.indexOf("T");
  String dayStamp = formattedDate.substring(0, splitT);
  Serial.print("DATE: ");
  Serial.println(dayStamp);
  // Extract time
  String timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  Serial.print("HOUR: ");
  Serial.println(timeStamp);
  long t = timeClient.getEpochTime();
  Serial.println(t);
  RtcDateTime manual = RtcDateTime(t - 946659600 - 25200);
  // RtcDateTime manual = RtcDateTime(t);
  rtcObject.SetDateTime(manual);
}
void readRTC()
{
  Wire.begin();
  RtcDS3231<TwoWire> rtcObject(Wire);
  rtcObject.Begin();
  RtcDateTime currentTime = rtcObject.GetDateTime(); // get the time from the RTC

  char str[20]; // declare a string as an array of chars
  Serial.println(currentTime.Epoch64Time());
  sprintf(str, "%d/%d/%d %d:%d:%d", //%d allows to print an integer to the string
          currentTime.Year(),       // get year method
          currentTime.Month(),      // get month method
          currentTime.Day(),        // get day method
          currentTime.Hour(),       // get hour method
          currentTime.Minute(),     // get minute method
          currentTime.Second()      // get second method
  );
  Serial.println(str); // print the string to the serial port

  int m = currentTime.Minute();
  int h = currentTime.Hour();
  int s = currentTime.Second();
  int Y = currentTime.Year();
  int M = currentTime.Month();
  int d = currentTime.Day();
}
void testRead(void)
{
  String a = test.getConfig("a", "y");
  TEST_ASSERT_EQUAL_STRING("x", a.c_str());
}
const byte DNS_PORT = 53;      // Capture DNS requests on port 53
IPAddress apIP(10, 10, 10, 1); // Private network for server
DNSServer dnsServer;           // Create the DNS object
                               // ESP8266WebServer webServer(80); // HTTP server
AsyncWebServer webServer(80);
void testDns()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("IoT --- Free WiFi"); // WiFi name

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // replay to all requests with same HTML
  // webServer.onNotFound([]()
  //                      { webServer.send(200, "text/html", responseHTML); });

  webServer.onNotFound([](AsyncWebServerRequest *request)
                       { request->send(200, "text/html", getpassword_html); });
  webServer.begin();

  test.openFile();
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  while (true)
  {
    dnsServer.processNextRequest();
  }
  delay(2000);
}
void ota()
{
  WiFi.begin("forpi", "04qwerty");
  while (WiFi.status() != WL_CONNECTED) // รอการเชื่อมต่อ
  {
    Serial.print(".");
    delay(1000);
  }
  WiFiClient client;
  String url = "http://192.168.88.5:2005/rest/fw/update/sensor/1";
  Serial.println(url);

  t_httpUpdate_return ret = ESPhttpUpdate.update(client, url);
  Serial.println("return " + ret);
  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.println("[update] Update failed.");
    break;
  case HTTP_UPDATE_NO_UPDATES:

    Serial.println("[update] Update no Update.");
    break;
  case HTTP_UPDATE_OK:
    Serial.println("[update] Update ok."); // may not called we reboot the ESP
    break;
  }
}
void testCheckin()
{
  WiFi.begin("forpi", "04qwerty");
  while (WiFi.status() != WL_CONNECTED) // รอการเชื่อมต่อ
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" +++++++++++++++++++++ Check in now ++++++++++++++++++++++++++++++++++++");
  DynamicJsonDocument dy(1024);

  dy["freemem"] = system_get_free_heap_size();
  dy["version"] = 1;
  dy["name"] = "for test";
  dy["ip"] = "10.10.10.1";
  dy["mac"] = "ff:ff:ff:ff:ff";
  dy["ssid"] = WiFi.SSID();
  dy["password"] = "";
  char buf[1024];
  serializeJsonPretty(dy, buf, 1024);
  WiFiClient client;
  Serial.println(buf);
  // put your main code here, to run repeatedly:
  HTTPClient http;
  String url = "http://fw1.pixka.me:3333/checkin"; // Declare object of class HTTPClient
  http.begin(client, url);                         // Specify request destination
  Serial.println(url);
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
    JsonObject obj = dy.as<JsonObject>();
    Serial.print("---------------------------------------------------------------");
    Serial.println(obj);
    Serial.print("---------------------------------------------------------------");
    String name = obj["pidevice"]["name"].as<String>();
  }
  // Serial.print(" Play load:");
  // Serial.println(payload); // Print request response payload
  http.end(); // Close connection
}
void checkconn()
{
  WiFi.begin("forpi", "04qwerty");
  while (WiFi.status() != WL_CONNECTED) // รอการเชื่อมต่อ
  {
    Serial.print(".");
    delay(200);
  }
  Configfile cfg("/config1.cfg");
  cfg.openFile();
  cfg.addConfig("talkurl", "http://192.168.88.2");

  int re = talktoServer("192.168.88.99", "ky", 20000, &cfg);
  Serial.print("Result:");
  Serial.print(re);
}
void testPointer()
{
  long long hex = 0x1a1b2a2b3a3b4a4bULL;
  char *ptr = (char *)&hex;
  int i;
  int j;
  for (i = 1, j = 0; i < 8, j < 7; i += 2, j += 2)
  {
    printf("0x%hx%hx at address %p \n", ptr[i], ptr[j], (void *)ptr + i);
  }
}
void testI2cscan()
{
  Wire.begin();
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}
void testSht()
{
  SHTSensor sht;
  Wire.begin();
  // delay(100);
  if (sht.init())
  {
    Serial.print("SHT init(): success\n");
    sht.setAccuracy(SHTSensor::SHT_ACCURACY_MEDIUM); // only supported by SHT3x
  }
  else
  {
    Serial.print("SHT init(): failed\n");
  }

  if (sht.readSample())
  {
    float pfHum = sht.getHumidity();
    float pfTemp = sht.getTemperature();
    Serial.println("Read SHT H:" + String(pfHum) + " T:" + String(pfTemp));
  }
  else
  {
    Serial.println("SHT ERROR");
  }
}
// TinyGPSPlus gps;
// SoftwareSerial ss(D6, D5); // RX, TX

void testGPS()
{

  // The serial connection to the GPS device

  Serial.println(F("DeviceExample.ino"));
  Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
  Serial.print(F("Testing TinyGPSPlus library v. "));
  Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();
  while (ss.available() > 0)
    if (gps.encode(ss.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    // while (true)
    //   ;
  }
}
unsigned long date, tttt;
String gpsdate = "";
String gpstime = "";
String gpsdatetime = "";
String gpsloc = "";
String gpsraw = "";
void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    gpsloc = "";
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
    gpsloc = "Location: LAT:" + String(gps.location.lat()) + " LNG:" + String(gps.location.lng());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));

  if (gps.date.isValid())
  {
    date = gps.date.value();
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
    char b[100];
    sprintf(b, "%d-%d-%d", gps.date.day(), gps.date.month(), gps.date.year());
    gpsdate = String(b);
  }
  else
  {
    gpsdate = NULL;
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    tttt = gps.time.value();
    if (gps.time.hour() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10)
      Serial.print(F("0"));
    Serial.print(gps.time.centisecond());

    char b[100];
    sprintf(b, "%d:%d:%d", gps.time.hour(), gps.time.minute(), gps.time.second());
    gpstime = String(b);
  }
  else
  {
    Serial.print(F("INVALID"));
    gpstime = NULL;
  }

  if (gpsdate != NULL && gpstime != NULL)
  {
    char buf[500];
    sprintf(buf, "%d-%d-%d %d:%d:%d", gps.date.day(), gps.date.month(), gps.date.year(), gps.time.hour(), gps.time.minute(),
            gps.time.second());
    gpsdatetime = String(buf);

    testConverttime();
  }
  Serial.println();
}
void testfindByid()
{
  Job j = Job();
  j.load("/j.job");

  Espjob *p = j.findById(2);

  TEST_ASSERT(p != NULL);

  p = j.findById(5);

  TEST_ASSERT(p == NULL);
}
void testHtask()
{

  Htask *t = new Htask();
  Runjob *rj = new Runjob();
  Espjob *job = new Espjob();

  t->init();
  Hdata *hd;
  t->read(hd);
  Serial.printf("\nH:%.2f T:%.2f\n", hd->h, hd->t);
  rj->setHtask(t);
  job->hlow = 0;
  job->hhigh = 100;
  rj->run(job);
  bool re = rj->checkh(job);
  TEST_ASSERT(re);
  // job = new Espjob();
  job->hlow = 102.20;
  job->hhigh = 200.22;
  // Serial.printf("\nHL:%.2f HH:%.2f\n", job->hlow, job->hhigh);
  // rj->run(job);
  Serial.printf("\nGetH:%f ", t->geth());
  Espjob *get = rj->getJob();
  Serial.printf("\n%f <= %f <= %f", get->hlow, t->geth(), get->hhigh);
  int iii = rj->checkh(job);
  // if(rj->checkh(job))
  // {
  //   Serial.println("ok");
  // }
  // else
  // Serial.println("Not ok");
  // Serial.println(iii);
  TEST_ASSERT(!iii);
}

void testEspjobRun()
{
  pinMode(D4, OUTPUT);
  digitalWrite(D4, 0);
  Htask *t = new Htask();
  Runjob *rj = new Runjob();
  Espjob *job = new Espjob();

  t->init();
  Hdata *hd;
  t->read(hd);
  Serial.printf("\nH:%.2f T:%.2f\n", hd->h, hd->t);
  rj->setHtask(t);
  job->hlow = 0;
  job->hhigh = 100;
  job->port = D4;
  job->runtime = 10;
  job->waittime = 0;
  job->out = 0;
  if (rj->run(job))
  {
    while (rj->isRun())
    {
      rj->update();
      delay(1);
    }
  }
}
void testDeleteEspjob()
{
  Job *jobs = new Job();
  long m1 = system_get_free_heap_size();

  Serial.println(system_get_free_heap_size());

  for (int i = 0; i < 10; i++)
  {
    Espjob *j = new Espjob();
    j->id = i;
    jobs->addJob(j);
  }
  Serial.println(system_get_free_heap_size());
  long m2 = system_get_free_heap_size();

  // Serial.println(jobs->toString());
  Serial.println(jobs->getsize());

  Espjob *p = jobs->findById(3);
  jobs->deletejob(p);

  Serial.println(jobs->getsize());
  Serial.println(jobs->toString());
  Serial.println("-=========================-");
  p = jobs->findById(0);
  jobs->deletejob(p);
  Serial.println(jobs->toString());
  Serial.println("-=========================-");
  p = jobs->findById(9);
  jobs->deletejob(p);
  Serial.println(jobs->toString());

  Serial.println("-=========================-");
  p = jobs->findById(6);
  jobs->deletejob(p);
  Serial.println(jobs->toString());
  Serial.println(system_get_free_heap_size());

  long m3 = system_get_free_heap_size();

  Serial.printf("\n%d %d %d\n", m1, m2, m3);

  p = new Espjob();
  p->id = 2;
  p->hhigh = 100;
  p->hlow = 1000;
  jobs->edit(p);

  Serial.println(jobs->toString());
}
void testDirectrun()
{
  Runjob *run = new Runjob();
  Espjob *j = new Espjob();
  j->id = 1;
  j->runtime = 3;
  j->waittime = 1;
  j->out = 0;
  j->port = D4;

  run->directrun(j);
  while (run->isRun())
  {
    /* code */
    run->update();
    delay(1);
  }
}
AsyncWebServer server(80);
Job *js = new Job();
void testAddjobviawww()
{

  IPAddress apIP(10, 10, 10, 1); // Private network for server
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("TestAddjob", "12345678"); // WiFi name

  server.on("/addjob", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/html", addjob_html); });
  server.on("/savejob", HTTP_GET, [](AsyncWebServerRequest *request)
            { 
               String port = request->getParam("port")->value();
               String hlow = request->getParam("hlow")->value();
               String hhigh = request->getParam("hhigh")->value();
               String runtime = request->getParam("runtime")->value();
               String waittime = request->getParam("waittime")->value();
               String out = request->getParam("output")->value();
                Serial.println(request->getParam("port")->value());
                char buf [2024];
                sprintf(buf, "%d,%.2f,%.2f,%d,%d,%d,%d,%d|", js->getsize()+1, hlow, hhigh, port, runtime, waittime, 1, out);
                js->addJob(js->n(buf));
                sprintf(buf,"size:%d Port:%s runttime: %s waittime: %s OUT:%s", js->getsize() ,port,runtime,waittime,out);
                request->send(200, "text/html",buf); });
  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(200, "text/html", addjob_html); });
  server.begin();
}

String filllist(const String &var)
{
  // Serial.println(var);
  char buf[2024];
  String tr = "";
  if (var == "list")
  {

    Serial.println("Fill list");
    Espjob *index = js->getfirst();
    // tr += "<tr><td>" + String(js->getsize()) + "</td></tr>";
    while (index != NULL)
    {
      Serial.println(index->id);
      sprintf(buf, "<tr><td>%d</td><td>%.2f</td><td>%.2f</td><td>%d</td><td>%d</td><td>%d</td></tr>\n",
              index->id, index->hlow, index->hhigh, index->port, index->runtime, index->waittime);
      tr += String(buf);

      index = index->n;
    }
  }
  if (var == "CONFIG")
  {
    Configfile cfg = Configfile("/config.cfg");
    cfg.setbuffer(2024);
    cfg.openFile();

    // cfg.load();
    cfg.addConfig("test-add", "11111");
    DynamicJsonDocument dy = cfg.getAll();

    JsonObject documentRoot = dy.as<JsonObject>();
    serializeJsonPretty(documentRoot, Serial);
    for (JsonPair keyValue : documentRoot)
    {
      Serial.println(keyValue.key().c_str());
      Serial.println(keyValue.value().as<const char *>());

      // String v = dy[keyValue.key()];
      // String k = keyValue.key().c_str();
      String v = keyValue.value().as<const char *>();
      String k = keyValue.key().c_str();
      tr += "<tr><td>" + k + "</td><td> <label id=" + k + "value>" + v + "</label> </td> <td> <input id = " + k + " value =\"" + v + "\"></td><td><button id=btn onClick=\"setvalue(this,'" + k + "','" + v + "')\">Set</button></td><td><button id=btn onClick=\"remove('" + k + "')\">Remove</button></td></tr>";
    }
    tr += "<tr><td>heap</td><td colspan=4>" + String(ESP.getFreeHeap()) + "</td></tr>";
  }

  return tr;
}
void testloadjob()
{
  js->load("/j.job");
  Serial.print("testList ");
  Serial.println(js->toString());
  Espjob *p = js->getfirst();

  while (p != NULL)
  {
    Serial.println(p->id);
    p = p->n;
  }

  p = js->getfirst();
  Serial.println(p->id);
}
void testFill()
{
  String v = filllist("CONFIG");
  Serial.println(v);

  // v = filllist("CONFIG");
  // Serial.println(v);
}
void testListjobs()
{

  IPAddress apIP(10, 10, 10, 1); // Private network for server
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("TEstlistjob", "12345678"); // WiFi name

  server.on("/listjobs", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", addjob_html, filllist); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(200, "text/html", addjob_html); });
  server.begin();
}
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
/**
 * @brief
 *
 * @return unsigned long
 */
unsigned long getEP()
{
  return timeSinceEpoch + ((millis() - updatetime) / 1000);
}
void testNTP()
{
  timeClient.setUpdateInterval(3600 * 6);
  timeClient.begin();
  // timeClient.setTimeOffset(25200);
  IPAddress apIP(10, 10, 10, 1); // Private network for server
  Serial.begin(9600);
  pinMode(2, OUTPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("TEstNTP", "12345678"); // WiFi name

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request)

            { 
              
              
             unsigned long t = getEP();
            //  unsigned long t1 = timeSinceEpoch;
            String bb = String(timeSinceEpoch);
             String b = "<i>"+bb+"</i> <b> "+String(t)+" </b> | GPS datetime: "+gpsdatetime +" Gps date:"+gpsdate+" Gps time:"+gpstime
             +" "+gpsloc;
              request->send(200, "text/html", b ); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    {  unsigned long t = timeClient.getEpochTime();
                      request->send(200, "text/html", String (t)); });
  server.begin();
}

void testConverttime()
{
  struct tm tm = {0};
  char buf[100];
  // timeClient.setEpochTime(0);
  // timeClient.forceUpdate();

  // Convert to tm struct
  // strptime("2023-11-12 18:31:01", "%Y-%m-%d %H:%M:%S", &tm);
  strptime(gpsdatetime.c_str(), "%d-%m-%Y %H:%M:%S", &tm);
  timeSinceEpoch = mktime(&tm);
  updatetime = millis();
  // timeClient.setEpochTime(timeSinceEpoch);
  Serial.println(timeSinceEpoch);

  // Can convert to any other format
  strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
  Serial.printf("%s", buf);

  struct tm startt = {0};
  strptime("15-12-2023 5:00:00", "%d-%m-%Y %H:%M:%S", &startt);
  Serial.println(mktime(&startt));
}

void testtimejobtotimestamp()
{
  time_t t = 1702730657;

  struct tm *tt = gmtime(&t);
  char buf[100];
  strftime(buf, sizeof(buf), "%d-%m-%Y", tt);

  Serial.println(buf);
  String bu = String(buf) + " 5:00:00";
  struct tm startt = {0};
  char bbb[100];
  bu.toCharArray(bbb,bu.length());
  strptime(bbb, "%d-%m-%Y %H:%M:%S", &startt);
  Serial.println( mktime(&startt));
}
void setup()
{

  Serial.begin(9600);
  pinMode(2, OUTPUT);
  // delay(2000);
  ss.begin(GPSBaud);
  UNITY_BEGIN();
  // RUN_TEST(checkconn);
  // RUN_TEST(testCheckin);
  // RUN_TEST(testSetConfig);
  // RUN_TEST(testRead);
  // RUN_TEST(Apmoderun);
  // RUN_TEST(ota);
  // RUN_TEST(testSht);
  // RUN_TEST(updateRTC);
  // RUN_TEST(readRTC);
  // RUN_TEST(testEspjob);
  // RUN_TEST(testPointer);
  // RUN_TEST(testI2cscan);
  // RUN_TEST(testGPS);
  // RUN_TEST(testEspjobtofile);
  // RUN_TEST(testConvert);
  // RUN_TEST(testSscanf);
  // RUN_TEST(testfindByid);
  // RUN_TEST(testHtask);
  // RUN_TEST(testEspjobRun);
  // RUN_TEST(testDeleteEspjob);
  // RUN_TEST(testDirectrun);
  // RUN_TEST(testAddjobviawww);
  // js->load("/j.job");
  // RUN_TEST(testListjobs);

  // RUN_TEST(testloadjob);
  // RUN_TEST(testFill);
  // RUN_TEST(testNTP);
  // RUN_TEST(testConverttime);
  RUN_TEST(testtimejobtotimestamp);

  UNITY_END();
}

void loop()
{

  digitalWrite(2, !digitalRead(2));
  delay(300);

  // while (ss.available() > 0)
  //   if (gps.encode(ss.read()))
  //     displayInfo();

  // if (millis() > 1000 && gps.charsProcessed() < 10)
  // {
  //   Serial.println(F("No GPS detected: check wiring."));
  //   // while (true)
  //   //   ;
  // }
}