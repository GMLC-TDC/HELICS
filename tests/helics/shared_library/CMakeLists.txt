##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

set(c_shared_library_test_sources
ctestFixtures.cpp
shared-library-tests.cpp
#data_viewTests.cpp
test-value-federate1.cpp
test-value-federate2.cpp
#ValueFederateTests2.cpp
FilterTests.cpp

test-message-federate.cpp
#test-message-federate_cpp.cpp
#FederateTests.cpp
#TimingTests.cpp
#iterationTests.cpp
#subPubObjectTests.cpp
#QueryTests.cpp
)

set(cpp_shared_library_test_sources
cpptestFixtures.cpp
cppshared-library-tests.cpp
#test-value-federate1_cpp.cpp
test-value-federate2_cpp.cpp
#FilterTests_cpp.cpp
test-message-federate_cpp.cpp
#FederateTests.cpp
#TimingTests.cpp
#iterationTests.cpp
#subPubObjectTests.cpp
#QueryTests.cpp
)

add_executable(shared-library-tests ${c_shared_library_test_sources} ctestFixtures.hpp)

add_executable(shared-library-tests-cpp ${cpp_shared_library_test_sources} cpptestFixtures.hpp)

target_include_directories(shared-library-tests SYSTEM PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(shared-library-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_include_directories(shared-library-tests-cpp SYSTEM PRIVATE ${Boost_INCLUDE_DIR})
target_include_directories(shared-library-tests-cpp PRIVATE ${PROJECT_SOURCE_DIR}/src)

IF (VERSION_OPTION)
	IF (MSVC)
		target_compile_options(shared-library-tests PUBLIC ${VERSION_OPTION})
		target_compile_options(shared-library-tests-cpp PUBLIC ${VERSION_OPTION})
	ELSE()
		target_compile_options(shared-library-tests PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>)
		target_compile_options(shared-library-tests-cpp PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>)
	ENDIF (MSVC)
ENDIF(VERSION_OPTION)

target_link_libraries(shared-library-tests helicsSharedLib ${Boost_LIBRARIES_core} ${Boost_LIBRARIES_test})
target_link_libraries(shared-library-tests-cpp helicsSharedLib ${Boost_LIBRARIES_core} ${Boost_LIBRARIES_test})

set_target_properties (shared-library-tests shared-library-tests-cpp PROPERTIES FOLDER tests)

target_compile_definitions(shared-library-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\"")
target_compile_definitions(shared-library-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\"")

target_compile_definitions(shared-library-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME shared-library-tests COMMAND shared-library-tests --log_level=message --report_level=short)

target_compile_definitions(shared-library-tests-cpp PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\"")
target_compile_definitions(shared-library-tests-cpp PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\"")

target_compile_definitions(shared-library-tests-cpp PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

add_test(NAME shared-library-tests-cpp COMMAND shared-library-tests-cpp --log_level=test_suite --report_level=short)

if (WIN32)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET shared-library-tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:shared-library-tests>/")                 # <--this is out-file path
endforeach(keyfile)

    add_custom_command(TARGET shared-library-tests POST_BUILD        # Adds a post-build event to api tests
        COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
            "$<TARGET_FILE:helicsSharedLib>"      # <--this is in-file
            "$<TARGET_FILE_DIR:shared-library-tests>/")                 # <--this is out-file path
endif(WIN32)

