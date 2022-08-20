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
void testSetConfig(void)
{

    test.addConfig("a", "x");
    String a = test.getConfig("a", "y");
    TEST_ASSERT_EQUAL_STRING("x", a.c_str());
}
// DNSServer dns;
// void Apmoderun()
// {
//     const byte DNS_PORT = 53;
//     IPAddress ip(10, 10, 10, 1);
//     Serial.println("Go to AP mode");
//     AsyncWebServer server(80);
//     const char *ssid1 = "ESP8266-Access-Point";
//     const char *password1 = "123456789";
//     WiFi.mode(WIFI_AP);
//     WiFi.softAPConfig(ip, ip, IPAddress(255, 255, 255, 0));

//     dns.start(DNS_PORT, "*", ip);

//     WiFi.softAP("ESP_SENSOR");

//     IPAddress IP = WiFi.softAPIP();
//     Serial.print("AP IP address: ");
//     Serial.println(IP);
//     server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
//               { request->send(200, "text/html", getpassword_html); });

//     server.on("/setpassword", HTTP_POST, [](AsyncWebServerRequest *request)
//               {
//               String password = request->arg("password");
//               String ssid = request->arg("ssid");
//             //   cfg.addConfig("ssid",ssid);
//             //   cfg.addConfig("password",password);
//               request->send(200, "text/html", "Ok SSID " + ssid + " Password " + password); });

//     server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request)
//               {
//               request->send(200, "text/html", "Re start");
//                ESP.restart(); });
//     server.begin();
// }
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
void setup()
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
   while(true)
   {
     dnsServer.processNextRequest();
   }
    delay(2000);

    UNITY_BEGIN();
    // RUN_TEST(testSetConfig);
    // RUN_TEST(testRead);
    // RUN_TEST(Apmoderun);
    UNITY_END();
}

void loop()
{
   
    // webServer.handleClient();
}