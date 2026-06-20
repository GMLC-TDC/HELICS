#!/bin/bash
# This uses bash variable substitution in a few places
# 1. getting the cmake directory for running cpack with an absolute path (chocolatey has an unfortunately named alias)

set -euo pipefail

# Unset VCPKG_ROOT for GitHub actions environment
unset VCPKG_ROOT

# Mark the workspace and known submodule paths as safe for nested git operations.
git config --global --add safe.directory "$(pwd)"
if [ -f .gitmodules ]; then
    while read -r submodule_path; do
        git config --global --add safe.directory "$(pwd)/${submodule_path}"
    done < <(git config --file .gitmodules --get-regexp path | awk '{print $2}')
fi

mkdir build && cd build || exit
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=20 -DHELICS_ZMQ_SUBPROJECT=ON -DHELICS_ENABLE_PACKAGE_BUILD=ON -DSTATIC_STANDARD_LIB=static -DHELICS_BUILD_EXAMPLES=OFF -DHELICS_BUILD_APP_EXECUTABLES=ON -DHELICS_BUILD_APP_LIBRARY=OFF -DHELICS_BUILD_BENCHMARKS=ON -DBUILD_TESTING=OFF ..
cmake --build . --config Release
cpack_dir="$(command -v cmake)"
cpack_dir="${cpack_dir%/cmake}"
"${cpack_dir}/cpack" -G "${CPACK_GEN}" -C Release -B "$(pwd)/../artifact"
cd ../artifact || exit
rm -rf _CPack_Packages
