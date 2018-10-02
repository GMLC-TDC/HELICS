#!/bin/bash

if [[ ${DEBUG_INSTALL_DEPENDENCY+x} ]]; then
    set -x
fi

# Compares two semantic version numbers (major.minor.revision)
check_minimum_version () {
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    local -a ver_min
    IFS='. ' read -r -a ver_min <<< $2

    if [[ ${ver[0]} -lt ${ver_min[0]} ]] || [[ ${ver[0]} -eq ${ver_min[0]} && ${ver[1]} -lt ${ver_min[1]} ]] || [[ ${ver[0]} -eq ${ver_min[0]} && ${ver[1]} -eq ${ver_min[1]} && ${ver[2]} -lt ${ver_min[2]} ]]; then
        return 1
    else
        return 0
    fi
}

# Download and untar a file
fetch_and_untar () {
    local output_name=$1
    local url=$2
    wget --no-check-certificate -O ${output_name} ${url}
    tar -zxf ${output_name}
}

install_swig () {
    #Download and install SWIG
    local swig_version=$1
    local swig_version_str=swig-${swig_version}
    local install_path=$2
    fetch_and_untar ${swig_version_str}.tar.gz \
        https://sourceforge.net/projects/swig/files/swig/${swig_version_str}/${swig_version_str}.tar.gz/download
    cd ${swig_version_str};
    ./configure --prefix ${install_path};
    make;
    make install;
}

install_zmq () {
    # Clone the zeromq repo and build it
    local zmq_version=$1
    local install_path=$2
    if [[ "${zmq_version}" == "HEAD" ]]; then
        git clone git://github.com/zeromq/libzmq.git;
    else
        git clone --branch v${zmq_version} git://github.com/zeromq/libzmq.git;
    fi
    cd libzmq;
    ./autogen.sh;
    mkdir -p build && cd build;
    cmake .. -DENABLE_CURVE=OFF -DWITH_PERF_TOOL=OFF -DZMQ_BUILD_TESTS=OFF -DENABLE_CPACK=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${install_path}"
    make;
    make install;
}

install_mpich () {
    # MPICH version number splitting
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    # Download and install MPICH (only works with v3+, version number scheme is different for v2)
    local mpich_version=$1
    local mpich_version_str=mpich-${mpich_version}
    local install_path=$2
    fetch_and_untar ${mpich_version_str}.tar.gz \
        http://www.mpich.org/static/downloads/${mpich_version}/${mpich_version_str}.tar.gz
    cd ${mpich_version_str}/;
    ./configure --prefix=${install_path} \
        --disable-dependency-tracking \
        --enable-fast=yes \
        --enable-g=none \
        --enable-timing=none \
        --enable-shared \
        --disable-static \
        --disable-java \
        --disable-fortran \
        --enable-threads=serialized ;
    make;
    make install;
}


install_openmpi () {
    # Open MPI version number splitting
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    # Download and install Open MPI
    local openmpi_version=$1
    local openmpi_short_ver=v${ver[0]}.${ver[1]}
    local openmpi_version_str=openmpi-${openmpi_version}
    local install_path=$2
    fetch_and_untar ${openmpi_version_str}.tar.gz \
        https://www.open-mpi.org/software/ompi/${openmpi_short_ver}/downloads/${openmpi_version_str}.tar.gz
    cd ${openmpi_version_str}/;
    ./configure --prefix=${install_path} \
        --disable-dependency-tracking \
        --enable-coverage=no \
        --enable-shared=yes \
        --enable-static=no \
        --enable-java=no \
        --enable-mpi-fortran=no ;
    make;
    make install;
}

