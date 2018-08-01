# RoadMap
This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change.  see the [projects](https://github.com/GMLC-TDC/HELICS-src/projects) for additional details


## [2.0 Alpha] ~ 2018-08-29
- see [HELICS 2.0 Alpha](https://github.com/GMLC-TDC/HELICS-src/projects/7) for up to date information
### Features
 - targeted integration with package managers for 2.0 so alpha version will start that process
 - a routing broker to connect different communication types and brokers together in the same federation
 - add input interface which will be a more general named version of the Subscriptions
 - change filters so they can link to multiple endpoints
 - exploration of more general convergence algorithms
 - C# interface
 - Pythonic API wrapper
 - Matlab wrapper API

### Improvements
 - Major API changes based on feedback and initial broader usage
 - Change C interface to use an error object instead of return code and corresponding updates to the Interface API's
 - Initial performance and scalability testing
 - relaxed interface registration requirements
 - additional tests of language API's and timing

## [2.0 Beta] ~ 2018-09-30
 - see [HELICS 2.0 Beta](https://github.com/GMLC-TDC/HELICS-src/projects/11) for up to date information
### Features
  - package manager integration

### Improvements
  - composable type system for data transfer
  - Additional performance and scalability testing
