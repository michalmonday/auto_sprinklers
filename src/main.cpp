#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"
#include "credentials.h"

// these are defined in credentials.h to avoid sharing my password on github
// const char* ssid = "ssid"; 
// const char* pass = "password"; 
const int VALVE_PIN = D3;

fauxmoESP fauxmo;

unsigned long start_time = 0;
const unsigned long MAX_DURATION = 60 * 1000 * 10; // 10 minutes

void setup() {
  // set serial speed
  Serial.begin(74880);
  delay(200);
  Serial.println("Starting...");


  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // blink
  }

  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
  digitalWrite(LED_BUILTIN, HIGH);

  fauxmo.createServer(true);
  fauxmo.setPort(80);
  fauxmo.enable(true);

  fauxmo.addDevice("sprinklers");

  fauxmo.onSetState([](unsigned char id, const char* name, bool state, unsigned char value) {
    // state = on/off, value = 0..100 (brightness)
    digitalWrite(VALVE_PIN, state ? HIGH : LOW);
    if (state) {
      start_time = millis();
    }
    digitalWrite(LED_BUILTIN, state ? LOW : HIGH);

    // Optional: treat "value" as minutes
    // if (state) start a timer for `value` minutes then turn off.
  });
}

void loop() {
  fauxmo.handle();
  // handle your optional timers here
  if (digitalRead(VALVE_PIN) == HIGH && millis() - start_time > MAX_DURATION) {
    digitalWrite(VALVE_PIN, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
