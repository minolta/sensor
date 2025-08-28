#include <Arduino.h>
#include "Configfile.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#ifndef CHECKCONNECTION_H
#define CHECKCONNECTION_H
int talktoServer(String ip, String name, long uptime,Configfile *p);
bool checkAndReconnectToIP(String targetIP = "192.168.88.1", int timeout = 5000);
#endif