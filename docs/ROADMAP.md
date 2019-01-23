# RoadMap

This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library.  All dates are approximate and subject to change. See the [projects](https://github.com/GMLC-TDC/HELICS-src/projects) for additional details


## [2.1] ~ 2019-03-21

- see [HELICS 2.X](https://github.com/GMLC-TDC/HELICS-src/projects/10) for up to date information
- all features here are tentative and subject to change

### Features

 - additional package manager integration
 - Julia interface
 - a major update to the timing system to allow improved testing and timing consistency in large cyclic dependencies
 - remove use of boost::program_options in favor of CLI11
 - performance Improvements
 - multi-broker to allow multiple comms to be connected
 - logging API for user log message
 - allow dynamic publications

### Improvements

 - Additional performance and scalability testing and improvements

### Notes

- drop testing support for Xcode 6.4 builds

## [2.2] ~ 2019-06-21

 - see [HELICS 2.X](https://github.com/GMLC-TDC/HELICS-src/projects/10) for up to date information
 - all features here are tentative and subject to change

### Features and Improvements

  - additional package manager integration
  - remove use of boost::asio in favor of asio standalone to complete removal of boost compiled libraries from the core HELICS library and simplify support
  - single thread CORE_TYPES
  - single federate cores using boost::fibers (will require boost 1.66 or greater)
  - dynamic federations
