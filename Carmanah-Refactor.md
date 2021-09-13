
# Overview of the refactoring

 To begin with this is an example of what may be possible. I don't have experience running DCC++EX other than on a short bit of track with a sensor at each end. Certainly actually using this code would necesitate changes but I'm hopeful they would be simpler to implement and test, after refactoring. That said, the objectives of the refactoring were three fold.

* To continue scratching the itch. I have recently retired from a career developing software. The last 13 involved the analysis of large scale legacy systems, including writing tools to aid in the effort.
* To pull apart the tight coupling between the functional parts of DCC++EX. To remove the need for them to know of others intimate internals and to expose their own. And make them individually testable.
* To showcase the capabilities of modern C++ with the hope that other developers would have reason to upgrade their skills. This may seem arrogant but in speaking with many legacy system developers they often are interested in new developments and are looking for a way forward.

The initial work involved removing the static variables and functions from the Sensor struct. A significant part of this required implementing a linked list that was separate from the functional components implementing this independently, namely Sensor, Output & Turnout. Keeping with the theme of being wary of library functions, this became the HashList class.

HashList is a templated class that provides linked list and hash lookup capability for any class type.
* Contains functions for get, remove, add and getOrAdd items of the templated type.
* Includes a WalkList function that calls a supplied function for each item in the list. And a WalkListWithKey function that provides the hash key as well. See the HashList test for an example.
* Contains a free item list for holding removed items for reuse. This to minimize heap fragmentation.
* Includes a count of the number of items in the list and a boolean to indicate that the list has changed since the last change reset.
* The HashList's Node class wraps the supplied item and itself is at no time exposed, only the item is returned.

The Sensors.cpp/h, Outputs.cpp/h & Turnouts.cpp/h were removed since they implemented both the individual sensor, etc and the overall sensors list. New classes for Sensor, Output & Turnout added that only know about the sensor, etc that each instance represents. There are no statics in these classes. Other than the analog part of Sensor, the functions in these classes were not tested while running DCC++EX. Just in the unit tests. So take this more as a structural example.

The EEStore class was refactored to reference the new Sensor, Output & Turnout classes. The load and store functions were removed from those classes and made part of EEStore. The load functions use the HashList getOrAdd and the Sensor, etc populate functions are called in building the lists. The store function uses the HashList walkList function to navigate each of these. Output and Turnout contain a holding variable for the data offset into their location in the EEPROM.

Similar to EEStore, DCCEXParser and WiThrottle were changed to access the Sensor, Output & Turnout classes. The parseT, parseS, parseZ and parseD functions in DCCEXParser reflect these changes. In the parse function in WiThrottle are calles to check if the turnout list has changed as well as to walk the list. Search for DCC_MANAGER in the code.

Since the Sensor, Output & Turnout instances and the EEStore instance need a home, there is the DccManager class. It's implemented as a singleton and accessed via the DCC_MANAGER macro. If desired the implementation type for DccManager can be changed by changing this macro. This would not affect the rest of the code base. The checkSensor and issueLocoReminders functions are implemented here but likely would move into other refactored areas in the code.

# Tests

There is a separate GitHub repository called jbrandrick/CommandStation-EX-PIO. This includes the entire PlatformIO folder. In there is a Test folder containing unit tests for many of the functions listed above. These run on the Arduino so have access to the hardware for testing of specific component arrangements.

In true Test Driven Development the tests are written first, then the code. That may not be popular but it's available when the full PlatformIO folder is included in GitHub. The intention going forward is to write a number of approval tests for the areas to be refactored. That wasn't done here but it's clear given the complexity of the code in DCC++EX that these will be very useful.

# Links to helpful resources

* Lambdas in C++: http://www.vishalchovatiya.com/learn-lambda-function-in-cpp-with-example/
* PlatformIO unit testing: https://docs.platformio.org/en/latest//plus/unit-testing.html
* Refactoring legacy code and many other useful videos: https://www.youtube.com/watch?v=p-oWHEfXEVs