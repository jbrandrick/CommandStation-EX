#include "Output.h"

void Output::activate (int s) {
  data.oStatus = (s > 0);

  // set state of output pin to HIGH or LOW
  // depending on whether bit zero of iFlag is set to 0 (ACTIVE=HIGH) or 1 (ACTIVE=LOW)
  digitalWrite (data.pin, activeValue ());
  
  persistStatus ();
}


void Output::populate (OutputData dataIn) {
  populate (dataIn.id, dataIn.pin, dataIn.iFlag, dataIn.oStatus);
}


void Output::populate (uint16_t id, uint8_t pin, uint8_t iFlag, uint8_t oStatus) {
  data.id       = id;
  data.pin      = pin;
  data.iFlag    = iFlag;

  // sets status to 0 (INACTIVE) is bit 1 of iFlag=0, otherwise set to value of bit 2 of iFlag
  if (bitRead (data.iFlag, 1))
    data.oStatus = bitRead (data.iFlag, 2);
  else
    data.oStatus = oStatus;

  digitalWrite (data.pin, activeValue ());
  pinMode (data.pin, OUTPUT);
}

/***********************************************************************

DCC++ BASE STATION supports optional OUTPUT control of any unused Arduino Pins for custom purposes.
Pins can be activited or de-activated.  The default is to set ACTIVE pins HIGH and INACTIVE pins LOW.
However, this default behavior can be inverted for any pin in which case ACTIVE=LOW and INACTIVE=HIGH.

Definitions and state (ACTIVE/INACTIVE) for pins are retained in EEPROM and restored on power-up.
The default is to set each defined pin to active or inactive according to its restored state.
However, the default behavior can be modified so that any pin can be forced to be either active or inactive
upon power-up regardless of its previous state before power-down.

To have this sketch utilize one or more Arduino pins as custom outputs, first define/edit/delete
output definitions using the following variation of the "Z" command:

  <Z ID PIN IFLAG>:            creates a new output ID, with specified PIN and IFLAG values.
                               if output ID already exists, it is updated with specificed PIN and IFLAG.
                               note: output state will be immediately set to ACTIVE/INACTIVE and pin will be set to HIGH/LOW
                               according to IFLAG value specifcied (see below).
                               returns: <O> if successful and <X> if unsuccessful (e.g. out of memory)

  <Z ID>:                      deletes definition of output ID
                               returns: <O> if successful and <X> if unsuccessful (e.g. ID does not exist)

  <Z>:                         lists all defined output pins
                               returns: <Y ID PIN IFLAG STATE> for each defined output pin or <X> if no output pins defined

where

  ID: the numeric ID (0-32767) of the output
  PIN: the arduino pin number to use for the output
  STATE: the state of the output (0=INACTIVE / 1=ACTIVE)
  IFLAG: defines the operational behavior of the output based on bits 0, 1, and 2 as follows:

          IFLAG, bit 0:   0 = forward operation (ACTIVE=HIGH / INACTIVE=LOW)
                          1 = inverted operation (ACTIVE=LOW / INACTIVE=HIGH)

          IFLAG, bit 1:   0 = state of pin restored on power-up to either ACTIVE or INACTIVE depending
                              on state before power-down; state of pin set to INACTIVE when first created
                          1 = state of pin set on power-up, or when first created, to either ACTIVE of INACTIVE
                              depending on IFLAG, bit 2

          IFLAG, bit 2:   0 = state of pin set to INACTIVE uponm power-up or when first created
                          1 = state of pin set to ACTIVE uponm power-up or when first created

Once all outputs have been properly defined, use the <E> command to store their definitions to EEPROM.
If you later make edits/additions/deletions to the output definitions, you must invoke the <E> command if you want those
new definitions updated in the EEPROM.  You can also clear everything stored in the EEPROM by invoking the <e> command.

To change the state of outputs that have been defined use:

  <Z ID STATE>:                sets output ID to either ACTIVE or INACTIVE state
                               returns: <Y ID STATE>, or <X> if turnout ID does not exist

where

  ID: the numeric ID (0-32767) of the turnout to control
  STATE: the state of the output (0=INACTIVE / 1=ACTIVE)

When controlled as such, the Arduino updates and stores the direction of each output in EEPROM so
that it is retained even without power.  A list of the current states of each output in the form <Y ID STATE> is generated
by this sketch whenever the <s> status command is invoked.  This provides an efficient way of initializing
the state of any outputs being monitored or controlled by a separate interface or GUI program.

**********************************************************************/
