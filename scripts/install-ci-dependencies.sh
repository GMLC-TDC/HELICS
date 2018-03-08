#!/bin/bash

if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    HOMEBREW_NO_AUTO_UPDATE=1 brew install pcre
fi

export CI_DEPENDENCY_DIR=${TRAVIS_BUILD_DIR}/dependencies

boost_version=$CI_BOOST_VERSION
if [[ -z "$CI_BOOST_VERSION" ]]; then
    boost_version=1.65.0
fi
boost_install_path=${CI_DEPENDENCY_DIR}/boost

cmake_version=3.4.3
cmake_install_path=${CI_DEPENDENCY_DIR}/cmake

swig_version=3.0.10
swig_install_path=${CI_DEPENDENCY_DIR}/swig

zmq_install_path=${CI_DEPENDENCY_DIR}/zmq

# Convert commit message to lower case
commit_msg=`tr '[:upper:]' '[:lower:]' <<< ${TRAVIS_COMMIT_MESSAGE}`

# Wipe out cached dependencies if commit message has '[update_cache]'
if [[ $commit_msg == *'[update_cache]'* ]]; then
    individual="false"
    if [[ $commit_msg == *'boost'* ]]; then
        rm -rf ${boost_install_path};
        individual="true"
    fi
    if [[ $commit_msg == *'zmq'* ]]; then
        rm -rf ${zmq_install_path};
        individual="true"
    fi
    if [[ $commit_msg == *'swig'* ]]; then
        rm -rf ${swig_install_path};
        individual="true"
    fi

    # If no dependency named in commit message, update entire cache
    if [[ "$individual" != 'true' ]]; then
        rm -rf ${CI_DEPENDENCY_DIR};
    fi
fi

if [[ ! -d "${CI_DEPENDENCY_DIR}" ]]; then
    mkdir -p ${CI_DEPENDENCY_DIR};
fi

# Install CMake
if [[ ! -d "${cmake_install_path}" ]]; then
    ./scripts/install-dependency.sh cmake ${cmake_version} ${cmake_install_path}
fi

# Set path to CMake executable depending on OS
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    export PATH="${cmake_install_path}/bin:${PATH}"
    echo "*** cmake installed ($PATH)"
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    export PATH="${cmake_install_path}/CMake.app/Contents/bin:${PATH}"
    echo "*** cmake installed ($PATH)"
fi

# Install SWIG
if [[ ! -d "${swig_install_path}" ]]; then
    ./scripts/install-dependency.sh swig ${swig_version} ${swig_install_path}
fi
export PATH="${swig_install_path}/bin:${PATH}"
echo "*** built swig successfully {$PATH}"

# Install ZeroMQ
if [[ ! -d "${zmq_install_path}" ]]; then
    echo "*** build libzmq"
    ./scripts/install-dependency.sh zmq ${zmq_install_path}
    echo "*** built zmq successfully"
fi

# Install Boost
if [[ ! -d "${boost_install_path}" ]]; then
    echo "*** build boost"
    travis_wait ./scripts/install-dependency.sh boost ${boost_version} ${boost_install_path}
    echo "*** built boost successfully"
fi

# Export variables and set load library paths
export ZMQ_INCLUDE=${zmq_install_path}/include
export ZMQ_LIB=${zmq_install_path}/lib

if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    export LD_LIBRARY_PATH=${zmq_install_path}/lib:${boost_install_path}/lib:$LD_LIBRARY_PATH
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${zmq_install_path}/lib:${boost_install_path}/lib:$DYLD_FALLBACK_LIBRARY_PATH
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
    brew upgrade python
    pip3 install pytest
else
    pyenv global 3.6.3
    python3 -m pip install --user --upgrade pip wheel
    python3 -m pip install --user --upgrade pytest
fi


