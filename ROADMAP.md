# RoadMap
This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change.  see the [projects](https://github.com/GMLC-TDC/HELICS-src/projects) for additional details


## [1.3] ~ 2018-07-31  Improvements planned before the 2.0 release
 - see [HELICS 1.2](https://github.com/GMLC-TDC/HELICS-src/projects/6) for up to date information
### Features
 - Support for TOML configuration
 - Octave support
 - rework of testing executables to reduce build time and simplify testing
 = move most examples to independent repository

### Improvements
 - some initial performance testing and improvements
 - exploration of more general convergence algorithms
 - increased testing of queries and timing and error conditions
 - more detail and testing in the C++ wrapper around the C interface
 - general code refactoring for threading and better modularity

## [2.0 Alpha] ~ 2018-08-29
### Features
 - targeted integration with package managers for 2.0 so alpha version will start that process
 - a routing broker to connect different communication types and brokers together in the same federation
 - add input interface which will be a more general named version of the Subscriptions
 - change filters so they can link to multiple endpoints

### Improvements
 - composable type system for data transfer[will likely be delayed]
 - Major API changes based on feedback and initial broader usage
 - Change C interface to use an error object instead of return code and corresponding updates to the Interface API's
 - Initial performance and scalability testing
 - relaxed subscription/endpoint registration requirements
