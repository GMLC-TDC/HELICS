#!/usr/bin/env bash

localdir=$(pwd)
#change to wherever boost was downloaded
cd ~/boost_1_66_0/


#add mpi if building with mpi
./bootstrap.sh --with-toolset=clang --prefix=$localdir/boost --with-libraries=system,test,program_options,filesystem

# change to "-std=c++1z" to build with C++17
./b2 install -j4 --prefix=$localdir/boost --build-dir=$localdir/buildboost toolset=clang variant=release link=static link=shared cxxflags="-std=c++14 -stdlib=libc++ -fPIC" linkflags="-stdlib=libc++ -fPIC" threading=multi address-model=64 

cd $localdir
