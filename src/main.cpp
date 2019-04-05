#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266httpUpdate.h>

#include "Timer.h"
#include <max6675.h>
#include <Ticker.h>
#include "KAnalog.h"
#include <Q2HX711.h>

class Portio
{
  public:
    int port;
    int value;
    int delay;
    int waittime;
    int run = 0;
    String closetime;
    Portio *n;
    Portio *p;
};
Portio ports[5];
#define b_led 2 // 1 for ESP-01, 2 for ESP-12
const char *host = "endpoint.pixka.me:5002";
const char *version = "1.0-a09f802999d3a35610d5b4a11924f8fb";
const char *token = "a09f802999d3a35610d5b4a11924f8fb";
int count = 0;
//WiFiServer server(80); //กำหนดใช้งาน TCP Server ที่ Port 80
ESP8266WebServer server(80);
//#define ONE_WIRE_BUS D4
OneWire ds(D4); // on pin D4 (a 4.7K resistor is necessary)
uint8_t deviceCount = 0;
float tempC;
Timer t;
KAnalog analog;
#define DHTPIN D3 // Pin which is connected to the DHT sensor.
#define cccc D6;
const byte hx711_data_pin = D1;
const byte hx711_clock_pin = D2;
Q2HX711 hx711(hx711_data_pin, hx711_clock_pin);
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

int ktcSO = D6;
int ktcCS = D7;
int ktcCLK = D5;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
ESP8266WiFiMulti WiFiMulti;
float ktypevalue = 0;
Ticker flipper;
void setport()
{
    ports[0].port = D1;
    ports[1].port = D2;
    ports[2].port = D5;
    ports[3].port = D6;
    ports[4].port = D7;
    //   ports[5].port = D8;
}
void addTorun(int port, int delay, int value, int wait)
{

    for (int i = 0; i < 6; i++)
    {
        if (ports[i].port == port)
        {
            pinMode(port,OUTPUT);
            // if (!ports[i].run)
            // {
            ports[i].value = value;
            ports[i].delay = delay;
            ports[i].waittime = wait;
            ports[i].run = 1;
            digitalWrite(ports[i].port, value);
            // }
            // else
            // {
            //   Serial.println("this port running");
            // }
        }
    }
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
    int value = v.toInt();

    //int d = server.arg("delay").toInt();
    int port = getPort(p);
    addTorun(port, d.toInt(), v.toInt(), w.toInt());
    // digitalWrite(port, value);
    server.send(200, "application/json", "ok");
    // delay(d.toInt() * 1000);
    // digitalWrite(port, !value);
    // delay(w.toInt() * 1000);
    // busy = false;
}
void readDS()
{
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte addr[8];
    float celsius, fahrenheit;

    if (!ds.search(addr))
    {
        Serial.println("No more addresses.");
        Serial.println();
        ds.reset_search();
        delay(250);
        return;
    }

    Serial.print("ROM =");
    for (i = 0; i < 8; i++)
    {
        Serial.write(' ');
        Serial.print(addr[i], HEX);
    }

    if (OneWire::crc8(addr, 7) != addr[7])
    {
        Serial.println("CRC is not valid!");
        return;
    }
    Serial.println();

    // the first ROM byte indicates which chip
    switch (addr[0])
    {
    case 0x10:
        Serial.println("  Chip = DS18S20"); // or old DS1820
        type_s = 1;
        break;
    case 0x28:
        Serial.println("  Chip = DS18B20");
        type_s = 0;
        break;
    case 0x22:
        Serial.println("  Chip = DS1822");
        type_s = 0;
        break;
    default:
        Serial.println("Device is not a DS18x20 family device.");
        return;
    }

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end

    delay(1000); // maybe 750ms is enough, maybe not
    // we might do a ds.depower() here, but the reset will take care of it.

    present = ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read Scratchpad

    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" ");
    for (i = 0; i < 9; i++)
    { // we need 9 bytes
        data[i] = ds.read();
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    Serial.print(" CRC=");
    Serial.print(OneWire::crc8(data, 8), HEX);
    Serial.println();

    // Convert the data to actual temperature
    // because the result is a 16 bit signed integer, it should
    // be stored to an "int16_t" type, which is always 16 bits
    // even when compiled on a 32 bit processor.
    int16_t raw = (data[1] << 8) | data[0];
    if (type_s)
    {
        raw = raw << 3; // 9 bit resolution default
        if (data[7] == 0x10)
        {
            // "count remain" gives full 12 bit resolution
            raw = (raw & 0xFFF0) + 12 - data[6];
        }
    }
    else
    {
        byte cfg = (data[4] & 0x60);
        // at lower res, the low bits are undefined, so let's zero them
        if (cfg == 0x00)
            raw = raw & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20)
            raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40)
            raw = raw & ~1; // 11 bit res, 375 ms
                            //// default is 12 bit resolution, 750 ms conversion time
    }
    celsius = (float)raw / 16.0;
    fahrenheit = celsius * 1.8 + 32.0;

    StaticJsonDocument<500> doc;
    doc["c"] = celsius;
    doc["f"] = fahrenheit;
    char jsonChar[100];
    serializeJsonPretty(doc, jsonChar, 100);
    server.send(200, "application/json", jsonChar);
}
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

