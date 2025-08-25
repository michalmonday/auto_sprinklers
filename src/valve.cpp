
#include "valve.h"

#include <Arduino.h>

#define VALVE_PIN D3

void Valve::begin() {
    pinMode(VALVE_PIN, OUTPUT);
    digitalWrite(VALVE_PIN, LOW); // start closed
}

bool Valve::is_opened() {
    return digitalRead(VALVE_PIN) == HIGH;
}

void Valve::open() {
    digitalWrite(VALVE_PIN, HIGH);
}

void Valve::close() {
    digitalWrite(VALVE_PIN, LOW);
}