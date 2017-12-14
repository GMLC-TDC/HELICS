Installation
============

From source with cmake
----------------------

You can install ``helics`` from source with cmake. On Unix platforms, from the source directory:

.. code-block:: bash

    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    make install

On Windows platforms, from the source directory:

.. code-block:: bash

    mkdir build
    cd build
    cmake -G "NMake Makefiles" -DCMAKE_INSTALL_PREFIX=/path/to/prefix ..
    nmake
    nmake install