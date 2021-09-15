#include "Sensor.h"


void Sensor::populate (SensorData dataIn) {
  populate (dataIn.snum, dataIn.pin, dataIn.pullUp);
}


void Sensor::populate (int snum, int pin, int pullUp) {
  data.snum       = snum;
  data.pin        = pin;
  data.pullUp     = (pullUp == 0 ? LOW : HIGH);

  pinMode (pin, INPUT);         // set mode to input
  digitalWrite (pin, pullUp);   // don't use Arduino's internal pull-up resistors for
                                // external infrared sensors
                                // each sensor must have its own 1K external pull-up resistor
}


bool Sensor::read () {
  return !digitalRead (data.pin );
}


void Sensor::check (Print *stream) {
  bool currentState = read ();

  if (currentState == active) {

    if (latchdelay != 0) {        // state changed back so reset count of data points
      // enable if you want to debug contact jitter
      //if (stream != NULL) StringFormatter::send(stream, F("JITTER %d %d\n"), 
      //                                          latchdelay, data.id);
       latchdelay = 0;
    }

  } else if (latchdelay < 127) {  // state still changed so collect data points to smooth jitter
    latchdelay++;

  } else {                          // state changed and data in is good
    active      = currentState;
    latchdelay  = 0;

    if (stream != NULL) {
      send (stream);
    }
  }
}
