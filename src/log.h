
#ifndef LOG_H
#define LOG_H

#include <Arduino.h>
#include <vector>
#include "sensors.h"


namespace Logger {
    void begin();
    bool log(int valve_open, const SensorData &sensor_data);
    std::vector<String> list_files();
    void print_files();
    void remove_old_files();
}

#endif
