# RoadMap

This document contains tentative plans for changes and improvements of note in upcoming versions of the HELICS library. All dates are approximate and subject to change, but this is a snapshot of the current planning thoughts. See the [projects](https://github.com/GMLC-TDC/HELICS/projects) for additional details

## \[2.8\] ~ 2021-08-15

This will be the last of the 2.X series releases, there will likely be at least one patch release after this before fully moving to 3.0

- Internal text based (probably JSON) message format option for general backwards compatibility
- Function deprecations to prepare people to move to 3.0

## \[3.1\] ~ 2021-09-15

Mostly things that didn't quite make it into the 3.0 release and a number of bug fixes that come from transitioning to HELICS 3.0.

- Full Dynamic Federation support
- Single thread cores (partial at release)
- Plugin architecture for user defined cores
- SSL capable core via plugin
- full xSDK compatibility
- Much more general debugging support
- performance improvements
- Profiling capability for profiling co-simulations (Some capabilities will be in 2.8 as well)
- bug fixes, tests and tuning related to HELICS 3 increased use.
- Separate out matlab HELICS interface in matlab oriented way vs current swig build
