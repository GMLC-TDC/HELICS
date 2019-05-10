#!/usr/bin/env bash

localdir=$(pwd)

#change to wherever boost was downloaded
cd ~/boost_1_66_0/

#add mpi if building with mpi
./bootstrap.sh --with-toolset=gcc --prefix=$localdir/boost --with-libraries=test

# change to "-std=c++1z" to build with C++17
./b2 install -j4 --prefix=$localdir/boost --build-dir=$localdir/buildboost toolset=gcc variant=release link=static link=shared cxxflags="-std=c++14 -fPIC" threading=multi address-model=64

cd $localdir
