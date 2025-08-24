#ifndef SENSORS_H
#define SENSORS_H

struct SensorData {
    float temperature;
    float soil_moisture;
    float light_level;
    bool rain_detected;
    float humidity;
};

void init_sensors();
float read_sensor(int mux_addr);
SensorData read_sensors();

#endif // SENSORS_H