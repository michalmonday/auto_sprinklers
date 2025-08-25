
#include <time.h>
#include "time_manager.h"

#include <ESP8266WiFi.h>

// UK timezone with DST:
//  - Base: GMT (UTC+0)
//  - DST (BST) starts: last Sunday in March at 01:00 UTC
//  - DST ends: last Sunday in October at 02:00 UTC
static const char* TZ_UK = "GMT0BST,M3.5.0/1,M10.5.0/2";

void initTimeUK() {
  // Configure NTP servers (use several for redundancy)
  configTime(0, 0, "pool.ntp.org", "time.google.com", "time.cloudflare.com");

  // Apply UK timezone and DST rules
  setenv("TZ", TZ_UK, 1);
  tzset();

  // Wait until time is set (epoch will jump from 1970 to a sane value)
  time_t now = time(nullptr);
  // uint8_t tries = 0;
  // while (now < 1700000000 && tries < 50) { // ~10 seconds max
  while (now < 1700000000) {
    delay(200);
    yield();
    now = time(nullptr);
    // tries++;
  }

  // Print both UTC and local (UK) time
  Serial.printf("UTC:   %s", ctime(&now));
  struct tm lm;
  localtime_r(&now, &lm);
  char buf[40];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %Z", &lm);
  Serial.printf("Local: %s\n", buf);
}
