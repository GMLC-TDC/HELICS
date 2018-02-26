#!/bin/bash

cmake_install_path=${CI_DEPENDENCY_DIR}/cmake

# Get rid of unneeded files that just take up extra space
if [[ "$TRAVIS_OS_NAME" == "linux" ]]; then
    rm -rf ${cmake_install_path}/doc
    rm -rf ${cmake_install_path}/man
    rm ${cmake_install_path}/bin/ccmake
    rm ${cmake_install_path}/bin/cmake-gui
elif [[ "$TRAVIS_OS_NAME" == "osx" ]]; then
    rm -rf ${cmake_install_path}/CMake.app/Contents/Frameworks
    rm -rf ${cmake_install_path}/CMake.app/Contents/MacOS
    rm -rf ${cmake_install_path}/CMake.app/Contents/Resources
    rm -rf ${cmake_install_path}/CMake.app/Contents/doc
    rm -rf ${cmake_install_path}/CMake.app/Contents/man
    rm ${cmake_install_path}/CMake.app/Contents/bin/ccmake
    rm ${cmake_install_path}/CMake.app/Contents/bin/cmake-gui
fi


