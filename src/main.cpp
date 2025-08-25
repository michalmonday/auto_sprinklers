// #include <Arduino.h>
// #if defined(ESP8266)
//     #include <ESP8266WiFi.h>
// #elif defined(ESP32)
//     #include <WiFi.h>
// #endif
// #include <ESPAsyncWebServer.h>
// #include "fauxmoESP.h"
// 
// // Rename the credentials.sample.h file to credentials.h and 
// // edit it according to your router configuration
// #include "credentials.h"
// 
// fauxmoESP fauxmo;
// AsyncWebServer server(80);
// 
// // -----------------------------------------------------------------------------
// 
// #define SERIAL_BAUDRATE                 115200
// #define LED                             2
// 
// // -----------------------------------------------------------------------------
// // Wifi
// // -----------------------------------------------------------------------------
// 
// void wifiSetup() {
// 
//     // Set WIFI module to STA mode
//     WiFi.mode(WIFI_STA);
// 
//     // Connect
//     Serial.printf("[WIFI] Connecting to %s ", ssid);
//     WiFi.begin(ssid, pass);
// 
//     // Wait
//     while (WiFi.status() != WL_CONNECTED) {
//         Serial.print(".");
//         delay(100);
//     }
//     Serial.println();
// 
//     // Connected!
//     Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
// 
// }
// 
// void serverSetup() {
// 
//     // Custom entry point (not required by the library, here just as an example)
//     server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
//         request->send(200, "text/plain", "Hello, world");
//     });
// 
//     // These two callbacks are required for gen1 and gen3 compatibility
//     server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
//         if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
//         // Handle any other body request here...
//     });
//     server.onNotFound([](AsyncWebServerRequest *request) {
//         String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
//         if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
//         // Handle not found request here...
//     });
// 
//     // Start the server
//     server.begin();
// 
// }
// 
// void setup() {
// 
//     // Init serial port and clean garbage
//     Serial.begin(SERIAL_BAUDRATE);
//     Serial.println();
//     Serial.println();
// 
//     // LED
//     pinMode(LED, OUTPUT);
//     digitalWrite(LED, HIGH); // Our LED has inverse logic (high for OFF, low for ON)
// 
//     // Wifi
//     wifiSetup();
// 
//     // Web server
//     serverSetup();
// 
//     // Set fauxmoESP to not create an internal TCP server and redirect requests to the server on the defined port
//     // The TCP port must be 80 for gen3 devices (default is 1901)
//     // This has to be done before the call to enable()
//     fauxmo.createServer(false);
//     fauxmo.setPort(80); // This is required for gen3 devices
// 
//     // You have to call enable(true) once you have a WiFi connection
//     // You can enable or disable the library at any moment
//     // Disabling it will prevent the devices from being discovered and switched
//     fauxmo.enable(true);
// 
//     // You can use different ways to invoke alexa to modify the devices state:
//     // "Alexa, turn kitchen on" ("kitchen" is the name of the first device below)
//     // "Alexa, turn on kitchen"
//     // "Alexa, set kitchen to fifty" (50 means 50% of brightness)
// 
//     // Add virtual devices
//     fauxmo.addDevice("kitchen");
// 	fauxmo.addDevice("livingroom");
// 
//     // You can add more devices
// 	//fauxmo.addDevice("light 3");
//     //fauxmo.addDevice("light 4");
//     //fauxmo.addDevice("light 5");
//     //fauxmo.addDevice("light 6");
//     //fauxmo.addDevice("light 7");
//     //fauxmo.addDevice("light 8");
// 
//     fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
//         
//         // Callback when a command from Alexa is received. 
//         // You can use device_id or device_name to choose the element to perform an action onto (relay, LED,...)
//         // State is a boolean (ON/OFF) and value a number from 0 to 255 (if you say "set kitchen light to 50%" you will receive a 128 here).
//         // Just remember not to delay too much here, this is a callback, exit as soon as possible.
//         // If you have to do something more involved here set a flag and process it in your main loop.
//         
//         // if (0 == device_id) digitalWrite(RELAY1_PIN, state);
//         // if (1 == device_id) digitalWrite(RELAY2_PIN, state);
//         // if (2 == device_id) analogWrite(LED1_PIN, value);
//         
//         Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
// 
//         // For the example we are turning the same LED on and off regardless fo the device triggered or the value
//         digitalWrite(LED, !state); // we are nor-ing the state because our LED has inverse logic.
// 
//     });
// 
// }
// 
// void loop() {
// 
//     // fauxmoESP uses an async TCP server but a sync UDP server
//     // Therefore, we have to manually poll for UDP packets
//     fauxmo.handle();
// 
//     // This is a sample code to output free heap every 5 seconds
//     // This is a cheap way to detect memory leaks
//     static unsigned long last = millis();
//     if (millis() - last > 5000) {
//         last = millis();
//         Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
//     }
// 
// }


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

fauxmoESP fauxmo;
AsyncWebServer server(80);   // your server on port 80

unsigned long start_time = 0;
const unsigned long MAX_WATERING_DURATION = 60 * 1000 * 10; // 10 minutes

const unsigned long LOG_INTERVAL = 60 * 1000 * 5; // log every 5 minutes
unsigned long last_log_time = 0;

void serverSetup() {

    // // Custom entry point (not required by the library, here just as an example)
    // server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    //     request->send(200, "text/plain", "Hello, world");
    // });

    // // These two callbacks are required for gen1 and gen3 compatibility
    // server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    //     if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
    //     // Handle any other body request here...
    // });
    // server.onNotFound([](AsyncWebServerRequest *request) {
    //     String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
    //     if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
    //     // Handle not found request here...
    // });

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

    // Start the server
    server.begin();

}

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
  // // buzzer->play_startup_tune();
  valve = new Valve();
  valve->begin();
  valve->close();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // blink
  }

  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  display->show_message({"Connected!", WiFi.localIP().toString()});
  // // buzzer->play_connected_tune();
  delay(500);

  Logger::begin();
  display->show_message({"Pulling time..."});
  Serial.println("Pulling time...");
  initTimeUK();

  serverSetup();
  
  fauxmo.createServer(false);
  fauxmo.setPort(80);
  fauxmo.enable(true);

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
    digitalWrite(LED_BUILTIN, state ? LOW : HIGH);
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
