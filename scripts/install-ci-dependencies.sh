#!/bin/bash

echo "*** build libzmq"
git clone git://github.com/zeromq/libzmq.git
(
    cd libzmq;
    ./autogen.sh;
    ./configure;
    make check;
    sudo make install;
    sudo ldconfig
)
echo "*** built zmq successfully"

echo "*** build boost"
wget -O boost_1_61_0.tar.gz http://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz/download && tar xzf boost_1_61_0.tar.gz
(
    cd boost_1_61_0/;
    ./bootstrap.sh --with-libraries=date_time,filesystem,program_options,system,test;
    ./b2 link=shared threading=multi variant=release > /dev/null;
    sudo ./b2 install > /dev/null
)
echo "*** built boost successfully"

if [[ ! -f "cmake-3.4.3-Linux-x86_64/bin/cmake" ]]; then
    wget --no-check-certificate http://cmake.org/files/v3.4/cmake-3.4.3-Linux-x86_64.tar.gz && tar -xzf cmake-3.4.3-Linux-x86_64.tar.gz;
fi
