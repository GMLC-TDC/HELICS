#!/bin/bash
# This uses bash variable substitution in a few places
# 1. replacing x86 with Win32 (setting the Python version uses x86)
# 2. getting the cmake directory for running cpack with an absolute path (chocolatey has an unfortunately named alias)

echo "Building ${CPACK_GEN} installer with ${BUILD_GEN} for ${BUILD_ARCH}"
choco install -y swig
mkdir build && cd build || exit
cmake -G "${BUILD_GEN}" -A "${BUILD_ARCH/x86/Win32}" -DCMAKE_BUILD_TYPE=Release -DHELICS_ENABLE_PACKAGE_BUILD=ON -DBUILD_PYTHON_INTERFACE=ON -DBUILD_JAVA_INTERFACE=ON -DSTATIC_STANDARD_LIB=static -DHELICS_USE_ZMQ_STATIC_LIBRARY=ON -DHELICS_BUILD_EXAMPLES=OFF -DHELICS_BUILD_APP_EXECUTABLES=ON -DHELICS_BUILD_APP_LIBRARY=ON -DHELICS_ENABLE_WEBSERVER=ON -DBUILD_TESTING=OFF ..
cmake --build . --config Release
cpack_dir="$(command -v cmake)"
cpack_dir="${cpack_dir%/cmake}"
"${cpack_dir}/cpack" -G "${CPACK_GEN}" -C Release -B "$(pwd)/../artifact"
cd ../artifact || exit
rm -rf _CPack_Packages
