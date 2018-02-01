#!/bin/bash


if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    HOMEBREW_NO_AUTO_UPDATE=1 brew install pcre
fi

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

if [[ ! -f "dependencies" ]]; then
    mkdir -p dependencies;
fi

install_boost () {
    # Split argument 1 into 'ver' array, with '.' as delimiter
    local -a ver
    IFS='.' read -r -a ver <<< $1
    local boost_version = $1
    local boost_version_str = boost_${ver[0]}_${ver[1]}_${ver[2]}
    wget -O ${boost_version_str}.tar.gz http://sourceforge.net/projects/boost/files/boost/${boost_version}/${boost_version_str}.tar.gz/download && tar xzf boost_$1_$2_$3.tar.gz
    (
        cd ${boost_version_str}/;
        ./bootstrap.sh --with-libraries=date_time,filesystem,program_options,system,test;
        ./b2 link=shared threading=multi variant=release > /dev/null;
        ./b2 install --prefix=../dependencies/boost > /dev/null;
    )
}

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    if [[ ! -f "cmake-3.4.3-Linux-x86_64/bin/cmake" ]]; then
        echo "*** install cmake"
        wget --no-check-certificate http://cmake.org/files/v3.4/cmake-3.4.3-Linux-x86_64.tar.gz && tar -xzf cmake-3.4.3-Linux-x86_64.tar.gz;
    fi

    export PATH="${PWD}/cmake-3.4.3-Linux-x86_64/bin:${PATH}"
    echo "*** cmake installed ($PATH)"

elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    if [[ ! -f "cmake-3.9.0-Darwin-x86_64/CMake.app/Contents/bin/cmake" ]]; then
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

# Install Boost
if [[ ! -d "dependencies/boost" ]]; then
    echo "*** build boost"
    if [[ "MINIMUM_DEPENDENCIES" == "true" ]]; then
        install_boost 1.61.0
    else
        install_boost 1.65.0
    fi
    echo "*** built boost successfully"
fi
export BOOST_ROOT=${TRAVIS_BUILD_DIR}/dependencies/boost}

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    sudo ldconfig ${PWD}/dependencies
    export LD_LIBRARY_PATH=${PWD}/dependencies/zmq/lib:${PWD}/dependencies/boost/lib:$LD_LIBRARY_PATH
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${PWD}/dependencies/zmq/lib:${PWD}/dependencies/boost/lib:$LD_LIBRARY_PATH
fi
