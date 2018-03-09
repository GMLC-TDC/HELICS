##############################################################################
#Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################


set(key_test_files
MessageFederateKeyTests.cpp
ValueFederateKeyTests.cpp
FilterTests.cpp
testFixtures.cpp
)

set(application_api_test_headers
testFixtures.hpp
ValueFederateTestTemplates.hpp
)

set(application_api_test_sources
${key_test_files}
ValueConverterTests.cpp
application-api-tests.cpp
data_viewTests.cpp
MessageFederateAdditionalTests.cpp
FederateTests.cpp
TimingTests.cpp
iterationTests.cpp
subPubObjectTests.cpp
ValueFederateAdditionalTests.cpp
)

set(extended_test_sources
QueryTests.cpp
heat-transfer-tests.cpp
ValueFederateExtendedTests.cpp
)

include_directories( SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

#  QUICK API TESTS

add_executable(application-api-tests ${application_api_test_sources} ${application_api_test_headers})
target_link_libraries(application-api-tests helics-static)

target_link_libraries(application-api-tests ${Boost_LIBRARIES_test})

set_target_properties (application-api-tests PROPERTIES FOLDER tests)

#  KEY TESTS
add_executable(key-tests key-tests.cpp ${key_test_files} ${application_api_test_headers})
target_link_libraries(key-tests helics-static)

target_link_libraries(key-tests ${Boost_LIBRARIES_test})

set_target_properties (key-tests PROPERTIES FOLDER tests)

#  EXTENDED TESTS
add_executable(extended-api-tests ${application_api_test_sources} ${extended_test_sources} ${application_api_test_headers})

target_link_libraries(extended-api-tests helics-static)

target_link_libraries(extended-api-tests ${Boost_LIBRARIES_test})

set_target_properties (extended-api-tests PROPERTIES FOLDER tests)


#add a bigobj indicator for this project
IF(MINGW)
target_compile_options(application-api-tests PRIVATE "-Wa,-mbig-obj")
ENDIF(MINGW)

if (MSVC)
target_compile_options(application-api-tests PRIVATE /bigobj)
endif(MSVC)

target_compile_definitions(application-api-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\"")
target_compile_definitions(application-api-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\"")

target_compile_definitions(application-api-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

target_compile_definitions(extended-api-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\"")
target_compile_definitions(extended-api-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\"")

target_compile_definitions(extended-api-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
target_compile_definitions(application-api-tests PRIVATE "-DQUICK_TESTS_ONLY")

target_compile_definitions(key-tests PRIVATE "-DKEY_TESTS")

add_test(NAME key-tests COMMAND key-tests --log_level=test_suite --report_level=short)
add_test(NAME application-api-tests COMMAND application-api-tests --log_level=test_suite --report_level=short)

add_test(NAME extended-api-tests COMMAND extended-api-tests --log_level=warning --report_level=short)


foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET application-api-tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:application-api-tests>/")                 # <--this is out-file path
endforeach(keyfile)

