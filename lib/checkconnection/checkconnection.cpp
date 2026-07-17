#include "checkconnection.h"

int talktoServer(String ip, String name, long uptime, Configfile *p)
{
  String pt = p->getConfig("talkurl", "http://192.168.88.21:3334/hello");

  String talkurl = pt + "/" + WiFi.localIP().toString() + "/" + uptime + "/" + name;
  WiFiClient client;
  HTTPClient http;

  Serial.print(" Http Code:");
  http.begin(client, talkurl);
  int httpResponseCode = http.GET();
  Serial.println(httpResponseCode);
  Serial.println(http.getString());
  http.end();
  return httpResponseCode;
}

static bool tcpReachable(const IPAddress &ip, uint16_t port, uint32_t timeoutMs)
{
  if (ip == IPAddress(0, 0, 0, 0))
    return false;
  WiFiClient client;
  client.setTimeout(timeoutMs / 1000 + 1);
  const unsigned long deadline = millis() + timeoutMs;
  while (millis() < deadline)
  {
    if (client.connect(ip, port))
    {
      client.stop();
      return true;
    }
    delay(100);
    yield();
  }
  return false;
}

static bool resolveTarget(const String &targetIP, IPAddress &ip)
{
  if (ip.fromString(targetIP))
    return true;
  return WiFi.hostByName(targetIP.c_str(), ip) == 1;
}

bool pingTarget(const String &target, int timeoutMs)
{
  if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0))
    return false;

  IPAddress ip;
  if (!resolveTarget(target, ip))
    return false;

  if (tcpReachable(ip, 80, timeoutMs))
    return true;

  IPAddress gw = WiFi.gatewayIP();
  if (gw != ip)
    return tcpReachable(gw, 80, timeoutMs);
  return false;
}

bool checkAndReconnectToIP(String targetIP, int timeout)
{
  if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0))
  {
    Serial.println(F("WiFi down, reconnecting"));
    return reconnectWifiNow();
  }

  IPAddress ip;
  if (!resolveTarget(targetIP, ip))
  {
    Serial.println(F("resolve target failed, reconnect WiFi"));
    if (!reconnectWifiNow())
      return false;
    if (!resolveTarget(targetIP, ip))
      return WiFi.status() == WL_CONNECTED;
  }

  if (tcpReachable(ip, 80, timeout))
    return true;

  IPAddress gw = WiFi.gatewayIP();
  if (gw != ip && tcpReachable(gw, 80, timeout))
    return true;

  Serial.println(F("network unreachable, reconnect WiFi"));
  if (!reconnectWifiNow())
    return false;

  if (resolveTarget(targetIP, ip) && tcpReachable(ip, 80, timeout))
    return true;

  gw = WiFi.gatewayIP();
  return tcpReachable(gw, 80, timeout);
}
