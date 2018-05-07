# Contributors
This File describes the contributors to the HELICS library and the software used as part of this projects
## Individual contributors
### Pacific Northwest National Lab
 - Jeff Daily (Now AMD)
 - Andy Fisher
 - Jason Fuller
 - Shwetha Niddodi

### Lawrence Livermore National Lab
 - Ryan Mast
 - Steve Smith
 - Philip Top

### National Renewable Energy Lab
 - Himanshu Jain
 - Dheepak Krishnamurthy
 - Bryan Palmintier

### Argonne National Lab
 - Shrirang Abhyankar

## Used Libraries or Code

### [BOOST](www.boost.org)
  Boost is used throughout the code, inluding Asio for the TCP and UDP protocols and Timers.  The unit, integration, and system test suite is written using boost test.  The IPC core uses the boost interprocess library, the program options library is used for parsing command line options and the filesystem library is used frequently when file manipulation is needed.  Some algorithms and other libraries are also used throughout the code.

### [zmq](www.zeromq.org)
  ZeroMQ message passing is used in the ZMQ core networking.  The autobuild currently uses version 4.2.3. No zmq library code is included in the HELICS source

### [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
  JsonCPP is used for parsing json files, it was chosen for easy inclusion the project and its support for comments.

### [tinytoml](https://github.com/mayah/tinytoml)
  Tinytoml is used for parsing toml files.  

### [GridDyn](https://github.com/LLNL/GridDyn)
GridDyn supports HELICS in experimental versions, and several components of GridDyn code were used in the development of HELICS, given they have several of the same authors.  

### [libguarded](https://github.com/copperspice/libguarded)
several components of libguarded are being used in the core and application libraries to better encapsulate the locks for threading.  The library was extended to add std::mutex and std::timed_mutex support for the shared_guarded class.  

### [Cereal](https://github.com/USCiLab/cereal)
The cereal library is used for serialization of messages sent around inside HELICS.  

### [FNCS](https://github.com/FNCS/fncs), IGMS, and FSKIT
While not used directly much of the inspiration for HELICS comes from three separate projects at the different National Labs.  These include FNCS at PNNL, FSKIT at LLNL(unreleased), and IGMS(unreleased) at NREL.  The lessons learned from these three co-simulation platforms was fed directly into the design of HELICS, and the hope that the combination and partnership is better than any one lab could have accomplished on their own.  

### [c++17 headers](https://github.com/tcbrindle/cpp17_headers)
Helics makes use of C++17 headers, but due to C++14 compatibility requirements these are not available on all supported compilers.  So included library headers are used from @tcbrindle including std::any, std::optional and std::string_view.  

###[mpark/variant](https://github.com/mpark/variant)
this variant was chosen for compatibility with C++17 over boost variant and better cross platform support than some of the other versions available.  The single header version is included with the source.

### cmake scripts
Several cmake scripts came from other sources and were either used of modified for use in HELICS.
 - Lars Bilke CodeCoverage.cmake
 - NATIONAL HEART, LUNG, AND BLOOD INSTITUTE  FindOctave.cmake
 - clang-format, clang-tidy scripts were creating using tips from [Emmanuel Fleury](http://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html)
