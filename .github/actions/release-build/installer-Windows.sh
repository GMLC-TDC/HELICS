#!/bin/bash
# This uses bash variable substitution for:
# - getting the cmake directory for running cpack with an absolute path (chocolatey has an unfortunately named alias)

echo "Building ${CPACK_GEN} installer with ${BUILD_GEN} for ${BUILD_ARCH}"

# Unset VCPKG_ROOT for GitHub actions environment
unset VCPKG_ROOT

# Install SWIG
choco install -y swig

# Install Boost
COMMON_SCRIPTS="$(cd "$(dirname "${BASH_SOURCE[0]}")/../common/Windows" && pwd)"
# shellcheck source=.github/actions/common/Windows/install-boost.sh
source "${COMMON_SCRIPTS}/install-boost.sh"

# Get the HELICS.ico in the correct format (not a renamed JPEG)
rm docs/logos/HELICS.ico
curl -o docs/logos/HELICS.ico https://raw.githubusercontent.com/GMLC-TDC/HELICS/v3.0.1/docs/img/HELICS.ico

# Find cpack command (chocolatey has a command with the same name)
cpack_dir="$(command -v cmake)"
cpack_dir="${cpack_dir%/cmake}"

# Build
mkdir build && cd build || exit
cmake -G "${BUILD_GEN}" -A "${BUILD_ARCH/x86/Win32}" -DCMAKE_BUILD_TYPE=Release -DHELICS_ENABLE_PACKAGE_BUILD=ON -DHELICS_BUILD_JAVA_INTERFACE=ON -DSTATIC_STANDARD_LIB=static -DHELICS_USE_ZMQ_STATIC_LIBRARY=ON -DHELICS_BUILD_EXAMPLES=OFF -DHELICS_BUILD_APP_EXECUTABLES=ON -DHELICS_BUILD_APP_LIBRARY=ON -DBUILD_TESTING=OFF ..
cmake --build . --config Release
"${cpack_dir}/cpack" -G "${CPACK_GEN}" -C Release -B "$(pwd)/../artifact"
cd ../artifact || exit
rm -rf _CPack_Packages
