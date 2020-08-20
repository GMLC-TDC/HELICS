# Contributors

This file describes the contributors to the HELICS library and the software used as part of this project
If you would like to contribute to the HELICS project see [CONTRIBUTING](CONTRIBUTING.md)

## Individual contributors

### Pacific Northwest National Lab

- [Jeff Daily](https://github.com/jeffdaily) (Now AMD)
- [Andy Fisher](https://github.com/afisher1)
- [Jason Fuller](https://github.com/jcfuller1)
- [Shwetha Niddodi](https://github.com/shwethanidd)
- [Trevor Hardy](https://github.com/trevorhardy)
- [Monish Mukherjee](https://github.com/MuMonish)
- [Jacob Hansen](https://github.com/Jacobhansens)
- [Marc Eberlein](https://github.com/eberleim)
- [Shrirang Abhyankar](https://github.com/abhyshr)
- [Corrine Roth](https://github.com/corinnegroth)

### Lawrence Livermore National Lab

- [Ryan Mast](https://github.com/nightlark)
- [Steve Smith](https://github.com/smithsg84)
- [Philip Top](https://github.com/phlptp)
- [Denis Nadeau](https://github.com/dnadeau4)
- [Ben Salazar](https://github.com/salazar33)
- [Brian Kelley](https://github.com/bmkelley)

### National Renewable Energy Lab

- [Himanshu Jain](https://github.com/HimanshuJain17)
- [Dheepak Krishnamurthy](https://github.com/kdheepak)
- [Bryan Palmintier](https://github.com/bpalmintier)
- [Bryan Richardson](https://github.com/activeshadow)
- [Matt Irish](https://github.com/mattirish)

### Argonne National Lab

- [Shrirang Abhyankar](https://github.com/abhyshr) (now PNNL)
- [Karthikeyan Balasubramaniam](https://github.com/karthikbalasu)
- [Manoj Kumar Cebol Sundarrajan](https://github.com/manoj1511)

## Used Libraries or Code

### [BOOST](https://www.boost.org)

Boost is used in a few places in the code. The IPC core uses the Boost.Interprocess library. Some of the header-only Boost algorithms and other libraries are also used throughout the code. Some of the string parsing can optionally use boost spirit. The webserver that is part of the broker_server uses Boost::Beast from Boost 1.70 or higher. Boost is licensed under the Boost Software License. Boost can be removed entirely from the source code with the use of a cmake flag.

### [Asio](https://think-async.com/Asio)

Asio is used for TCP and UDP communication, as well as resolving IP addresses. The Asio library is included as a submodule. Asio is licensed under the [Boost Software License](https://github.com/chriskohlhoff/asio/blob/master/asio/LICENSE_1_0.txt).

### [cppzmq](https://github.com/zeromq/cppzmq)

The header only bindings for the ZeroMQ library are used to interact with the ZeroMQ library. The header files are modified to include some string operations and are included in the HELICS source. cppzmq is licensed under the [MIT](https://github.com/zeromq/cppzmq/blob/master/LICENSE) license.

### [JsonCpp](https://github.com/open-source-parsers/jsoncpp)

JsonCpp is used for parsing json files. It is included as a submodule from a slightly modified [fork](https://github.com/phlptp/jsoncpp.git) to add support for object libraries and to clean up some warning issues. JsonCpp is licensed under public domain or MIT in case public domain is not recognized [LICENSE](https://github.com/open-source-parsers/jsoncpp/blob/master/LICENSE).

### [CLI11](https://github.com/CLIUtils/CLI11)

CLI11 is a command line parser and was used as a replacement for boost::program_options. The modifications used in HELICS were merged to the CLI11 library and are now fully integrated in the upstream repository as of the 1.9 release of CLI11. The project was created by [Henry Schreiner](https://github.com/henryiii). Release 1.9 the CLI11 single file header is included with a HELICS install. CLI11 is available under a [3-Clause BSD](https://github.com/CLIUtils/CLI11/blob/master/LICENSE) license.

### [toml11](https://github.com/ToruNiina/toml11)

toml11 is a C\++11 header-only TOML parser/encoder depending only on the C++ standard library. Compatible with the latest version of TOML v0.5.0. toml11 is licensed under the [MIT](https://github.com/ToruNiina/toml11/blob/master/LICENSE) license. HELICS includes it through a submodule of a library [fork](https://github.com/GMLC-TDC/toml11) until the changes can be merged upstream.

### [GridDyn](https://github.com/LLNL/GridDyn)

GridDyn supports HELICS in experimental versions, and several components of GridDyn code were used in the development of HELICS, given they have several of the same authors.

### [libGuarded](https://github.com/copperspice/libguarded)

Several components of libGuarded are being used in the core and application libraries to better encapsulate the locks for threading. The library was modified to allow use of std::mutex and std::timed_mutex support for the shared_guarded class, and also modified to use handles. It is now included through the [gmlc/concurrency](https://github.com/GMLC-TDC/concurrency). libGuarded is licensed under [BSD 2 clause](https://github.com/copperspice/libguarded/blob/master/LICENSE).

### [fmt](http://fmtlib.net/latest/index.html)

fmt replaces boost::format for internal logging and message printing. The library is included as a submodule. fmt is licensed under [BSD 2 clause](https://github.com/fmtlib/fmt/blob/master/LICENSE.rst) license.

### [gmlc/containers](https://github.com/GMLC-TDC/containers)

Several containers developed for HELICS and other applications were branched into a separate repository as a header only library. It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/GMLC-TDC/containers/blob/master/LICENSE) license.

### [gmlc/concurrency](https://github.com/GMLC-TDC/concurrency)

Several concurrency related structures and containers were developed for HELICS and other applications and were branched into a separate repository as a header only library and also includes the modified [libGuarded](https://github.com/copperspice/libguarded). It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/GMLC-TDC/concurrency/blob/master/LICENSE) license.

### [gmlc/utilities](https://github.com/GMLC-TDC/utilities)

Several generic operations and utilities from HELICS and GridDyn are encapsulated in a separate repository, mostly dealing with String Operations but including a few other features as well. It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/GMLC-TDC/utilities/blob/master/LICENSE) license.

### [LLNL/units](https://github.com/LLNL/units)

A library that provides runtime unit values, instead of individual unit types, for the purposes of working with units of measurement at run time possibly from user input. It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/LLNL/units/blob/master/LICENSE) license.

### [cereal](https://github.com/USCiLab/cereal)

The cereal library is used for serialization of messages sent around inside HELICS. A modified version of cereal is included in the HELICS source and licensed under the [BSD-3 clause](https://github.com/USCiLab/cereal/blob/master/LICENSE) license. The modifications include modifying the headers to use relative paths instead of absolute so it can be included in different locations.

### [spdlog](https://https://github.com/gabime/spdlog)

Very fast, header-only/compiled, C++ logging library. The spdlog library is used for logging. It is included in HELICS as a submodule and is released under a [MIT](https://github.com/gabime/spdlog/blob/v1.x/LICENSE) license.

### [FNCS](https://github.com/FNCS/fncs), [IGMS](https://www.nrel.gov/docs/fy16osti/65552.pdf), and FSKIT

While not used directly, much of the inspiration for HELICS comes from three separate projects at the different National Labs. These include FNCS at PNNL, FSKIT at LLNL(unreleased), and IGMS(unreleased) at NREL. The lessons learned from these three co-simulation platforms was fed directly into the design of HELICS, and the hope that the combination and partnership is better than any one lab could have accomplished on their own.

### [c++17 headers](https://github.com/tcbrindle/cpp17_headers)

HELICS makes use of `C++17` headers, but due to `C++14` compatibility requirements these are not available on all supported compilers. So included library headers are used from @tcbrindle including std\::any, and std\::string_view; std\::optional is used via [containers](https://github.com/GMLC-TDC/containers). These fall under the boost license, this library is an aggregate from a number of different sources, see the [README](https://github.com/tcbrindle/cpp17_headers/blob/master/README.md) for more details. The Boost versions of these libraries are not used due to incompatibilities through different boost versions that HELICS supports, so a single stable source was used. When the minimum boost version and compiler is updated this code will likely be removed.

### [mpark/variant](https://github.com/mpark/variant)

This variant was chosen for compatibility with C++17 over boost variant and better cross platform support than some of the other versions available. The single header version is included with the source. [Boost Software License](https://github.com/mpark/variant/blob/master/LICENSE.md).

### [gulrak/filesystem](https://github.com/gulrak/filesystem)

A single-header standalone variant of the filesystem library(v1.3.0) is used to avoid compiled libraries in boost. The single header version is included with the source and is released with a [MIT](https://github.com/gulrak/filesystem/blob/master/LICENSE) license (used as of HELICS 2.4.1) prior versions had a BSD-3-Clause license.

### [gmlc/netif](https://github.com/GMLC-TDC/netif)

A header-only library for getting a list of network interfaces in a cross-platform way. It is included in HELICS as a submodule and is released under a [BSD-3 clause](https://github.com/GMLC-TDC/netif/blob/master/LICENSE) license.

### CMake scripts

Several CMake scripts came from other sources and were either used or modified for use in HELICS.

- Lars Bilke [CodeCoverage.cmake](https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake)
- NATIONAL HEART, LUNG, AND BLOOD INSTITUTE FindOctave.cmake
- clang-format, clang-tidy scripts were created using tips from [Emmanuel Fleury](http://www.labri.fr/perso/fleury/posts/programming/using-clang-tidy-and-clang-format.html)
- Viktor Kirilov, useful CMake macros [ucm](https://github.com/onqtam/ucm) particularly for the set_runtime macro to use static runtime libraries
- scripts for cloning get repositories are included from [tschuchortdev/cmake_git_clone](https://github.com/tschuchortdev/cmake_git_clone) used with [MIT](https://github.com/tschuchortdev/cmake_git_clone/blob/master/LICENSE.TXT) License
- Some scripts for including google test were borrowed and modified from [CLI11](https://github.com/CLIUtils/CLI11)
