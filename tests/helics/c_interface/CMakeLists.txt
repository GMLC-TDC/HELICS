##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################


set(c_interface_test_headers
ctestFixtures.h
)

set(c_interface_test_sources
test_configuration.h
c_interface_tests.cpp
test-value-federate1.cpp
test-value-federate2.cpp
test-message-federate.cpp
test-filter-federate.cpp
federate_tests.cpp
iteration_tests.cpp
ctestFixtures.cpp
)

include_directories( SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src/helics/shared_api_library)

add_executable(c_interface_tests ${c_interface_test_sources} ${c_interface_test_headers})


target_link_libraries(c_interface_tests helicsSharedLib ${external_library_list})
target_link_libraries(c_interface_tests helicsSharedLib ${Boost_LIBRARIES_test})

add_test(NAME c_interface_tests COMMAND c_interface_tests)

#-----------------------------------------------------------------------------
# Setup CTEST environment
#-----------------------------------------------------------------------------
include(CTest)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET c_interface_tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:c_interface_tests>/")                 # <--this is out-file path
endforeach(keyfile)

if(BUILD_SHARED_LIBS)
    add_custom_command(TARGET c_interface_tests POST_BUILD        # Adds a post-build event to api tests
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
            "$<TARGET_FILE:helicsSharedLib>"      # <--this is in-file
            "$<TARGET_FILE_DIR:c_interface_tests>/")                 # <--this is out-file path
endif()
