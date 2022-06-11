# Changelog

All notable changes to this project after the 1.0.0 release will be documented in this file

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

A note on future revisions.
Everything within a major version number should be code compatible (with the exception of experimental interfaces). The most notable example of an experimental interface is the support for multiple source inputs. The APIs to deal with this will change in future minor releases. Everything within a single minor release should be network compatible with other federates on the same minor release number. Compatibility across minor release numbers may be possible in some situations but we are not going to guarantee this as those components are subject to performance improvements and may need to be modified at some point. Patch releases will be limited to bug fixes and other improvements not impacting the public API or network compatibility. Check the [Public API](./docs/Public_API.md) for details on what is included and excluded from the public API and version stability.

## [3.2.1][] - 2022-06-11

The Matlab interface to HELICS has been moved to [matHELICS](https://www.github.com/GMLC-TDC/matHELICS), the C++ interface now uses `std::string_view` in nearly all places, and the translator C API, and the dataBuffer API to the C interface were fleshed out a bit more and will be finalized in the 3.3 release. Subsequent releases will change the minimum compiler requirements to build HELICS to Visual Studio 2019, GCC 8.0, Clang 7, Xcode 11.0, and CMake 3.11.

### Fixed

- A bug related to iterations and the `wait_for_current_time` flag in which a deadlock may occur if the flag were set and another federate was requesting iterations at time zero.
- A bug which could cause a deadlock if a very large number of iterations occurred
- A naming bug in the creation of globalInput objects through the C API.
- Fixed some potential race conditions in the Webserver
- Fixed some race conditions and potential segmentation faults when running the C shared library tests
- Fix some tests that were failing when executed without ZMQ support.
- Fix the symbol visibility on Linux systems to reduce the number of visible symbols in the C shared library
- Fix an issue with very long file names when used for configuration
- Fix a race condition and deadlock potential in the log buffer tests.
- Fix a Typo in `HELICS_DEPRECATED` use.

### Changed

- Docker images were updated to be based on Ubuntu 22.04.
- All string interfaces in the C++ interface were altered to use `std::string_view` instead of `const std::string &` this corresponds with updates in some third party libraries to better take advantage of the capabilities of std::string_view. The exception to this is strings which reference file names or file contents. These have been left as std::string to match up with iostreams and other file interfaces.
- Updated the Google test version in use.
- Change the symbol visibility for mac systems to be explicitly limited to the HELICS related operations.
- Changed the signal abort code to match standard SIGINT codes
- Some enumerations used inside the CoreLibrary were updated to better match the Style Guide
- Updated the Circle-CI build tests to use more recent compilers and tweak the tests to be more appropriate to test being executed.
- Update the Units, frozen, asio, filesystem, and spdlog libraries to recent releases.
- Updated support CMake 3.23, Boost 1.79, and Visual Studio 2022.

### Added

- Numerous functions in the C data API to support all HELICS supported types
- `helicsFederateProtect` method to all federates to be recovered by name if all references to the C HelicsFederate object was freed.
- Added a callback method for translators in the C API.
- Added a "--force" option for ZMQ brokers to allow a broker to override an existing broker for the network connection and terminate the existing broker to be replaced by a new one.
- Added additional documentation and examples for using iterations

### Removed

- The `HELICS_BUILD_MATLAB_INTERFACE` has been removed and all HELICS support for Matlab in the future will be through the [matHELICS](https://www.github.com/GMLC-TDC/matHELICS) repository.

## [3.2.0][] - 2022-05-03

Major new features include beta release for Translators, global time coordinator and Data API and the addition of Support for encrypted communication through the TCP and TCP SS core types.
Numerous bug fixes were included related to timing in unusual edge cases.

### Fixed

- Fix some undefined behavior warnings with duplicate named federates.
- Fix a discrepancy in the way subscriptions were processed for config files to better match how they are handled for inputs.
- Fix some accidentally disabled test cases testing different network configurations.
- Fix missing dependency generation when subscribing to publications from Endpoints.
- Fixed a few sporadic failures in the test cases.
- Fixed some issues with iterations particularly in initialization mode.
- Fixed a series of edge cases in timing in unusual communication patterns and configurations mostly related to filters, and more complex iteration schemes.
- Fixed an issue related to missing source code on the release source archive.

### Changed

- Docker images were updated to be based on Ubuntu 21.10.
- Update asio, json_cpp, and units to recent releases.
- Errors in the networking layer are now propagated through the logging system in HELICS for better diagnostics of networking issues.
- Benchmark tests are now built with Visual studio 2022.
- Code coverage tests are now run nightly instead of on develop PR's.
- Refactored how Apps were handling arguments to the federate to resolve some oddities in argument processing.
- Update CLI11 to use release 2.2.
- The callback for a filter to can return a message Object.

### Added

- Added support for openSSL based encryption on the tcp tcpss cores and some tests using that capability.
- Added a callback option which triggers when a federate changes mode.
- Added [Translators]() as a beta API, this is not version locked and likely has a few bugs. The API is subject to minor revisions based on user feedback.
- Added Data API to the C interface as a beta. API is subject to further revisions based on user feedback in upcoming releases until 3.3.
- Added support for command operations through the REST API on the webserver.
- Added swagger documentation for the REST API and queries.
- Added a global time coordinator(in Beta) for handling some unusual cases.
- Added additional callbacks in the application API for entry and exit from a timeRequest call.
- Added support for using external FMT and SPDLOG libraries when linking HELICS.
- Added additional documentation for complex networking environments

### Deprecated

- CI tests for visual studio 2017 were deprecated. HELICS still compiles fine on Visual studio 2017. But this is the last release that is specified for. Future releases will not be tested on Visual Studio 2017. HELICS 3.3 will actively change the minimum required to Visual Studio 2019 along with other related minimum compiler updates and requirements

## [3.1.2][] - 2022-01-25

This patch release is primarily to address some build issues on MSYS2, MINGW, and CYGWIN platforms. It also includes some preliminary features for the 3.2 release that were already merged including time monitors, remote logging, and a log buffer.

### Fixed

- Fixed issue building on CYGWIN with the latest FMT library.
- Build issues related to MSYS2 and Mingw in the networking submodule (submodule updated).
- Fixed argument processing issue in the comboFed Example.

### Changed

- Updates to FMT and SPDLOG
- Several documentation updates and cleanup
- Copyright date changed to 2022
- Targeted endpoints now allow all method calls, with the restriction being that the destination must be in the target list otherwise an error is generated. An empty destination in methods that allow destinations will operate identically to calls which do not specify a destination.
- Code cleanup for defining the default port numbers for network core types.

### Added

- A process comms method for federates to instruct HELICS to process messages for a specific amount of wall clock time without granting time or changing state.
- Added a threaded signal handler to deal with some particular issues with killing processes in python and potentially other language interfaces.
- Added a log buffer to brokers/cores/federates to store the latest N messages and retrieve it via a query. See [log buffer](./docs/user-guide/fundamental_topics/logging.md#log-buffer) for more details.
- Added a time monitor object to brokers to use a particular federate as the query time standard for brokers, this does not affect the cosimulation only the logging and queries. See [time monitor](./docs/user-guide/fundamental_topics/logging.md#time-monitor) for more details.
- Added a callback to be executed when the time is updated and before the value or message interface callbacks are executed.
- Added remote logging command to mirror logs from one HELICS object to another. See [remote logging](./docs/user-guide/fundamental_topics/logging.md#remote-logging) for more details.

## [3.1.1][] - 2021-12-14

Primarily a bug fix release to fix a build issue resulting in internal logging messages being disabled for some of the release packages. Also includes a `maxcosimduration` argument for brokers to kill the co-sim after a certain amount of wall clock time.

### Fixed

- CMake issue resulting in internal logging messages being disabled for some builds including some package releases.
- Fixed a few deadlock pathways if the core was killed internally.
- Fixed some timeout issues in the CI builds.
- Fixed a potential race condition when setting the log file after execution has started on helics cores.

### Changed

- CMake version 3.22 tested and used if available.
- The TCP networking components that were not core to HELICS have been moved to a separate repo and linked as a submodule. See [networking](www.github.com/GMLC-TDC/networking).
- Several documentation fixes and updates.
- Moved to using the upstream toml11 repo instead of a custom modified version. Customized modifications have now been merged upstream.

### Added

- '--maxcosimduration' flag added in the broker to specify a max co-sim duration after which the cosimulation will be terminated.
- `helicsGetSystemInfo()` function was added to the API to retrieve HELICS version and system info for debugging and diagnostic purposes.
- Added a threaded signal handler to execute close out operations in a new thread and return control back to the main thread to wait for error signals.
- Added `helicsFederateInfoLoadFromString` to better match C++ API available functions. This loads the federateInfo structure from command line arguments contained in a string.

## [3.1.0][] - 2021-11-25

This release includes some bug fixes and enhancements to HELICS 3 which is now the default. The [migrating 2 to 3](./docs/developer-guide/porting-2-to-3.md) page includes some specific details on migrating from HELICS 2 to 3.

### Fixed

- Fixed an issue with null strings lacking a null termination when retrieved from the C API. This primarily affected the Python and other language API's.
- CMake fix for some older linkers.
- A fix for ASIO aligned_alloc when used with MinGW.
- Fix to IPC core to support Boost 1.77.
- A few issues around the JsonSerialization options for backwards compatibility with HELICS 2, and add some interoperability tests that can be used to check future versions.
- Fix an issue with the timeCoordinator where a large time value was requested and with certain other configuration parameters the time could overflow and cause a deadlock.

### Changed

- precommit-ci now used in the CI chain, pre-commit checks were updated, and a check for python formatting inside the docs was added.
- Update ThirdParty library versions including ASIO, CLI11, toml11, FMT, spdlog, jsoncpp, filesystem.
- Mac binaries are now universal binaries to support ARM based CPU types.
- Update some of the TCP helper classes to better support future encrypted communication efforts.

### Added

- Add the ability to add observer federates which can join a federation after it has started and subscribe to values, or make queries of the federation while in progress.
- Add a configurable grant timeout which will trigger diagnostic action if a federate has been waiting sufficiently long for a time grant.
- A document on the [Type conversion](./docs\developer-guide/typeConversion.md) that HELICS can perform and a series of tests to verify the correct operation of the type conversions.
- Additional missing functions related to linking targeted endpoints from a core, so it can work similar to dataLink methods. These methods were added to Core and Broker applications.
- New benchmark based on the Barabasi-Albert network.

### Deprecated

## [3.0.1][] - 2021-08-26

This release includes some bug fixes and refinements to the build process for HELICS3, as well as general bug fixes and the addition of a compatibility layer making it possible for HELICS 2 federates to interact with a HELICS 3 broker if absolutely required.

### Fixed

- Timing issues using UNINTERUPTIBLE_FLAG and iterations together resulted in incorrect timing
- Some issues with the automated generation of interface files for inclusion in the repository (Java, and Matlab)
- Fixed several broken links in the documentation pages

### Added

- JSON serialization method for HELICS supported data types
- JSON serialization method for HELICS actionMessage to allow some level of backwards compatibility support. The intention is that using the --json flag on a federate will allow full forward compatibility in the future. It is slower but as it is a text stream and it includes a version code, future versions can adapt.
- Profiling capability, see [Profiling](./docs/user-guide/advanced_topics/profiling.md)

## [3.0.0][] - 2021-07-15

HELICS 3.0 is a major update to HELICS. The major features that have been added are the command interface and targeted Endpoints. Internally major changes include updating the minimum compiler to C++17, and updates to internal libraries. The binary serialization protocol was shifted from Cereal to a custom format that embeds the data type so is more suitable to HELICS data types. The initial release is an alpha release for some initial testing.

### Changed

- Data serialization moved to a custom protocol specific to HELICS.
- Minimum build requirements to C++17.
- Minimum boost library for use is 1.67.
- Many of the API functions now use `string_view` instead of `const std::string &`
- The C shared library now comes with only a single header `helics.h` this should be included for all uses of the C shared library
- The name of the C based shared library changed to `libhelics.dll/so`
- The name of the C++ shared library changed to `libhelicscpp.dll/so`
- The name of the apps library changed to `libhelicscpp-apps.dll/so`
- The style of enumerations and structures was changed to match an updated [style guide](./docs/developer-guide/style.md)
- All HELICS specific CMake variables start with `HELICS_`
- The format for log messages now includes a simulation time stamp `[t=xxxx]`
- Log level numerical values have been expanded (multiplied by 3) to allow more gradations in log levels than was previously allowed
- The allowed set of string names has been reduced to avoid confusion and remove duplicate entries
- All queries (except `global_value`) return a valid json string. Errors from queries return a structure with an HTTP error code and message

### Fixed

- All bug fixes included in HELICS 2.X are included in HELICS 3

### Added

- Command interface
- Targeted Endpoints
- Interface Tags
- Federate and Core Tags

### Removed

- Message structure from C API
- Deprecated functions from HELICS 2
- The separate headers for the C shared library are no longer installed. Instead only a single header (`helics.h`) is needed and includes all functions and operations.
- The cereal library is no longer installed or used with HELICS
- The C++ API no longer has generic type support through Cereal.

[3.0.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v3.0.0
[3.0.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v3.0.1
[3.1.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v3.1.0
[3.1.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v3.1.1
[3.1.2]: https://github.com/GMLC-TDC/HELICS/releases/tag/v3.1.2
[3.2.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v3.2.0
[3.2.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v3.2.0

The changelog for HELICS 1.X and 2.X can be found [here](./docs/HELICS2_CHANGELOG.md)
