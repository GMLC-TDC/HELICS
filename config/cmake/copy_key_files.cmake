# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

macro(copy_key_files_to_target_location target)
    if(WIN32)

        foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
            add_custom_command(
                TARGET ${target} POST_BUILD # Adds a post-build event to api tests
                COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E
                                                          # copy_if_different..."
                    "${keyfile}" # <--this is in-file
                    "$<TARGET_FILE_DIR:${target}>/"
            )
            # <--this is out-file path
        endforeach(keyfile)

        if(TARGET libzmq)
            add_custom_command(
                TARGET ${target} POST_BUILD # Adds a post-build event to core tests
                COMMAND
                    ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E
                                                          # copy_if_different..."
                    "$<TARGET_FILE:libzmq>" # <--this is in-file
                    "$<TARGET_FILE_DIR:${target}>/"
            ) # <--this is out- file path
        endif()

    endif(WIN32)
endmacro()
macro(copy_shared_target target shared_target)
    add_custom_command(
        TARGET ${target} POST_BUILD # Adds a post-build event to api tests
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different # which executes "cmake - E
                                                  # copy_if_different..."
            "$<TARGET_FILE:${shared_target}>" # <--this is in- file
            "$<TARGET_FILE_DIR:${target}>/"
    ) # <--this is out- file path
endmacro()
macro(install_key_files_with_comp comp)
    install(
        FILES $<TARGET_FILE:helicsSharedLib>
        DESTINATION ${comp}
        COMPONENT ${comp}
    )
    install(
        FILES ${KEY_LIBRARY_FILES}
        DESTINATION ${comp}
        COMPONENT ${comp}
    )

    if(WIN32)
        if(TARGET libzmq)
            install(
                FILES $<TARGET_FILE:libzmq>
                DESTINATION ${comp}
                COMPONENT ${comp}
            )
        endif()

    endif(WIN32)
endmacro()
