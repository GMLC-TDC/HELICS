##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################


set(shared_library_test_headers
ctestFixtures.hpp
cpptestFixtures.hpp
)

set(shared_library_test_sources
ctestFixtures.cpp
cpptestFixtures.cpp
#ValueConverterTests.cpp
shared-library-tests.cpp
#data_viewTests.cpp
#ValueFederateTests1.cpp
test-value-federate1.cpp
test-value-federate2.cpp
test-value-federate1_cpp.cpp
test-value-federate2_cpp.cpp
#ValueFederateTests2.cpp
#MessageFederateTests.cpp
FilterTests.cpp
FilterTests_cpp.cpp
test-message-federate.cpp
test-message-federate_cpp.cpp
#FederateTests.cpp
#TimingTests.cpp
#iterationTests.cpp
#subPubObjectTests.cpp
#QueryTests.cpp
)

include_directories( SYSTEM ${Boost_INCLUDE_DIR})

include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(shared-library-tests ${shared_library_test_sources} ${shared_library_test_headers})
target_link_libraries(shared-library-tests helicsSharedLib)

target_link_libraries(shared-library-tests ${Boost_LIBRARIES_test})

set_target_properties (shared-library-tests PROPERTIES FOLDER tests)


target_compile_definitions(shared-library-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\"")
target_compile_definitions(shared-library-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\"")

target_compile_definitions(shared-library-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME shared-library-tests COMMAND shared-library-tests --log_level=test_suite --report_level=short)



if (WIN32)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET shared-library-tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:shared-library-tests>/")                 # <--this is out-file path
endforeach(keyfile)

if(BUILD_C_SHARED_LIB)
    add_custom_command(TARGET shared-library-tests POST_BUILD        # Adds a post-build event to api tests
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
            "$<TARGET_FILE:helicsSharedLib>"      # <--this is in-file
            "$<TARGET_FILE_DIR:shared-library-tests>/")                 # <--this is out-file path
endif()
endif(WIN32)

