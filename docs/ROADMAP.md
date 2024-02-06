# RoadMap

This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library. All dates are approximate and subject to change, but this is a snapshot of the current planning thoughts. See the [projects](https://github.com/GMLC-TDC/HELICS/projects) for additional details

## \[3.6\] ~ Summer 2024

- Single thread cores
- Update IPC core
- Some of the other features listed below
- This release will likely update HELICS to use C++20 and update minimum Compilers, CMake, boost, and other dependencies.
  - GCC 11
  - clang 14
  - CMake 3.22
  - MSVC 16.10
  - XCode 14

## Nearer term features

- Full xSDK compatibility
- Separate Java Interface
- Observer App
- Tag based subscriptions

## Further in the future

- Updated MPI core
- Some sort of rollback operations
- Remote procedure call type of federate
- Plugin architecture for user defined cores
- Separate octave interface
- Enable mesh networking in HELICS
