# HELICS CMake options

## Main Options

-  `CMake_INSTALL_PREFIX`:  CMake variable listing where to install the files
-   `HELICS_BUILD_APP_LIBRARY` :  \[Default=ON\] tell HELICS to build the [app]() Library
-   `HELICS_BUILD_APP_EXECUTABLES` : \[Default=ON\]build some executables associated with the apps
-   `HELICS_BUILD_BENCHMARKS` :  \[Default=OFF\]Build some timing benchmarks associated with HELICS
-   `HELICS_BUILD_CXX_SHARED_LIB` :  \[Default=OFF\]Build a C++ shared library with the underlying C++ interface to HELICS
-   `HELICS_BUILD_EXAMPLES` :  \[Default=OFF\]Build a few select examples using HELICS,  this is mostly for testing purposes.  The main examples repo is [here]()
-   `HELICS_BUILD_TESTS` :  \[Default=OFF\]Build the HELICS unit and system test executables.
-   `HELICS_ENABLE_LOGGING` :  \[Default=ON\] Enable debug and higher levels of logging,  if this is turned off that capability is completely removed from HELICS
-   `HELICS_ENABLE_PACKAGE_BUILD` : \[Default=OFF\] Enable the generation of some installer packages for HELICS
-   `HELICS_GENERATE_DOXYGEN_DOC` :  \[Default=OFF\] Generate doxygen documentation for HELICS
-   `HELICS_WITH_CMAKE_PACKAGE` : \[Default=ON\] Generate a `HELICSConfig.cmake` file on install for loading into other libraries
-   `BUILD_OCTAVE_INTERACE`  : \[Default=OFF\] Build the HELICS Octave Interface
-   `BUILD_PYTHON_INTERACE`  : \[Default=OFF\] Build the HELICS Python3 Interface
-   `BUILD_PYTHON2_INTERACE`  : \[Default=OFF\] Build the HELICS Python2 Interface (can be used at the same time as BUILD_PYTHON_INTERFACE)
-   `BUILD_JAVA_INTERACE`  : \[Default=OFF\] Build the HELICS Java Interface
-   `BUILD_MATLAB_INTERACE`  : \[Default=OFF\] Build the HELICS Matlab Interface
-   `BUILD_CSHARP_INTERACE`  : \[Default=OFF\] Build the HELICS CSharp Interface

## Advanced Options
