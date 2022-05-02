# RoadMap

This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library. All dates are approximate and subject to change, but this is a snapshot of the current planning thoughts. See the [projects](https://github.com/GMLC-TDC/HELICS/projects) for additional details

## \[2.8.1\] ~ 2022-05-20

This will be the last of the 2.X series releases

Mainly includes any relevant bug fixes and other minor tweaks for compatibility, and a final refresh of the third-party libraries to bring them up to date.

## \[3.2.1\] ~ 2022-05-28

Mostly refinement of the 3.2 release and additional testing a bug fixes

- Additional refinement on Translators and their interface
- Additional refinement and development of the data API in the C based interface
- Separate out Matlab HELICS interface in Matlab oriented way vs current swig build
- additional testing of the global time coordinator

## \[3.3\] ~ July 2022

Mostly things that didn't make it into the 3.2 release and some features related to performance

- Full Dynamic Federation support (Observer feds in 3.1)
- Single thread cores (partial at release)
- Full xSDK compatibility
- Plugin architecture for user defined cores
- Performance improvements
- Bug fixes, tests and tuning related to HELICS 3 increased use.
- Update of the minimum compiler, CMake, and library requirements.
