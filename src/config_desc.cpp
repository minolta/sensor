#include "config_desc.h"
#include <string.h>

struct ConfigDescItem
{
    const char *key;
    const char *desc;
};

static const ConfigDescItem CONFIG_DESC[] = {
    {"ssid", "Primary WiFi network name (ssid)"},
    {"password", "Primary WiFi password"},
    {"ssid2", "Second WiFi network (optional)"},
    {"password2", "Second WiFi password"},
    {"ssid3", "Third WiFi network (optional)"},
    {"password3", "Third WiFi password"},
    {"ssid4", "Fourth WiFi network (optional)"},
    {"password4", "Fourth WiFi password"},
    {"ssid5", "Fifth WiFi network (optional)"},
    {"password5", "Fifth WiFi password"},
    {"wifitimeout", "Seconds before WiFi reconnect (default 60)"},
    {"maxconnecttimeout", "Max WiFi connect attempts (default 60)"},
    {"havetorestart", "1 = restart if WiFi fails repeatedly"},
    {"stanalone", "1 = AP-only mode (no station WiFi)"},
    {"stanalonename", "AP SSID prefix in standalone mode"},
    {"stanalonepassword", "AP password in standalone mode"},
    {"apmodetimeout", "Seconds before config AP portal (default 60)"},
    {"description", "Device label in status JSON"},
    {"jsonbuffer", "JSON buffer size in bytes"},
    {"logslots", "In-memory log ring size 4-48 (default 16)"},
    {"checkinurl", "Server URL for check-in POST"},
    {"checkintoken", "Bearer JWT for check-in auth"},
    {"checkintime", "Check-in interval seconds (default 600)"},
    {"checkconnectiontime", "Network health check interval seconds (default 600)"},
    {"checkactivetimeout", "Activity timeout for connection check"},
    {"pinghost", "IP/hostname to ping for link check (empty = gateway). Test: GET /ping"},
    {"talkurl", "Hello/status server base URL"},
    {"otaurl", "Firmware OTA update URL base"},
    {"otatime", "OTA check interval seconds (default 600)"},
    {"updatetimestampurl", "URL to sync device timestamp"},
    {"updatetimestamp", "Timestamp sync interval seconds"},
    {"D3mode", "D3 pin mode: 0=INPUT 1=OUTPUT"},
    {"D3initvalue", "D3 initial output: 0=LOW 1=HIGH"},
    {"D5mode", "D5 pin mode: 0=INPUT 1=OUTPUT"},
    {"D5initvalue", "D5 initial output: 0=LOW 1=HIGH"},
    {"D6mode", "D6 pin mode: 0=INPUT 1=OUTPUT"},
    {"D6initvalue", "D6 initial output: 0=LOW 1=HIGH"},
    {"D7mode", "D7 pin mode: 0=INPUT 1=OUTPUT"},
    {"D7initvalue", "D7 initial output: 0=LOW 1=HIGH"},
    {"D8mode", "D8 pin mode: 0=INPUT 1=OUTPUT"},
    {"D8initvalue", "D8 initial output: 0=LOW 1=HIGH"},
    {"havedht", "1 = enable DHT temp/humidity sensor"},
    {"havesht", "1 = enable SHT temp/humidity sensor"},
    {"haveds", "1 = enable DS18B20 temperature sensor"},
    {"havea0", "1 = enable analog A0 (pressure/soil)"},
    {"havertc", "1 = enable DS3231 RTC module"},
    {"havepmsensor", "1 = enable PM air-quality sensor"},
    {"havepm25", "1 = include PM values in status JSON"},
    {"havesonic", "1 = enable ultrasonic distance sensor"},
    {"haveoled", "1 = enable SSD1306 OLED display"},
    {"havewater", "1 = enable water flow pulse meter"},
    {"havewaterlimit", "1 = enable water usage limit cutout"},
    {"havepzem", "1 = enable PZEM-004T power meter"},
    {"havegps", "1 = enable GPS module"},
    {"havetm", "1 = enable TM1637 7-segment display"},
    {"havesoisensor", "1 = enable soil moisture on A0"},
    {"havefastport", "1 = enable fast-port proximity triggers"},
    {"readdhttime", "DHT read interval seconds"},
    {"readdstime", "DS18B20 read interval seconds"},
    {"readshttime", "SHT read interval seconds"},
    {"reada0time", "A0 analog read interval seconds"},
    {"readdistancetime", "Ultrasonic read interval seconds"},
    {"readpzemtime", "PZEM read interval seconds"},
    {"rtctimeupdate", "RTC sync interval seconds"},
    {"ntpupdatetime", "NTP sync interval seconds"},
    {"va0", "A0 voltage offset for PSI zero"},
    {"sensorvalue", "PSI scale factor (42.5=172psi)"},
    {"senservalue", "PSI scale factor (legacy key name)"},
    {"flowinterrrupport", "GPIO for flow pulse interrupt (e.g. D6)"},
    {"flowlow", "Min flow rate; stop pump if below"},
    {"flowchecktime", "Flow check interval while pump runs"},
    {"flowfaillimit", "Low-flow count before pump stop"},
    {"flowfailtime", "Pump block time after flow fail (sec)"},
    {"waterlimitvalue", "Max water pulses per session"},
    {"wateridletime", "No-flow seconds before session ends"},
    {"wateroverlimit", "Limit hits before long cutout"},
    {"waterlimittime", "Short pump cutout duration (sec)"},
    {"wateroverlimitvalue", "Long pump cutout duration (sec)"},
    {"fastport0", "Fast trigger GPIO name (e.g. D5)"},
    {"fastport1", "Fast trigger GPIO name (e.g. D6)"},
    {"fastport0check", "Fast port 0 re-check interval (sec)"},
    {"fastport1check", "Fast port 1 re-check interval (sec)"},
    {"fastport0time", "Fast port 0 hold time (sec)"},
    {"fastport1time", "Fast port 1 hold time (sec)"},
    {"fastport0statustime", "OLED status time port 0 (sec)"},
    {"fastport1statustime", "OLED status time port 1 (sec)"},
    {"nextreadsoi", "Soil read interval milliseconds"},
    {"soienablepin", "GPIO to power soil sensor (e.g. D5)"},
    {"airvalue", "A0 raw value in dry air (/finddry)"},
    {"wetvalue", "A0 raw value in water (/findwet)"},
};

