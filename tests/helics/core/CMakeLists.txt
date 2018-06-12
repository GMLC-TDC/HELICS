##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

#-----------------------------------------------------------------------------
# Core tests using Boost
#-----------------------------------------------------------------------------

set(core_test_headers
testFixtures.h
)

set(core_test_sources
core-tests.cpp
InfoClass-tests.cpp
FederateState-tests.cpp
ActionMessage-tests.cpp
#CoreBroker-tests.cpp
CoreFactory-tests.cpp
#CommonCore-tests.cpp
TestCore-tests.cpp
Node_Core-tests.cpp
testFixtures.cpp
IPCcore_tests.cpp
data-block-tests.cpp
UdpCore-tests.cpp
ForwardingTimeCoordinatorTests.cpp
TimeCoordinatorTests.cpp
networkInfoTests.cpp
)
if (HELICS_HAVE_ZEROMQ)
	list(APPEND core_test_sources ZeromqCore-tests.cpp)
endif(HELICS_HAVE_ZEROMQ)

if (NOT DISABLE_TCP_CORE)
    list(APPEND core_test_sources TcpCore-tests.cpp)
endif()

if (MPI_ENABLE)
    list(APPEND core_test_sources MpiCore-tests.cpp)
endif()

add_executable(core-tests ${core_test_sources} ${core_test_headers})
target_link_libraries(core-tests helics-static helics_test_base)

target_include_directories(core-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
set_target_properties (core-tests PROPERTIES FOLDER tests)

add_test(NAME core-tests COMMAND core-tests --log_level=message --report_level=short)
set_property(TEST core-tests PROPERTY LABELS Core)

# Tests for Continuous, Daily, and Release builds
add_test(NAME core-ci-tests COMMAND core-tests --run_test=@ci --log_level=message --report_level=short)
add_test(NAME core-daily-tests COMMAND core-tests --run_test=@daily --log_level=message --report_level=short)
add_test(NAME core-release-tests COMMAND core-tests --run_test=@release --log_level=message --report_level=short)
set_property(TEST core-ci-tests PROPERTY LABELS Continuous)
set_property(TEST core-daily-tests PROPERTY LABELS Daily)
set_property(TEST core-release-tests PROPERTY LABELS Release)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET core-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:core-tests>/")                 # <--this is out-file path
endforeach(keyfile)

