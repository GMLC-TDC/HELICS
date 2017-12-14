Ubuntu Installation
-------------------

Requirements
============

* Ubuntu 16 or newer
* C++11 compiler (C++14 preferred)
* CMake 3.4 or newer
* git
* Boost 1.58 or newer
* ZeroMQ 4.1.4 or newer (if ZeroMQ support is needed)
* [for future versions] OpenMPI v8 or newer (if MPI support is needed)

Setup
=====

To set up your environment:

   1. Install dependencies using apt-get.
      1. sudo apt-get install libboost-dev
      2. sudo apt-get install libboost-program-options-dev
      3. sudo apt-get install libboost-test-dev
      4. sudo apt-get install libboost-filesystem-dev
      5. sudo apt-get install libboost-date-time-dev
      6. sudo apt-get install libzmq5-dev
   2. Make sure *cmake* and *git* are available in the Command Prompt. If they aren't, add them to the system PATH variable.

Getting and building from source:

1. Use `git clone` to to check out a copy of HELICS.
2. Create a build folder. Run cmake and give it the path that HELICS was checked out into.
3. Run "make".

.. code-block:: bash

    git clone https://github.com/GMLC-TDC/HELICS-src
    cd HELICS-src
    mkdir build
    cd build
    cmake ../
    ccmake . # optional, to change install path or other configuration settings
    make
    make install


Testing
=======

A quick test is to double check the versions of the HELICS player and recorder:

.. code-block:: bash

    cd /path/to/helics_install/bin

    $ helics_player --version
    0.1

    $ helics_recorder --version
    0.1
