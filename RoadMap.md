# RoadMap
This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change

## [1.1] ~ 2018-06-13
### Features
 - Support for TOML configuration
 - Addition of namedPoints to well defined type into the C and C++ interfaces
 - Octave support
 - addition of more convenient broker initialization so brokers can be easily added to executables


### Improvements
 - some initial performance testing and improvements
 - increased testing of queries and timing
 - more detail and testing in the C++ wrapper around the C interface
 - initial performance testing
 - general code refactoring for threading and better modularity

## [2.0 Beta] ~ 2018-08-23
### Features
 - targeted integration with package managers for 2.0 so Beta version will start that process
 - add a control point type for N to 1 control of federates this new interface will be symmetric with the publication and subscriptions interface which is a 1 to N fanout.
 - a routing broker to connect different communication types and brokers together in the same federation

### Improvements
 - Major API changes based on feedback and initial broader usage
 - Change C interface to use an error object instead of return code and corresponding updates to the Interface API's
 - Initial performance and scalability testing
 - relaxed subscription registration requirements
