# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

add_executable(comboFed comboFed.cpp)

target_link_libraries(comboFed PUBLIC helics_apps)
target_link_libraries(comboFed PRIVATE compile_flags_target)
set_target_properties(comboFed PROPERTIES FOLDER examples)

if(HELICS_BUILD_TESTS AND NOT MSVC)
    add_test(NAME comboFed_broker_example
             COMMAND ${PROJECT_SOURCE_DIR}/scripts/test_example.sh 10 --broker
                     $<TARGET_FILE:helics_broker> $<TARGET_FILE:comboFed>
    )
    set_property(TEST comboFed_broker_example PROPERTY LABELS Continuous)
endif()

# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh ${CMAKE_CURRENT_BINARY_DIR}/run.sh COPYONLY)

add_custom_command(
    TARGET comboFed
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/run.sh" # <--this is in- file
        "$<TARGET_FILE_DIR:comboFed>"
) # <--this is out-file path
