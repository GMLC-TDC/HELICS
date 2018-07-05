#!/bin/bash

# Set variables based on build environment
if [[ "$TRAVIS" == "true" ]]; then
    if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        HOMEBREW_NO_AUTO_UPDATE=1 brew install pcre
        HOMEBREW_NO_AUTO_UPDATE=1 brew install ccache
        export PATH="/usr/local/opt/ccache/libexec:$PATH"
    fi

    export CI_DEPENDENCY_DIR=${TRAVIS_BUILD_DIR}/dependencies

    WAIT_COMMAND=travis_wait

    # Convert commit message to lower case
    commit_msg=$(tr '[:upper:]' '[:lower:]' <<< ${TRAVIS_COMMIT_MESSAGE})

    if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
        os_name="Linux"
    elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        os_name="Darwin"
    fi
else
    export CI_DEPENDENCY_DIR=$1
    commit_msg=""
    os_name="$(uname -s)"
fi

shared_lib_ext=so
if [[ "$os_name" == "Darwin" ]]; then
    shared_lib_ext=dylib
fi

boost_version=$CI_BOOST_VERSION
if [[ -z "$CI_BOOST_VERSION" ]]; then
    boost_version=1.65.0
fi
boost_install_path=${CI_DEPENDENCY_DIR}/boost

cmake_version=3.4.3
cmake_install_path=${CI_DEPENDENCY_DIR}/cmake

if [[ "$USE_MPI" ]]; then
    mpi_install_path=${CI_DEPENDENCY_DIR}/mpi
    case $USE_MPI in
        mpich*)
            mpi_implementation=mpich
            mpi_version=3.2
            ;;
        openmpi*)
            mpi_implementation=openmpi
            mpi_version=3.0.0
            ;;
        *)
            echo "USE_MPI must be either mpich or openmpi to build mpi as a dependency"
            ;;
    esac
fi

swig_version=3.0.10
swig_install_path=${CI_DEPENDENCY_DIR}/swig

zmq_version=4.2.3
zmq_install_path=${CI_DEPENDENCY_DIR}/zmq

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
    if [[ "$USE_MPI" ]]; then
        if [[$commit_msg == *'mpi'* ]]; then
            rm -rf ${mpi_install_path};
            individual="true"
        fi
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
if [[ "$os_name" == "Linux" ]]; then
    export PATH="${cmake_install_path}/bin:${PATH}"
    echo "*** cmake installed ($PATH)"
elif [[ "$os_name" == "Darwin" ]]; then
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
    ./scripts/install-dependency.sh zmq ${zmq_version} ${zmq_install_path}
    echo "*** built zmq successfully"
fi

# Install MPI if USE_MPI is set
if [[ "$USE_MPI" ]]; then
    if [[ ! -d "${mpi_install_path}" ]]; then
        # if mpi_implementation isn't set, then the mpi implementation requested wasn't recognized
        if [[ ! -z "${mpi_implementation}" ]]; then
            ${WAIT_COMMAND} ./scripts/install-dependency.sh ${mpi_implementation} ${mpi_version} ${mpi_install_path}
        fi
    fi
fi

# Install Boost
if [[ ! -d "${boost_install_path}" ]]; then
    echo "*** build boost"
    ${WAIT_COMMAND} ./scripts/install-dependency.sh boost ${boost_version} ${boost_install_path}
    echo "*** built boost successfully"
fi

# Export variables and set load library paths
export ZMQ_INCLUDE=${zmq_install_path}/include
export ZMQ_LIB=${zmq_install_path}/lib

if [[ "$os_name" == "Linux" ]]; then
    export LD_LIBRARY_PATH=${zmq_install_path}/lib:${boost_install_path}/lib:$LD_LIBRARY_PATH
elif [[ "$os_name" == "Darwin" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${zmq_install_path}/lib:${boost_install_path}/lib:$DYLD_FALLBACK_LIBRARY_PATH
fi

if [[ "$os_name" == "Linux" ]]; then
    export LD_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$LD_LIBRARY_PATH
elif [[ "$os_name" == "Darwin" ]]; then
    export DYLD_FALLBACK_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$DYLD_FALLBACK_LIBRARY_PATH
fi

if [[ "$os_name" == "Darwin" ]]; then
    brew update
    brew install python3
    echo "brew upgrade python"
    brew upgrade python
    pip3 install pytest
fi

pyenv
echo $(command -v pyenv)
if [[ -x "$(command -v pyenv)" ]]; then
    pyenv versions

    # Default path listing order (pyenv versions is a bash script) should place latest version at the end (unless jython/miniconda/etc are installed)
    last_pyversion=$(pyenv versions | tail -1)

    # This bash line strips the bit at the end saying where the version was set
    # The result is then piped to sed to remove any leading asterisk (though setting the version is redundant in that case)
    pyenv global $(echo ${last_pyversion%(*)} | sed 's/\*//')
fi

pyver=$(python3 -c 'import sys; ver=sys.version_info[:3]; print(".".join(map(str,ver)))')
pyver_short=$(python3 -c 'import sys; ver=sys.version_info[:2]; print(".".join(map(str,ver)))')

python3 -m pip install --user --upgrade pip wheel
python3 -m pip install --user --upgrade pytest

export PYTHON_LIB_PATH=$(python3-config --prefix)/lib/libpython${pyver_short}m.${shared_lib_ext}
export PYTHON_INCLUDE_PATH=$(python3-config --prefix)/include/python${pyver_short}m/
