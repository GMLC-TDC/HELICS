
Mac Installation
================

Install using brew
------------------

Requirements
------------

* brew

Install brew_. It is a package manager for MacOS.

.. _brew: https://brew.sh/

Once you install brew, you can open a terminal and type the following.

.. code-block:: bash

    brew tap GMLC-TDC/helics
    brew install helics

OR

.. code-block:: bash

    brew install GMLC-TDC/helics/helics

If you want to install it with the Python extension, you can use the following.

.. code-block:: bash

    brew reinstall helics --with-python --with-python-include-dir=$(python3-config --prefix)/include/python3.6m/

You must pass ``--with-python-include-dir`` with a value. The easiest way to find out what the ``--with-python-include-dir`` argument should be is by using ``python3-config`` as shown above.

If you want to install using Python2 instead, you can use ``--with-python-include-dir=$(python-config --prefix)/include/python2.7/``.
It is important that the Python interpreter used to run ``import helics`` was built using the header files included in ``python-config --prefix``.
That is to say, you cannot build using Python3 and run using Python2.

Additionally, if required, you can add ``--HEAD`` to install from the latest ``develop`` branch.


Install from source
-------------------

Requirements
------------

* C++11 compiler (C++14 preferred).
* CMake 3.4 or newer
* git
* Boost 1.58 or newer
* ZeroMQ 4.1.4 or newer (if ZeroMQ support is needed)
* [for future versions] OpenMPI v8 or newer (if MPI support is needed)

Setup
-----

To set up your environment:

    1. (if needed) Install git on your system for easy access to the HELICS source. Download from `git-scm <https://git-scm.com/downloads>`_. This installs the command line which is described here. GUI's interfaces such as `SourceTree <https://www.sourcetreeapp.com/>`_ are another option.

    2. (if desired) Many required libraries are easiest installed using the `homebrew <https://brew.sh/>`_ package manager. These directions assume this approach, so unless you prefer to track these libraries and dependencies down yourself, install it if you don't have it yet.

    3. (if needed) Setup a command-line compile environment

         a) Install a C++11 compiler (C++14 preferred). e.g. ``clang`` from the Xcode command line tools. These can be installed from the command line in Terminal by typing ``xcode-select --install`` and following the on-screen prompts.
         b) Install cmake with ``brew install cmake``. Alternately, a DMG file is available for cmake from their `website <https://cmake.org/download/>`_.

    4. Install most dependencies using homebrew.

        .. code-block:: bash

            brew install boost
            brew install zeromq
            brew install cmake

    5. Make sure *cmake* and *git* are available in the Command Prompt with ``which cmake`` and ``which git`` If they aren't, add them to the system PATH variable.

Getting and building from source:

    1. Use ``git clone`` to to check out a copy of HELICS.

    2. Create a build folder. Run cmake and give it the path that HELICS was checked out into.

    3. Run ``make``.

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
-------

A quick test is to double check the versions of the HELICS player and recorder:

.. code-block:: bash

    cd /path/to/helics_install/bin

    $ helics_player --version
    0.1

    $ helics_recorder --version
    0.1

Building HELICS using gcc and python
------------------------------------

First you will need to build boost using gcc from source. Download boost_ from the boost.org website.

.. _boost: http://www.boost.org/users/history/version_1_64_0.html

Unzip the folder `boost_1_64_0` to any location, for example Downloads.

.. code-block:: bash

    $ cd ~/Downloads/boost_1_64_0
    $ ./bootstrap.sh --with-python=/Users/$USER/miniconda3/python3 --prefix=/usr/local/Cellar/gcc/7.2.0_1/bin/gcc-7
    $ ./b2
    $ ./b2 install --prefix=/Users/$USER/local/boost-gcc-1.64

This will install boost in the ~/local/boost-gcc-1.64 folder

Next, you will need to build HELICS and tell it what the BOOST_ROOT is.

.. code-block:: bash

    $ cmake -DCMAKE_INSTALL_PREFIX="/Users/$USER/local/helics-gcc-1.0.0a/" -DBOOST_ROOT="/Users/$USER/local/boost-gcc-1.64" -DBUILD_PYTHON=ON -DPYTHON_LIBRARY=$(python3-config --prefix)/lib/libpython3.6m.dylib -DPYTHON_INCLUDE_DIR=$(python3-config --prefix)/include/python3.6m -DCMAKE_C_COMPILER=/usr/local/Cellar/gcc/7.2.0_1/bin/gcc-7 -DCMAKE_CXX_COMPILER=/usr/local/Cellar/gcc/7.2.0_1/bin/g++-7 ../
    $ make clean; make -j 4; make install






