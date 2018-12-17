# RoadMap
This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change.  see the [projects](https://github.com/GMLC-TDC/HELICS-src/projects) for additional details


## [2.0 Beta] ~ 2018-10-02
- see [HELICS 2.0](https://github.com/GMLC-TDC/HELICS-src/projects/7) for up to date information
### Features
 - will be released when the 2.0 branch is passing all tests
 - add input interface which will be a more general named version of the Subscriptions
 - change filters so they can link to multiple endpoint
 - Pythonic API wrapper

### Improvements
 - Major API changes based on feedback and initial broader usage
 - Change C interface to use an error object instead of return code and corresponding updates to the Interface API's


## [2.0] ~ 2018-10-31
 - see [HELICS 2.0](https://github.com/GMLC-TDC/HELICS-src/projects/11) for up to date information
### Features
  - package manager integration
  - C# interface
  - Matlab wrapper API
  - a routing broker to connect different communication types and brokers together in the same federation
  - targeted integration with package managers for 2.0 so alpha version will start that process
  - a single socket tcp broker

### Improvements
  - additional tests of language API's and timing for new features and capabilities
  - Additional performance and scalability testing

## [2.1] ~ 2018-11-21
- see [HELICS 2.X](https://github.com/GMLC-TDC/HELICS-src/projects/10) for up to date information
- all features here are tentative and subject to change
### Features
 - additional package manager integration
 - Julia interface
 - convergence app for helping with convergence issues between multiple federates

### Improvements
 - Additional performance and scalability testing and improvements
