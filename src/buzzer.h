#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include <melody_player.h>
#include <melody_factory.h>

class Buzzer {
public:
    Buzzer();
    void play_startup_tune();
    void play_connected_tune();
    void play_valve_on_tune();
    void play_valve_off_tune();
    void play_water_warning_tune();
private:
};

#endif