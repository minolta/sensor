#include "memlog.h"
#include <stdlib.h>
#include <string.h>

#define MEMLOG_MSG 28
#define MEMLOG_SLOTS_MIN 4
#define MEMLOG_SLOTS_MAX 48
#define MEMLOG_SLOTS_DEFAULT 16
#define MEMLOG_TASK_MIN_MS 500

struct MemLogEntry
{
    uint32_t uptime;
    uint8_t type;
    int16_t code;
    char msg[MEMLOG_MSG];
};

static MemLogEntry *_log = nullptr;
static uint8_t _maxSlots = MEMLOG_SLOTS_DEFAULT;
static uint8_t _idx = 0;
static uint8_t _count = 0;
static char _jsonBuf[MEMLOG_JSON_BUF_MAX];
static uint32_t _lastTaskLogMs = 0;

static void memlogEnsure()
{
    if (_log == nullptr)
        memlogSetSlots(MEMLOG_SLOTS_DEFAULT);
}

static void copyMsg(char *dst, const char *src)
{
    if (src == nullptr)
        src = "";
    strncpy(dst, src, MEMLOG_MSG - 1);
    dst[MEMLOG_MSG - 1] = '\0';
}

void memlogSetSlots(uint8_t n)
{
    if (n < MEMLOG_SLOTS_MIN)
        n = MEMLOG_SLOTS_MIN;
    if (n > MEMLOG_SLOTS_MAX)
        n = MEMLOG_SLOTS_MAX;
    if (_log != nullptr && _maxSlots == n)
        return;
    if (_log != nullptr)
        free(_log);
    _maxSlots = n;
    _log = (MemLogEntry *)malloc(_maxSlots * sizeof(MemLogEntry));
    _idx = 0;
    _count = 0;
}

uint8_t memlogMaxSlots()
{
    memlogEnsure();
    return _maxSlots;
}

size_t memlogJsonCapacity()
{
    memlogEnsure();
    const size_t need = 144 + (size_t)_maxSlots * 96;
    return need < MEMLOG_JSON_BUF_MAX ? need : MEMLOG_JSON_BUF_MAX;
}

char *memlogJsonBuffer()
{
    return _jsonBuf;
}

size_t memlogJsonBufferSize()
{
    return sizeof(_jsonBuf);
}

void memlogAdd(MemLogType type, int code, const char *msg)
{
    memlogEnsure();
    if (_log == nullptr)
        return;
    MemLogEntry &e = _log[_idx];
    e.uptime = millis() / 1000;
    e.type = type;
    e.code = (int16_t)code;
    copyMsg(e.msg, msg);
    _idx = (_idx + 1) % _maxSlots;
    if (_count < _maxSlots)
        _count++;
}

void memlogAdd(MemLogType type, int code, const String &msg)
{
    memlogAdd(type, code, msg.c_str());
}

void memlogTask(const char *name, int code)
{
    const uint32_t now = millis();
    if (code == 0 && (now - _lastTaskLogMs) < MEMLOG_TASK_MIN_MS)
        return;
    _lastTaskLogMs = now;
    memlogAdd(MEMLOG_TASK, code, name);
}

void memlogClear()
{
    _idx = 0;
    _count = 0;
}

uint8_t memlogCount()
{
    return _count;
}

static const char *memlogTypeName(uint8_t type)
{
    switch (type)
    {
    case MEMLOG_CHECKIN:
        return "checkin";
    case MEMLOG_ERROR:
        return "error";
    case MEMLOG_OTA:
        return "ota";
    case MEMLOG_SOI:
        return "soi";
    case MEMLOG_SHT:
        return "sht";
    case MEMLOG_TASK:
        return "task";
    default:
        return "?";
    }
}

static char *appendJsonString(char *p, char *end, const char *s)
{
    if (s == nullptr)
        s = "";
    while (*s && p < end - 1)
    {
        if (*s == '"' || *s == '\\')
        {
            if (p + 2 >= end)
                break;
            *p++ = '\\';
        }
        *p++ = *s++;
    }
    *p = '\0';
    return p;
}

size_t memlogWriteJson(char *buf, size_t cap, const char *fwVersion)
{
    memlogEnsure();
    if (_log == nullptr || buf == nullptr || cap < 48)
        return 0;

    if (fwVersion == nullptr)
        fwVersion = "";

    char *p = buf;
    char *end = buf + cap;
    int n = snprintf(p, (size_t)(end - p),
                     "{\"version\":\"%s\",\"count\":%u,\"max\":%u,\"heap\":%u,\"logs\":[",
                     fwVersion, _count, _maxSlots, ESP.getFreeHeap());
    if (n <= 0 || (size_t)n >= cap)
        return 0;
    p += n;

    const uint8_t oldest = (_count < _maxSlots) ? 0 : _idx;
    for (uint8_t i = 0; i < _count; i++)
    {
        const MemLogEntry &e = _log[(oldest + i) % _maxSlots];
        n = snprintf(p, (size_t)(end - p), "%s{\"t\":%u,\"type\":\"%s\",\"code\":%d,\"msg\":\"",
                     i ? "," : "", e.uptime, memlogTypeName(e.type), e.code);
        if (n <= 0 || p + n >= end)
            break;
        p += n;
        p = appendJsonString(p, end, e.msg);
        if (p + 2 >= end)
            break;
        *p++ = '"';
        *p++ = '}';
        *p = '\0';
    }

    n = snprintf(p, (size_t)(end - p), "]}");
    if (n <= 0 || p + n >= end)
        return 0;
    p += n;
    return (size_t)(p - buf);
}