install_boost () {
    # Split argument 1 into 'ver' array, using '.' as delimiter
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    # Download and install Boost
    local boost_version=$1
    local boost_version_str=boost_${ver[0]}_${ver[1]}_${ver[2]}
    local install_path=$2
    local boost_toolset=$3

    local b2_extra_options=""
    local cxxflags_var=""
    local cxxflags_arr=()
    if [[ "${BOOST_CXXFLAGS}" ]]; then
        cxxflags_arr+=("${BOOST_CXXFLAGS}")
    fi

    local linkflags_var=""
    local linkflags_arr=()
    if [[ "${BOOST_LINKFLAGS}" ]]; then
        linkflags_arr+=("${BOOST_LINKFLAGS}")
    fi

    local b2_link_type=shared
    if [[ "${BOOST_USE_STATIC}" ]]; then
        b2_link_type=static
        cxxflags_arr+=("-fPIC")
    fi

    if [[ "${BOOST_SANITIZER}" ]]; then
        cxxflags_arr+=("-fsanitize=${BOOST_SANITIZER}")
        linkflags_arr+=("-fsanitize=${BOOST_SANITIZER}")
    fi

    if [[ "${cxxflags_arr[@]}" ]]; then
        cxxflags_var="cxxflags=${cxxflags_arr[@]}"
    fi

    if [[ "${linkflags_arr[@]}" ]]; then
        linkflags_var="linkflags=${linkflags_arr[@]}"
    fi

    echo Boost link type: $b2_link_type

    echo Boost b2 extra options ${b2_extra_options}

    fetch_and_untar ${boost_version_str}.tar.gz \
        http://sourceforge.net/projects/boost/files/boost/${boost_version}/${boost_version_str}.tar.gz/download
    cd ${boost_version_str}/;
    ./bootstrap.sh --with-libraries=date_time,filesystem,program_options,system,chrono,timer,test --with-toolset=${boost_toolset};
    ./b2 install -j2 --prefix=${install_path} \
        variant=release \
        link=${b2_link_type} \
        threading=multi \
        toolset=${boost_toolset} \
        "${cxxflags_var}" \
        "${linkflags_var}" \
        ${b2_extra_options} >/dev/null;
}

install_cmake () {
    # Split CMake version
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    # Download cmake
    # uname for Linux/Darwin
    local os_name="$(uname -s)"
    local cmake_version=$1
    local cmake_version_str=cmake-${cmake_version}-${os_name}-x86_64
    local install_path=$2
    fetch_and_untar ${cmake_version_str}.tar.gz \
        http://cmake.org/files/v${ver[0]}.${ver[1]}/${cmake_version_str}.tar.gz

    # Move cmake to "install" location
    mv ${cmake_version_str} ${install_path};
}


install_version=$2
if [[ $3 == '/'* ]]; then
    install_path=$3
else
    install_path=$(pwd)/$3
fi

compiler_toolset=$4
if [[ -z ${compiler_toolset} ]]; then
    case $COMPILER in
        gcc*)
            compiler_toolset=gcc
            ;;
        clang*)
            compiler_toolset=clang
            ;;
        intel*)
            compiler_toolset=intel
            ;;
        *)
            compiler_toolset=gcc
    esac
fi

if [[ "$CXX_STANDARD" == 17 ]]; then
    echo "Install dependency with C++17 flag requested"
    BOOST_CXX_FLAGS="-std=c++17"
elif [[ "$CXX_STANDARD" == 14 ]]; then
    echo "Install dependency with C++14 flag requested"
    BOOST_CXX_FLAGS="-std=c++14"
fi


# If FORCE_TOOLSET is set, create symlinks and add directory to path
# May be needed to force boost to build with the right compiler version
if [[ ${FORCE_TOOLSET+x} ]]; then
    case "${compiler_toolset}" in
        gcc*)
            ln -s $(which ${CC}) gcc
            ln -s $(which ${CXX}) g++
            ;;
        clang*)
            ln -s $(which ${CC}) clang
            ln -s $(which ${CXX}) clang++
            ;;
        intel*)
            ln -s $(which ${CXX}) icc
            ;;
        *)
            echo "Unrecognized compiler toolset"
    esac
    PATH=$(pwd):$PATH
fi

# Create and use a temp directory for downloading/building dependencies
# First command may fail on older versions of macOS
dependency_temp_dir=$(mktemp -d 2>/dev/null || mktemp -d -t 'deptmpdir')
pushd ${dependency_temp_dir}

case "$1" in
    boost)
        install_boost ${install_version} ${install_path} ${compiler_toolset}
        ;;
    cmake)
        install_cmake ${install_version} ${install_path}
        ;;
    mpich)
        install_mpich ${install_version} ${install_path}
        ;;
    openmpi)
        install_openmpi ${install_version} ${install_path}
        ;;
    swig)
        install_swig ${install_version} ${install_path}
        ;;
    zmq)
        if [[ -z ${install_path:+x} ]]; then
            install_version="HEAD"
            install_path=$2
        fi
        install_zmq ${install_version} ${install_path}
        ;;
    *)
        echo "Usage:"
        echo "$0 (cmake|mpich|openmpi|swig) version install_path"
        echo "$0 boost version install_path [toolset=gcc]"
        echo "$0 zmq [version=HEAD] install_path"
esac

# Return to the original directory and get rid of the temp directory
popd
rm -rf ${dependency_temp_dir}

if [[ ${DEBUG_INSTALL_DEPENDENCY+x} ]]; then
    set +x
fi
