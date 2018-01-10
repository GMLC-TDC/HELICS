##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################


set(application_api_test_headers
testFixtures.h
)

set(application_api_test_sources
value-converter-tests.cpp
application-api-tests.cpp
data-view-tests.cpp
test-value-federate1.cpp
test-value-federate2.cpp
test-message-federate.cpp
test-filters.cpp
federate_tests.cpp
timing_tests.cpp
iteration_tests.cpp
subPubObjectTests.cpp
testFixtures.cpp
)

IF (NOT QUICK_TESTS_ONLY)
list(APPEND application_api_test_sources 
testQueries.cpp
heat-transfer-tests.cpp
)
ENDIF(NOT QUICK_TESTS_ONLY)


include_directories( SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(application-api-tests ${application_api_test_sources} ${application_api_test_headers})
target_link_libraries(application-api-tests helics-static)

target_link_libraries(application-api-tests ${Boost_LIBRARIES_test})

set_target_properties (application-api-tests PROPERTIES FOLDER tests)

#add a bigobj indicator for this project
IF(MINGW)
target_compile_options(application-api-tests PRIVATE "-Wa,-mbig-obj")
ENDIF(MINGW)

if (MSVC)
target_compile_options(application-api-tests PRIVATE /bigobj)
endif(MSVC)

ADD_DEFINITIONS(-DHELICS_BROKER_LOCATION="${HELICS_BROKER_LOC}")
ADD_DEFINITIONS(-DHELICS_INSTALL_LOCATION="${CMAKE_INSTALL_PREFIX}")

add_test(NAME application-api-tests COMMAND application-api-tests --log_level=success --report_level=detailed)



foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET application-api-tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:application-api-tests>/")                 # <--this is out-file path
endforeach(keyfile)

