# Mac Installation

## Requirements

- C++11 compiler (C++14 preferred).
- CMake 3.4 or newer
- git
- Boost 1.58 or newer
- ZeroMQ 4.1.4 or newer (if ZeroMQ support is needed)
- MPI-2 implementation (if MPI support is
  needed)

## Useful Resources

Some basics on using the macOS Terminal (or any Unix/Linux shell) will be useful to fully understand this guide. Articles and tutorials you may find useful include:

- [How to add a new path to PATH](http://osxdaily.com/2014/08/14/add-new-path-to-path-command-line/)
- [Getting to Understand Linux Shell(s)](https://medium.com/coding-blocks/getting-to-understand-linux-shell-s-start-up-scripts-and-the-environments-path-variable-fc672107b2d7)
- [Paths - where's my command](https://developer.ibm.com/tutorials/l-lpic1-103-1/#paths-where-s-my-command-)
- [Unix/Linux for Beginners](https://www.tutorialspoint.com/unix/unix-environment.htm)
- [Settling into Unix](http://matt.might.net/articles/settling-into-unix/).

## Setup

_Note_: Keep in mind that your cmake version should be newer than the boost version. If you have an older cmake, you may want an older boost version. Alternatively, you can choose to upgrade your version of cmake.

To set up your environment:

1. (if needed) Install git on your system for easy access to the
   HELICS source. Download from
   [git-scm](https://git-scm.com/downloads). This installs the
   command line which is described here. GUI's interfaces such as
   [SourceTree](https://www.sourcetreeapp.com/) are another option.
2. (if desired) Many required libraries are easiest installed using
   the [homebrew](https://brew.sh/) package manager. These directions
   assume this approach, so unless you prefer to track these
   libraries and dependencies down yourself, install it if you don't
   have it yet. As an alternative package manager, you can use
   [vcpkg](https://github.com/microsoft/vcpkg#getting-started) -- it
   is slower because it builds all dependencies for source, but instead
   of following step below you could either run `cmake` using
   `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake`
   as shown in the vcpkg getting started instructions, or by setting the
   environment variable `VCPKG_ROOT=[path to vcpkg]` prior to running `cmake`.
3. (if needed) Setup a command-line compile environment

   a) Install a C++11 compiler (C++14 preferred). e.g. `clang`
   from the Xcode command line tools. These can be installed
   from the command line in Terminal by typing
   `xcode-select --install` and following the on-screen
   prompts.
   b) Install cmake with `brew install cmake`. Alternately, a DMG
   file is available for cmake from their
   [website](https://cmake.org/download/).

4. Install most dependencies using homebrew.

   ```bash
   brew install boost
   brew install zeromq
   brew install cmake
   ```

5. Make sure _cmake_ and _git_ are available in the Command Prompt
   with `which cmake` and `which git` If they aren't, add them to the
   system PATH variable.

Getting and building from source:

1. Use `git clone` to check out a copy of HELICS.
2. Create a build folder. Run cmake and give it the path that HELICS
   was checked out into.

   ```bash
   git clone https://github.com/GMLC-TDC/HELICS
   cd HELICS
   mkdir build
   cd build
   ```

## Compile and Install

There are a number of different options and approaches at this point depending on your needs, in particular with respect to programming language support.

<div class="admonition note">

Note: For any of these options, if you want to install in a custom location, you can add the following CMake argument: `-DCMAKE_INSTALL_PREFIX=/path/to/install/folder/`. There are also many other options, and you can check them out by running `ccmake .` in the `build` folder.

Keep in mind running HELICS commands like `helics_app` will not work from just any old random folder with a custom install folder.
You will either need to run them from inside the `bin` subfolder of your custom install, or provide a more complete path to the command.
To run HELICS commands from any folder, you must add the `bin` subfolder of your custom install to the `PATH` environment variable. See the
first link in the [Useful Resources](#useful-resources) section for details.

</div>

### Basic Install (without language bindings)

Run the following:

```bash
cmake ../
ccmake . # optional, to change install path or other configuration settings
make
make install
```

### Building HELICS with python support

Run the following:

```bash
cmake -DBUILD_PYTHON_INTERFACE=ON -DCMAKE_INSTALL_PREFIX=$HOME/local/helics-master/ ..
make -j8
make install
```

Add the following to your `~/.bashrc` file.

```bash
export PYTHONPATH=$HOME/local/helics-master/python:$PYTHONPATH
```

### Building HELICS with MATLAB support

To install HELICS with MATLAB support, you will need to add run cmake with the `-DBUILD_MATLAB_INTERFACE=ON` option.

The important thing to note is that the MATLAB binaries are in the PATH.
Specifically, `mex` must be available in the PATH.

<div class="admonition note">

Note: To check if `mex` is in the PATH, type `which mex` and see if it returns a PATH to the `mex` compiler.

If it does not, you should install MATLAB and add the path to all the MATLAB binaries to your PATH.

```bash
export PATH="/Applications/MATLAB_R2017b.app/bin/:$PATH"
```

</div>

```bash
git clone https://github.com/GMLC-TDC/HELICS
cd HELICS
mkdir build-osx
cd build-osx
cmake -DBUILD_MATLAB_INTERFACE=ON -DCMAKE_INSTALL_PREFIX=$HOME/local/helics-master/ ..
make -j8
make install
```

### Building HELICS MATLAB support manually

If you have changed the C-interface and want to regenerate the SWIG MATLAB bindings, you will need to use a custom version of SWIG to build the MATLAB interface.
To do that, you can follow the following instructions.

- Install [SWIG with MATLAB](https://github.com/jaeandersson/swig/)
- `./configure --prefix=$HOME/local/swig_install; make; make install;`
- Ensure that SWIG and MATLAB are in the PATH

The below generates the MATLAB interface using SWIG.

```bash
cd ~/GitRepos/GMLC-TDC/HELICS/interfaces/
mkdir matlab
swig -I../src/helics/shared_api_library -outdir ./matlab -matlab ./helics.i
mv helics_wrap.cxx matlab/helicsMEX.cxx
```

You can copy these files into the respective `HELICS/interfaces/matlab/` folder and run the cmake command above.
Alternatively, you wish to build the MATLAB interface without using CMake, and you can do the following.

```bash
cd ~/GitRepos/GMLC-TDC/HELICS/interfaces/
mex -I../src/helics/shared_api_library ./matlab/helics_wrap.cxx -lhelicsSharedLib -L/path/to/helics_install/lib/helics/
mv helicsMEX.* matlab/
```

You will need HELICS installed correctly before the above can be run successfully.

### Building HELICS using gcc and python

Firstly, you'll need gcc. You can `brew install gcc`. Depending on the version of gcc you'll need to modify the following instructions slightly. These instructions are for `gcc-8.2.0`.

First you will need to build boost using gcc from source. Download the latest version of boost from the
boost.org website.
In the following example we are doing to use [boost v1.69.0](http://www.boost.org/users/history/version_1_69_0.html)
Keep in mind that your cmake version should be newer than the boost version, so if you have an older cmake you may want an older boost version. Alternatively, you can choose to upgrade your version of cmake as well.

Unzip the folder `boost_1_69_0` to any location, for example Downloads.

```bash
$ cd ~/Downloads/boost_1_69_0
$ ./bootstrap.sh --prefix=/ --prefix=$HOME/local/boost-gcc-1.69.0
```

Open `project-config.jam` and changes the lines as follows:

```bash
# Compiler configuration. This definition will be used unless
# you already have defined some toolsets in your user-config.jam
# file.
# if ! darwin in [ feature.values <toolset> ]
# {
    # using darwin ;
# }

# project : default-build <toolset>darwin ;

using gcc : 8.2 : /usr/local/bin/g++-8 ;
```

```bash
$ ./b2
$ ./b2 install
$ # OR
$ ./bjam cxxflags='-fPIC' cflags='-fPIC' -a link=static install # For static linking
```

This will install boost in the `~/local/boost-gcc-1.69.0` folder

Next, you will need to build HELICS and tell it what the `BOOST_ROOT` is.

```bash
$ cmake -DCMAKE_INSTALL_PREFIX="$HOME/local/helics-gcc-X.X.X/" -DBOOST_ROOT="$HOME/local/boost-gcc-1.69.0" -DBUILD_PYTHON_INTERFACE=ON -DCMAKE_C_COMPILER=/usr/local/Cellar/gcc/8.2.0/bin/gcc-8 -DCMAKE_CXX_COMPILER=/usr/local/Cellar/gcc/8.2.0/bin/g++-8 ../
$ make clean; make -j 4; make install
```

## Testing HELICS

### Basic test (without language bindings)

A quick test is to double check the versions of the HELICS player and
recorder:

```bash
cd /path/to/helics_install/bin

$ helics_player --version
x.x.x (20XX-XX-XX)

$ helics_recorder --version
x.x.x (20XX-XX-XX)
```

### Testing HELICS with python support

If you open a interactive Python session and import helics, you should be able to get the version of `helics` and an output that is similar to the following.

```bash
$ ipython
Python 3.6.4 |Anaconda, Inc.| (default, Jan 16 2018, 12:04:33)
Type 'copyright', 'credits' or 'license' for more information
IPython 6.2.1 -- An enhanced Interactive Python. Type '?' for help.

In [1]: import helics

In [2]: helics.helicsGetVersion()
Out[2]: 'x.x.x (20XX-XX-XX)'

```

### Testing HELICS with MATLAB support

To run the MATLAB HELICS extension, one would have to load the `helicsSharedLib` in the MATLAB file.
This is run by the `helicsStartup` function in the generated MATLAB files.
You can test this by opening MATLAB from the terminal or using the icon.

```bash
/Applications/MATLAB_R2017b.app/bin/matlab -nodesktop -nosplash -nojvm
```

and running

```matlab
>> helicsStartup
```

<div class="admonition note">

Note: See [Helics issue #763](https://github.com/GMLC-TDC/HELICS/issues/763/), if your installation doesn't point the dylib to the correct location.

</div>

You can run the following in two separate windows to test an example from the following repository:

```bash
git clone https://github.com/GMLC-TDC/HELICS-examples
```

Run the following in one MATLAB instance

```bash
matlab -nodesktop -nosplash
cd ~/GitRepos/GMLC-TDC/HELICS-examples/matlab
pireceiver
```

Run the following in a separate MATLAB instance.

```bash
matlab -nodesktop -nosplash
cd ~/GitRepos/GMLC-TDC/HELICS-examples/matlab
pisender
```
