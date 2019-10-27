
# RoadMap

This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change, but this is a snapshot of the current planning thoughts. See the [projects](https://github.com/GMLC-TDC/HELICS/projects) for additional details


## \[2.3\] ~ 2019-11-10

-   See [HELICS 2.3](https://github.com/GMLC-TDC/HELICS/projects/15) for up to date information
-   All features and dates here are tentative and subject to change

### Features and Improvements
-   Additional unit tests and more porting to google tests
-   modernization and cleanup of the HELICS cmake build system
-   bug fixes
-   construct a C++ shared library with the public API for helics that is more usable and restricted to the public interface
-   additional CI builds
-   library updates
-   Benchmark tests
-   automated generation of some parts of the code


## \[2.4\] ~ Winter 2019-2020
-   Network reliability related improvements
-   Some additional logging capabilities
-   Multi-broker to allow multiple communication cores to be connected in the same federation(possible)
-   Broker-server continued development and support for tcp/udp cores
-   Additional package manager integration
-   Performance improvements and tests
-   messageObject callbacks into the C library
-   Separate out networking library
-   Single thread cores
-   Debugging tools
-   SSL capable core
-   split core library between comms layer components and actual core api
-   plugin architecture for user defined cores

## \[3.0\] ~ Late Spring-Early Summer 2020
-   Upgrade minimum compilers and build systems. Currently planned targets are gcc 7.0, clang 5.0, MSVC 2017 15.7, CMake 3.10.  This is a setup which should be supported on Ubuntu 18.04 repositories.  Minimum Boost version will also be updated though Boost is becoming less critical for the HELICS core so may not be that important. Certain features may require a newer boost version than what would be available on 18.04.  HELICS 3.0 will not be released until Ubuntu 20.04 LTS is out and RHEL 8.0 has been out for a year, and RHEL 8.1 is released.  Minimum required compilers for building on macOS and systems using ICC will include XCode 10 and ICC 19.  The Minimum ZMQ version will also be bumped up to 4.2.  
-   Control interface
-   Dynamic Federation support
-   remove deprecated functions
-   change values for log level enumerations
-   some renaming of CMAKE variables 
-   possible renaming of the libraries
