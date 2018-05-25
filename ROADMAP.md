# RoadMap
This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change.  see the [projects](https://github.com/GMLC-TDC/HELICS-src/projects) for additional details

## [1.2] ~ 2018-06-13
 - see [HELICS 1.2](https://github.com/GMLC-TDC/HELICS-src/projects/6) for up to date information

## [1.3] ~ 2018-07-16  Improvements planned before the 2.0 release
### Features
 - Support for TOML configuration
 - Octave support
 - add a control point type for N to 1 control of federates this new interface will be symmetric with the publication and subscriptions interface which is a 1 to N fanout.

### Improvements
 - some initial performance testing and improvements
 - increased testing of queries and timing
 - more detail and testing in the C++ wrapper around the C interface
 - general code refactoring for threading and better modularity

## [2.0 Beta] ~ 2018-08-23
### Features
 - targeted integration with package managers for 2.0 so Beta version will start that process
 - a routing broker to connect different communication types and brokers together in the same federation

### Improvements
 - Major API changes based on feedback and initial broader usage
 - Change C interface to use an error object instead of return code and corresponding updates to the Interface API's
 - Initial performance and scalability testing
 - relaxed subscription/endpoint registration requirements
