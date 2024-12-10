# Linux Installations

## Ubuntu Installation

### Requirements

- Ubuntu 22.04 or newer
- C++20 compiler (GCC 11 or clang 15 or higher)
- CMake 3.22 or newer
- git
- Boost 1.73 or newer
- ZeroMQ 4.2 or newer (generally recommended but technically not essential)
- MPI-2 implementation (if MPI support is needed)

### Setup

_Note_: Keep in mind that your CMake version should be newer than the boost version. If you have an older CMake, you may want an older boost version. Alternatively, you can choose to upgrade your version of CMake.

To set up your environment:

1. Install dependencies using apt-get.

   ```bash
   $ sudo apt install libboost-dev
   $ sudo apt install libzmq5-dev
   $ sudo apt install git
   $ sudo apt install cmake-curses-gui #includes ccmake gui
   ```

   As an alternative, you can use [vcpkg](https://github.com/microsoft/vcpkg#getting-started) -- it is slower
   because it builds all dependencies from source but could have newer versions of dependencies than apt-get.
   To use it, follow the vcpkg getting started directions to install vcpkg and then run `cmake` using
   `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake`, or by setting the environment
   variable `VCPKG_ROOT=[path to vcpkg]` prior to running `cmake`.

2. Make sure _CMake_ and _git_ are available in the Command Prompt. If they aren't, add them to the system PATH variable. This should be covered by installing them as above.

   ```bash
   $ which git
   /usr/local/git

   $ which cmake
   /usr/local/cmake

   $ which ccmake
   /usr/local/ccmake
   ```

3. Checkout the source code and build from source:

#### Notes for Ubuntu

```bash

git clone https://github.com/GMLC-TDC/HELICS
cd HELICS
mkdir build
cd build
cmake ../
# the options can be modified by altering the CMakeCache.txt file or by using the ccmake command to edit them
# the cmake GUI will also work to graphically edit the configuration options.
ccmake . #optional, invokes the cmake GUI to edit options
make
make install
```

### Testing

A quick test is to double check the versions of the HELICS player and recorder:

```bash
cd /path/to/helics_install/bin

$ helics_player --version
3.x.x (20XX-XX-XX)

$ helics_recorder --version
3.x.x (20XX-XX-XX)
```

To run a full co-simulation go to the "examples/comboFederate1" folder and run the "run3.sh". This will produce four output files: "broker.out", "fed1.out", "fed2.out", and "fed3.out". Opening "fed1.out" should show it sending messages to fed2 and receiving messages from fed3.

```text
[2022-01-19 11:53:05.308] [console] [info] fed1 (0)::registering PUB fed1/pub
[2022-01-19 11:53:05.308] [console] [info] fed1 (0)::registering Input
entering init State
[2022-01-19 11:53:05.310] [console] [info] fed1 (131072)[t=-9223372036.854776]::Registration Complete
[2022-01-19 11:53:05.311] [console] [debug] fed1 (131072)[t=-9223372036.854776]::Granting Initialization
[2022-01-19 11:53:05.311] [console] [debug] fed1 (131072)[t=-9223372036.854776]::Granted Time=-9223372036.854776
entered init State
[2022-01-19 11:53:05.312] [console] [debug] fed1 (131072)[t=-1000000]::Granting Execution
[2022-01-19 11:53:05.312] [console] [debug] fed1 (131072)[t=0]::Granted Time=0
entered exec State
message sent from fed1 to fed2/endpoint at time 1
[2022-01-19 11:53:05.313] [console] [debug] fed1 (131072)[t=1e-09]::Granted Time=1e-09
processed time 1e-09
received message from fed3/endpoint at 0 ::message sent from fed3 to fed1/endpoint at time 1
received updated value of 1 at 1e-09s from fed2/pub
message sent from fed1 to fed2/endpoint at time 2
[2022-01-19 11:53:05.315] [console] [debug] fed1 (131072)[t=2e-09]::Granted Time=2e-09
processed time 2e-09
received message from fed3/endpoint at 1e-09 ::message sent from fed3 to fed1/endpoint at time 2
received updated value of 2 at 2e-09s from fed2/pub
message sent from fed1 to fed2/endpoint at time 3

...
```

## A few Specialized Platforms

The HELICS build supports a few specialized platforms, more will be added as needed. Generally the build requirements are automatically detected but that is not always possible. So a system configuration can be specified in the HELICS_BUILD_CONFIGURATION variable of CMake.

### Raspberry PI

To build on Raspberry PI system using Raspbian use `HELICS_BUILD_CONFIGURATION=PI` This will add a few required libraries to the build so it works without other configuration. Otherwise it is also possible to build using `-DCMAKE_CXX_FLAGS=-latomic`
