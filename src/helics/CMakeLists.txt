# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include(GenerateExportHeader)

option(
    HELICS_BUILD_APP_LIBRARY
    "Build helics application library (a static library containing objects for broker, player, recorder, and helics_apps"
    ON
)

cmake_dependent_option(
    HELICS_BUILD_APP_EXECUTABLES
    "Build helics applications to run various apps, including broker, player, recorder, and helics_apps"
    ON
    "HELICS_BUILD_APP_LIBRARY;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME"
    OFF
)

# for including the helics_export.h
target_include_directories(helics_base INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)

# -------------------------------------------------------------
# Add the main HELICS libraries
# -------------------------------------------------------------

add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(network)
add_subdirectory(application_api)

if(HELICS_BUILD_CXX_SHARED_LIB)

    add_subdirectory(cxx_shared_library)

    if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        set_target_properties(helics-shared PROPERTIES FOLDER helics)
    endif()
    generate_export_header(helics-shared BASE_NAME helics_cxx)
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/helics_cxx_export.h
         DESTINATION ${PROJECT_BINARY_DIR}/include/helics_cxx/helics/application_api
    )
else()
    generate_export_header(helics_application_api BASE_NAME helics_cxx)
endif()

file(COPY ${CMAKE_CURRENT_BINARY_DIR}/helics_cxx_export.h
     DESTINATION ${PROJECT_BINARY_DIR}/helics_generated_includes
)

if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_target_properties(helics_application_api helics_common helics_core PROPERTIES FOLDER helics)
endif()

if(HELICS_BUILD_APP_LIBRARY)
    add_subdirectory(apps)
    if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        set_target_properties(helics_apps PROPERTIES FOLDER helics)
    endif()
endif(HELICS_BUILD_APP_LIBRARY)

if(NOT HELICS_DISABLE_C_SHARED_LIB)
    add_subdirectory(shared_api_library)
    add_subdirectory(cpp98)
endif()
