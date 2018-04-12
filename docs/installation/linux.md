Ubuntu Installation
===================

Requirements
------------

* Ubuntu 16 or newer
* C++11 compiler (C++14 preferred)
* CMake 3.4 or newer
* git
* Boost 1.58 or newer
* ZeroMQ 4.1.4 or newer (if ZeroMQ support is needed)
* [for future versions] OpenMPI v8 or newer (if MPI support is needed)

Setup
-----

To set up your environment:

1. Install dependencies using apt-get.

```bash
sudo apt-get install libboost-dev
sudo apt-get install libboost-program-options-dev
sudo apt-get install libboost-test-dev
sudo apt-get install libboost-filesystem-dev
sudo apt-get install libboost-date-time-dev
sudo apt-get install libzmq5-dev
```

2. Make sure *cmake* and *git* are available in the Command Prompt. If they aren't, add them to the system PATH variable.

Getting and building from source:

1. Use `git clone` to to check out a copy of HELICS.

2. Create a build folder. Run cmake and give it the path that HELICS was checked out into.

3. Run "make".

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

Building HELICS with python support
-----------------------------------

Run the following:

```bash
$ cmake -DBUILD_PYTHON_INTERFACE=ON -DPYTHON_INCLUDE_DIR=$(python3-config --prefix)/include/python3.6m/ -DPYTHON_LIBRARY=$(python3-config --prefix)/lib/python3.6m/libpython3.6m.so -DCMAKE_INSTALL_PREFIX=~/local/helics-1.0.0/ ..
$ make -j8
$ make install
```

Add the following to your `~/.bashrc` file.

```bash
export PYTHONPATH=~/local/helics-1.0.0/python:$PYTHONPATH
```

If you open a interactive Python session and import helics, you should be able to get the version of `helics` and an output that is similar to the following.

```bash
$ ipython
Python 3.6.4 |Anaconda, Inc.| (default, Jan 16 2018, 12:04:33)
Type 'copyright', 'credits' or 'license' for more information
IPython 6.2.1 -- An enhanced Interactive Python. Type '?' for help.

In [1]: import helics

In [2]: helics.helicsGetVersion()
Out[2]: '1.0.0-alpha.3 (02-12-18)'

```


Testing
-------

A quick test is to double check the versions of the HELICS player and recorder:

```bash
cd /path/to/helics_install/bin

$ helics_player --version
0.1

$ helics_recorder --version
0.1
```

