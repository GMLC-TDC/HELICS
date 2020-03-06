
# RoadMap

This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change, but this is a snapshot of the current planning thoughts. See the [projects](https://github.com/GMLC-TDC/HELICS/projects) for additional details


## \[2.5\] ~ 2020-04-15
-   Multi-broker to allow multiple communication cores to be connected in the same federation
-   Some dynamic federation support
-   C API callbacks for custom filters
-   Improved REST API support
-   Clarification and API support for multiple publications to a single input N-1 value based
-   Websocket based core type
-   Separate out additional networking library components
-   Split core library between comms layer components and actual core API
-   Debugging tools (Global time synchronization points)
-   Additional package manager integration
-   Performance improvements and tests
-   Increased code Coverage (target 80%)
-   Deprecate C api message structure

## \[2.6\] ~ 2020-07-15
This will be the last of the 2.X series releases, there will likely be at least one patch release after this before fully moving to 3.0
-   Internal text based (probably JSON) message format option for general backwards compatibility
-   Debugging tools
-   Additional package manager integration
-   Performance improvements and tests

## \[3.0\] ~ 2020-08-26
-   Upgrade minimum compilers and build systems. Currently planned targets are gcc 7.0, clang 5.0, MSVC 2017 15.7, CMake 3.10.  This is a setup which should be supported on Ubuntu 18.04 repositories.  Minimum Boost version will also be updated though Boost is becoming less critical for the HELICS core so may not be that important.  The likely minimum tested target will likely be 1.65.1 though the core make work with older versions and can be disabled completely. Certain features may require a newer boost version(1.70) than what would be available on Ubuntu 18.04.  HELICS 3.0 will not be released until Ubuntu 20.04 LTS is out and RHEL 8.0 has been out for a year, and RHEL 8.1 is released, and probably 8.2.  Minimum required compilers for building on macOS and systems using ICC will include Xcode 10 and ICC 19.  The minimum ZMQ version will also be bumped up to 4.2.  General policy for Mac builds will be supporting Xcode compilers on versions of MacOS that receive security upgrades which is generally the last 3 versions, though 11 and 10 will likely be the only 2 supported at HELICS 3.0 release.   MSVC compilers will be supported for at least 2 years from release date, an appropriate CMake (meaning slightly newer than the compiler) will also be required for Visual studio builds.    
-   Control interface
-   Full Dynamic Federation support
-   Much more general debugging support
-   Targeted endpoints
-   General API changes based on feedback
-   SSL capable core
-   Single thread cores
-   Plugin architecture for user defined cores
-   Remove deprecated functions
-   Change values for log level enumerations
-   Some additional renaming of CMake variables
-   xSDK compatibility
-   Renaming of the libraries and reorganization of the header locations
