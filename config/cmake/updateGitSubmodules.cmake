# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Based on https://cliutils.gitlab.io/modern-cmake/chapters/projects/submodule.html

find_package(Git QUIET)
if(GIT_FOUND AND (GIT_VERSION_STRING VERSION_GREATER "1.5.2"))
    if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
        option(${PROJECT_NAME}_ENABLE_SUBMODULE_UPDATE "Checkout and update git submodules" ON)
        mark_as_advanced(${PROJECT_NAME}_ENABLE_SUBMODULE_UPDATE)
    else()
        message(STATUS "${PROJECT_SOURCE_DIR} is not a Git repository. Clone ${PROJECT_NAME} with Git or ensure you get copies of all the Git submodules code.")
    endif()
endif()

macro(submod_update_all)
  if (${PROJECT_NAME}_ENABLE_SUBMODULE_UPDATE)
    message(STATUS "Git Submodule Update")
            execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                            RESULT_VARIABLE GIT_RESULT
                            OUTPUT_VARIABLE GIT_OUTPUT)
      if(GIT_RESULT)
        message(WARNING "Automatic submodule checkout with `git submodule --init` failed with error ${GIT_RESULT} and output ${GIT_OUTPUT}. Checkout the submodules before building.")
      endif()
    else()
     message(WARNING "SUBMODULE update has been disabled")
   endif()
endmacro()

macro(submod_update target)
  if (${PROJECT_NAME}_ENABLE_SUBMODULE_UPDATE)
     message(STATUS "Git Submodule Update ${target}")
     execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init -- ${target}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GIT_RESULT
        OUTPUT_VARIABLE GIT_OUTPUT)
     if(GIT_RESULT)
         message(WARNING "Automatic submodule checkout with `git submodule --init -- $[target}` failed with error ${GIT_RESULT} and output ${GIT_OUTPUT}. Checkout the submodules before building.")
     endif()
   else()
     message(WARNING "SUBMODULE update has been disabled")
   endif()
endmacro()

function(check_submodule_status)
    if (${PROJECT_NAME}_ENABLE_SUBMODULE_UPDATE)
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule status
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_RESULT
                OUTPUT_VARIABLE GIT_OUTPUT)

        if (GIT_RESULT)
            message(WARNING "Automatic submodule verification with `git submodule status` failed with error ${GIT_RESULT} and output ${GIT_OUTPUT}. Verify submodules before building.")
        endif ()

        if (WIN32 AND NOT (MSYS OR CYGWIN))
            execute_process(COMMAND powershell "-c" "echo '${GIT_OUTPUT}' | Select-String -Pattern '^[-\\|+\\|U].*'"
                    OUTPUT_VARIABLE SUBMODULE_STATUS
                    )
        else ()
            execute_process(COMMAND bash "-c" "echo '${GIT_OUTPUT}' | grep '^[-\\|+\\|U].*'"
                    OUTPUT_VARIABLE SUBMODULE_STATUS
                    )
        endif ()
        if (NOT "${SUBMODULE_STATUS}" STREQUAL "")
            message(WARNING "Submodules are not up to date. Update submodules by running `git submodule update --init` before building HELICS."
            "\nOut of date submodules:\n ${SUBMODULE_STATUS}")
        endif ()
    endif ()
endfunction()
