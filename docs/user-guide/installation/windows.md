# Windows Installation

## Windows Installers

Windows installers are available with the different [releases](https://github.com/GMLC-TDC/HELICS/releases). The release includes zip archives with static libraries containing both the Debug version and Release version for several versions of Visual Studio. There is also an installer and zip file for getting the HELICS apps and shared library along with a pre-built Java 1.8 interface. There is also an archive with just the C shared library and headers, intended for use with 3rd party interfaces.

## Build Requirements

- Microsoft Visual C++ 2019 or newer (MS Build Tools also works)
- CMake 3.22 or newer (CMake should be newer than the Visual Studio and Boost version you are using;)
- git
- Boost 1.75 or newer (needs to have boost cmake configuration)- 1.73 and 1.74 may work with older versions of Cmake
- MS-MPI v8 or newer (if MPI support is needed)

## Setup for Visual Studio

_Note_: Keep in mind that your CMake version should be newer than the boost version and your visual studio version. If you have an older CMake, you may want an older boost version. Alternatively, you can choose to upgrade your version of CMake.

### Set up your Environment

**1.** Install Microsoft Visual C++ 2019 or newer that supports C++20 [MSVC](https://visualstudio.microsoft.com/)

**2.** Install [Boost](https://dl.bintray.com/boostorg/release/1.84.0/binaries/)
1.75 or later. For CMake to detect it automatically either
extract Boost to the root of your drive, or set the `BOOST_INSTALL_PATH`
environment variable to the install location. The CMake will only automatically find
Boost 1.67 or newer.
Building with Visual Studio 2019 will require boost 1.75 or newer and CMake 3.22+
or newer.
Boost 1.84 with CMake 3.28+ is the current recommended configuration.

As an (experimental) alternative for installing Boost (and ZeroMQ), you can use [vcpkg](https://github.com/microsoft/vcpkg#getting-started). It is slower
because it builds all dependencies but handles getting the right install paths to dependencies set correctly.
To use it:

1. Follow the [vcpkg getting started directions](https://github.com/microsoft/vcpkg#getting-started) to install vcpkg
2. Run `cmake` using
   `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake`, or by setting the environment
   variable `VCPKG_ROOT=[path to vcpkg]` prior to running `cmake`.

**3.** _Optional_ Only if you need a global Install of ZeroMQ [ZeroMQ](http://zeromq.org/build:_start). We **highly recommend skipping** this step and building HELICS via CMake with the `HELICS_ZMQ_SUBPROJECT=ON` option enabled (which is default on Windows) to automatically set up a project-only copy of ZeroMQ. The ZeroMQ Windows installer is **very** outdated and will not work with new versions of Visual Studio. The CMake generator from ZeroMQ on Windows also works and can be used to store ZMQ in another location that will need to be specified for HELICS.

**4.** _Optional_ Install [MS-MPI](<https://msdn.microsoft.com/en-us/library/bb524831(v=vs.85).aspx>) if you need MPI support.

**5.** _Optional_ Install [SWIG](http://www.swig.org/download.html)
if you wish to generate the interface libraries for Java, appropriate build files are included in the repository so it shouldn't be necessary to regenerate unless the libraries are modified. For C\# a SWIG install is necessary. The simplest way to install SWIG is to use [chocolatey](https://chocolatey.org/) from Windows PowerShell with

```shell
    choco install swig
```

**6.** Open a Developer PowerShell for Visual Studio command line and make sure _CMake_ and _git_ are available in the Command Prompt. This can be done with `Get-Command`. If they aren't, add them to the system PATH variable.

```powershell
PS C:\Users\sampleUser\localrepos\HELICS> Get-Command cmake

CommandType   Name          Version    Source
-----------   ----          -------    ------
Application   cmake.exe     3.22.3.0   C:\Program Files\CMake\bin\cmake.exe


PS C:\Users\sampleUser\localrepos\HELICS> Get-Command git

CommandType   Name          Version    Source
-----------   ----          -------    ------
Application   git.exe       2.35.1.2   C:\Program Files\Git\cmd\git.exe


PS C:\Users\sampleUser\localrepos\HELICS> Get-Command cmake-gui

CommandType   Name          Version    Source
-----------   ----          -------    ------
Application   cmake-gui.exe 3.22.3.0   C:\Program Files\CMake\bin\cmake-gui.exe
```

### Getting and building from source

**1.** Open the Developer PowerShell VS Command prompt. Navigate to where you would like to project to be and use `git clone` to check out a copy of
HELICS.

```bash
    git clone https://github.com/GMLC-TDC/HELICS.git
```

**2.** Go to the checked out HELICS project folder (the default folder
name is HELICS). Create a build folder and go to the build
folder.

```bash
    cd HELICS
    mkdir build
    cd build
```

**3.** Run CMake or CMake GUI. It should automatically detect where MPI is installed if the system path variables are set up correctly, otherwise you will have to set the CMake path manually. `ZMQ_LOCAL_BUILD` is set to `ON` by default
so ZeroMQ will automatically be built unless the option is changed.

Make sure to set `CMAKE_INSTALL_PREFIX` to the path of the install folder.

If you need CMake to use a generator for an IDE or build system other
than the default (ex: Ninja instead of a Visual Studio project), the `-G`
option can be used to specify one of the generators listed by `CMake --help`.
If you are using a Visual Studio generator, such as Visual Studio 2019,
and need to select an architecture other than the default (ex: building a
32-bit target on a 64-bit host or vice versa), the `-A` option can be used
to specify a target platform name. For example, for a 32-bit x86 build with
Visual Studio 2019 on a 64-bit copy of Windows, you would use the cmake
options `-G "Visual Studio 16 2019" -A Win32`. Similarly, `-A x64` can
be used to build for an x64 processor.

Information on CMake usage and cross-compiling for different target
architectures can be found in the CMake documentation at
<https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html>,
and is recommended as a source of information on CMake as it will be
more up-to-date on the latest version of CMake than this guide.

To avoid problems when building later, the target architecture and Visual
Studio version should match the version of the Boost libraries you are using.

If you installed Boost into the root of the C or D drives with the
default location (or the `BOOST_INSTALL_PATH` environment variable has been set), CMake should automatically detect their location. Otherwise the
location will need to be manually given to CMake through `Boost_INCLUDE_DIR`. This should be used if the Boost version is new and not known to HELICS or you want to be explicit about which boost folder to use.
NOTE: CMake 3.14 and later separate the architecture into a separate field for the generator

A basic call with `cmake` using Visual Studio 2022 on a 64bit Windows machine and installing to a folder called `install` inside the repository would be:

```shell
    cmake --install-prefix 'C:\Users\sampleUser\localrepos\HELICS - Copy\install' -G "Visual Studio 17 2022" ..
```

**4.** Open the Visual Studio solution generated by CMake (his can be done from the command prompt with `start HELICS.sln`). In the _Solution Explorer_ Under `Solution 'HELICS'\CMakePredefinedTargets`, right-click on `INSTALL` and select `Build`:

![](https://github.com/GMLC-TDC/helics_doc_resources/blob/main/user_guide/windows_build_vs.png?raw=true)

Alternatively, in the MSBuild command prompt, run the command `msbuild HELICS.sln` from the build folder to compile the entire solution.

"HELICS.sln" can be replaced with the name of one of the projects to build only that part of HELICS.

If the build was successful there should be a "bin" folder inside the "install" folder with `helics.dll` inside (or `helicsd.dll` if Debug mode).

**5.** _Optional_ If interfacing with [PYHELICS](https://python.helics.org/) (assuming already installed via `pip install helics`) the `PYHELICS_INSTALL` environment variable needs to be set to the path of the "install" folder, and "install\bin" (the folder with "helics.dll") needs to be added to the system path. This can be done via the environment variable GUI in windows or temporarily via the command line[^1]

[^1]: Especially if you plan on regularly switching between versions of HELICS temporarily setting `PYHELICS_INSTALL` might not be such a bad idea.

In Cmd:

```shell
set PYHELICS_INSTALL=C:\path\to\HELICS\install
set PATH=%PATH%;%PYHELICS_INSTALL%\bin
```

In Powershell:

```powershell
$env:PYHELICS_INSTALL = "C:\path\to\HELICS\install"
$env:Path += ";$env:PYHELICS_INSTALL\bin"
```

To verify PYHELICS is pointing to the right version try:

```shell
helics --version
```

## Testing

A quick test is to double check the versions of the HELICS player and
recorder (located in the 'build/src/helics/apps/player/Debug' folder):

```shell
> cd C:/Path/To/build/src/helics/apps/Debug

> helics_player.exe --version
x.x.x 20XX-XX-XX

> helics_recorder.exe --version
x.x.x 20XX-XX-XX
```

there may be additional build information if a non tagged version is built.

## MSYS2

MSYS2 provides a Linux like terminal environment on your Windows system. MSYS2 can be installed from [here](https://www.msys2.org/). Once MSYS2 has been installed start up msys2.exe. Follow first time updates as described on the MSYS2 website.

### Using pacman package manager

HELICS is available on the Mingw-32 and Mingw-64 environments through the MSYS2 repositories. From the MINGW64 shell

```bash
$ pacman -Sy mingw64/mingw-w64-x86_64-helics
:: Synchronizing package databases...
 mingw32               453.3 KiB  2.86 MiB/s 00:00 [#####################] 100%
 mingw32.sig           119.0   B  0.00   B/s 00:00 [#####################] 100%
 mingw64               456.0 KiB  2.77 MiB/s 00:00 [#####################] 100%
 mingw64.sig           119.0   B  0.00   B/s 00:00 [#####################] 100%
 msys                  185.9 KiB  1804 KiB/s 00:00 [#####################] 100%
 msys.sig              119.0   B  0.00   B/s 00:00 [#####################] 100%
resolving dependencies...
looking for conflicting packages...

Packages (8) mingw-w64-x86_64-gcc-libs-9.2.0-2  mingw-w64-x86_64-gmp-6.2.0-1
             mingw-w64-x86_64-libsodium-1.0.18-1
             mingw-w64-x86_64-libwinpthread-git-8.0.0.5574.33e5a2ac-1
             mingw-w64-x86_64-mpc-1.1.0-1  mingw-w64-x86_64-mpfr-4.0.2-2
             mingw-w64-x86_64-zeromq-4.3.2-1  mingw-w64-x86_64-helics-2.4.0-1

Total Download Size:    9.17 MiB
Total Installed Size:  65.78 MiB

:: Proceed with installation? [Y/n] y

```

you will be asked to proceed with installation, answering `y` will install HELICS and the required dependencies.

```bash
$ helics_broker --version
2.4.0 (2020-02-16)
```

The helics apps and libraries are now installed, and can be updated when HELICS gets an update. For the MINGw32 use

```bash
$ pacman -Sy mingw32/mingw-w64-i686-helics
```

if you are installing both the 32 and 64 bit versions or just want a simpler command to type

```bash
$ pacboy -Sy helics
:: Synchronizing package databases...
```

if the python interface is needed on MSYS2 it can be installed through pip but requires some setup first.

```bash
$export CMAKE_GENERATOR="MSYS Makefiles"
$pip install helics
```

This will install the HELICS python extension in the correct location. The pacman package should be installed first

### Building HELICS From Source on Windows with MSYS2

After MSYS2 has been successfully updated Some packages need to be installed in order to configure and build HELICS. The following packages need to be installed:

- base-devel
- mingw-w64-x86_64-toolchain
- git
- mingw-w64-x86_64-CMake
- mingw-w64-x86_64-boost
- mingw-w64-x86_64-qt6 (only if you want to be able to run cmake-gui which this guide recommends.)
- mingw-w64-x86_64-zeromq

All packages can be installed by typing the following:

```bash
$ pacman -Sy base-devel mingw-w64-x86_64-toolchain git mingw-w64-x86_64-CMake mingw-w64-x86_64-boost mingw-w64-x86_64-qt5 mingw-w64-x86_64-zeromq
```

For base-devel and mingw-w64-x86_64-toolchain you may have to hit enter for installing all packages that are part of the group package. The qt5 package is quite large, if you are only using it once it might be faster to use ccmake which is a text based interface to CMake. After all the packages have been installed has been done /mingw64/bin must be in the PATH environment variable. If it isn't then it must be added. Please note that this is only necessary if you are compiling in MSYS2 shell. If you are compiling in the MSYS2 MINGW-64bit shell then /mingw64/bin will be automatically added to the PATH environment variable. If not

```bash
$ export PATH=$PATH:/mingw64/bin
```

#### Download HELICS Source Code

Now that the MSYS2 environment has been setup and all prerequisite packages have been installed the source code can be compiled and installed. The HELICS source code can be cloned from GitHub by performing the following:

```bash
$ git clone https://github.com/GMLC-TDC/HELICS.git
```

git will clone the source code into a folder in the current working directory called HELICS. This path will be referred to by this guide as HELICS_ROOT_DIR.

#### Compiling HELICS From Source

Change directories to HELICS_ROOT_DIR. Create a directory called helics-build. This can be accomplished by using the mkdir command. cd into this directory. Now type the following:

```bash
$ cmake-gui ../
```

If this fails that is because mingw-w64-x86_64-qt5 was not installed. If you did install it the CMake gui window should pop up. click the Advanced check box next to the search bar. Then click Configure. A window will pop up asking you to specify the generator for this project. Select "MSYS Makefiles" from the dropdown menu. The native compilers can be used and will most likely default to gcc. The compilers can also be specified manually. Select Finish; once the configure process completes finished several variables will show up highlighted in red. Since this is the first time setup the Boost and ZeroMQ library. Below are the following CMake variables that could to be verified.

- `HELICS_ENABLE_CXX_SHARED_LIB` should be checked if you are using HELICS with GridLAB-D, GridLAB-D dynamically links with the shared c++ library of HELICS, the default is off so you would need to change it

For others the advanced checkbox can be selected to see some other variables

- Boost_INCLUDE_DIR C:/msys64/mingw64/include
- Boost_LIBRARY_DIR_DEBUG/RELEASE C:/msys64/mingw64/bin
- CMake_INSTALL_PREFIX /usr/local or location of your choice
- ZeroMQ_INCLUDE_DIR C:/msys64/mingw64/include
- ZeroMQ_INSTALL_PATH C:/msys64/mingw64
- ZeroMQ_LIBRARY C:/msys64/mingw64/bin/libzmq.dll.a
- ZeroMQ_ROOT_DIR C:/msys64/mingw64

Once these CMake variables have been correctly verified click Configure if anything was changed. Once that is complete click Generate then once that is complete the CMake-gui can be closed.

Back in the MSYS2 command window\[ make sure you are in the build directory\] type:

```bash
$ make -j x
```

where x is the number of threads you can give the make process to speed up the build. Then once that is complete type: `make -j` will just use the number of cores you have available

```bash
$ make install
```

unless you changed the value of CMake_INSTALL_PREFIX everything the default install location /usr/local/helics_2_1_0. This install path will be referred to as HELICS_INSTALL for the sections related to GridLab-D.
If you want to build Gridlab-d on Windows with HELICS see [Building with HELICS](http://gridlab-d.shoutwiki.com/wiki/Building_GridLAB-D_on_Windows_with_MSYS2#Building_with_the_HELICS_Library). Please use branch feature/1179 to build with HELICS 2.1 or later instead of the branch listed.

#### Compiling with clang

Clang does not work to compile on MSYS2 at this time. It has in the past but there are various issues with the clang standard library on MSYS yet so this will be updated if the situation changes. It is getting closer as of (1/30/2020) Mostly it compiles when linked with Libc++ and libc++abi, but there seems to be some missing functions as of yet, so cannot be used other than for some warning checks.

For building with clang using libc++, CMake 3.18+ must be used.

## Building with mingw

HELICS can also be built with the standalone MinGW

- We assume you have MinGW installed or know how to install it.
- [Boost](https://www.boost.org/doc/libs/1_75_0/more/getting_started/windows.html); you can use the [Windows installer](https://sourceforge.net/projects/boost/files/boost-binaries/) for Boost installed in the default location
- Run CMake to configure and generate build files, using "MinGW Makefiles" as the generator,
- Run mingw32-make -j to build

## Building with cygwin

Cygwin is another UNIX like environment on Windows. It has some peculiarities.
HELICS will only build on the 32 bit version due to incompatibilities with ASIO and the 64 bit build. But it does build on the 32 bit versions completely and on the 64 bit version if `HELICS_DISABLE_ASIO=ON` is set

- required packages include CMake, libboost-devel, make, gcc, g++, libzmq(if using zmq)
- use the unix makefiles generator
