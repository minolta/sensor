#include "test_checkin.h"
#include "Configfile.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <unity.h>

extern void connect();
extern Configfile test;

void setUp(void)
{
}

void tearDown(void)
{
}

int checkinPostToUrl(const char *url, const char *token, const char *deviceName, const char *deviceVersion)
{
    if (WiFi.status() != WL_CONNECTED)
        return -1;
    if (url == nullptr || url[0] == '\0')
        return -2;
    if (token == nullptr || token[0] == '\0')
        return -4;

    DynamicJsonDocument doc(768);
    doc["freemem"] = system_get_free_heap_size();
    doc["version"] = deviceVersion;
    doc["name"] = deviceName;
    doc["ip"] = WiFi.localIP().toString();
    doc["mac"] = WiFi.macAddress();
    doc["ssid"] = WiFi.SSID();
    doc["password"] = "";

    char body[768];
    const size_t n = serializeJson(doc, body, sizeof(body));
    if (n >= sizeof(body))
        return -3;

    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);
    http.addHeader(F("Content-Type"), F("application/json"));
    http.addHeader(F("Authorization"), String("Bearer ") + token);
    const int httpCode = http.POST(reinterpret_cast<uint8_t *>(body), n);
    (void)http.getString();
    http.end();
    return httpCode;
}

static String getCheckinTestToken()
{
#ifdef CHECKIN_TEST_TOKEN
    if (strlen(CHECKIN_TEST_TOKEN) > 0)
        return String(CHECKIN_TEST_TOKEN);
#endif
    test.openFile();
    return test.getConfig("checkintoken", "");
}

void test_checkin_payload_fields()
{
    DynamicJsonDocument doc(768);
    doc["freemem"] = 12345;
    doc["version"] = "194";
    doc["name"] = "unit-test";
    doc["ip"] = "192.168.1.10";
    doc["mac"] = "AA:BB:CC:DD:EE:FF";
    doc["ssid"] = "test-ssid";
    doc["password"] = "";

    char body[768];
    const size_t n = serializeJson(doc, body, sizeof(body));
    TEST_ASSERT_LESS_THAN(sizeof(body), n);

    DynamicJsonDocument parsed(768);
    const DeserializationError err = deserializeJson(parsed, body, n);
    TEST_ASSERT_TRUE(err == DeserializationError::Ok);
    TEST_ASSERT_EQUAL(12345, parsed["freemem"].as<int>());
    TEST_ASSERT_EQUAL_STRING("194", parsed["version"]);
    TEST_ASSERT_EQUAL_STRING("unit-test", parsed["name"]);
    TEST_ASSERT_EQUAL_STRING("192.168.1.10", parsed["ip"]);
    TEST_ASSERT_EQUAL_STRING("AA:BB:CC:DD:EE:FF", parsed["mac"]);
    TEST_ASSERT_EQUAL_STRING("test-ssid", parsed["ssid"]);
    TEST_ASSERT_EQUAL_STRING("", parsed["password"]);
}

void test_checkin_post_ok()
{
    connect();
    TEST_ASSERT_EQUAL(WL_CONNECTED, WiFi.status());

    const String token = getCheckinTestToken();
    if (token.length() == 0)
    {
        TEST_IGNORE_MESSAGE("Set checkintoken (Bearer JWT) in /testconfig.cfg or CHECKIN_TEST_TOKEN build flag");
    }

    Serial.printf("checkin test POST %s\n", CHECKIN_TEST_URL);
    const int httpCode = checkinPostToUrl(CHECKIN_TEST_URL, token.c_str(), "sensor-checkin-test", "test");
    Serial.printf("checkin test HTTP %d\n", httpCode);

    if (httpCode == 401)
    {
        TEST_FAIL_MESSAGE("HTTP 401: checkintoken invalid or expired (server requires Bearer JWT)");
    }
    TEST_ASSERT_EQUAL(HTTP_CODE_OK, httpCode);
}
