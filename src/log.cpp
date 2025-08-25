
#include "log.h"
#include <Arduino.h>
// #include <FS.h>
#include <LittleFS.h>

// logging data to esp8266 filesystem

#define MAX_LOG_FILES 30

void Logger::remove_old_files() {
    std::vector<String> files = list_files();
    if (files.size() <= MAX_LOG_FILES) {
        return;
    }
    // sort files by name (which is also by date due to naming scheme)
    std::sort(files.begin(), files.end());
    size_t files_to_remove = files.size() - MAX_LOG_FILES;
    for (size_t i = 0; i < files_to_remove; i++) {
        String filename = files[i];
        Serial.println("Removing old log file: " + filename);
        // SPIFFS.remove(filename);
        LittleFS.remove(filename);
    }
}

void Logger::begin() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }
    if (!LittleFS.exists("/logs")) {
        LittleFS.mkdir("/logs");
    }
    Serial.println("File system mounted");
}

bool Logger::log(int valve_open, const SensorData &sensor_data) {
    // store in a csv file named YYYY-MM-DD.csv in "logs" directory

    time_t now = time(nullptr);
    struct tm *lt = localtime(&now);
    int year = lt->tm_year + 1900;
    int month = lt->tm_mon + 1;
    int day = lt->tm_mday;
    int hour = lt->tm_hour;
    int minute = lt->tm_min;
    int second = lt->tm_sec;

    String filename = String("/logs/") + String(year) + "-" + (month < 10 ? "0" : "") + String(month) + "-" + (day < 10 ? "0" : "") + String(day) + ".csv";
    File file = LittleFS.open(filename, "a");
    if (!file) {
        Serial.println("Failed to open log file");
        return false;
    }
    if (file.size() == 0) {
        // write header
        file.println("year,month,day,hour,minute,second,valve_open," + sensor_data.csv_header());
    }
    String log_entry = String(year) + "," + String(month) + "," + String(day) + "," + String(hour) + "," + String(minute) + "," + String(second) + "," + String(valve_open) + "," + sensor_data.to_csv();
    file.println(log_entry);
    file.close();

    Logger::remove_old_files();
    return true;
}

std::vector<String> Logger::list_files() {
    std::vector<String> files;
    Dir dir = LittleFS.openDir("/logs");
    while (dir.next()) {
        files.push_back(dir.fileName());
    }
    return files;
}

void Logger::print_files() {
    std::vector<String> files = Logger::list_files();
    Serial.println("Log files:");
    for (const String &file : files) {
        Serial.println(file);
    }
}


