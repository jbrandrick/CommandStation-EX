#ifndef Turnout_h
#define Turnout_h

#include <Arduino.h>
#include <EEPROM.h>
#include "RingStream.h"
#include "StringFormatter.h"

const byte STATUS_ACTIVE        = 0x80; // Flag as activated
const byte STATUS_PWM           = 0x40; // Flag as a PWM turnout
const byte STATUS_PWMPIN        = 0x3F; // PWM  pin 0-63
const int  LCN_TURNOUT_ADDRESS  = -1;   // spoof dcc address -1 indicates a LCN turnout

struct TurnoutData {
  int id;
  uint8_t tStatus; // has STATUS_ACTIVE, STATUS_PWM, STATUS_PWMPIN  
  union {
    uint8_t subAddress;
    char moveAngle;
  }; //DCC  sub address or PWM difference from inactiveAngle  
  union {
    int address;
    int inactiveAngle;
  }; // DCC address or PWM servo angle 
};

class Turnout {
  private:
    int eeStoreTurnoutDataOffset;  // EEPROM address of tStatus in TurnoutData struct, or zero if not stored.

  public:

    TurnoutData data;

    Turnout () {
      eeStoreTurnoutDataOffset = 0;
    }
    
    void populate (TurnoutData);
    void populate (int id , int address , int subAddress);
    void populate (int id , byte pin , int activeAngle, int inactiveAngle);
    void populate (int id);
    void activate (bool state);

    bool isActive () {
      return data.tStatus & STATUS_ACTIVE;
    }
    void persistStatus () {
      // if (EEStore::eeStore->data.nTurnouts > 0  &&  num > 0) // ??? why do we need data.nTurnouts > 0?
      if (eeStoreTurnoutDataOffset > 0)
        EEPROM.put (eeStoreTurnoutDataOffset, data.tStatus);
    }
    void setTurnoutDataOffset (int pointer) {
      eeStoreTurnoutDataOffset = pointer + offsetof(TurnoutData, tStatus);
    }
    void send (Print* stream) {
      StringFormatter::send (stream, F("<H %d %d>\n"), data.id, (data.tStatus & STATUS_ACTIVE) != 0);
    }
    void sendDef (Print* stream) {
      StringFormatter::send(stream, F("<H %d %d %d %d>\n"), data.id, data.address, data.subAddress, isActive ());
    }
    void sendWifi (RingStream* stream) {
      StringFormatter::send (stream,F("]\\[%d}|{%d}|{%c"), data.id, data.id, isActive () ? '4' : '2');
    }
};
  
#endif
