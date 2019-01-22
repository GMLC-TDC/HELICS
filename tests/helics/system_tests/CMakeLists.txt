#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#

set(
  system_test_sources
  helics_system_tests.cpp
  TimingTests.cpp
  iterationTests.cpp
  ErrorTests.cpp
  federateRealTimeTests.cpp
  TimingTests2.cpp
  QueryTests.cpp
  heat-transfer-tests.cpp
  flagTests.cpp
  ../application_api/testFixtures.cpp
  ../application_api/testFixtures.hpp
)

add_executable(system-tests ${system_test_sources} ${system_test_headers})
target_link_libraries(system-tests helics-static helics_test_base)

set_target_properties(system-tests PROPERTIES FOLDER tests)

target_compile_definitions(
  system-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
  system-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)
target_compile_definitions(
  system-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)

add_test(
  NAME system-tests
  COMMAND system-tests --log_level=message --report_level=short
)
set_property(TEST system-tests PROPERTY LABELS System Coverage Daily)

# Tests for Continuous Integration builds
add_test(
  NAME system-ci-tests
  COMMAND
    system-tests
    --run_test=@ci
    --log_level=message
    --report_level=short
)
set_property(TEST system-ci-tests PROPERTY LABELS Continuous Valgrind)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
  add_custom_command(
    TARGET
    system-tests
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "${keyfile}" # <--this is in-file
      "$<TARGET_FILE_DIR:system-tests>/"
  ) # <--this is out-file path
endforeach(keyfile)
