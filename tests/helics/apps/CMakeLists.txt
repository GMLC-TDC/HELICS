#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#

set(
  helics_apps_test_sources
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
  exeTestHelper.h
)

add_executable(helics_apps-tests ${helics_apps_test_sources})

set_target_properties(helics_apps-tests PROPERTIES FOLDER tests)

target_link_libraries(helics_apps-tests helics_apps helics_test_base)

target_include_directories(helics_apps-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_compile_definitions(
  helics_apps-tests PRIVATE -DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"
)
target_compile_definitions(
  helics_apps-tests PRIVATE -DHELICS_BUILD_LOC=\"${CMAKE_BINARY_DIR}/src/helics/\"
)
target_compile_definitions(
  helics_apps-tests PRIVATE
  -DHELICS_INSTALL_LOC=\"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
)

if(DISABLE_SYSTEM_CALL_TESTS)
  target_compile_definitions(helics_apps-tests PRIVATE -DDISABLE_SYSTEM_CALL_TESTS=1)
endif()

add_test(NAME helics_apps-tests COMMAND helics_apps-tests --report_level=short)
set_property(TEST helics_apps-tests PROPERTY LABELS HelicsApps Coverage Daily)

# Tests for Continous Integration builds
add_test(
  NAME helics_apps-ci-tests
  COMMAND helics_apps-tests --run_test=@ci --report_level=short --log_level=test_suite
)
set_property(TEST helics_apps-ci-tests PROPERTY LABELS Continuous)

if(WIN32)
  foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
    add_custom_command(
      TARGET
      helics_apps-tests
      POST_BUILD # Adds a post-build event to core tests
      COMMAND
        ${CMAKE_COMMAND}
        -E
        copy_if_different # which executes "cmake - E copy_if_different..."
        "${keyfile}" # <--this is in-file
        "$<TARGET_FILE_DIR:helics_apps-tests>/"
    )
    # <--this is out-file path
  endforeach(keyfile)
endif(WIN32)
