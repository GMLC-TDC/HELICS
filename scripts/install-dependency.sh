#!/bin/bash

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

install_swig () {
    #Download and install SWIG
    local swig_version=$1
    local swig_version_str=swig-${swig_version}
    local install_path=$2
    curl -s -J -k -L -O https://sourceforge.net/projects/swig/files/swig/${swig_version_str}/${swig_version_str}.tar.gz/download && tar -zxf ${swig_version_str}.tar.gz
    (
        cd ${swig_version_str};
        ./configure --prefix ${install_path};
        make;
        make install;
    )
    rm ${swig_version_str}.tar.gz
}

install_zmq () {
    # Clone the zeromq repo and build it
    local install_path=$1
    git clone git://github.com/zeromq/libzmq.git;
    (
        cd libzmq;
        ./autogen.sh;
        mkdir -p build && cd build;
        cmake .. -DWITH_PERF_TOOL=OFF -DZMQ_BUILD_TESTS=OFF -DENABLE_CPACK=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="${install_path}"
        make;
        make install;
    )
}

install_mpich () {
    # MPICH version number splitting
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    # Download and install MPICH (only works with v3+, version number scheme is different for v2)
    local mpich_version=$1
    local mpich_version_str=mpich-${mpich_version}
    local install_path=$2
    wget --no-check-certificate -O ${mpich_version_str}.tar.gz http://www.mpich.org/static/downloads/${mpich_version}/${mpich_version_str}.tar.gz;
    tar xzf ${mpich_version_str}.tar.gz ;
    (
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
    )
    rm ${mpich_version_str}.tar.gz
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
    wget --no-check-certificate -O ${openmpi_version_str}.tar.gz https://www.open-mpi.org/software/ompi/${openmpi_short_ver}/downloads/${openmpi_version_str}.tar.gz;
    tar xzf ${openmpi_version_str}.tar.gz ;
    (
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
    )
    rm ${openmpi_version_str}.tar.gz
}

install_boost () {
    # Split argument 1 into 'ver' array, using '.' as delimiter
    local -a ver
    IFS='. ' read -r -a ver <<< $1

    # Download and install Boost
    local boost_version=$1
    local boost_version_str=boost_${ver[0]}_${ver[1]}_${ver[2]}
    local install_path=$2
    wget --no-check-certificate -O ${boost_version_str}.tar.gz http://sourceforge.net/projects/boost/files/boost/${boost_version}/${boost_version_str}.tar.gz/download && tar xzf ${boost_version_str}.tar.gz
    (
        cd ${boost_version_str}/;
        ./bootstrap.sh --with-libraries=date_time,filesystem,program_options,system,chrono,timer,test;
        ./b2 link=shared threading=multi variant=release > /dev/null;
        ./b2 install --prefix=${install_path} > /dev/null;
    )
    rm ${boost_version_str}.tar.gz
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
    wget --no-check-certificate -O ${cmake_version_str}.tar.gz http://cmake.org/files/v${ver[0]}.${ver[1]}/${cmake_version_str}.tar.gz;
    tar -xzf ${cmake_version_str}.tar.gz ;

    # Move cmake to "install" location
    mv ${cmake_version_str} ${install_path};
    rm ${cmake_version_str}.tar.gz
}


install_version=$2
install_path=$3
case "$1" in
    boost)
        install_boost ${install_version} ${install_path}
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
        install_path=$2
        install_zmq ${install_path}
        ;;
    *)
        echo "Usage:"
        echo "$0 (boost|cmake|mpich|openmpi|swig) version install_path"
        echo "$0 zmq install_path"
esac

