##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

set(application_api_test_headers
testFixtures.h
)

set(application_api_test_sources
test_configuration.h
value-converter-tests.cpp
application-api-tests.cpp
data-block-tests.cpp
data-view-tests.cpp
test-simple-queue.cpp
testBlockingQueue.cpp
test-value-federate1.cpp
test-value-federate2.cpp
test-message-federate.cpp
test-filter-federate.cpp
federate_tests.cpp
heat-transfer-tests.cpp
subPubObjectTests.cpp
testFixtures.cpp
)

add_executable(application-api-tests ${application_api_test_sources} ${application_api_test_headers})
target_link_libraries(application-api-tests helics)

link_directories(${external_link_directories})

include_directories(${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)

target_link_libraries(application-api-tests ${external_library_list})
ADD_DEFINITIONS(-DHELICS_BROKER_LOCATION="${HELICS_BROKER_LOC}")
ADD_DEFINITIONS(-DHELICS_INSTALL_LOCATION="${CMAKE_INSTALL_PREFIX}")

add_test(NAME application-api-tests COMMAND application-api-tests)

#-----------------------------------------------------------------------------
# Setup CTEST environment
#-----------------------------------------------------------------------------
include(CTest)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET application-api-tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:application-api-tests>/")                 # <--this is out-file path
endforeach(keyfile)

