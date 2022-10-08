# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

find_package(Git)

# this function is to get a nicely formatted build string for later reference in a human readable
# format that has some utility for automation. the resulting format would be empty if the build
# matches a tag exactly, and otherwise <branch_name>-<git hash>[-dirty]  if dirty is present there
# are uncommitted changes in the repo called like git_version_describe(${PROJECT_SOURCE_DIR} result)
# then result will contain the description string

function(git_version_describe source_path result)
    set(tag_desc "")
    # message(STATUS "Source ${source_path}")
    if(GIT_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} -C ${source_path} --work-tree ${source_path} describe --tags
            OUTPUT_VARIABLE TAG_DESCRIPTION
            ERROR_VARIABLE TAGGING_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if(TAGGING_ERROR)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} -C ${source_path} --work-tree ${source_path} describe
                OUTPUT_VARIABLE TAG_DESCRIPTION
                ERROR_VARIABLE TAGGING_ERROR
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        endif()
        if(TAGGING_ERROR)
            execute_process(
                COMMAND ${GIT_EXECUTABLE} -C ${source_path} --work-tree ${source_path} describe
                        --all --tags --dirty --long
                OUTPUT_VARIABLE TAG_DESCRIPTION
                ERROR_VARIABLE TAGGING_ERROR
                OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            message(STATUS "got ${TAG_DESCRIPTION} from third try")
        endif()
        if(NOT TAGGING_ERROR)
            string(LENGTH ${TAG_DESCRIPTION} tag_length)
            if(tag_length GREATER 16)
                string(FIND ${TAG_DESCRIPTION} "-" last_dash_loc REVERSE)
                if(last_dash_loc GREATER 0)
                    string(SUBSTRING ${TAG_DESCRIPTION} ${last_dash_loc} -1 hash_string)
                    execute_process(
                        COMMAND ${GIT_EXECUTABLE} -C ${source_path} --work-tree ${source_path}
                                describe --all --tags --dirty
                        OUTPUT_VARIABLE TAG_DESCRIPTION2
                        ERROR_VARIABLE TAGGING_ERROR
                        OUTPUT_STRIP_TRAILING_WHITESPACE
                    )
                    if(TAGGING_ERROR)
                        set(TAG_DESCRIPTION2 ${TAG_DESCRIPTION})
                    endif()
                    string(FIND ${TAG_DESCRIPTION2} ${hash_string} hash_loc)
                    if(hash_loc LESS 0)
                        string(FIND ${TAG_DESCRIPTION2} "-dirty" dirty_loc)
                        if(dirty_loc GREATER 0)
                            string(SUBSTRING ${TAG_DESCRIPTION2} 0 ${dirty_loc} tag_desc)
                            string(APPEND tag_desc ${hash_string} "-dirty")
                        else()
                            string(APPEND tag_desc ${TAG_DESCRIPTION2} ${hash_string})
                        endif()
                    else()
                        string(APPEND tag_desc ${TAG_DESCRIPTION2})
                    endif()
                    string(FIND ${tag_desc} "heads/" headloc)
                    if(headloc EQUAL 0)
                        string(SUBSTRING ${tag_desc} 6 -1 tag_desc)
                    endif()
                endif()
            endif()
        endif()

        set(${result} ${tag_desc} PARENT_SCOPE)
    else(GIT_FOUND)
        set(${result} "" PARENT_SCOPE)
    endif(GIT_FOUND)
endfunction()
