#include <Arduino.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
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
const char *ssid = "Sirifarm";       //กำหนด SSID (อย่าลืมแก้เป็นของตัวเอง)
const char *password = "0932154741"; //กำหนด Password(อย่าลืมแก้เป็นของตัวเอง)

const char *ssid1 = "pksy";         //กำหนด SSID (อย่าลืมแก้เป็นของตัวเอง)
const char *password1 = "04qwerty"; //กำหนด Password(อย่าลืมแก้เป็นของตัวเอง)
//const char *ssid = "Sirifarm";       //กำหนด SSID (อย่าลืมแก้เป็นของตัวเอง)
//const char *password = "0932154741"; //กำหนด Password(อย่าลืมแก้เป็นของตัวเอง)
const char *host = "endpoint.pixka.me:5002";
int count = 0;
//WiFiServer server(80); //กำหนดใช้งาน TCP Server ที่ Port 80
ESP8266WebServer server(80);
#define ONE_WIRE_BUS D4
uint8_t deviceCount = 0;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float tempC;
Timer t;
#define DHTPIN D2 // Pin which is connected to the DHT sensor.

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

    http.end(); //Close connection
}
void sendDht()
{
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

    http.begin("http://endpoint.pixka.me:5002/dht/add"); //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header

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

    http.begin("http://endpoint.pixka.me:5002/checkin"); //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header

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
void httpService()
{
    /*
    WiFiClient client = server.available();
    if (client)
    {
        String req = client.readStringUntil('\r');
        Serial.println("Req:"+req);
      
        bool success = readRequest(client);
        if (success)
        {

            String req = client.readStringUntil('\r');
            Serial.println("Req:"+req);
            delay(1000);

            digitalWrite(LED_BUILTIN, LOW);
            readDHT();
            digitalWrite(LED_BUILTIN, HIGH);

            StaticJsonBuffer<500> jsonBuffer;
            JsonObject &json = prepareResponse(jsonBuffer);
            writeResponse(client, json);
        }
       
        delay(1);
        client.stop();
    }
    */
}
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

void readKtype()
{
    float DC = ktc.readCelsius();
    ktypevalue = DC;
    Serial.print("K type value ");
    Serial.println(DC);
}
void readA0()
{
    int sensorValue = analogRead(A0);
    Serial.print("ADC 10 bit = ");
    Serial.print(sensorValue); // print out the value you read:

    float volts = 3.30 * (float)sensorValue / 1023.00;
    float psi = (volts - 1.45) * 10;
    Serial.print(" , Voltage = ");
    Serial.print(volts, 2);
    Serial.println(" V");
    a0value = psi;
    rawvalue = sensorValue;
}
void DHTtoJSON()
{
    digitalWrite(LED_BUILTIN, LOW);
    readDHT();
    digitalWrite(LED_BUILTIN, HIGH);

    StaticJsonBuffer<300> jsonBuffer;
    JsonObject &json = prepareResponse(jsonBuffer);
    char jsonChar[100];
    json.printTo((char *)jsonChar, json.measureLength() + 1);
    server.send(200, "application/json", jsonChar);
    /*
    String message;
    for (int i = 0; i < server.args(); i++)
    {

        message += "Arg n" + (String)i + " –> "; //Include the current iteration value
        message += server.argName(i) + ": ";      //Get the name of the parameter
        message += server.arg(i) + "\n";          //Get the value of the parameter
    }
    Serial.println("Arg: "+message);
    */
    //writeResponse(client, json);
}
void senddata()
{
    digitalWrite(LED_BUILTIN, LOW);
    checkin();
    readDHT();
    sendDht();
    sendA0();
    sendKtype();
    digitalWrite(LED_BUILTIN, HIGH);
    // readKtype();
}

void connect()
{
    int waitforconnect = 0;
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);           //เชื่อมต่อกับ AP
    while (WiFi.status() != WL_CONNECTED) //รอการเชื่อมต่อ
    {
        delay(500);
        Serial.print(".");
        waitforconnect++;

        if (waitforconnect >= 20)
        {
            break;
        }
    }
    waitforconnect = 0;
    Serial.print("Connecting to ");
    Serial.println(ssid1);
    WiFi.begin(ssid1, password1);         //เชื่อมต่อกับ AP
    while (WiFi.status() != WL_CONNECTED) //รอการเชื่อมต่อ
    {
        delay(500);
        Serial.print(".");
        waitforconnect++;

        if (waitforconnect >= 20)
        {
            break;
        }
    }

    Serial.println("");
    Serial.println("WiFi connected"); //แสดงข้อความเชื่อมต่อสำเร็จ
}
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D1, OUTPUT);
    pinMode(D3, OUTPUT);
    Serial.begin(9600);
    Serial.println();
    Serial.println();

    // connect();
    WiFiMulti.addAP("Sirifarm", "0932154741");
    WiFiMulti.addAP("pksy", "04qwerty");

    while (WiFiMulti.run() != WL_CONNECTED) //รอการเชื่อมต่อ
    {
        delay(500);
        Serial.print(".");
    }

    server.on("/dht", DHTtoJSON);
    server.on("/command", runCommand);
    server.begin(); //เปิด TCP Server
    Serial.println("Server started");

    Serial.println(WiFi.localIP()); // แสดงหมายเลข IP ของ Server
    String mac = WiFi.macAddress();
    Serial.println(mac); // แสดงหมายเลข IP ของ Server
    // Initialize device.
    dht.begin();
    t.every(60000, senddata);
}

void loop()
{

    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status

        t.update();
        server.handleClient();
        // httpService();
    }
    else
    {

        Serial.println("Error in WiFi connection");
    }

    readA0();
    readKtype();
    delay(1000);

    //delay(30000); //Send a request every 30 seconds
    countsend++;
}
