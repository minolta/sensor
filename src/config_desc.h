#ifndef CONFIG_DESC_H
#define CONFIG_DESC_H

#include <Arduino.h>

const char *configDescFor(const char *key);
String configRowHtml(const String &k, const String &v);
size_t configDescWriteJson(char *buf, size_t cap);

#endif
