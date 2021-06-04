# Changelog

All notable changes to this project after the 1.0.0 release will be documented in this file

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

A note on future revisions.
Everything within a major version number should be code compatible (with the exception of experimental interfaces). Everything within a single minor release should be network compatible with other federates on the same minor release number. Compatibility across minor release numbers may be possible in some situations but we are not going to guarantee this as those components are subject to performance improvements and may need to be modified at some point. Patch releases will be limited to bug fixes and other improvements not impacting the public API or network compatibility. Check the [Public API](./docs/Public_API.md) for details on what is included and excluded from the public API and version stability.

## [2.7.1][] - 2021-06-03

There were several bug fixes in this patch release. Some of them related to changes in [2.7.0][] and some new ones that came up from bug reports. Some new enhancements are experimental signal handlers in the C-api, which will be used in the python interface to provide a little better user experience when trying to kill a co-simulation.

### Changed

- String output on recorders is now always JSON compatible and allows escaped characters. This allows some additional values to be displayed in ascii format vs base 64 encoding. #1910
- Players read the string fields through a JSON parser unless marked with b64\[\] to match the string output on recorders #1910
- The default webserver port is now 8080 to allow user space execution on non-Windows platforms #1936

### Fixed

- An issue with recorders writing text fields in the incorrect order which could result in incorrect playback #1910
- Fix an issue with core naming that occasionally resulted in same broker name errors when using default names on federates #1919
- Fix an issue where queries were not being resolved when a core disconnects which could result in deadlock. #1931
- The `wait_for_current_time` flag was not working properly in some cases where time interruption was also taking place #1933
- Fixed issue with the webserver not responding with the index page when requested or detecting the correct broker for certain trivial requests #1936

### Added

- Signal handlers for catching SIGINT and optional user callback are available in the C shared API #1915
- Added support for environment variables for setting some network connection settings and other information #1921
- Queries now have timeouts #1931
- Command line and environment variable options for setting the webserver port numbers #1936

## [2.7.0][] - 2021-04-28

This release includes a major change internally for filters. Testing and usage revealed some scenarios which could cause deadlock or acausal message arrival. These scenarios were not common so the release was delayed until a fix was in place. As of the 2.7.0 release all the identified issues related to the initial bug have been resolved. There remains some outstanding cases that fail rarely in the CI systems specifically related to rerouting filters that are separate from both the location they are rerouting from and to. The resolution of these is uncertain but will be available in a patch release when resolved. Additional changes include major changes to the CI builds due to changing support of Travis CI and other CI services.

### Changed

