# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

include(GenerateExportHeader)

#for including the helics_export.h
target_include_directories(
        helics_base_includes
        INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    )


add_subdirectory(common)
add_subdirectory(core)
add_subdirectory(application_api)

# -------------------------------------------------------------
# Add the main HELICS library
# -------------------------------------------------------------


if(HELICS_BUILD_CXX_SHARED_LIB)

    add_subdirectory(cxx_shared_library)

    if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        set_target_properties(helics-shared PROPERTIES FOLDER helics)
    endif()
    generate_export_header(helics-shared BASE_NAME helics_cxx)
    file(
       COPY ${CMAKE_CURRENT_BINARY_DIR}/helics_cxx_export.h 
       DESTINATION ${PROJECT_BINARY_DIR}/include/helics/application_api
    )
else()
    generate_export_header(helics_application_api BASE_NAME helics_cxx)
endif()

file(
     COPY ${CMAKE_CURRENT_BINARY_DIR}/helics_cxx_export.h 
     DESTINATION ${PROJECT_BINARY_DIR}/include/
)

if(NOT HELICS_BINARY_ONLY_INSTALL)
    # Install the header if the C++ shared library is built
    if(HELICS_INSTALL_CPP14_HEADERS)
        install(
            FILES helics.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
            COMPONENT headers
        )
    endif()

endif()


if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_target_properties(
        helics_application_api helics_common helics_core
        PROPERTIES FOLDER helics
    )
endif()

option(
    HELICS_BUILD_APP_LIBRARY
    "Build helics application library (a static library containing objects for broker, player, recorder, and helics_apps"
    ON
)

cmake_dependent_option(
    HELICS_BUILD_APP_EXECUTABLES
    "Build helics applications to run various apps, including broker, player, recorder, and helics_apps"
    ON
    "HELICS_BUILD_APP_LIBRARY AND CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME"
    OFF
)

if(HELICS_BUILD_APP_LIBRARY)
    add_subdirectory(apps)
    if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        set_target_properties(helics_apps PROPERTIES FOLDER helics)
    endif()
endif(HELICS_BUILD_APP_LIBRARY)

if(NOT HELICS_DISABLE_C_SHARED_LIB)
    add_subdirectory(shared_api_library)
    add_subdirectory(cpp98)
    if(NOT HELICS_BINARY_ONLY_INSTALL)
        install(
            FILES chelics.h
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
            COMPONENT headers
        )
    endif()
endif()
