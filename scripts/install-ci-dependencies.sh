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

# Convert commit message to lower case
commit_msg=`tr '[:upper:]' '[:lower:]' <<< ${TRAVIS_COMMIT_MESSAGE}`
# Wipe out cached dependencies if commit message has '[update_cache]'
if [[ $commit_msg == *'[update_cache]'* ]]; then
    local individual
    if [[ $commit_msg == *'boost'* ]]; then
        rm -rf dependencies/boost;
        individual="true"
    fi
    if [[ $commit_msg == *'zmq'* ]]; then
        rm -rf dependencies/zmq;
        individual="true"
    fi

    # If no dependency named in commit message, update entire cache
    if [[ "$individual" != 'true' ]]; then
        rm -rf dependencies;
    fi
fi

if [[ ! -f "dependencies" ]]; then
    mkdir -p dependencies;
fi

# Compares two semantic version numbers (major.minor.revision)
check_minimum_version () {
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    local -a ver_min
    IFS='. ' read -r -a ver_min <<< $1

    if [[ ver[0] -lt ver_min[0] ]] || [[ ver[0] -eq ver_min[0] && ver[1] -lt ver_min[1] ]] || [[ ver[0] -eq ver_min[0] && ver[1] -eq ver_min[1] && ver[2] -lt ver_min[2] ]]; then
        return 1
    else
        return 0
    fi
}

install_boost () {
    # Split argument 1 into 'ver' array, using '.' as delimiter
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    # Download and install Boost
    local boost_version=$1
    local boost_version_str=boost_${ver[0]}_${ver[1]}_${ver[2]}
    wget --no-check-certificate -O ${boost_version_str}.tar.gz http://sourceforge.net/projects/boost/files/boost/${boost_version}/${boost_version_str}.tar.gz/download && tar xzf ${boost_version_str}.tar.gz
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

# Install Boost
if [[ ! -d "dependencies/boost" ]]; then
    echo "*** build boost"
    if [[ -z "$CI_BOOST_VERSION" ]]; then
        install_boost 1.65.0
    else
        install_boost $CI_BOOST_VERSION
        if ! check_minimum_version $CI_BOOST_VERSION 1.61.0 then
            export CI_NO_TESTS=true
        fi
    fi
    echo "*** built boost successfully"
fi

export ZMQ_INCLUDE=${TRAVIS_BUILD_DIR}/dependencies/zmq/include
export ZMQ_LIB=${TRAVIS_BUILD_DIR}/dependencies/zmq/lib

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    export LD_LIBRARY_PATH=${PWD}/dependencies/zmq/lib:${PWD}/dependencies/boost/lib:$LD_LIBRARY_PATH
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${PWD}/dependencies/zmq/lib:${PWD}/dependencies/boost/lib:$LD_LIBRARY_PATH
fi

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    export LD_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$LD_LIBRARY_PATH
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$DYLD_FALLBACK_LIBRARY_PATH
fi

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    # HOMEBREW_NO_AUTO_UPDATE=1 brew install boost
    brew update
    brew install python3
    pip3 install pytest
else
    pyenv global 3.6.3
    python3 -m pip install --user --upgrade pip wheel
    python3 -m pip install --user --upgrade pytest
fi


