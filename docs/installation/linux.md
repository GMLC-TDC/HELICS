# Linux Installations

## Ubuntu Installation

### Requirements

- Ubuntu 16 or newer
- C++14 compiler
- CMake 3.4 or newer
- Gcc 4.9 or newer (GCC 7.3.1 has a bug and won't work)
- git
- Boost 1.58 or newer
- ZeroMQ 4.1.4 or newer (if ZeroMQ support is needed)
- MPI-2 implementation (if MPI support is needed)

### Setup

_Note_: Keep in mind that your CMake version should be newer than the boost version. If you have an older CMake, you may want an older boost version. Alternatively, you can choose to upgrade your version of CMake.

To set up your environment:

1. Install dependencies using apt-get.

   ```bash
   sudo apt-get install libboost-dev
   sudo apt-get install libzmq5-dev
   ```

   As an alternative, you can use [vcpkg](https://github.com/microsoft/vcpkg#getting-started) -- it is slower
   because it builds all dependencies from source but could have newer versions of dependencies than apt-get.
   To use it, follow the vcpkg getting started directions to install vcpkg and then run `cmake` using
   `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake`, or by setting the environment
   variable `VCPKG_ROOT=[path to vcpkg]` prior to running `cmake`.

2. Make sure _CMake_ and _git_ are available in the Command Prompt. If they aren't, add them to the system PATH variable.

Getting and building from source:

1. Use `git clone` to to check out a copy of HELICS.

2. Create a build folder. Run CMake and give it the path that HELICS was checked out into.

3. Run "make".

#### Notes for Ubuntu

Building with GCC 4.9 and 5.X on Ubuntu requires some additional flags due to the way Ubuntu builds those compilers
add `-DCMake_CXX_FLAGS="-D_GLIBCXX_USE_C99 -D_GLIBCXX_USE_C99_MATH"` to make it work.
If you built the compilers from source this may not be required.

```bash

git clone https://github.com/GMLC-TDC/HELICS
cd HELICS
mkdir build
cd build
CMake ../
cCMake . # optional, to change install path or other configuration settings
make
make install
```

### Testing

A quick test is to double check the versions of the HELICS player and recorder:

```bash
cd /path/to/helics_install/bin

$ helics_player --version
x.x.x (20XX-XX-XX)

$ helics_recorder --version
x.x.x (20XX-XX-XX)
```

### Building HELICS with python support

Run the following:

```bash
$ sudo apt-get install python3-dev
$ CMake -DBUILD_PYTHON_INTERFACE=ON -DCMake_INSTALL_PREFIX=~/.local/helics-X.X.X/ ..
$ make -j8
$ make install
```

Add the following to your `~/.bashrc` file.

```bash
export PYTHONPATH=~/.local/helics-X.X.X/python:$PYTHONPATH
export PATH=~/.local/bin:$PATH
```

### Testing HELICS with python support

If you open a interactive Python session and import HELICS, you should be able to get the version of `helics` and an output that is similar to the following.

```bash
$ ipython
Python 3.6.4 |Anaconda, Inc.| (default, Jan 16 2018, 12:04:33)
Type 'copyright', 'credits' or 'license' for more information
IPython 6.2.1 -- An enhanced Interactive Python. Type '?' for help.

In [1]: import helics

In [2]: helics.helicsGetVersion()
Out[2]: 'x.x.x (20XX-XX-XX)'

```

## A few Specialized Platforms

The HELICS build supports a few specialized platforms, more will be added as needed. Generally the build requirements are automatically detected but that is not always possible. So a system configuration can be specified in the HELICS_BUILD_CONFIGURATION variable of CMake.

### Raspbery PI

To build on Raspberry PI system using Raspbian use `HELICS_BUILD_CONFIGURATION=PI` This will add a few required libraries to the build so it works without other configuration. Otherwise it is also possible to build using `-DCMAKE_CXX_FLAGS=-latomic`
