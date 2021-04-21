# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(helics_apps_test_sources
    helics_apps_tests.cpp
    multi_player_tests.cpp
    multi_tests.cpp
    PlayerTests.cpp
    RecorderTests.cpp
    exeTestHelper.cpp
    combo_tests.cpp
    EchoTests.cpp
    TracerTests.cpp
    SourceTests.cpp
    CloneTests.cpp
    BrokerServerTests.cpp
    CoreAppTests.cpp
    BrokerAppTests.cpp
    MultiBrokerTests.cpp
    exeTestHelper.h
)

add_executable(helics_apps-tests ${helics_apps_test_sources})

set_target_properties(helics_apps-tests PROPERTIES FOLDER tests)

target_link_libraries(helics_apps-tests helics_apps helics_test_base)

target_include_directories(helics_apps-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_compile_definitions(
    helics_apps-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test_files/\""
)
target_compile_definitions(
    helics_apps-tests PRIVATE "-DGLOBAL_TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
)
target_compile_definitions(
    helics_apps-tests PRIVATE "-DHELICS_BUILD_LOC=\"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\""
)
target_compile_definitions(
    helics_apps-tests
    PRIVATE "-DHELICS_INSTALL_LOC=\"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\""
)

if(HELICS_DISABLE_SYSTEM_CALL_TESTS)
    target_compile_definitions(helics_apps-tests PRIVATE -DDISABLE_SYSTEM_CALL_TESTS=1)
endif()

add_test(NAME helics_apps-tests COMMAND helics_apps-tests)
set_property(TEST helics_apps-tests PROPERTY LABELS HelicsApps Coverage Daily)

# Tests for Continuous Integration builds
add_test(NAME helics_apps-ci-tests COMMAND helics_apps-tests --gtest_filter=-*_exe*:*ci_skip*)
set_property(TEST helics_apps-ci-tests PROPERTY LABELS HelicsAppsCI Continuous)

if(HELICS_BUILD_CXX_SHARED_LIB)
    set(helics_shared_apps_test_sources
        helics_apps_tests.cpp
        PlayerTests.cpp
        RecorderTests.cpp
        EchoTests.cpp
        TracerTests.cpp
        SourceTests.cpp
        CloneTests.cpp
    )

    add_executable(shared_cxx_app-tests ${helics_shared_apps_test_sources})

    target_link_libraries(
        shared_cxx_app-tests helics_test_base helics_base HELICS::helics-apps-shared
    )

    set_target_properties(shared_cxx_app-tests PROPERTIES FOLDER tests)

    add_test(NAME shared_cxx_app-tests COMMAND shared_cxx_app-tests)
    set_property(TEST shared_cxx_app-tests PROPERTY LABELS ApplicationApi Coverage Daily Continuous)

    target_compile_definitions(
        shared_cxx_app-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test_files/\""
    )

    target_compile_definitions(shared_cxx_app-tests PRIVATE DISABLE_SYSTEM_CALL_TESTS)

endif()
