# RoadMap

This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library. All dates are approximate and subject to change, but this is a snapshot of the current planning thoughts. See the [projects](https://github.com/GMLC-TDC/HELICS/projects) for additional details

## \[2.8\] ~ 2021-06-15

This will be the last of the 2.X series releases, there will likely be at least one patch release after this before fully moving to 3.0

- Internal text based (probably JSON) message format option for general backwards compatibility
- Function deprecations to prepare people to move to 3.0

## \[3.0\] ~ 2021-05-05 Beta, Final release approximately a month later

Upgrade minimum compilers and build systems. Currently planned minimum targets are gcc 7.0, clang 5.0, MSVC 2017 15.7, XCode 10.1, and CMake 3.10. This is a setup which should be supported on Ubuntu 18.04 repositories. Minimum Boost version will also be updated though Boost is becoming less critical for the HELICS core so may not be that important. The likely minimum tested target will likely be 1.65.1 though the core might work with older versions and its use can be disabled completely. Certain features may require a newer boost version(1.70) than what would be available on Ubuntu 18.04. General target requirements will allow HELICS to build on the most recent 2 LTS versions of Ubuntu using readily available repo packages. Minimum required compilers for building on macOS and systems using ICC will include Xcode 10 and ICC 19. The minimum ZMQ version will also be bumped up to 4.2. General policy for Mac builds will be supporting Xcode compilers on versions of MacOS that receive security upgrades which is generally the last 3 versions, though 10.1+ and 11 will likely be the only 2 supported at HELICS 3.0 release, and support minor releases for at least 2 years. MSVC compilers will be supported for at least 2 years from release date, an appropriate CMake (meaning slightly newer than the compiler) will also be required for Visual Studio builds.

- Control interface
- Targeted endpoints
- General API changes based on feedback and code review
- Remove deprecated functions
- Change values for log level enumerations
- Some additional renaming of CMake variables
- Renaming of some of the libraries and reorganization of the header locations

## \[3.1\] ~ 2021-07-15

Mostly things that didn't quite make it into the 3.0 release and a number of bug fixes that come from transitioning to HELICS 3.0.

- SSL capable core (unlikely in 3.1 but someday)
- Full Dynamic Federation support
- Single thread cores (partial at release)
- Plugin architecture for user defined cores
- xSDK compatibility
- Much more general debugging support
