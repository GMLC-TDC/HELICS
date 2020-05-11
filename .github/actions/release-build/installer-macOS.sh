#!/bin/bash
# This uses bash variable substitution in a few places
# 1. getting the cmake directory for running cpack with an absolute path (chocolatey has an unfortunately named alias)

brew install swig boost
mkdir build && cd build || exit
cmake -DCMAKE_BUILD_TYPE=Release -DHELICS_ZMQ_SUBPROJECT=ON -DHELICS_ENABLE_PACKAGE_BUILD=ON -DBUILD_PYTHON_INTERFACE=ON -DBUILD_JAVA_INTERFACE=ON -DHELICS_BUILD_EXAMPLES=OFF -DHELICS_BUILD_APP_EXECUTABLES=ON -DHELICS_BUILD_APP_LIBRARY=ON -DBUILD_TESTING=OFF ..
cmake --build . --config Release
cpack_dir="$(command -v cmake)"
cpack_dir="${cpack_dir%/cmake}"
"${cpack_dir}/cpack" -G "${CPACK_GEN}" -C Release -B "$(pwd)/../artifact"
cd ../artifact || exit
rm -rf _CPack_Packages
