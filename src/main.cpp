#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "fauxmoESP.h"
#include "credentials.h"
#include "display.h"
#include "sensors.h"
#include "buzzer.h"
#include "valve.h"
#include "log.h"
#include "time_manager.h"

// these are defined in credentials.h to avoid sharing my password on github
// const char* ssid = "ssid"; 
// const char* pass = "password"; 

static Display *display;
static Buzzer *buzzer;
static Valve *valve;

AsyncWebServer server(80);   // your server on port 80
fauxmoESP fauxmo;

unsigned long start_time = 0;
const unsigned long MAX_WATERING_DURATION = 60 * 1000 * 10; // 10 minutes

const unsigned long LOG_INTERVAL = 60 * 1000 * 5; // log every 5 minutes
unsigned long last_log_time = 0;

void setup() {
  // set serial speed
  buzzer = new Buzzer();
  Serial.begin(74880);
  delay(200);
  Serial.println("Starting...");
  init_sensors();
  display = new Display();
  display->init();
  delay(1000);
  display->show_message({"Connecting to", ssid});
  // buzzer->play_startup_tune();
  valve = new Valve();
  valve->begin();
  valve->close();

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
  display->show_message({"Connected!", WiFi.localIP().toString()});
  // buzzer->play_connected_tune();
  delay(500);

  Logger::begin();
  display->show_message({"Pulling time..."});
  Serial.println("Pulling time...");
  initTimeUK();

    // Your own endpoints
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* req){
    bool on = valve->is_opened();
    req->send(200, "application/json", String("{\"sprinklers\":") + (on ? "true" : "false") + "}");
  });

  server.on("/api/list-files", HTTP_GET, [](AsyncWebServerRequest* req){
    std::vector<String> file_list = Logger::list_files();
    String csv_response = "";
    for (int i=0; i < file_list.size(); i++) {
      csv_response += file_list[i] + "\n";
    }
    req->send(200, "text/plain", csv_response);  
  });

  server.on("/api/file", HTTP_GET, [](AsyncWebServerRequest* req){
    if (!req->hasParam("name")) {
      req->send(400, "text/plain", "Missing 'name' parameter");
      return;
    }
    String filename = req->getParam("name")->value();
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    if (!filename.startsWith("/logs/")) {
      req->send(400, "text/plain", "Invalid file path");
      return;
    }
    // replace ".." 
    filename.replace("..", "");
    if (!LittleFS.exists(filename)) {
      req->send(404, "text/plain", "File not found");
      return;
    }
    req->send(LittleFS, filename, "text/plain");
  });

  server.on("/api/remove-file", HTTP_GET, [](AsyncWebServerRequest* req){
    if (!req->hasParam("name")) {
      req->send(400, "text/plain", "Missing 'name' parameter");
      return;
    }
    String filename = req->getParam("name")->value();
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    if (!filename.startsWith("/logs/")) {
      req->send(400, "text/plain", "Invalid file path");
      return;
    }
    // replace ".." 
    filename.replace("..", "");
    if (!LittleFS.exists(filename)) {
      req->send(404, "text/plain", "File not found");
      return;
    }
    LittleFS.remove(filename);
    req->send(200, "text/plain", "File removed");
  });

  // These two callbacks are required for gen1 and gen3 compatibility
  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
      // Handle any other body request here...
  });
  server.onNotFound([](AsyncWebServerRequest *request) {
      String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
      if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
      // Handle not found request here...
  });
  
  fauxmo.createServer(false);
  fauxmo.setPort(80);

  server.begin();

  fauxmo.addDevice("sprinklers");

  fauxmo.onSetState([](unsigned char id, const char* name, bool state, unsigned char value) {
    // state = on/off, value = 0..100 (brightness)
    if (state) {
      valve->open();
      start_time = millis();
      // buzzer->play_valve_on_tune();
    } else {
      valve->close();
      // buzzer->play_valve_off_tune();
    }
  fauxmo.enable(true);

    digitalWrite(LED_BUILTIN, state ? LOW : HIGH);

    

    // Optional: treat "value" as minutes
    // if (state) start a timer for `value` minutes then turn off.
  });
}

void loop() {
  fauxmo.handle();
  // handle your optional timers here
  if (valve->is_opened() && millis() - start_time > MAX_WATERING_DURATION) {
    valve->close();
    digitalWrite(LED_BUILTIN, HIGH);
    // buzzer->play_valve_off_tune();
  }

  // read sensors and update display
  // SensorData sensor_data = read_sensors();
  update_sensors_weighted_moving_averages();
  SensorData sensor_data = get_sensors_weighted_moving_averages();

  unsigned long time_until_off_ms = 0;
  if (valve->is_opened()) {
    time_until_off_ms = MAX_WATERING_DURATION - (millis() - start_time);
  }
  display->set_data(valve->is_opened(), time_until_off_ms, sensor_data);

  if (millis() - last_log_time > LOG_INTERVAL) {
    last_log_time = millis();
    Logger::log(valve->is_opened(), sensor_data);
  }

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
