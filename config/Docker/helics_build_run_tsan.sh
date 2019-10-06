#!/usr/bin/bash
git clone --recurse-submodules --branch develop https://github.com/GMLC-TDC/HELICS.git
cd HELICS
mkdir build-tsan
cd build-tsan
cmake .. -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_FLAGS="${TSAN_CFLAGS}"  -DBUILD_HELICS_BOOST_TESTS=OFF -DZMQ_SUBPROJECT=ON  -DZMQ_FORCE_SUBPROJECT=ON
cd /root/develop/HELICS/build-tsan
make -j2

./sanitizer_tests.sh
