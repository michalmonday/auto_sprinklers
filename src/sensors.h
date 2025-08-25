#ifndef SENSORS_H
#define SENSORS_H

struct SensorData {
    float temperature;
    float soil_moisture;
    float light_level;
    bool rain_detected;
    float humidity;

    String csv_header() const {
        return "temperature,soil_moisture,light_level,rain_detected,humidity";
    }

    String to_csv() const {
        return String(temperature) + "," + String(soil_moisture) + "," + String(light_level) + "," + String(rain_detected ? 1 : 0) + "," + String(humidity);
    }
};

void init_sensors();
float read_sensor(int mux_addr);
SensorData read_sensors();

void update_sensors_weighted_moving_averages();
SensorData get_sensors_weighted_moving_averages();

#endif // SENSORS_H