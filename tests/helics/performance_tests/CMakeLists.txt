##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################


set(perf_sources
performance-tests.cpp
multiEchoTest.cpp
)


add_executable(perf-tests ${perf_sources})
target_link_libraries(perf-tests helics-static helics_test_base)

set_target_properties (perf-tests PROPERTIES FOLDER tests)

target_compile_definitions(perf-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\"")
target_compile_definitions(perf-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\"")

target_compile_definitions(perf-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
add_custom_command(TARGET perf-tests POST_BUILD        # Adds a post-build event to api tests
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${keyfile}"      # <--this is in-file
        "$<TARGET_FILE_DIR:perf-tests>/")                 # <--this is out-file path
endforeach(keyfile)

