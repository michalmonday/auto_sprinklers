#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"
#include "credentials.h"
#include "display.h"
#include "sensors.h"

// these are defined in credentials.h to avoid sharing my password on github
// const char* ssid = "ssid"; 
// const char* pass = "password"; 
const int VALVE_PIN = D3;

static Display *display;

fauxmoESP fauxmo;

unsigned long start_time = 0;
const unsigned long MAX_DURATION = 60 * 1000 * 10; // 10 minutes

void setup() {
  // set serial speed
  Serial.begin(74880);
  delay(200);
  Serial.println("Starting...");
  init_sensors();
  display = new Display();
  display->init();
  delay(1000);
  display->show_message({"Connecting to", ssid});

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(VALVE_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // blink
    display->show_message({"Connecting to", ssid, String("...")});
  }

  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

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

  // read sensors and update display
  SensorData sensor_data = read_sensors();
  unsigned long time_until_off_ms = 0;
  if (digitalRead(VALVE_PIN) == HIGH) {
    time_until_off_ms = MAX_DURATION - (millis() - start_time);
  }
  display->set_data(digitalRead(VALVE_PIN), time_until_off_ms, sensor_data);

  // std::vector<float> sensor_values;
  // for (int i=0; i < 16; i++) {
  //   float sensor_value = read_sensor(i);
  //   sensor_values.push_back(sensor_value);
  // }
  // Serial.println("Sensor values:");
  // for (int i=0; i < sensor_values.size(); i++) {
  //   Serial.printf("%.0f  ", sensor_values[i]);
  //   String format_ms_to_human_readable(unsigned long ms, String &out_str);
  // }
  // Serial.println();

}
