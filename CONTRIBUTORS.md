# Contributors
This file describes the contributors to the HELICS library and the software used as part of this project
If you would like to contribute to the HELICS project see [CONTRIBUTING](CONTRIBUTING.md)
## Individual contributors
### Pacific Northwest National Lab
 - Jeff Daily (Now AMD)
 - Andy Fisher*
 - Jason Fuller*
 - Shwetha Niddodi*
 - Monish Mukherjee*
 - Jacob Hansen*
 - Marc Eberlein*

### Lawrence Livermore National Lab
 - Ryan Mast*
 - Steve Smith
 - Philip Top*

### National Renewable Energy Lab
 - Himanshu Jain*
 - Dheepak Krishnamurthy*
 - Bryan Palmintier*
 - Bryan Richardson**

### Argonne National Lab
 - Shrirang Abhyankar
 - Karthikeyan Balasubramaniam*

 `*` currently active
 `**` subcontractor

## Used Libraries or Code

### [BOOST](https://www.boost.org)
  Boost is used throughout the code.  The unit, integration, and system test suite is written using boost test.  The IPC core uses the boost interprocess library, the program options library is used for parsing command line options and the filesystem library is used frequently when file manipulation is needed.  Some algorithms and other libraries are also used throughout the code. Boost is licensed under the boost license

### [Asio](https://think-async.com/Asio)
  Asio is used for TCP and UDP communication, as well as resolving IP addresses. Asio is licensed under the [Boost Software License](https://github.com/chriskohlhoff/asio/blob/master/asio/LICENSE_1_0.txt).

### [cppzmq](https://github.com/zeromq/cppzmq)
  The header only bindings for the ZeroMQ library are used to interact with the ZeroMQ library the header files are modified to include some string operations and are included in HELICS source.  Cppzmp is licensed under the [MIT](https://github.com/zeromq/cppzmq/blob/master/LICENSE) license.

### [jsoncpp](https://github.com/open-source-parsers/jsoncpp)
  JsonCPP is used for parsing json files, it was chosen for easy inclusion in the project and its support for comments. Jsoncpp is licensed under public domain or MIT in case public domain is not recognized [LICENSE](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE)

### [tinytoml](https://github.com/mayah/tinytoml)
  Tinytoml is used for parsing toml files.  TinyToml is licensed under [BSD 2-clause](https://github.com/mayah/tinytoml/blob/master/LICENSE)  The header file is included in HELICS source

### [GridDyn](https://github.com/LLNL/GridDyn)
GridDyn supports HELICS in experimental versions, and several components of GridDyn code were used in the development of HELICS, given they have several of the same authors.  

### [libguarded](https://github.com/copperspice/libguarded)
several components of libguarded are being used in the core and application libraries to better encapsulate the locks for threading.  The library was modified to allow use of std::mutex and std::timed_mutex support for the shared_guarded class, and also modified to use handles.  An atomic_guarded and a staged_guarded class were added for std::atomic like support for allocating classes.  Libguared is licensed under [BSD 2 clause](https://github.com/copperspice/libguarded/blob/master/LICENSE).  The modified version being used in HELICS is available at https://github.com/phlptp/libguarded/tree/modified

### [fmt](http://fmtlib.net/latest/index.html)
FMT replaces boost::format for internal logging and message printing.  The library is included in the source code.  The CMAKE scripts were modified so they don't trigger a bunch of unnecessary checks and warnings as nearly all checks are already required for building HELICS based on minimum compiler support.  HELICS uses the header only library for the time being.  FMT is licensed under [BSD 2 clause](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst)

### [Cereal](https://github.com/USCiLab/cereal)
The cereal library is used for serialization of messages sent around inside HELICS.  Cereal is included in the HELICS source and licensed under [BSD-3 clause](https://github.com/USCiLab/cereal/blob/master/LICENSE)

### [FNCS](https://github.com/FNCS/fncs), [IGMS](https://www.nrel.gov/docs/fy16osti/65552.pdf), and FSKIT
While not used directly, much of the inspiration for HELICS comes from three separate projects at the different National Labs.  These include FNCS at PNNL, FSKIT at LLNL(unreleased), and IGMS(unreleased) at NREL.  The lessons learned from these three co-simulation platforms was fed directly into the design of HELICS, and the hope that the combination and partnership is better than any one lab could have accomplished on their own.  

### [c++17 headers](https://github.com/tcbrindle/cpp17_headers)
HELICS makes use of `C++17` headers, but due to `C++14` compatibility requirements these are not available on all supported compilers.  So included library headers are used from @tcbrindle including std::any, std::optional and std::string_view.  These fall under the boost license, this library is an aggregate from a number of different sources, see the readme at the above link for more details.  The Boost versions of these libraries are not used due to incompatibilities through different boost versions that HELICS supports, so a single stable source was used.  When the minimum boost version and compiler is updated this code will likely be removed.     

### [mpark/variant](https://github.com/mpark/variant)
This variant was chosen for compatibility with C++17 over boost variant and better cross platform support than some of the other versions available.  The single header version is included with the source.  [Boost Software License](https://github.com/mpark/variant/blob/master/LICENSE.md)

### cmake scripts
Several cmake scripts came from other sources and were either used or modified for use in HELICS.
 - Lars Bilke [CodeCoverage.cmake](https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake)
 - NATIONAL HEART, LUNG, AND BLOOD INSTITUTE  FindOctave.cmake
 - clang-format, clang-tidy scripts were created using tips from [Emmanuel Fleury](http://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html)
 - Viktor Kirilov, useful cmake macros [ucm](https://github.com/onqtam/ucm)  particularly for the set_runtime macro to use static runtime libraries