static const char DESC_DEFAULT[] = "Custom config key";

const char *configDescFor(const char *key)
{
    if (key == nullptr)
        return DESC_DEFAULT;
    const size_t n = sizeof(CONFIG_DESC) / sizeof(CONFIG_DESC[0]);
    for (size_t i = 0; i < n; i++)
    {
        if (strcmp(CONFIG_DESC[i].key, key) == 0)
            return CONFIG_DESC[i].desc;
    }
    return DESC_DEFAULT;
}

String configRowHtml(const String &k, const String &v)
{
    return String("<tr><td>") + k + "</td><td class=\"desc\">" + configDescFor(k.c_str()) +
           "</td><td><label class=\"val\" id=" + k + "value>" + v +
           "</label></td><td><input id=" + k + " type=\"text\" value=\"" + v +
           "\"></td><td><button class=\"btn btn-set\" type=\"button\" onClick=\"setvalue(this,'" + k +
           "','" + v + "')\">Save</button></td><td><button class=\"btn btn-rm\" type=\"button\" onClick=\"remove('" +
           k + "')\">Del</button></td></tr>";
}

size_t configDescWriteJson(char *buf, size_t cap)
{
    if (buf == nullptr || cap < 8)
        return 0;

    char *p = buf;
    char *end = buf + cap;
    int n = snprintf(p, (size_t)(end - p), "{");
    if (n <= 0)
        return 0;
    p += n;

    const size_t cnt = sizeof(CONFIG_DESC) / sizeof(CONFIG_DESC[0]);
    for (size_t i = 0; i < cnt; i++)
    {
        n = snprintf(p, (size_t)(end - p), "%s\"%s\":\"%s\"",
                     i ? "," : "", CONFIG_DESC[i].key, CONFIG_DESC[i].desc);
        if (n <= 0 || p + n >= end)
            break;
        p += n;
    }

    n = snprintf(p, (size_t)(end - p), "}");
    if (n <= 0 || p + n >= end)
        return 0;
    p += n;
    return (size_t)(p - buf);
}
