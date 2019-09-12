# Changelog
All notable changes to this project after the 1.0.0 release will be documented in this file

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/).  
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

A note on future revisions.  
  Everything within a major version number should be code compatible (with the exception of experimental interfaces).  The most notable example of an experimental interface is the support for multiple source inputs.  The APIs to deal with this will change in future minor releases.  Everything within a single minor release should be network compatible with other federates on the same minor release number.  Compatibility across minor release numbers may be possible in some situations but we are not going to guarantee this as those components are subject to performance improvements and may need to be modified at some point.  Patch releases will be limited to bug fixes and other improvements not impacting the public API or network compatibility.  Check the [Public API](./docs/Public_API.md) for details on what is included and excluded from the public API and version stability.

## \[2.2.1\] ~ 2019-09-15
Minor release with bug fixes and a few additional features
### Changed
-   helics apps tests is converted to use google test and is now being run through the sanitizers
-   **BREAKING CHANGE** The C interface helics logging callback specifications now include a user data object.  This is technically a breaking change, but there were a few issues with the current implementation so it is not entirely clear it was usable as it was.  There are now some tests for the functionality.  This log callback specification was not available in the language API's and the C++ API has not changed, only the C interface to specifying direct logging callbacks.  This is considered a minor change due to no known users of this interface at present and as it was it wasn't entirely operational.  No further changes are expected.  
-  The use of Boost C++ in the helics core and application api are now limited to the IPC core(there are no plans to remove this usage) and an option to `DISABLE_BOOST` is available in the CMAKE files.  This will turn off the IPC_CORE and any optional uses of boost in some of the libraries.  Future features may use Boost but should retain the ability to disable its use.  
- Some function names in the C\+\+98 API were changed to better match the C\+\+ API and were documented more completely through doxygen
- The doxygen project was renamed

### Fixed
-  there was a 32 bit issue when using certain vector operation functions in HELICS when compiled with 32 bit, this was preventing the arm 32 from running the tests fully.  This issue has been fixed.
- some issues related to file logs

### Added
-   logMessage functions in the federate for user specified log messages and levels
    -  `logDebugMessage`, `logWarningMessage`, `logErrorMessage`, `logInfoMessage` function in all API's to simplify common logging operations
-   function to set the log file from the core C++ API
-   A CMAKE option to disable BOOST entirely

### Removed
 

## \[2.2.0\] - 2019-08-26
Minor release with some updates to the networking portion of HELICS and some API additions.

### Changed
-   Submodule updates for filesystem, libfmt, and google test
-   A utilities lib containing many string processing and small functions is now used instead of directly including it.  

### Fixed
-   A error response to a core registration will immediately generate an error on federates waiting for registration instead of waiting for a timeout
-   HELICS can now compile with standalone mingw and cygwin 32 bit on Windows. ASIO is not compatible with Cygwin 64 bit so no support for that is expected in the near future.  Tests in travis exercise the MinGW build.  
-   Some issues with the ZMQ core generating an error on close due to incorrect builds of ZMQ in some installations.  
-   Some changes to the network interface selection process that cause issues on certain platforms.  

