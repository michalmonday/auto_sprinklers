
#include <Arduino.h>
#include "display.h"

static SH1106 display(0x3c, D2, D1); // ADDRESS, SDA, SCL
Display::Display() {
    // SH1106 *display = new SH1106(0x3c, D2, D1); // ADDRESS, SDA, SCL
}

void Display::init() {
    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);
    display.clear();
    
    // show initial message
    std::vector<String> lines = {"Auto Sprinklers", "Starting..."};
    show_message(lines);

    display.display();
}

void Display::set_data(int valve_state, unsigned long time_until_off_ms, const SensorData &sensor_data) {
    std::vector<String> lines;
    lines.push_back("Valve: " + String(valve_state ? "ON" : "OFF"));
    if (valve_state) {
        lines.push_back(format_ms_to_human_readable(time_until_off_ms) + " left");
    }
    lines.push_back("Temp: " + String(sensor_data.temperature, 1) + "C");
    lines.push_back("Soil: " + String(sensor_data.soil_moisture, 1) + "%");
    lines.push_back("Light: " + String(sensor_data.light_level, 1) + "lx");
    lines.push_back("Rain: " + String(sensor_data.rain_detected ? "Yes" : "No"));
    lines.push_back("Humidity: " + String(sensor_data.humidity, 1) + "%");
    show_message(lines);
}

void Display::show_message(std::vector<String> lines) {
    display.clear();
    int y = 0;
    int x = 0;
    for (const auto &line : lines) {
        display.drawString(x, y, line);
        y += 12; // line height
        if ((y + 12) > display.height()) 
        {
            y = 0;
            x += 64;
        }
    }
    display.display();
}

void Display::clear() {
    display.clear();
    display.display();
}

String Display::format_ms_to_human_readable(unsigned long ms) {
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    seconds = seconds % 60;
    minutes = minutes % 60;

    String out_str;
    if (hours > 0) {
        out_str += String(hours) + "h ";
    }
    if (minutes > 0 || hours > 0) {
        out_str += String(minutes) + "m ";
    }
    out_str += String(seconds) + "s";
    return out_str;
}