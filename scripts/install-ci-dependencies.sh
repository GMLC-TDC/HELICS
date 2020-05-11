#!/bin/bash

# Set variables based on build environment
if [[ "$TRAVIS" == "true" ]]; then
    if [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
        export PATH="/usr/local/opt/ccache/libexec:$PATH"
    fi

    export CI_DEPENDENCY_DIR=${TRAVIS_BUILD_DIR}/dependencies

    WAIT_COMMAND=travis_wait

    # Convert commit message to lower case
    commit_msg=$(tr '[:upper:]' '[:lower:]' <<<"${TRAVIS_COMMIT_MESSAGE}")

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
    boost_version=1.72.0
fi
boost_install_path=${CI_DEPENDENCY_DIR}/boost

cmake_version=${USE_CMAKE_VERSION}
if [[ -z "$USE_CMAKE_VERSION" ]]; then
    cmake_version=3.4.3
fi
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
        rm -rf "${boost_install_path}"
        individual="true"
    fi
    if [[ $commit_msg == *'zmq'* ]]; then
        rm -rf "${zmq_install_path}"
        individual="true"
    fi
    if [[ $commit_msg == *'swig'* ]]; then
        rm -rf "${swig_install_path}"
        individual="true"
    fi
    if [[ "$USE_MPI" ]]; then
        if [[ $commit_msg == *'mpi'* ]]; then
            rm -rf "${mpi_install_path}"
            individual="true"
        fi
    fi

    # If no dependency named in commit message, update entire cache
    if [[ "$individual" != 'true' ]]; then
        rm -rf "${CI_DEPENDENCY_DIR}"
    fi
fi

if [[ ! -d "${CI_DEPENDENCY_DIR}" ]]; then
    mkdir -p "${CI_DEPENDENCY_DIR}"
fi

# Only compile these dependencies on Linux, to install them on macOS use the Brewfile .ci/Brewfile.travis
if [[ "$os_name" == "Linux" ]]; then
    # Install CMake
    if [[ ! -d "${cmake_install_path}" ]]; then
        ./scripts/install-dependency.sh cmake ${cmake_version} "${cmake_install_path}"
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
        ./scripts/install-dependency.sh swig ${swig_version} "${swig_install_path}"
    fi
    export PATH="${swig_install_path}/bin:${PATH}"
    echo "*** built swig successfully {$PATH}"

    # Install ZeroMQ
    if [[ "$TRAVIS" != "true" && ! -d "${zmq_install_path}" ]]; then
        echo "*** build libzmq"
        ./scripts/install-dependency.sh zmq ${zmq_version} "${zmq_install_path}"
        echo "*** built zmq successfully"
    fi

    # Install Boost
    if [[ ! -d "${boost_install_path}" ]]; then
        echo "*** build boost"
        ${WAIT_COMMAND} ./scripts/install-dependency.sh boost ${boost_version} "${boost_install_path}"
        echo "*** built boost successfully"
    fi

    # Export ZMQ variables and set load library paths
    export ZMQ_INCLUDE=${zmq_install_path}/include
    export ZMQ_LIB=${zmq_install_path}/lib

    if [[ "$os_name" == "Linux" ]]; then
        export LD_LIBRARY_PATH=${zmq_install_path}/lib:${boost_install_path}/lib:$LD_LIBRARY_PATH
    elif [[ "$os_name" == "Darwin" ]]; then
        export DYLD_LIBRARY_PATH=${zmq_install_path}/lib:${boost_install_path}/lib:$DYLD_LIBRARY_PATH
    fi
# End of the Linux only dependency compiling
fi

# Install MPI if USE_MPI is set
if [[ "$USE_MPI" ]]; then
    if [[ ! -d "${mpi_install_path}" ]]; then
        # if mpi_implementation isn't set, then the mpi implementation requested wasn't recognized
        if [[ -n "${mpi_implementation}" ]]; then
            ${WAIT_COMMAND} ./scripts/install-dependency.sh ${mpi_implementation} ${mpi_version} "${mpi_install_path}"
        fi
    fi
fi

# Export HELICS shared library variables and set load library paths
if [[ "$os_name" == "Linux" ]]; then
    export LD_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$LD_LIBRARY_PATH
elif [[ "$os_name" == "Darwin" ]]; then
    export DYLD_LIBRARY_PATH=${PWD}/build/src/helics/shared_api_library/:$DYLD_LIBRARY_PATH
fi

if [[ "$os_name" == "Darwin" && -x "$(command -v brew)" ]]; then
    wget https://repo.continuum.io/miniconda/Miniconda3-latest-MacOSX-x86_64.sh -O miniconda.sh
    bash miniconda.sh -b -p "$HOME/miniconda"
    export PATH="$HOME/miniconda/bin:$PATH"
    conda config --set always_yes yes --set changeps1 no
    conda update -q conda
    conda info -a
    pip install --user --upgrade pytest
else
    if hash pyenv; then
        if [[ ${DEBUG_INSTALL_DEPENDENCY+x} ]]; then
            pyenv versions
        fi

        # Default path listing order (pyenv versions is a bash script) should place latest version at the end (unless jython/miniconda/etc are installed)
        last_pyversion=$(pyenv versions | tail -1)
        # Remove a leading asterisk if present (though setting the version is redundant, since that is the one that is already active)
        last_pyversion=${last_pyversion/#\*/}
        # Remove a trailing set of parenthesis saying where the version was set
        last_pyversion=${last_pyversion%(*)}
        # Remove whitespace
        last_pyversion="$(echo -e "$last_pyversion" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')"
        pyenv global "${last_pyversion}"
    fi

    python3 -m pip install --user --upgrade pip wheel
    python3 -m pip install --user --upgrade pytest
fi

pyver=$(python3 -c 'import sys; ver=sys.version_info[:2]; print(".".join(map(str,ver)))')

PYTHON_LIB_PATH=$(python3-config --prefix)/lib/libpython${pyver}m.${shared_lib_ext}
export PYTHON_LIB_PATH
PYTHON_INCLUDE_PATH=$(python3-config --prefix)/include/python${pyver}m/
export PYTHON_INCLUDE_PATH
PYTHON_EXECUTABLE=$(command -v python3)
export PYTHON_EXECUTABLE

# Tell macOS users to use Homebrew to install additional dependencies
if [[ "$TRAVIS" != "true" && "$os_name" == Darwin ]]; then
    echo "To install additional dependencies on macOS, please use Homebrew to install the packages in .ci/Brewfile.travis"
fi
