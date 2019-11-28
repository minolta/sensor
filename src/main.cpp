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
#include <OneWire.h>
#include "Timer.h"
#include <max6675.h>
#include <Ticker.h>
#include "KAnalog.h"
#include <Q2HX711.h>
#include <EEPROM.h>
#include "Adafruit_Sensor.h"
#include "Adafruit_AM2320.h"

#define ADDR 100
#define someofio 5
const String version = "24";
long uptime = 0;
long checkintime = 0;
long readdhttime = 0;
long readdstime = 0;
String message = "";
long reada0time = 0;
float tmpvalue = 0;

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
} configdata;

Dhtbuffer dhtbuffer;
long otatime = 0;
int readdhtstate = 0;
int apmode = 0;
String otahost = "fw1.pixka.me";
String type = "D1IO";
String urlupdate = "/espupdate/nodemcu/" + version;
// OneWire  ds(D4);  // on pin D4 (a 4.7K resistor is necessary)
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
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP WIFI </title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    SSID: <input type="text" name="ssid">
    PASSWORD: <input type="password" name="password">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";
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
OneWire ds(D3); // on pin D4 (a 4.7K resistor is necessary)

#define DHTPIN D4 // Pin which is connected to the DHT sensor.

uint8_t deviceCount = 0;
float tempC;
Timer t;
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

int ktcSO = D6;
int ktcCS = D7;
int ktcCLK = D5;

MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
ESP8266WiFiMulti WiFiMulti;
float ktypevalue = 0;
Ticker flipper;
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
    EEPROM.put(ADDR, wifidata);
    EEPROM.commit();
    server.send(200, "application/json", "SSID" + ssd + " P:" + password);
}
void setwifi()
{
    server.send(200, "text/html", index_html);
}
void setReadtmplimit()
{
    String v = server.arg("value");
    if (v != NULL)
        configdata.readtmpvalue = v.toInt();
    EEPROM.put(ADDR + 100, configdata);
    EEPROM.commit();
    server.send(200, "text/html", "OK:" + v);
}
void setReada0limit()
{
    String v = server.arg("value");
    if (v != NULL)
        configdata.a0readtime = v.toInt();
    EEPROM.put(ADDR + 100, configdata);
    EEPROM.commit();
    server.send(200, "text/html", "OK:" + v);
}
void setport()
{
}
void readDHT()
{
    #ifdef DHT
    readdhtstate = 1;
    pinMode(D4, INPUT);
    // dht.begin();
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
    }

    pinMode(D4, OUTPUT);
    readdhtstate = 0;
    #endif
}
void status()
{
    StaticJsonDocument<1000> doc;
    doc["name"] = name;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["ssid"] = WiFi.SSID();
    doc["version"] = version;
    // readDHT();
    doc["h"] = pfHum;
    doc["t"] = pfTemp;
    doc["uptime"] = uptime;
    doc["dhtbuffer.time"] = dhtbuffer.count;
    doc["type"] = type;
    doc["message"] = message;
    doc["a0"] = a0value;
    doc["tmp"] = tmpvalue;
    //  int readtmpvalue = 120;
    // int a0readtime = 120;
    doc["reada0time"] = configdata.a0readtime;
    doc["readtmptime"] = configdata.readtmpvalue;
    char jsonChar[1000];
    serializeJsonPretty(doc, jsonChar, 1000);
    server.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
    server.sendHeader("Access-Control-Allow-Headers", "application/json");
    // 'Access-Control-Allow-Headers':'application/json'
    server.send(200, "application/json", jsonChar);
}
void addTorun(int port, int delay, int value, int wait)
{
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
void updateCheckin()
{
    String s = server.arg("newurl");
    s.toCharArray(checkinhost, s.length());
}
int foundds = 0;
byte addr[8];
byte type_s;
int searchDs()
{
    if (!ds.search(addr))
    {
        Serial.println("No more addresses.");
        Serial.println();
        ds.reset_search();
        delay(250);
        return 0;
    }
    foundds = 1; //เจอแล้ว
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
        return 0;
    }
    return 1;
}
//อ่านค่าของ DS18b20
DS18b20 DS()
{
    DS18b20 value;
    value.c = -1;
    value.f = -1;
    value.t = -1;
    byte i;
    byte present = 0;

    byte data[12];

    float celsius, fahrenheit;
    if (!foundds)
        searchDs();

    if (OneWire::crc8(addr, 7) != addr[7])
    {
        Serial.println("CRC is not valid!");
        return value;
    }
    Serial.println();

    ds.reset();
    ds.select(addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end

    delay(750); // maybe 750ms is enough, maybe not
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

    value.c = celsius;
    value.f = fahrenheit;
    value.t = celsius;
    return value;
}
void readDS()
{
    DS18b20 v = DS();
    StaticJsonDocument<500> doc;
    doc["c"] = v.c;
    doc["f"] = v.f;
    doc["t"] = v.t;
    char jsonChar[500];
    serializeJsonPretty(doc, jsonChar, 500);
    server.send(200, "application/json", jsonChar);
}

void read40()
{
    // float v = hx711.read() / 100.0;
    // StaticJsonDocument<500> doc;
    // doc["value"] = v;
    // char jsonChar[100];
    // serializeJsonPretty(doc, jsonChar, 100);
    // server.send(200, "application/json", jsonChar);
}
void reset()
{
    StaticJsonDocument<1000> doc;
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
    char jsonChar[1000];
    serializeJsonPretty(doc, jsonChar, 1000);
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
    Serial.println("OTA");

    Serial.println(urlupdate);
    t_httpUpdate_return ret = ESPhttpUpdate.update(otahost, 8080, urlupdate);
    // t_httpUpdate_return ret = ESPhttpUpdate.update("192.168.88.15", 8889,"/espupdate/nodemcu/"+version, version);
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

void checkin()
{
    StaticJsonDocument<500> doc;
    doc["mac"] = WiFi.macAddress();
    doc["password"] = "";
    doc["ip"] = WiFi.localIP().toString();
    doc["uptime"] = uptime;
    JsonObject dht = doc.createNestedObject("dhtvalue");
    dht["t"] = pfTemp;
    dht["h"] = pfHum;
    doc["type"] = type;

    char JSONmessageBuffer[300];
    serializeJsonPretty(doc, JSONmessageBuffer, 300);
    Serial.println(JSONmessageBuffer);
    // put your main code here, to run repeatedly:
    HTTPClient http; //Declare object of class HTTPClient

    http.begin(checkinhost);                            //Specify request destination
    http.addHeader("Content-Type", "application/json"); //Specify content-type header
    // http.addHeader("Authorization", "Basic VVNFUl9DTElFTlRfQVBQOnBhc3N3b3Jk");

    int httpCode = http.POST(JSONmessageBuffer); //Send the request
    String payload = http.getString();           //Get the response payload
    Serial.print(" Http Code:");
    Serial.println(httpCode); //Print HTTP return code
    if (httpCode == 200)
    {
        Serial.print(" Play load:");
        Serial.println(payload); //Print request response payload
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();
        name = obj["pidevice"]["name"].as<String>();
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
void reada0()
{
    Serial.println("READ A0");
    a0value = analog.readA0();
    Serial.print("A0 value:");
    Serial.println(a0value);
}
void a0()
{
    StaticJsonDocument<500> doc;
    doc["a0"] = a0value;
    JsonObject device = doc.createNestedObject("device");
    device["mac"] = WiFi.macAddress();
    char jsonChar[500];
    serializeJsonPretty(doc, jsonChar, 500);
    server.send(200, "application/json", jsonChar);
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
    float psi = analog.readPsi(0.5, 42.5);
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
    doc["rawvalue"] = analog.getRawvalue();
    doc["pressurevalue"] = a0value;
    doc["psi"] = a0value;
    doc["bar"] = a0value / 14.504;
    doc["volts"] = analog.getReadVolts();
    JsonObject device = doc.createNestedObject("device");
    device["mac"] = WiFi.macAddress();
    char jsonChar[200];
    serializeJsonPretty(doc, jsonChar, 200);
    // root.printTo((char *)jsonChar, root.measureLength() + 1);
    server.send(200, "application/json", jsonChar);
}
void readTmp()
{
    Serial.println("Read TMP");
    tmpvalue = readKtype();
    if (isnan(tmpvalue) || tmpvalue < 1)
    {
        DS18b20 ds = DS();
        tmpvalue = ds.c;
    }
    Serial.print("READ");
    Serial.println(tmpvalue);
    Serial.println("End Read TMP");
}
void KtypetoJSON()
{

    //  digitalWrite(D3, 1);
    StaticJsonDocument<500> doc;
    //root["mac"] = WiFi.macAddress();
    // root["mac"] = WiFi.macAddress();
    //root["pressurevalue"] = a0value;
    // JsonObject &ds18value = root.createNestedObject("ds18value");
    doc["t"] = tmpvalue;

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
void inden()
{
    uptime++;
    checkintime++;
    otatime++;
    readdhttime++; //บอกเวลา สำหรับอ่าน DHT
    readdstime++;
    dhtbuffer.count--;
    reada0time++;

    if (!readdhtstate)
        digitalWrite(b_led, !digitalRead(b_led));
}

void setAPMode()
{
    String mac = WiFi.macAddress();
    WiFi.softAP("ESP" + mac, "12345678");
    IPAddress IP = WiFi.softAPIP();
    Serial.println(IP.toString());
    apmode = 1;
}
void printIPAddressOfHost(const char *host)
{
    IPAddress resolvedIP;
    if (!WiFi.hostByName(host, resolvedIP))
    {
        Serial.println("DNS lookup failed.  Rebooting...");
        Serial.flush();
        ESP.reset();
    }
    Serial.print(host);
    Serial.print(" IP: ");
    Serial.println(resolvedIP);
}

void connect()
{
}

void setup()
{
    EEPROM.begin(1000); // Use 1k for save value
    //WiFi.hostname("D1-sensor-1");
    Serial.begin(9600);
    Serial.println();
    Serial.println();
    int flag = EEPROM.read(0);
    // EEPROM.get(ADDR + 200, saveconfig);
    // String v = saveconfig.save;
    if (flag != 22)
    {
        EEPROM.write(0, 22);
        EEPROM.put(ADDR, wifidata);
        EEPROM.put(ADDR + 100, configdata);

        EEPROM.commit();
    }
    else
    {
        EEPROM.get(ADDR, wifidata);
        EEPROM.get(ADDR + 100, configdata);
    }
    Serial.println();
    Serial.println("-----------------------------------------------");
    Serial.println(wifidata.ssid);
    Serial.println(wifidata.password);
    Serial.println("-----------------------------------------------");
    WiFiMulti.addAP(wifidata.ssid, wifidata.password);

    // WiFi.mode(WIFI_STA);
    pinMode(b_led, OUTPUT); //On Board LED
                            //  pinMode(D4, OUTPUT);
                            // pinMode(LED_BUILTIN, OUTPUT);
                            //pinMode(D6, OUTPUT);
                            //pinMode(D5, OUTPUT);
    //pinMode(D1, OUTPUT);
    //  pinMode(D3, OUTPUT);
    // digitalWrite(D3,0);

    // connect();
    WiFiMulti.addAP("forpi", "04qwerty");
    WiFiMulti.addAP("forpi2", "04qwerty");
    // WiFiMulti.addAP("forpi3", "04qwerty");
    // WiFiMulti.addAP("Sirifarm", "0932154741");
    WiFiMulti.addAP("test", "12345678");

    // WiFiMulti.addAP("forgame", "0894297443");
    WiFiMulti.addAP("pksy", "04qwerty");
    // WiFiMulti.addAP("SP", "04qwerty");
    // WiFiMulti.addAP("SP3", "04qwerty");

    int co = 0;
    while (WiFiMulti.run() != WL_CONNECTED) //รอการเชื่อมต่อ
    {

        delay(500);
        Serial.print(".");
        co++;
        if (co > 50)
        {
            setAPMode();
            break;
        }
    }
    if (!apmode)
    {
        checkin();
        ota();
        Serial.println(WiFi.localIP()); // แสดงหมายเลข IP ของ Server
        String mac = WiFi.macAddress();
        Serial.println(mac); // แสดงหมายเลข IP ของ Server
    }
    server.on("/dht", DHTtoJSON);
    server.on("/pressure", PressuretoJSON);
    // server.on("/command", runCommand);
    server.on("/ktype", KtypetoJSON);
    server.on("/info", info);
    server.on("/read40", read40);
    server.on("/ds18b20", readDS);
    server.on("/run", run);
    server.on("/a0", a0);
    server.on("/setwifi", setwifi);
    server.on("/setreadtmp", setReadtmplimit);
    server.on("/setreada0", setReada0limit);
    server.on("/get", get);
    server.on("/updatecheckin", updateCheckin);
    server.on("/readam", readam);

    server.on("/status", status);
    server.on("/reset", reset);

    server.on("/", status);

    server.begin(); //เปิด TCP Server
    Serial.println("Server started");

    Serial.println(WiFi.localIP()); // แสดงหมายเลข IP ของ Server
    String mac = WiFi.macAddress();
    Serial.println(mac); // แสดงหมายเลข IP ของ Server
    // Initialize device.
    Serial.print("DNS");
    Serial.println(WiFi.dnsIP().toString());
    printIPAddressOfHost("fw1.pixka.me");

    pinMode(D4, OUTPUT);
    // t.every(60000, senddata);
    flipper.attach(1, inden);
    am2320.begin();
    readAm2320();
}

void loop()
{

    // t.update();
    server.handleClient();
    //  delay(100);
    if (checkintime > 600)
    {
        checkintime = 0;
        checkin();
    }
    if (otatime > 60)
    {
        otatime = 0;
        ota();
    }
    if (readdhttime > 5 && dhtbuffer.count < 1)
    {
        readdhttime = 0;
        message = "Read DHT";
        // readDHT();
        readAm2320();
    }
    if (readdstime > configdata.readtmpvalue)
    {
        readdstime = 0;
        readTmp();
    }
    if (reada0time > configdata.a0readtime)
    {
        reada0time = 0;
        reada0();
    }
}
