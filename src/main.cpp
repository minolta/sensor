#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266httpUpdate.h>

#include "Timer.h"
#include <max6675.h>

#include "KAnalog.h"
#define b_led 2 // 1 for ESP-01, 2 for ESP-12
const char *host = "endpoint.pixka.me:5002";
const char *version = "1.0-a09f802999d3a35610d5b4a11924f8fb";
const char *token = "a09f802999d3a35610d5b4a11924f8fb";
int count = 0;
//WiFiServer server(80); //กำหนดใช้งาน TCP Server ที่ Port 80
ESP8266WebServer server(80);
//#define ONE_WIRE_BUS D4
uint8_t deviceCount = 0;
float tempC;
Timer t;
KAnalog analog;
#define DHTPIN D3 // Pin which is connected to the DHT sensor.

// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11
#define DHTTYPE DHT22 // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht(DHTPIN, DHTTYPE);
int countsend = 0;
uint32_t delayMS;
float pfDew, pfHum, pfTemp, pfVcc;
float a0value;
float rawvalue = 0;

int ktcSO = 12;
int ktcCS = 13;
int ktcCLK = 14;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
ESP8266WiFiMulti WiFiMulti;
float ktypevalue = 0;
void readDHT()
{
    count++;
    Serial.print("count :");
    Serial.println(count);
    // Delay between measurements.
    // delay(delayMS);
    // Get temperature event and print its value.
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (isnan(event.temperature))
    {
        Serial.println("Error reading temperature!");
    }
    else
    {
        Serial.print("Temperature: ");
        Serial.print(event.temperature);
        Serial.println(" *C");
        pfTemp = event.temperature;
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
    }

    // tempC = sensors.getTempC(t);
}

void sendA0()
{

    if (rawvalue < 100)
    {
        Serial.println("Error value A0");
        return;
    }
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject &JSONencoder = JSONbuffer.createObject();
    JSONencoder["rawvalue"] = rawvalue;
    JSONencoder["pressurevalue"] = a0value;
    //pressurevalue
    // JSONencoder["t"] = pfTemp;
    // JSONencoder["h"] = pfHum;
    JsonObject &pidevice = JSONencoder.createNestedObject("device");
    pidevice["mac"] = WiFi.macAddress();

    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
    // put your main code here, to run repeatedly:
    HTTPClient http; //Declare object of class HTTPClient

    http.begin("http://endpoint.pixka.me:5002/pressure/add"); //Specify request destination
    http.addHeader("Content-Type", "application/json");       //Specify content-type header

    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();           //Get the response payload
    Serial.print(" Http Code:");
    Serial.println(httpCode); //Print HTTP return code
    Serial.print(" Play load:");
    Serial.println(payload); //Print request response payload

    http.end(); //Close connection
}

