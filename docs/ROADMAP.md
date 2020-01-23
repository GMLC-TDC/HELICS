
# RoadMap

This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change, but this is a snapshot of the current planning thoughts. See the [projects](https://github.com/GMLC-TDC/HELICS/projects) for additional details


## \[2.5\] ~ 2020-04-30
-   Multi-broker to allow multiple communication cores to be connected in the same federation
-   Single thread cores(possible)
-   Plugin architecture for user defined cores (possible)
-   SSL capable core (probably not)
-   Some dynamic federation support
-   Websocket based core type
-   Internal text based message format option for general backwards compatibility
-   Separate out networking library
-   Split core library between comms layer components and actual core API
-   Debugging tools
-   Additional package manager integration
-   Performance improvements and tests
-   Increased code Coverage

## \[2.6\] ~ Summer 2020
It is unclear if there will be a 2.6 release or the changes would just get rolled into a 3.0 release.  If there is it would be the last of the 2.X series  

## \[3.0\] ~ Summer 2020
-   Upgrade minimum compilers and build systems. Currently planned targets are gcc 7.0, clang 5.0, MSVC 2017 15.7, CMake 3.10.  This is a setup which should be supported on Ubuntu 18.04 repositories.  Minimum Boost version will also be updated though Boost is becoming less critical for the HELICS core so may not be that important. Certain features may require a newer boost version than what would be available on 18.04.  HELICS 3.0 will not be released until Ubuntu 20.04 LTS is out and RHEL 8.0 has been out for a year, and RHEL 8.1 is released, and probably 8.2.  Minimum required compilers for building on macOS and systems using ICC will include XCode 10 and ICC 19.  The minimum ZMQ version will also be bumped up to 4.2.  General policy for Mac builds will be supporting Xcode compilers on versions of MacOS that receive security upgrades which is generally the last 3 versions, though 11 and 10 will likely be the only 2 supported at HELICS 3.0 release.    
-   Control interface
-   Dynamic Federation support
-   Much more general debugging support
-   Targeted endpoints
-   Remove deprecated functions
-   Change values for log level enumerations
-   Some additional renaming of CMake variables
-   xSDK compatibility
-   Renaming of the libraries and reorganization of the header locations
