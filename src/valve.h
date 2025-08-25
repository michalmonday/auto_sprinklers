#ifndef VALVE_H
#define VALVE_H

class Valve {
public:
    void begin();
    bool is_opened();
    void open();
    void close();
};

#endif