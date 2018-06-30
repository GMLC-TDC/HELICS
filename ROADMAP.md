# RoadMap
This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change.  see the [projects](https://github.com/GMLC-TDC/HELICS-src/projects) for additional details


## [1.3] ~ 2018-07-29  Improvements planned before the 2.0 release
 - see [HELICS 1.2](https://github.com/GMLC-TDC/HELICS-src/projects/6) for up to date information
### Features
 - Support for TOML configuration
 - Octave support
 - add a control point type for N to 1 control of federates this new interface will be symmetric with the publication and subscriptions interface which is a 1 to N fanout.
 - rework of testing executables to reduce build time and simplify testing

### Improvements
 - some initial performance testing and improvements
 - exploration of more general convergence algorithms
 - increased testing of queries and timing and error conditions
 - more detail and testing in the C++ wrapper around the C interface
 - general code refactoring for threading and better modularity

## [2.0 Beta] ~ 2018-08-23
### Features
 - targeted integration with package managers for 2.0 so Beta version will start that process
 - a routing broker to connect different communication types and brokers together in the same federation

### Improvements
 - composable type system for data transfer
 - Major API changes based on feedback and initial broader usage
 - Change C interface to use an error object instead of return code and corresponding updates to the Interface API's
 - Initial performance and scalability testing
 - relaxed subscription/endpoint registration requirements
