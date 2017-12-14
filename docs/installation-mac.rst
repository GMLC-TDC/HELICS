
Mac Installation
----------------

Requirements
============

* C++11 compiler (C++14 preferred).
* CMake 3.4 or newer
* git
* Boost 1.58 or newer
* ZeroMQ 4.1.4 or newer (if ZeroMQ support is needed)
* [for future versions] OpenMPI v8 or newer (if MPI support is needed)

Setup
=====

To set up your environment:

    1. (if needed) Install git on your system for easy access to the HELICS source. Download from `git-scm <https://git-scm.com/downloads>`_. This installs the command line which is described here. GUI's interfaces such as `SourceTree <https://www.sourcetreeapp.com/>`_ are another option.

    2. (if desired) Many required libraries are easiest installed using the `homebrew <https://brew.sh/>`_ package manager. These directions assume this approach, so unless you prefer to track these libraries and dependencies down yourself, install it if you don't have it yet.

    3. (if needed) Setup a command-line compile environment

         a) Install a C++11 compiler (C++14 preferred). e.g. `clang` from the Xcode command line tools. These can be installed from the command line in Terminal by typing `xcode-select --install` and following the on-screen prompts.
         b) Install cmake with `brew install cmake`. Alternately, a DMG file is available for cmake from their `website <https://cmake.org/download/>`_.

    4. Install most dependencies using homebrew.

        .. code-block:: bash

            brew install boost
            brew install zmq

    5. Make sure *cmake* and *git* are available in the Command Prompt with ``which cmake`` and ``which git`` If they aren't, add them to the system PATH variable.

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
