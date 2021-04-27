# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_minimum_required(VERSION 2.8.7)
set(CLANG_FORMAT_MIN_VERSION "7.0")

function(set_git_hooks_enabled)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} config --local core.hooksPath .githooks
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    message(STATUS "Git hooks enabled")
endfunction()

function(set_git_hooks_disabled)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} config --local --unset core.hooksPath
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    message(STATUS "Git hooks disabled")
endfunction()

if (NOT DEFINED CLANG_FORMAT)
    find_program(CLANG_FORMAT clang-format)
endif ()

if (CLANG_FORMAT)
    execute_process(COMMAND ${CLANG_FORMAT} -version
            OUTPUT_VARIABLE CLANG_FORMAT_VERSION
            ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
endif ()

if (CLANG_FORMAT_VERSION MATCHES "^clang-format version .*")
    string(REGEX
            REPLACE "clang-format version ([.0-9]+).*"
            "\\1"
            CLANG_FORMAT_VERSION
            "${CLANG_FORMAT_VERSION}")

    if (NOT CLANG_FORMAT_VERSION VERSION_LESS ${CLANG_FORMAT_MIN_VERSION})
        set(CLANG_FORMAT_VERSION_OK ON)
    endif ()
else ()
    set(CLANG_FORMAT_VERSION_OK OFF)
endif ()

mark_as_advanced(CLANG_FORMAT)

if (HELICS_ENABLE_GIT_HOOKS AND NOT CLANG_FORMAT_VERSION_OK)
    message(WARNING
            "HELICS git hooks require clang-format version ${CLANG_FORMAT_MIN_VERSION} or higher.\n"
            "Install or update clang-format to use git hooks.\n"
            "To select a different clang-format executable, update the CLANG_FORMAT property in CMake.")
endif ()

if (EXISTS "${PROJECT_SOURCE_DIR}/.git")
    set(GIT_REPO_FOUND ON)
else ()
    set(GIT_REPO_FOUND OFF)
endif ()

if (WIN32)
    #there are a lot of potential issues doing this on windows, but the option should be available if desired
    cmake_dependent_advanced_option(HELICS_ENABLE_GIT_HOOKS "Activate git hooks to run clang-format on committed files." OFF "GIT_REPO_FOUND;CLANG_FORMAT;CLANG_FORMAT_VERSION_OK" OFF)
else()
    cmake_dependent_advanced_option(HELICS_ENABLE_GIT_HOOKS "Activate git hooks to run clang-format on committed files." ON "GIT_REPO_FOUND;CLANG_FORMAT;CLANG_FORMAT_VERSION_OK" OFF)
endif()

find_package(Git)

if (GIT_FOUND AND NOT GIT_VERSION_STRING VERSION_LESS "2.9")
    if (HELICS_ENABLE_GIT_HOOKS AND GIT_REPO_FOUND)
        set_git_hooks_enabled()
    elseif (GIT_REPO_FOUND)
        set_git_hooks_disabled()
    else()
        message(STATUS "Directory is not a Git repository. Git hooks are disabled automatically.")
    endif ()
elseif (HELICS_ENABLE_GIT_HOOKS AND GIT_FOUND AND GIT_VERSION_STRING VERSION_LESS "2.9")
    message(WARNING "Git version earlier than 2.9 found, update Git to enable git hooks.")
elseif (HELICS_ENABLE_GIT_HOOKS AND NOT GIT_FOUND)
    message(WARNING "Git not found, git hooks will not be enabled.")
endif ()
