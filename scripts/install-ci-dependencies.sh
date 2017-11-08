#!/bin/bash

if [[ ! -f "dependencies" ]]; then
    mkdir dependencies;
fi

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    if [[ ! -f "cmake-3.4.3-Linux-x86_64/bin/cmake" ]]; then
        echo "*** install cmake"
        wget --no-check-certificate http://cmake.org/files/v3.4/cmake-3.4.3-Linux-x86_64.tar.gz && tar -xzf cmake-3.4.3-Linux-x86_64.tar.gz;
    fi
    export PATH="${PWD}/cmake-3.4.3-Linux-x86_64/bin:${PATH}"
    echo "*** cmake installed ($PATH)"
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    if [[ ! -d "cmake-3.4.3-Darwin-x86_64/CMake.app" ]]; then
        echo "*** install cmake"
        wget --no-check-certificate http://cmake.org/files/v3.4/cmake-3.4.3-Darwin-x86_64.tar.gz && tar -xzf cmake-3.4.3-Darwin-x86_64.tar.gz;
    fi
    export PATH="${PWD}/cmake-3.4.3-Darwin-x86_64/CMake.app/Contents/bin:${PATH}"
    echo "*** cmake installed ($PATH)"
fi

if [[ ! -d "dependencies/zmq" ]]; then
    echo "*** build libzmq"
    git clone git://github.com/zeromq/libzmq.git
    (
        cd libzmq;
        ./autogen.sh;
        mkdir build && cd build;
        cmake .. -DWITH_PERF_TOOL=OFF -DZMQ_BUILD_TESTS=OFF -DENABLE_CPACK=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../../dependencies/zmq
        make;
        make install;
    )
    echo "*** built zmq successfully"
fi

if [[ ! -d "dependencies/boost" ]]; then
    echo "*** build boost"
    wget -O boost_1_61_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz/download && tar xzf boost_1_61_0.tar.gz
    (
        cd boost_1_61_0/;
        ./bootstrap.sh --with-libraries=date_time,filesystem,program_options,system,test;
        ./b2 link=shared threading=multi variant=release > /dev/null;
        ./b2 install --prefix=../dependencies/boost > /dev/null;
    )
    echo "*** built boost successfully"
fi

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    sudo ldconfig ${PWD}/dependencies
    export LD_LIBRARY_PATH=${PWD}/dependencies/zmq/lib:${PWD}/dependencies/boost/lib:$LD_LIBRARY_PATH
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${PWD}/dependencies/zmq/lib:${PWD}/dependencies/boost/lib:$LD_LIBRARY_PATH
fi
