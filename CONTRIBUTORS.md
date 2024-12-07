# Contributors

This file describes the contributors to the HELICS library and the software used as part of this project
If you would like to contribute to the HELICS project see [CONTRIBUTING](CONTRIBUTING.md)

## Individual contributors

### Pacific Northwest National Lab

- [Jeff Daily](https://github.com/jeffdaily) (Now AMD)
- [Andy Fisher](https://github.com/afisher1)
- [Jason Fuller](https://github.com/jcfuller1)
- [Shwetha Niddodi](https://github.com/shwethanidd)
- [Trevor Hardy](https://www.pnnl.gov/people/trevor-hardy)
- [Monish Mukherjee](https://github.com/MuMonish)
- [Jacob Hansen](https://github.com/Jacobhansens)
- [Marc Eberlein](https://github.com/eberleim)
- [Shrirang Abhyankar](https://github.com/abhyshr)
- [Corrine Roth](https://github.com/corinnegroth)
- [Allison Campbell](https://github.com/allisonmcampbell)
- [Emily Barret](https://github.com/ELBarrett)

### Lawrence Livermore National Lab

- [Ryan Mast](https://github.com/nightlark)
- [Steve Smith](https://github.com/smithsg84)
- [Philip Top](https://github.com/phlptp)
- [Denis Nadeau](https://github.com/dnadeau4) (Now LANL)
- [Ben Salazar](https://github.com/salazar33)
- [Brian Kelley](https://github.com/bmkelley)
- [Hoa Ngo](https://github.com/hgngo) (Now SVB)
- [Ai Enkoji](https://github.com/aenkoji1)
- [Nan Duan](https://github.com/nan-duan)
- [Nathan Yee](https://github.com/yee29) (Now Accenture)

### National Renewable Energy Lab

- [Himanshu Jain](https://github.com/HimanshuJain17)
- [Dheepak Krishnamurthy](https://github.com/kdheepak)
- [Bryan Palmintier](https://github.com/bpalmintier)
- [Bryan Richardson](https://github.com/activeshadow)
- [Matt Irish](https://github.com/mattirish)
- [Slava Barsuk](https://github.com/vbarsuk)
- [Joseph McKinsey](https://github.com/josephmckinsey)

### Argonne National Lab

- [Shrirang Abhyankar](https://github.com/abhyshr) (now PNNL)
- [Karthikeyan Balasubramaniam](https://github.com/karthikbalasu)
- [Manoj Kumar Cebol Sundarrajan](https://github.com/manoj1511)

### Other

- [Beroset](https://github.com/beroset)
- [Nitin Barthwal](https://github.com/nitin-barthwal)
- [Parth Bansal](https://github.com/parthb83)
- [Gaurav Kumar](https://github.com/slashgk)
- [Mehdi Chinoune](https://github.com/MehdiChinoune)

## Used Libraries or Code

### [Asio](https://think-async.com/Asio)

Asio is used for TCP and UDP communication, as well as resolving IP addresses. The Asio library is included as a submodule. Asio is licensed under the [Boost Software License](https://github.com/chriskohlhoff/asio/blob/master/asio/LICENSE_1_0.txt).

### [cppzmq](https://github.com/zeromq/cppzmq)

The header only bindings for the ZeroMQ library are used to interact with the ZeroMQ library. The header files are modified to include some string operations and are included in the HELICS source. cppzmq is licensed under the [MIT](https://github.com/zeromq/cppzmq/blob/master/LICENSE) license.

### [JSON for Modern C++](https://github.com/nlohmann/json)

JSON for Modern C++ is used for parsing json files. The single file header and forward declarations is included in this repo [LICENSE](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT).

### [CLI11](https://github.com/CLIUtils/CLI11)

CLI11 is a command line parser used throughout HELICS. Several modification used and tested in HELICS have been merged upstream. The project was created by [Henry Schreiner](https://github.com/henryiii). CLI11 is available under a [3-Clause BSD](https://github.com/CLIUtils/CLI11/blob/master/LICENSE) license.

### [toml11](https://github.com/ToruNiina/toml11)

toml11 is a C\++11 header-only TOML parser/encoder depending only on the C++ standard library. Compatible with the latest version of TOML v1.0.0. toml11 is licensed under the [MIT](https://github.com/ToruNiina/toml11/blob/master/LICENSE) license. HELICS includes it through a submodule of a library [fork](https://github.com/GMLC-TDC/toml11) until the changes can be merged upstream.

### [GridDyn](https://github.com/LLNL/GridDyn)

GridDyn supports HELICS in experimental versions, and several components of GridDyn code were used in the development of HELICS, given they have several of the same authors.

### [libGuarded](https://github.com/copperspice/libguarded)

Several components of libGuarded are being used in the core and application libraries to better encapsulate the locks for threading. The library was modified to allow use of std::mutex and std::timed_mutex support for the shared_guarded class, and also modified to use handles. It is now included through the [gmlc/concurrency](https://github.com/GMLC-TDC/concurrency). libGuarded is licensed under [BSD 2 clause](https://github.com/copperspice/libguarded/blob/master/LICENSE).

### [fmt](http://fmtlib.net/latest/index.html)

fmt is used for string formatting and in the spdlog library as well. Current FMT version is 8.0+ The library is included as a submodule. fmt is licensed under [BSD 2 clause](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst) license.

### [gmlc/containers](https://github.com/GMLC-TDC/containers)

Several containers developed for HELICS and other applications were branched into a separate repository as a header only library. It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/GMLC-TDC/containers/blob/master/LICENSE) license.

### [gmlc/concurrency](https://github.com/GMLC-TDC/concurrency)

Several concurrency related structures and containers were developed for HELICS and other applications and were branched into a separate repository as a header only library and also includes the modified [libGuarded](https://github.com/copperspice/libguarded). It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/GMLC-TDC/concurrency/blob/master/LICENSE) license.

### [gmlc/utilities](https://github.com/GMLC-TDC/utilities)

Several generic operations and utilities from HELICS and GridDyn are encapsulated in a separate repository, mostly dealing with String Operations but including a few other features as well. It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/GMLC-TDC/utilities/blob/master/LICENSE) license.

### [LLNL/units](https://github.com/LLNL/units)

A library that provides runtime unit values, instead of individual unit types, for the purposes of working with units of measurement at run time possibly from user input. It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/LLNL/units/blob/master/LICENSE) license.

### [spdlog](https://https://github.com/gabime/spdlog)

Very fast, header-only/compiled, C++ logging library. The spdlog library is used for logging. It is included in HELICS as a submodule and is released under a [MIT](https://github.com/gabime/spdlog/blob/v1.x/LICENSE) license.

### [FNCS](https://github.com/FNCS/fncs), [IGMS](https://www.nrel.gov/docs/fy16osti/65552.pdf), and FSKIT

While not used directly, much of the inspiration for HELICS comes from three separate projects at the different National Labs. These include FNCS at PNNL, FSKIT at LLNL(unreleased), and IGMS(unreleased) at NREL. The lessons learned from these three co-simulation platforms was fed directly into the design of HELICS, and the hope that the combination and partnership is better than any one lab could have accomplished on their own.

### [gmlc/networking](https://github.com/GMLC-TDC/networking)

A networking library with helper functions around Asio and other network interface and address operations used within HELICS and other related tools. It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/GMLC-TDC/networking/blob/main/LICENSE) license.

### CMake scripts

Several CMake scripts came from other sources and were either used or modified for use in HELICS.

- Lars Bilke [CodeCoverage.cmake](https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake)
- clang-format, clang-tidy scripts were created using tips from [Emmanuel Fleury](http://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html)
- Viktor Kirilov, useful CMake macros [ucm](https://github.com/onqtam/ucm) particularly for the set_runtime macro to use static runtime libraries
- scripts for cloning get repositories are included from [tschuchortdev/cmake_git_clone](https://github.com/tschuchortdev/cmake_git_clone) used with [MIT](https://github.com/tschuchortdev/cmake_git_clone/blob/master/LICENSE.TXT) License
- Some scripts for including google test were borrowed and modified heavily from an old version of [CLI11](https://github.com/CLIUtils/CLI11)

## Optional components

A list of optional component that are not included in HELICS but are optionally used by the library

### [BOOST](https://www.boost.org)

Boost is used in a few places in the code. The IPC core uses the Boost.Interprocess library. Some of the header-only Boost algorithms and other libraries are also used throughout the code. Some of the string parsing can optionally use boost spirit. The webserver that is part of the broker_server uses Boost::Beast. Boost is licensed under the Boost Software License. Boost can be removed entirely from the source code with the use of a [cmake](https://docs.helics.org/en/latest/user-guide/installation/helics_cmake_options.html) flag.

#### [zmq](http://www.zeromq.org)

ZeroMQ is one of many backends that can be used by HELICS for message passing (ZMQ core networking). The automatic download currently uses version 4.3.5. No ZMQ library code is included in the HELICS source. HELICS installers include ZeroMQ binary libraries. ZeroMQ is licensed under [MPL](https://github.com/zeromq/libzmq/blob/master/LICENSE)
Previous versions(prior to 4.3.5) were under LGPL with a modification to allow for linking and in various forms and distribution of the binary under different terms if the library was not modified. Clarification on static linking being okay can be found in [this github issue](https://github.com/zeromq/libzmq/issues/3787). No modification of the ZMQ library or any of the ZeroMQ source files is included in the HELICS source code. Currently the Windows installers and shared library builds static link ZeroMQ. When building from source it is an optional component and can be excluded by setting `HELICS_ENABLE_ZMQ_CORE` to `OFF`

### [Google Test](https://github.com/google/googletest)

HELICS tests are written to use the Google Test and mock frameworks. Google Test is included in the source tarball but is downloaded as an optional component. Google Test is released with a [BSD-3 clause](https://github.com/google/googletest/blob/master/LICENSE) license.

### [Google Benchmark](https://github.com/google/benchmark)

Some timing benchmarks with HELICS are written to use the Google Benchmark library. Benchmarks is an optional component and is not included in the main source tarball and must be downloaded separately. Google Benchmark is released with an [Apache 2.0](https://github.com/google/benchmark/blob/v1.5.0/LICENSE) license.
