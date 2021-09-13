
# Overview of the refactoring

 The objectives of the refactoring were three fold.

* To continue scratching the itch. I have recently retired from a career developing software. The last 13 involved the analysis of large scale legacy systems, including writing tools to aid in the effort.
* To pull apart the tight coupling between the functional parts of DCC++EX. To remove the need for them to know of others intimate internals and expose their own. And make them individually testable.
* To showcase the capabilities of modern C++ with the hope that other developers would have reason to upgrade their skills.

The initial work involved removing the static variables and methods from Sensor struct. To do this required implementing a linked list that was separate from the functional components implementing this independently, namely Sensor, Output & Turnout.

HashList is a templated class that provides linked list and hash lookup capability for any class type.
* Contains methods for get, remove, add and getOrAdd items of the templated type.
* The HashList's Node class wraps the supplied item and itself is at no time exposed, only the item is returned.
* Includes a WalkList method that calls a supplied function for each item in the list. And a WalkListWithKey method that provides the hash key as well. See the HashList test for an example.
* Contains a free item list for holding removed items for reuse. This to minimize heap fragmentation.
* Includes a count of the number of items in the list and a boolean to indicate that the list has changed since last change reset.

The Sensors,cpp/h, Outputs.cpp/h & Turnouts.cpp/h were removed since they implemented both the individual sensor, etc and the overall sensors list. New classes for Sensor, Output & Turnout added that only know about the sensor, etc that each instance represents. There are no statics in these classes. Other than the analog part of Sensor, the functioning of the methods in these classes were not tested so take this more as a structural example.

The EEStore class was refactored to reference the new Sensor, Output & Turnout classes. The load and store methods were removed from those classes and made part of EEStore. The load methods use the HashList getOrAdd and Sensor, etc populate methods to build the lists. The store method uses the HashList walkList method to navigate each of those. Output and Turnout contain a holding variable for the data offset into their location in the EEPROM.

Similar to EEStore, DCCEXParser and WiThrottle were changed to access the Sensor, Output & Turnout classes. The parseT, parseS, parseZ and parseD methods in DCCEXParser reflect these changes. Search for DCC_MANAGER to find them.

Since the Sensor, Output & Turnout instances and the EEStore instance need a home, there is the DccManager class. It's implemented as a singleton and accessed via the DCC_MANAGER macro. The implementation type for DccManager can be changed by changing this macro. This would not affect the rest of the code base. The checkSensor and issueLocoReminders methods exist are implemented here but likely would move into other refactored areas in the code.

# Tests

There is a separate GitHub repository called jbrandrick/CommandStation-EX-PIO. This includes the entire PlatformIO folder. In there is a Test folder containing unit tests for many of the methods listed above. These run on the Arduino so have access to the hardware. If testing of specific component arrangements is added.