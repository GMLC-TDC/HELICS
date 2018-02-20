##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
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
ZeromqCore-tests.cpp
MpiCore-tests.cpp
Node_Core-tests.cpp
testFixtures.cpp
IPCcore_tests.cpp
data-block-tests.cpp
UdpCore-tests.cpp
ForwardingTimeCoordinatorTests.cpp
TimeCoordinatorTests.cpp
)

if (NOT DISABLE_TCP_CORE)
list(APPEND core_test_sources TcpCore-tests.cpp)
endif()

add_executable(core-tests ${core_test_sources} ${core_test_headers})
target_link_libraries(core-tests helics-static)

include_directories(SYSTEM ${Boost_INCLUDE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ThirdParty)
include_directories(${PROJECT_SOURCE_DIR}/src)


target_link_libraries(core-tests ${Boost_LIBRARIES_test})

add_test(NAME core-tests COMMAND core-tests --log_level=warning --report_level=short)
set_target_properties (core-tests PROPERTIES FOLDER tests)

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET core-tests POST_BUILD        # Adds a post-build event to core tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:core-tests>/")                 # <--this is out-file path
endforeach(keyfile)
