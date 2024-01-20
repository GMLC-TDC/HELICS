# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# cmake script to set up build output directories

# -----------------------------------------------------------------------------
# set the install path to a local directory
# -----------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(WIN32)
        if(MSVC)
            set(CMAKE_INSTALL_PREFIX "C:/local/helics_${HELICS_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path" FORCE
            )
        elseif(MINGW AND NOT MSYS)
            set(CMAKE_INSTALL_PREFIX "C:/local/helics_${HELICS_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path" FORCE
            )
        elseif(MSYS)
            # use CMAKE_OBJCOPY here since it is somewhat less likely to be overridden by users
            # rather than the compiler
            get_filename_component(path_bin ${CMAKE_OBJCOPY} DIRECTORY)
            get_filename_component(path_install ${path_bin} DIRECTORY)
            set(CMAKE_INSTALL_PREFIX ${path_install} CACHE PATH "default install path" FORCE)
        endif(MSVC)
    endif(WIN32)
endif()

# Warning if CMAKE_INSTALL_PREFIX is empty. Likely set by using the wrong environment variable.
if(NOT CMAKE_INSTALL_PREFIX)
    message(
        WARNING
            "CMAKE_INSTALL_PREFIX is set to nothing. If you are using an environment variable for handling prefix paths, that variable might not have been set before using it with CMake to set the CMAKE_INSTALL_PREFIX option."
    )
endif()

# Check to make sure the install prefix isn't the build folder, if it is, build errors will happen
get_filename_component(tmp_install_prefix "${CMAKE_INSTALL_PREFIX}" REALPATH)
get_filename_component(tmp_proj_bindir "${PROJECT_BINARY_DIR}" REALPATH)
# Windows paths are case insensitive
if(WIN32)
    string(TOLOWER "${tmp_install_prefix}" tmp_install_prefix)
    string(TOLOWER "${tmp_proj_bindir}" tmp_proj_bindir)
endif()
if(tmp_install_prefix STREQUAL tmp_proj_bindir)
    message(FATAL_ERROR "CMAKE_INSTALL_PREFIX must not be set to the build folder")
endif()

if(MSYS
   OR CYGWIN
   OR UNIX
   OR APPLE
)
    set(UNIX_LIKE TRUE)
endif()

# Set the build output paths
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH
                                                                           "Archive output dir."
        )
    endif()
    if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib" CACHE PATH
                                                                           "Library output dir."
        )
    endif()
    if(NOT CMAKE_PDB_OUTPUT_DIRECTORY)
        set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            CACHE PATH "PDB (MSVC debug symbol)output dir."
        )
    endif()
    if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
            CACHE PATH "Executable/dll output dir."
        )
    endif()
endif()

# Prohibit in-source build
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    if(EXISTS "${PROJECT_SOURCE_DIR}/CMakeCache.txt")
        message(
            WARNING
                "The source directory ${PROJECT_SOURCE_DIR} contains CMakeCache.txt and possibly other cmake folders like CMakeFiles that might interfere with the current build"
        )
    endif()
    message(
        FATAL_ERROR
            "In-source build is not supported. Please, use an empty directory for building the project."
    )
endif()
