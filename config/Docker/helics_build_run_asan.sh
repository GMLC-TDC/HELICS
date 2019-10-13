#!/usr/bin/bash
git clone https://github.com/GMLC-TDC/HELICS.git
cd HELICS
mkdir build-asan
cd build-asan
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_FLAGS="${ASAN_CFLAGS}" -DHELICS_BUILD_TESTS=ON -DHELICS_ZMQ_SUBPROJECT=ON  -DHELICS_ZMQ_FORCE_SUBPROJECT=ON
cd /root/develop/HELICS/build-asan
make -j2

./sanitizer_tests.sh
