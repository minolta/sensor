#ifndef TEST_CHECKIN_H
#define TEST_CHECKIN_H

#include <Arduino.h>

static const char CHECKIN_TEST_URL[] = "http://192.168.88.5:888/rest/piserver/checkin";

// POST check-in JSON to url; returns HTTP status code (or negative on client error).
int checkinPostToUrl(const char *url, const char *token, const char *deviceName, const char *deviceVersion);

void test_checkin_post_ok();
void test_checkin_payload_fields();

#endif
