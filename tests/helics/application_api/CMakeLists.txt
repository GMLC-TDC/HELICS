#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved. See LICENSE
# file and DISCLAIMER for more details.
#

set(application_api_test_headers testFixtures.hpp ValueFederateTestTemplates.hpp)

set(
  application_api_test_sources
  MessageFederateKeyTests.cpp
  ValueFederateKeyTests.cpp
  FilterTests.cpp
  testFixtures.cpp
  PrimaryTypeConversionTests.cpp
  FilterAdditionalTests.cpp
  ValueConverterTests.cpp
  application-api-tests.cpp
  data_viewTests.cpp
  MessageFederateAdditionalTests.cpp
  FederateTests.cpp
  subPubObjectTests.cpp
  ValueFederateAdditionalTests.cpp
  CombinationFederateTests.cpp
  ValueFederateExtendedTests.cpp
)

add_executable(
  application-api-tests ${application_api_test_sources} ${application_api_test_headers}
)
target_link_libraries(application-api-tests helics-static helics_test_base)

set_target_properties(application-api-tests PROPERTIES FOLDER tests)

target_compile_definitions(
  application-api-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
  application-api-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)
target_compile_definitions(
  application-api-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)
target_compile_definitions(application-api-tests PRIVATE "-DQUICK_TESTS_ONLY")

add_test(
  NAME application-api-tests
  COMMAND application-api-tests --log_level=message --report_level=short
)
set_property(TEST application-api-tests PROPERTY LABELS ApplicationApi Daily)

# Tests for Continuous Integration builds
add_test(
  NAME application-api-ci-tests
  COMMAND
    application-api-tests
    --run_test=@ci
    --log_level=test_suite
    --report_level=short
)
set_property(TEST application-api-ci-tests PROPERTY LABELS Continuous)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
  add_custom_command(
    TARGET
    application-api-tests
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "${keyfile}" # <--this is in-file
      "$<TARGET_FILE_DIR:application-api-tests>/"
  )
  # <--this is out-file path
endforeach(keyfile)
