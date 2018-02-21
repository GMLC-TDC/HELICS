##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################


set(application_api_test_headers
testFixtures.hpp
)

set(application_api_test_sources
testFixtures.cpp
#ValueConverterTests.cpp
application-api-tests.cpp
#data_viewTests.cpp
ValueFederateTests1.cpp
#ValueFederateTests2.cpp
#MessageFederateTests.cpp
#FilterTests.cpp
#FederateTests.cpp
#TimingTests.cpp
#iterationTests.cpp
#subPubObjectTests.cpp
)

IF (NOT QUICK_TESTS_ONLY)
#list(APPEND application_api_test_sources 
#QueryTests.cpp
#heat-transfer-tests.cpp
#)
ENDIF(NOT QUICK_TESTS_ONLY)


include_directories( SYSTEM ${Boost_INCLUDE_DIR})
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(c-application-api-tests ${application_api_test_sources} ${application_api_test_headers})
target_link_libraries(c-application-api-tests helicsSharedLib)

target_link_libraries(c-application-api-tests ${Boost_LIBRARIES_test})

set_target_properties (c-application-api-tests PROPERTIES FOLDER tests)

#add a bigobj indicator for this project
IF(MINGW)
target_compile_options(c-application-api-tests PRIVATE "-Wa,-mbig-obj")
ENDIF(MINGW)

if (MSVC)
target_compile_options(c-application-api-tests PRIVATE /bigobj)
endif(MSVC)

target_compile_definitions(c-application-api-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\"")
target_compile_definitions(c-application-api-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\"")

target_compile_definitions(c-application-api-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME c-application-api-tests COMMAND c-application-api-tests --log_level=warning --report_level=short)



foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET c-application-api-tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:c-application-api-tests>/")                 # <--this is out-file path
endforeach(keyfile)