- Update spdlog, fmtlib, filesystem, asio, and units libraries to latest releases (#1798, #1802, #1803)
- Default `HELICS_USE_ZMQ_STATIC_LIB` to `ON` if only the static library is found in the search path #1845
- Primary CI systems are now on azure instead of travis #1819
- Only a very limited CI test set is run on formatting PR's #1761

### Fixed

- Tests and fixes allowing multiple filters on the same endpoint #1852
- Fixed some failing broker server tests related to input arguments #1825
- Fixed an issue with barrier and maxTime requests #1788
- Fixed a timing bug when using offset and some specific time requests immediately after the enterExecutingMode #1759
- Several fixes and changes to CI systems related to changes in CI infrastructure #1754, #1790, #1828, #1744, #1739
- Fixed deadlock caused when querying a disconnected HELICS object #1705
- Fixed major timing bug with the use of filters #1717
- Fixed issue when sending messages before execution time #1717

### Added

- Support for ZMQ 4.3.4 (this will become default in the next version) #1841
- Added a `global_flush` query to sweep the internal action message queues #1832
- A vcpkg manifest file for some vcpkg support #1835
- Added an event triggered flag to better handle timing on federates that are primarily or exclusively triggered by events like filters #1804
- Added ordered queries which allow queries to run on the normal vs priority pathways for queries that are desired to be synchronous with the other helics messages #1793
- Added github workflow to compress images #1626
- Additional and clearer warning messages when a message is sent to an unknown destination #1702

## [2.6.1][] - 2020-10-15

Several small bug fixes and minor enhancements to query operations

### Changed

- In `helics_enum.h` flags were separated into separate enums with the same symbols splitting up flags specific to federates, cores, and those applicable to all entities
- CMAKE 3.18 was tested and verified and used as the baseline version when available.
- Default libzmq was updated to 4.3.3

### Fixed

- A few flags were unable to be queried through `getOptionFlag` operations #1655
- The index values for some flags were not able to be retrieved via getFlagIndex operations #1645
- In some cases specifying a custom port of a number less than the default ports led to federates being unable to bind the appropriate port #1648
- Duplicate target specification and warnings were improved #1639
- Certain property strings did not generate the correct property index #1642
- For large packets in the TCP core on particular operating systems partial buffers may be sent and this was not handled property in the tcp core #1600
- Boost 1.74 deprecated some interfaces used in the webserver. The code was updated to support the latest release of boost. #1629
- The requested_time field in the `current_time` query for federates was missing #1619
- Some broker queries did not reset properly when changes in the federation occurred #1617
- Handle cases of empty install prefix #1577

### Added

- The C api now has a query callback method for responding to federate specific queries. #1634
- Some tutorials for the hello_world example on visual studio #1621
- A `helicsMessageClear` method was added to the C API for clearing the data from a message object #1622
- A `global_state` query to actively query the current state of all objects in a federation. #1614
- A strict config checking flag to generate errors on potentially incorrect configuration files #1607
-

## [2.6.0][] - 2020-08-20

Bug fixes and major logging update

### Changed

- The build flag function now returns correct debug or release flags depending on the build
- The debug postfix `d` is no longer added to the interface libraries
- Spdlog is now being used for logging inside HELICS and the old logger has been removed this results in fewer thread being generated by HELICS.
- CMake will now error if the install directory is set to the build directory
- Some argument names in the C API have been changed for consistency
- Output a more descriptive error message for mismatched data sizes when converting types #1521
- Some C++98 API functions were added and changed for consistency, specifically endpoint get type no returns a `char *` instead of std::string, and a getCurrentTime function was added to `Federate`
- logging level properties from a federateInfo structure will be inherited by a core for the first registered federate

### Fixed

- String with negative numerical values were not acknowledging the negation Issue #1306
- Config file parsing was not acknowledging "unit" string #1512
- A performance issue with the tcpss and tcp cores in some cases has been resolved by setting the no_delay option
- Inconsistency in type returned by endpoint getType in C++98 API #1523
- a potential segmentation fault when calling some methods in the C shared library after calling helicsCloseLibrary

### Added

- Flags for `dumplog` and `force_logging_flush` were added to the C API
- Added missing C++98 call to `getCurrentTime`
- Added `closeLibrary` function to the C++98 API
- Added a Python benchmark file
- An option to install the benchmark executables has been added
- Data logging output for both send and receive of messages
- A GitHub Actions workflow to build packages for Linux with the benchmark executables

### Removed

- The previous logger including logger.h has been replaced with spdlog

## [2.5.2][] - 2020-06-15

Bug fix release for some build issues and a fix to the `wait_for_current_time` flag

### Fixed

- Bug in the timing subsystem that was preventing the `wait_for_current_time` flag from functioning properly
- Fixed some oddities in the java tests, that were doing confusing things and happened to work because of the bug in the timing subsystem
- A build system issue that caused the automated generation of python packages to fail on the 2.5.1 release. This was caused by overriding the output build location in all cases, when it should have been limited if the python interface is getting built separately.
- A few cppcheck issue from the new check (#1414)

### Added

- Add print_systeminfo flag to root helics_benchmark command (#1417)
- Added cppcheck github action for PR's

## [2.5.1][] - 2020-06-05

### Changed

- All ZeroMQ related files are now located in the network library and under a single namespace
- Use Python 3.8 instead of 3.6 for any release build installers that include a copy of the Python interface (`pip` or `anaconda` are the recommended ways to install the Python interface)
- Update units library to include some typical natural gas units and conversions
- Use a separate action for automated pr generation
- Update the CLI11 library
- The setOption/getOption functions now take an int32_t as a value instead of a boolean. This does not change the API since in the C library the `helics_bool` was already an int.
- In the case of multiple sources, `getInjectionType`, and `getInjectionUnits` now will return a json string vector.
- The CMake build generation now uses a central location for all build artifacts instead of individual directories.
- Updated the ASIO library to 1-16
- Minor updates to the clang-format to allow better alignment and comment reflow
- Numerous code refactorings to simplify and clean up code
- Move all ZMQ related items to the network library
- Updated Python packages DLL load failed error to suggest installing the latest Visual C++ Runtime

### Fixed

- Sporadic failures in the Webserver and websocket tests
- A bug in the translation of vectors to complex vectors
- A bug in the copying of vectors into the C shared library
- Numerous clang-tidy identified issues mostly for code readability
- Some issues with the exists query not working in certain circumstances and for cores and brokers
- The nonlings test would fail if the branch name had `error` in it. A check was put into eliminate this false negative test failure.
- A few sporadic failure cases in the http and websocket tests
- A build generation issue with disabling the ZMQ core
- An error from the config interpreter where some flags were not getting correctly propagated to the Federate configuration.

### Added

- A helics::zmq target was added for linking with zeromq if using HELICS as a subproject
- A `HELICS_BENCHMARK_SHIFT_FACTOR` CMake option was added to allow the benchmarks to scale depending on computational resources
- "version" and "version_all" queries to get the local version string and the version strings of all the cores/brokers in the federation
- A few missing operations to the C++98 interface for Message objects, add `helicsMessageClone` and `helicsEndpointCreateMessage` functions in the C interface. Add a test case for some of the C++98 message operations.
- `helicsQuerySetTarget` and `helicsQuerySetQueryString` operations to modify an existing query in the C interface
- A set of reduction operations for multi-input handling on inputs, options for setting input source priority and the number of expected connections
- A Watts-Strogatz like benchmark
- A few more parameters that can be handled in the Webserver and support for use of uuid instead of name
- A few missing message operators to the C++98 API, such as `data`, `append`, `setFlag`, `checkFlag`, `sendMessageZeroCopy`
- Made the Message class return a self Reference for the setters
- A test to run the helics-broker executable as part of the CI tests
- A bug in the helics_broker that no longer ran correct defaults
- A "version_all" query, to retrieve the version of HELICS in use for all cores/brokers, and a "version" query to retrieve the version of a specific target.
- A series of checks for markdown, spelling, shellcheck, python formatting, cpplint, end-of-line and automated generation of PR scripts for the formatting updates.
- Some level of automated scaling for benchmarks for small systems
- API functions for retrieving the build flags used to generate the library
- Some additional message interpreters to aid in debugging
- A test using the standalone `helics_broker` to run an example

## [2.5.0][] - 2020-04-26

Some library reorganization, additional static analysis(CppLint and clang-tidy), multiBroker, Webserver updates including a websocket interface and the ability to create and destroy brokers from the HTTP and websocket interfaces.

### Changed

- Split the HELICS core library into a separate core and network library
- Update FMT library to version 6.2.0
- The core and broker Factories use a map instead of a fixed list which is a step toward allowing user defined cores
- Updated CLI11 included code to customized version to allow configuration of cores and brokers through Json files
- The ordering of the helics_error_types enum is in ascending order
- Refactored the Matlab and Java swig interface builds to enable standalone builds

### Fixed

- Added CPPlint and fixed a number of issues that check identified.

### Added

- `helicsEndpointSendMessageObjectZeroCopy` to allow transferring messages with minimal copying.
- `helics<Interface>IsValid` functions to the C API
- `helicscpp::cleanHelicsLibrary` to the C++98 API.
- A Comm factory to the Core to enable constructing Comm interfaces directly from the type.
- The REST API in the webserver was updated to include ability to create and destroy brokers.
- A websocket server similar to the REST API but will respond to JSON queries.
- A test suite for the HTTP and websocket servers.
- A Multibroker that can allow multiple communication types to interact together.
- Support for `HELICS_BUILD_CONFIGURATION` cmake variable for building on unique platforms. The only supported one right now is "PI" for building on raspberry pi platforms.

### Deprecated

- in the C shared library `helicsFederateGetMessage`, `helicsEndpointGetMessage`, and `helicsEndpointSendMessage` are deprecated in favor of the object version These will be removed in HELICS 3.0
- deprecated `helicsEndpointClearMessage` this function does nothing right now, all messages are on a federate level.

### Removed

## [2.4.2][] - 2020-03-27

Increased code coverage and additional bug fixes.

### Changed

- Update toml11 library to 3.3.1 with some warning fixes for C++17
- The query handling in the core library was cleaned up to be more extensible and uniform

### Fixed

- MacOS build with python 2.7 failure
- Fixed some issues with the build/test process if the `ENABLE_ZMQ_CORE=OFF`
- Fixed a potential issue with queries if they are triggered before the connection ack
- An issue with host name resolution on some systems with restricted DNS lookup
- Allow camelCase in file parameters from JSON
- Fixed linking error with default OpenMPI Spack package
- Fixed timing benchmark federate name

### Added

- A series of tests for MessageFederate.cpp to increase coverage on that file to 100%
- Callbacks for custom filters in the C shared library
- A series of tests for CoreApp, BrokerApp, and FederateInfo and a few fixes for them
- A few additional tests of helics supports types and conversions
- CoreApp has a connect() and reset() method and constructor from a Core shared pointer
- BrokerApp has a connect() method and constructor from a Broker shared pointer
- Added a data_flow_graph query which gets all the connections in a federation

### Deprecated

### Removed

## [2.4.1][] - 2020-03-06

Increased code coverage and additional bug fixes. The error propagation in HELICS was improved such that local errors can be escalated to global errors, and a federate can define errors coming from the federate that are handled appropriately and can halt a co-simulation.

### Changed

- The HELICS webserver will build by default if the conditions are met
- Update filesystem library to [v1.3.0](https://github.com/gulrak/filesystem/releases/tag/v1.3.0)
- The behavior of the `Federate*Complete` slightly modified to be uniform and consistent, no API changes
- Configuration of flags and targets for interfaces in JSON and TOML files can be done in multiple sections
- The benchmark federates have been changed to use a common base benchmark federate class for more consistent behavior
- Switched to including netif as a git submodule
- the `error` Function in the C++ API is now the same as `localError` previously it was primary useful for logging and didn't do much, and will be deprecated in the next release.
- Updated the GitHub actions (clang-format, swig interface updates, and release builds) to use actions/checkout@v2
- Cleaned up the Windows installer (better component names/descriptions and groups, link to Gitter, and require installing Headers to install SWIG)
- Updated the HELICS apps manpages with new options

### Fixed

- Issue with iterative requests that were not being honored if the federate was acting in isolation
- A few pathways which would allow segmentation faults if a federate was disconnected and particular functions were called
- ValueFederate `addIndexedTargets`, the function template would not work as was written and was unusable, it is now tested and operational.

### Added

- `HELICS_DISABLE_WEBSERVER` option to turn off building of the webserver. It will build by default if Boost is enabled and is version 1.70 or higher; otherwise it is disabled.
- A series of tests for Federate.cpp to increase coverage on that file to 100%
- A series of tests for ValueFederate.\*pp to increase coverage on that file to 100%
- Docker image for a helics builder which includes build tools and the helics installation
- helics can be installed on [MSYS2](https://helics.readthedocs.io/en/latest/installation/windows.html#msys2) using pacman.
- Standalone benchmark federates for use in multinode benchmark runs
- A FreeBSD 12.1 CI build using Cirrus CI
- Sending an event from GitHub Actions release builds to trigger updating additional HELICS packages when a new release is made
- `localError`, and `GlobalError` function calls the Federate API and in the C++ and sharedLibrary.
- `helics_terminate_on_error` flag to escalate what would be a local error into a global one that will halt the co-simulation. This flag can be specified through the flag to federates or to brokers and cores through a command line option `--terminate_on_error`
- `addDependency` function was added to the C++ Federate API and shared library API, it can add a direct dependency between federates manually.
- A 32-bit Windows zip install archive for releases
- "global_time", "current_time", and "state" queries for brokers and cores, and "current_time" query for federates.
- Support for a 'helics-release-build' event trigger to the release build GitHub Actions workflow

### Deprecated

### Removed

- `HELICS_ENABLE_WEBSERVER` option to enable the webserver. This option was added as experimental in 2.4.0
- VS2015 Windows Server 2012 CI build is removed. Azure Pipelines is [removing the image](https://devblogs.microsoft.com/devops/removing-older-images-in-azure-pipelines-hosted-pools/) because it is outdated and sees little use. VS2015 is still tested through Appveyor for the time being.

## [2.4.0][] - 2020-02-04

A few bug fixes, code coverage on the shared library increased to 100%, library updates, Broker server enhancements including an http REST API, and a lot of work on the build systems to enable easier releases and packaging.

### Changed

- filesystem include updated to 1.2.10
- CLI11 updated to 1.9
- fmt updated to 6.1.2
- variant header updated to latest release
- Update the units library (v0.3.0)
- The TOML interpreter used in HELICS was changed to [toml11](https://github.com/ToruNiina/toml11)
- Some unnecessary files were removed from the all source package
- some internal CMake options and messages were not using new format for message
- Major updates to the python modules build system including merging the python3 and python 2 builds into the same CMake generator
- CMake cleanup and formatting
- A series of changes to the build for more widely supported installations on Linux and MacOS
- The .clang-format file was modified slightly and the entire code base reformatted to the new specification
- the metadata information for the benchmarks was updated
- The FilterOperator class was altered to include a vector output for use with cloning
- TCP and UDP core types are not by default backwards compatible with <2.4 brokers. The flag "--noack_connect" will need to be passed as part of the coreinitialization string to allow this to work if need be. ZMQ_SS cores are not considered interoperable with <2.4 brokers due to a number of bugs. In a few select cases it might still work.

### Fixed

- macOS rpath information on the built binaries
- Some issues with swig include directories to allow it to work in other circumstances
- an issue with building the java interface in MSYS2
- an issue with the `HELICS_USE_NEW_PYTHON_FIND` CMake option
- Some thread sanitizer identified issues
- A series of issues from static analyzers
- an issue in the shared library create core that could emit an exception
- A series of issues related to remote cloning filters not being inline
- Several issues with the zmqss core type it is not backwards compatible with <2.4 brokers
- The [code coverage](https://codecov.io/gh/GMLC-TDC/HELICS) on the C shared library was increased to 100% and a number of small bugs fixed as a result. The overall coverage increased to 71.5%

### Added

- Several installers for Linux and Mac and builds for `pip install`
- Allow standalone builds for the python interface
- Added a Ring Message benchmark, like the ring Benchmark except using messages as the token
- Added a Multinode phold benchmark
- Added a c shared library echo benchmark
- git logic to check if the submodules are at the correct version
- an option for a githook to check the formatting
- git warning if the submodule versions are not at the correct version
- a timing benchmark similar to the echo benchmark
- a number of tests for the C shared library including evil tests for testing bad input
- Hooks to test the coverage builds
- a feature to mark a broker or federate as slow responding so it doesn't time out automatically
- EditorConfig and .gitattributes for cleaner diff and automatic editor configuration
- An incorrect call in the Matlab swig code was fixed
- Automatic generation of pull requests for source code formatting
- Add metadata information to the benchmarks for automatic processing
- Broker server functionality for tcp core, zmqss core, and udp core
- An experimental web server that can be used with the broker server or the broker executables. (requires boost 1.70+ to build)
- man pages for the helics applications

### Deprecated

- The `ZMQ_SS` is not generally compatible between 2.3 and 2.4 Minor releases due to bug fixes.

### Removed

## [2.3.1][] - 2019-11-22

Bug Fixes and some code refactoring, pkg-config files have been added to the installs

### Changed

- Default installation path for MSYS2 is now configured to be part of the system path, typically `/mingw64/` or `/mingw32/`
- `HELICS_ENABLE_SLOW_PACKAGING_TESTS` renamed to `HELICS_ENABLE_SUBPROJECT_TESTS` to better reflect usage
- filesystem library updated to clear up some warnings
- The CI system now runs Xcode9 as the oldest release
- Automatic releases build system was changed to use scripts

### Fixed

- Some documentation links in the docs
- Missing `helics-enums.h` header from the install if `HELICS_BUILD_CXX_SHARED_LIB` was not enabled
- ZMQ install locations on Linux and macOS if ZMQ is used as a subproject without the HELICS_USE_ZMQ_STATIC_LIB option enabled
- The linux shared library release build so it is compatible with a larger number of systems including older ones back to glibc 2.12.
- Fix some documentation and issues with using the STATIC_STANDARD_LIB CMake option

### Added

- CMake option for `HELICS_DISABLE_ASIO` to completely remove the use the ASIO library, turns off the UDP, and TCP core types, all real-time capabilities, and timeout and heartbeat detection for cores and brokers. ASIO doesn't support all version of cygwin.
- pkg-config files for the shared libraries are now installed to `<prefix>/lib/pkg-config` on unix like systems
- Tests and CI builds for installed CMake package files and pkg-config files

### Deprecated

- Trying to install on linux/macos systems with cmake older than 3.13 and ZMQ used as a subproject with the shared library is no longer supported. It is likely this use scenario was broken before, now it produces a warning.

### Removed

- If `HELICS_BUILD_BENCHMARKS` is enabled, the option for `ENABLE_INPROC_CORE` will not show in the cmake-gui.
- If `HELICS_BUILD_TESTS` is enabled, the option for `ENABLE_TEST_CORE` will not show in the cmake-gui.

## [2.3.0][] - 2019-11-12

Minor release with lots of CMake updates and build changes and a few fixes and additions. The biggest change is in the C++ shared library and complete removal of boost\:\:test.

### Changed

- Converted the shared_library_tests and application_api tests to use Google test instead of Boost test
- Most HELICS CMake options have changed to `HELICS_**`, with the exception of `BUILD_XXX_INTERFACE`, and `ENABLE_XXX_CORE`. These options will not change until HELICS 3.0, at which point all HELICS related CMake options that are not standard CMake options will have a leading `HELICS_`
- The version string printed by HELICS will include git hash codes and base version information for non-release builds
- Some attempts were made to further modernize the usage in CMake. This effort ended up fixing a few bugs in certain conditions and simplifying things, and the CMake code was also run through a formatter
- The exported C++ shared library has been heavily modified to only include functions in the public API, and is now the recommended way to link with HELICS directly in a C++ program. A `HELICS::helics-shared` target is now available for linking through CMake. If libraries were previously linking with the installed static library this is a BREAKING Change. Those previously linking with the C++ shared library may also need modifications. Changes include:
  - The coreFactory and brokerFactory headers are deprecated as part of the public headers, they are still used internally but should not be used by linking libraries. The public version will remain stable but show deprecated messages. The internal version used by the core will likely be modified in the future.
  - New headers for CoreApp and BrokerApp can be used to provide nearly all the same capabilities in the application API.
  - New headers `typeOperations.hpp` and `timeOperations.hpp` were added to the application_api to provide string operations for the time and core types. In the shared-library core-time, and core-type headers included these headers but that will be deprecated in the future.
  - CMake options for building utilities/units/json as object libraries have been removed as they were no longer needed.
  - The cereal library is moved to the external folder in the helics directory and is now required to be available for the C++ shared library, so a CMake variable making it optional was removed.
  - The reason for this change was partly as a stepping stone for other internal library changes, and to simplify the build complexity and allow more flexibility in linking libraries without impacting the installed interfaces. The previous methods and installed libraries were coming into conflict with other packages and posing increasing challenges in maintenance and linking. This change forced more separation in the HELICS layers, and the installed libraries and simplified a lot of the build generation scripts.
- CLI11, utilities, filesystem and units libraries were updated with latest revisions.

### Fixed

- Race condition when removing subscriptions or targets from an interface
- Fixed mistakenly excluded tests and the resulting failures in the CI builds
- Some of the interface functions (Python, Java, etc) were generating incorrect code for getting raw data from inputs.
- The language API's were not handling Ctrl-C user disconnects well, so some fixes were added to handle that situation better.

### Added

- A set of included HELICS benchmarks using the Google benchmark library.
  - echo benchmark
  - echo message benchmark
  - ring benchmark
  - PHOLD benchmarks for single machine
  - message size and count benchmark
  - filter benchmark based on echo message benchmark
  - actionMessage benchmarks
  - data conversion benchmarks
- The src, test, benchmarks directory can now be used as a root directory for CMake to do the appropriate build with few options.
- Dedicated internal functions for conversion of bool operators, strings such as "off", "false", "disabled", "inactive" are now supported as valid bool values that can be passed.
- Shared libraries for the C++ Application api and apps library are built and installed containing only public API functions and classes. **potential breaking change as the CMake library names have changed and the C++ shared library is modified**
- Tests executing and linking with the shared libraries
- Example linking with the shared libraries
- a `build_flags_target` is exported with flags that may effect compilation
- a `compile_flags_target` is exported, mostly for seeing which non-abi related flags HELICS was built with.
- a `helicsXXXMakeConnections` function which takes a file to establish linkages for Core and Broker to the C shared API.
- Automated generation of interface code for Python, Matlab, and Java interfaces and automatic PR's with the changes
- Overloads of federate creation functions in C++ for CoreApp
- Overloads of filter creation function in C++ to use CoreApp
- Docstrings were added using swig -doxygen to Python, Python2 and Java interfaces
- Add "queries" query to core, federate, and broker which gets a list of available queries
- Add "isconnected", "filters", "inputs" query to core to retrieve list of available filters and inputs, and if the core is connected.
- Added an INPROC core type, which replaces the TEST core for most user uses, the TEST core does the same thing but has additional functionality to mock network issues for testing, more of these capabilities will be added. The INPROC core will remain simplified and as fast as possible for in process federations.
- Windows CI builds for visual studio 2019, 2017, 2015 on Azure, reduced workload on Appveyor.
- Automatic release file generation for a shared library package on macOS and Linux, and a more complete macOS installation archive. Supported versions are macOS Catalina 10.15 and Ubuntu 18.04, though the macOS binaries might work as far back as 10.11 and the Linux binary should work for older versions and different distributions.

### Deprecated

- Use of coreFactory and brokerFactory when using the C++ shared library (use CoreApp and BrokerApp instead)
- coreType and helics-time string conversion functions are no longer defined in the helics-time header. They are still there currently but are deprecated and will be removed in HELICS 3.0
  use the typeOperations.hpp and timeOperations.hpp header instead which now defines those functions.

### Removed

- All tests using boost\:\:test have now been replaced with Google test, so references and linking to boost\:\:test has been removed
- Exporting and installing the static libraries has been removed (they can still be used by using HELICS as a CMake subproject)
- CMake option to exclude static libs from the install has been removed as no longer needed
- CMake options for building JSONCPP, Utilities, and units libraries as object libraries have been removed as object libraries are no longer being used
- JSONCPP, Utilities, and units libraries are no longer installed in any form, libraries or headers.
- CMake option to install CEREAL headers (they are now required, but are in a different location)

## [2.2.2][] - 2019-10-27

Bug fix release

### Fixed

- Links in the README changed with an automated move to travis-ci.com
- Fix issue #853, which was causing core connections to timeout if no direct communication was observed for a period of time. This bug fix release fixes that issue where the pings were not being correctly accounted for in the timeout detection.
- Fix Ctrl-C issue when using HELICS in some language api's (python and Julia)

## [2.2.1][] - 2019-09-27

Minor release with bug fixes and a few additional features

### Changed

- helics apps tests is converted to use Google test and is now being run through the sanitizers
- **BREAKING CHANGE** The C interface helics logging callback specifications now include a user data object. This is technically a breaking change, but there were a few issues with the current implementation so it is not entirely clear it was usable as it was. There are now some tests for the functionality. This log callback specification was not available in the language API's and the C++ API has not changed, only the C interface to specifying direct logging callbacks. This is considered a minor change due to no known users of this interface at present and as it was it wasn't entirely operational. No further changes are expected.
- The use of Boost C++ in the helics core and application api are now limited to the IPC core(there are no plans to remove this usage) and an option to `DISABLE_BOOST` is available in the CMAKE files. This will turn off the IPC_CORE and any optional uses of boost in some of the libraries. Future features may use Boost but should retain the ability to disable its use.
- **BREAKING CHANGE** Some function names in the C\+\+98 API were changed to better match the C\+\+ API and were documented more completely through doxygen, these were listed as potentially changing in the [Public API](/docs/Public_API.md) so this is not a consideration for semantic versioning. The C++98 API also has limited numbers of users at this point yet and may not be fully stable until HELICS 3.0 release
- The doxygen CMake project was renamed from `doc` to `helics_doxygen`
- several variables used by submodules in CMake were hidden
- updated zmq subproject version to 4.3.2

### Fixed

- There was a 32 bit issue when using certain vector operation functions in HELICS when compiled with 32 bit, this was preventing the arm 32 from running the tests fully. This issue has been fixed.
- Fixed a race condition related to queries of subscriptions and inputs of a federate if done remotely. The core could lock or a race condition could occur.
- some issues related to file logs
- started to address some recommendations for `include-what-you-use`
- The CMake conditions for building the C# interface and Python2 interface were not completely correct and incorrectly showed an error which was also incorrectly ignored, so it all worked unless there was an actual error, but those issues have been resolved.

### Added

- logMessage functions in the federate for user specified log messages and levels
  - `logDebugMessage`, `logWarningMessage`, `logErrorMessage`, `logInfoMessage` function in all API's to simplify common logging operations
- function to set the log file from the core C++ API
- A CMAKE option to disable BOOST entirely `DISABLE_BOOST`
- A CMAKE option `HELICS_BINARY_ONLY_INSTALL` which will restrict the install to executables and shared libraries with no headers or static libraries.
- Some CMAKE capabilities to better generate the interface files.
- Timeouts on the broker for broker connections, more work is likely needed in the future but for now if a path times out, if things were already disconnecting it assumes it is the equivalent of a disconnect, and if not the federation errors and terminates.
- Automatic release file generation for visual studio builds, windows installers, full source code tar files, and a shared library package.

### Removed

- The included build files for the Octave interface have been removed. It is now required to use swig to build these files. The interface file was only valid for Octave 4.2 and had potential to break in later versions. Given the 3 versions of octave in common use it was deemed prudent to just remove the included file and require swig to generate the correct interface, this may be added back in the next release if more testing shows this to not be an issue.

## [2.2.0][] - 2019-08-26

Minor release with some updates to the networking portion of HELICS and some API additions.

### Changed

- Submodule updates for filesystem, libfmt, and google test
- A utilities lib containing many string processing and small functions is now used instead of directly including it.

### Fixed

- A error response to a core registration will immediately generate an error on federates waiting for registration instead of waiting for a timeout
- HELICS can now compile with standalone mingw and cygwin 32 bit on Windows. ASIO is not compatible with Cygwin 64 bit so no support for that is expected in the near future. Tests in travis exercise the MinGW build.
- Some issues with the ZMQ core generating an error on close due to incorrect builds of ZMQ in some installations.
- Some changes to the network interface selection process that cause issues on certain platforms.

### Added

- The ability to specify a broker key for brokers and cores to limit linking to those cores with the appropriate key
- A units library into HELICS, mismatched units are checked and units published as a double with units on the publication and subscription converted internally
- A new API for messages in the C interface. The old interface has difficulties when working with binary data in the message structure. So a message object API was created with appropriate methods to access the data. The previous message API will be deprecated in release 2.3 and removed in 3.0.
- A clone app for cloning an existing federate including all publications and subscriptions and all data that is being sent out. It is accessible through the helics_app clone subcommand
- CI tests using docker for clang memory sanitizer and the octave interface.
- Scripts for generating a single zip file with all the code including submodules. This will be generated for each new release.
- A broker server that generate multiple brokers on a single system and handles the port allocation intelligently. (Only ZMQ currently supported, this is not backwards compatible, though regular 2.2 brokers should work with 2.1 federates if needed.)
- A Docker image containing the HELICS apps (available on Docker Hub for tagged releases and the latest develop branch at [https://hub.docker.com/r/helics/helics](https://hub.docker.com/r/helics/helics))

### Removed

- ENABLE_SWIG option in CMake as always ON. This option will only appear for interfaces that have existing build files. For swig generated interfaces that do not have prebuilt files (octave, python2, and C#) this option will no longer appear as swig is required.

## [2.1.1][] - 2019-07-15

Minor release which fixes a few bugs and add some JSON related input and queries

### Changed

- moved concurrency related structures to a standalone library
- System-tests is now based on google test instead of boost test
- Shared_libary_cpp tests now based on google_test instead of boost test
- the deserializer for `ActionMessage` now uses `memcpy` to avoid possible undefined behavior
- The value of `helics_time_maxtime` has been changed for consistency with the C++ equivalent
- The return type of the helicsCLI11App is now named `parse_output` instead of `parse_return`
- fmt and googletest were updated to latest version

### Fixed

- a few possible race conditions found by thread-sanitizer
- cleared up a couple scenarios that were triggering occasional test failure in the system tests
- `helics_broker` and `helics_app` were returning non-zero return values when `--version` or `--help` were used, they now return 0 in those cases
- a small memory leak when a JSON stream builder was created and not destroyed properly
- an inconsistency between the `helics_time_maxtime` in the C shared library and the maxTime value used in C++, this could in some cases result in failing termination conditions

### Added

- queries for getting all current inputs in JSON format.
- query for getting all updated inputs in JSON format
- publication function that accepts a JSON structure for multiple publications
- registration function that generates publications based on same JSON structure as the function that accepts JSON for group publication
- function on the inputs to clear the updates, is used from a query
- a const version of the `isUpdated` function call on inputs
- Shared OBJECT (SO) versions to the shared libraries

### Removed

- libguarded and several concurrency related structures as they are now in a standalone repository that is included through submodules

## [2.1.0][] - 2019-06-27

The main focus of this minor release is cleaning up the build system and extracting required compiled libraries from the HELICS build process, no changes in the C API, and a few additions and deprecations in the C++ API related to command line arguments.

### Changed

- remove use of boost::program options and replace usage with CLI11
- remove boost::asio and replace with a submodule for ASIO
- remove included fmt code and replace with submodule
- remove JsonCpp code and replace with a submodule which generates a compiled library - this removed the need to continually regenerate the single header/file with customized namespaces, though if you are using the helics-static library built with a separate JsonCpp static library, the HELICS copy of the jsoncpp static library must be linked with manually (for build systems other than CMake such as waf, meson, premake, etc). Also included is an option to incorporate JsonCpp as an object library within a single helics-static library (default on macOS/Linux), and create a target HELICS::jsoncpp_headers.
- extract several containers used in HELICS to a separate repository for better maintenance and possible reuse elsewhere. Any reference to the containers library was removed from the Public API.
- all required references to boost were removed from the public API.
- the logger headers were split into two sections. The logger.h which includes the logger objects for use in federates was split from the loggerCore which is not publicly accessible.
- The command line arguments are error checked and the help prints all available options (thanks to CLI11)
- the core tests and common tests now use google test instead of boost test. More tests are expected to be migrated in the future.
- updates to the HELICSConfig.cmake file that gets installed to be more resilient to different directory structures.
- use ZMQ as a subproject if needed instead of an autobuild and install it as a target if needed. The CMake option to enable this is ZMQ_SUBPROJECT, replacing AUTOBUILD_ZMQ.
- the cereal library is not installed by default except on visual studio, and there is a CMAKE option to install it `HELICS_INSTALL_CEREAL`
- some update to the noexcept policy on c++98 interface

### Fixed

- an issue with the isUpdated function not registering access (mainly an issue in the C and language interfaces), Issue #655
- certain flags when used with certain brokers could cause errors, Issue #634
- certain flags when used with certain brokers could cause errors Issue #634
- potential issue with only_update_on_change_flag when used at the federate level, along with some tests

### Added

- the HELICS library can now operate as a subproject in a larger CMake project if needed
- tcp cores have a --reuse-address flag to allow multiple brokers on the same port, mostly useful for the test suite to prevent spurious failures due to the OS not releasing tcp ports in a timely manner.
- several C++ api functions for using a vector of strings as command line arguments, in the federates and in the broker/core factory, this is related to the transition to CLI11
- tests for building HELICS with musl instead of glibc
- tests for building HELICS on ARM/ARM64

### Removed

- tested support of XCode 6.4 and 7.3; these probably still work but we are not testing them anymore.

## [2.0.0][] - 2019-02-12

This is a major revision so this changelog will not capture all the changes that have been made in detail. Some highlights:

- major revision to the API including
  - use of an error object in the C api function instead of a return code.
  - better match the C++ api in terms of function names and layers.
  - The C++ api now uses objects for the interfaces instead of identification ids.
- Filters can have multiple Targets
- Define an input object which can be addressed from outside the federate
- add a ZMQ_SS core type useful for large numbers of federates on a single machine.
- add a TCP_SS socket for firewall usage though it may be applicable in other situations
- numerous bug fixes and internal refactorings.
- add target functions to the interface objects to add and remove targets
- functions to allow cores and brokers to add links between federates
- an octave interface
- an early version of the C# interface.
- an ability to set a global value (as a string) that can be queried.
- a local info field for all the interfaces for user defined string data.
- many other small changes.
- License file changed to match BSD-3-clause exactly(terms are the same but the file had some extra disclaimers in it, now it matches the standard BSD-3-clause license)
- tag source files with appropriate licensing information

## [1.3.1][] - 2018-09-23

### Changed

- wait_for_Broker now uses a condition variable instead of sleep and checking repeatedly
- changed the logging levels to be error, warning, summary, connections, interfaces, timing, data, and trace to better match debugging levels used in development and make the purpose of each level clearer
- comm objects now can use the same logging system as the rest of HELICS

### Fixed

- some race conditions in a few test cases and in user disconnection calls for brokers
- certain types of federates would occasionally hang during off nominal shutdown call sequences. Fixing this led to a substantial rewrite of the tcp comms

### Added

- federate, broker, and core destroy functions to the C api
- tcp cores have a --reuse-address flag to allow multiple brokers on the same port, mostly useful for the test suite to prevent spurious failures due to the OS not releasing tcp ports in a timely manner.

## [1.3.0][] - 2018-07-31

### Changed

- some CMake options have been removed (BUILD_BROKER)
- major changes to the build of the CTest testing Framework
- moved most examples to new [HELICS-Examples](https://github.com/GMLC-TDC/HELICS-Examples) Repo
- added better code for allowing static runtime builds
- use the CMake version numbers instead of independent variables
- Environment variables are recognized in CMAKE find options- split API tests from system wide tests
- added options on MSVC to build with embedded system libraries and embedded debug info.

### Fixed

- potential segmentation fault in C shared library when calling free with invalid object.
- autobuild recognizes build configuration

### Added

- working octave interface for Linux
- some additional tests for the shared library
- TOML readers for interface description in Federates
- interactive command line for helics_broker
- a few new queries on brokers see [Queries](docs/user_guide/Queries.md)
- CPACK can now build a dmg files
- Players can have multiline comments in input file and omit the tag for repeated messages
- marker option on player, recorder, tracer to print time advancement message

### Removed

- Most examples are now located in [HELICS-Examples](https://github.com/GMLC-TDC/HELICS-Examples)

## [1.2.1][] - 2018-06-30

### Fixed

- bug in the conversion of named points from strings
- MATLAB helicsSubscriptionGetVector function was producing a segmentation fault, now this is fixed
- performance issue in the delay buffers of federateState
- findMPI for MPI libraries with multiple libraries
- federates will now error on missing required publications

### Added

- first cut of MATLAB interface tests
- some additional Java test cases
- Python test cases for named point and bool tests
- MATLAB helper scripts for loading the library
- String length function for subscriptions

### Changed

- conversion of doubles into the internal time base now rounds to the nearest ns instead of truncating
- unify CMake scripts to use lower case commands

## [1.2.0][] - 2018-06-18

### Fixed

- issue with various filter types and random drop filters
- a few minor issues with C API (helicsFederateSetMaxIterations)
- potential threading issues when updating the logger on an executing federate
- federates will now propagate errors properly for duplicate publications and endpoints
- federates will now error on missing required publications

### Changed

- implement use of FMT library instead of boost::format
- improved python installation scripts
- general threading and refactoring of the core
- automatic name generation no longer uses random uuid's, but a shorter 20 fully character random string

### Added

- queryCoreExecute, queryBrokerExecute to the C API to allow queries to be executed directly on brokers and cores
- C++ API changes to for Brokers and Core to allow queries
- Get StringLength function to the C and interface API
- new queries (federate_map, and dependency_graph(partial))
- additional filter tests and query tests
- realtime mode for HELICS specified by activating the realtime flag, and specifying rt_lag and rt_lead the federate will then delay grant or force_grant based on computer clock to match wall time.

## [1.1.1][] - 2018-05-25

### Added

- BrokerApp as a slightly more convenient runner to Brokers
- getXXSize functions directly in the Subscription object instead of a roundabout call in the C api
- more complete error catching for the C library
- added helics-config executable for getting paths and links and used flags
- BrokerApp as a slightly more convenient runner to Brokers

### Changed

- upgrade autobuild ZMQ version to 4.2.5 and change CMake scripts to use zmq target
- updated HELICSConfig.cmake install file to link properly to external libraries and find them if necessary, also included some find functions. The find_package(HELICS) should work properly now
- changed boost inclusion to use targets instead of files directly
- changed MPI inclusion to work better on windows and use targets instead of direct links
- update cereal library with latest bug fixes
- update jsoncpp with latest version
- update cppzmq with the latest version
- moved helics_broker executable code to the apps repository
- the CXX shared library can now be built alongside the C shared library and can be built on Windows.

### Fixed

- compilation issue with Xcode 8.0
- inconsistent numerical conversion from vectors to doubles in subscriptions

### Removed

- installation of HELICSImport.cmake this is now redundant with updated HELICSConfig.cmake

## [1.1.0][] - 2018-05-09

### Added

- namedpoint functions in the C++ for publications and subscriptions, and corresponding functions in the C interface and language API's
- Boolean publication and subscription for C++ interface, and corresponding functions in the C interface and language API's
- new options for brokers, --local, --ipv4, --ipv6, --all, are shortcuts for specifying external network interfaces
- additional documentation, CONTRIBUTORS, ROADMAP, CONTRIBUTIONS, and some other documentation improvements

### Changed

- the default interface configuration for federates and brokers. The --interface option is less important as interfaces should mostly get automatically determined by the broker address
- minor configuration changes to CMAKE configuration to be more conforming with modern CMAKE best practices
- cleaned up header installation for app directory
- shared library construction now uses some headers generated by CMAKE

### Fixed

- better error checking in the C interface
- fixes for occasionally failing tests

## [1.0.3][] - 2018-04-28

### Fixed

- Fix bug preventing federates from terminating if its dependencies are disconnected and using purely interrupt driven timing, such as a recorder

## [1.0.2][] - 2018-04-27

### Fixed

- Bug not allowing command line parameters separate from the command if a positional argument was in usage
- Fixed Bug for federate not allowing changes in period or minTimeDelay after entry to execution mode
- added python2 interface option (this will be available but not fully capable going forward)
- A few more race conditions fixed from clang thread-sanitizer

## [1.0.1][] - 2018-04-22

### Fixed

- Allow Boost 1.67 usage
- allow building with AUTOBUILD for ZeroMQ on Linux
- Clang tidy and static analyzer fixes
- fix some potential race conditions spotted by clang thread-sanitizer
- Fix some documentation to better match recent updates

[1.0.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.0.1
[1.0.2]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.0.2
[1.0.3]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.0.3
[1.1.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.1.0
[1.1.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.1.1
[1.2.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.2.0
[1.2.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.2.1
[1.3.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.3.0
[1.3.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v1.3.1
[2.0.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.0.0
[2.1.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.1.0
[2.1.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.1.1
[2.2.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.2.0
[2.2.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.2.1
[2.2.2]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.2.2
[2.3.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.3.0
[2.3.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.3.1
[2.4.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.4.0
[2.4.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.4.1
[2.4.2]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.4.2
[2.5.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.5.0
[2.5.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.5.1
[2.5.2]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.5.2
[2.6.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.6.0
[2.6.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.6.1
[2.7.0]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.7.0
[2.7.1]: https://github.com/GMLC-TDC/HELICS/releases/tag/v2.7.1
