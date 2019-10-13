#!/usr/bin/bash
git git clone --recurse-submodules --branch develop https://github.com/GMLC-TDC/HELICS.git
cd HELICS
mkdir build-msan
cd build-msan
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_FLAGS="${MSAN_CFLAGS}"  -DHELICS_BUILD_TESTS=ON -DHELICS_ZMQ_SUBPROJECT=ON  -DHELICS_ZMQ_FORCE_SUBPROJECT=ON
cd /root/develop/HELICS/build-msan
make -j2

./sanitizer_tests.sh
