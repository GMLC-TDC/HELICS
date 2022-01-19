# Linux Installations

## Ubuntu Installation

### Requirements

- Ubuntu 18 or newer
- C++17 compiler (GCC 7.4 or newer -- GCC 7.3.1 has a bug and won't work)
- CMake 3.10 or newer (if using clang with libc++, use 3.18+)
- git
- Boost 1.67 or newer
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

## A few Specialized Platforms

The HELICS build supports a few specialized platforms, more will be added as needed. Generally the build requirements are automatically detected but that is not always possible. So a system configuration can be specified in the HELICS_BUILD_CONFIGURATION variable of CMake.

### Raspbery PI

To build on Raspberry PI system using Raspbian use `HELICS_BUILD_CONFIGURATION=PI` This will add a few required libraries to the build so it works without other configuration. Otherwise it is also possible to build using `-DCMAKE_CXX_FLAGS=-latomic`
