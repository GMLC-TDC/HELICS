# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

add_executable(comboFed_shared comboFed.cpp)

target_link_libraries(comboFed_shared PRIVATE compile_flags_target)
set_target_properties(comboFed_shared PROPERTIES FOLDER examples)
target_link_libraries(comboFed_shared PUBLIC helics-shared)

add_custom_command(
    TARGET comboFed_shared
    POST_BUILD # Adds a post-build event to api tests
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake -E copy_if_different..."
        "${CMAKE_CURRENT_SOURCE_DIR}/run.sh" # <--this is in- file
        "$<TARGET_FILE_DIR:comboFed_shared>/"
) # <--this is out-file path
