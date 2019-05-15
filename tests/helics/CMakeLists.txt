#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# add a baseline library for underlying dependencies and flags for test executables
add_library(helics_test_base INTERFACE)
target_link_libraries(helics_test_base INTERFACE Boostlibs::test)
target_link_libraries(helics_test_base INTERFACE helics_base_includes)

add_subdirectory(common)

add_subdirectory(core)
add_subdirectory(application_api)
add_subdirectory(system_tests)
if(BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
    add_subdirectory(shared_library)
endif()

add_subdirectory(apps)

add_subdirectory(performance_tests)


# Tests for other CMake projects including and using HELICS
add_test(find-package-tests ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE}
                            --build-and-test "${CMAKE_CURRENT_SOURCE_DIR}/find_package_tests" "${CMAKE_CURRENT_BINARY_DIR}/find_package_tests"
                            --build-generator ${CMAKE_GENERATOR}
                            --build-options
                            "-DHELICS_DIR=${PROJECT_BINARY_DIR}"
                            "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}"
                            "-DBUILD_C_SHARED_LIB=${BUILD_C_SHARED_LIB}"
                            "-DBUILD_APPS=${BUILD_APPS}"
                            "-DZeroMQ_INSTALL_PATH=${ZeroMQ_INSTALL_PATH}"
                            "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
                            "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
)
set_property(TEST find-package-tests PROPERTY LABELS Continuous Daily)
