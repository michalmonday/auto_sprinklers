#include "buzzer.h"
#include <melody_player.h>
#include <melody_factory.h>

#define BUZZER_PIN D8

MelodyPlayer player(BUZZER_PIN);

Buzzer::Buzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); 
}

void Buzzer::play_startup_tune() {
    const char melodyString[] = "PowerOn1:d=8,o=5,b=120:c,e,g,c6";
    Melody melody = MelodyFactory.loadRtttlString(melodyString);
    if (!melody)
        return;
    player.play(melody);
    digitalWrite(BUZZER_PIN, LOW);
}

void Buzzer::play_connected_tune() {
    const char melodyString[] = "HappyTune:d=8,o=5,b=120:c,e,g,c6,g,e,c,2p,g,e,c6,g,e,c";
    Melody melody = MelodyFactory.loadRtttlString(melodyString);
    if (!melody)
        return;
    player.play(melody);
    digitalWrite(BUZZER_PIN, LOW);
}


void Buzzer::play_valve_on_tune() {
    const char melodyString[] = "LittleMarch:d=4,o=5,b=100:c,c,g,g,a,a,g,2p,f,f,e,e,d,d,c";
    Melody melody = MelodyFactory.loadRtttlString(melodyString);
    if (!melody)
        return;
    player.play(melody);
    digitalWrite(BUZZER_PIN, LOW);
}

void Buzzer::play_valve_off_tune() {
    const char melodyString[] = "PowerOff1:d=8,o=5,b=100:g,e,c,2p,c";
    Melody melody = MelodyFactory.loadRtttlString(melodyString);
    if (!melody)
        return;
    player.play(melody);
    digitalWrite(BUZZER_PIN, LOW);
}

void Buzzer::play_water_warning_tune() {
    const char melodyString[] = "Arpeggio:d=8,o=5,b=140:c,e,g,c6,g,e,c,2p,c6,g,e,c,g,e,c";
    Melody melody = MelodyFactory.loadRtttlString(melodyString);
    if (!melody)
        return;
    player.play(melody);
    digitalWrite(BUZZER_PIN, LOW);
}