### Added
-   The ability to specify a broker key for brokers and cores to limit linking to those cores with the appropriate key
-   A units library into HELICS,  mismatched units are checked and units published as a double with units on the publication and subscription converted internally
-   A new API for messages in the C interface.  The old interface has difficulties when working with binary data in the message structure.  So a message object API was created with appropriate methods to access the data.  The previous message API will be deprecated in release 2.3 and removed in 3.0.  
-   A clone app for cloning an existing federate including all publications and subscriptions and all data that is being sent out.  It is accessible through the helics_app clone subcommand
-   CI tests using docker for clang memory sanitizer and the octave interface.
-   Scripts for generating a single zip file with all the code including submodules.  This will be generated for each new release.  
-   A broker server that generate multiple brokers on a single system and handles the port allocation intelligently. (Only ZMQ currently supported, this is not backwards compatible, though regular 2.2 brokers should work with 2.1 federates if needed.)
-   A Docker image containing the HELICS apps (available on Docker Hub for tagged releases and the latest develop branch at [https://hub.docker.com/r/helics/helics](https://hub.docker.com/r/helics/helics))

### Removed
-   ENABLE_SWIG option in cmake as always ON.  This option will only appear for interfaces that have existing build files.  For swig generated interfaces that do not have prebuilt files (octave, python2, and C#) this option will no longer appear as swig is required.  

## \[2.1.1\] - 2019-07-15
Minor release which fixes a few bugs and add some JSON related input and queries

### Changed
-   moved concurrency related structures to a standalone library
-   System-tests is now based on google test instead of boost test
-   Shared_libary_cpp tests now based on google_test instead of boost test
-   the deserializer for `ActionMessage` now uses `memcpy` to avoid possible undefined behavior
-   The value of `helics_time_maxtime` has been changed for consistency with the C++ equivalent
-   The return type of the helicsCLI11App is now named `parse_output` instead of `parse_return`
-   fmt and googletest were updated to latest version

### Fixed
-   a few possible race conditions found by thread-sanitizer
-   cleared up a couple scenarios that were triggering occasional test failure in the system tests
-   `helics_broker` and `helics_app` were returning non-zero return values when `--version` or `--help` were used, they now return 0 in those cases
-   a small memory leak when a JSON stream builder was created and not destroyed properly
-   an inconsistency between the `helics_time_maxtime` in the C shared library and the maxTime value used in C++, this could in some cases result in failing termination conditions

### Added
-   queries for getting all current inputs in JSON format.
-   query for getting all updated inputs in JSON format
-   publication function that accepts a JSON structure for multiple publications
-   registration function that generates publications based on same JSON structure as the function that accepts JSON for group publication
-   function on the inputs to clear the updates, is used from a query
-   a const version of the `isUpdated` function call on inputs
-   Shared OBJECT (SO) versions to the shared libraries

### Removed
-   libguarded and several concurrency related structures as they are now in a standalone repository that is included through submodules


## \[2.1.0\] - 2019-06-27
The main focus of this minor release is cleaning up the build system and extracting required compiled libraries from the HELICS build process, no changes in the C API, and a few additions and deprecations in the C++ API related to command line arguments.  

### Changed
-   remove use of boost::program options and replace usage with CLI11
-   remove boost::asio and replace with a submodule for ASIO
-   remove included fmt code and replace with submodule
-   remove JsonCpp code and replace with a submodule which generates a compiled library - this removed the need to continually regenerate the single header/file with customized namespaces, though if you are using the helics-static library built with a separate JsonCpp static library, the HELICS copy of the jsoncpp static library must be linked with manually (for build systems other than CMake such as waf, meson, premake, etc).  Also included is an option to incorporate JsonCpp as an object library within a single helics-static library (default on macOS/Linux), and create a target HELICS::jsoncpp_headers.
-   extract several containers used in HELICS to a separate repository for better maintenance and possible reuse elsewhere.  Any reference to the containers library was removed from the Public API.
-   all required references to boost were removed from the public API.  
-   the logger headers were split into two sections.  The logger.h which includes the logger objects for use in federates was split from the loggerCore which is not publicly accessible.  
-   The command line arguments are error checked and the help prints all available options (thanks to CLI11)
-   the core tests and common tests now use google test instead of boost test.  More tests are expected to be migrated in the future.  
-   updates to the HELICSConfig.cmake file that gets installed to be more resilient to different directory structures.
-   use ZMQ as a subproject if needed instead of an autobuild and install it as a target if needed. The CMake option to enable this is ZMQ_LOCAL_BUILD, replacing AUTOBUILD_ZMQ.
-   the cereal library is not installed by default except on visual studio, and there is a CMAKE option to install it `HELICS_INSTALL_CEREAL`
-   some update to the noexcept policy on c++98 interface

### Fixed
-   an issue with the isUpdated function not registering access (mainly an issue in the C and language interfaces), Issue #655
-   certain flags when used with certain brokers could cause errors, Issue #634
-   certain flags when used with certain brokers could cause errors Issue #634
-   potential issue with only_update_on_change_flag when used at the federate level, along with some tests

### Added
-   the HELICS library can now operate as a subproject in a larger cmake project if needed
-   tcp cores have a --reuse-address flag to allow multiple brokers on the same port,  mostly useful for the test suite to prevent spurious failures due to the OS not releasing tcp ports in a timely manner.  
-   several C++ api functions for using a vector of strings as command line arguments, in the federates and in the broker/core factory, this is related to the transition to CLI11
-   tests for building HELICS with musl instead of glibc
-   tests for building HELICS on ARM/ARM64

### Removed
-   tested support of XCode 6.4 and 7.3;  these probably still work but we are not testing them anymore.

## \[2.0.0\] - 2019-02-12

This is a major revision so this changelog will not capture all the changes that have been made in detail. Some highlights:
-   major revision to the API including
    -   use of an error object in the C api function instead of a return code.
    -   better match the C++ api in terms of function names and layers.
    -   The C++ api now uses objects for the interfaces instead of identification ids.
-   Filters can have multiple Targets
-   Define an input object which can be addressed from outside the federate
-   add a ZMQ_SS core type useful for large numbers of federates on a single machine.
-   add a TCP_SS socket for firewall usage though it may be applicable in other situations
-   numerous bug fixes and internal refactorings.
-   add target functions to the interface objects to add and remove targets
-   functions to allow cores and brokers to add links between federates
-   an octave interface
-   an early version of the C# interface.
-   an ability to set a global value (as a string) that can be queried.
-   a local info field for all the interfaces for user defined string data.  
-   many other small changes.
-   License file changed to match BSD-3-clause exactly(terms are the same but the file had some extra disclaimers in it, now it matches the standard BSD-3-clause license)
-   tag source files with appropriate licensing information

## \[1.3.1\] - 2018-09-23

### Changed
-   wait for Broker now uses a condition variable instead of sleep and checking repeatedly

### Fixed
-   some race conditions in a few test cases and in user disconnection calls for brokers
-   certain types of federates would occasionally hang during off nominal shutdown call sequences.  Fixing this led to a substantial rewrite of the tcp comms

### Added
-   federate, broker, and core destroy functions to the C api
-   tcp cores have a --reuse-address flag to allow multiple brokers on the same port,  mostly useful for the test suite to prevent spurious failures due to the OS not releasing tcp ports in a timely manner.  

### Changed
-   changed the logging levels to be error, warning, summary, connections, interfaces, timing, data, and trace to better match debugging levels used in development and make the purpose of each level clearer
-   comm objects now can use the same logging system as the rest of HELICS


## \[1.3.0\] - 2018-07-31

### Changed
-   some CMake options have been removed (BUILD_BROKER)
-   major changes to the build of the CTest testing Framework
-   moved most examples to new [HELICS-Examples](https://github.com/GMLC-TDC/HELICS-Examples) Repo

### Fixed
-   potential segmentation fault in C shared library when calling free with invalid object.
-   autobuild recognizes build configuration

### Added
-   working octave interface for Linux
-   some additional tests for the shared library
-   TOML readers for interface description in Federates
-   interactive command line for helics_broker
-   a few new queries on brokers see [Queries](docs/user_guide/Queries.md)
-   CPACK can now build a dmg files
-   Players can have multiline comments in input file and omit the tag for repeated messages
-   marker option on player, recorder, tracer to print time advancement message

### Changed
-   added better code for allowing static runtime builds
-   use the cmake version numbers instead of independent variables
-   Environment variables are recognized in CMAKE find options- split API tests from system wide tests
-   added options on MSVC to build with embedded system libraries and embedded debug info.

### Removed
-   Most examples are now located in [HELICS-Examples](https://github.com/GMLC-TDC/HELICS-Examples)

## \[1.2.1\] - 2018-06-30

### Fixed
-   bug in the conversion of named points from strings
-   MATLAB helicsSubscriptionGetVector function was producing a segmentation fault, now this is fixed
-   performance issue in the delay buffers of federateState
-   findMPI for MPI libraries with multiple libraries
-   federates will now error on missing required publications

### Added
-   first cut of MATLAB interface tests
-   some additional Java test cases
-   Python test cases for named point and bool tests
-   MATLAB helper scripts for loading the library
-   String length function for subscriptions

### Changed
-   conversion of doubles into the internal time base now rounds to the nearest ns instead of truncating
-   unify cmake scripts to use lower case commands

## \[1.2.0\] - 2018-06-18

### Fixed
-   issue with various filter types and random drop filters
-   a few minor issues with C API (helicsFederateSetMaxIterations)
-   potential threading issues when updating the logger on an executing federate
-   federates will now propagate errors properly for duplicate publications and endpoints
-   federates will now error on missing required publications

### Changed
-   implement use of FMT library instead of boost::format
-   improved python installation scripts
-   general threading and refactoring of the core
-   automatic name generation no longer uses random uuid's, but a shorter 20 fully character random string

### Added
-   queryCoreExecute, queryBrokerExecute to the C API to allow queries to be executed directly on brokers and cores
-   C++ API changes to for Brokers and Core to allow queries
-   Get StringLength function to the C and interface API
-   new queries (federate_map, and dependency_graph(partial))
-   additional filter tests and query tests
-   realtime mode for HELICS specified by activating the realtime flag, and specifying rt_lag and rt_lead  the federate will then delay grant or force_grant based on computer clock to match wall time.

## \[1.1.1\] - 2018-05-25

### Added
-   BrokerApp as a slightly more convenient runner to Brokers
-   getXXSize functions directly in the Subscription object instead of a roundabout call in the C api
-   more complete error catching for the C library
-   added helics-config executable for getting paths and links and used flags
-   added a broker app that can start up a broker easily

### Added
-   BrokerApp as a slightly more convenient runner to Brokers

### Changed
-   upgrade autobuild ZMQ version to 4.2.5 and change CMake scripts to use zmq target
-   updated HELICSConfig.cmake install file to link properly to external libraries and find them if necessary, also included some find functions.  The find_package(HELICS) should work properly now
-   changed boost inclusion to use targets instead of files directly
-   changed MPI inclusion to work better on windows and use targets instead of direct links
-   update cereal library with latest bug fixes
-   update jsoncpp with latest version
-   update cppzmq with the latest version
-   moved helics_broker executable code to the apps repository
-   the CXX shared library can now be built alongside the C shared library and can be built on Windows.

### Fixed
-   compilation issue with Xcode 8.0
-   inconsistent numerical conversion from vectors to doubles in subscriptions

### Removed
-   installation of HELICSImport.cmake  this is now redundant with updated HELICSConfig.cmake

## \[1.1.0\] - 2018-05-09

### Added
-   namedpoint functions in the C++ for publications and subscriptions, and corresponding functions in the C interface and language API's
-   Boolean publication and subscription for C++ interface, and corresponding functions in the C interface and language API's
-   new options for brokers, --local, --ipv4, --ipv6, --all,  are shortcuts for specifying external network interfaces
-   additional documentation, CONTRIBUTORS, ROADMAP, CONTRIBUTIONS, and some other other documentation improvements

### Changed
-   the default interface configuration for federates and brokers.  The --interface option is less important as interfaces should mostly get automatically determined by the broker address
-   minor configuration changes to CMAKE configuration to be more conforming with modern CMAKE best practices
-   cleaned up header installation for app directory
-   shared library construction now uses some headers generated by CMAKE

### Fixed
-   better error checking in the C interface
-   fixes for occasionally failing tests

## \[1.0.3\] - 2018-04-28

### Fixed
-   Fix bug preventing federates from terminating if its dependencies are disconnected and using purely interrupt driven timing, such as a recorder

## \[1.0.2\] - 2018-04-27

### Fixed
-   Bug not allowing command line parameters separate from the command if a positional argument was in usage
-   Fixed Bug for federate not allowing changes in period or minTimeDelay after entry to execution mode
-   added python2 interface option (this will be available but not fully capable going forward)
-   A few more race conditions fixed from clang thread-sanitizer

## \[1.0.1\] - 2018-04-22

### Fixed
-   Allow Boost 1.67 usage
-   allow building with AUTOBUILD for ZeroMQ on Linux
-   Clang tidy and static analyzer fixes
-   fix some potential race conditions spotted by clang thread-sanitizer
-   Fix some documentation to better match recent updates