void sendKtype()
{
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject &JSONencoder = JSONbuffer.createObject();
    JSONencoder["mac"] = WiFi.macAddress();
    // JSONencoder["t"] = pfTemp;
    // JSONencoder["h"] = pfHum;
    JsonObject &dht = JSONencoder.createNestedObject("ds18value");
    dht["t"] = ktypevalue;

    JsonObject &sensor = dht.createNestedObject("ds18sensor");
    sensor["name"] = WiFi.macAddress();
    sensor["callname"] = WiFi.macAddress();

    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
    // put your main code here, to run repeatedly:
    HTTPClient http; //Declare object of class HTTPClient

    http.begin("http://endpoint.pixka.me:5002/ds18value/add"); //Specify request destination
    http.addHeader("Content-Type", "application/json");        //Specify content-type header

    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();           //Get the response payload
    Serial.print(" Http Code:");
    Serial.println(httpCode); //Print HTTP return code
    Serial.print(" Play load:");
    Serial.println(payload); //Print request response payload
    http.end();              //Close connection
}
void sendDht()
{
    readDHT();
    if (pfTemp == 0 || pfHum == 0)
    {
        return;
    }
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject &JSONencoder = JSONbuffer.createObject();
    JSONencoder["mac"] = WiFi.macAddress();
    // JSONencoder["t"] = pfTemp;
    // JSONencoder["h"] = pfHum;
    JsonObject &dht = JSONencoder.createNestedObject("dhtvalue");

    dht["t"] = pfTemp;
    dht["h"] = pfHum;

    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
    // put your main code here, to run repeatedly:
    HTTPClient http; //Declare object of class HTTPClient

    http.begin("http://endpoint.pixka.me:8081/dht/add"); //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header
    http.addHeader("Authorization", "Basic VVNFUl9DTElFTlRfQVBQOnBhc3N3b3Jk");
    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();           //Get the response payload
    Serial.print(" Http Code:");
    Serial.println(httpCode); //Print HTTP return code
    Serial.print(" Play load:");
    Serial.println(payload); //Print request response payload

    http.end(); //Close connection
}
void checkin()
{
    StaticJsonBuffer<300> JSONbuffer;
    JsonObject &JSONencoder = JSONbuffer.createObject();
    JSONencoder["mac"] = WiFi.macAddress();
    JSONencoder["password"] = "";
    JSONencoder["ip"] = WiFi.localIP().toString();
    // JSONencoder["t"] = pfTemp;
    // JSONencoder["h"] = pfHum;
    JsonObject &dht = JSONencoder.createNestedObject("dhtvalue");
    dht["t"] = pfTemp;
    dht["h"] = pfHum;

    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
    // put your main code here, to run repeatedly:
    HTTPClient http; //Declare object of class HTTPClient

    http.begin("http://endpoint.pixka.me:8081/checkin"); //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header
    http.addHeader("Authorization", "Basic VVNFUl9DTElFTlRfQVBQOnBhc3N3b3Jk");

    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();           //Get the response payload
    Serial.print(" Http Code:");
    Serial.println(httpCode); //Print HTTP return code
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
    json.prettyPrintTo(client);
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
JsonObject &prepareResponse(JsonBuffer &jsonBuffer)
{
    JsonObject &root = jsonBuffer.createObject();
    root["t"] = pfTemp;
    root["h"] = pfHum;
    return root;
}
/**
 * 
 * Run command คือการเปิด port ที่ส่งมา
 * */
void runCommand()
{
    String s = server.arg("port");
    Serial.println("Port:" + s);
    String set = server.arg("set");
    StaticJsonBuffer<300> jb;
    JsonObject &json = jb.createObject();
    json["add"] = "ok";
    json["port"] = s;
    json["set"] = set;
    char jsonChar[100];
    json.printTo((char *)jsonChar, json.measureLength() + 1);
    server.send(200, "application/json", jsonChar);
    Serial.println("Set port " + s + " to " + set);
    digitalWrite(s.toInt(), set.toInt());
}

float readKtype()
{
    float DC = ktc.readCelsius();
    ktypevalue = DC;
    Serial.print("K type value ");
    Serial.println(DC);
    return DC;
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
    float psi = analog.readPsi(0.50, 37.5);
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
void DHTtoJSON()
{
    digitalWrite(b_led, LOW);
    readDHT();
    digitalWrite(b_led, HIGH);
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject &json = prepareResponse(jsonBuffer);
    char jsonChar[100];
    json.printTo((char *)jsonChar, json.measureLength() + 1);
    server.send(200, "application/json", jsonChar);
}
void PressuretoJSON()
{
    digitalWrite(LED_BUILTIN, LOW);
    readA0();
    digitalWrite(LED_BUILTIN, HIGH);
    StaticJsonBuffer<300> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    //root["mac"] = WiFi.macAddress();
    root["rawvalue"] = rawvalue;
    root["pressurevalue"] = a0value;
    JsonObject &device = root.createNestedObject("device");
    device["mac"] = WiFi.macAddress();
    char jsonChar[200];
    root.printTo((char *)jsonChar, root.measureLength() + 1);
    server.send(200, "application/json", jsonChar);
}
void KtypetoJSON()
{
    digitalWrite(D3, 1);
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    //root["mac"] = WiFi.macAddress();
    // root["mac"] = WiFi.macAddress();
    //root["pressurevalue"] = a0value;
    // JsonObject &ds18value = root.createNestedObject("ds18value");
    root["t"] = readKtype();

    JsonObject &pidevice = root.createNestedObject("pidevice");
    pidevice["mac"] = WiFi.macAddress();

    JsonObject &ds18sensor = root.createNestedObject("ds18sensor");
    ds18sensor["name"] = WiFi.macAddress();
    ds18sensor["callname "] = WiFi.macAddress();

    JsonObject &device = root.createNestedObject("device");
    device["mac"] = WiFi.macAddress();

    char jsonChar[500];
    root.printTo((char *)jsonChar, root.measureLength() + 1);
    server.send(200, "application/json", jsonChar);
    // server.close();
    digitalWrite(D3, 0);
}
void info()
{
    Serial.print("Mac:");
    Serial.println(WiFi.macAddress());
    Serial.print("IP:");
    Serial.println(WiFi.localIP());
    Serial.print("Version:");
    Serial.println(version);

    StaticJsonBuffer<500> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    //root["mac"] = WiFi.macAddress();
    // root["mac"] = WiFi.macAddress();
    //root["pressurevalue"] = a0value;
    // JsonObject &ds18value = root.createNestedObject("ds18value");
    root["mac"] = WiFi.macAddress();
    root["IP"] = WiFi.localIP().toString();
    root["version"] = version;
    char jsonChar[500];
    root.printTo((char *)jsonChar, root.measureLength() + 1);
    server.send(200, "application/json", jsonChar);
}
void ota()
{

    t_httpUpdate_return ret = ESPhttpUpdate.update("endpoint.pixka.me", 5002, "/espupdate", version);
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
void senddata()
{
    digitalWrite(b_led, 1);
    checkin();
    //readDHT();
    //sendDht();
    //sendA0();
    //sendKtype();
    ota();
    digitalWrite(b_led, 0);

    // readKtype();
}

void connect()
{
}
void setup()
{

    pinMode(b_led, OUTPUT); //On Board LED
    // pinMode(LED_BUILTIN, OUTPUT);
    //pinMode(D6, OUTPUT);
    //pinMode(D5, OUTPUT);
    // pinMode(D1, OUTPUT);
    //  pinMode(D3, OUTPUT);
    Serial.begin(9600);
    Serial.println();
    Serial.println();

    // connect();
    WiFiMulti.addAP("Sirifarm", "0932154741");
    WiFiMulti.addAP("pksy", "04qwerty");
    WiFiMulti.addAP("SP", "04qwerty");

    while (WiFiMulti.run() != WL_CONNECTED) //รอการเชื่อมต่อ
    {
        delay(500);
        Serial.print(".");
    }

    server.on("/dht", DHTtoJSON);
    server.on("/pressure", PressuretoJSON);
    server.on("/command", runCommand);
    server.on("/ktype", KtypetoJSON);
    server.on("/info", info);
    server.begin(); //เปิด TCP Server
    Serial.println("Server started");

    Serial.println(WiFi.localIP()); // แสดงหมายเลข IP ของ Server
    String mac = WiFi.macAddress();
    Serial.println(mac); // แสดงหมายเลข IP ของ Server
    // Initialize device.
    dht.begin();
    t.every(60000, senddata);
}
void inden()
{
    digitalWrite(b_led, 0);
    delay(500);
    digitalWrite(b_led, 1);
    delay(500);
}
void loop()
{

    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status

        t.update();
        server.handleClient();
        digitalWrite(b_led, 0);
        delay(500);
        digitalWrite(b_led, 1);
        delay(500);
        // httpService();
    }
    else
    {
        Serial.println("Error in WiFi connection");
    }
}
