#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. 
# SPDX-License-Identifier: BSD-3-Clause
#

# add a baseline library for underlying dependencies and flags for test executables
if (BUILD_BOOST_TESTS)
add_library(helics_boost_test_base INTERFACE)
target_link_libraries(helics_boost_test_base INTERFACE Boostlibs::test)
target_link_libraries(helics_boost_test_base INTERFACE helics_base_includes)
endif()

include(addGoogletest)
add_library(helics_google_test_base INTERFACE)
target_link_libraries(helics_google_test_base INTERFACE gtest gtest_main gmock)
target_link_libraries(helics_google_test_base INTERFACE helics_base_includes)

add_subdirectory(core)

if (BUILD_BOOST_TESTS)
add_subdirectory(common)

add_subdirectory(application_api)
add_subdirectory(system_tests)
if(BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
    add_subdirectory(shared_library)
endif()

add_subdirectory(apps)

add_subdirectory(performance_tests)

endif()