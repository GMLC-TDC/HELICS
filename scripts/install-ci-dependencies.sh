#!/bin/bash

if [[ ! -f "dependencies" ]]; then
    mkdir -p dependencies;
fi

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    if [[ ! -f "cmake-3.9.0-Linux-x86_64/bin/cmake" ]]; then
        echo "*** install cmake"
        wget --no-check-certificate http://cmake.org/files/v3.9/cmake-3.9.0-Linux-x86_64.tar.gz && tar -xzf cmake-3.9.0-Linux-x86_64.tar.gz;
    fi

    export PATH="${PWD}/cmake-3.9.0-Linux-x86_64/bin:${PATH}"
    echo "*** cmake installed ($PATH)"
    (
        cd /tmp/;
        curl -s -J -O -k -L 'https://sourceforge.net/projects/swig/files/swig/swig-3.0.10/swig-3.0.10.tar.gz/download';
        tar zxf swig-3.0.10.tar.gz;
        cd swig-3.0.10;
        ./configure --prefix $HOME/swig/;
        make;
        make install;
    )
    export PATH="$HOME/swig/bin:${PATH}"
    echo "*** built swig successfully {$PATH}"

elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    if [[ ! -f "cmake-3.4.3-Darwin-x86_64/CMake.app/Contents/bin/cmake" ]]; then
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
