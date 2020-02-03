# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
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

# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh ${CMAKE_CURRENT_BINARY_DIR}/run.sh
# COPYONLY)

add_custom_command(
    TARGET
    comboFed
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
        ${CMAKE_COMMAND}
        -E
        copy_if_different # which executes "cmake - E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/run.sh" # <--this is in- file
        "$<TARGET_FILE_DIR:comboFed>"
) # <--this is out-file path

copy_key_files_to_target_location(comboFed)

