Mac Installation
================

Install from source
-------------------

Requirements
------------

- C++11 compiler (C++14 preferred).
- CMake 3.4 or newer
- git
- Boost 1.58 or newer
- ZeroMQ 4.1.4 or newer (if ZeroMQ support is needed)
- MPI-2 implementation (if MPI support is
  needed)

Setup
-----

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
   have it yet.
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

5. Make sure *cmake* and *git* are available in the Command Prompt
   with `which cmake` and `which git` If they aren't, add them to the
   system PATH variable.

Getting and building from source:

1. Use `git clone` to to check out a copy of HELICS.
2. Create a build folder. Run cmake and give it the path that HELICS
   was checked out into.
3. Run `make`.

```bash
git clone https://github.com/GMLC-TDC/HELICS-src
cd HELICS-src
mkdir build
cd build
cmake ../
ccmake . # optional, to change install path or other configuration settings
make
make install
```

Testing
-------

A quick test is to double check the versions of the HELICS player and
recorder:

```bash
cd /path/to/helics_install/bin

$ helics_player --version
x.x.x (XX-XX-XX)

$ helics_recorder --version
x.x.x (XX-XX-XX)
```

Building HELICS with python support
-----------------------------------


Run the following:

```bash
$ cmake -DBUILD_PYTHON_INTERFACE=ON -DCMAKE_INSTALL_PREFIX=/Users/$(whoami)/local/helics-2.0.0/ ..
$ make -j8
$ make install
```

Add the following to your `~/.bashrc` file.

```bash
export PYTHONPATH=/Users/$(whoami)/local/helics-X.X.X/python:$PYTHONPATH
```

If you open a interactive Python session and import helics, you should be able to get the version of `helics` and an output that is similar to the following.

```bash
$ ipython
Python 3.6.4 |Anaconda, Inc.| (default, Jan 16 2018, 12:04:33)
Type 'copyright', 'credits' or 'license' for more information
IPython 6.2.1 -- An enhanced Interactive Python. Type '?' for help.

In [1]: import helics

In [2]: helics.helicsGetVersion()
Out[2]: 'x.x.x (XX-XX-XX)'

```

Building HELICS using gcc and python
------------------------------------

First you will need to build boost using gcc from source. Download
[boost](http://www.boost.org/users/history/version_1_64_0.html) from the
boost.org website.

Unzip the folder boost\_1\_64\_0 to any location, for example Downloads.

```bash
$ cd ~/Downloads/boost_1_64_0
$ ./bootstrap.sh --with-python=/Users/$USER/miniconda3/python3 --prefix=/usr/local/Cellar/gcc/7.2.0_1/bin/gcc-7
$ ./bootstrap.sh --prefix=/ --prefix=/Users/$USER/local/boost-gcc-1.64
$ ./b2
$ ./b2 install
$ # OR
$ ./bjam cxxflags='-fPIC' cflags='-fPIC' -a link=static install # For static linking
```

This will install boost in the \~/local/boost-gcc-1.64 folder

Next, you will need to build HELICS and tell it what the BOOST\_ROOT is.

```bash
$ cmake -DCMAKE_INSTALL_PREFIX="/Users/$USER/local/helics-gcc-1.0.0/" -DBOOST_ROOT="/Users/$USER/local/boost-gcc-1.64" -DBUILD_PYTHON_INTERFACE=ON -DCMAKE_C_COMPILER=/usr/local/Cellar/gcc/7.2.0_1/bin/gcc-7 -DCMAKE_CXX_COMPILER=/usr/local/Cellar/gcc/7.2.0_1/bin/g++-7 ../
$ make clean; make -j 4; make install
```


Building HELICS with MATLAB support
-----------------------------------

To install HELICS with MATLAB support, you will need to add `BUILD_MATLAB_INTERFACE=ON`.

The important thing to note is that the MATLAB binaries are in the PATH.
Specifically, `mex` must be available in the PATH.

<div class="admonition note">

Note: To check if `mex` is in the PATH, type `which mex` and see if it returns a PATH to the `mex` compiler

</div>

```
git clone https://github.com/GMLC-TDC/HELICS-src
cd HELICS-src
mkdir build-osx
cd build-osx
cmake -DBUILD_MATLAB_INTERFACE=ON -DCMAKE_INSTALL_PREFIX=/Users/$(whoami)/local/helics-develop/ ..
make -j8
make install
```

If you have changed the C-interface, and want to regenerate the SWIG MATLAB bindings, you will need to use a custom version of SWIG to build the MATLAB interface.
To do that, you can follow the following instructions.

- Install [SWIG with MATLAB](https://github.com/jaeandersson/swig/)
- `./configure --prefix=/Users/$USER/local/swig_install; make; make install;`
- Ensure that SWIG and MATLAB are in the PATH

The below generates the MATLAB interface using SWIG.

```bash
cd ~/GitRepos/GMLC-TDC/HELICS-src/swig/
mkdir matlab
swig -I../src/helics/shared_api_library -outdir ./matlab -matlab ./helics.i
mv helics_wrap.cxx matlab/helicsMEX.cxx
```

You can copy these files into the respective `HELICS-src/swig/matlab/` folder and run the cmake command above.
Alternatively, you wish to build the MATLAB interface without using CMake, and you can do the following.

```bash
cd ~/GitRepos/GMLC-TDC/HELICS-src/swig/
mex -I../src/helics/shared_api_library ./matlab/helics_wrap.cxx -lhelicsSharedLib -L/path/to/helics_install/lib/helics/
mv helicsMEX.* matlab/
```

## Test HELICS MATLAB extension

To run the MATLAB HELICS extension, one would have to load the `helicsSharedLib` in the MATLAB file.
You can modify the first line of the files listed below to point them to your current `helicsSharedLib` install.

Run the following in two separate windows.

```bash
matlab -nodesktop -nosplash
cd ~/GitRepos/GMLC-TDC/HELICS-src/swig/matlab
pireceiver
```

```bash
matlab -nodesktop -nosplash
cd ~/GitRepos/GMLC-TDC/HELICS-src/swig/matlab
pisender
```