void read40()
{
    float v = hx711.read() / 100.0;
    StaticJsonDocument<500> doc;
    doc["value"] = v;
    char jsonChar[100];
    serializeJsonPretty(doc, jsonChar, 100);
    server.send(200, "application/json", jsonChar);
}
void sendA0()
{

    if (rawvalue < 100)
    {
        Serial.println("Error value A0");
        return;
    }
    StaticJsonDocument<500> doc;
    doc["rawvalue"] = rawvalue;
    doc["pressurevalue"] = a0value;
    //pressurevalue
    // JSONencoder["t"] = pfTemp;
    // JSONencoder["h"] = pfHum;
    JsonObject pidevice = doc.createNestedObject("device");
    pidevice["mac"] = WiFi.macAddress();

    char JSONmessageBuffer[300];
    serializeJsonPretty(doc, JSONmessageBuffer, 300);
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
    StaticJsonDocument<500> doc;
    doc["mac"] = WiFi.macAddress();
    // JSONencoder["t"] = pfTemp;
    // JSONencoder["h"] = pfHum;
    JsonObject dht = doc.createNestedObject("ds18value");
    dht["t"] = ktypevalue;

    JsonObject sensor = dht.createNestedObject("ds18sensor");
    sensor["name"] = WiFi.macAddress();
    sensor["callname"] = WiFi.macAddress();

    char JSONmessageBuffer[300];
    serializeJsonPretty(doc, JSONmessageBuffer, 300);
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
    StaticJsonDocument<500> doc;
    doc["mac"] = WiFi.macAddress();
    // JSONencoder["t"] = pfTemp;
    // JSONencoder["h"] = pfHum;
    JsonObject dht = doc.createNestedObject("dhtvalue");

    dht["t"] = pfTemp;
    dht["h"] = pfHum;

    char JSONmessageBuffer[300];
    serializeJsonPretty(doc, JSONmessageBuffer, 300);
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
    StaticJsonDocument<500> doc;
    doc["mac"] = WiFi.macAddress();
    doc["password"] = "";
    doc["ip"] = WiFi.localIP().toString();
    // JSONencoder["t"] = pfTemp;
    // JSONencoder["h"] = pfHum;
    JsonObject dht = doc.createNestedObject("dhtvalue");
    dht["t"] = pfTemp;
    dht["h"] = pfHum;

    char JSONmessageBuffer[300];
    serializeJsonPretty(doc, JSONmessageBuffer, 300);
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

    StaticJsonDocument<500> doc;
    doc["add"] = "ok";
    doc["port"] = s;
    doc["set"] = set;
    char jsonChar[100];
    serializeJsonPretty(doc, jsonChar, 100);
    server.send(200, "application/json", jsonChar);
    Serial.println("Set port " + s + " to " + set);
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
    float psi = analog.readPsi(0.50, 42.5);
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
    StaticJsonDocument<500> doc;
    doc["t"] = pfTemp;
    doc["h"] = pfHum;
    char jsonChar[100];

    serializeJsonPretty(doc, jsonChar, 100);
    server.send(200, "application/json", jsonChar);
}
void PressuretoJSON()
{
    digitalWrite(LED_BUILTIN, LOW);
    readA0();
    digitalWrite(LED_BUILTIN, HIGH);
    StaticJsonDocument<500> doc;
    //root["mac"] = WiFi.macAddress();
    doc["rawvalue"] = rawvalue;
    doc["pressurevalue"] = a0value;
    JsonObject device = doc.createNestedObject("device");
    device["mac"] = WiFi.macAddress();
    char jsonChar[200];
    serializeJsonPretty(doc, jsonChar, 200);
    // root.printTo((char *)jsonChar, root.measureLength() + 1);
    server.send(200, "application/json", jsonChar);
}
void KtypetoJSON()
{
    //  digitalWrite(D3, 1);
    StaticJsonDocument<500> doc;
    //root["mac"] = WiFi.macAddress();
    // root["mac"] = WiFi.macAddress();
    //root["pressurevalue"] = a0value;
    // JsonObject &ds18value = root.createNestedObject("ds18value");
    doc["t"] = readKtype();

    JsonObject pidevice = doc.createNestedObject("pidevice");
    pidevice["mac"] = WiFi.macAddress();

    JsonObject ds18sensor = doc.createNestedObject("ds18sensor");
    ds18sensor["name"] = WiFi.macAddress();
    ds18sensor["callname "] = WiFi.macAddress();

    JsonObject device = doc.createNestedObject("device");
    device["mac"] = WiFi.macAddress();

    char jsonChar[500];
    serializeJsonPretty(doc, jsonChar, 500);
    server.send(200, "application/json", jsonChar);
    // server.close();
    //  digitalWrite(D3, 0);
}
void info()
{
    Serial.print("Mac:");
    Serial.println(WiFi.macAddress());
    Serial.print("IP:");
    Serial.println(WiFi.localIP());
    Serial.print("Version:");
    Serial.println(version);
    StaticJsonDocument<500> doc;
    //root["mac"] = WiFi.macAddr ess();
    // root["mac"] = WiFi.macAddress();
    //root["pressurevalue"] = a0value;
    // JsonObject &ds18value = root.createNestedObject("ds18value");
    doc["mac"] = WiFi.macAddress();
    doc["IP"] = WiFi.localIP().toString();
    doc["version"] = version;
    char jsonChar[500];
    // root.printTo((char *)jsonChar, root.measureLength() + 1);
    serializeJsonPretty(doc, jsonChar, 500);
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
void inden()
{
    int state = digitalRead(b_led);
    digitalWrite(b_led, !state);
    for (int i = 0; i < 6; i++)
    {
        //  Serial.println("Check port " + ports[i].port);
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
void connect()
{
}
void setup()
{
    WiFi.hostname("D1-sensor-1");
    WiFi.mode(WIFI_STA);
    setport();
    pinMode(b_led, OUTPUT); //On Board LED
                            //  pinMode(D4, OUTPUT);
                            // pinMode(LED_BUILTIN, OUTPUT);
                            //pinMode(D6, OUTPUT);
                            //pinMode(D5, OUTPUT);
    //pinMode(D1, OUTPUT);
    //  pinMode(D3, OUTPUT);
    // digitalWrite(D3,0);

    Serial.begin(9600);
    Serial.println();
    Serial.println();

    // connect();
    WiFiMulti.addAP("forpi", "04qwerty");
    // WiFiMulti.addAP("Sirifarm", "0932154741");
    // WiFiMulti.addAP("forgame", "0894297443");
    // WiFiMulti.addAP("pksy", "04qwerty");
    // WiFiMulti.addAP("SP", "04qwerty");
    // WiFiMulti.addAP("SP3", "04qwerty");
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
    server.on("/read40", read40);
    server.on("/ds18b20", readDS);
    server.on("/run", run);
    server.begin(); //เปิด TCP Server
    Serial.println("Server started");

    Serial.println(WiFi.localIP()); // แสดงหมายเลข IP ของ Server
    String mac = WiFi.macAddress();
    Serial.println(mac); // แสดงหมายเลข IP ของ Server
    // Initialize device.
    dht.begin();
    t.every(60000, senddata);
    flipper.attach(1, inden);
}

void loop()
{

    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status

        t.update();
        server.handleClient();
    }
    else
    {
        Serial.println("Error in WiFi connection");
    }
}
