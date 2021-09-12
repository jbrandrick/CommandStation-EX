#ifndef DccManager_h
#define DccManager_h

#if !defined(DCC_MANAGER) 
#define DCC_MANAGER DccManager::getInstance()
#endif

#include "HashList.h"
#include "EEStore.h"
#include "Turnout.h"
#include "Sensor.h"
#include "Output.h"

class DccManager {

  private:

    DccManager () {}

    static DccManager* instance;

    int currentSensorKey;

  public:

    DccManager (DccManager& other) = delete;
    void operator= (const DccManager&) = delete;

    static DccManager* getInstance ();


    EEStore* eeStore;

    HashList<Turnout>* turnouts;
    HashList<Sensor>* sensors;
    HashList<Output>* outputs;

    void init ();

    void checkSensor (Stream&);
};

#endif