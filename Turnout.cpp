#include "Turnout.h"
#include "DCC.h"
#include "LCN.h"
#include "PWMServoDriver.h"

void Turnout::activate (bool state) { // activate is virtual here so that it can be overridden by a non-DCC turnout mechanism

  if (data.address == LCN_TURNOUT_ADDRESS) {  // A LCN turnout is transmitted to the LCN master.
     LCN::send ('T', data.id, state);
     return;                                  // The tStatus will be updated by a message from the LCN master, later.    
  }

  if (state)
    data.tStatus |= STATUS_ACTIVE;
  else
    data.tStatus &= ~STATUS_ACTIVE;

  if (data.tStatus & STATUS_PWM)
    PWMServoDriver::setServo (data.tStatus & STATUS_PWMPIN, (data.inactiveAngle + (state ? data.moveAngle : 0)));
  else
    DCC::setAccessory (data.address, data.subAddress, state);
  
  persistStatus ();
}

void Turnout::populate (TurnoutData dataIn) {
  if (dataIn.tStatus & STATUS_PWM)
    populate (dataIn.id, dataIn.tStatus & STATUS_PWMPIN, dataIn.inactiveAngle, dataIn.moveAngle);
  else
    populate (dataIn.id, dataIn.address, dataIn.subAddress);
}

void Turnout::populate (int id, int add, int subAdd) {
  data.id         = id;
  data.address    = add;
  data.subAddress = subAdd;
  data.tStatus    = 0;
}

// are activeAngle & inactiveAngle in the right order?
void Turnout::populate (int id, byte pin, int activeAngle, int inactiveAngle) {
  data.id             = id;
  data.tStatus        = STATUS_PWM | (pin & STATUS_PWMPIN);
  data.inactiveAngle  = inactiveAngle;
  data.moveAngle      = activeAngle - inactiveAngle;
}