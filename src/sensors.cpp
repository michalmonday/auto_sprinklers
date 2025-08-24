
#include <Arduino.h>
#include "sensors.h"


#include <DHT.h>
#include <DHT_U.h>

// there is only 1 analog pin on the Wemos D1 mini
// so a 16 to 1 multiplexer is used
#define ANALOG_PIN A0

#define MUX_ADDR0 D7
#define MUX_ADDR1 D6
#define MUX_ADDR2 D5
#define MUX_ADDR3 D0

#define RAIN_MUX_ADDR 0
#define SOIL_MOISTURE_MUX_ADDR 1
#define LIGHT_SENSOR_MUX_ADDR 2

DHT dht(D4, DHT11);

void set_mux_address(int addr) {
    digitalWrite(MUX_ADDR0, (addr & 0x01) ? HIGH : LOW);
    digitalWrite(MUX_ADDR1, (addr & 0x02) ? HIGH : LOW);
    digitalWrite(MUX_ADDR2, (addr & 0x04) ? HIGH : LOW);
    digitalWrite(MUX_ADDR3, (addr & 0x08) ? HIGH : LOW);
}

void init_sensors() {
    pinMode(MUX_ADDR0, OUTPUT);
    pinMode(MUX_ADDR1, OUTPUT);
    pinMode(MUX_ADDR2, OUTPUT);
    pinMode(MUX_ADDR3, OUTPUT);
    pinMode(ANALOG_PIN, INPUT);
    dht.begin();
}

float read_sensor(int mux_addr) {
    set_mux_address(mux_addr);
    delay(10); // wait for the mux to settle
    int value = analogRead(ANALOG_PIN);
    return (value / 1023.0) * 100.0; // return percentage
}


SensorData read_sensors() {
    SensorData data;

    data.rain_detected = read_sensor(RAIN_MUX_ADDR) > 50.0; // threshold at 50%
    data.soil_moisture = read_sensor(SOIL_MOISTURE_MUX_ADDR);
    data.light_level = read_sensor(LIGHT_SENSOR_MUX_ADDR);
    data.temperature = dht.readTemperature();
    data.humidity = dht.readHumidity();
    if (isnan(data.temperature)) {
        data.temperature = -1.0; // error value
    }
    if (isnan(data.humidity)) {
        data.humidity = -1.0; // error value
    }

    return data;
}
