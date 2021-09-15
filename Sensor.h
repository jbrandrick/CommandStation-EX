#ifndef Sensor_h
#define Sensor_h

#include "Arduino.h"
#include "StringFormatter.h"

struct SensorData {
  int         snum;
  uint8_t     pin;
  uint8_t     pullUp;
};

class Sensor {
  public:
    bool active;
    byte latchdelay;

    SensorData data;

    Sensor () {
      active      = false;
      latchdelay  = 0;
    }

    void populate (SensorData);
    void populate (int, int, int);
    void check (Print *);
    bool read ();

    void send (Print *stream) {
      StringFormatter::send (stream, F("<%c %d>\n"), active ? 'Q' : 'q', data.snum);
    }
    void sendDef (Print *stream) {
      StringFormatter::send (stream, F("<Q %d %d %d>\n"), data.snum, data.pin, data.pullUp);
    }
};

#endif