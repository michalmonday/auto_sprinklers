#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <vector>
#include "sensors.h"

#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
// #include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
// #include "SH1106Wire.h"   // legacy: #include "SH1106.h"
#include "SH1106.h"


class Display {
  public:
    Display(); 
    void init();
    void set_data(int valve_state, unsigned long time_until_off_ms, const SensorData &sensor_data);
    void show_message(std::vector<String> lines);
    void clear();

    private:
    // // wemos D1 mini pins
    // SH1106 *display; //(0x3c, D2, D1);     // ADDRESS, SDA, SCL
    String format_ms_to_human_readable(unsigned long ms);
};

#endif // DISPLAY_H